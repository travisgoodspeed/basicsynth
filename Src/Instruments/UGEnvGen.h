//////////////////////////////////////////////////////////////////////
/// @file UGEnvGen.h Envelope Generators for ModSynth
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef UG_ENVGEN_H
#define UG_ENVGEN_H

extern UGParam envARParams[];
extern UGParam envADSRParams[];
extern UGParam envSegNParams[];

#define UGAR_INP   0
#define UGAR_ATK   1
#define UGAR_SUS   2
#define UGAR_REL   3
#define UGAR_SON   4
#define UGAR_TYP   5
#define UGAR_FIX   6
#define UGAR_SCL   7

/// @brief AR envelope generator UG
class UGEnvAR : public ModSynthUGImpl<UGEnvAR, EnvGenAR, 8>
{
public:
	const UGParam *GetParamList()
	{
		return envARParams;
	}

	const char *GetType()
	{
		return "ENVAR";
	}

	UGEnvAR()
	{
		inputs[UGAR_INP] = 1.0f;
		inputs[UGAR_SCL] = 1.0f;
	}

	void InitDefault()
	{
		inputs[UGAR_INP] = 1.0f; // input sample
		inputs[UGAR_ATK] = 0.1f; // attack rate
		inputs[UGAR_SUS] = 1.0f; // sustain level
		inputs[UGAR_REL] = 0.1f; // release rate
		inputs[UGAR_SON] = 1.0f; // sustain on
		inputs[UGAR_TYP] = 1.0f; // curve type
		inputs[UGAR_FIX] = 1.0f; // fixed/relative
		inputs[UGAR_SCL] = 1.0f; // level scale
	}

	void Start()
	{
		float scale = inputs[UGAR_SCL];
		gen.InitAR(FrqValue(inputs[UGAR_ATK]), 
			       AmpValue(inputs[UGAR_SUS]*scale),
				   FrqValue(inputs[UGAR_REL]),
			       (int) inputs[UGAR_SON], 
				   (EGSegType)(int)inputs[UGAR_TYP], 
				   (int)inputs[UGAR_FIX]);
		anyChange = 0;
	}

	void Stop()
	{
		gen.Release();
	}

	int IsFinished()
	{
		return gen.IsFinished();
	}
};

#define UGADSR_INP   0
#define UGADSR_STL   1
#define UGADSR_ATK   2
#define UGADSR_PKL   3
#define UGADSR_DEC   4
#define UGADSR_SUS   5
#define UGADSR_REL   6
#define UGADSR_END   7
#define UGADSR_TYP   8
#define UGADSR_FIX   9
#define UGADSR_SCL  10

/// @brief ADSR envelope generator UG
class UGEnvADSR : public ModSynthUGImpl<UGEnvADSR, EnvGenADSR, 11>
{
public:
	const UGParam *GetParamList()
	{
		return envADSRParams;
	}

	const char *GetType()
	{
		return "ENVADSR";
	}

	UGEnvADSR()
	{
		inputs[UGADSR_INP] = 1.0f;
		inputs[UGADSR_SCL] = 1.0f;
	}

	void InitDefault()
	{
		inputs[UGADSR_INP] = 1.0f; // input sample
		inputs[UGADSR_STL] = 0.0f; // start level
		inputs[UGADSR_ATK] = 0.1f; // attack rate
		inputs[UGADSR_PKL] = 1.0f; // peak level
		inputs[UGADSR_DEC] = 0.0f; // decay rate
		inputs[UGADSR_SUS] = 1.0f; // sustain level
		inputs[UGADSR_REL] = 0.1f; // release rate
		inputs[UGADSR_END] = 0.0f; // end level
		inputs[UGADSR_TYP] = 1.0f; // curve type
		inputs[UGADSR_FIX] = 1.0f; // fixed or relative durations
		inputs[UGADSR_SCL] = 1.0f; // level scale
	}

	void Start()
	{
		float scale = inputs[UGADSR_SCL];
		gen.InitADSR(inputs[UGADSR_STL]*scale, 
			         inputs[UGADSR_ATK], 
					 inputs[UGADSR_PKL]*scale,
					 inputs[UGADSR_DEC], 
					 inputs[UGADSR_SUS]*scale,
			         inputs[UGADSR_REL], 
					 inputs[UGADSR_END]*scale, 
					 (EGSegType)(int)inputs[UGADSR_TYP], 
					 (int) inputs[UGADSR_FIX]);
		anyChange = 0;
	}

	void Stop()
	{
		gen.Release();
	}

	int IsFinished()
	{
		return gen.IsFinished();
	}
};

// maximum of 12 envelope segments, numbered 0-11
#define UGEG_INP    0
#define UGEG_SEGN   1
#define UGEG_ST     2
#define UGEG_SUS    3
#define UGEG_SCL    4
#define UGEG_SEGLVL(n) (5+(n*4))
#define UGEG_SEGRTE(n) (6+(n*4))
#define UGEG_SEGTYP(n) (7+(n*4))
#define UGEG_SEGFIX(n) (8+(n*4))
#define UGEG_MAXSEG 12
#define UGEG_MAXPARAM ((UGEG_MAXSEG*4)+5)

/// @brief Variable segment envelope generator UG
class UGEnvSegN : public ModSynthUGImpl<UGEnvSegN, EnvGenSegSus, UGEG_MAXPARAM>
{
private:
	short numInputs;

public:
	const UGParam *GetParamList()
	{
		return envSegNParams;
	}

	const char *GetType()
	{
		return "ENVSEGN";
	}

	UGEnvSegN()
	{
		numInputs = 5;
		inputs[UGEG_INP] = 1.0f;
		inputs[UGEG_SCL] = 1.0f; // level scale
	}

	virtual void InitDefault()
	{
		numInputs = 13;
		inputs[UGEG_INP] = 1.0f; // input sample
		inputs[UGEG_SEGN] = 2.0f; // number of segments
		inputs[UGEG_ST]  = 0.0f; // start
		inputs[UGEG_SUS] = 1.0f; // sustain on
		inputs[UGEG_SCL] = 1.0f; // level scale
		int index = UGEG_SEGLVL(0);
		inputs[index++] = 0.1f; // level
		inputs[index++] = 0.1f; // rate
		inputs[index++] = 1.0f; // curve type
		inputs[index++] = 1.0f; // fixed duration
		inputs[index++] = 0.0f; // level
		inputs[index++] = 0.1f; // rate
		inputs[index++] = 1.0f; // curve type
		inputs[index++] = 1.0f; // fixed duration

		for (int segn = 2; segn < UGEG_MAXSEG; segn++)
		{
			inputs[index++] = 0.0f; // level
			inputs[index++] = 0.0f; // rate
			inputs[index++] = 1.0f; // curve type
			inputs[index++] = 1.0f; // fixed duration
		}
	}

	virtual short GetNumInputs()
	{
		return numInputs;
	}

	void SetInput(short index, float val)
	{
		if (index < UGEG_MAXPARAM)
		{
			if (index == UGEG_SEGN)
			{
				// number of segments
				if (val > UGEG_MAXSEG)
					val = UGEG_MAXSEG;
				numInputs = (short)(val * 4) + 5;
			}
			inputs[index] = val;
		}
	}

	void Start()
	{
		int nsegs = (int) inputs[UGEG_SEGN];
		gen.SetSegs(nsegs);
		gen.SetStart(inputs[UGEG_ST]);
		gen.SetSusOn((int)inputs[UGEG_SUS]);
		float scale = inputs[UGEG_SCL];
		for (int seg = 0; seg < nsegs; seg++)
		{
			int index = UGEG_SEGLVL(seg);
			gen.SetSegN(seg, inputs[index]*scale, inputs[index+1], 
				       (EGSegType)(int)inputs[index+2], (int)inputs[index+3]);
		}
		gen.Reset(0);
	}

	void Stop()
	{
		gen.Release();
	}

	int IsFinished()
	{
		return gen.IsFinished();
	}

	virtual int Load(XmlSynthElem *elem)
	{
		elem->GetAttribute("segments", inputs[UGEG_SEGN]);
		elem->GetAttribute("start", inputs[UGEG_ST]);
		elem->GetAttribute("sustain", inputs[UGEG_SUS]);
		elem->GetAttribute("scale", inputs[UGEG_SCL]);
		if (inputs[UGEG_SCL] == 0.0f)
			inputs[UGEG_SCL] = 1.0f;

		int segs = 0;
		int index = UGEG_SEGLVL(0);
		XmlSynthElem node(elem->Document());
		XmlSynthElem *child = elem->FirstChild(&node);
		while (child)
		{
			if (index < UGEG_MAXPARAM && child->TagMatch("seg"))
			{
				child->GetAttribute("level", inputs[index++]);
				child->GetAttribute("rate", inputs[index++]);
				child->GetAttribute("curve", inputs[index++]);
				child->GetAttribute("time", inputs[index++]);
				segs++;
			}
			child = node.NextSibling(&node);
		}
		if (segs < 1)
			segs = 1;
		inputs[UGEG_SEGN] = (float) segs;
		numInputs = (short) (segs * 4) + 5;
		return 0;
	}

	virtual int Save(XmlSynthElem *elem)
	{
		elem->SetAttribute("segments", inputs[UGEG_SEGN]);
		elem->SetAttribute("start", inputs[UGEG_ST]);
		elem->SetAttribute("sustain", inputs[UGEG_SUS]);
		elem->SetAttribute("scale", inputs[UGEG_SCL]);

		int segs = (int) inputs[UGEG_SEGN];
		int index = UGEG_SEGLVL(0);
		while (segs > 0)
		{
			XmlSynthElem *child = elem->AddChild("seg");
			if (!child)
				return 1;
			child->SetAttribute("level", inputs[index++]);
			child->SetAttribute("rate", inputs[index++]);
			child->SetAttribute("curve", inputs[index++]);
			child->SetAttribute("time", inputs[index++]);
			segs--;
			delete child;
		}
		return 0;
	}
};
//@}
#endif
