//////////////////////////////////////////////////////////////////////
/// @file ModSynth.h BasicSynth Modular Synthesis instrument
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef MODSYNTH_H
#define MODSYNTH_H

#include "ModSynthUG.h"
#include "UGOscil.h"
#include "UGEnvGen.h"
#include "UGFilter.h"
#include "UGDelay.h"
#include "UGCalc.h"

struct ModSynthUGType
{
	const char *name;
	const char *shortName;
	const char *longName;
	ModSynthUG *(*fn)();
};

extern ModSynthUGType ugTypes[];

class ModSynth : public InstrumentVP
{
private:
	int numUnits;
	bsInt16 maxID;

	UGValue head; // sr
	UGOut   tail; // out

	UGValue *frqParam;
	UGValue *volParam;
	UGValue *durParam;
	UGValue *pitParam;

	int chnl;
	bsInt32 dur;
	AmpValue vol;
	FrqValue frq;

	InstrManager *im;
public:
	ModSynth();
	~ModSynth();

	static Instrument *ModSynthFactory(InstrManager *, Opaque tmplt);
	static SeqEvent *ModSynthEventFactory(Opaque tmplt);
	static bsInt16 MapParamID(const char *name, Opaque tmplt);
	static const char *MapParamName(bsInt16 id, Opaque tmplt);

	void Copy(ModSynth *tp);
	void CopyConn(ModSynthUG *ug);

	int GetNumUnits();
	int GetNumParams();
	ModSynthUGType *FindType(const char *type);
	ModSynthUG *AddUnit(const char *type, const char *name, ModSynthUG *before = 0);
	ModSynthUG *FindUnit(const char *name);
	ModSynthUG *FindUnit(bsInt16 id);
	ModSynthUG *FirstUnit();
	ModSynthUG *NextUnit(ModSynthUG *ug);
	void RemoveUnit(ModSynthUG *ug, int dodel = 1);
	void MoveUnit(ModSynthUG *ug, ModSynthUG *before);
	UGValue *AddParam(const char *name, bsInt16 id);
	void Connect(const char *src, const char *dst);
	void Connect(ModSynthUG *ug, const char *dst);
	void Connect(ModSynthUG *src, ModSynthUG *dst, int index = 0, int when = 3);
	void Disconnect(ModSynthUG *src, ModSynthUG *dst, int index = -1);
	void DumpConnect(void (*fn)(const char *));

	void Start(SeqEvent *evt);
	void Param(SeqEvent *evt);
	void Stop();
	void Tick();
	int IsFinished();
	int Load(XmlSynthElem *parent);
	int Save(XmlSynthElem *parent);
	int SaveConnect(XmlSynthElem *parent, ModSynthUG *ug);

	int  SetParams(VarParamEvent *params);
	int  GetParams(VarParamEvent *params);
	int  SetParam(bsInt16 idval, float val);
	int  GetParam(bsInt16 idval, float *val);
};
//@}
#endif
