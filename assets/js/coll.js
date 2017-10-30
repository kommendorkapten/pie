var COLLD_PORT = 8081;
var EDITD_PORT = 8080;
var COLLD_HOST = null;
var EDITD_HOST = null;
var PROTO = null;
var mobCache = {};
var exifCache = {};
var selectedMobId = "";
var selectedCollection = {};
var zoomMode = {
    "enabled": false,
    "mode": "center",
    "drag": false,
    "startX": 0,
    "startY": 0,
    "dx": 0,
    "dy": 0,
};
var globalFilterRate = 0;
var globalFilterColor = 0;
var NAV_LEFT = 37;
var NAV_UP = 38;
var NAV_RIGHT = 39;
var NAV_DOWN = 40;
var histChart;
var histYMax = 255;
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
var MOB_COLOR_NONE = 0;
var MOB_COLOR_RED = 1;
var MOB_COLOR_GREEN = 2;
var MOB_COLOR_BLUE = 3;
var MOB_COLOR_YELLOW = 4;

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

function rateFilenameFromMob(mob) {
    var filename = "img/rate_0.png";

    switch(mob.rating) {
    case 1:
        filename = "img/rate_1.png";
        break;
    case 2:
        filename = "img/rate_2.png";
        break;
    case 3:
        filename = "img/rate_3.png";
        break;
    case 4:
        filename = "img/rate_4.png";
        break;
    case 5:
        filename = "img/rate_5.png";
        break;
    }

    return filename;
}

function getCollectionModifiers() {
    var sortSelect = document.getElementById("sort-select");
    var sortOrderSelect = document.getElementById("sort-order-select");
    var sort = sortSelect.options[sortSelect.selectedIndex].value;
    var sortOrder = sortOrderSelect.options[sortOrderSelect.selectedIndex].value;
    var filterOpSelect = document.getElementById("filter-op-select");
    var filterOp = filterOpSelect.options[filterOpSelect.selectedIndex].value;
    var options = {
        "sort": {
            "key": sort,
            "order": sortOrder
        },
        "rating": {
            "value": globalFilterRate,
            "op": filterOp
        },
        "color": globalFilterColor
    };

    return options;
}

function loadCollection(collectionId) {
    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE ) {
           if (xmlhttp.status == 200) {
               var coll = JSON.parse(xmlhttp.responseText);
               var options = getCollectionModifiers();

               selectedCollection = coll;

               renderCollection(selectedCollection, options);
           } else if (xmlhttp.status == 400) {
               console.log("There was an error 400");
           } else {
               console.log("something else other than 200 was returned");
           }
        }
    };

    xmlhttp.open("GET", "collection/" + collectionId, true);
    xmlhttp.send();
}

function renderCollection(coll, options) {
    var start = Date.now();
    if (coll == null) {
        console.log("No collection selected");
        return;
    }

    var assets = [];

    for (asset of coll.assets) {
        var keep = false;

        /* Color */
        switch (options.color) {
        case 0:
            keep = true;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            if (asset.mob.color == options.color) {
                keep = true;
            }
            break;
        }

        if (!keep) {
            continue;
        }
        /* rate */
        switch (options.rating.op) {
        case "gt":
            keep = asset.mob.rating > options.rating.value;
            break;
        case "gte":
            keep = asset.mob.rating >= options.rating.value;
            break;
        case "lt":
            keep = asset.mob.rating < options.rating.value;
            break;
        case "lte":
            keep = asset.mob.rating <= options.rating.value;
            break;
        case "eq":
            keep = asset.mob.rating == options.rating.value;
            break;
        case "ne":
            keep = asset.mob.rating != options.rating.value;
            break;
        }

        if (keep) {
            assets.push(asset);
        }
    }

    /* Sort */
    assets.sort(function(a, b) {
        var va;
        var vb;
        var ret;

        if (options.sort.key == "added_time") {
            va = a.mob.added_ts_ms;
            vb = b.mob.added_ts_ms;
        } else {
            va = a.mob.capture_ts_ms;
            vb = b.mob.capture_ts_ms;
        }

        if (options.sort.order == "asc") {
            ret = va - vb;

            if (ret == 0) {
                ret = a.mob.name.localeCompare(b.mob.name);
            }
        } else {
            ret = vb - va;

            if (ret == 0) {
                ret = b.mob.name.localeCompare(a.mob.name);
            }
        }

        return ret;
    });

    var w = window.innerWidth;
    var h = window.innerHeight;
    /* pane: 232px
       margin 10px r, 10px l
       thumb size is 212px
       min number of columns is 3
       border spacing is 3
       scroll window is 15px
       footer is 150px
       header is 40px */
    var columns = 3;
    var img_x = w - 15 - 2 * 238;
    var thumb_size = 212;
    var table = document.getElementById("meta_data_tbl_top");
    var innerHtml = "";
    var newRow = "<tr>";
    var div = "<div class=\"grid-view-table-cell\">";
    var count = 1;
    var closed = false;

    table = document.getElementById("meta_data_tbl_1");
    table.rows[1].cells[1].innerHTML = coll.path;

    columns = Math.floor(img_x / (thumb_size + 28));
    innerHtml += newRow
    for (i of assets) {
        var cellId = "grid-cell-mob-" + i.id;
        mobCache[i.id] = i.mob;
        var newCell = "<td id=\"" + cellId +"\"class=\"grid-view-table-td\" onclick=\"selectMob('" + i.id + "',this);\">";
        var color = "";

        innerHtml += newCell;
        innerHtml += div;

        switch(i.mob.orientation) {
        case 3: /* image is rotated 180 cw */
            innerHtml += "<img class=\"rotate180\" width=\"" + thumb_size + "\" src=\"thumb/" + i.id + ".jpg\">";
            break;
        case 6:/* image is rotated 270 cw */
            innerHtml += "<img class=\"rotate90\" width=\"" + thumb_size + "\" src=\"thumb/" + i.id + ".jpg\">";
            break;
        case 8: /* image is rotated 90 cw */
            innerHtml += "<img class=\"rotate270\" width=\"" + thumb_size + "\" src=\"thumb/" + i.id + ".jpg\">";
            break;
        default:
            innerHtml += "<img width=\"" + thumb_size + "\" src=\"thumb/" + i.id + ".jpg\">";
        }

        switch (i.mob.color) {
        case MOB_COLOR_RED:
            color = "red";
            break;
        case MOB_COLOR_GREEN:
            color = "green";
            break;
        case MOB_COLOR_BLUE:
            color = "blue";
            break;
        case MOB_COLOR_YELLOW:
            color = "yellow";
            break;
        }

        var rating = rateFilenameFromMob(i.mob);
        innerHtml += "</div>";
        innerHtml += "<div class=\"grid-view-table-footer\">";

        /* rating */
        innerHtml += "<img class=\"rate-img\" width=\"80\" src=\"" + rating +"\">";

        /* Color */
        innerHtml += "<div onclick=\"colorDropdToggle('" + i.id + "');\"class=\"color-dropdown\">";
        innerHtml += "<button class=\"color-btn " + color + "\"></button>";
        innerHtml += "<div id=\"colorDropdown-" + i.id + "\" class=\"color-dropdown-content\">";
        innerHtml += "<a onclick=\"coloriseMob('" + i.id + "',MOB_COLOR_RED)\" href=\"#\"><button class=\"color-btn-indrop red\"></button>Red</a>";
        innerHtml += "<a onclick=\"coloriseMob('" + i.id + "',MOB_COLOR_GREEN)\" href=\"#\"><button class=\"color-btn-indrop green\"></button>Green</a>";
        innerHtml += "<a onclick=\"coloriseMob('" + i.id + "',MOB_COLOR_BLUE)\" href=\"#\"><button class=\"color-btn-indrop blue\"></button>Blue</a>";
        innerHtml += "<a onclick=\"coloriseMob('" + i.id + "',MOB_COLOR_YELLOW)\" href=\"#\"><button class=\"color-btn-indrop yellow\"></button>Yellow</a>";
        innerHtml += "<a onclick=\"coloriseMob('" + i.id + "',MOB_COLOR_NONE)\" href=\"#\"><button class=\"color-btn-indrop\"></button>None</a>";
        innerHtml += "</div></div>";

        /* edit marker */
        if (i.developed) {
            innerHtml += "<img class=\"dev-icon-img\" src='../img/dev_icon_20.png'>";
        }

        innerHtml += "</div></td>";
        closed = false;

        if (count % columns == 0) {
            innerHtml += "</tr>";
            innerHtml += newRow;
            closed = true;
        }
        count++;
    }

    if (!closed) {
        innerHtml += "</tr>";
    }

    var collTable = document.getElementById("coll-grid-table");
    collTable.innerHTML = innerHtml;

    var dur = Date.now() - start;
    console.log("renderCollection: " + dur + "ms");
}

function colorDropdToggle(id) {
    var cellId = "colorDropdown-" + id;
    var elem = document.getElementById(cellId);

    if (elem) {
        elem.classList.toggle("color-dropdown-show");
    }
}

function selectMob(id, cell) {
    /* Clear selected item */
    var collTable = document.getElementById("coll-grid-table");

    for (row of collTable.rows) {
        for (column of row.cells) {
            column.className = "grid-view-table-td";
        }
    }
    /* Mark new element */
    cell.classList.add("grid-view-table-td-active");

    /* Get the img element */
    var imgElem = cell.childNodes[0].childNodes[0];

    calculateHistogram(imgElem);
    loadExif(id);
    selectedMobId = id;
}

function navigateMob(direction) {
    if (!selectedMobId || selectedMobId == "") {
        console.log("No mob selected");
        return;
    }

    if (!("assets" in selectedCollection)) {
        return;
    }

    var currMob = -1;
    for (i = 0; i < selectedCollection.assets.length; i++) {
        if (selectedMobId == selectedCollection.assets[i].id) {
            currMob = i;
            break;
        }
    }

    if (currMob < 0) {
        /* selected mob not fount */
        console.log("navigateMob: selected mob not found");
        return;
    }

    var newMob = currMob;
    var collTable = document.getElementById("coll-grid-table");
    var numCols = collTable.rows[0].cells.length;

    switch(direction) {
    case NAV_LEFT:
        newMob -= 1;
        break;
    case NAV_UP:
        newMob -= numCols;
        break;
    case NAV_RIGHT:
        newMob += 1;
        break;
    case NAV_DOWN:
        newMob += numCols;
        break;
    }

    newMob = Math.min(selectedCollection.assets.length - 1, newMob);
    newMob = Math.max(0, newMob);
    var mobId = selectedCollection.assets[newMob].id;

    if (currMob == newMob) {
        return;
    }

    /* Get the new cell to and select it */
    var y = 0;
    while (newMob >= numCols) {
        y++;
        newMob -= numCols;
    }

    var newCell = collTable.rows[y].cells[newMob];
    selectMob(mobId, newCell);
    updateSingleView(mobId);
}

function loadExif(id) {
    if (id in exifCache) {
        renderExif(exifCache[id]);
        return;
    }

    var xmlhttp = new XMLHttpRequest();
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            if (xmlhttp.status == 200) {
                var exif = JSON.parse(xmlhttp.responseText);

                exifCache[exif.id] = exif;
                renderExif(exif);
            }
        }
    };

    xmlhttp.open("GET", "exif/" + id, true);
    xmlhttp.send();
}

function mobColorString(color) {
    switch (color) {
    case 1:
        return "Red";
    case 2:
        return "Green";
    case 3:
        return "Blue";
    case 4:
        return "Yellow";
    }

    return "";
}

function renderExif(exif) {
    var table = document.getElementById("meta_data_tbl_top");
    var mob = mobCache[exif.id];

    table.rows[0].cells[0].innerHTML = "ISO " + exif.iso;
    table.rows[0].cells[1].innerHTML = exif.focal_len + " mm";
    table.rows[0].cells[2].innerHTML = "f/" + (exif.fnumber / 10);
    table.rows[0].cells[3].innerHTML = exif.exposure_time + " sec";

    table = document.getElementById("meta_data_tbl_1");
    table.rows[0].cells[1].innerHTML = mob.name;

    table = document.getElementById("meta_data_tbl_2");
    table.rows[0].cells[1].innerHTML = exif.copyright;
    table.rows[1].cells[1].innerHTML = exif.artist;

    table = document.getElementById("meta_data_tbl_3");
    table.rows[0].cells[1].innerHTML = mob.rating + "/5";
    table.rows[1].cells[1].innerHTML = mobColorString(mob.color);

    table = document.getElementById("meta_data_tbl_4");
    table.rows[0].cells[1].innerHTML = exif.date;

    table = document.getElementById("meta_data_tbl_5");
    table.rows[0].cells[1].innerHTML = exif.x + " x " + exif.y;
    table.rows[1].cells[1].innerHTML = exif.exposure_time + " sec at f/" + (exif.fnumber / 10);
    table.rows[2].cells[1].innerHTML = exif.focal_len + " mm";
    table.rows[3].cells[1].innerHTML = exif.iso;
    if (exif.flash & 0x1) {
        table.rows[4].cells[1].innerHTML = "Fired";
    } else {
        table.rows[4].cells[1].innerHTML = "Did not fire";
    }
    table.rows[5].cells[1].innerHTML = exif.make;
    table.rows[6].cells[1].innerHTML = exif.model;
    table.rows[7].cells[1].innerHTML = exif.lens;
}

function viewSingleMob(mobId) {
    var modal = document.getElementById("view-modal");

    if (!mobId || mobId == "") {
        console.log("None selected");
        return;
    }

    /* Enable modal view */
    modal.style.display = "block";
    /* Render image */
    updateSingleView(mobId);
}

function updateSingleView(mobId) {
    var modal = document.getElementById("view-modal");
    if (modal.style.display == "none") {
        return;
    }

    var mob = mobCache[mobId];
    var image = new Image();

    image.onload = function() {
        var canvas = document.getElementById("single-image-view");
        var ctx = canvas.getContext('2d');
        var scale = 1.0;
        var x = this.width;
        var y = this.height;
        var offsetX = 0;
        var offsetY = 0;
        var ratio = x / y;

        /* This may cause problem with down sampling, if so, perform it in
           steps to trigger "larger" down sampling matrix. */

        /* When calculating scaling, the presented orientation
           must be used. */
        if (mob.orientation == 6 ||
            mob.orientation == 8) {
            var tmp = x;
            x = y;
            y = tmp;
        }

        if (x > canvas.scrollWidth) {
            scale = canvas.scrollWidth / x;
        }

        if (y * scale > (window.innerHeight - 100)) {
            scale = (innerHeight - 100)/ y;
        }

        canvas.width = Math.ceil(y * scale * ratio);
        canvas.height = Math.ceil(y * scale);

        if (zoomMode.enabled) {
            scale = 1.0;
            var lim;

            lim = x - canvas.width;
            lim = -lim;
            if (zoomMode.dx > 0.0) {
                zoomMode.dx = 0.0;
            } else if (zoomMode.dx < lim) {
                zoomMode.dx = lim;
            }
            offsetX = zoomMode.dx;

            lim = y - canvas.height;
            lim = - lim;
            if (zoomMode.dy > 0.0) {
                zoomMode.dy = 0.0;
            } else if (zoomMode.dy < lim) {
                zoomMode.dy = lim;
            }
            offsetY = zoomMode.dy;
        } else {
            var newX = Math.ceil(x * scale);
            offsetX = (canvas.width - newX) / 2;
        }

        /*
        console.log("picture dim: " + x + ":" + y);
        console.log("canvas dim: " + canvas.width + ":" + canvas.height);
        console.log("picture dim in canvas " + Math.ceil(canvas.height * ratio) + ":" + canvas.height);
        console.log("Offset: " + offsetX + ":" + offsetY);
        */
        switch (mob.orientation) {
        case 1: /* 0 */
            ctx.transform(scale, 0,
                          0, scale,
                          offsetX, offsetY);
            break;
        case 3: /* 180 */
            ctx.transform(-scale, 0,
                          0, -scale,
                          offsetX + scale * this.width, offsetY + scale * this.height);
            break;
        case 6: /* 270 */
            ctx.transform(0, scale,
                          -scale, 0,
                          offsetX + scale * this.height, offsetY);
            break;
        case 8: /* 90 */
            ctx.transform(0, -scale,
                          scale, 0,
                          offsetX, offsetY + scale * this.width);
            break;
        }

        ctx.drawImage(this, 0, 0, this.width, this.height);
    };

    image.src = "/proxy/" + mobId + ".jpg";
}

function closeSingleView() {
    var modal = document.getElementById("view-modal");
    modal.style.display = "none";
}

/*
 * Create an offscreen canvas and draw the image. Extract the pixel
 * data and calculate lum, r, g, b histograms.
 */
function calculateHistogram(img) {
    var canvas = document.createElement('canvas');
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

    /* Reset histograms */
    for (i = 0; i < 256; i++) {
        histLum[i] = 0;
        histRed[i] = 0;
        histGreen[i] = 0;
        histBlue[i] = 0;
    }

    canvas.width = img.width;
    canvas.height = img.height;
    canvas.getContext('2d').drawImage(img, 0, 0, img.width, img.height);
    imgData = canvas.getContext('2d').getImageData(0, 0, canvas.width, canvas.height);
    pixels = imgData.data;

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

    histDataL.data = pl;
    histDataR.data = pr;
    histDataG.data = pg;
    histDataB.data = pb;
    histChart.update();
}

function rateMob(mobId, rate) {
    if (!mobId || mobId == "") {
        console.log("No mob selected");
        return
    }

    var mob = mobCache[mobId];
    var not = document.getElementById("popup-rate-set");
    var opt = getCollectionModifiers();

    /* Update and show rating information */
    not.innerHTML = "Set rating to " + rate;
    not.style.display = "block";
    setTimeout(function(){
        not.style.display = "none";
    }, 800);

    mob.rating = rate;
    updateMob(mob);

    /* If filters is active, update collection */
    if (opt.rating.value > 0 || opt.color > 0) {
        renderCollection(selectedCollection, opt);
    }
}

function filterRate(rate) {
    var btnGrp = document.getElementById("rate-btn-grp");
    /*
      button layout: 1, 3, 5, 7, 9
      rating:        5, 4, 3, 2, 1
    */

    /* save state */
    globalFilterRate = rate;

    /* reset */
    for (i = 1; i < 10; i += 2) {
        btnGrp.childNodes[i].className = "";
    }

    if (rate == 0) {
        /* update view */
        var options = getCollectionModifiers();
        renderCollection(selectedCollection, options);
        return;
    }

    rate = 5 - rate;
    rate = rate * 2;

    for (i = rate + 1; i < 10; i += 2) {
        btnGrp.childNodes[i].classList.add("selected");
    }

    /* update view */
    var options = getCollectionModifiers();
    renderCollection(selectedCollection, options);
}

function filterColor(color) {
    var btnGrp = document.getElementById("color-btn-grp");
    /*
      button layout: 1, 3, 5, 7, 9
      rating:        r, g, b, y, none
      values:        1, 2, 3, 4, 0
    */

    globalFilterColor = color;

    for (i = 1; i < 10; i += 2) {
        btnGrp.childNodes[i].style.border = "3px solid #686868";
    }

    if (color == 0) {
        /* update view */
        var options = getCollectionModifiers();
        renderCollection(selectedCollection, options);
        return;
    }

    color -= 1;
    color *= 2;
    color += 1;

    btnGrp.childNodes[color].style.border = "0px";

    /* update view */
    var options = getCollectionModifiers();
    renderCollection(selectedCollection, options);
}

function coloriseMob(mobId, color) {
    if (!mobId || mobId == "") {
        console.log("No mob selected");
        return
    }

    var mob = mobCache[mobId];
    var not = document.getElementById("popup-rate-set");
    var opt = getCollectionModifiers();

    /* Update and show rating information */
    not.innerHTML = "Set color to " +  mobColorString(color);
    not.style.display = "block";
    setTimeout(function(){
        not.style.display = "none";
    }, 800);

    mob.color = color;
    updateMob(mob);

    /* If filters is active, update collection */
    if (opt.rating.value > 0 || opt.color > 0) {
        renderCollection(selectedCollection, opt);
    }
}

function updateMob(mob) {
    var xmlhttp = new XMLHttpRequest();
    var jsonString = JSON.stringify(mob);

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            if (xmlhttp.status == 200) {
                var newMob = JSON.parse(xmlhttp.responseText);
                mobCache[newMob.id] = newMob;

                /* update rating */
                var cellId = "grid-cell-mob-" + newMob.id;
                var cell = document.getElementById(cellId);

                if (cell == null) {
                    /* This can happen during update of a mob when a filter is
                       active, i.e filter on 3+ rating, and lower ratint to
                       not match filter. When the response for update mob
                       is returned, the collection list may already have been
                       rebuilt. */
                    console.log("Could not get cell for mob: " + newMob.id);
                    return;
                }


                var imgElem = cell.childNodes[1].childNodes[0];

                imgElem.src = rateFilenameFromMob(newMob);

                /* update color */
                var colorElem = cell.childNodes[1].childNodes[1].childNodes[0];

                colorElem.classList.remove("red");
                colorElem.classList.remove("green");
                colorElem.classList.remove("blue");
                colorElem.classList.remove("yellow");

                switch (newMob.color) {
                case MOB_COLOR_RED:
                    colorElem.classList.add("red");
                    break;
                case MOB_COLOR_GREEN:
                    colorElem.classList.add("green");
                    break;
                case MOB_COLOR_BLUE:
                    colorElem.classList.add("blue");
                    break;
                case MOB_COLOR_YELLOW:
                    colorElem.classList.add("yellow");
                    break;
                }

                /* Make new data visible */
                loadExif(newMob.id);
            }
        }
    };

    xmlhttp.open("PUT", "mob/" + mob.id, true);
    xmlhttp.setRequestHeader('Content-Type', 'application/json');
    xmlhttp.send(jsonString);
}

function updateCounts(coll) {
    var c = coll["children"];
    var count = 0;

    for (var prop in c) {
        if (c.hasOwnProperty(prop)) {
            updateCounts(c[prop]);
            count += c[prop]["count"];
        }
    }

    coll["count"] += count;
}

window.addEventListener("load", function(evt) {
    var id = getParameterByName("id")
    var xmlhttp = new XMLHttpRequest();

    /* Prepare histogram */
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

    /* Fetch all collections and draw them. */
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            if (xmlhttp.status == 200) {
                var coll_tree = {};
                var coll = JSON.parse(xmlhttp.responseText);

                /* Build the tree structure of all collections */
                coll.sort(function(a,b) {
                    return a.path.localeCompare(b.path);
                });

                if (coll[0].path != "/") {
                    alert("This is not right");
                }
                coll_tree["path"] = "Collection root";
                coll_tree["id"] = coll[0].id;
                coll_tree["count"] = coll[0].count;
                coll_tree["children"] = {};

                for (i of coll) {
                    if (i.path == "/") {
                        continue;
                    }

                    var comps = i.path.split("/");
                    var root = coll_tree;

                    for (c of comps) {
                        if (c.length == 0) {
                            continue;
                        }

                        if (!(c in root.children)) {
                            var child = {
                                "children": {},
                                "path": c,
                                "count": 0,
                            };
                            root["children"][c] = child;
                        }

                        root = root["children"][c];
                    }

                    root["id"] = i.id;
                    root["count"] = i.count;
                }

                /* Update cumulative count of assets */
                updateCounts(coll_tree);

                /* Create the HTML for it by doing a depth first
                   traversal. */
                var stack = [];
                var innerHtml = "";

                stack.push(coll_tree);
                while (stack.length > 0) {
                    var node = stack.pop();
                    var name = node.path + " (" + node.count + ")";

                    if ("closeLi" in node) {
                        innerHtml += "</li>";
                        continue;
                    }
                    if ("closeUl" in node) {
                        innerHtml += "</ul>";
                        continue;
                    }

                    innerHtml += "<li>";

                    if ("id" in node) {
                        innerHtml += "<a href=\"#\" onclick=\"loadCollection('" + node.id + "');\">" + name + "</a>";
                    } else {
                        innerHtml += name;
                    }

                    stack.push({
                        "closeLi": true,
                    });

                    var childs = Object.keys(node.children);
                    if (childs.length > 0) {
                        innerHtml += "<ul>";
                        stack.push({
                            "closeUl": true,
                        });
                        for (i = childs.length - 1; i >= 0; i--) {
                            stack.push(node.children[childs[i]]);
                        }
                    }
                }

                var collUl = document.getElementById("coll-list");
                collUl.innerHTML = innerHtml;
                /* Initialize the list */
                CollapsibleLists.apply();
            } else {
                console.log("Error");
                console.log(xmlhttp);
            }
        }
    };

    /* Mouse listeners for single image canvas */
    var siCanvas = document.getElementById("single-image-view");
    siCanvas.addEventListener('mousedown', function(event) {
        if (zoomMode.enabled) {
            zoomMode.drag = true;
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
        }
    });

    siCanvas.addEventListener('mouseup', function(event) {
        if (zoomMode.enabled) {
            zoomMode.drag = false;
        }
    });

    siCanvas.addEventListener('mousemove', function(event) {
        if (zoomMode.enabled && zoomMode.drag){
            zoomMode.dx += (event.pageX - zoomMode.startX);
            zoomMode.dy += (event.pageY - zoomMode.startY);
            updateSingleView(selectedMobId);
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
        }
    });

    xmlhttp.open("GET", "collection/", true);
    xmlhttp.send();

    /* Load collection if present */
    if (id) {
        loadCollection(id);
    }

    /* Configure hosts */
    var url = window.location.href.split("/");
    PROTO = url[0];
    if (COLLD_HOST == null) {
        COLLD_HOST = url[2].split(":")[0];
    }
    if (EDITD_HOST == null) {
        EDITD_HOST = url[2].split(":")[0];
    }

    var col = getParameterByName('col');
    if (col != null && col != "") {
        loadCollection(col);
    }
});

window.onclick = function(event) {
    if (!event.target.matches('.color-btn')) {
        var dropdowns = document.getElementsByClassName("color-dropdown-content");

        for (var i = 0; i < dropdowns.length; i++) {
            var openDropdown = dropdowns[i];
            openDropdown.classList.remove('color-dropdown-show');
        }
    }
};

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
        navigateMob(evt.keyCode);
        break;
        /* rating */
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
        var rate = evt.keyCode - 48;
        rateMob(selectedMobId, rate);
        break;
        /* color */
    case 54:
    case 55:
    case 56:
    case 57:
        var color = evt.keyCode - 53;
        coloriseMob(selectedMobId, color);
        break;
    case 68: /* d */
        var newUrl = PROTO + "//" + EDITD_HOST + ":" + EDITD_PORT + "/?img=" + selectedMobId + "&col=" + selectedCollection.id;
        window.location.replace(newUrl);
        break;
    case 69: /* e */
        viewSingleMob(selectedMobId);
        break;
    case 71: /* g */
        closeSingleView();
        break;
    case 90:
        if (zoomMode.enabled) {
            zoomMode.enabled = false;
        } else {
            zoomMode.enabled = true;
        }
        updateSingleView(selectedMobId);
        break;
    }

};
