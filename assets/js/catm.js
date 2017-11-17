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

var curvePoints = [
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
var linePoints = [];
var selectedPoint = null;

function insertControlPoint(np) {
    var newP = [];
    var changed = false;

    /* controlPoints is sorted by x axis */
    for (let p of curvePoints) {
        if (np.x < p.x && !changed) {
            newP.push(np)
            changed = true;
        }
        newP.push(p);
    }

    curvePoints = newP;

    return changed;
}

function removeControlPoint(cp) {
    let newP = [];
    let len = curvePoints.length;
    let i = 0;

    /* Never allow removal of i = 0, 1, len - 2, len - 1*/
    for (let p of curvePoints) {
        let keep = p != cp;

        if (!keep) {
            if (i == 0 ||
                i == 1 ||
                i == len - 2 ||
                i == len -1) {
                keep = true;
            }
        }

        if (keep) {
            newP.push(p);
        }
        i++;
    }

    curvePoints = newP;

    realignControlPoints();
}

function moveControlPoint(cp, np) {
    let len = curvePoints.length;
    let i = 0;

    for (let p of curvePoints) {
        if (p == cp) {
            /* Do not allow movement of i = 0, len - 1*/
            if (i == 0 || i == len - 1) {
                break;
            }

            /* end points can only move along the axis */
            if (i == 1) {
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
            }

            p.x = np.x;
            p.y = np.y;

        }
        i++;
    }

    realignControlPoints();
}

function realignControlPoints() {
    /* make sure the outer most control point are in a straight line
       with the outer most points in relation with the closest control
       point. This gives a smoother curve */
    let len = curvePoints.length;

    /* Left part: 0 1 2*/
    let dx = curvePoints[2].x - curvePoints[1].x;
    let dy = curvePoints[2].y - curvePoints[1].y;
    curvePoints[0].x = curvePoints[1].x - dx;
    curvePoints[0].y = curvePoints[1].y - dy;

    /* right part: length - 3, length - 2, length - 1 */
    dx = curvePoints[len - 2].x - curvePoints[len - 3].x;
    dy = curvePoints[len - 2].y - curvePoints[len - 3].y;
    curvePoints[len - 1].x = curvePoints[len - 2].x + dx;
    curvePoints[len - 1].y = curvePoints[len - 2].y + dy;
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

function toCanvasCoord(p) {
    /* ps is [0, 1] */
    /* lower left is 200, 400
       upper right is 400, 200 */
    var scale = 400 - 200;
    var cp = {'x': 0.0, 'y': 0.0};

    cp.x = p.x * scale + 200;
    cp.y = 400 - p.y * scale;

    return cp;
}

function to01Coord(p) {
    var scale = 400 - 200;
    var cp = {'x': 0.0, 'y': 0.0};

    cp.x = (p.x - 200) / scale;
    cp.y = (400 - p.y) / scale;

    return cp;
}

function renderCurve() {
    var c = document.getElementById("curve_canvas");
    var ctx = c.getContext("2d");

    linePoints = pieCatmRomChain(curvePoints, 100);

    /* reset canvas */
    ctx.setTransform(1, 0, 0, 1, 0, 0);
    ctx.clearRect(0, 0, c.width, c.height);

    /* Draw cuve */
    ctx.beginPath();
    ctx.moveTo(200, 400);
    for (let p of linePoints) {
        let cp = toCanvasCoord(p);
        ctx.lineTo(cp.x, cp.y);
    }
    ctx.stroke();

    ctx.beginPath();
    ctx.moveTo(200, 400);
    ctx.lineTo(400, 200);
    ctx.stroke();

    ctx.strokeStyle = "black";
    ctx.strokeRect(200, 200, 200, 200);

    /* plot control points */
    for (let p of curvePoints) {
        let cp = toCanvasCoord(p);

        ctx.fillStyle = "red";
        ctx.fillRect(cp.x - 2, cp.y - 2, 5, 5);
    }
}

window.addEventListener("load", function(evt) {
    var crvCanvas = document.getElementById("curve_canvas");

    crvCanvas.addEventListener('mousedown', function(event) {
        let rect = crvCanvas.getBoundingClientRect();
        let x = event.clientX - rect.left;
        let y = event.clientY - rect.top;
        let candidate = null;
        let dist = 10000;

        /* Is an existing control point selected? */
        for (let p of curvePoints) {
            let cp = toCanvasCoord(p)
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
            selectedPoint = candidate;
            return;
        }

        candidate = null;
        thisDist = 10000;
        for (let p of linePoints) {
            let cp = toCanvasCoord(p)
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
            if (insertControlPoint(candidate)) {
                renderCurve();
                selectedPoint = candidate;
            }
        }
    });

    crvCanvas.addEventListener('dblclick', function(event) {
        let rect = crvCanvas.getBoundingClientRect();
        let x = event.clientX - rect.left;
        let y = event.clientY - rect.top;
        let candidate = null;
        let dist = 10000;

        /* Is an existing control point selected? */
        for (let p of curvePoints) {
            let cp = toCanvasCoord(p)
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
            removeControlPoint(candidate);
            renderCurve();
        }
    });

    crvCanvas.addEventListener('mouseup', function(event) {
        selectedPoint = null;
    });

    crvCanvas.addEventListener('mousemove', function(event) {
        if (selectedPoint != null){
            let rect = crvCanvas.getBoundingClientRect();
            let x = event.clientX - rect.left;
            let y = event.clientY - rect.top;
            let np = to01Coord({'x': x, 'y': y});

            moveControlPoint(selectedPoint, np);
            renderCurve();
        }
    });

    renderCurve();
});
