###########################################################
# Makefile for BasicSynth Common library
#
# Targets:
#        all - make the library
#        clean - remove the object files
#        new - clean then make the library
###########################################################
include ../BasicSynth.cfg
TINYXML=tinyxml

SRCS=\
	DLSFile.cpp \
	Global.cpp \
	InstrManager.cpp \
	MIDIControl.cpp \
	MIDIInput.cpp \
	MIDISequencer.cpp \
	Player.cpp \
	Sequencer.cpp \
	SequenceFile.cpp \
	SFFile.cpp \
	SMFFile.cpp \
	SoundBank.cpp \
	WaveFile.cpp \
	SynthString.cpp \
	SynthMutex.cpp \
	SynthThread.cpp \
	XmlWrapU.cpp \
	XmlWrapN.cpp \
	XmlWrapT.cpp \
	$(TINYXML)/tinyxml.cpp \
	$(TINYXML)/tinyxmlerror.cpp \
	$(TINYXML)/tinyxmlparser.cpp \
	$(TINYXML)/tinystr.cpp

ifeq ($(SYS),Msys)
SRCS += SynthFileW.cpp WaveOutDirect.cpp
else
SRCS += SynthFileU.cpp
endif

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

.PHONY: new all clean chkdirs

all:  chkdirs $(CMNLIB)

new: clean all

$(CMNLIB): $(OBJS)
	@test $@ || rm $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)
	@test -d $(ODIR)/$(TINYXML) || mkdir $(ODIR)/$(TINYXML)
	
clean:
	-@rm $(OBJS)
	-@rm $(CMNLIB)

DLSFile.cpp: \
	$(BSINC)/DLSDefs.h \
	$(BSINC)/DLSFile.h \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SoundBank.h

Player.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Player.h

MIDISequencer.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDISequencer.h

MIDIControl.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/XmlWrap.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/GenWaveWT.h \
	$(BSINC)/MIDIDefs.h \
	$(BSINC)/MIDIControl.h

MIDIInput.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDIDefs.h \
	$(BSINC)/MIDIInput.h

Sequencer.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/WaveTable.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/SynthMutex.h \
	$(BSINC)/Sequencer.h

SequenceFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/Mixer.h \
	$(BSINC)/WaveFile.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/SequenceFile.h

SFFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SFDefs.h \
	$(BSINC)/SoundBank.h \
	$(BSINC)/SFFile.h

SoundBank.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SoundBank.h

SMFFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/SeqEvent.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h \
	$(BSINC)/MIDIDefs.h \
	$(BSINC)/SMFFile.h

WaveFile.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthFile.h \
	$(BSINC)/WaveFile.h

InstrManager.cpp: \
	$(BSINC)/SynthDefs.h \
	$(BSINC)/SynthString.h \
	$(BSINC)/SynthList.h \
	$(BSINC)/Instrument.h \
	$(BSINC)/Sequencer.h

SynthFileU.cpp: $(BSINC)/SynthFile.h

SynthFileW.cpp: $(BSINC)/SynthFile.h

SynthString.cpp: $(BSINC)/SynthString.h

SynthMutex.cpp: $(BSINC)/SynthMutex.h

SynthThread.cpp: $(BSINC)/SynthThread.h

Global.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveTable.h

WaveOutDirect.cpp: $(BSINC)/WaveOutDirect.h

XmlWrapU.cpp: $(BSINC)/XmlWrap.h

XmlWrapN.cpp: $(BSINC)/XmlWrap.h

XmlWrapT.cpp: $(BSINC)/XmlWrap.h

