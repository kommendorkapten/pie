function get_appropriate_ws_url()
{
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

window.addEventListener("load", function(evt) {
    var ws;

    document.getElementById("open").onclick = function(evt) {
        if (ws) {
            return false;
        }
        ws = new WebSocket(get_appropriate_ws_url(),"pie-cmd");
        ws.binaryType = "arraybuffer";

        ws.onopen = function(evt) {
            console.log("Opening websocket...");
        }

        ws.onclose = function(evt) {
            console.log("Closing websocket...");
            ws = null;
        }

        ws.onmessage = function(evt) {
            var now = Date.now()
            var dur = now - ws.pieStartTs;
            ws.pieStartTs = now;
            /*console.log(evt);*/
            console.log("RT in " + dur + "ms");
            if (typeof evt.data === "string") {
                conslog.log("Got data: " + evt.data);
            } else {
                var pixels = new Uint8ClampedArray(evt.data);
                console.log("br: " + pixels.length);
                var bm = new ImageData(pixels, 1024, 100);

                console.log("r: " + bm.data[4*1023]);
                console.log("g: " + bm.data[4*1023 + 1]);
                console.log("b: " + bm.data[4*1023 + 2]);
                console.log("a: " + bm.data[4*1023 + 3]);

                /* Update canvas */
                var c = document.getElementById("img_canvas");
                var ctx = c.getContext("2d");
                ctx.putImageData(bm, 0, 0);
                now = Date.now();
                dur = now - ws.pieStartTs;
                ws.pieStartTs = now;
                console.log("Draw image in " + dur + "ms");                
            }
        }

        ws.onerror = function(evt) {
            console.log("ERROR: " + evt.data);
        }
        return false;
    };

    document.getElementById("get").onclick = function(evt) {
        if (!ws) {
            return false;
        }

        ws.pieStartTs = Date.now();
        ws.send("Give me data yo");

        return false;
    };

    document.getElementById("close").onclick = function(evt) {
        if (!ws) {
            return false;
        }
        ws.close();
        ws = null;
        return false;
    };
/*
    document.getElementById("range1").oninput = function(evt) {
        if (!ws) {
            return false;
        }

        var targ;
        if (evt.target) {
            targ = evt.target;
        } else {
            tart = evt.srcElement;
        }

        ws.send(targ.value);

        return true;
    };
*/
    document.getElementById("range1").onchange = function(evt) {
        if (!ws) {
            return false;
        }
        console.log("on change done");
        ws.pieStartTs = Date.now();
        var targ;
        if (evt.target) {
            targ = evt.target;
        } else {
            tart = evt.srcElement;
        }

        ws.send(targ.value);

        return true;
    };

});
