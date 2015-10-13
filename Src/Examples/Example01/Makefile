###########################################################################
# Makefile for BasicSynth example01
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../../BasicSynth.cfg

.PHONY: all new clean

EXENAME=$(BSBIN)/example01$(EXE)

$(EXENAME): main.cpp $(CMNLIB)
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) -lm 

all: $(EXENAME)

new: clean $(EXENAME)

clean:
	-rm $(EXENAME)

main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h
