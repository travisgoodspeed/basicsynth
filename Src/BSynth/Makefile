###########################################################################
# Makefile for BasicSynth command line synthesizer
#
# "make all" makes all modules
# "make clean" removes libraries and executable images 
# "make new" cleans then rebuilds
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../BasicSynth.cfg

.PHONY: all new clean testdata

DATAFILES=\
	testinst.xml \
	testprj.xml \
	test.nl \
	test2.nl \
	tstaddsynth.xml \
	tstaddsynth.nl \
	tstmatsynth.xml \
	tstmatsynth.nl \
	tstsubsynth.xml \
	tstsubsynth.nl \
	tstfmsynth.xml \
	tstfmsynth.nl \
	tsttonesynth.xml \
	tsttonesynth.nl \
	testnl.xml \
	testnl.nl

EXENAME=$(BSBIN)/BSynth$(EXE)

all: $(EXENAME) testdata

new: clean all

testdata: $(DATAFILES)
	cp $(DATAFILES) $(BSBIN)

$(EXENAME): main.cpp $(CMNLIB) $(NLLIB) $(INSTLIB)
	$(CPP) $(CPPFLAGS) -o $@ main.cpp $(XMLLIB) -I../Notelist \
		$(NLLIB) $(INSTLIB) $(CMNLIB) -lm

clean:
	-rm -f $(EXENAME)
	-rm -f $(BSBIN)/testinst.xml $(BSBIN)/testprj.xml
	-rm -f $(BSBIN)/test.nl $(BSBIN)/test2.nl
	-rm -f $(BSBIN)/testnl.xml $(BSBIN)/testnl.nl
	-rm -f $(BSBIN)/tst*synth.xml
	-rm -f $(BSBIN)/tst*synth.nl
