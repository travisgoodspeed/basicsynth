///////////////////////////////////////////////////////////
/// @file ConverterSCO.cpp Implementation of the Notelist converter for CSound score files
//
// Convert to CSound score file.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <BasicSynth.h>
#include "NLConvert.h"
#include "ConverterSCO.h"

nlConverterSCO::nlConverterSCO()
{
	fpOutput = stdout;
	sconame = NULL;
}

nlConverterSCO::~nlConverterSCO()
{
	delete sconame;
}

int nlConverterSCO::Convert(char *filename, nlLexIn *in)
{
	sconame = SetExtension(filename, ".sco");
	return nlConverter::Convert(filename, in);
}

void nlConverterSCO::BeginNotelist()
{
	if (sconame != NULL)
	{
		fpOutput = fopen(sconame, "wa");

		if (fpOutput == NULL)
			fprintf(stderr, "Cannot open file: %s\n", sconame);
	}
	nlConverter::BeginNotelist();
}

void nlConverterSCO::EndNotelist()
{
	fputs("\ne\n", fpOutput);
	if (fpOutput != stdout)
		fclose(fpOutput);
	fpOutput = NULL;
	nlConverter::EndNotelist();
}

void nlConverterSCO::BeginVoice(nlVoice *vp)
{
	if (fpOutput)
		fprintf(fpOutput, "; BeginVoice %d\n", vp->voiceNum);
	nlConverter::BeginVoice(vp);
}

void nlConverterSCO::EndVoice(nlVoice *vp)
{
	if (fpOutput)
		fprintf(fpOutput, "; EndVoice %d\n", vp->voiceNum);
	nlConverter::EndVoice(vp);
}

void nlConverterSCO::BeginNote(double start, double dur)
{
	if (curVoice == NULL || fpOutput == NULL)
		return;

	double amp = curVoice->lastVol;
	if (amp > 0)
	{
		if (gen.GetVoldbMode())
			amp = pow(10.0, (amp - 100.0) / 20.0);
		else
			amp *= 0.01;
	}
	amp *= curVoice->volMul;
	fprintf(fpOutput, "i%ld %g %g %g", curVoice->instr, start, dur, amp);

	if (gen.GetFrequencyMode())
	{
		fprintf(fpOutput, " %g", curVoice->lastPit * pow(2.0, (double)curVoice->transpose/12.0));
	}
	else
	{
		long ipit = (long) (curVoice->lastPit + curVoice->transpose);
		fprintf(fpOutput, " %ld%c%02ld", 4 + (ipit / 12), '.', (ipit % 12));
	}

	int pn;
	int pc = curVoice->numParam;
	for (pn = 0; pn < pc; pn++)
		fprintf(fpOutput, " %g", curVoice->lastParam[pn]);

	fputc('\n', fpOutput);

	if (curVoice->doublex != 0)
	{
		fprintf(fpOutput, "i%ld %g %g %g", curVoice->instr, start, dur, amp * curVoice->doublev);
		if (gen.GetFrequencyMode())
		{
			fprintf(fpOutput, " %g", curVoice->lastPit * pow(2.0, (double)(curVoice->transpose + curVoice->doublex)/12.0));
		}
		else
		{
			long ipit = (long) (curVoice->lastPit + curVoice->transpose + curVoice->doublex);
			fprintf(fpOutput, " %ld%c%02ld", 4 + (ipit / 12), '.', (ipit % 12));
		}
		fputc('\n', fpOutput);
	}
}

void nlConverterSCO::RestartNote(double start, double dur)
{
	BeginNote(start, dur);
}

void nlConverterSCO::ContinueNote(double start)
{
	// how do we do this in CSound?
}

void nlConverterSCO::MixerEvent(int fn, double *params)
{
	if (mixInstr < 0)
		return;

	// i inum time dur fn fx# from to frq wt
	fprintf(fpOutput, "i%d %g %g %d %g %g %g %g %g\n",
		mixInstr, curVoice->curTime, params[3], fn,
		params[0], params[1], params[2], params[4], params[5]);
}

void nlConverterSCO::MidiEvent(short mmsg, short val1, short val2)
{
}

void nlConverterSCO::TrackOp(int op, int trk, int cnt)
{
}

void nlConverterSCO::Write(char *txt)
{
	fputs(txt, fpOutput);
	fputc('\n', fpOutput);
	nlConverter::Write(txt);
}

char *nlConverterSCO::SetExtension(const char *name, const char *ext)
{
	size_t len = strlen(name) + strlen(ext) + 1;
	char *newname = new char[len];
	if (newname == NULL)
		return NULL;

	strcpy(newname, name);

	char *lastp = strrchr(newname, '\\');
	if (lastp == NULL)
		lastp = strrchr(newname, '/');
	if (lastp == NULL)
		lastp = newname;

	char *dotp = strrchr(lastp, '.');
	if (dotp != NULL)
		strcpy(dotp, ext);
	else
		strcat(newname, ext);
	return newname;
}
