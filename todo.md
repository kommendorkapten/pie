# High prio
* MOB id in url when navigating
* mark image for deletion with 'x'
* Make sure date format is ISO 8601 in export view
* Crop.
* Multi select in UI.

# Sorted
* After MOB is deleted/moved update collection tree (break out to new
  class).
* Editd, when zoomed in histogram is updated based on zoomed image.
* Auto expand coll from query param in coll view.
* Allow creation of new collections from web ui.
* collectiond should update mobs via mediad.
* When navigating from dev to coll, expand list and show last state.
* Load collection in editd.

# Unsorted

## Editd
* empty workspace after x min of activity.
* prefetch files to workspace.
* zoom/pan in dev, fix bug with pixmap not being reload from ws before
  drag starts, causes reset of drag.
* presets.
* "time line" with pictures in bottom with selected collection. Both
  dev and coll view.

## Ingestd
* handle file name collisions during import.
* Verify file integrity during import.

## Collectiond
* use cache for thumbnails and proxy images.
* Smarter management of collections, only load visible thumbnails.

## Mediad

## Image
* raw post processing (chroma noise in particular) (tojpg contains ideas).
* Calculate new radius for clarity when an image is loaded. The one
  stored in the database may be for the wrong resolution.
* Black adjustment is quite simple, pure linear for both +/-. For
  lightening blacks a non-linear method should be used.
* White adjustment is quite simple, pure linear for both +/-. For
  darkening whites a non-linear method should be used.
* modify alg_contr to use same function call parameters as rest.
  extend with per alg struct to only allow a single val to be provided.

## General
* Refactor new_media.c
* Add method to load file + params via mob (used in editd.c and new_mediad.c)
* Allow config path to be set during compile time.
* Add method to get fd from q_consumser.
* better javascript management for color tag dropdown. Remove global
  js func.
* Timestamp to log messages
* Remove CORS headers from pie_http_lws_write, replace with hmap for
  custom headers.
* libraw to use lcms on sparc.
* Exif parsing, shutter speed greater than 1s
* virtual copy of a mob.
* Proper transfer flow, ingest first create, then copy. Send to mediad
  d, extract info and send to export for thumb/proxy generation.
* Bench only sending delta/pixel over websocket.
* Copy development settings to a different mob.
* Presets.

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
* Downsample with scale factor ~1.2-1.5 still yields bad results.

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
* ~~editd, remove histogram websocket. Calculate histogram in ui as of
  collectiond.~~
* ~~editd, loading an image should respect the orientation.~~
* ~~zoom/pan in development view~~
* ~~Thumbnail in development view, that shows zoomed part of image (navigation).~~
* ~~Auto expand top collection.~~
* ~~save filter param in url, useful when switch from dev -> coll.~~
* ~~Images created via libraw has blownout highlights.~~
* ~~add medfilt (3x3).~~
* ~~add curves.~~
* ~~Downsample yields weird results when scale factor is close to 1.~~
* ~~Break out histogram calculation to a separate event~~
* ~~add size to min~~
* ~~Delete mob from collection.~~
* ~~Move exisiting mob to collection.~~
* ~~Delete MOB(s) from UI.~~
* ~~Export picture (exportd)~~
* ~~Write install/config script.~~
* ~~Make sure image is in linear space before doing modifications.~~
* ~~Fix broken sharpening adjustments in ui.~~
* ~~update thumb nail & proxy with dev settings.~~
* ~~Rewrite mediad to use poll(2) when reading from queues.~~
* ~~Move MOB(s) from UI.~~
