//////////////////////////////////////////////////////////////////////
/// @file LoadInstrLib.h BasicSynth instrument library load functions
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
extern int LoadInstrLib(InstrManager& mgr, const char *fname);
extern int LoadInstrLib(InstrManager& mgr, XmlSynthElem *inst);
extern InstrConfig *LoadInstr(InstrManager& mgr, XmlSynthElem *inst);
