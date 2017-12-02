/*
* Copyright (C) 2017 Fredrik Skogman, skogman - at - gmail.com.
* This file is part of pie project
*
* The contents of this file are subject to the terms of the Common
* Development and Distribution License (the "License"). You may not use this
* file except in compliance with the License. You can obtain a copy of the
* License at http://opensource.org/licenses/CDDL-1.0. See the License for the
* specific language governing permissions and limitations under the License.
* When distributing the software, include this License Header Notice in each
* file and include the License file at http://opensource.org/licenses/CDDL-1.0.
*/

'use strict';

function defaultControlPoints() {
    let points =  [
        {
            'x': -0.2, 'y': -0.2
        },
        {
            'x': 0.0, 'y': 0.0
        },
        {
            'x': 1.0, 'y': 1.0
        },
        {
            'x': 1.2, 'y': 1.2
        },
    ];

    return points;
}

function Curve(canvas, color, callback = null) {
    this.canvas = canvas;
    this.controlPoints = defaultControlPoints();
    this.linePoints = [];
    this.color = color;
    this.w = canvas.width;
    this.h = canvas.height;
    this.selectedPoint = null;
    this.callback = callback;

    if (this.color == null || this.color == "") {
        this.color = "black";
    }

    if (this.w != this.h) {
        alert("curve canvas is not square");
    }

    /*
     * Initialize listeners to canvas.
     */
    setupCurveListeners(this, canvas);

    /*
     * Insert a new control point. Point's coordinates in [0, 1]
     */
    this.insertControlPoint = function(np) {
        var newP = [];
        var changed = false;

        /* controlPoints is sorted by x axis */
        for (let p of this.controlPoints) {
            if (np.x < p.x && !changed) {
                newP.push(np)
                changed = true;
            }
            newP.push(p);
        }

        this.controlPoints = newP;

        return changed;
    };

    /*
     * Remove a control point.
     */
    this.removeControlPoint = function(cp) {
        let newP = [];
        let len = this.controlPoints.length;
        let i = 0;

        /* Never allow removal of i = 0, 1, len - 2, len - 1*/
        for (let p of this.controlPoints) {
            let keep = p != cp;

            if (!keep) {
                if (i == 0 || i == len - 1) {
                    keep = true;
                } else if (i == 1) {
                    p.x = 0.0;
                    p.y = 0.0;
                    keep = true;
                } else if (i == len - 2) {
                    p.x = 1.0;
                    p.y = 1.0;
                    keep = true;
                }
            }

            if (keep) {
                newP.push(p);
            }
            i++;
        };

        this.controlPoints = newP;
        this.realignControlPoints();
    };

    /*
     * Make sure the outer most control point are in a straight line
     * with the outer most points in relation with the closest control
     * point. This gives a smoother curve
     */
    this.realignControlPoints = function() {
        let len = this.controlPoints.length;

        /* Left part: 0 1 2*/
        let dx = this.controlPoints[2].x - this.controlPoints[1].x;
        let dy = this.controlPoints[2].y - this.controlPoints[1].y;
        this.controlPoints[0].x = this.controlPoints[1].x - dx;
        this.controlPoints[0].y = this.controlPoints[1].y - dy;

        /* right part: length - 3, length - 2, length - 1 */
        dx = this.controlPoints[len - 2].x - this.controlPoints[len - 3].x;
        dy = this.controlPoints[len - 2].y - this.controlPoints[len - 3].y;
        this.controlPoints[len - 1].x = this.controlPoints[len - 2].x + dx;
        this.controlPoints[len - 1].y = this.controlPoints[len - 2].y + dy;
    };

    /*
     * Move a control point cp to the new position np.
     * Coordinates are in [0, 1].
     */
    this.moveControlPoint = function(cp, np) {
        let len = this.controlPoints.length;

        /* Do not allow movement of edge nodes */
        for (let i = 1; i < this.controlPoints.length - 1; i++) {
            let p = this.controlPoints[i];

            if (p != cp) {
                continue;
            }

            /* end points can only move along the axis */
            if (i == 1) {
                console.log(np);
                if (np.x < 0) {
                    np.x = 0;
                }
                if (np.y < 0) {
                    np.y = 0;
                }
                if (np.x > np.y) {
                    np.y = 0;
                } else {
                    np.x = 0;
                }
            } else if (i == len - 2) {
                if (np.x > 1.0) {
                    np.x = 1.0;
                }
                if (np.y > 1.0) {
                    np.y = 1.0;
                }

                if (np.x > np.y) {
                    np.x = 1.0;
                } else {
                    np.y = 1.0;
                }
            } else {
                /* Neighbour points always exist */
                let prev = this.controlPoints[i-1];
                let next = this.controlPoints[i+1];
                /* Do not allow moving a point before/past a neighbouring
                   point */

                if (np.x < prev.x + 0.05) {
                    continue;
                }
                if (np.x > next.x - 0.05) {
                    continue;
                }

                /* Prohibit creating a line that can not be described by a
                   function (will have multiple outputs for the same input) */
                if (np.x < 0.15 && np.y > 0.85) {
                    continue;
                }
                if (np.x > 0.85 && np.y < 0.15) {
                    continue;
                }
            }
            p.x = np.x;
            p.y = np.y;
        }

        this.realignControlPoints();
    };

    this.render = function() {
        var ctx = this.canvas.getContext("2d");

        this.linePoints = pieCatmRomChain(this.controlPoints, 100);

        /* reset canvas */
        ctx.setTransform(1, 0, 0, 1, 0, 0);
        ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        /* Draw grid */
        ctx.save();
        ctx.lineWidth = 1;
        ctx.strokeStyle = "#595959";
        ctx.setLineDash([3, 5]); /* dash, space pixels */
        ctx.beginPath();
        /* x axis */
        ctx.moveTo(50, 195);
        ctx.lineTo(50, 5);
        ctx.moveTo(100, 195);
        ctx.lineTo(100, 5);
        ctx.moveTo(150, 195);
        ctx.lineTo(150, 5);
        /* y axis */
        ctx.moveTo(5, 50);
        ctx.lineTo(195, 50);
        ctx.moveTo(5, 100);
        ctx.lineTo(195, 100);
        ctx.moveTo(5, 150);
        ctx.lineTo(195, 150);
        ctx.stroke();
        ctx.restore();

        /* Draw cuve */
        ctx.strokeStyle = this.color;
        ctx.beginPath();
        ctx.moveTo(0, 200);
        for (let p of this.linePoints) {
            let cp = this.toCanvasCoord(p);
            ctx.lineTo(cp.x, cp.y);
        }
        ctx.stroke();

        /* Draw a straight line for reference */
        ctx.beginPath();
        ctx.moveTo(0, 200);
        ctx.lineTo(200, 0);
        ctx.stroke();

        /* plot control points */
        for (let i = 1; i < this.controlPoints.length - 1; i++) {
            let cp = this.toCanvasCoord(this.controlPoints[i]);

            ctx.fillStyle = this.color;
            ctx.fillRect(cp.x - 2, cp.y - 2, 5, 5);
        }
    }

    /*
     * Change base from [0, 1] to the canvas'.
     */
    this.toCanvasCoord = function(p) {
        var scale = this.w;
        var cp = {'x': 0.0, 'y': 0.0};

        cp.x = p.x * scale;
        cp.y = this.h - p.y * scale;

        return cp;
    };

    /*
     * Change base from canvas' to [0, 1].
     */
    this.to01Coord = function(p) {
        var scale = this.w;
        var cp = {'x': 0.0, 'y': 0.0};

        cp.x = p.x / scale;
        cp.y = (this.h - p.y) / scale;

        return cp;
    };
    this.render();
};

function setupCurveListeners(curve, canvas) {
    canvas.addEventListener('mousedown', function(event) {
        let rect = canvas.getBoundingClientRect();
        let x = event.clientX - rect.left;
        let y = event.clientY - rect.top;
        let candidate = null;
        let dist = 10000;

        /* Is an existing control point selected? */
        for (let p of curve.controlPoints) {
            let cp = curve.toCanvasCoord(p)
            let dx = Math.abs(cp.x - x);
            let dy = Math.abs(cp.y - y);

            if (dx < 7 && dy < 7) {
                let thisDist = dx * dx + dy * dy;

                if (thisDist < dist) {
                    candidate = p;
                    dist = thisDist;
                }
            }
        }

        if (candidate != null) {
            curve.selectedPoint = candidate;
            return;
        }

        candidate = null;
        dist = 10000;
        for (let p of curve.linePoints) {
            let cp = curve.toCanvasCoord(p)
            let dx = Math.abs(cp.x - x);
            let dy = Math.abs(cp.y - y);

            if (dx < 7 && dy < 7) {
                let thisDist = dx * dx + dy * dy;

                if (thisDist < dist) {
                    candidate = p;
                    dist = thisDist;
                }
            }
        }

        if (candidate != null) {
            if (curve.insertControlPoint(candidate)) {
                curve.render();
                curve.selectedPoint = candidate;
            }
        }
    });

    canvas.addEventListener('dblclick', function(event) {
        let rect = canvas.getBoundingClientRect();
        let x = event.clientX - rect.left;
        let y = event.clientY - rect.top;
        let candidate = null;
        let dist = 10000;

        /* Is an existing control point selected? */
        for (let p of curve.controlPoints) {
            let cp = curve.toCanvasCoord(p)
            let dx = Math.abs(cp.x - x);
            let dy = Math.abs(cp.y - y);

            if (dx < 7 && dy < 7) {
                let thisDist = dx * dx + dy * dy;

                if (thisDist < dist) {
                    candidate = p;
                    dist = thisDist;
                }
            }
        }

        if (candidate != null) {
            curve.removeControlPoint(candidate);
            curve.render();

            if (curve.callback != null) {
                curve.callback();
            }
        }
    });

    canvas.addEventListener('mouseup', function(event) {
        curve.selectedPoint = null;
    });

    canvas.addEventListener('mousemove', function(event) {
        if (curve.selectedPoint != null){
            let rect = canvas.getBoundingClientRect();
            let x = event.clientX - rect.left;
            let y = event.clientY - rect.top;
            let np = curve.to01Coord({'x': x, 'y': y});

            curve.moveControlPoint(curve.selectedPoint, np);
            curve.render();

            if (curve.callback != null) {
                curve.callback();
            }
        }
    });
}

/**
 * p is four control points (P0, P1, P2, P3).
 * numP is number of points in output curve.
 */
function pieCatmRom(p, numP) {
    var out = [];
    var t0 = 0.0;
    var t1 = pieCmTj(t0, p[0], p[1]);
    var t2 = pieCmTj(t1, p[1], p[2]);
    var t3 = pieCmTj(t2, p[2], p[3]);
    var step = (t2 - t1) / (numP - 1);
    var t = t1;

    for (var i = 0; i < numP; i++) {
        var a1 = {'x': 0.0, 'y': 0.0};
        var a2 = {'x': 0.0, 'y': 0.0};
        var a3 = {'x': 0.0, 'y': 0.0};
        var b1 = {'x': 0.0, 'y': 0.0};
        var b2 = {'x': 0.0, 'y': 0.0};
        var c = {'x': 0.0, 'y': 0.0};

        a1.x = ((t1 - t) * p[0].x)/(t1 - t0) + ((t - t0) * p[1].x)/(t1 - t0);
        a1.y = ((t1 - t) * p[0].y)/(t1 - t0) + ((t - t0) * p[1].y)/(t1 - t0);
        a2.x = ((t2 - t) * p[1].x)/(t2 - t1) + ((t - t1) * p[2].x)/(t2 - t1);
        a2.y = ((t2 - t) * p[1].y)/(t2 - t1) + ((t - t1) * p[2].y)/(t2 - t1);
        a3.x = ((t3 - t) * p[2].x)/(t3 - t2) + ((t - t2) * p[3].x)/(t3 - t2);
        a3.y = ((t3 - t) * p[2].y)/(t3 - t2) + ((t - t2) * p[3].y)/(t3 - t2);

        b1.x = ((t2 - t) * a1.x)/(t2 - t0) + ((t - t0) * a2.x)/(t2 - t0);
        b1.y = ((t2 - t) * a1.y)/(t2 - t0) + ((t - t0) * a2.y)/(t2 - t0);
        b2.x = ((t3 - t) * a2.x)/(t3 - t1) + ((t - t1) * a3.x)/(t3 - t1);
        b2.y = ((t3 - t) * a2.y)/(t3 - t1) + ((t - t1) * a3.y)/(t3 - t1);

        c.x = ((t2 - t) * b1.x)/(t2 - t1) + ((t - t1) * b2.x)/(t2 - t1);
        c.y = ((t2 - t) * b1.y)/(t2 - t1) + ((t - t1) * b2.y)/(t2 - t1);

        t += step;
        out.push(c);
    }

    return out;
}

/**
 * Chain centripetal Catmull-Rom splines.
 * Connecteing points will be duplicated.
 * p is the list of control points.
 * if p is p0, p1, p2, p3, p4 lines will be drawn in [p1, p3].
 * numP is number of points to generate in each segment.
 */
function pieCatmRomChain(p, numP) {
    var out = [];

    for (var i = 0; i < p.length - 3; i++) {
        var tmp = pieCatmRom(p.slice(i, i + 4), numP);

        Array.prototype.push.apply(out, tmp);
    }

    return out;
}

function pieCmTj(ti, pi, pj) {
    var alpha = 0.5;
    var tx = pj.x - pi.x;
    var ty = pj.y - pi.y;
    var res;

    tx = tx * tx;
    ty = ty * ty;

    res = Math.pow(tx + ty, 0.5);
    res = Math.pow(res, alpha) + ti;

    return res;
}
