###########################################################################
# Makefile for BasicSynth example07b
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../../BasicSynth.cfg

.PHONY: new clean

EXENAME=$(BSBIN)/example07b$(EXE)

$(EXENAME): main.cpp
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) -lm

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/WaveTable.h $(BSINC)/GenWaveWT.h \
	$(BSINC)/Mixer.h $(BSINC)/DelayLine.h \
	$(BSINC)/AllPass.h $(BSINC)/Flanger.h
