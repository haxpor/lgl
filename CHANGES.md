Keep track of changes for reference in this file.
On-going changes are added as another bullet points in each example 

# `CameraImgui`

* first example integrated with ImGui, and first to implement Gizmo

# `Sphere`

* first geometry example to draw sphere programmatically

# `LineIntersection3D`

* first example implement drawing Sphere programmatically instead of dot, enhanced version from `LineIntersection2d`. Introduced `Sphere.h` and `Sphere.cpp`
* introduced `Gizmo.h`, and `Gizmo.cpp` refactoring from `CameraImgui`
* Line intersection in 3d is much more reliable to use shortest distance calculation between two lines in 3d space. Closed form which derived via cross-product and dot-product are not reliable and not always give the correct result. With the former, we can set how far distance should be for us to regard it as intersected.
* [2019-11-08] Gizmo is fixed and improved in its draw call.
