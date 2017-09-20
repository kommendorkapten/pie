# In progress

# Feature

## Sorted
* Add hex (sha1) to min.
* Create links from collectiond to editd.
* Editd, when loading image, automatically set parameters in ui.
* editd, remove histogram websocket. Calculate histogram in ui as of
  collectiond.
* In library view, metadata field to right, present proper collection
  name.
* Zoom in library mode.

## Unsorted
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
