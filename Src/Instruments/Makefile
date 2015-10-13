###########################################################
# Makefile for BasicSynth Instruments library
#
# Targets:
#        all - make the library
#        clean - remove the object files
#        new - clean then rebuild
###########################################################
include ../BasicSynth.cfg

SRCS=\
	LoadInstrLib.cpp \
	AddSynth.cpp \
	BuzzSynth.cpp \
	Chuffer.cpp \
	FMSynth.cpp \
	GMPlayer.cpp \
	LFO.cpp \
	PitchBend.cpp \
	MatrixSynth.cpp \
	MixerControl.cpp \
	ModSynth.cpp \
	SFPlayer.cpp \
	SubSynth.cpp \
	Tone.cpp \
	WFSynth.cpp

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

.PHONY: all new clean chkdirs

all: chkdirs $(INSTLIB)

new: clean chkdirs $(INSTLIB)

$(INSTLIB):  $(OBJS)
	@test $@ || rm $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)

clean:
	-@rm -f $(OBJS) Includes.h.gch
	-@rm -f $(INSTLIB)

$(OBJS) : Includes.h.gch

Includes.h.gch: Includes.h $(BSINC)/BasicSynth.h
	$(CPP) -x c++ -c $(CPPFLAGS) Includes.h -o $@
	
AddSynth.cpp: AddSynth.h Includes.h

BuzzSynth.cpp: BuzzSynth.h Includes.h

Chuffer.cpp: Chuffer.h Includes.h

FMSynth.cpp: FMSynth.h Includes.h

LFO.cpp: LFO.h Includes.h

GMPlayer.cpp: GMPlayer.h $(BSINC)/SoundBank.h $(BSINC)/MIDIControl.h

PitchBend.cpp: PitchBend.h Includes.h

MatrixSynth.cpp: MatrixSynth.h Includes.h

MixerControl.cpp: MixerControl.h Includes.h

ModSynth.cpp: ModSynth.h ModSynthUG.h UGOscil.h UGFilter.h UGEnvGen.h UGDelay.h UGCalc.h Includes.h

SFPlayer.cpp: SFPlayer.h

SubSynth.cpp: SubSynth.h Includes.h

Tone.cpp: Tone.h Includes.h

WFSynth.cpp: WFSynth.h Includes.h
