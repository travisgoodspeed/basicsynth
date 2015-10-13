This is a port of BasicSynth Composer to the FLTK toolkit.

To compile, you will need the FLTK 1.1.10 or 1.3.x libraries (http://www.fltk.org/).
Version 1.3.x is preferred since it has a tree widget. 

In addition, a regex library is needed (e.g., http://www.arglist.com/regex/).

The VS project file is set up to look in OpenSource/fltk-1.3.x for fltk stuff
and OpenSource/regex for regex stuff. The paths are defined as macros in the 
properties file fltk.vsprops.

The Codeblocks project has a global variable to set the FLTK root directory.