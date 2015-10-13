BasicSynth version 1.5 - Release Notes

BasicSynth is a set of C++ libraries for sound generation and sequencing. 
The source distribution contains the source code to the libraries along with
numerous example programs and a complete multi-instrument synthesizer.

The ZIP file contains sources for all platforms. The CHM file is a 
Windows compiled help file for the library. This can be built from the
source distribution using doxygen.

BasicSynth Composer is a complete synthesis system based on the BasicSynth
sound generation libraries.

This release is focused on bug fixes, unicode support, and improving the 
build scripts.

1. Bug fixes
1.1 SoundFont playback
   Fixed a problem that would cause a crash if the MIDI modulation wheel
   is applied to the vibrato LFO.
1.2 Project files not loading float values
   Floating point values were not converted from strings due to locale settings.
1.3 Mixer configuration crash.
1.4 wxWidgets version of BSynthComposer
  This release fixes a variety of problems with BSynthCompserWX. This is
  still relatively new code, but is stable enough for use now. Also,
  the editor for Notelist now uses the "styled text control" (a/k/a scintilla).

2. Project files and Makefiles
2.1 Some VS project files would not build 64-bit versions.
  Paths and compiler settings were corrected.
2.2 Makefiles
  The makefiles have been modified so that the same
  file works for Linux and mingw on Windows. On Windows,
  run make from the msys shell.
2.3 Code::Blocks projects
  These have been modified to assume wxWidgets on Windows
  is built with mingw running under the msys shell.

 3. Unicode support
 3.1 Project files
  The project files now correctly output UTF-8 characters. This may
  cause some compatibility problems with prior versions. Prior versions
  marked the project files with encoding=UTF-8 but did not correctly 
  convert the text. Any non-ascii text in the project file will need to
  be corrected by bringing up the associtated properties dialog and
  reentering the values.
 3.2. BSynthComposer
  The mswin BSynthComposer supports display and entry of Unicode
  characters. Text files (e.g. Notelist) are assumed to be UTF-8.
  Notelist scores can now contain unicode characters in variable names,
  comments, and text strings.
 
 4. The FLTK version of BSynthComposer
  This is now deprecated since the wxWidgets version provides a 
  portable version that is easier to maintain with a better UI.
  The code is still included, but will not be updated or fixed.
