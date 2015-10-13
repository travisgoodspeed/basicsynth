This directory contains third-party files from open source code libraries that are
needed to build the Utilities and BSynthComposer. 

The scintilla* directories contain VS project files that will build and copy the
Scintilla text edior for BSynthComposer. The distribution for BasicSynth contains
pre-built Scintilla.dll files in the basicsynth Bin and Binx64 directories, along with
.h files here. Source code and up-to-date info is available at the scintilla website
(http://www.scintilla.org).

The WTL80 directory contains the .h files used to build BSynthCompser\mswin and
the projects in the Utilities source directory. Source code is available on
sourceforge under the WTL project (http://sourceforge.net/projects/wtl/).

The wx* directory contains the source code to wxWidgets, if you need to build it.
If you need to rebuild the libraries, download the source and extract it here.
If you already have these on the computer, you can edit the appropriate 
basicsynth project files and point to the location of the .h files and libraries.
Source code and up-to-date info is available on the wxWidgets website.
(http://wxWidgets.org/)

The fltk* directory contains source code for FLTK. 
Source code and up-to-date info is available on the fltk website (http://www.fltk.org/).

Note: the public source repository for BasicSynth does not include all of these files.
They are available elsewhere on sourceforge. The BasicSynth source distribution contains
some of these files -- enough to build BasicSynth.