This directory contains files for automated source code documentation using Doxygen.

Doxyfile.txt - Doxygen project file
Doxyfilew.txt - Doxygen project file with windows paths.
BasicSynthLaout.xml - layout file for BasicSynth documentation

You can edit these files as desired to customize the documentation as you see fit.

Output is produced in the BasicSynth/Doxygen/html directory by default.

There are jpg files in Documents that must be copied to the html output directory manualy:

AddSynthDiag.jpg
ChufferDiag.jpg
FMSynthDiag.jpg
MatSynthDiag.jpg
ToneFMDiag.jpg
ToneDiag.jpg
WFPlayDiag.jpg
gmplayerdiag.jpg
SoundBankDiag.jpg

For Windows users, get the Doxygen program from http://www.doxygen.org
or http://sourceforge.net/projects/doxygen and install it. If you want nice class
diagrams you must also install Graphviz (http://graphviz.org).

For Debian GNU/Linux users, there are apptitude packages for both.

apt-get install doxygen
apt-get install doxygen-doc
apt-get install doxygen-gui
apt-get install graphviz
apt-get install graphviz-doc

