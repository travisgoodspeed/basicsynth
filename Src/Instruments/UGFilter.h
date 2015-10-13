//////////////////////////////////////////////////////////////////////
/// @file UGFilter.h Filters for ModSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef UG_FILTER_H
#define UG_FILTER_H


//////////////////////////////////////////////////////////////////////////
// Filters
//////////////////////////////////////////////////////////////////////////
extern UGParam filterParams1[];
extern UGParam filterParams2[];
extern UGParam filterParams3[];
extern UGParam allpassParams[];

#define UGFLT_INP  0
#define UGFLT_FRQ  1
#define UGFLT_GAIN 2
#define UGFLT_RES  2
#define UGFLT_ORD  2
#define UGFLT_CRT  3
#define UGFLT_LHP  4

template <class DT, class UG, short IP = 1>
class ModSynthFiltImpl : public ModSynthUGImpl<DT, UG, IP>
{
protected:
	bsInt32 crCount; // coefficient recalc counter

public:
	int IsFinished()
	{
		return 1;
	}

	void SetInput(short index, float value)
	{
		DT *pdt = (DT*)this;
		if (index == UGFLT_INP)
			pdt->inputs[UGFLT_INP] += value;
		else if (index < IP)
			pdt->inputs[index] = value;
		pdt->anyChange |= 1 << index;
	}

	virtual void Recalc()
	{
	}

	void Start()
	{
		DT *pdt = (DT*)this;
		pdt->Recalc();
		pdt->gen.Reset(0);
		crCount = (bsInt32) pdt->inputs[UGFLT_CRT];
		pdt->inputs[UGFLT_INP] = 0.0f;
		pdt->out = 0;
	}

	void Tick()
	{
		DT *pdt = (DT*)this;
		if (--crCount <= 0)
		{
			crCount = (bsInt32) pdt->inputs[UGFLT_CRT];
			if (pdt->anyChange & ((1<<UGFLT_FRQ)|(1<<UGFLT_GAIN)))
			{
				pdt->Recalc();
				pdt->anyChange = 0;
			}
		}
		pdt->out = pdt->gen.Sample(pdt->inputs[0]);
		pdt->inputs[0] = 0;
		pdt->Send(pdt->out, UGP_GEN);
	}
};

class UGLowpass : public ModSynthFiltImpl<UGLowpass, FilterLP, 4>
{
public:
	const UGParam *GetParamList()
	{
		return filterParams1;
	}

	const char *GetType()
	{
		return "LOWPASS";
	}

	UGLowpass()
	{
		crCount = 0;
	}

	void InitDefault()
	{
		crCount = 0;
		inputs[UGFLT_FRQ] = 1000.0;
		inputs[UGFLT_GAIN] = 1.0;
		inputs[UGFLT_CRT] = 0.0;
	}

	void Recalc()
	{
		gen.Init(inputs[UGFLT_FRQ], inputs[UGFLT_GAIN]);
	}
};

class UGHipass : public ModSynthFiltImpl<UGHipass, FilterHP, 4>
{
public:
	const char *GetType()
	{
		return "HIPASS";
	}

	const UGParam *GetParamList()
	{
		return filterParams1;
	}

	UGHipass()
	{
		crCount = 0;
	}

	void InitDefault()
	{
		inputs[UGFLT_FRQ] = 100.0;
		inputs[UGFLT_GAIN] = 1.0;
		inputs[UGFLT_CRT] = 0.0;
	}

	void Recalc()
	{
		gen.Init(inputs[UGFLT_FRQ], inputs[UGFLT_GAIN]);
	}
};

class UGLowpassR : public ModSynthFiltImpl<UGLowpassR, FilterIIR2p, 4>
{
public:
	const char *GetType()
	{
		return "LOWPASSR";
	}

	const UGParam *GetParamList()
	{
		return filterParams2;
	}

	UGLowpassR()
	{
		crCount = 0;
	}

	void InitDefault()
	{
		inputs[UGFLT_FRQ] = 1000.0;
		inputs[UGFLT_RES] = 1.0;
		inputs[UGFLT_CRT] = 0.0;
	}

	void Recalc()
	{
		gen.CalcCoef(inputs[UGFLT_FRQ], inputs[UGFLT_RES]);
	}
};

class UGReson : public ModSynthFiltImpl<UGReson, Reson, 4>
{
public:
	const char *GetType()
	{
		return "RESON";
	}

	const UGParam *GetParamList()
	{
		return filterParams2;
	}

	UGReson()
	{
		crCount = 0;
	}

	void InitDefault()
	{
		inputs[UGFLT_FRQ] = 1000.0;
		inputs[UGFLT_RES] = 1.0;
		inputs[UGFLT_CRT] = 0.0;
	}

	void Recalc()
	{
		gen.SetRes(inputs[UGFLT_RES]);
		gen.SetFrequency(inputs[UGFLT_FRQ]);
		gen.SetGain(1.0);
		gen.CalcCoef();
	}
};

class UGFilterFIRn : public ModSynthFiltImpl<UGFilterFIRn, FilterFIRn, 5>
{
public:
	const UGParam *GetParamList()
	{
		return filterParams3;
	}

	const char *GetType()
	{
		return "FILTFIRN";
	}

	UGFilterFIRn()
	{
		crCount = 0;
	}

	void InitDefault()
	{
		inputs[UGFLT_FRQ] = 1000.0; // fc
		inputs[UGFLT_ORD] = 3.0; // order
		inputs[UGFLT_LHP] = 0.0; // 0=lp or 1=hp
		inputs[UGFLT_CRT] = 0.01; // cr
	}

	void Recalc()
	{
		gen.CalcCoef(inputs[UGFLT_FRQ], (int) inputs[UGFLT_LHP]);
	}

	void Start()
	{
		int ord = (int) inputs[2];
		if (ord < 0)
			ord = -ord;
		if (!(ord & 1))
			ord++;
		gen.AllocImpResp(ord);
		Recalc();
		gen.Reset(0);
		crCount = (bsInt32) inputs[UGFLT_CRT];
		inputs[0] = 0;
		out = 0;
	}
};

class UGAllpass : public ModSynthUGImpl<UGAllpass, AllPassFilter, 2>
{
public:
	const UGParam *GetParamList()
	{
		return allpassParams;
	}

	const char *GetType()
	{
		return "ALLPASS";
	}

	UGAllpass()
	{
	}

	void SetInput(short index, float val)
	{
		if (index == 0)
			inputs[0] += val;
		else
			inputs[index] = val;
		anyChange |= 1 << index;
	}

	void Start()
	{
		gen.InitAP(inputs[1]);
		inputs[0] = 0;
		out = 0;
	}

	void Tick()
	{
		if (anyChange & 2)
		{
			gen.InitAP(inputs[1]);
			anyChange = 0;
		}
		out = gen.Sample(inputs[0]);
		inputs[0] = 0;
		Send(out, UGP_GEN);
	}
};
//@}
#endif
