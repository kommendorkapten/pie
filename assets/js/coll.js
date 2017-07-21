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
    var cellPadding = 4;
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
               //               document.getElementById("myDiv").innerHTML = xmlhttp.responseText;
               var coll = JSON.parse(xmlhttp.responseText);
               var innerHtml = "";
               var newRow = "<tr>";
               var newCell = "<td class=\"grid-view-table-td\">";
               var div = "<div class=\"grid-view-table-cell\">";
               var count = 1;
               var closed = false;

               innerHtml += newRow
               for (i of coll.assets) {
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
