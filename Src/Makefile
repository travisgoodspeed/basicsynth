###########################################################################
# Master makefile for BasicSynth libraries and examples
#
# "make all" makes all modules
# "make [module]" to make one specific example
# "make clean" removes libraries and executable images 
# "make new" to clean and rebuild everything
#
# Note: There seems to be a bug in GNU make when descending
# into sub-makes that causes a 'w' flag to be appended to
# $MAKEFLAGS. Use 'make --no-print-directory' if that happens.
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
BSDIR ?= $(shell dirname `pwd`)
include BasicSynth.cfg

LIBMODULES= Common Instruments Notelist
BINMODULES= Examples GMSynth BSynth
BSMODULES= $(LIBMODULES) $(BINMODULES)

.PHONY: all chkdirs clean new tests $(BSMODULES)

all: chkdirs $(BSMODULES)
	@echo All done

libs: chkdirs $(LIBMODULES)

chkdirs:
	@echo Building from $(BSDIR)
	@test -d $(BSBIN) || mkdir $(BSBIN)
	@test -d $(BSLIB) || mkdir $(BSLIB)

new: chkdirs
	for dd in $(BSMODULES); do $(MAKE) -C $$dd  --no-print-directory BSDIR=$(BSDIR) new; done

clean:
	for dd in $(BSMODULES); do $(MAKE) -C $$dd  --no-print-directory BSDIR=$(BSDIR) clean; done
	
tests:
	-rm -f $(BSBIN)/example*.wav
	cd $(BSBIN); for d in example* ; do ./$$d ; done

$(BSMODULES):
	@echo make $@
	@$(MAKE) -C $@ $(MAKEFLAGS) --no-print-directory BSDIR=$(BSDIR)

$(BINMODULES): $(LIBMODULES)

