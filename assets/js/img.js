var lowerPaneHeight = 220;

var pieStateHack = {
    "image": "lena.png"
};

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

    c.width = w;
    c.height = h;
    
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
    var wsCmd;
    var wsHist;
    var wsImg;

    pieInitEdit();

    wsCmd = new WebSocket(getWsUrl(), "pie-cmd");
    wsCmd.binaryType = "arraybuffer";
    wsHist = new WebSocket(getWsUrl(), "pie-hist");
    wsHist.binaryType = "arraybuffer";
    wsImg = new WebSocket(getWsUrl(), "pie-img");
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
            var bm = new ImageData(pixels, 640, 100);

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

    document.getElementById("load").onclick = function(evt) {
        if (!wsCmd) {
            return false;
        }

        var c = document.getElementById("img_canvas");
        wsCmd.pieStartTs = Date.now();
        wsCmd.send("LOAD " + pieStateHack.image + " " + c.width + " " + c.height);

        return false;
    };

    /*
     * I N P U T   S L I D E R S
     */
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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

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
        }, 50);

        return true;
    };

    /*
     * I N P U T   V A L I D A T O R S
     */
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

    /*
     * M A P   I N P U T   T O   S L I D E R S 
     */
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
        wsCmd.send("EXPOS " + targ.value);
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
        wsCmd.send("SATUR " + targ.value);
    };
});

/*
  window.innerWidth // without toolbars etc
  window.innerHeight
  window.outerWidth // with toolbars etc
  window.outerHeight
*/

