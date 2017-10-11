# In progress
* Create links from collectiond to editd.

# Feature

## Sorted
* Editd, when loading image, automatically set parameters in ui.
* Editd, persist parameters in db.
* editd, remove histogram websocket. Calculate histogram in ui as of
  collectiond.
* In library view, metadata field to right, present proper collection
  name.
* Zoom in library mode.

## Unsorted
+ handle file name collisions during import.
+ show num files in collection list.
* Update proxy and thumb with any adjustments from editd.
* Auto expand top collection.
* Support for color tagging in collection mode.
* Filter based on rating and color.
* Move images between collections (rename(2), EXDEV (different file
  sys)).
* Load collection in editd.
* Black adjustment is quite simple, pure linear for both +/-. For
  lightening blacks a non-linear method should be used.
* White adjustment is quite simple, pure linear for both +/-. For
  darkening whites a non-linear method should be used.

# Bugs

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
