This is a version of BasicSynth Composer that uses the wxWidgets toolkit for UI.
It is provided for those who want to compile on a non-windows platform, or using
some compiler other than Visual C++ on windows. As much as possible, the functionality
is the same as the mswin version.

To compile, you will need wxWidgets headers and libraries (http://www.wxwidgets.org/).
Configure with --enable-graphics_ctx, to get the nice gradient fills and anti-alias line graphics, 
and --enable-stc, to get the scintilla editor. (These are usually enabled by default.)

For Visual Studio, the root wx path is defined as a user macro (wxroot) in the properties
file wx.vsprops. The default for wxroot is the OpenSource/wxWidgets-2.9.3 directory.
Compile wxWidgets using the solution files in wx*/build/msw.

The Makefile will work with g++ on Linux and msys on Windows. For other compilers,
you may need to edit the Makefile appropriately.

The Code::Blocks project for Windows has a global variable to set the wxWidgets 
root directory. On Linux, the "wx-config" script sets the command line options and
libraries. When using Code::Blocks on Windows, it is best to use the msys/mingw package
and build the wxWidgets libraries using config then make rather than makefile.gcc.
Don't use the mingw that is installed with Code::Blocks as it does not include
the gdi+ headers and libraries.

The installed packages on Linux for wxWidgets may be a few versions behind, and
may not include the wxGraphicsContext class. It is sometimes necessary to build
from the source distribution to get the right configuration.

On windows, the program uses registry values set during installation to locate various files.
During development, these must be created manually. Create the key
\HKEY_CURRENT_USER\Software\BasicSynthWX and set string values for:
  Install = directory where the exe is located.
  Forms = directory where the forms are located (Src\BSynthComposer\Forms)
  HelpFile = help project file (Src\BSynthComposer\Help\BSynthHelp.hpp)
The first time the program is run, registry entries for other values will be created.
