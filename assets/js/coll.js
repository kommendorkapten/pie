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

function loadTable(id) {
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
        
    console.log(h);

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
               for (i of coll.items) {
                   innerHtml += newCell;
                   innerHtml += div;
                   innerHtml += "<img width=\"" + thumb_size + "\" src=\"" + i.url + "\">";
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
               console.log('There was an error 400');
           }
           else {
               console.log('something else other than 200 was returned');
           }
        }
    };

    xmlhttp.open("GET", "collection/" + 123, true);
    xmlhttp.send();
}   

window.addEventListener("load", function(evt) {
    var id = getParameterByName("id")
    console.log("hey " + id);
    loadTable(id);
});
