include ../../BasicSynth.cfg

ifeq ($(SYS),Msys)
wxliblist=xrc gl html adv core base xml stc
ALSA=
WINLIBS=-lwinmm
else
wxliblist=xrc gl html adv core base xml stc
ALSA=$(shell pkg-config --libs alsa)
WINLIBS=
endif
WXINC=$(shell wx-config --cxxflags)
WXLIB=$(shell wx-config --libs $(wxliblist))

CORELIB=../Lib/$(LIBPREFIX)Core$(LIBSUFFIX).a
EXENAME=bsynthcomposerwx$(EXE)

CPPFLAGS+=-I$(BSDIR)/Src/Notelist -I../Core -DPOSIX=1 $(WXINC)

SRCS=\
main.cpp \
FindReplDlg.cpp \
FormEditor.cpp \
GenerateDlg.cpp \
ItemPropertiesDlg.cpp \
InitRes.cpp \
KeyboardDlg.cpp \
MainFrame.cpp \
MixerSetupDlg.cpp \
ModSynthWx.cpp \
OptionsDlg.cpp \
ProjectOptionsWX.cpp \
ProjectPropertiesDlg.cpp \
QueryValueDlg.cpp \
ScoreErrorsDlg.cpp \
SoundBankSelect.cpp \
SynthWidgetWx.cpp \
SynthWidgetWxDC.cpp \
TextEditor.cpp \
TextEditorStc.cpp \
WavetableSelectDlg.cpp

# Include this for compiled-in resources
#SRCS+=resource.cpp

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

ifeq ($(SYS),Msys)
DEVINIT=basicsynth.ini
RESFILE=$(ODIR)/wx.res
# 	windres.exe -i $< -J rc -o $@ -O coff 
RESCOMP=$(shell wx-config --rescomp)
else
DEVINIT=$(HOME)/.basicsynth
RESFILE=
endif
BUILDDIR:=$(shell pwd)
BSCOMP:=$(dir $(BUILDDIR))

.PHONY: all new chkdirs clean

all: chkdirs $(EXENAME) $(DEVINIT)

new: clean all

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)
	@test -d $(BINDIR) || mkdir $(BINDIR)
	
$(EXENAME): $(OBJS) $(CMNLIB) $(NLLIB) $(INSTLIB) $(CORELIB) $(RESFILE)
	@echo Linking $@
	$(CPP) $(XMLLIB) $(CPPFLAGS) -o $@ $(OBJS) $(RESFILE) \
		$(CORELIB) $(NLLIB) $(INSTLIB) $(CMNLIB) \
		$(WXLIB) $(ALSA) -lm $(WINLIBS)

# create an initial options file that points to the development directories.
$(DEVINIT):
	echo Install=$(BSCOMP) > $(DEVINIT)
	echo Forms=$(BSCOMP)/Forms >> $(DEVINIT)
	echo HelpFile=$(BSCOMP)Help/BSynthHelp.hhp >> $(DEVINIT)
	echo Colors=Colors.xml >> $(DEVINIT)

clean:
	-rm -f globinc.h.gch $(OBJS) $(EXENAME)

$(OBJS): globinc.h.gch

ifneq ($(RESFILE),)
$(RESFILE): wx.rc
	$(RESCOMP) -J rc -O coff -i $< -o $@
endif

globinc.h.gch: globinc.h
	@echo create $@
	@$(CPP) -x c++ -c -o $@ $(CPPFLAGS) $<

resource.cpp: resource.xml
	wxrc -c resource.xml

FindReplDlg.cpp: globinc.h FindReplDlg.h

FormEditor.cpp: globinc.h FormEditor.h

GenerateDlg.cpp: globinc.h GenerateDlg.h

ItemPropertiesDlg.cpp: globinc.h ProjectItemDlg.h  ItemPropertiesDlg.h

KeyboardDlg.cpp: globinc.h KeyboardDlg.h

main.cpp: globinc.h MainFrame.h ScoreErrorsDlg.h  KeyboardDlg.h

MainFrame.cpp: globinc.h ProjectItemDlg.h MainFrame.h \
  ScoreErrorsDlg.h KeyboardDlg.h AboutDlg.h ProjectPropertiesDlg.h \
  OptionsDlg.h FormEditor.h QueryValueDlg.h GenerateDlg.h \
  ItemPropertiesDlg.h MixerSetupDlg.h TextEditor.h

MixerSetupDlg.cpp: globinc.h ProjectItemDlg.h  MixerSetupDlg.h

ModSynthDlg.cpp: globinc.h ModSynthDlg.h

ModSynthWx.cpp: globinc.h 

OptionsDlg.cpp: globinc.h OptionsDlg.h

ProjectOptionsWX.cpp: globinc.h 

ProjectPropertiesDlg.cpp: globinc.h ProjectItemDlg.h ProjectPropertiesDlg.h

QueryValueDlg.cpp: globinc.h QueryValueDlg.h

ScoreErrorsDlg.cpp: globinc.h ScoreErrorsDlg.h

SoundBankSelect.cpp: globinc.h SoundBankSelect.h

SynthWidgetWx.cpp: globinc.h 

TextEditor.cpp: globinc.h  TextEditor.h FindReplDlg.h

TextEditorStc.cpp: globinc.h  TextEditorStc.h FindReplDlg.h

WavetableSelectDlg.cpp: globinc.h  WavetableSelectDlg.h

globinc.h: ../Core/ComposerCore.h \
../../../Include/BasicSynth.h \
../../Instruments/Instruments.h \
../../Notelist/NLConvert.h
