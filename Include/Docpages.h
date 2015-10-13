/// @file Docpages.h Documentation
/// This file is used to create documentation pages with Doxygen and does not need to be included in compiles.

/*! \mainpage BasicSynth Synthesizer Library

\section secIntro Introduction


<p>The <em>BasicSynth</em> source is provided to support the book <em>BasicSynth: Creating a Music Synthesizer in Software</em>.
(see <a href="http://basicsynth.com">http://basicsynth.com</a>.) The book provides detailed explanations of the signal processing functions
required to generate and process sounds, and shows the derivation of the programs using the C++ language.
Although the book is not required to use and understand the source, it should be read first if you do not
have any experience with signal processing programming.</p>

<p>The <em>BasicSynth</em> source distribution contains the following parts.</p>
<ul>
<li>Common library - signal generation and processing library.</li>
<li>Instruments library - various synthesis instruments.</li>
<li>Notelist - a text-based music score language.</li>
<li>Examples and utilities - working programs that use the libraries.</li>
<li>BSynthComposer - a complete software synthesis program.</li>
</ul>


<h3>Common library</h3>

<p>The <em>BasicSynth</em> Common library  provides the signal generation 
and processing foundation for a software synthesizer. Much of the implementation 
is contained in the header files located in the Include directory. 
The file <em>BasicSynth.h</em> includes all of the header files, or individual files 
can be included as needed. The methods are typically short, and placing them
in the header allows the compiler to optimally expand the code as inline functions. 
Longer functions, and those that are not time critical, are located in the 
Src/Common directory and compiled into the Common library as object modules.</p>

<p>To build your own synthesis program with <em>BasicSynth</em> you need at a
minimum the Include files and the Common library (<em>Lib/Common.lib</em>). The Include
directory and the Common library provide the sound generation, wave file and
sequencer code. The header file <em>BasicSynth.h</em> includes all of the library.</p>

<p>Note: Libraries for GCC (including MINGW) have the <em>.a</em> extension instead of <em>.lib</em></p>

<h3>Instruments</h3>

<p>A set of example instruments built using the library are located in Src/Instruments.
The instruments are intended for use with BSynthComposer (see below) but can also
be used for other software synthesis systems.</p>

<p>If you want to use the <em>BasicSynth</em> instruments collection in your
program, you must also include the header file <em>Src/Instruments/Instruments.h</em> and link 
the <em>Lib/Instruments.lib</em> library.</p>

<h3>Notelist</h3>

<p>Notelist is a musician friendly, text-based music score format. It allows simple representation of notes, and
also includes programming functions to calculate note values. Notelist allows specifying synthesis
parameters on a note-by-note basis. This provides complete control of the sound, not simply note-on
and note-off specification.</p>

<p>To include the Notelist score processor in your program, you must include the header file
<em>NLConvert.h</em> from the <em>Src/Notelist</em> directory and link the <em>Lib/Notelist.lib</em>
library.</p>

<h3>Examples</h3>

<p>The source distribution includes numerous example programs that show usage of the library. 
The exmple programs are executed from the command line and are portable to many
platforms. Some examples are under the directory Src/Utilties. These are specific
to MS-WINDOWS.</p>

<h3>BasicSynth Composer</h3>

<p>The BasicSynth source is intended to provide examples to someone who wants to 
create a personalized software synthesizer, and should not be treated as a completed
synthesis project. However, a complete synthesizer (BSynthComposer) designed by the
author for his own use is also included. This can be compiled and run as-is, but is also
intended as a reference for how the BasicSynth libraries can be used as the foundation
for your own synthesizer.</p>

<p>Three versions of BSynthComposer are available.</p>
<ul>
<li>Windows - uses WTL for the GUI framework, GDI+ for graphics, DirectSound for audio.</li>
<li>FLTK - uses the FLTK for the GUI framework.</li>
<li>WX - uses wxWidgets for the GUI framework.</li>
</ul>
<p>The MSWIN version only compiles with Visual Studio and only runs on Windows. The FLTK and WX versions
can be compiled with the GNU or MinGW compilers and will run on Windows or Linux.</p>

<p>Several compositions for BSynthComposer are located in the ExampleProjects directory.</p>

<h3>Documentation</h3>
<p>Reference documentation on the libraries is  built with Doxygen. Use the files in the <em>Doxygen</em> directory
to build the documentation .html files. In addition, documentation in Windows help file format can be downloaded from
the BasicSynth project on sourceforge.</p>

\section secDirs Directory Structure

\image html Structure.jpg

<table border="1" cellpadding="4" cellspacing="0" style="margin-left:0.25;">
  <tr style="background:#888;color:#fff;">
   <td >
   <p>Directory</p>
   </td>
   <td >
   <p>Usage</p>
   </td>
  </tr>
 <tr>
  <td >
  <p>Bin</p>
  </td>
  <td >
  <p>Executable images</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Lib</p>
  </td>
  <td >
  <p>Object file libraries</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Binx64</p>
  </td>
  <td >
  <p>64-bit Executable images</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Libx64</p>
  </td>
  <td >
  <p>64-bit Object file libraries</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Include</p>
  </td>
  <td >
  <p>Source include files. Much of the <em>BasicSynth</em>
  sound generation code is contained in these files as inline class functions.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Documents</p>
  </td>
  <td >
  <p>Various documents.</p>
  </td>
 </tr>
  <tr>
  <td >
  <p>Doxygen</p>
  </td>
  <td >
  <p>Configuration file to produce source code documentation with Doxygen.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>OpenSource</p>
  </td>
  <td >
  <p>Open source libraries and include files. These are only used to build of BSynthComposer.
  These are not needed to build the libraries and examples. However, the Windows utilities
  require WTL 8.0.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>OpenSource/scintilla*</p>
  </td>
  <td >
  <p>Header files and VS 2008 project files to build scintilla for BSynthComposer.
  By default, the BSynthComposer looks here for the header files.
  If scintilla is installed already, change the project file for BSynthComposer to reference the include directory.
</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>OpenSource/WTL80</p>
  </td>
  <td >
  <p>WTL files used for BSynthCompser.
  By default, the BSynthComposer looks here for the header files.
  If WTL is installed already, change the project file for BSynthComposer to reference the include directory.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>OpenSource/fltk-*</p>
  </td>
  <td >
  <p>FLTK files used for BSynthCompser. (These are not needed to build the libraries and examples.)</p>
  <p><em>NOTE: FLTK is no longer maintained and will eventually be removed. Use the wxWidgets
  version for portability.</em></p>
  </td>
 </tr>
 <tr>
  <td >
  <p>OpenSource/wxWidgets-*</p>
  </td>
  <td >
  <p>wxWidget files used for BSynthCompser. (These are not needed to build the libraries and examples.)
  The current configuration uses version 2.9.3.  Other versions may work, but have not been tested. 
  See the README.TXT file in Src/BSynthComposer/wx for more information.
  </p>
  </td>
 </tr>
<tr>
  <td >
  <p>Src</p>
  </td>
  <td >
  <p>Source code</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/Common</p>
  </td>
  <td >
  <p>Common source, mostly implementation of sequencers and
  file I/O</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/Instruments</p>
  </td>
  <td >
  <p>Source to the <em>BasicSynth</em> instruments collection
  library.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/Notelist</p>
  </td>
  <td >
  <p>Source to the notelist parser and interpreter library.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/Examples</p>
  </td>
  <td >
  <p>Source to the example programs. Each sub-directory
  contains one example program. The theory and algorithm of each program is explained
  in the associated book chapter.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/Utilities</p>
  </td>
  <td >
  <p>Source to the utility programs. Each sub-directory
  contains one utility program. These are Windows-only GUI programs that use
  the <em>BasicSynth</em> library and demonstrate various synthesis techniques.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynth</p>
  </td>
  <td >
  <p>Command line version of the synthesizer.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/GMSynth</p>
  </td>
  <td >
  <p>Plug-in synthesizer DLL using MIDI and SF2/DLS files.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer</p>
  </td>
  <td >
  <p>A complete synthesizer with a GUI interface. This is one way of using the
  libraries to create a personalized software synthesizer. It is similar in
  concept to the <em>MUSIC N</em> systems (e.g. CSound) but with a more readable score
  languague, some prebuilt instruments, and interactive instrument configuration.
  </p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/Core</p>
  </td>
  <td >
  <p>Platform independent parts.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/mswin</p>
  </td>
  <td >
  <p>MS-Windows specific version using ATL/WTL.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/Forms</p>
  </td>
  <td >
  <p>XML files defining editor forms.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/Help</p>
  </td>
  <td >
  <p>Help files in HTML format and compiled help project for MS-Windows.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/fltk</p>
  </td>
  <td >
  <p>FLTK specific code.</p>
  </td>
 </tr>
 <tr>
  <td >
  <p>Src/BSynthComposer/wx</p>
  </td>
  <td >
  <p>wxWidgets specific code.</p>
  </td>
 </tr>
</table>

<p>To build your own synthesis program with <em>BasicSynth</em> you need at a
minimum the Include files and the Common library (<em>Lib/Common.lib</em>). The Include
directory and the Common library provide the sound generation, wave file and
sequencer code. The header file <em>BasicSynth.h</em> includes all of the library.</p>

<p>If you want to use the <em>BasicSynth</em> instruments collection in your
program, you must also include the header file <em>Src/Instruments/Instruments.h</em> and link 
the <em>Lib/Instruments.lib</em> library.</p>

<p>To include the Notelist score processor, you must include the header file
<em>NLConvert.h</em> from the <em>Src/Notelist</em> directory and link the <em>Lib/Notelist.lib</em>
library.</p>

<p>Note: Libraries for GCC (including MINGW) have the <em>.a</em> extension instead of <em>.lib</em></p>

<p>Reference documentation on the libraries is  built with Doxygen. Use the files in the <em>Doxygen</em> directory
to build the documentation .html files. In addition, documentation in Windows help file format can be downloaded from
the BasicSynth project on sourceforge.</p>

<h3> Building the Software</h3>

<p><em>Note:</em> The author's target development environment for <em>BasicSynth</em> is Windows using Visual Studio 2008.
Consequently, the build procedure receives much more use and testing on Windows than other environments. 
Compiling with makefiles or Code:Blocks, GNU C++, MINGW, and building
for Linux, may require some adjustments to make them work correctly on your computer.</p>

<hr size=2 width="100%" align="center">

<h3>Building on MS-Windows with Visual Studio</h3>

<p>There are several solution files that can be built depending on what you want to do. 
Default solution files are for Visual Studio 2008 (i.e. version 9). Solution files with 2005
in the name are for VisualStudio 2005 (version 8). Visual Studio 6 project files (.dsp) are
also included. However, the older project files are not always kept up to date and you may
need to modify them to produce working versions on  the latest code.
The VirtualKBD utility and BSynthComposer will not build with version 6 as they need the gdi+ libraries. 
If you have a different version of the compiler, you will have to produce the project and/or solution files from one
of the supplied versions.
</p>
<p>Visual Studio 2010 has not been tested, but should be able to automatically convert the solution and project files.
Visual C++ Express edition(s) do not have ATL, and will not load project files with 64-bit targets. They will 
probably work, but will require some modifications to the project files.
</p>

<p>Each Visual Studio project defines four targets:</p>

<ol>
<li>Release Win32 - 32-bit version without debug info.</li>
<li>Debug Win32 - 32-bit version with debugging.</li>
<li>Release x64 - 64-bit version without debug info.</li>
<li>Debug x64 - 64-bit version with debug info.</p>
</ol>

<p>Output of the 32-bit release version is in Bin and Lib. The 64-bit version produces output in Bin64 and Lib64</p>

<p>To build everything, use the solution file <em>Src/BasicSynth.sln</em></p>

<p>To build just the libraries, use the solution files in <em>Src/Common,
Src/Instruments, and Src/Notelist</em> directories.</p>

<p>To build the Example programs, use the solution file in <em>Src/Examples</em>. This
builds all the example programs and libraries.</p>

<p>To build the utilities, use the solution file in <em>Src/Utilities</em>.</p>

<p>To build the stand-alone command line synthesizer, build the project in <em>Src/BSynth</em>.</p>

<p>To build the GUI synthesizer, build the project in Src/BSynthComposer</p>

<p>The utility programs and VC version of <em>BSynthComposer</em> use WTL 8.0, available on sourceforge.net.
The parts used by <em>BasicSynth</em> are located in the <em>OpenSource/WTL80</em> directory.
<em>BSynthComposer</em> requires the <em>scintilla</em> header files and library DLL, 
also available on sourceforge.net. The parts used by <em>BasicSynth</em> are located in the 
<em>OpenSource/scintilla</em> directory. Pre-built DLLs are installed in the <em>Bin</em>
and <em>Binx64</em> directories.</p>

<p>The fltk version of <em>BSynthComposer</em> requires the FLTK header and library
files. Source code is available from the <a href="http://www.fltk.org">FLTK website.</a>
(Note that the FLTK version is a "dead end" and will not be maintained.)
</p>

<p>The wx version of <em>BSynthComposer</em> requires the wxWidgets header and library
files. Source code is available from the <a href="http://www.wxwidgets.org">wxWidgets website.</a>
See the README.TXT in the wx directory for more information.
</p>

<hr size=2 width="100%" align="center">

<h3>Building with Code::Blocks</h3>

<p>There are two Code::Blocks workspace files. In the Src directory,
the <em>basicsynth.workspace</em> file will build the libraries and example programs. 
In the Src/BSynthComposer directory the <em>bsynthcomp.workspace</em> file will build the
synthesizer program using wxWidgets. </p>

<p>Projects contain four targets:</p>
<ol>
<li>Release Win32 - 32-bit Windows version without debug info.</li>
<li>Debug Win32 - 32-bit Windows version with debugging.</li>
<li>Release UNIX - Linux version without debug info.</li>
<li>Debug UNIX - Linux with debug info.</p>
</ol>

<p>There is a .cbp project file for each of the libraries, example programs, FLTK and wx versions
of BasicSynth Composer. (The Utilities and MSWIN version of BasicSynth Composer use
ATL and must be compiled with VisualStudio, so no .cbp files are provided.) </p>

<p>A global variable <em>bshome</em> defines the location of  include and library files. The first time
you build the basicsynth workspace, Code::Blocks will prompt for the needed value.
You can also set this in advance from the <i>Settings:Global Variables</i> menu.

The bshome value is the top directory where BasicSynth is installed (i.e., the <i>basicsynth</i> directory).
It's best to put the source in a path without spaces since the MINGW compiler and linker
will have problems otherwise. However, if the path has spaces in it, there are ways
around the problem. For Windows, you can create a network share name for the root folder and 
use the UNC path to access it. (e.g. \\\\MYCOMPUTER\\basicsynth). </p>

<p>When compiling on Linux the projects use the appropriate config file to determine the
path for include and library files for wxWidgets.</p>

<hr size=2 width="100%" align="center">

<h3>Building with Makefiles</h3>

<p>There is a master Makefile located in the Src directory. This will build the libraries and example programs. 
The file <em>BasicSynth.cfg</em> in the Src directory contains the settings for the compiler and linker.
The BSDIR variable defines the root path for the source. It is set using a shell pwd command. 
If this does not work, set the BSDIR value on the make command line.</p>
<p>There is also a Makefile in each directory that can be used if you only want to build part of the system. However, you
must set the BSDIR variable on the make command line.</p>

<p>Output is to the directories Bin and Lib under the BasicSynth install
directory. (Note the capitalization!)</p>

<p>There is a Makefile in <em>BSynthComposer</em> that will build BasicSynth Composer. There are install targets,
installwx and installfltk, to install the binaries. You must <em>su</em> to root for the install make.</p>

<p><em>Note:</em> I have noticed
a problem with the GNU make that causes make to halt when descending three
directory levels. Running make with the command line argument
<em>--no-print-directory</em> fixes this.</p>

<hr size=2 width="100%" align="center">

<h3>Building with MINGW</h3>

<p>The makefiles will work with MINGW on Windows if you also install MSYS and run make from the msys shell.</p>

<p>Compiling BSynthComposer with mingw requires using a version of mingw that includes gdi+ headers and libraries.
Although there is a makefile.gcc available for wxWidgets, it does not automatically use the
high quality graphics classes available when wxUSE_GRAPHICS_CONTEXT is enabled. Also, that makefile
produces libraries with names different from the ones produced with the configure script. You can edit the
setup.h file, and alter the library paths, but <em>BasicSynth</em> will be easier to build if you build everything under the msys shell.
Build wxWidgets by running configure, make, then "make install" in the wxWidgets directory.</p>

<hr size=2 width="100%" align="center">

\section libdoc Building the library documentation

There is a Doxygen project file in the Doxygen directory that can be used to generate
a new copy of this documentation if you modify the library. Check the paths to insure
they point to the correct locations. The instrument diagram files in the Documents 
directory must be manually copied to the output directory indicated in the doxygen project file.

<hr size=2 width="100%" align="center">

\section license License

The source code to BasicSynth is &copy; 2008-2012, Daniel R. Mitchell and is licensed under the
Creative Commons/GNU-GPL (http://creativecommons.org/licenses/GPL/2.0/). Source code may be
used for non-commercial purposes without further restrictions.

Compositions in the ExampleProjects directory are &copy; Daniel R. Mitchell and are licensed
under the Creative Commons Attribution-Noncommercial-No Derivitave Works license. 
(http://creativecommons.org/licenses/by-nc-nd/3.0/us/</a>)

\page architecture BasicSynth Archtecture

 The BasicSynth library can be used in a variety of contexts with as little or as much
 of the library as needed. However, the library is specifically designed to support
 the architecture used by the BSynth program.

\section archover Overview

BasicSynth is a component-based system intended to be used as a compositional environment. Thus it centers around
the sequencing of a series of events defined by the score and allows a variable set of 
instrument definitions. The sequencer reads the score and invokes one or more synthesis 
instruments to generate samples. Samples are passed through a common mixer and then to the 
output, either a wave file or a sound output device (DAC). The project information and 
parameters component contains general synthesizer information and also allows the sequencer 
to discover the available instruments and scores that are to be used. 

\image html architecture2.jpg

Because it is a component architecture, it is possible to replace any of the components in the 
system with custom versions. So long as the interface to the component is maintained, the
remainder of the system will work without modification.

Since all configuration files are in XML format, a simple text editor may be used to configure the system. 
The \e BasicSynth Composer program implements GUI editors for each part of the system.

<table border="1" cellspacing="0">
<tr style="background-color:black;color:white;"><td>Module</td><td>Description</td></tr>
<tr><td>Sequence</td><td>Generates sample output based on a sequence of events. Events can be read from a file, added directly by a program, or added interactively through a user interface.</td></tr>
<tr><td>Seq. Edit</td><td>Editor (or generator) for sequencer files. Multiple formats can be defined by adding the appropriate file loading code to the sequencer.</td></tr>
<tr><td>Instr.</td><td>An Instrument, or sound generation and modification module. Instruments use generator, filter and effects objects to control sound generation.</td></tr>
<tr><td>Instr. Lib</td><td>File containing instrument configuration.</td></tr>
<tr><td>Instr. Edit</td><td>Editor for instrument definitions. This can be a simple text editor or a complex graphic interface.</td></tr>
<tr><td>Wave Buffer</td><td>Buffer for sample output.</td></tr>
<tr><td>Wave File</td><td>Output to a sound file, usually in WAV format, but possibly in other formats as well.</td></tr>
<tr><td>Play</td><td>Immediate output of samples to the computer's sound output device.</td></tr>
<tr><td>Mixer</td><td>Mixes sounds and applies effects.</td></tr>
<tr><td>Effects</td><td>Processing effects such as reverb, echo, flanger, etc.</td></tr>
<tr><td>Params</td><td>System wide parameters, such as sample rate, wave table length, input and output directories, etc., along with lists of instrument and sequencer files</td></tr>
<tr><td>Prjoject Edit</td><td>Editor for project parameters.</td></tr>
</table>

\section sequencer Sequencer
The sequencer is the synthesizer component that cycles through events, dispatching
each event in the proper order and at the proper time. Originally a sequencer was
a module on an analog synthesizer that could step through a series of control voltages
and produce a trigger at each step. When applied to an oscillator and envelope generator
in combination, the sequencer played a short tune. The addition of microprocessor control
allowed the sequencer to store any series of synthesizer control parameters and playback
the control signals automatically. In a software synthesis system, the sequencer is expanded
to provide the ability to automate timed control of any synthesizer function.

\image html sequencer.jpg

The Sequence File Loader implements functions to read a list of events and event parameters from a file. 
Each event (SeqEvent) includes a unique identifier that indicates which instrument will process the event. 
As each event is loaded, the loader calls the Instrument Manager to instantiate an Event object specific to the target instrument. 
The Instrument Manager uses the instrument identifier to locate information about the instrument in the Instrument Definition Table. 
The Instrument Manager then calls a method on the Instrument interface to instantiate the event object. 
Parameters from the file are added to the event object by calling the set parameter method on the event object. 
The Event object is then added to the Sequencer Loop track event list.

The Sequencer maintains a group of Track objects (SeqTrack). Each Track represents a time ordered list of sequencer events.
Track zero is the master track that is started when the Sequencer is started. Other tracks can be started
and stopped indpendently of the main track. The track is determined by a value in the event object.

Events can also be passed directly to the Sequencer for immediate execution. These events typically come
from a MIDI keyboard, but can be generated from any source that has access to the sequencer. Immediate
events do not have a start time, and are removed from the event list as soon as they are played.

The Instrument Definition Table contains entries to identify each instrument in the synthesizer. 
This list can be dynamically created when the synthesizer is loaded, or can be compiled into the program. 
Typically, the table is loaded from a file containing instrument types and default settings. 
The table contains the address of a factory for each instrument and event type.
The factory is the code that "manufactures" (i.e., instantiates) the instrument or event, and is defined as a part
of the instrument object implementation. By storing the information in a table created at runtime, the synthesizer
can have access to an unlimited number of instrument definitions.

Once the sequence is loaded, playback is started by calling a method on the Sequencer Loop. 
The Sequencer Loop calls the start method on the Instrument Manager. This allows the Instrument Manager to 
pre-allocate instruments if needed and also to initialize the Mixer and Wave File outputs. 
The Sequencer Loop scans the list of events associated with each Track until the start time of an event is reached. 
The Sequencer Loop then invokes the Instrument Manager to allocate an instance of the instrument identified by the
Event object and receives back a reference to the Instrument interface, which is stored in the Active Event list. 
The Event is then passed to the instrument using the start method. 

The instrument associated with the event is called for each sample time until the duration of the event is completed. 
The Sequencer Loop then calls the instrument stop method to signal the instrument to end. 
However, the instrument remains active, and the tick method continues to be called, so long as the \e isfinished method returns false. 
This allows the instrument to produce samples beyond the limit of its event duration if necessary. 
For example, the instrument can wait to begin the release portion of the envelope until a stop signal is received, 
and processors such as delay lines can empty the delay line before terminating. When the instrument indicates it has finished, 
the Sequencer Loop removes the instrument from the active list and notifies the Instrument Manager that the instrument is no 
longer in use by invoking the deallocate method. The Instrument Manager can then either destroy the instrument instance, or recycle it if possible.

When the tick method is called on an instrument, the instrument produces a sample and passes it to the Instrument Manager 
which then adds the sample to the appropriate Mixer input. However, an instrument does not need to produce samples. 
An instrument can also be defined to control other synthesizer functions, such as panning, mixer levels, etc. 
When all active instruments have been processed, the Sequencer Loop calls the tick method on the Instrument Manager. 
The output of the Mixer is then retrieved and passed to the Wave File output buffer.

Once all events have been completed, the stop method on the Instrument Manager is called and the Sequencer Loop stops.

This design allows for nearly unlimited flexibility in the sequencer. We only need a few event types and can sequence any number of instruments, 
processors and controllers without having to build the knowledge of the instruments into the sequencer. Any synthesizer function that can be 
performed with the stop, start, change, and tick signals can be sequenced. The key to this design lies in the flexibility of the event object, 
the dynamic allocation of instrument instances, and the abstract instrument interface.

\page pgbsynth BSynth synthesizer

\htmlinclude BSynth.html

\page instrDoc BasicSynth Instruments

\htmlinclude Instruments.html

\page notelist Notelist Score Script

\htmlinclude Notelist.html

\defgroup grpGeneral General Classes

\defgroup grpIO File I/O Classes

\defgroup grpOscil Oscillators

\defgroup grpNoise Noise Generators

\defgroup grpEnv Envelope Generators

\defgroup grpFilter Filters

\defgroup grpDelay Delay Lines

\defgroup grpMix Mixing and Effects

\defgroup grpSeq Sequencing

\defgroup grpMIDI MIDI specific classes

\defgroup grpInstrument Synthesis Instrument Classes

\defgroup grpSoundbank SoundFont(R) and DLS File Classes

*/

