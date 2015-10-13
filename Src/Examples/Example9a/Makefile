###########################################################################
# Makefile for BasicSynth example9a
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../../BasicSynth.cfg

.PHONY: new clean

EXENAME=$(BSBIN)/example9a$(EXE)

$(EXENAME): main.cpp MIDINames.cpp $(INSTLIB) $(CMNLIB)
	$(CPP) $(CPPFLAGS) -o $@ main.cpp MIDINames.cpp $(INSTLIB) $(CMNLIB) -lm 

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/BiQuad.h \
	$(BSINC)/GenNoise.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/DelayLine.h $(BSINC)/Mixer.h \
	$(BSINC)/Sequencer.h $(BSINC)/SMFFile.h \
	$(BSINC)/SFFile.h $(BSINC)/SoundBank.h \
	$(BSINC)/DLSFile.h $(BSINC)/DLSDefs.h \
	$(BSINC)/SFGen.h $(BSINC)/SFDefs.h
