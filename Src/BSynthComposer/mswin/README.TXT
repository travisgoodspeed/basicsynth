The mswin project implements the native Windows version of BasicSynth Composer.
This is the reference version. Other versions are effectively a "port" of
the windows version.

The mswin project requires VisualStudio 2005 (version 8) or later.
(MSVC v6 will definitely not work, but .NET might, with some adjustments)

It requries the WTL header files and the Scintilla text editor includes.
The mswin.vsprops file contains macros for the WTL and scintilla paths.

WTL 8.0 is included in the basicsynth source. 
The current WTL is available from the microsoft website, or on sourceforge.

Scintilla is available from http://scintilla.org/. The default scintilla
make files do not build a 64-bit version. There are VC project files
under basicsynth\OpenSource\scintilla* that will build both 32-bit and 64-bit
versions if they need to be built. The source distribution of basicsynth
includes prebuilt dlls for scintilla.

wx and fltk projects build BasicSynth Composer using portable frameworks.
See the README in the wx and fltk folders for more information.
