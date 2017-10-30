# In progress

# Sorted
* collectiond should update mobs via mediad.
* Add method to get fd from q_consumser.
* Rewrite mediad to use poll(2) when reading from queues.

# Unsorted
* better javascript management for color tag dropdown. Remove global
  js func.
* editd, remove histogram websocket. Calculate histogram in ui as of
  collectiond.
* Export pictures.
* Thumbnail in single view, that shows zoomed part of image (navigation).
* zoom/pan in development view
* Timestamp to log messages
* Remove CORS headers from pie_http_lws_write, replace with hmap for
  custom headers.
* Smarter management of collections, only load visible thumbnails.
* Calculate new radius for clarity when an image is loaded. The one
  stored in the database may be for the wrong resolution.
* handle file name collisions during import.
* Update proxy and thumb with any adjustments from editd.
* Auto expand top collection.
* Move images between collections (rename(2), EXDEV (different file
  sys)).
* Load collection in editd.
* Black adjustment is quite simple, pure linear for both +/-. For
  lightening blacks a non-linear method should be used.
* White adjustment is quite simple, pure linear for both +/-. For
  darkening whites a non-linear method should be used.
* "time line" with pictures in bottom with selected collection. Both
  dev and coll view.
* libraw to use lcms on sparc.
* add size to min
* add min to proxy
* When navigating from dev to coll, expand list and show last state.
* Canvas in editd sometimes contains a non-used line in the top (white
  colored). Can be seen for image: IMG_0259.CR2
* Exif parsing, shutter speed greater than 1s
* Downsample yields weird results when scale factor is close to 1.

# Performance:
* Memory leak check
** Editd (may need to be converted to single thread).
** Ingestd
** Collectiond
** Mediad
* Downsample.
* Blur for powerpc.
* Multi threaded algs.

# Misc
* Convert Makefile to POSIX make.

# Done

* ~~Read/write 16b PNG.~~
* ~~Limit histogram dynamic range in coll.js.~~
* ~~Load exif metadata via libraw.~~
* ~~Load image from libraw.~~
* ~~Exif parsing, wierd fractions for shutter speed < 1s.~~
* ~~Editd/session, remove command/response in session object.
  Fix duplicate global server object.~~
* ~~Editd, manage img via mobs.~~
* ~~collection in meta data pane.~~
* ~~Add hex (sha1) to min.~~
* ~~Short cut for color (6-9).~~
* ~~Create links from collectiond to editd.~~
* ~~In library view, metadata field to right, present proper collection
  name.~~
* ~~Editd, persist parameters in db.~~
* ~~Editd, when loading image, automatically set parameters in ui.~~
* ~~Zoom/pan in library mode.~~
* ~~Move pie dwn sample into pie_bm.h~~
* ~~sort order in collecton view.~~
* ~~Filter based on rating and color.~~
* ~~Better visualization for color tag in collection view.~~
* ~~show info on images with development settings.~~
* ~~Update collection table in UI when an image's rating/color is updated.~~
* ~~show num files in collection list.~~
