//////////////////////////////////////////////////////////////////////
/// @file UGCalc.h BasicSynth Pseudo-UGs that perform calculations or output values
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef UG_CALC_H
#define UG_CALC_H

extern UGParam valueParam[];
#define UGVAL_INP 0

/// @brief Store a parameter value
/// @details The value UG is used internally to 
/// represent an event parameter: frequency, duration, or volume
class UGValue : public ModSynthUGImpl<UGValue, GenUnit, 1>
{
private:

public:
	const char *GetType()
	{
		return "VALUE";
	}

	const UGParam *GetParamList()
	{
		return &valueParam[0];
	}

	UGValue()
	{
	}

	void Start()
	{
		anyChange = 0;
		out = inputs[UGVAL_INP];
		Send(out, UGP_INIT);
	}

	void Tick()
	{
		if (anyChange)
		{
			out = inputs[UGVAL_INP];
			Send(out, UGP_GEN);
			anyChange = 0;
		}
	}
};

extern UGParam valOut[];

#define UGOUT_INP  0
#define UGOUT_LFT  1
#define UGOUT_RGT  2
#define UGOUT_VOL  3
#define UGOUT_PAN  4
#define UGOUT_PON  5
#define UGOUT_NUMINP 6

/// @brief Output unit
/// @details The out UG accumulates samples for one tick
/// and then sends them to the mixer (via the instrument manager).
/// Out includes an optional internal panner for dynamic panning functions.
class UGOut : public ModSynthUGImpl<UGOut, GenUnit, UGOUT_NUMINP>
{
public:
	Panner panSet;
	int panOn;
	AmpValue lftOut;
	AmpValue rgtOut;

public:
	const char *GetType()
	{
		return "OUT";
	}

	const UGParam *GetParamList()
	{
		return valOut;
	}

	UGOut()
	{
		panOn = 0;
		out = 0;
		lftOut = 0;
		rgtOut = 0;
	}

	void InitDefault()
	{
		inputs[UGOUT_VOL] = 1.0f;
		inputs[UGOUT_PAN] = 0.0f;
		inputs[UGOUT_PON] = 0.0f;
	}

	// we set the inputs[] array for save/load
	// we set the internal values for performance
	void SetInput(short index, float val)
	{
		switch (index)
		{
		case UGOUT_INP:
		case UGOUT_LFT:
		case UGOUT_RGT:
			inputs[index] += val;
			break;
		case UGOUT_VOL:
			inputs[UGOUT_VOL] = val;
			break;
		case UGOUT_PAN:
			inputs[UGOUT_PAN] = val;
			panSet.Set(panTrig, val);
			break;
		case UGOUT_PON:
			inputs[UGOUT_PON] = val;
			panOn = (int) val;
			break;
		}
	}

	void Start()
	{
		out = 0;
		lftOut = 0;
		rgtOut = 0;
		inputs[UGOUT_INP] = 0.0f;
		inputs[UGOUT_LFT] = 0.0f;
		inputs[UGOUT_RGT] = 0.0f;
	}

	int IsFinished()
	{
		return 1; 
	}

	void Tick() 
	{
		float vol = inputs[UGOUT_VOL];
		float inp = inputs[UGOUT_INP];
		if (panOn)
		{
			lftOut = vol * (inputs[UGOUT_LFT] + (inp * panSet.panlft));
			rgtOut = vol * (inputs[UGOUT_RGT] + (inp * panSet.panrgt));
			inputs[UGOUT_LFT] = 0.0f;
			inputs[UGOUT_RGT] = 0.0f;
		}
		else
			out = inp * vol;
		inputs[UGOUT_INP] = 0.0f;
	}

	void Output(InstrManager *mgr, int ch)
	{
		if (panOn)
			mgr->Output2(ch, lftOut, rgtOut);
		else
			mgr->Output(ch, out);
	}
};

extern UGParam calcParams[];

#define UGCALC_V1  0
#define UGCALC_V2  1
#define UGCALC_OP  2

#define UGOP_ADD   1 // v1+v2
#define UGOP_SUB   2 // v1-v2
#define UGOP_MUL   3 // v1*v2
#define UGOP_DIV   4 // v1/v2
#define UGOP_MOD   5 // v1 % v2
#define UGOP_EXP   6 // v1^v2
#define UGOP_SQR   7 // v1 * sqrt(v2)
#define UGOP_LOG   8 // log(v1) * v2
#define UGOP_EXE   9 // e^v1
#define UGOP_LGN  10 // ln(v1) * v2
#define UGOP_AVG  11 // (v1+v2) / 2
#define UGOP_SUM  12 // out += v1, out *= v2
#define UGOP_MIN  13 // min(v1, v2)
#define UGOP_MAX  14 // max(v1, v2)
#define UGOP_SIN  15 // v1 * sin(v2)
#define UGOP_COS  16 // v1 * cos(v2)
#define UGOP_D2R  17 // (v1 / 360) * twoPI
#define UGOP_F2R  18 // (v1 / SR) * twoPI
#define UGOP_HYP  19 // sqrt((v1*v1)+(v2*v2))
#define UGOP_LT   20 // v1 < v2
#define UGOP_LE   21 // v1 <= v2
#define UGOP_GT   22 // v1 > v2
#define UGOP_GE   23 // v1 >= v2
#define UGOP_EQ   24 // v1 == v2
#define UGOP_NE   25 // v1 != v2
#define UGOP_WRP  26 // set: 0 <= v1 < v2
#define UGOP_SHFT 27 // y[n] = x[n-1], x[n-1] = x[n]
#define UGOP_FLOOR 28 // floor(val1)
#define UGOP_FRACT 29 // val1 - floor(val1)

/// @brief Calculator unit
/// @details The calculator performs one of several calculations.
/// Although possible, it's not a good idea to change the
/// opcode during playback.
class UGCalc : public ModSynthUGImpl<UGCalc, GenUnit, 3>
{
public:

public:
	const char *GetType()
	{
		return "CALC";
	}

	const UGParam *GetParamList()
	{
		return calcParams;
	}

	UGCalc()
	{
	}

	void Start()
	{
		anyChange = 0;
		if ((int)inputs[2] == UGOP_SUM)
			inputs[0] = 0;
		else
			CalcValue();
		Send(out, UGP_INIT);
	}

	int IsFinished()
	{
		return 1;
	}

	void SetInput(short index, float val)
	{
		if (index == 0 && (int)inputs[2] == UGOP_SUM)
		{
			anyChange |= 1;
			inputs[0] += val;
		}
		else if (index >= 0 && index < 3)
		{
			inputs[index] = val;
			anyChange |= 1 << index;
		}
	}

	void Tick() 
	{
		if (anyChange)
		{
			anyChange = 0;
			CalcValue();
			Send(out, UGP_GEN);
		}
	}

	void CalcValue()
	{
		int op = (int) inputs[2];
		switch(op)
		{
		case UGOP_ADD: // v1 + v2
			out = inputs[0] + inputs[1];
			break;
		case UGOP_SUB: // v1 - v2
			out = inputs[0] - inputs[1];
			break;
		case UGOP_MUL: // v1 * v2
			out = inputs[0] * inputs[1];
			break;
		case UGOP_DIV: // v1 / v2
			if (inputs[1] != 0.0f)
				out = inputs[0] / inputs[1];
			else
				out = 0.0;
			break;
		case UGOP_MOD: // v1 mod v2
			out = fmod(inputs[0], inputs[1]);
			break;
		case UGOP_EXP: // v1^v2
			out = pow(inputs[0], inputs[1]);
			break;
		case UGOP_SQR:
			out = inputs[1] * sqrt(inputs[0]);
			break;
		case UGOP_EXE: // e^v1 * v2
			out = inputs[1] * exp(inputs[0]);
			break;
		case UGOP_LOG: // pow
			out = inputs[1] * log10(inputs[0]);
			break;
		case UGOP_LGN: // v2 * loge(v1)
			out = inputs[1] * log(inputs[0]);
			break;
		case UGOP_AVG: // avg
			out = (inputs[0] + inputs[1]) / 2;
			break;
		case UGOP_SUM: // sum(v1) * v2
			out = inputs[0] * inputs[1];
			inputs[0] = 0;
			break;
		case UGOP_MIN: // min(v1, v2)
			if (inputs[0] < inputs[1])
				out = inputs[0];
			else
				out = inputs[1];
			break;
		case UGOP_MAX: // max(v1, v2)
			if (inputs[0] > inputs[1])
				out = inputs[0];
			else
				out = inputs[1];
			break;
		case UGOP_SIN: // v2 * sin(v1)
			out = inputs[1] * sin(inputs[0]);
			break;
		case UGOP_COS: // v2 * cos(v1)
			out = inputs[1] * cos(inputs[0]);
			break;
		case UGOP_D2R: // (v1 / 360) * twoPI
			out = (inputs[0] / 360.0f) * twoPI;
			break;
		case UGOP_F2R: // (v1 / SR) * twoPI
			out = (inputs[0] / synthParams.sampleRate) * twoPI;
			break;
		case UGOP_HYP: // sqrt(v1*v1 + v2*v2)
			out = sqrt((inputs[0]*inputs[0]) + (inputs[1]*inputs[1]));
			break;
		case UGOP_LT: // v1 < v2
			out = (inputs[0] < inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_LE: // v1 <= v2
			out = (inputs[0] <= inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_GT: // v1 > v2
			out = (inputs[0] > inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_GE: // v1 >= v2
			out = (inputs[0] >= inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_EQ: // v1 == v2
			out = (inputs[0] == inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_NE: // v1 != v2
			out = (inputs[0] != inputs[1]) ? 1.0f : 0.0f;
			break;
		case UGOP_WRP:
			out = inputs[0];
			while (out >= inputs[1])
				out -= inputs[1];
			while (out < 0)
				out += inputs[1];
			break;
		case UGOP_SHFT:
			out = inputs[1];
			inputs[1] = inputs[0];
			break;
		case UGOP_FLOOR:
			out = floor(inputs[0]);
			break;
		case UGOP_FRACT:
			out = inputs[0] - floor(inputs[0]);
			break;
		default:
			out = 0.0f;
			break;
		}
	}
};

#define UGSCL_INP 0
#define UGSCL_OMX 1
#define UGSCL_OMN 2
#define UGSCL_IMX 3
#define UGSCL_IMN 4
extern UGParam scaleParams[];

/// @brief Scale a value
/// The scale operator converts an input range to an output
/// range using the equation:
/// @code
///                      (outmax - outmin)
/// out = (in - inmin) * ----------------- + outmin
///                       (inmax - inmin)
/// @endcode
class UGScale : public ModSynthUGImpl<UGScale, GenUnit, 5>
{
public:
	float scale;
public:
	const char *GetType()
	{
		return "SCALE";
	}

	const UGParam *GetParamList()
	{
		return scaleParams;
	}

	UGScale()
	{
		scale = 1.0f;
	}

	void Start()
	{
		anyChange = 0x1F;
		CalcValue();
		Send(out, UGP_INIT);
	}

	int IsFinished()
	{
		return 1;
	}

	void Tick() 
	{
		if (anyChange)
		{
			CalcValue();
			Send(out, UGP_GEN);
		}
	}

	void CalcValue()
	{
		if (anyChange & 0x1E)
		{
			float range = inputs[UGSCL_IMX] - inputs[UGSCL_IMN];
			if (range != 0.0f)
				scale = (inputs[UGSCL_OMX] - inputs[UGSCL_OMN]) / range;
			else
				scale = 0.0f;
		}
		anyChange = 0;
		out = ((inputs[UGSCL_INP] - inputs[UGSCL_IMN]) * scale) + inputs[UGSCL_OMN];
	}

};

#define UGTBL_NDX  0
#define UGTBL_WVT  1
#define UGTBL_INT  2
extern UGParam tableParams[];

class UGTable : public ModSynthUGImpl<UGTable, GenUnit, 3>
{
public:
	float *table;
public:
	const char *GetType()
	{
		return "TABLE";
	}

	const UGParam *GetParamList()
	{
		return tableParams;
	}

	UGTable()
	{
		table = wtSet.wavSin;
	}

	void Start()
	{
		anyChange = 3;
		CalcValue();
		Send(out, UGP_INIT);
	}

	int IsFinished()
	{
		return 1;
	}

	void Tick() 
	{
		if (anyChange)
		{
			CalcValue();
			anyChange = 0;
			Send(out, UGP_GEN);
		}
	}

	void CalcValue()
	{
		if (anyChange & 2) // table id
			table = wtSet.GetWavetable(wtSet.FindWavetable((bsInt32) inputs[UGTBL_WVT]));
		float index = inputs[UGTBL_NDX];
		while (index > synthParams.ftableLength)
			index -= synthParams.ftableLength;
		while (index < 0)
			index += synthParams.ftableLength;
		if ((int)inputs[UGTBL_INT] != 0) // interpolate
		{
			float ipart = floor(index);
			float fract = index - ipart;
			int ii = (int) ipart;
			float v1 = table[ii];
			float v2 = table[ii+1];
			out = v1 + ((v2 - v1) * fract);
		}
		else
		{
			out = table[(int)(index+0.5)];
		}
		anyChange = 0;
	}
};
//@}
#endif
