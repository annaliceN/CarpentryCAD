# Carpentry CAD Software
=======================

This is a CAD software that can help people customise or reconfigure their furnitures easily.

## Dependency

[Qt library](http://www.qt.io/): Graphical user interface, ver. 5.11

[OpenCASCADE](https://www.opencascade.com/): An object-oriented C++ library for CAD/CAM/CAE, ver. 7.3.0

Acknowledge:
During the development of this software, we use some codes from [FreeCAD](https://github.com/freeCAD/FreeCAD) and [OpenCASCADE Sketcher](http://www.laduga.com/software/occsketcher/index.html).

WARNING: This repository is still under development, please use it with cautions. Any related feedback is welcome!

## TODO List

### Jan. 4th

- [x] Understand onChanged mechanism
- [x] Understand mustExecute mechanism
- [x] Implement drill and cut operations
- [x] Implement HELM feedback mechanism

### Jan. 10th: References

- [x] For drill, you need two faces not parallel to each other, distance to these.
- [x] Start an edge -> end an edge, move each points, attach to edges.
- [ ] Internal cuts, an initial point reference, automatically change it.
- [x] Cut-through: For the polycut, priority.
- [x] Cool for this tool: List a bunch of tools.