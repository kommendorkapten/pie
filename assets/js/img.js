'use strict';

var COLLD_PORT = 8081;
var EDITD_PORT = 8080;
var COLLD_HOST = null;
var EDITD_HOST = null;
var PROTO = null;
var wsCmd = null;
var wsImg = null;
var histChart;
var lowerPaneHeight = 220;
var histYMax = 255;
var bigEndian = 1;
var devSetScale = 10000.0;
var zoomMode = {
    "enabled": false,
    "mode": "center",
    "drag": false,
    "startX": 0,
    "startY": 0,
    "dx": 0,
    "dy": 0,
    "moved": false,
};
var histDataL = {
    borderColor: "rgba(200, 200, 200, 0.5)",
    backgroundColor: "rgba(200, 200, 200, 0.7)",
    pointRadius: 0,
    lineTension: 0,
    data: [{
        x: 0,
        y: 0
    }]
};
var histDataR = {
    borderColor: "rgba(255, 0, 0, 0.5)",
    backgroundColor: "rgba(255, 0, 0, 0.4)",
    pointRadius: 0,
    lineTension: 0,
    data: [{
        x: 0,
        y: 0
    }]
};
var histDataG = {
    borderColor: "rgba(0, 255, 0, 0.5)",
    backgroundColor: "rgba(0, 255, 0, 0.4)",
    pointRadius: 0,
    lineTension: 0,
    data: [{
        x: 0,
        y: 0
    }]
};
var histDataB = {
    borderColor: "rgba(0, 0, 255, 0.5)",
    backgroundColor: "rgba(0, 0, 255, 0.4)",
    pointRadius: 0,
    lineTension: 0,
    data: [{
        x: 0,
        y: 0
    }]
};
let sliderTimeout = 200;
let exif = null;
let fullsizeProxy = null;
/* the bitmap received from the server */
let bm = null;
let curves = null;
let curveControlPoints = [defaultControlPoints(),
                          defaultControlPoints(),
                          defaultControlPoints(),
                          defaultControlPoints()];
let curveColors = ["#a9a9a9", "#e05b5d", "#57c160", "#5d88de"];
let CURVE_LUM = 0;
let CURVE_RED = 1;
let CURVE_GREEN = 2;
let CURVE_BLUE = 3;
let devParams = null;

function ISController(input, slider, options, callback = null) {

    slider.oninput = function(evt) {
        let targ = null;

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        input.value = targ.value / options.scale;

        if (targ.iscTimer) {
            clearTimeout(targ.iscTimer);
        }
        targ.iscTimer = setTimeout(function() {
            callback(targ.value);
        }, 200);

        return true;
    };

    input.onchange = function(evt) {
        let targ = null;


        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > options.max) {
            targ.value = options.max;
        } else if (targ.value < options.min) {
            targ.value = options.min;
        }

        let realVal = targ.value * options.scale;
        slider.value = realVal;
        callback(realVal);
    };

    input.onkeydown = function(evt) {
        /* Only allow 0-9 - */
        /* ASCII 0-9 is 48 to 57 */
        /* - 189 or 173 */
        /* . 190 */
        /* arrows left right 37 39 */
        /* backspace (8) delete (46) and enter (13)*/
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };
}

function ControlProxy(wsImg, wsCmd) {
    /* Sync in a hackish way */
    let wsSync = 2;

    wsCmd.onopen = function(evt) {
        console.log("Opening command websocket..." + wsSync);
        wsSync = wsSync - 1;

        if (wsSync == 0) {
            loadImage(wsCmd);
        }
    }

    wsImg.onopen = function(evt) {
        console.log("Opening image websocket..." + wsSync);
        wsSync = wsSync - 1;

        if (wsSync == 0) {
            loadImage(wsCmd);
        }
    }

    wsCmd.onclose = function(evt) {
        console.log("Closing websocket...");
        wsCmd = null;
    }

    wsCmd.onerror = function(evt) {
        console.log("ERROR: " + evt.data);
    }

    wsCmd.onmessage = function(evt) {
        console.log("ERROR: " + evt);
    }

    wsImg.onmessage = function(evt) {
        let now = Date.now();
        let dur = now - wsCmd.pieStartTs;

        if (typeof evt.data === "string") {
            conslog.log("ERROR: Got data: " + evt.data);
            return;
        }

        let le = true;
        let w;
        let h;
        let msgType;
        let server_dur;
        let tx;

        /* Data arrives as :
           0:3 duration in milli seconds
           4:7 type of message
           8:11 width (uint32, network order)
           12:15 height (uint32, network order)
           16:  rgba */

        /* width and height are in network order */
        server_dur = new DataView(evt.data, 0, 4).getUint32(0, false);
        msgType = new DataView(evt.data, 4, 4).getUint32(0, false);
        w = new DataView(evt.data, 8, 4).getUint32(0, false);
        h = new DataView(evt.data, 12, 4).getUint32(0, false);

        console.log("Server duration: " + server_dur + "ms");
        console.log("RT in " + dur + "ms");
        tx = dur - server_dur;
        let mbs = evt.data.byteLength / (1024.0 * 1024.0);
        console.log("Estimaged tx: " + tx + "ms for " + mbs + "MiB");
        mbs = mbs * 1000.0 / tx;
        console.log("Estimaged bw " + mbs + "MiB/s");
        console.log("Type: " + msgType + " width: " + w + " height:" + h);

        let pixels = new Uint8ClampedArray(evt.data, 16);
        bm = new ImageData(pixels, w, h);
        dur = Date.now() - now;
        console.log("Unpacked image in " + dur + "ms");

        renderImage(bm);
        /* load fullsize jpeg */
        let img = getParameterByName("mob");
        let imgHolder = new Image();
        let url = PROTO + "//" + COLLD_HOST + ":" + COLLD_PORT;

        url += "/proxy/" + img + ".jpg";
        console.log("Get proxy " + url);
        imgHolder.onload = function() {
            /* Save for later use */
            fullsizeProxy = this;
            updateImgNavigation(document.getElementById("img_nav_canvas"),
                                fullsizeProxy,
                                exif,
                                document.getElementById("img_canvas"));
        }
        imgHolder.setAttribute('src', url);
    };

    this.send = function (cmd) {
        wsCmd.pieStartTs = Date.now();
        wsCmd.send(cmd);
    };
}

function getWsUrl(){
    var pcol;
    var u = document.URL;

    /*
     * We open the websocket encrypted if this page came on an
     * https:// url itself, otherwise unencrypted
     */

    if (u.substring(0, 5) == "https") {
	pcol = "wss://";
	u = u.substr(8);
    } else {
	pcol = "ws://";
	if (u.substring(0, 4) == "http")
	    u = u.substr(7);
    }

    u = u.split('/');

    /* + "/xxx" bit is for IE10 workaround */

    return pcol + u[0] + "/xxx";
}

function getParameterByName(name, url) {
    if (!url) {
      url = window.location.href;
    }
    name = name.replace(/[\[\]]/g, "\\$&");
    var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)"),
        results = regex.exec(url);
    if (!results) return null;
    if (!results[2]) return '';
    return decodeURIComponent(results[2].replace(/\+/g, " "));
}

function loadImage(ws) {
    var colldUrl = PROTO + "//" + COLLD_HOST + ":" + COLLD_PORT;
    var devpClient = new XMLHttpRequest();
    var exifClient = new XMLHttpRequest();
    var img = getParameterByName("mob");
    var c = document.getElementById("img_canvas");

    if (!img) {
        return false;
    }

    ws.pieStartTs = Date.now();
    ws.send("LOAD " + img + " " + c.width + " " + c.height);

    /* Load development settings */
    devpClient.onreadystatechange = function() {
        if (devpClient.readyState == XMLHttpRequest.DONE) {
            if (devpClient.status == 200) {
                devParams = JSON.parse(devpClient.responseText);

                // Settings are scaled with devSetScale
                devParams.colort = Math.round(devParams.colort / devSetScale);
                devParams.tint = Math.round(devParams.tint / devSetScale);
                devParams.expos = Math.round(devParams.expos / devSetScale);
                devParams.contr = Math.round(devParams.contr / devSetScale);
                devParams.highl = Math.round(devParams.highl / devSetScale);
                devParams.shado = Math.round(devParams.shado / devSetScale);
                devParams.white = Math.round(devParams.white / devSetScale);
                devParams.black = Math.round(devParams.black / devSetScale);
                devParams.clarity.amount = Math.round(devParams.clarity.amount / devSetScale);
                devParams.clarity.rad    = Math.round(devParams.clarity.rad    / devSetScale);
                devParams.clarity.thresh = Math.round(devParams.clarity.thresh / devSetScale);
                devParams.vibra = Math.round(devParams.vibra / devSetScale);
                devParams.satur = Math.round(devParams.satur / devSetScale);
                devParams.rot = Math.round(devParams.rot / devSetScale);
                devParams.sharp.amount = Math.round(devParams.sharp.amount / devSetScale);
                devParams.sharp.rad    = Math.round(devParams.sharp.rad    / devSetScale);
                devParams.sharp.thresh = Math.round(devParams.sharp.thresh / devSetScale);

                updateDevParams(devParams);
            }
        }
    };

    exifClient.onreadystatechange = function() {
        if (exifClient.readyState == XMLHttpRequest.DONE) {
            if (exifClient.status == 200) {
                var table = document.getElementById("meta_data_tbl");

                exif = JSON.parse(exifClient.responseText);

                table.rows[0].cells[0].innerHTML = "ISO " + exif.iso;
                table.rows[0].cells[1].innerHTML = exif.focal_len + " mm";
                table.rows[0].cells[2].innerHTML = "f/" + (exif.fnumber / 10);
                table.rows[0].cells[3].innerHTML = exif.exposure_time + " sec";
            }
        }
    };

    devpClient.open("GET", colldUrl + "/devparams/" + img);
    devpClient.send();
    exifClient.open("GET", colldUrl + "/exif/" + img);
    exifClient.send();

    return true;
}

function changeChannelCurve() {
    var channelSelect = document.getElementById("channel-curve-select");
    var channel = parseInt(channelSelect.options[channelSelect.selectedIndex].value);

    curveControlPoints[curves.selectedChannel] = curves.controlPoints;
    curves.controlPoints = curveControlPoints[channel];
    curves.selectedChannel = channel;
    curves.color = curveColors[channel];
    curves.render();
}

function persistCurve() {
    if (curves.timeO) {
        clearTimeout(curves.timeO);
    }
    curves.timeO = setTimeout(function(){
        console.log(curves.controlPoints);
    }, sliderTimeout);
}

function setViewport(ws, x0, y0, x1, y1, w, h) {
    var cmd = "VIEWP " + x0 + " " + y0 + " " + x1 + " " + y1 + " " + w + " " + h;

    ws.pieStartTs = Date.now();
    ws.send(cmd);
}

function pieInitEdit() {
    var w = window.innerWidth;
    var c = document.getElementById("img_canvas");
    var h;

    w = w - 2 * 282 - 50; // edit pane and margin

    if (w < 640) {
        w = 640;
    }
    h = Math.ceil((w / 3) * 2);
    if ((window.outerHeight - h) < lowerPaneHeight) {
        h = window.outerHeight - lowerPaneHeight;
        w = (h * 3) / 2;

        if (w < 640) {
            w = 640;
            h = Math.ceil((w / 3) * 2);
        }
    }

    console.log("Set img_canvas to " + w + "x" + h);
    c.width = w;
    c.height = h;

    /* Install drag and drop for canvas */
    c.addEventListener('mousedown', function(event) {
        if (zoomMode.enabled) {
            zoomMode.drag = true;
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
            zoomMode.moved = false;
            renderImage(fullsizeProxy);
        }
    });

    c.addEventListener('mouseup', function(event) {
        if (zoomMode.enabled) {
            zoomMode.drag = false;

            if (zoomMode.moved) {
                /* dx and dy are the coordinate transformations.
                   they must be inverted to get the correct coordinaes
                   in the source image. */
                setViewport(wsCmd,
                            -zoomMode.dx,
                            -zoomMode.dy,
                            -(zoomMode.dx - c.width),
                            -(zoomMode.dy - c.height),
                            c.width,
                            c.height);
            } else {
                renderImage(bm);
            }

        }
    });

    c.addEventListener('mousemove', function(event) {
        if (zoomMode.enabled && zoomMode.drag){
            zoomMode.dx += (event.pageX - zoomMode.startX);
            zoomMode.dy += (event.pageY - zoomMode.startY);
            renderImage(fullsizeProxy);
            updateImgNavigation(document.getElementById("img_nav_canvas"),
                                fullsizeProxy,
                                exif,
                                document.getElementById("img_canvas"));
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
            zoomMode.moved = true;
        }
    });
}

function updateDevParams(params) {
    /*
      sl_colortemp   -30,  30
      sl_tint        -30,  30
      sl_exposure    -50,  50
      sl_contrast   -100, 100
      sl_highlights -100, 100
      sl_shadows    -100, 100
      sl_white      -100, 100
      sl_black      -100, 100
      sl_clarity    -100, 100
      sl_vibrance   -100, 100
      sl_saturation -100, 100
      sl_sharp_a       0, 300
      sl_sharp_r,      1, 100
      sl_sharp_t,      0,  20
     */
    document.getElementById("sl_colortemp").value = params.colort;
    document.getElementById("in_colortemp").value = params.colort;
    document.getElementById("sl_tint").value = params.tint;
    document.getElementById("in_tint").value = params.tint;
    document.getElementById("sl_exposure").value = params.expos;
    document.getElementById("in_exposure").value = params.expos / 10;
    document.getElementById("sl_contrast").value = params.contr;
    document.getElementById("in_contrast").value = params.contr;
    document.getElementById("sl_highlights").value = params.highl;
    document.getElementById("in_highlights").value = params.highl;
    document.getElementById("sl_shadows").value = params.shado;
    document.getElementById("in_shadows").value = params.shado;
    document.getElementById("sl_white").value = params.white;
    document.getElementById("in_white").value = params.white;
    document.getElementById("sl_black").value = params.black;
    document.getElementById("in_black").value = params.black;
    document.getElementById("sl_clarity").value = params.clarity.amount;
    document.getElementById("in_clarity").value = params.clarity.amount;
    document.getElementById("sl_vibrance").value = params.vibra;
    document.getElementById("in_vibrance").value = params.vibra;
    document.getElementById("sl_saturation").value = params.satur;
    document.getElementById("in_saturation").value = params.satur;
    document.getElementById("sl_sharp_a").value = params.sharp.amount;
    document.getElementById("in_sharp_a").value = params.sharp.amount;
    document.getElementById("sl_sharp_r").value = params.sharp.rad;
    document.getElementById("in_sharp_r").value = params.sharp.rad / 10;
    document.getElementById("sl_sharp_t").value = params.sharp.thresh;
    document.getElementById("in_sharp_t").value = params.sharp.thresh;
}

function renderImage(bm) {
    var c = document.getElementById("img_canvas");
    var offsetX = 0;
    var offsetY = 0;
    var ctx = c.getContext("2d");
    var now = Date.now();
    var dur;
    var scale = 1;
    var ratio = bm.width / bm.height;
    var w = bm.width;
    var h = bm.height;
    var clearRect = false;

    /* reset canvas */
    ctx.setTransform(1, 0, 0, 1, 0, 0);

    /* When calculating scaling, the presented orientation
       must be used. */
    if (exif.orientation == 6 ||
        exif.orientation == 8) {
        var tmp = w;
        w = h;
        h = tmp;
    }

    if (w > c.width) {
        scale = c.width / w;
    }

    if (h * scale > c.height) {
        scale = c.height / h;
    }

    if ((w < c.width) || (h < c.height)) {
        /* Require some change */
        if (c.width - w > 5 ||
            c.height - h > 5) {
            clearRect = true;
        }
    }

    if (zoomMode.enabled && zoomMode.drag) {
        scale = 1;
        var lim;

        lim = w - c.width;
        lim = -lim;

        if (zoomMode.dx > 0.0) {
            zoomMode.dx = 0.0;
        } else if (zoomMode.dx < lim) {
            zoomMode.dx = lim;
        }
        offsetX = zoomMode.dx;

        lim = h - c.height;
        lim = - lim;
        if (zoomMode.dy > 0.0) {
            zoomMode.dy = 0.0;
        } else if (zoomMode.dy < lim) {
            zoomMode.dy = lim;
        }
        offsetY = zoomMode.dy;
    } else {
        offsetX = (c.width - w * scale) / 2;
        offsetY = (c.height - h * scale) / 2;
    }

    /* The entier canvas is not going to be drawn. Clear it first */
    if (clearRect) {
        ctx.clearRect(0, 0, c.width, c.height);
    }

    switch(exif.orientation) {
    case 1: /* 0 */
        ctx.transform(scale, 0,
                      0, scale,
                      offsetX, offsetY);
        break;
    case 3: /* 180 */
        ctx.transform(-scale, 0,
                      0, -scale,
                      offsetX + scale * bm.width, offsetY + scale * bm.height);
        break;
    case 6: /* 270 */
        ctx.transform(0, scale,
                      -scale, 0,
                      offsetX + scale * bm.height, offsetY);
        break;
    case 8: /* 90 */
        ctx.transform(0, -scale,
                      scale, 0,
                      offsetX, offsetY + scale * bm.width);
        break;
    }

    if (zoomMode.enabled && zoomMode.drag) {
        ctx.drawImage(bm, 0, 0, bm.width, bm.height);
    } else {
        createImageBitmap(bm)
            .then(img => {
                ctx.drawImage(img, 0, 0, img.width, img.height);
                dur = Date.now() - now;
                console.log("Draw image in " + dur + "ms");

                now = Date.now();
                var hist = calculateHistogram(c);
                dur = Date.now() - now;
                console.log("Calculate histogram in " + dur + "ms");

                histDataL.data = hist.pl;
                histDataR.data = hist.pr;
                histDataG.data = hist.pg;
                histDataB.data = hist.pb;
                histChart.update();
            });
    }
}

window.onresize = function(evt) {
    var w = window.innerWidth;
    var c = document.getElementById("img_canvas");
    var h;

    w = w - 282 - 50; // edit pane and margin

    if (w < 640) {
        w = 640;
    }
    h = Math.ceil((w / 3) * 2);
    if ((window.outerHeight - h) < lowerPaneHeight) {
        return;
    }
    c.width = w;
    c.height = h;
}

window.addEventListener("load", function(evt) {
    let ctlProxy = null;

    pieInitEdit();


    /* Configure hosts */
    var url = window.location.href.split("/");
    PROTO = url[0];
    if (COLLD_HOST == null) {
        COLLD_HOST = url[2].split(":")[0];
    }
    if (EDITD_HOST == null) {
        EDITD_HOST = url[2].split(":")[0];
    }

    /* Determine endianess */
    var b = new ArrayBuffer(4);
    var a = new Uint32Array(b);
    var c = new Uint8Array(b);
    a[0] = 0xcafebabe;
    if (c[0] == 0xbe) {
        bigEndian = 0;
    }

    wsCmd = new WebSocket(getWsUrl(), "pie-cmd");
    wsCmd.binaryType = "arraybuffer";
    wsImg = new WebSocket(getWsUrl(), "pie-img");
    wsImg.binaryType = "arraybuffer";

    ctlProxy = new ControlProxy(wsImg, wsCmd);

    /*
     * I N P U T   S L I D E R S
     */
    new ISController(document.getElementById("in_colortemp"),
                     document.getElementById("sl_colortemp"),
                     {'max': 30, 'min': -30, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("COLORT " + val);
                         devParams.colort = val;
                     });
    new ISController(document.getElementById("in_tint"),
                     document.getElementById("sl_tint"),
                     {'max': 30, 'min': -30, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("TINT " + val);
                         devParams.tint = val;
                     });
    new ISController(document.getElementById("in_exposure"),
                     document.getElementById("sl_exposure"),
                     {'max': 50, 'min': -50, 'scale': 10,},
                     function (val) {
                         ctlProxy.send("EXPOS " + val);
                         devParams.expos = val;
                     });
    new ISController(document.getElementById("in_contrast"),
                     document.getElementById("sl_contrast"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("CONTR " + val);
                         devParams.contr = val;
                     });
    new ISController(document.getElementById("in_highlights"),
                     document.getElementById("sl_highlights"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("HIGHL " + val);
                         devParams.highl = val;
                     });
    new ISController(document.getElementById("in_shadows"),
                     document.getElementById("sl_shadows"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("SHADO " + val);
                         devParams.shado = val;
                     });
    new ISController(document.getElementById("in_white"),
                     document.getElementById("sl_white"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("WHITE " + val);
                         devParams.white = val;
                     });
    new ISController(document.getElementById("in_black"),
                     document.getElementById("sl_black"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("BLACK " + val);
                         devParams.black = val;
                     });
    new ISController(document.getElementById("in_clarity"),
                     document.getElementById("sl_clarity"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("CLARI " + val);
                         devParams.clarity.amount = val;
                     });
    new ISController(document.getElementById("in_vibrance"),
                     document.getElementById("sl_vibrance"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("VIBRA " + val);
                         devParams.vibra = val;
                     });
    new ISController(document.getElementById("in_saturation"),
                     document.getElementById("sl_saturation"),
                     {'max': 100, 'min': -100, 'scale': 1,},
                     function (val) {
                         ctlProxy.send("SATUR " + val);
                         devParams.satur = val;
                     });
    new ISController(document.getElementById("in_sharp_a"),
                     document.getElementById("sl_sharp_a"),
                     {'max': 300, 'min': 0, 'scale': 1,},
                     function (val) {
                         devParams.sharp.amount = val;

                         let cmd = "SHARP " + devParams.sharp.amount + " " +
                             devParams.sharp.rad + " " +
                             devParams.sharp.thresh;

                         ctlProxy.send(cmd);
                     });
    new ISController(document.getElementById("in_sharp_r"),
                     document.getElementById("sl_sharp_r"),
                     {'max': 10, 'min': 0.1, 'scale': 10.0,},
                     function (val) {
                         devParams.sharp.rad = val;

                         let cmd = "SHARP " + devParams.sharp.amount + " " +
                             devParams.sharp.rad + " " +
                             devParams.sharp.thresh;
                         ctlProxy.send(cmd);
                     });
    new ISController(document.getElementById("in_sharp_t"),
                     document.getElementById("sl_sharp_t"),
                     {'max': 20, 'min': 0, 'scale': 1,},
                     function (val) {
                         devParams.sharp.thresh = val;

                         let cmd = "SHARP " + devParams.sharp.amount + " " +
                             devParams.sharp.rad + " " +
                             devParams.sharp.thresh;
                         ctlProxy.send(cmd);
                     });

    var histCanvas = document.getElementById("hist_canvas").getContext("2d");
    histChart = new Chart(histCanvas, {
        type: 'line',
        data: {
            datasets: [
                histDataL,
                histDataR,
                histDataG,
                histDataB
            ]
        },
        options: {
            legend: {
                display: false
            },
            responsive: false,
            scales: {
                xAxes: [{
                    type: 'linear',
                    position: 'bottom',
                    display: false,
                    ticks: {
                        min: 0,
                        max: 255
                    }
                }],
                yAxes: [{
                    type: 'linear',
                    display: false,
                    ticks: {
                        min: 0,
                        max: histYMax
                    }
                }]
            }
        }
    });

    curves = new Curve(document.getElementById("curve_canvas"),
                       curveColors[0],
                       persistCurve);
    curves.selectedChannel = 0;
});

document.onkeydown = function(evt) {
    evt = evt || window.event;

    switch (evt.keyCode) {
    case 27:
        console.log("esc");
        alert("escape clicked yo!");
        break;
        /* nav */
    case 37:
    case 38:
    case 39:
    case 40:
        console.log("navigate yo");
        break;
        /* rating */
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
        console.log("rate yo");
        break;
        /* color */
    case 54:
    case 55:
    case 56:
    case 57:
        console.log("color yo");
        break;
    case 71: /* g */
        var col = getParameterByName("col");
        var mob = getParameterByName("mob");
        var rate = getParameterByName("rate");
        var color = getParameterByName("color");
        var newUrl = PROTO + "//" + COLLD_HOST + ":" + COLLD_PORT + "/";

        /* MOB should always be provided */
        if (mob != null && mob != "") {
            newUrl += "?mob=" + mob;
        }
        if (col != null && col != "") {
            newUrl += "&col=" + col;
        }
        if (rate != null && rate != "") {
            newUrl += "&rate=" + rate;
        }
        if (color != null && color != "") {
            newUrl += "&color=" + color;
        }
        window.location.replace(newUrl);
        break;
    case 90:
        var c = document.getElementById("img_canvas");

        if (zoomMode.enabled) {
            zoomMode.enabled = false;

            setViewport(wsCmd,
                        0,
                        0,
                        -1,
                        -1,
                        c.width,
                        c.height);
        } else {
            zoomMode.enabled = true;

            /* in canvas origo is upper left
               in bitmap space origo is lower left */
            setViewport(wsCmd,
                        -zoomMode.dx,
                        -zoomMode.dy,
                        -(zoomMode.dx - c.width),
                        -(zoomMode.dy - c.height),
                        c.width,
                        c.height);
        }
        break;
    }

};
