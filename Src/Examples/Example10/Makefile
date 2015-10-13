###################################################
# Makefile for BasicSynth example10
#
# "make new" rebuilds all executables
# "make clean" removes the executable images from $(BSBIN)
#
# Dan Mitchell (http://basicsynth.com)
########################
include ../../BasicSynth.cfg

DATAFILES=data.xml test1.wav test2.wav

.PHONY: new clean

EXENAME=$(BSBIN)/example10$(EXE)

$(EXENAME): main.cpp $(INSTLIB) $(CMNLIB)
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(INSTLIB) $(CMNLIB) $(XMLLIB) -lm
	-cp $(DATAFILES) $(BSBIN)

new: clean $(EXENAME)

clean:
	-rm -f $(EXENAME)
	-rm -f $(BSBIN)/data.xml $(BSBIN)/data.xml $(BSBIN)/_data.xml $(BSBIN)/test?.wav

main.cpp: \
$(BSINC)/SynthDefs.h \
$(BSINC)/WaveTable.h \
$(BSINC)/GenWave.h \
$(BSINC)/GenWaveWT.h \
$(BSINC)/GenNoise.h \
$(BSINC)/EnvGen.h \
$(BSINC)/EnvGenSeg.h \
$(BSINC)/BiQuad.h \
$(BSINC)/AllPass.h \
$(BSINC)/Filter.h \
$(BSINC)/DelayLine.h \
$(BSINC)/Flanger.h \
$(BSINC)/Reverb.h \
$(BSINC)/Mixer.h \
$(BSINC)/WaveFile.h \
$(BSINC)/Sequencer.h \
$(BSINC)/XmlWrap.h \
$(BSINSTR)/Tone.h \
$(BSINSTR)/LFO.h \
$(BSINSTR)/SubSynth.h \
$(BSINSTR)/AddSynth.h \
$(BSINSTR)/FMSynth.h \
$(BSINSTR)/MatrixSynth.h \
$(BSINSTR)/WFSynth.h
