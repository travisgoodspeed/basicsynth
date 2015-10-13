///////////////////////////////////////////////////////////////
// BasicSynth - Wave Table set
//
/// @file WaveTable.h Class to hold wave tables for oscillators
///
/// A single global instance of this class must be defined (@see wtSet)
/// if you intend to use the GenWaveWT class and its derivations.
// 
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpGeneral
//@{
#ifndef _WAVETABLE_H_
#define _WAVETABLE_H_

/// Index for Sine wave
#define WT_SIN 0
/// Index for Sawtooth wave (sum of partials)
#define WT_SAW 1
/// Index for Square wave (sum of partials)
#define WT_SQR 2
/// Index for Triangle wave (sum of partials)
#define WT_TRI 3
/// Index for Pulse wave (sum of partials)
#define WT_PLS 4
/// Index for LFO Sawtooth wave (interploated, not BW limited)
#define WT_SAWL 5
/// Index for LFO Square wave (interpolated, not BW limited)
#define WT_SQRL 6
/// Index for LFO Trianlge wave (interpolated, not BW limited)
#define WT_TRIL 7
/// Index for positive only Sawtooth wave (interpolated, not BW limited)
#define WT_SAWP 8
/// Index for positive only Triangle wave (interpolated, not BW limited)
#define WT_TRIP 9
/// Index for user-defined waveform
#define WT_USR(n) ((n)+10)

/// Structure to hold information about a wavetable.
/// The wavID member is used to lookup the table.
struct WaveTable
{
	AmpValue *wavTbl;
	bsInt32   wavID;
};

/// Global, pre-calculated waveform tables.
/// These waveforms are used by wavetable oscillators.
/// The first nine entries are pre-defined. Additional
/// entries can be defined by calling the SetWaveTable()
/// method. 
///
/// Since the wavetable array is a public member, it is allowable to initialize
/// the wavetable directly. Memory should be allocated using 
/// @code
/// int n = wavSet.GetFreeWavetable(id);
/// wavSet[n].wavTbl = new AmpValue[synthParams.itableLength+1];
/// @endcode
/// Although possible to replace the default waveforms,
/// this is not a good idea, especially for WT_SIN.
class WaveTableSet
{
public:
	/// pointer to the sine wave table (WT_SIN)
	AmpValue *wavSin;
	/// pointer to the square wave table (WT_SQR)
	AmpValue *wavSqr;
	/// pointer to the sawtooth wave table (WT_SAW)
	AmpValue *wavSaw;
	/// pointer to the triangle wave table (WT_TRI)
	AmpValue *wavTri;
	/// pointer to the pulse wave table (WT_PLS)
	AmpValue *wavPls;
	/// pointer to the sawtooth wave table created by direct calculation (WT_SAWL)
	AmpValue *lfoSaw;
	/// pointer to the triangle wave table created by direct calculation (WT_TRIL)
	AmpValue *lfoTri;
	/// pointer to the square wave table created by direct calculation (WT_SQRL)
	AmpValue *lfoSqr;
	/// pointer to the positive only sawtooth wave table created by direct calculation (WT_SAWP)
	AmpValue *posSaw;
	/// pointer to the positive only triangle wave table created by direct calculation (WT_TRIP)
	AmpValue *posTri;
	/// array of wavetables, wavTblMax in length
	WaveTable *wavSet;
	/// number of wavetables
	bsInt32 wavTblMax;

	WaveTableSet()
	{
		wavSin = NULL;
		wavSqr = NULL;
		wavSaw = NULL;
		wavTri = NULL;
		wavPls = NULL;
		lfoSqr = NULL;
		lfoSaw = NULL;
		lfoTri = NULL;
		posSaw = NULL;
		posTri = NULL;
		wavSet = NULL;
		wavTblMax = 0;
	}

	~WaveTableSet()
	{
		DestroyTables();
	}

	/// Destroy all wavetables
	void DestroyTables()
	{
		if (wavSet)
		{
			for (bsInt32 i = 0; i < wavTblMax; i++)
				delete wavSet[i].wavTbl;
			delete[] wavSet;
		}
		wavSet = NULL;
		wavTblMax = 0;
	}

	/// Set the number of wavetables.
	/// The wavetable array is reallocated and copied if needed.
	/// @param newSize number of wavetable entries (must be > WT_USR(0))
	/// @returns 0 on success, -1 on failure to allocate memory.
	int SetMax(bsInt32 newSize)
	{
		if (newSize < WT_USR(0))
			newSize = WT_USR(0);
		WaveTable *sav = wavSet;
		wavSet = new WaveTable[newSize];
		if (!wavSet)
		{
			wavSet = sav;
			return -1;
		}
		if (sav)
		{
			for (int n = 0; n < wavTblMax && n < newSize; n++)
			{
				wavSet[n].wavTbl = sav[n].wavTbl;
				wavSet[n].wavID  = sav[n].wavID;
			}
			delete[] sav;
		}
		while (wavTblMax < newSize)
		{
			wavSet[wavTblMax].wavTbl = 0;
			wavSet[wavTblMax].wavID = -1;
			wavTblMax++;
		}
		return 0;
	}

	/// Get the next available wavetable spot.
	/// This method does not check for duplicate entries.
	/// Use FindWavetable() first to determine if the ID
	/// has already been established.
	/// @param id ID of wavetable
	/// @returns index of empty wavetable or -1 if full
	bsInt32 GetFreeWavetable(bsInt32 id)
	{
		for (bsInt32 n = 0; n < wavTblMax; n++)
		{
			if (wavSet[n].wavID == -1)
			{
				wavSet[n].wavID = id;
				return n;
			}
		}
		return -1;
	}

	/// Find a wavetable index by ID.
	/// @param id wavetable ID
	/// @returns index of wavetable or -1 if not found
	bsInt32 FindWavetable(bsInt32 id)
	{
		for (bsInt32 n = 0; n < wavTblMax; n++)
		{
			if (wavSet[n].wavID == id)
				return n;
		}
		return -1;
	}

	/// Get wavetable by index.
	/// If the requested wavetable is not allocated the sin wave table is returned.
	/// @param ndx wave table index
	AmpValue *GetWavetable(bsInt32 ndx)
	{
		AmpValue *wt = 0;
		if (ndx >= 0 && ndx < wavTblMax)
			wt = wavSet[ndx].wavTbl;
		if (wt)
			return wt;
		return wavSin;
	}

	/// Initialize the default wavetables. 
	/// The length of wavetables is set by the synthParams itableLength member.
	/// An additional guard point is added to the end of all tables.
	/// Table values are normalized to the range[-1,+1].
	/// The guard point is set to the value at index 0
	/// to allow round-up of the table index without overflow.
	///
	/// The NUM_PARTS constant is set to allow oscillator frequencies 
	/// up to 2 octaves above middle C.
	/// Higher pitches will produce alias frequencies.
	/// For higher partials, or higher pitches, use GenWaveSum, or
	/// add bandwith limited waveforms using SetWaveTable().
	///
	/// When Init is called, existing wavetables are destroyed. 
	/// Thus any oscillators using wavetables MUST be deleted before calling Init.
	/// Generally, Init is called once at startup. However, in an
	/// interactive system, it may be necessary to call Init if
	/// the user desires to add new wavetables.
	/// @param wtUsr number of user-defined wavetables
	void Init(bsInt32 wtUsr = 0)
	{
		DestroyTables();
		SetMax(WT_USR(wtUsr));

		size_t allocSize = synthParams.itableLength+1;
		wavSin = new AmpValue[allocSize];
		wavSaw = new AmpValue[allocSize];
		wavSqr = new AmpValue[allocSize];
		wavTri = new AmpValue[allocSize];
		wavPls = new AmpValue[allocSize];
		lfoSaw = new AmpValue[allocSize];
		lfoSqr = new AmpValue[allocSize];
		lfoTri = new AmpValue[allocSize];
		posSaw = new AmpValue[allocSize];
		posTri = new AmpValue[allocSize];

#define NUM_PARTS 16
		double phsInc[NUM_PARTS];
		double phsVal[NUM_PARTS];
		int partNum;
		int partMax = 1;

		phsInc[0] = twoPI / (double) synthParams.ftableLength;
		phsVal[0] = 0.0f;
		for (partNum = 1; partNum < NUM_PARTS; partNum++)
		{
			phsInc[partNum] = phsInc[0] * (partNum+1);
			phsVal[partNum] = 0.0;
			if (phsInc[partNum] < PI)
				partMax++;
		}
		
		double sawValue = 0.0;
		double sawPeak = 0.0;

		double sqrValue;
		double sqrPeak = 0.0;

		double triValue;
		double triPeak = 0.0;

		double plsValue;
		double plsPeak = 0.0;

		double sigK = PI / partMax;
		double sigN;
		double sigma;
		double amp;

		double value;
		double partP1;

		bsInt16 index;
		for (index = 0; index < synthParams.itableLength; index++)
		{
			value = sin(phsVal[0]);
			wavSin[index] = (AmpValue) value;

			// direct calculation for LFO, not BW limited..
			if (phsVal[0] > PI)
				lfoSqr[index] = -1.0;
			else
				lfoSqr[index] = 1.0;

			lfoSaw[index] = (phsVal[0] / PI) - 1;
			posSaw[index] = (phsVal[0] / twoPI);
			lfoTri[index] = 1 - (2/PI * fabs(phsVal[0] - PI));
			posTri[index] = 1 - (fabs(phsVal[0] - PI) / PI);

			phsVal[0] += phsInc[0];

			// Calculate these by inverse Fourier transform
			sawValue = value;
			sqrValue = value;
			triValue = value;
			plsValue = value;

			sigN = sigK;
			partP1 = 2;
			for (partNum = 1; partNum < partMax; partNum++)
			{
				// no adjustment:
				//amp = 1.0 / partP1;
				// Adjustment with Lanczos sigma to minimize Gibbs phenomenon
				sigma = sin(sigN) / sigN; 
				plsValue += sin(phsVal[partNum]) * sigma;
				amp = sigma / partP1;
				sigN += sigK;
				value = sin(phsVal[partNum]) * amp;
				if (!(partNum & 1))
				{
					sawValue += value;
					sqrValue += value;
					triValue += ((cos(phsVal[partNum]) * sigma) / (partP1 * partP1));
				}
				else
					sawValue -= value;
				phsVal[partNum] += phsInc[partNum];
				partP1 += 1.0;
			}
			if (fabs(sawValue) > sawPeak)
				sawPeak = fabs(sawValue);
			if (fabs(sqrValue) > sqrPeak)
				sqrPeak = fabs(sqrValue);
			if (fabs(triValue) > triPeak)
				triPeak = fabs(triValue);
			if (fabs(plsValue) > plsPeak)
				plsPeak = fabs(plsValue);
			wavSaw[index] = (AmpValue) sawValue;
			wavSqr[index] = (AmpValue) sqrValue;
			wavTri[index] = (AmpValue) triValue;
			wavPls[index] = (AmpValue) plsValue;
		}

		// Normalize summed values
		for (index = 0; index < synthParams.itableLength; index++)
		{
			wavSaw[index] = wavSaw[index] / (AmpValue) sawPeak;
			wavSqr[index] = wavSqr[index] / (AmpValue) sqrPeak;
			wavTri[index] = wavTri[index] / (AmpValue) triPeak;
			wavPls[index] = wavPls[index] / (AmpValue) plsPeak;
		}

		// Set gaurd point for interpolation/round-up
		wavSin[synthParams.itableLength] = wavSin[0];
		wavSaw[synthParams.itableLength] = wavSaw[0];
		wavSqr[synthParams.itableLength] = wavSqr[0];
		wavTri[synthParams.itableLength] = wavTri[0];
		wavPls[synthParams.itableLength] = wavPls[0];
		lfoSaw[synthParams.itableLength] = lfoSaw[0];
		lfoSqr[synthParams.itableLength] = lfoSqr[0];
		lfoTri[synthParams.itableLength] = lfoTri[0];
		posSaw[synthParams.itableLength] = posSaw[0];
		posTri[synthParams.itableLength] = posTri[0];

		wavSet[WT_SIN].wavTbl = wavSin;
		wavSet[WT_SIN].wavID = WT_SIN;
		wavSet[WT_SAW].wavTbl = wavSaw;
		wavSet[WT_SAW].wavID = WT_SAW;
		wavSet[WT_SQR].wavTbl = wavSqr;
		wavSet[WT_SQR].wavID = WT_SQR;
		wavSet[WT_TRI].wavTbl = wavTri;
		wavSet[WT_TRI].wavID = WT_TRI;
		wavSet[WT_PLS].wavTbl = wavPls;
		wavSet[WT_PLS].wavID = WT_PLS;
		wavSet[WT_SAWL].wavTbl = lfoSaw;
		wavSet[WT_SAWL].wavID = WT_SAWL;
		wavSet[WT_SQRL].wavTbl = lfoSqr;
		wavSet[WT_SQRL].wavID = WT_SQRL;
		wavSet[WT_TRIL].wavTbl = lfoTri;
		wavSet[WT_TRIL].wavID = WT_TRIL;
		wavSet[WT_SAWP].wavTbl = posSaw;
		wavSet[WT_SAWP].wavID = WT_SAWP;
		wavSet[WT_TRIP].wavTbl = posTri;
		wavSet[WT_TRIP].wavID = WT_TRIP;
	}

	/// Set an indexed wave table. This is the method to fill in a user wavetable
	/// by summing a set of sinusoids. 	Table values are automatically normalized
	/// to the range [-1,+1]. Thus the actual amplitudes only need to be relative.
	/// E.G., you can set amplitudes as 10,5,1 etc. and still produce a wavetable
	/// with amplitudes in the range [-1,+1]. Phase values, if given, are in radians.
	///
	/// @param ti table index
	/// @param nparts number of partials
	/// @param mul array of partial numbers, cann be NULL if all partials 1 through n included
	/// @param amp array of partial relative amplitudes (required)
	/// @param phs array of phase offsets, NULL for all 0 phase
	/// @param gibbs turn gibbs correction on/off
	int SetWaveTable(bsInt32 ti, bsInt32 nparts, bsInt32 *mul, double *amp, double *phs, int gibbs)
	{
		if (ti < 0 || ti >= wavTblMax || amp == NULL)
			return -1;

		AmpValue *wavTable = wavSet[ti].wavTbl;
		if (wavTable == 0)
		{
			wavTable = new AmpValue[synthParams.itableLength+1];
			if (wavTable == NULL)
				return -1;
			wavSet[ti].wavTbl = wavTable;
		}

		double *phsVal = new double[nparts];
		double *phsInc = new double[nparts];
		double *sigma = new double[nparts];

		double incr = twoPI / (double) synthParams.ftableLength;
		int index = 0;
		int mulMax = 0;
		int partNum;
		int partMax = 0;
		for (partNum = 0; partNum < nparts; partNum++)
		{
			if (mul != NULL)
			{
				if (mul[partNum] > mulMax)
					mulMax = mul[partNum];
				phsInc[partNum] = incr * mul[partNum];
			}
			else
				phsInc[partNum] = incr * (partNum + 1);
			if (phsInc[partNum] < PI)
				partMax++;
			if (phs)
				phsVal[partNum] = phs[partNum];
			else
				phsVal[partNum] = 0.0;
		}

		if (mulMax == 0)
			mulMax = partMax;
		double value;
		double maxvalue = 0.00001;
		double sigK = PI / (double) mulMax;
		for (partNum = 0; partNum < partMax; partNum++)
		{
			sigma[partNum] = 1.0;
			if (gibbs)
			{
				if (mul)
					value = mul[partNum] * sigK;
				else
					value = (double) partNum * sigK;
				if (value > 0)
					sigma[partNum] = sin(value) / value;
			}
		}

		for (index = 0; index < synthParams.itableLength; index++)
		{
			value = 0;
			for (partNum = 0; partNum < partMax; partNum++)
			{
				if (amp[partNum] != 0)
				{
					value += sin(phsVal[partNum]) * amp[partNum] * sigma[partNum];
					phsVal[partNum] += phsInc[partNum];
				}
			}
			wavTable[index] = (AmpValue) value;
			if (fabs(value) > maxvalue)
				maxvalue = fabs(value);
		}

		// Normalize summed values
		for (index = 0; index < synthParams.itableLength; index++)
			wavTable[index] = wavTable[index] / (AmpValue) maxvalue;

		wavTable[synthParams.itableLength] = wavTable[0];
		delete phsVal;
		delete phsInc;
		delete sigma;

		return 0;
	}

	/// Set an indexed wave table. This is the method to fill in a user wavetable
	/// from a set of linear segments. Each segment is defined by a length and
	/// a level. The length is a fraction of the table length, i.e., 0.25 indicates
	/// one-fourth of the table length. The total values of the lengths must sum to 1.0.
	/// The level values are used as-is. Generally the levels should range [-1,+1] but
	/// can be specified as any values. The start value is 0, but a different start can
	/// be created by setting the length of the first segment to 0.
	///
	/// @param ti table index
	/// @param nsegs number of segments
	/// @param len array of lengths
	/// @param val array of levels
	int SegWaveTable(bsInt32 ti, bsInt32 nsegs, double *len, double *val)
	{
		if (ti < 0 || ti >= wavTblMax || val == 0 || len == 0)
			return -1;

		AmpValue *wavTable = wavSet[ti].wavTbl;
		if (wavTable == 0)
		{
			wavTable = new AmpValue[synthParams.itableLength+1];
			if (wavTable == NULL)
				return -1;
			wavSet[ti].wavTbl = wavTable;
		}

		bsInt32 index = 0;
		double level = 0.0;
		for (int ns = 0; ns < nsegs; ns++)
		{
			bsInt32 count = (bsInt32) (synthParams.ftableLength * len[ns]);
			if (count > 0)
			{
				double incr = (val[ns] - level) / (double) count;
				while (count > 0 && index < synthParams.itableLength)
				{
					wavTable[index++] = level;
					level += incr;
					count--;
				}
			}
			level = val[ns];
		}
		while (index < synthParams.itableLength)
			wavTable[index++] = level;
		wavTable[synthParams.itableLength] = wavTable[0];
		return 0;
	}

	/// Get sin(ndx) using interpolation of a table.
	/// @param ndx current table index (phase)
	/// @return sin value as double precision
	inline double SinWT(PhsAccum ndx)
	{
		int intIndex = (int) ndx; // floor(ndx)
		double fract = ndx - (double) intIndex;
		double v1 = (double) wavSin[intIndex];
		double v2 = (double) wavSin[intIndex+1];
		return v1 + ((v2 - v1) * fract);
	}

	/// Get cos(ndx) using interpolation of a table.
	/// @param ndx current table index (phase)
	/// @return sin value as double precision
	inline double CosWT(PhsAccum ndx)
	{
		// add PI/2 radians to phase
		ndx += synthParams.ftableLength / 4.0;
		if (ndx >= synthParams.ftableLength)
			ndx -= synthParams.ftableLength;
		return SinWT(ndx);
	}
};

/// Global wavetable object. This global must be allocated somewhere. It is shared
/// by all wavetable oscillators. Typically, it is defined by including the
/// common library and initialized by the InitSynthesizer() method. It is also
/// possible to define your own global variable and/or initialize it as you see fit.
extern WaveTableSet wtSet;

//@}
#endif
