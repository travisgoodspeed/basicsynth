include ../../BasicSynth.cfg

.PHONY: new clean

EXENAME=$(BSBIN)/example02a$(EXE)

$(EXENAME): main.cpp
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(CMNLIB) -lm

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)
	
main.cpp: $(BSINC)/SynthDefs.h $(BSINC)/WaveFile.h $(BSINC)/EnvGen.h $(BSINC)/EnvGenSeg.h
