###########################################################
# Makefile for BasicSynth Notelist library
#
# Targets:
#        all - make the library
#        clean - remove the object files
#        new - clean then rebuild
###########################################################
include ../BasicSynth.cfg

SRCS=\
	Converter.cpp \
	Parser.cpp \
	Lex.cpp \
	Generate.cpp

.PHONY: all new clean chkdirs

OBJS=$(patsubst %.cpp,$(ODIR)/%.o,$(SRCS))

all: chkdirs $(NLLIB)

new: clean all

$(NLLIB): $(OBJS)
	@test $@ || rm $@
	$(AR) r $@ $(OBJS)
	$(RANLIB) $@

chkdirs:
	@test -d $(ODIR) || mkdir $(ODIR)

clean:
	-@rm -f $(OBJS)
	-@rm -f $(NLLIB)

Converter.cpp: Converter.h Lex.h Parser.h NLConvert.h 

Parser.cpp: Parser.h Lex.h NLConvert.h

Lex.cpp: Lex.h NLConvert.h

Generate.cpp: Generate.h Converter.h NLConvert.h
