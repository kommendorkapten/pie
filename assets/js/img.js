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
var sliderTimeout = 200;
var exif = null;
var fullsizeProxy = null;

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
    var proxyClient = new XMLHttpRequest();
    var img = getParameterByName('img');
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
                var settings = JSON.parse(devpClient.responseText);

                // Settings are scaled with devSetScale
                settings.colort = Math.round(settings.colort / devSetScale);
                settings.tint = Math.round(settings.tint / devSetScale);
                settings.expos = Math.round(settings.expos / devSetScale);
                settings.contr = Math.round(settings.contr / devSetScale);
                settings.highl = Math.round(settings.highl / devSetScale);
                settings.shado = Math.round(settings.shado / devSetScale);
                settings.white = Math.round(settings.white / devSetScale);
                settings.black = Math.round(settings.black / devSetScale);
                settings.clarity.amount = Math.round(settings.clarity.amount / devSetScale);
                settings.clarity.rad    = Math.round(settings.clarity.rad    / devSetScale);
                settings.clarity.thresh = Math.round(settings.clarity.thresh / devSetScale);
                settings.vibra = Math.round(settings.vibra / devSetScale);
                settings.satur = Math.round(settings.satur / devSetScale);
                settings.rot = Math.round(settings.rot / devSetScale);
                settings.sharp.amount = Math.round(settings.sharp.amount / devSetScale);
                settings.sharp.rad    = Math.round(settings.sharp.rad    / devSetScale);
                settings.sharp.thresh = Math.round(settings.sharp.thresh / devSetScale);

                updateDevParams(settings);
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

function setViewport(ws, x0, y0, x1, y1, w, h) {
    var cmd = "VIEWP " + x0 + " " + y0 + " " + x1 + " " + y1 + " " + w + " " + h;

    ws.pieStartTs = Date.now();
    ws.send(cmd);
}

function pieInitEdit() {
    var w = window.innerWidth;
    var c = document.getElementById("img_canvas");
    var h;

    w = w - 282 - 50; // edit pane and margin

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
        }
    });

    c.addEventListener('mouseup', function(event) {
        if (zoomMode.enabled) {
            zoomMode.drag = false;
        }
    });

    c.addEventListener('mousemove', function(event) {
        if (zoomMode.enabled && zoomMode.drag){
            zoomMode.dx += (event.pageX - zoomMode.startX);
            zoomMode.dy += (event.pageY - zoomMode.startY);
            renderImage(fullsizeProxy);
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
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
    var rotate = false;

    console.log("Image dimension: " + bm.width + "x" + bm.height);
    console.log("Ctx dimension: " + c.width + "x" + c.height);

    /* Rotate image if:
     * 1) we are in zoom and dragging (ie usng proxy image)
     * 2) we are looking at a downsampled raw image */
    if (zoomMode.enabled && zoomMode.dragging) {
        rotate = true;
    } else if (!zoomMode.enabled) {
        rotate = true;
    }

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
        console.log("zm dx: " + offsetX);

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

    console.log("offset x: " + offsetX + " y: " + offsetY);
    console.log("Scale: " + scale);
    /* reset transformation */
    ctx.setTransform(1, 0, 0, 1, 0, 0);

    if (rotate) {
        switch(exif.orientation) {
        case 1: /* 0 */
            console.log("rotate 0");
            ctx.transform(scale, 0,
                          0, scale,
                          offsetX, offsetY);
            break;
        case 3: /* 180 */
            console.log("rotate 3");
            ctx.transform(-scale, 0,
                          0, -scale,
                          offsetX + scale * bm.width, offsetY + scale * bm.height);
            break;
        case 6: /* 270 */
            console.log("rotate 6");
            ctx.transform(0, scale,
                          -scale, 0,
                          offsetX + scale * bm.height, offsetY);
            break;
        case 8: /* 90 */
            console.log("rotate 8");
            ctx.transform(0, -scale,
                          scale, 0,
                          offsetX, offsetY + scale * bm.width);
            break;
        }
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
        console.log("draw imgae");
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
    /* Sync in a hackish way */
    var wsSync = 2;

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
        ws = null;
    }

    wsCmd.onerror = function(evt) {
        console.log("ERROR: " + evt.data);
    }

    wsCmd.onmessage = function(evt) {
        console.log("ERROR: " + evt);
    }

    wsImg.onmessage = function(evt) {
        var now = Date.now();
        var dur = now - wsCmd.pieStartTs;

        if (typeof evt.data === "string") {
            conslog.log("ERROR: Got data: " + evt.data);
        } else {
            var le = true;
            var w;
            var h;
            var msgType;
            var server_dur;
            var tx;

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
            var mbs = evt.data.byteLength / (1024.0 * 1024.0);
            console.log("Estimaged tx: " + tx + "ms for " + mbs + "MiB");
            mbs = mbs * 1000.0 / tx;
            console.log("Estimaged bw " + mbs + "MiB/s");
            console.log("Type: " + msgType + " width: " + w + " height:" + h);

            var pixels = new Uint8ClampedArray(evt.data, 16);
            var bm = new ImageData(pixels, w, h);
            dur = Date.now() - now;
            console.log("Unpacked image in " + dur + "ms");

            renderImage(bm);
            /* load fullsize jpeg */
            var img = getParameterByName('img');
            var imgHolder = new Image();
            var url = PROTO + "//" + COLLD_HOST + ":" + COLLD_PORT;

            url += "/proxy/" + img + ".jpg";
            imgHolder.onload = function() {
                /* Save for later use */
                fullsizeProxy = this;
            }
            imgHolder.setAttribute('src', url);
        }
    }

    /*
     * I N P U T   S L I D E R S
     */
    document.getElementById("sl_colortemp").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_colortemp").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("COLORT " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_tint").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_tint").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("TINT " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_exposure").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_exposure").value=targ.value / 10.0;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("EXPOS " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_contrast").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_contrast").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("CONTR " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_highlights").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_highlights").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("HIGHL " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_shadows").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_shadows").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("SHADO " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_white").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_white").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("WHITE " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_black").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_black").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("BLACK " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_clarity").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_clarity").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("CLARI " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_vibrance").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_vibrance").value = targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("VIBRA " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_saturation").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_saturation").value = targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("SATUR " + targ.value);
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_sharp_a").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_sharp_a").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            var a = document.getElementById("sl_sharp_a").value;
            var r = document.getElementById("sl_sharp_r").value;
            var t = document.getElementById("sl_sharp_t").value;
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_sharp_r").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_sharp_r").value=targ.value / 10.0;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            var a = document.getElementById("sl_sharp_a").value;
            var r = document.getElementById("sl_sharp_r").value;
            var t = document.getElementById("sl_sharp_t").value;
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
        }, sliderTimeout);

        return true;
    };

    document.getElementById("sl_sharp_t").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        document.getElementById("in_sharp_t").value=targ.value;

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            var a = document.getElementById("sl_sharp_a").value;
            var r = document.getElementById("sl_sharp_r").value;
            var t = document.getElementById("sl_sharp_t").value;
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
        }, sliderTimeout);

        return true;
    };

    /*
     * I N P U T   V A L I D A T O R S
     */
    document.getElementById("in_colortemp").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_tint").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_exposure").onkeydown = function(evt) {
        /* Only allow 0-9 - */
        /* ASCII 0-9 is 48 to 57 */
        /* - 189 or 173*/
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

    document.getElementById("in_contrast").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_highlights").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_shadows").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_white").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_black").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_clarity").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_vibrance").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_saturation").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_sharp_a").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_sharp_r").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    document.getElementById("in_sharp_t").onkeydown = function(evt) {
        var valid = [8, 13, 37, 39, 46, 173, 189, 190];

        if (valid.indexOf(evt.keyCode) !== -1) {
            return;
        }

        if (evt.keyCode > 47 && evt.keyCode < 58) {
            return;
        }

        evt.preventDefault();
    };

    /*
     * M A P   I N P U T   T O   S L I D E R S
     */
    document.getElementById("in_colortemp").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 30) {
            targ.value = 30;
        } else if (targ.value < -30) {
            targ.value = -30;
        }

        document.getElementById("sl_colortemp").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("COLORT " + targ.value);
    };

    document.getElementById("in_tint").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 30) {
            targ.value = 30;
        } else if (targ.value < -30) {
            targ.value = -30;
        }

        document.getElementById("sl_tint").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("TINT " + targ.value);
    };

    document.getElementById("in_exposure").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 5) {
            targ.value = 5;
        } else if (targ.value < -5) {
            targ.value = -5;
        }

        document.getElementById("sl_exposure").value=targ.value * 10;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("EXPOS " + Math.ceil(targ.value * 10));
    };

    document.getElementById("in_contrast").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_contrast").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("CONTR " + targ.value);
    };

    document.getElementById("in_highlights").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_highlights").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("HIGHL " + targ.value);
    };

    document.getElementById("in_shadows").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_shadows").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("SHADO " + targ.value);
    };

    document.getElementById("in_white").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_white").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("WHITE " + targ.value);
    };

    document.getElementById("in_black").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_black").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("BLACK " + targ.value);
    };

    document.getElementById("in_clarity").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_clarity").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("CLARI " + targ.value);
    };

    document.getElementById("in_vibrance").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_vibrance").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("VIBRA " + targ.value);
    };

    document.getElementById("in_saturation").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 100) {
            targ.value = 100;
        } else if (targ.value < -100) {
            targ.value = -100;
        }

        document.getElementById("sl_saturation").value=targ.value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("SATUR " + targ.value);
    };

    document.getElementById("in_sharp_a").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 300) {
            targ.value = 300;
        } else if (targ.value < 0) {
            targ.value = -0;
        }

        document.getElementById("sl_sharp_a").value=targ.value;
        var a = document.getElementById("sl_sharp_a").value;
        var r = document.getElementById("sl_sharp_r").value;
        var t = document.getElementById("sl_sharp_t").value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
    };

    document.getElementById("in_sharp_r").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 10) {
            targ.value = 10;
        } else if (targ.value < 0.1) {
            targ.value = 0.1;
        }

        document.getElementById("sl_sharp_r").value=targ.value * 10;
        var a = document.getElementById("sl_sharp_a").value;
        var r = document.getElementById("sl_sharp_r").value;
        var t = document.getElementById("sl_sharp_t").value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
    };

    document.getElementById("in_sharp_t").onchange = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            targ = evt.srcElement;
        }

        if (isNaN(targ.value)) {
            targ.value = 0;
        } else if (targ.value > 20) {
            targ.value = 20;
        } else if (targ.value < 0) {
            targ.value = 0;
        }

        document.getElementById("sl_sharp_t").value=targ.value;
        var a = document.getElementById("sl_sharp_a").value;
        var r = document.getElementById("sl_sharp_r").value;
        var t = document.getElementById("sl_sharp_t").value;
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("SHARP " + a + " " + r + " " + t + " ");
    };

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
});


document.onkeydown = function(evt) {
    evt = evt || window.event;

    console.log("Captured key code: " + evt.keyCode);
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
        var col = getParameterByName('col');
        var newUrl = PROTO + "//" + COLLD_HOST + ":" + COLLD_PORT + "/";

        if (col != null && col != "") {
            newUrl += "?col=" + col;
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

            setViewport(wsCmd,
                        zoomMode.dx,
                        zoomMode.dy,
                        zoomMode.dx + c.width,
                        zoomMode.dy + c.height,
                        c.width,
                        c.height);
        }
        break;
    }

};
