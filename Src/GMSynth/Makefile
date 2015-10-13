###########################################################
# Makefile for GMSynth library
# This builds both static lib and shared lib.
###########################################################
include ../BasicSynth.cfg

SRCS=GMSynth.cpp

OBJS=$(ODIR)/GMSynth.o

MODULE=GMSynth

GMSYNTHLIB=$(BSLIB)/$(LIBPREFIX)$(MODULE)$(LIBSUFFIX).a

ifeq ($(SYS),Msys)
GMSYNTHDLL=$(BSLIB)/$(LIBPREFIX)$(MODULE)dll$(LIBSUFFIX).dll
IMPLIB=-Wl,--out-implib,$(BSLIB)/$(LIBPREFIX)$(MODULE)dll.a
SNDLIB=-lWinmm
else
GMSYNTHDLL=$(BSLIB)/$(LIBPREFIX)$(MODULE)$(LIBSUFFIX).so
IMPLIB=
SNDLIB=-lasound
endif

CPPFLAGS+=-DSTATIC_GMSYNTH=1

.PHONY: all new clean chkdirs

all: chkdirs $(GMSYNTHLIB)

new: clean chkdirs $(GMSYNTHLIB)

$(GMSYNTHLIB): $(OBJS)
	@test $@ | rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

$(GMSYNTHDLL): $(OBJS)
	$(CPP) -fPIC -shared -o $@ $(OBJS) $(LINKLIBS) $(SNDLIB) $(IMPLIB)

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)

clean:
	-@rm -f $(OBJS)
	-@rm -f $(GMSYNTHLIB)
	-@rm -f $(GMSYNTHDLL)

$(SRCS): GMSynthDLL.h GMInstrManager.h stdafx.h
