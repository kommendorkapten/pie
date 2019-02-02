'use strict';

var COLLD_PORT = 8081;
var EDITD_PORT = 8080;
var COLLD_HOST = null;
var EDITD_HOST = null;
var PROTO = null;
var mobCache = {};
var exifCache = {};
var selectedMobId = "";
var selectedCollection = {};
/* activeAssets is updated by renderCollection */
var activeAssets = [];
var zoomMode = {
    "enabled": false,
    "mode": "center",
    "drag": false,
    "startX": 0,
    "startY": 0,
    "dx": 0,
    "dy": 0,
};
var collTree = null;
var globalFilterRate = 0;
var globalFilterColor = 0;
var NAV_LEFT = 37;
var NAV_UP = 38;
var NAV_RIGHT = 39;
var NAV_DOWN = 40;
var histChart;
var histChartSingle;
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
var exifRotationClass = {
    3: 'rotate180',
    6: 'rotate90',
    8: 'rotate270',
};
var fullsizeProxy = null;

function allowDrop(ev) {
    ev.preventDefault();
}

function drag(ev) {
    let mobId = ev.target.attributes["mobid"];
    if (mobId) {
        mobId = mobId.value;
    }
    ev.dataTransfer.setData("text", mobId);
}

function drop(ev) {
    ev.preventDefault();
    let mobId = ev.dataTransfer.getData("text");
    let colId = ev.target.attributes["colid"];

    if (mobId == "undefined") {
        console.log("no MOB provided");
        return;
    }

    if (colId) {
        colId = colId.value;
        let name = ev.target.innerText;

        console.log("Add " + mobId + " to " + colId + " [" + name + "]");
        prepareMoveMob(colId, name, mobId);
    } else {
        console.log(ev.target);
        console.log(ev.target.innerText);
        console.log("Not a valid collection");
    }
}

function prepareMoveMob(colId, name, mobId) {
    let form = document.getElementById("mov_form_popup");

    form.style.display = "block";
    form.querySelector("h2").innerText = "Move 1 item to " + name;
    form.colId = colId
    form.mobId = mobId
}

function moveMob() {
    let form = document.getElementById("mov_form_popup");
    let colId = form.colId;
    let mobId = form.mobId;
    let url = "collection/" + colId + "/asset/" + mobId;
    let xmlhttp = new XMLHttpRequest();

    if (!(colId && mobId)) {
        return;
    }

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            console.log(xmlhttp.status);
            if (xmlhttp.status == 200) {
                let coll = JSON.parse(xmlhttp.responseText);

                selectedCollection = coll;
                renderCollection(selectedCollection, getCollectionModifiers());
            }
        }
    }

    xmlhttp.open("POST", url, true);
    xmlhttp.send("-");

    form.style.display = "none";
}

function closeMovForm() {
    let form = document.getElementById("mov_form_popup");
    form.style.display = "none";
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

function loadCollection(collectionId, mobId) {
    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE ) {
           if (xmlhttp.status == 200) {
               var coll = JSON.parse(xmlhttp.responseText);
               var options = getCollectionModifiers();

               selectedCollection = coll;

               if (mobId != null && mobId != "") {
                   options.mobId = mobId
               }

               renderCollection(selectedCollection, options);
           } else if (xmlhttp.status == 400) {
               console.log("There was an error 400");
           } else {
               console.log("something else other than 200 was returned: " + xmlhttp.status);
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

    activeAssets = [];

    for (let asset of coll.assets) {
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
            activeAssets.push(asset);
        }
    }

    /* Sort */
    activeAssets.sort(function(a, b) {
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
    var table;
    var innerHtml = "";
    var newRow = "<tr>";
    var div = "<div class=\"grid-view-table-cell\">";
    var count = 1;
    var closed = false;

    table = document.getElementById("meta_data_tbl_1");
    table.rows[1].cells[1].innerHTML = coll.path;

    columns = Math.floor(img_x / (thumb_size + 28));
    innerHtml += newRow
    for (let i of activeAssets) {
        var cellId = "grid-cell-mob-" + i.id;
        mobCache[i.id] = i.mob;
        var newCell = "<td id=\"" + cellId +"\"class=\"grid-view-table-td\" onclick=\"selectMob('" + i.id + "',this);\" mobid=\"" + i.id + "\">";
        var color = "";

        innerHtml += newCell;
        innerHtml += div;
	innerHtml += '<img class="' + (exifRotationClass[i.mob.orientation] || '') +
	    '" width="' + thumb_size +
            '" draggable="true" ondragstart="drag(event)"' +
            ' mobid="' + i.id + '"' +
	    ' src="thumb/' + i.id + '.jpg">';

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
    collTable.addEventListener('contextmenu', contextMenu);

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

function selectMob(id, cell=null) {
    /* Clear selected item */
    var collTable = document.getElementById("coll-grid-table");

    for (let row of collTable.rows) {
        for (let column of row.cells) {
            column.className = "grid-view-table-td";
        }
    }
    /* Mark new element */
    cell.classList.add("grid-view-table-td-active");

    /* Get the img element */
    var imgElem = cell.childNodes[0].childNodes[0];

    renderHistogram(imgElem);
    loadExif(id);
    selectedMobId = id;
}

function navigateMob(direction) {
    if (!selectedMobId || selectedMobId == "") {
        console.log("No mob selected");
        return;
    }

    var currMob = -1;
    for (let i = 0; i < activeAssets.length; i++) {
        if (selectedMobId == activeAssets[i].id) {
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

    newMob = Math.min(activeAssets.length - 1, newMob);
    newMob = Math.max(0, newMob);
    var mobId = activeAssets[newMob].id;

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

function contextMenu(e) {
    e.preventDefault();
    var tgt = e.target;
    var left = e.clientX;
    var top = e.clientY;
    var menuBox = document.getElementById("coll-context-menu");
    var delLink = document.getElementById("coll-context-menu-del");
    var expLink = document.getElementById("coll-context-menu-exp");
    var mobid = null;

    mobid = tgt.getAttribute("mobid");
    if (mobid == null && tgt.localName == "img") {
        mobid = tgt.parentElement.parentElement.getAttribute("mobid");
    }

    if (!mobid){
        return;
    }

    menuBox.style.left = (left - 10) + "px";
    menuBox.style.top = (top - 10) + "px";
    menuBox.classList.add("coll-context-menu-active");
    menuBox.active = true;
    delLink.mobId = mobid;
    expLink.mobId = mobid;
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
    var table;
    var mob = mobCache[exif.id];

    /* Update first table in grid view */
    table = document.getElementById("meta_data_tbl_top");
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

    /* Update first table in single image view */
    table = document.getElementById("meta_data_tbl_top_single");
    table.rows[0].cells[0].innerHTML = "ISO " + exif.iso;
    table.rows[0].cells[1].innerHTML = exif.focal_len + " mm";
    table.rows[0].cells[2].innerHTML = "f/" + (exif.fnumber / 10);
    table.rows[0].cells[3].innerHTML = exif.exposure_time + " sec";

    table = document.getElementById("meta_data_tbl_1_single");
    table.rows[0].cells[1].innerHTML = mob.name;

    table = document.getElementById("meta_data_tbl_2_single");
    table.rows[0].cells[1].innerHTML = exif.copyright;
    table.rows[1].cells[1].innerHTML = exif.artist;

    table = document.getElementById("meta_data_tbl_3_single");
    table.rows[0].cells[1].innerHTML = mob.rating + "/5";
    table.rows[1].cells[1].innerHTML = mobColorString(mob.color);

    table = document.getElementById("meta_data_tbl_4_single");
    table.rows[0].cells[1].innerHTML = exif.date;

    table = document.getElementById("meta_data_tbl_5_single");
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

/* This implementation is somewhat broken. The image
   is reloaded every time, and something makes the
   ctx's transform be reset which makes it work.
   Otherwise the the transformations would accumulate */
function updateSingleView(mobId) {
    var modal = document.getElementById("view-modal");

    if (!modal.style.display) {
        return;
    }
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

        fullsizeProxy = this;
        var exif = exifCache[mobId];
        updateImgNavigation(document.getElementById("img_nav_canvas"),
                            fullsizeProxy,
                            exif,
                            canvas);

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
function renderHistogram(img) {
    var canvas = document.createElement('canvas');

    canvas.width = img.width;
    canvas.height = img.height;

    /* Fill canvas with data */
    canvas.getContext('2d').drawImage(img, 0, 0, canvas.width, canvas.height);
    var hist = calculateHistogram(canvas);

    histDataL.data = hist.pl;
    histDataR.data = hist.pr;
    histDataG.data = hist.pg;
    histDataB.data = hist.pb;
    histChart.update();
    histChartSingle.update();
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

function deleteMob(mobId) {
    if (!mobId) {
        return;
    }

    let url = "collection/" + selectedCollection.id + "/asset/" + mobId;
    var xmlhttp = new XMLHttpRequest();

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE ) {
           if (xmlhttp.status == 200) {
               var coll = JSON.parse(xmlhttp.responseText);

               selectedCollection = coll;
               renderCollection(selectedCollection, getCollectionModifiers());
           }
        }
    };

    xmlhttp.open("DELETE", url, true);
    xmlhttp.send();
}

function exportMob() {
    let form = document.getElementById("exp_form_popup");
    let resize = form.querySelectorAll('[name=resize]')[0].checked;
    let sharpen = form.querySelectorAll('[name=sharpen]')[0].checked;
    let req = {
        "max_x": -1,
        "max_y": -1,
        "sharpen": 0,
        "disable_exif": false,
        "mobs": form.mobIds
    };
    let path = form.querySelectorAll('[name=path]')[0].value;
    let stgId = form.querySelectorAll('[name=storages]')[0].value;
    let xmlhttp = new XMLHttpRequest();

    if (sharpen) {
        req.sharpen = 1;
    }
    if (resize) {
        req.max_x = form.querySelectorAll('[name=max_x]')[0].value;
        req.max_y = form.querySelectorAll('[name=max_y]')[0].value;
    }

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            if (xmlhttp.status == 200) {
                document.getElementById("exp_form_popup").style.display = "none";
            }
        }
    }

    let url = "export/" + stgId + "/" + path;
    xmlhttp.open("POST", url, true);
    xmlhttp.send(JSON.stringify(req))
}

function prepareExportMob(mobId) {
    let form = document.getElementById("exp_form_popup");
    let xmlhttp = new XMLHttpRequest();
    let sharpen = form.querySelectorAll('[name=sharpen]')[0].checked = true;

    form.mobIds = [mobId];
    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE ) {
            if (xmlhttp.status == 200) {
                let stgs = JSON.parse(xmlhttp.responseText);
                let expStgs = stgs.storages.filter(s => s.type == "export");
                let path = form.querySelectorAll('[name=path]')[0];
                let select = form.querySelectorAll('[name=storages]')[0];
                let d = new Date();

                path.value = d.getFullYear() + "-" + (d.getMonth() + 1) + "-" + d.getDate();

                for (let stg of expStgs) {
                    let opt = document.createElement('option');

                    opt.text = stg.name + "@" + stg.hostname;
                    opt.value = stg.id;

                    select.add(opt);
                }

                form.style.display = "block";
           }
        }
    };

    xmlhttp.open("GET", "storage", true);
    xmlhttp.send();
}

function closeExpForm() {
    document.getElementById("exp_form_popup").style.display = "none";
}

function updateUiFilterRate(rate) {
    var btnGrp = document.getElementById("rate-btn-grp");
    /*
      button layout: 1, 3, 5, 7, 9
      rating:        5, 4, 3, 2, 1
    */


    /* reset */
    for (let i = 1; i < 10; i += 2) {
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

    for (let i = rate + 1; i < 10; i += 2) {
        btnGrp.childNodes[i].classList.add("selected");
    }
}

function filterRate(rate) {
    /* save state */
    globalFilterRate = rate;

    updateUiFilterRate(rate);

    /* update view */
    var options = getCollectionModifiers();
    renderCollection(selectedCollection, options);
}

function updateUiFilterColor(color) {
    var btnGrp = document.getElementById("color-btn-grp");
    /*
      button layout: 1, 3, 5, 7, 9
      rating:        r, g, b, y, none
      values:        1, 2, 3, 4, 0
    */

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
}

function filterColor(color) {
    globalFilterColor = color;

    updateUiFilterColor(color);

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

window.addEventListener("load", function(evt) {
    var col = getParameterByName('col');
    var mob = getParameterByName("mob");
    var rate = getParameterByName("rate");
    var color = getParameterByName("color");
    var xmlhttp = new XMLHttpRequest();

    /* Load filter */
    if (rate != null && rate != "") {
        rate = parseInt(rate);
        if (rate < 6 && rate > 0) {
            globalFilterRate = rate;
            updateUiFilterRate(rate);
        }
    }
    if (color != null && color != "") {
        color = parseInt(color);
        if (color < 5 && color > 0) {
            globalFilterColor = color;
            updateUiFilterColor(color);
        }
    }

    /* Prepare histogram */
    var histCanvas = document.getElementById("hist_canvas").getContext("2d");
    var histCanvasSingle = document.getElementById("hist_canvas_single").getContext("2d");

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
    histChartSingle = new Chart(histCanvasSingle, {
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
                var coll = JSON.parse(xmlhttp.responseText);
                var collUl = document.getElementById("coll-list");

                collTree = new CollectionTree(collUl);
                collTree.load(coll);
                collTree.countCumulative();
                collTree.render();
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
            updateImgNavigation(document.getElementById("img_nav_canvas"),
                                fullsizeProxy,
                                exifCache[selectedMobId],
                                siCanvas);
            zoomMode.startX = event.pageX;
            zoomMode.startY = event.pageY;
        }
    });

    xmlhttp.open("GET", "collection/", true);
    xmlhttp.send();

    /* Configure hosts */
    var url = window.location.href.split("/");
    PROTO = url[0];
    if (COLLD_HOST == null) {
        COLLD_HOST = url[2].split(":")[0];
    }
    if (EDITD_HOST == null) {
        EDITD_HOST = url[2].split(":")[0];
    }

    if (col != null && col != "") {
        loadCollection(col, mob);
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

    var menuBox = document.getElementById("coll-context-menu");
    if (menuBox.active) {
        menuBox.classList.remove("coll-context-menu-active");
        menuBox.active = true;
    }
};

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
        var newUrl = PROTO + "//" + EDITD_HOST + ":" + EDITD_PORT;

        newUrl += "/?mob=" + selectedMobId;
        newUrl += "&col=" + selectedCollection.id;
        newUrl += "&rate=" + globalFilterRate;
        newUrl += "&color=" + globalFilterColor;

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
