'use strict';

function CollectionTree(elem) {
    let tree = {};

    /**
     * Load a list of collection.
     * @param colls an array of collections.
     */
    this.load = function (colls) {
        let tree = {};

        /* Build the tree structure of all collections */
        colls.sort(function(a, b) {
            return a.path.localeCompare(b.path);
        });

        if (colls[0].path != "/") {
            alert("This is not right");
        }
        tree["path"] = "Collection root";
        tree["id"] = colls[0].id;
        tree["count"] = colls[0].count;
        tree["children"] = {};

        for (let i of colls) {
            if (i.path == "/") {
                continue;
            }

            let comps = i.path.split("/");
            let root = tree;

            for (let c of comps) {
                if (c.length == 0) {
                    continue;
                }

                if (!(c in root.children)) {
                    let child = {
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

        this.tree = tree;
    };

    this.render = function() {
        /* Create the HTML for it by doing a depth first
           traversal. */
        var stack = [];
        var innerHtml = "";

        stack.push(this.tree);
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
                innerHtml += "<a href=\"#\" onclick=\"loadCollection('" + node.id + "');\" colid=\"" + node.id + "\">" + name + "</a>";
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
                for (let i = childs.length - 1; i >= 0; i--) {
                    stack.push(node.children[childs[i]]);
                }
            }
        }

        elem.innerHTML = innerHtml;
        /* Initialize the list */
        CollapsibleLists.apply();
        CollapsibleLists.toggle(elem.children[0]);
    };

    this.countCumulative = function() {
        countCumulative(this.tree)
    };
}

function countCumulative(colls) {
    var c = colls["children"];
    var count = 0;

    for (var prop in c) {
        if (c.hasOwnProperty(prop)) {
            countCumulative(c[prop]);
            count += c[prop]["count"];
        }
    }

    colls["count"] += count;
}
