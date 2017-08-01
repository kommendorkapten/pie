var mobCache = {};
var exifCache = {};
var selectedMobId = "";
var selectedCollection = {};
var NAV_LEFT = 37;
var NAV_UP = 38;
var NAV_RIGHT = 39;
var NAV_DOWN = 40;

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

function loadCollection(collectionId) {
    var xmlhttp = new XMLHttpRequest();
    var w = window.innerWidth;
    var h = window.innerHeight;
    // pane: 280px width (282 with borders)
    // margin 10px r, 10px l
    // max thumb size is 256
    // min number of columns is3
    // border spacing is 3
    // scroll window is 15px
    // footer is 150px
    // header is 40px
    var columns = 3;
    var img_x = w - 2 * 282 - 15;
    var cellPadding = 5;
    var basePadding = 15;
    var heightModifier = 150 + 40 + 20;

    columns = Math.floor(img_x / 256 + 0.3);
    if (navigator.appVersion.indexOf("Mac")!=-1) {
        cellPadding = 6;
        basePadding = 20;
    }
    thumb_size = Math.floor(img_x / columns - basePadding - columns * cellPadding);
    if (thumb_size > 256) {
        thumb_size = 256;
    }

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE ) {
           if (xmlhttp.status == 200) {
               var coll = JSON.parse(xmlhttp.responseText);
               var innerHtml = "";
               var newRow = "<tr>";
               var div = "<div class=\"grid-view-table-cell\">";
               var count = 1;
               var closed = false;

               innerHtml += newRow
               for (i of coll.assets) {
                   mobCache[i.id] = i.mob;
                   
                   var newCell = "<td class=\"grid-view-table-td\" onclick=\"selectMob('" + i.id + "',this);\">";

                   innerHtml += newCell;
                   innerHtml += div;
                   innerHtml += "<img width=\"" + thumb_size + "\" src=\"thumb/" + i.id + ".jpg\">";
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
               var collGrid = document.getElementById("grid-collection-pane");
               collGrid.style.height = h - heightModifier;

               /* Save selected collection last after any sorting has been
                  applied */
               selectedCollection = coll;
           }
           else if (xmlhttp.status == 400) {
               console.log("There was an error 400");
           }
           else {
               console.log("something else other than 200 was returned");
           }
        }
    };

    xmlhttp.open("GET", "collection/" + collectionId, true);
    xmlhttp.send();
}

function selectMob(id, cell) {
    // Clear selected item
    var collTable = document.getElementById("coll-grid-table");

    for (row of collTable.rows) {
        for (column of row.cells) {
            column.className = "grid-view-table-td";
        }
    }
    // Mark new element
    cell.className += " grid-view-table-td-active";

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
    case 5:
        return "Black";
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
    table.rows[1].cells[1].innerHTML = "collection";

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
    if (!mobId || mobId == "") {
        console.log("None selected");
        return;
    }
    
    var modal = document.getElementById("view-modal");
    var modalImg = document.getElementById("single-image-view");
    
    modal.style.display = "block";
    modalImg.src = "/proxy/" + mobId + ".jpg";
}

function updateSingleView(mobId) {
    var modal = document.getElementById("view-modal");
    var modalImg = document.getElementById("single-image-view");
    
    if (modal.style.display == "none") {
        return;
    }
    modalImg.src = "/proxy/" + mobId + ".jpg";    
}

function closeSingleView() {
    var modal = document.getElementById("view-modal");
    modal.style.display = "none";    
}

window.addEventListener("load", function(evt) {
    var id = getParameterByName("id")
    var xmlhttp = new XMLHttpRequest();
    // Get all collections and draw them.

    xmlhttp.onreadystatechange = function() {
        if (xmlhttp.readyState == XMLHttpRequest.DONE) {
            if (xmlhttp.status == 200) {
                var coll_tree = {};
                var coll = JSON.parse(xmlhttp.responseText);

                // Build the tree structure of all collections
                coll.sort(function(a,b) {
                    return a.path.localeCompare(b.path);
                });

                if (coll[0].path != "/") {
                    alert("This is not right");
                }
                coll_tree["path"] = "Collection root";
                coll_tree["id"] = coll[0].id;
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
                            };
                            root["children"][c] = child;
                        }

                        root = root["children"][c];
                    }

                    root["id"] = i.id;
                }

                console.log(coll_tree);
                // Create the HTML for it by doing a depth first
                // traversal.
                var stack = new Array();
                var innerHtml = "";

                stack.push(coll_tree);
                while (stack.length > 0) {
                    var node = stack.pop();

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
                        innerHtml += "<a href=\"#\" onclick=\"loadCollection('" + node.id + "');\">" + node.path + "</a>";
                    } else {
                        innerHtml += node.path;
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
                // Initialize the list
                CollapsibleLists.apply();
            } else {
                console.log("Error");
                console.log(xmlhttp);
            }
        }
    };

    xmlhttp.open("GET", "collection/", true);
    xmlhttp.send();

    // Load collection if present
    if (id) {
        console.log("Get collection: " + id);
        loadCollection(id);
   }
});

document.onkeydown = function(evt) {
    evt = evt || window.event;

    console.log(evt.keyCode);
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
        console.log("rate " + rate);
        break;
    case 68: /* d */
        console.log("Develop view");
        break;
    case 69: /* e */
        viewSingleMob(selectedMobId);
        break;
    case 71: /* g */
        closeSingleView();
        break;
    case 90:
        console.log("zoom");
        break;
    }

};
