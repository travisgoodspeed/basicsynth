###########################################################################
# Master makefile for BasicSynth Composer
#
# This is the GUI synthesizer using the BasicSynth sound
# generation and instrument libraries. Three versions are available
#   mswin - MS Windows only, uses ATL/WTL and requires VisualStudio
#   fltk - Uses FLTK framework. Version 1.3.x is preferred.
#   wx - Uses wxWidgets framework. Version 2.9 is preferred.
#
# "make all" makes all modules
# "make [module]" to make one specific example
# "make clean" removes libraries and executable images
# "make new" cleans then rebuilds
#
# The BUILD variable can be set to build a debug version:
#    "make BUILD=debug [target]
#
# To build the fltk or wx versions, you must install the appropriate libraries.
# On Linux, use apt-get or go to:
#      http://fltk.org for FLTK 
#      http://wxwidgets.org for wxWidgets
# The makefiles use fltk-config and wx-config to get the  compile and link flags.
#
# For Windows, use msys/mingw32. Other compilers may work but
# have not been tested.
#
# Installation should be run as root. The basicsynth source directory
# and the target directory are set by variables $BSDIR and $prefix
# in the BasicSynth.cfg file. These can be overriden on the make
# command line (make BSDIR=srcpath prefix=dstpath).
#   "make installwx" to install the wx version
#   "make installfltk" to install the fltk version
#   "make uninstall" to remove the installed verion
# After install, run "bsynthcomposer"
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
BSDIR := $(dir $(shell dirname `pwd`))
include ../BasicSynth.cfg

BSMODULES= \
Core \
wx

formsdir=$(datadir)/Forms
helpdir=$(datadir)/Help
himgdir=$(helpdir)/images
exampdir=$(datadir)/ExampleProjects
formsrc=$(BSDIR)/Src/BSynthComposer/Forms
helpsrc=$(BSDIR)/Src/BSynthComposer/Help
ifeq ($(SYS),Msys)
optfile=$(datadir)/basicsynth.ini
else
optfile=$(datadir)/.basicsynth
endif

.PHONY: all new chkdirs clean $(BSMODULES) installcommon installwx installfltk

all: chkdirs $(BSMODULES)
	@echo BSynthComposer done

# remove old build and rebuild from scratch
new: clean all

# create directory for Core library
chkdirs:
	@test -d Lib || mkdir Lib

# remove compiler output
clean:
	$(MAKE) -C Core $(MAKEFLAGS) BSDIR=$(BSDIR) clean
	$(MAKE) -C fltk $(MAKEFLAGS) BSDIR=$(BSDIR) clean
	$(MAKE) -C wx $(MAKEFLAGS) BSDIR=$(BSDIR) clean

wx fltk: Core

$(BSMODULES):
	-$(MAKE) -C $@ $(MAKEFLAGS) BSDIR=$(BSDIR)

fltk:
	-$(MAKE) -C $@ $(MAKEFLAGS) BSDIR=$(BSDIR)
	
# install targets can be run with prefix=xxx for testing or to install to an alternate directory
# actuall installation should be run as root and BSDIR should be set on the command line:
# make BSDIR=/XXX/basicsynth install{fltk|wx}
installfltk: installcommon
	cp  fltk/bsynthcomposerfltk $(bindir)/bsynthcomposer
	chmod 0755 $(bindir)/bsynthcomposer
	echo HelpFile=$(helpdir)/index.html >> $(optfile)

# this will have problems on windows because the exe looks in 
# the registry for the paths to things like forms and image files.
installwx: installcommon
	cp wx/bsynthcomposerwx$(EXE) $(bindir)/bsynthcomposer$(EXE)
	chmod 0755 $(bindir)/bsynthcomposer$(EXE)
	cp wx/resource.xml $(datadir)/resource.xml
	chmod 0644 $(datadir)/resource.xml
	mkdir -p $(datadir)/res
	cp wx/res/*.png $(datadir)/res
	chmod 0644 $(datadir)/res/*.png
	chmod 0755 $(datadir)/res
	echo HelpFile=$(helpdir)/BSynthHelp.hhp >> $(optfile)

installcommon:
	mkdir --parents $(bindir) $(datarootdir) $(datadir) $(formsdir) $(helpdir) $(himgdir) $(exampdir)
	cp $(formsrc)/*.xml $(formsdir)
	cp $(formsrc)/*.png $(formsdir)
	cp $(formsrc)/*.jpg $(formsdir)
	cp $(helpsrc)/*.html $(helpdir)
	cp $(helpsrc)/*.hh? $(helpdir)
	cp $(helpsrc)/images/*.jpg $(himgdir)
	cp $(BSDIR)/ExampleProjects/TestJig.bsprj $(exampdir)
	cp $(BSDIR)/ExampleProjects/jig.nl $(exampdir)
	cp $(BSDIR)/ExampleProjects/SunflowerSonata.bsprj $(exampdir)
	cp $(BSDIR)/ExampleProjects/sflwr.nl $(exampdir)
	echo Install=$(datarootdir) > $(optfile)
	echo Forms=$(formsdir) >> $(optfile)
	echo Colors=Colors.xml >> $(optfile)
	echo InclNotelist=1 >> $(optfile)
	echo InclSequence=0 >> $(optfile)
	echo InclScripts=0 >> $(optfile)
	echo InclTextFiles=0 >> $(optfile)
	echo InclLibraries=0 >> $(optfile)
	echo InclSoundFonts=1 >> $(optfile)
	echo InclInstruments=ffffffff >> $(optfile)
	echo Latency=0.1 >> $(optfile)
	echo TickRes=0.0005 >> $(optfile)
	echo file1=$(exampdir)/TestJig.bsprj >> $(optfile)
	echo file2=$(exampdir)/SunflowerSonata.bsprj >> $(optfile)
	chmod 0644 $(formsdir)/*.xml
	chmod 0644 $(formsdir)/*.jpg
	chmod 0644 $(formsdir)/*.png
	chmod 0644 $(helpdir)/*.html
	chmod 0644 $(helpdir)/*.hh?
	chmod 0644 $(himgdir)/*.jpg
	chmod 0644 $(exampdir)/*.bsprj
	chmod 0644 $(exampdir)/*.nl
	chmod 0755 $(datadir) $(formsdir) $(helpdir) $(himgdir) 
	chmod 0777 $(exampdir)

uninstall:
	rm -f $(bindir)/bsynthcomposer
	rm -rf $(datadir)
