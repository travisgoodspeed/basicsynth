///////////////////////////////////////////////////////////
/// @file ConverterSCO.h Notelist conversion for CSound scores
// Example converter-derived class showing output to
// a system other than BasicSynth. This produces output
// in the form of a CSound score file.
//
// N.B.: This is for example only and is unsupported code.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////
#include <stdio.h>
#include "Converter.h"

// derived class for CSound score file (.SCO) output
class nlConverterSCO : public nlConverter
{
private:
	FILE *fpOutput;
	char *sconame;

	char *SetExtension(const char *name, const char *ext);

public:
	nlConverterSCO();
	virtual ~nlConverterSCO();

	virtual int  Convert(char *filename, nlLexIn *in);
	virtual void BeginNotelist();
	virtual void EndNotelist();
	virtual void BeginVoice(nlVoice *vp);
	virtual void EndVoice(nlVoice *vp);
//	virtual void BeginNote(double start, double dur, double vol, double pit, int pcount, double *params);
//	virtual void ContinueNote(double start, double vol, double pit, int pcount, double *params);
	virtual void BeginNote(double start, double dur);
	virtual void RestartNote(double start, double dur);
	virtual void ContinueNote(double start);
	virtual void Write(char *txt);
	virtual void MixerEvent(int fn, double *params);
	virtual void MidiEvent(short mmsg, short ccnum, short ccval);
	virtual void TrackOp(int op, int trk, int cnt);
};

