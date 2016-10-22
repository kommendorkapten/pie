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
    var wsCmd;
    var wsHist;
    var wsImg;

    wsCmd = new WebSocket(get_appropriate_ws_url(),"pie-cmd");
    wsCmd.binaryType = "arraybuffer";
    wsHist = new WebSocket(get_appropriate_ws_url(),"pie-hist");
    wsHist.binaryType = "arraybuffer";
    wsImg = new WebSocket(get_appropriate_ws_url(),"pie-img");
    wsImg.binaryType = "arraybuffer";

    wsCmd.onopen = function(evt) {
        console.log("Opening websocket...");
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
        var now = Date.now()
        var dur = now - wsCmd.pieStartTs;

        /* Keep start point to measure draw time */
        wsCmd.pieStartTs = now;

        console.log("RT in " + dur + "ms");
        if (typeof evt.data === "string") {
            conslog.log("ERROR: Got data: " + evt.data);
        } else {
            var pixels = new Uint8ClampedArray(evt.data);
            var bm = new ImageData(pixels, 1024, 100);

            /* Update canvas */
            var c = document.getElementById("img_canvas");
            var ctx = c.getContext("2d");
            ctx.putImageData(bm, 0, 0);
            now = Date.now();
            dur = now - wsCmd.pieStartTs;
            wsCmd.pieStartTs = now;
            console.log("Draw image in " + dur + "ms");
        }
    }

    document.getElementById("get").onclick = function(evt) {
        if (!wsCmd) {
            return false;
        }

        wsCmd.pieStartTs = Date.now();
        wsCmd.send("LOAD");

        return false;
    };

    document.getElementById("range1").oninput = function(evt) {
        var targ;

        if (!wsCmd) {
            return false;
        }

        if (evt.target) {
            targ = evt.target;
        } else {
            tart = evt.srcElement;
        }

        if (targ.wsCall) {
            clearTimeout(targ.wsCall);
        }
        targ.wsCall = setTimeout(function(){
            wsCmd.pieStartTs = Date.now();
            wsCmd.send("CONTR " + targ.value);
        }, 100);


        return true;
    };
});
