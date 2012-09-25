ofxMSAFluid
=====================================

Introduction
------------
C++ openFrameworks addon for solving and drawing 2D fluid systems based on Navier-Stokes equations and Jos Stam's paper "Real-Time Fluid Dynamics for Games" [http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf](http://www.dgp.toronto.edu/people/stam/reality/Research/pdf/GDC03.pdf)

Demo at [www.memo.tv/msafluid/](http://www.memo.tv/msafluid)

Other useful resources and implementations I looked at while building this library:  

- Mike Ash (C), http://mikeash.com/?page=pyblog/fluid-simulation-for-dummies.html
- Alexander McKenzie (Java), http://www.multires.caltech.edu/teaching/demos/java/stablefluids.htm
- Pierluigi Pesenti (AS3 port of Alexander's), http://blog.oaxoa.com/2008/01/21/actionscript-3-fluids-simulation/
- Gustav Taxen (C), http://www.nada.kth.se/~gustavt/fluids/
- Dave Wallin (C++), http://nuigroup.com/touchlib/ (uses portions from Gustav's)

Licence
-------
The code in this repository is available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).  
Copyright (c) 2008-2012 Memo Akten, [www.memo.tv](http://www.memo.tv)  
The Mega Super Awesome Visuals Company


Installation
------------
Copy to your openFrameworks/addons folder.

Dependencies
------------
- MSACore

Compatibility
------------
openFrameworks 0072  
I am generally testing only with [openFrameworks](www.openframeworks.cc), however it should work with [Cinder](www.libcinder.org) too. If it doesn't, please file an issue.


Known issues
------------
Probably will not work with Cinder without some (minor) changes

Version history
------------
### v2.1    23/09/2012
- compatible with OF0072
- renamed (uppercase) MSA namespace to (lowercase) msa. (kept MSA as an alias for backwards compatibility)
- all classes are now inside a new namespace 'msa::fluid::'

### v2.0
- move to centralized MSALibs (requires MSACore)
- everything is msa:: namespace
- u[] and v[] condensed to (Vec2f uv[])
- r[], g[], b[] condensed to (Vec3f color[])
- unified API for getting and setting info:
   - all vel & colors set and get with the structs
   - all getters and setters have 3 functions, index, (i, j), Vec2f pos

### v1.2	02/05/2009
- unified API with processing.org version
- solver u, v, r, g, b arrays now public
- drawer can incDrawMode and decDrawMode
- loads of optimizations by Maa (http://www.lagraine.com/ - new content coming soon)

### v1.1	07/04/2009
- changed license to revised BSD (a lot more more permissive than GPL)

### v1.0
- added RGB or monochrome functionality (enableRGB())
- vector drawing implemented
- get and set info much improved
- added draw mode system
- setup() now only takes dimensions, other parameters have their own setters

### v0.9	04/12/08
- initial version



