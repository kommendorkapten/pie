function calculateHistogram(canvas) {
    var pl = [];
    var pr = [];
    var pg = [];
    var pb = [];
    var histLum = [];
    var histRed = [];
    var histGreen = [];
    var histBlue = [];
    var histSum = 0;
    var imgData = null;
    var pixels = null;
    var max = 0;

    imgData = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height);
    pixels = imgData.data;

    /* Reset histograms */
    for (i = 0; i < 256; i++) {
        histLum[i] = 0;
        histRed[i] = 0;
        histGreen[i] = 0;
        histBlue[i] = 0;
    }

    for(i = 0; i < pixels.length; i += 4) {
        var red = pixels[i];
        var green = pixels[i+1];
        var blue = pixels[i+2];
        /* Omit alpha */

        var lum = red * 0.2126 + green * 0.7152 + blue * 0.0722;

        histLum[Math.floor(lum)] += 1;
        histRed[red] += 1;
        histGreen[green] += 1;
        histBlue[blue] += 1;
    }

    /* Normalize */
    for (c of histLum) {
        histSum += c;
        if (c > max) {
            max = c;
        }
    }
    for (c of histRed) {
        histSum += c;
        if (c > max) {
            max = c;
        }
    }
    for (c of histGreen) {
        histSum += c;
        if (c > max) {
            max = c;
        }
    }
    for (c of histBlue) {
        histSum += c;
        if (c > max) {
            max = c;
        }
    }
    var histMu = histSum / 1024;
    var histLimit = histMu * 13.0;
    if (histLimit < max) {
        max = histLimit;
    }

    for (i = 0; i < histLum.length; i++) {
        var nl = histLum[i] < histLimit ? histLum[i]: histLimit;
        var nr = histRed[i] < histLimit ? histRed[i]: histLimit;
        var ng = histGreen[i] < histLimit ? histGreen[i]: histLimit;
        var nb = histBlue[i] < histLimit ? histBlue[i]: histLimit;

        nl = (nl / histLimit) * histYMax;
        nr = (nr / histLimit) * histYMax;
        ng = (nb / histLimit) * histYMax;
        nb = (ng / histLimit) * histYMax;

        pl[i] = {
            x: i,
            y: nl
        };
        pr[i] = {
            x: i,
            y: nr
        };
        pg[i] = {
            x: i,
            y: ng
        };
        pb[i] = {
            x: i,
            y: nb
        };
    }

    return {
        "pl": pl,
        "pr": pr,
        "pg": pg,
        "pb": pb
    }
}
