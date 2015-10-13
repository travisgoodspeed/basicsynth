include ../../BasicSynth.cfg

# mingw/msys variants
# mingw means running under windows command line
ifeq ($(SYS),mingw)
# macros to set these manually
FLTKBASE=$(BSDIR)/OpenSource/fltk-1.3.x
FLTKINC=-I$(FLTKBASE)
FLTKLIB=-L$(FLTKBASE)/lib -lfltk_images -lfltk_jpeg -lfltk_png -lfltk_z -fltk
ALSA=
WINLIBS=-lwinmm
else
# macros to use installed FLTK
# if fltk-config doesn't work, set the values manually (see above)
FLTKINC=$(shell fltk-config --cxxflags --use-images)
FLTKLIB=$(shell fltk-config --ldflags --use-images)
ifeq ($(SYS),Msys)
ALSA=
WINLIBS=-lwinmm
else
ALSA=$(shell pkg-config --libs alsa)
WINLIBS=
endif
endif

CPPFLAGS+=-I$(BSDIR)/Src/Notelist -I../Core -DPOSIX=1 $(FLTKINC)
CORELIB=../Lib/$(LIBPREFIX)Core$(LIBSUFFIX).a
EXENAME=bsynthcomposerfltk$(EXE)

SRCS=\
main.cpp \
FindReplDlgFltk.cpp \
FormEditorFltk.cpp \
KbdGenDlg2.cpp \
MainFrm.cpp \
MainMenu.cpp \
OptionsDlg.cpp \
ProjectTreeFltk.cpp \
PropertiesDlgFltk.cpp \
SoundBankSelectFltk.cpp \
SynthWidgetFltk.cpp \
TabsView.cpp \
TextEditorFltk.cpp \
WvtableSelectDlg.cpp

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS)) $(ODIR)/ProjectOptionsFile.o

ifeq ($(SYS),mingw)
BSCOMP=$(BSDIR)/Src/BSynthComposer
BUILDDIR=$(BSCOMP)/fltk
DEVINIT=basicsynth.ini
else
ifeq ($(SYS),Msys)
# Msys returns /c/path for c:/path
# the values written into the .ini file must be real Windows paths!
PWDDIR:=$(shell pwd)
BUILDDIR:=$(subst /c,C:,$(dir $(PWDDIR)))
BSCOMP:=$(subst /c,C:,$(dir $(PWDDIR)))
DEVINIT=basicsynth.ini
else
BUILDDIR:=$(shell pwd)
BSCOMP:=$(dir $(BUILDDIR))
DEVINIT=$(HOME)/.basicsynth
endif
endif

.PHONY: all new chkdirs clean

all: chkdirs globinc.h.gch $(EXENAME)  $(DEVINIT)

new: clean all

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)

$(EXENAME): $(OBJS) $(CMNLIB) $(NLLIB) $(INSTLIB) $(CORELIB)
	@echo Linking $@
	@$(CPP) $(XMLLIB) $(CPPFLAGS) -o $@ $(OBJS) \
		$(CORELIB) $(NLLIB) $(INSTLIB) $(CMNLIB) \
		$(FLTKLIB)  $(ALSA) $(WINLIBS) -lregex -lm 

# create an initial options file that points to the development directories.
# Note: On Windows, copy to the Windows user directory manually.
# When running the exe from the build directory, you can leave the file where it is.
$(DEVINIT):
	echo Install=$(BSCOMP) > $(DEVINIT)
	echo Forms=$(BSCOMP)Forms >> $(DEVINIT)
	echo HelpFile=$(BSCOMP)/Help/index.html >> $(DEVINIT)
	echo Colors=Colors.xml >> $(DEVINIT)

clean:
	-rm -f globinc.h.gch $(OBJS) $(EXENAME)

FormEditorFltk.cpp: globinc.h FormEditorFltk.h

KbdGenDlg2.cpp: globinc.h KbdGenDlg2.h

MainFrm.cpp: globinc.h \
FormEditorFltk.h \
TextEditorFltk.h \
PropertiesDlgFltk.h \
OptionsDlg.h \
MainMenu.h \
ProjectTreeFltk.h \
MainFrm.h

main.cpp: globinc.h 

MainMenu.cpp: globinc.h MainMenu.h ProjectTreeFltk.h

OptionsDlg.cpp: globinc.h OptionsDlg.h

ProjectTreeFltk.cpp: globinc.h ProjectTreeFltk.h

PropertiesDlgFltk.cpp: globinc.h PropertiesDlgFltk.h

SoundBankSelectFltk.cpp: globinc.h SoundBankSelectFltk.h

SynthWidgetFltk.cpp: globinc.h

TabsView.cpp: globinc.h TextEditorFltk.h

WvtableSelectDlg.cpp: globinc.h WvtableSelectDlg.h

$(ODIR)/ProjectOptionsFile.o: ../Core/ProjectOptionsFile.cpp
	@$(CPP) -c $(CPPFLAGS) -o $@ $<

globinc.h: ../Core/ComposerCore.h \
../../../Include/BasicSynth.h \
../../Instruments/Instruments.h \
../../Notelist/NLConvert.h

globinc.h.gch: globinc.h
	$(CPP) $(CPPFLAGS) $<
