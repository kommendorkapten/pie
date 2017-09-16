# In progress

# Feature

* Editd, when loading image, automatically set parameters in ui.
* Editd, manage img via mobs.
* Add hex (sha1) to min.
* editd, remove histogram websocket. Calculate histogram in ui as of
  collectiond.
* Update proxy and thumb with any adjustments from editd.
* Zoom in library mode.
* Auto expand top collection.
* In library view, metadata field to right, present proper collection
  name.
* Support for color.
* Filter based on rating and color.
* Move images between collections (rename(2), EXDEV (different file
  sys)).
* Create links from collectiond to editd.
* Load collection in editd.
* Black adjustment is quite simple, pure linear for both +/-. For
  lightening blacks a non-linear method should be used.
* White adjustment is quite simple, pure linear for both +/-. For
  darkening whites a non-linear method should be used.

# Bugs

* Exif parsing, shutter speed greater than 1s
* Downsample yields weird results when scale factor is close to 1.

# Performance:

* Write of jpg.
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
