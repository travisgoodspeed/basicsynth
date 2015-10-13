###########################################################################
# Makefile for BasicSynth example09
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../../BasicSynth.cfg

.PHONY: new clean

EXENAME=$(BSBIN)/example09$(EXE)

$(EXENAME): main.cpp
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) -lm

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/BiQuad.h \
	$(BSINC)/GenNoise.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/DelayLine.h $(BSINC)/Mixer.h \
	$(BSINC)/Sequencer.h $(BSINC)/MIDISequencer.h
