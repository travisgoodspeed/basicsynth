###########################################################################
# Makefile for BasicSynth examples
#
# "make all" makes all executables
# "make Examplenn" to make one specific example
# "make clean" removes the executable images from $(BSBIN)
# "make new" cleans and rebuilds
#
# Dan Mitchell (http://basicsynth.com)
###########################################################################
include ../BasicSynth.cfg

EXAMPLES= \
Example01 \
Example02 \
Example02a \
Example02b \
Example03 \
Example04 \
Example4a \
Example05 \
Example06 \
Example07 \
Example07a \
Example07b \
Example07c \
Example08 \
Example09 \
Example9a \
Example10

.PHONY: all new clean $(EXAMPLES)

all: $(EXAMPLES)

$(EXAMPLES):
	@$(MAKE) -C $@ $(MAKEFLAGS)

new:
	for d in $(EXAMPLES) ; do $(MAKE) -C $$d $(MAKEFLAGS) new ; done

clean:
	for d in $(EXAMPLES) ; do $(MAKE) -C $$d $(MAKEFLAGS) clean ; done
