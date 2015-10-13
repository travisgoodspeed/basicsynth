###########################################################################
# Makefile for BasicSynth example05
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../../BasicSynth.cfg

.PHONY: new clean

EXENAME=$(BSBIN)/example05$(EXE)

$(EXENAME): main.cpp
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) -lm

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h \
	$(BSINC)/EnvGen.h $(BSINC)/GenWave.h \
	$(BSINC)/GenWaveWT.h $(BSINC)/WaveTable.h \
	$(BSINC)/GenNoise.h $(BSINC)/BiQuad.h \
	$(BSINC)/AllPass.h $(BSINC)/Filter.h
