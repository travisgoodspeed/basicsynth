///////////////////////////////////////////////////////////////
// BasicSynth -
//
/// @file EnvGenSeg.h Segment based envelope generators.
/// These have variable segments with release
///
///  - AR
///  - ADSR
///  - Multi-Seg with fixed sustain
///  - Multi-seg with variable sustain
///  - Pre-calculated table
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
///////////////////////////////////////////////////////////////
/// @addtogroup grpEnv
//@{
#ifndef _ENVGENSEG_H_
#define _ENVGENSEG_H_

/// Curve types for the multi-segment envelope generators.
enum EGSegType
{
	nulSeg = 0, /// undefined
	linSeg = 1, /// linear
	expSeg = 2, /// exponential
	logSeg = 3, /// logarithmic
	susSeg = 4, /// sustain - constant value
	sqrSeg = 5, /// x^2 concave curve
};

///////////////////////////////////////////////////////////
/// A Sustain segment. This segment always returns the peak value.
///////////////////////////////////////////////////////////
class EnvSeg : public GenUnit
{
protected:
	long  count;
	AmpValue value;
	AmpValue start;
	AmpValue end;
	FrqValue rate;

public:
	EnvSeg()
	{
		count = 0;
		value = 0;
		start = 0;
		end = 0;
		rate = 0;
	}

	/// Get segment type.
	/// @return EGSegType::susSeg
	virtual EGSegType GetType()
	{
		return susSeg;
	}

	/// Initialize the envelope segment. The values are taken from the value array \n
	/// v[0] = rate \n
	/// v[1] = start \n
	/// v[2] = end \n
	/// @param n number of values (3)
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n >= 3)
			InitSeg(FrqValue(v[0]), AmpValue(v[1]), AmpValue(v[2]));
	}

	/// Set the start level.
	inline void SetStart(AmpValue s) { start = s; }
	/// Set the end level. 
	inline void SetLevel(AmpValue s) { end = s; }
	/// Set the rate (time for the segment).
	inline void SetRate(FrqValue r)
	{ 
		rate = r; 
		count = (long) (rate * synthParams.sampleRate);
	}

	/// Initialize the segment. Sets the segment with explicit arguments
	/// Values can also be set with the Set/Get methods. The InitSeg
	/// method will also reset the envelope to its start point.
	/// @param r rate (time for this segment)
	/// @param s start level
	/// @param e end level
	virtual void InitSeg(FrqValue r, AmpValue s, AmpValue e)
	{
		rate = r;
		start = s;
		end = e;
		count = (long) (rate * synthParams.sampleRate);
		Reset();
	}

	/// Get the start level
	inline AmpValue GetStart() { return start; }
	/// Get the end level
	inline AmpValue GetLevel() { return end; }
	/// Get the rate
	inline FrqValue GetRate()  { return rate; }

	/// Return the envelope settings.
	/// @param r rate
	/// @param s start value
	/// @param e end value
	virtual void GetSettings(FrqValue& r, AmpValue& s, AmpValue& e)
	{
		r = rate;
		s = start;
		e = end;
	}

	/// Initialize the envelope segment. The values are copied from the supplied object.
	/// @param tp source object
	virtual void Copy(EnvSeg *tp)
	{
		rate = tp->rate;
		start = tp->start;
		end = tp->end;
	}

	/// Initialize the segment. Like InitSeg, but using samples for the rate.
	/// @param r rate (time for this segment)
	/// @param s start level
	/// @param e end level
	/// @sa InitSeg
	virtual void InitSegTick(long r, AmpValue s, AmpValue e)
	{
		rate = (FrqValue)r * synthParams.sampleRate;
		count = r;
		start = s;
		end = e;
		Reset(0);
	}

	/// Reset the envelope segment. The phase argument is ignored.
	/// @param initPhs ignored
	virtual void Reset(float initPhs = 0)
	{
		value = start;
	}

	/// Get the current value.	This returns the current value
	///  of the generator without moving forward to the next value.
	/// @return current amplitude value
	virtual AmpValue Value()
	{
		return value;
	}

	/// Generate the next sample. This moves to the next value
	///  and multiplies it by the input amplitude value.
	/// @param in amplitude scale value
	/// @return next amplitude value
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Generate the next value. This moves to the next value in the segment
	/// @return next amplitude value
	virtual AmpValue Gen()
	{
		count--;
		return value;
	}

	/// Test if the envelope segment is finished. When the segment reaches the
	/// end, this return true. This is used by multi-segment generators to
	/// determine when to move to the next segment. It is also used 
	/// by instruments and sequencers to determine when the envelope release
	/// cycle is complete.
	/// @return true if segment is completed
	virtual int IsFinished()
	{
		return count <= 0;
	}
};

///////////////////////////////////////////////////////////
/// Linear envelope segment. An envelope segment generated
/// using the equation for a line: y = ax + b
/// @sa EnvSeg
///////////////////////////////////////////////////////////
class EnvSegLin : public EnvSeg
{
protected:
	AmpValue incr;

public:
	EnvSegLin()
	{
		incr = 1;
	}

	/// Get the type.
	/// @return EgSegType::linSeg
	virtual EGSegType GetType()
	{ 
		return linSeg;
	}

	/// @copydoc EnvSeg::Reset()
	virtual void Reset(float initPhs = 0)
	{
		EnvSeg::Reset(initPhs);
		incr = (end - start);
		if (count > 0)
			incr /= (AmpValue) count;
	}

	/// @copydoc EnvSeg::Gen()
	virtual AmpValue Gen()
	{
		if (--count > 0)
			return value += incr;
		return end;
	}

};

///////////////////////////////////////////////////////////
/// Exponential curve envelope segment. An envelope segment
/// generated using the equation: y = ab^x + c
/// @sa EnvSeg
///////////////////////////////////////////////////////////
class EnvSegExp : public EnvSeg
{
protected:
	AmpValue range;
	AmpValue bias;
	AmpValue offs;
	AmpValue incr;

public:
	EnvSegExp()
	{
		bias = 0.2;
		range = 1;
		offs = 0;
		incr = 0;
	}

	/// Get the segment type
	/// @return EGSegType::expSeg
	virtual EGSegType GetType()
	{ 
		return expSeg;
	}

	/// @copydoc EnvGenExp::SetBias
	void SetBias(float b)
	{
		bias = (AmpValue)b;
	}

	/// @copydoc EnvSeg::Reset()
	virtual void Reset(float initPhs = 0)
	{
		EnvSeg::Reset(initPhs);
		range = fabs(end - start);
		if (count > 0)
		{
			AmpValue ratio;
			if (end >= start)
			{
				value = bias;
				ratio = (1+bias)/bias;
				offs = start;
			}
			else
			{
				value = 1.0+bias;
				ratio = bias/value;
				offs = end;
			}
			incr = (AmpValue) pow(ratio, 1.0f / (float) count);
		}
		else
			incr = 1;
	}

	/// @copydoc EnvSeg::Gen()
	virtual AmpValue Gen()
	{
		if (--count > 0)
		{
			value *= incr;
			return ((value - bias) * range) + offs;
		}
		return end;
	}
};

///////////////////////////////////////////////////////////
/// Logarithmic curve envelope segment. An envelope segment
/// generated using the equation: y = a(1 - (b^x)) + c
/// @sa EnvSeg EnvSegExp
///////////////////////////////////////////////////////////
class EnvSegLog : public EnvSegExp
{
protected:

public:
	virtual EGSegType GetType()
	{ 
		return logSeg; 
	}

	/// @copydoc EnvSeg::Reset
	virtual void Reset(float initPhs = 0)
	{
		count = (long) (rate * synthParams.sampleRate);
		value = 0;
		range = fabs(end - start);
		if (count > 0)
		{
			AmpValue ratio;
			if (end >= start)
			{
				value = 1+bias;
				ratio = bias/value;
				offs = start;
			}
			else
			{
				value = bias;
				ratio = (1+bias)/bias;
				offs = end;
			}
			incr = (AmpValue) pow(ratio, 1.0f / (float) count);
		}
		else
			incr = 1;
	}

	/// @copydoc EnvSeg::Gen
	virtual AmpValue Gen()
	{
		if (--count > 0)
		{
			value *= incr;
			return ((1.0 - (value - bias)) * range) + offs;
		}
		return end;
	}
};

///////////////////////////////////////////////////////////
/// Square curve envelope segment. 
/// This envelope is generated by squaring the value
/// of a normalized linear envelope, i.e. 
/// y = a + x^2 * b, 0 <= x <= 1
/// @sa EnvSeg
///////////////////////////////////////////////////////////
class EnvSegSqr : public EnvSeg
{
private:
	AmpValue range;
	AmpValue incr;

public:
	EnvSegSqr()
	{
		incr = 1.0;
		range = 1.0;
	}

	/// Get the type.
	/// @return EgSegType::linSeg
	virtual EGSegType GetType()
	{ 
		return sqrSeg;
	}

	virtual void Reset(float initPhs = 0)
	{
		EnvSeg::Reset(initPhs);
		range = (end - start);
		if (count > 0)
			incr = 1.0 / (AmpValue) count;
		else
			incr = 1.0;
		value = 0.0;
	}

	/// @copydoc EnvSeg::Gen()
	virtual AmpValue Gen()
	{
		if (--count > 0)
		{
			value += incr;
			return start + (range * (value * value));
		}
		return end;
	}
};

/// Structure to hold values for an envelope segment.
struct SegVals
{
	/// Time in seconds.
	FrqValue rate;
	/// End level.
	AmpValue level;
	/// Curve type
	EGSegType type;
	/// Fixed/relative duration
	int fixed;
	/// calculated rate
	FrqValue crt;
};

///////////////////////////////////////////////////////////
/// Structure to hold values for an envelope generator.
/// This class provides a dynamic array of SegVals,
/// (level,rate,type) tuples. The multi-segment envelope generators
/// can be initialized by passing an object of this type
/// to the SetEnvDef member function.
///////////////////////////////////////////////////////////
struct EnvDef
{
	int nsegs;
	int suson;
	AmpValue start;
	SegVals *segs;

	EnvDef()
	{
		nsegs = 0;
		start = 0;
		suson = 1;
		segs = NULL;
	}

	~EnvDef()
	{
		delete segs;
	}

	/// Allocate space for segments.
	/// Existing values are discarded. The starting value and sustain-on flags
	/// are also set here.
	/// @param n number of segments
	/// @param s starting level for the envelope
	/// @param so sustain-on flag (1=sustain, 0=no sustain)
	void Alloc(int n, AmpValue s, int so = 1)
	{
		Clear();
		nsegs = n;
		start = s;
		suson = 1;
		if (n > 0)
			segs = new SegVals[n];
	}

	/// Get the number of segments allocated.
	int NumSegs()
	{ 
		return nsegs; 
	}

	/// Set the starting value for the envelope.
	/// @param st starting value
	inline void SetStart(AmpValue st)
	{ 
		start = st; 
	}
	/// Set the rate for segment n
	/// @param n segment
	/// @param rt rate (time from start to end)
	inline void SetRate(int n, FrqValue rt)
	{ 
		if (n < nsegs)
			segs[n].rate = rt;
	}
	/// Set the level for segment n
	/// @param n segment
	/// @param lv end level
	inline void SetLevel(int n, AmpValue lv)
	{ 
		if (n < nsegs)
			segs[n].level = lv; 
	}

	/// Set the type for segment n
	/// @param n segment
	/// @param ty segment type
	inline void SetType(int n, EGSegType ty)
	{ 
		if (n < nsegs)
			segs[n].type = ty; 
	}

	/// Set the fixed/relative duration flag
	inline void SetFixed(int n, int f)
	{
		if (n < nsegs)
			segs[n].fixed = f;
	}

	/// Get the starting value
	inline AmpValue GetStart()
	{ 
		return start; 
	}
	/// Get the rate for segment \p n
	inline FrqValue GetRate(int n)
	{ 
		if (n < nsegs)
			return segs[n].rate; 
		return 0.0;
	}
	/// Get the level for segment \p n
	inline AmpValue GetLevel(int n)
	{ 
		if (n < nsegs)
			return segs[n].level; 
		return 0.0;
	}
	/// Get the type for segment \p n
	inline EGSegType GetType(int n)
	{ 
		if (n < nsegs)
			return segs[n].type; 
		return nulSeg;
	}

	/// Get the fixed flag for segment \p n
	inline int GetFixed(int n)
	{ 
		if (n < nsegs)
			return segs[n].fixed; 
		return 0;
	}

	/// Set one segment.
	/// Set the rate, level and type for a segment.
	/// The Alloc method must be called before setting a segment.
	/// @param n segment number 
	/// @param rt rate
	/// @param lv level
	/// @param ty type (EGSegType)
	/// @param fix fixed time or relative to duration
	void Set(int n, FrqValue rt, AmpValue lv, EGSegType ty, int fix = 1)
	{
		if (n < nsegs)
		{
			segs[n].rate = rt;
			segs[n].level = lv;
			segs[n].type = ty;
			segs[n].fixed = fix;
			segs[n].crt = rt;
		}
	}

	/// Get one segment.
	/// Get the rate, level and type for a segment
	/// @param n segment number 
	/// @param rt rate
	/// @param lv end level
	/// @param ty type (EGSegType)
	/// @param fix fixed time or relative to duration
	void Get(int n, FrqValue& rt, AmpValue& lv, EGSegType& ty, int& fix)
	{
		if (n < nsegs)
		{
			rt = segs[n].rate;
			lv = segs[n].level;
			ty = segs[n].type;
			fix = segs[n].fixed;
		}
	}

	/// Initialize from a copy. Copy the envelope segment settings from the
	/// object dp.
	/// @param dp source object
	void Copy(EnvDef *dp)
	{
		Alloc(dp->nsegs, dp->start, dp->suson);
		if (nsegs > 0)
			memcpy(segs, dp->segs, nsegs*sizeof(SegVals));
	}

	/// Clear the segments array.
	void Clear()
	{
		if (segs)
			delete segs;
		nsegs = 0;
		start = 0;
		suson = 0;
		segs = NULL;
	}
};

///////////////////////////////////////////////////////////
// Class definitions for multi-segment envelope generators
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
/// Multi-segment Envelope generator.
/// This is the interface definition for multi-segment envelope generators.
/// It defines the common methods, but provides no implementation except 
/// for the Sample method.
///////////////////////////////////////////////////////////
class EnvGenUnit : public GenUnit
{
public:
	/// Generate the next value. This method calculates
	/// the next value and steps to the next segment
	/// when the current segment is finished. When the end
	/// of the envelope is reached, the final value is
	/// returned until Reset() is called.
	virtual AmpValue Gen() { return 0.0; }

	/// Generate the next value and multiply by the
	/// current sample value.
	/// @param in current sample amplitude value
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Move to the release segment.
	virtual void Release() { }

	/// Return the envelope settings in an EnvDef structure.
	/// @param def envelope definition
	virtual void GetEnvDef(EnvDef *def) { }

	/// Initialize the envelope from an EnvDef structure.
	/// @param def envelope definition
	virtual void SetEnvDef(EnvDef *def) { }

	/// Initialize from a copy.
	/// The envelope values are copied from the source object.
	/// @param tp pointer to source object
	virtual void Copy(EnvGenUnit *tp) { }
};

///////////////////////////////////////////////////////////
/// Variable multi-segment envelope generator
///
/// The array of segment values is dynamic, allowing any
/// number of segments. Each segment has a rate, level and
/// type, thus allowing a mixture of linear, exponential,
/// log curves. The segObj array is initialized to 
/// the appropriate curve generator when the SetType
/// method is called.
///
/// This class does not implement indeterminate sustain. 
/// However, it is possible to create a fixed duration 
/// sustain segment by setting the start and end points
/// equal for one of the segments and setting the type
/// to susSeg. 
//
/// The AR and ADSR classes derive from this class
/// and implement a state-machine that stops at the sustain
/// segment if it is set to on.
///////////////////////////////////////////////////////////
class EnvGenSeg : public EnvGenUnit
{
protected:
	int numSeg;
	int index;
	int susOn;
	SegVals *segRLT;
	EnvSeg **segObj;

	EnvSegLin egsLin;
	EnvSegExp egsExp;
	EnvSegLog egsLog;
	EnvSegSqr egsSqr;
	EnvSeg    egsSus;
	EnvSeg *seg;

	AmpValue lastVal;
	AmpValue segStart;
	FrqValue duration;

public:
	EnvGenSeg()
	{
		numSeg = 0;
		index = 0;
		lastVal = 0;
		susOn = 0;
		segStart = 0;
		segObj = NULL;
		seg = NULL;
		segRLT = NULL;
		duration = 0;
	}

	virtual ~EnvGenSeg()
	{
		delete segRLT;
		delete segObj;
	}

	/// @copydoc EnvGenUnit::Copy()
	virtual void Copy(EnvGenUnit *tp)
	{
		EnvGenSeg *ap = (EnvGenSeg *)tp;
		SetStart(ap->segStart);
		SetSegs(ap->numSeg);
		susOn = ap->susOn;
		duration = ap->duration;
		SegVals *sv = ap->segRLT;
		for (int n = 0; n < ap->numSeg; n++, sv++)
			SetSegN(n, sv->rate, sv->level, sv->type, sv->fixed);
	}

	/// Initialize envelope generator.
	/// The first array value is the starting level for the envelope. Each
	/// following tuple of three values contains the rate, end level, and type for
	/// one segment. The number of segments is thus (n - 1)/3
	/// v[0] = starting level for the envelope
	/// v[1...n] = array of rate, level, type tuples
	//      L0,{R1,L1.T1}..{Rn,Ln,Tn,Fn}
	/// @param n number of values
	/// @param v array of values
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			segStart = *v++;
			n--;
			SetSegs(n/4);
			for (int i = 0; i < numSeg; i++)
			{
				SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2], (int) v[3]);
				v += 4;
			}
		}
		Reset();
	}

	/// @copydoc EnvGenUnit::SetEnvDef()
	virtual void SetEnvDef(EnvDef *def)
	{
		SetSegs(def->nsegs);
		SetStart(def->start);
		susOn = def->suson;
		SegVals *sv = def->segs;
		for (int n = 0; n < numSeg; n++, sv++)
			SetSegN(n, sv->rate, sv->level, sv->type, sv->fixed);
	}

	/// @copydoc EnvGenUnit::GetEnvDef()
	virtual void GetEnvDef(EnvDef *def)
	{
		def->Alloc(numSeg, segStart, susOn);
		if (numSeg > 0)
			memcpy(def->segs, segRLT, numSeg*sizeof(SegVals));
	}

	/// Get the number of segments
	int GetSegs()
	{ 
		return numSeg; 
	}

	/// Set the number of segments. This method allocates
	/// an array of SegVals and initializes them to zero.
	/// @param count number of segments
	virtual void SetSegs(int count)
	{
		if (count == 0)
			count = 1;
		if (count == numSeg)
			return;

		SegVals *segRLTn = new SegVals[count];
		EnvSeg **segObjn = new EnvSeg *[count];
		for (int n = 0; n < count; n++)
		{
			if (n < numSeg)
			{
				segRLTn[n].level = segRLT[n].level;
				segRLTn[n].rate = segRLT[n].rate;
				segRLTn[n].type = segRLT[n].type;
				segRLTn[n].fixed = segRLT[n].fixed;
				segRLTn[n].crt = segRLT[n].crt;
				segObjn[n] = segObj[n];
			}
			else
			{
				segRLTn[n].level = 0;
				segRLTn[n].rate = 0;
				segRLTn[n].type = linSeg;
				segRLTn[n].fixed = 1;
				segRLTn[n].crt = 0;
				segObjn[n] = &egsLin;
			}
		}
		numSeg = count;
		if (segRLT)
			delete[] segRLT;
		segRLT = segRLTn;
		if (segObj)
			delete[] segObj;
		segObj = segObjn;
	}

	/// Set the sustain flag on/off. When the sustain flag is set
	/// on the envelope will stop before the final segment and
	/// continue to return the current value until Release is called.
	/// @param on sustain on flag, 1 = on, 0 = off
	inline void SetSusOn(int on)
	{
		susOn = on; 
	}

	/// Set the starting value for the envelope. Typically this
	/// is set to zero for amplitude envelopes. It will often
	/// be set to a non-zero value when the envelope is used
	/// to sweep a filter or an FM index value.
	/// @param lvl starting level
	inline void SetStart(AmpValue lvl)
	{
		segStart = lvl; 
	}

	inline void SetDuration(FrqValue d)
	{
		duration = d;
	}

	/// Set the rate for a segment. This sets the seconds
	/// to transition to the end level for the segment.
	/// @param segn segment number
	/// @param rt time in seconds for this segment
	inline void SetRate(int segn, FrqValue rt)
	{
		if (segn < numSeg)
			segRLT[segn].rate = rt;
	}

	/// Set the ending level for a segment. This is the value
	/// reached at the end of the segment. The starting
	/// value is always the current level when the segment
	/// begins.
	/// @param segn segment number
	/// @param lvl end level
	inline void SetLevel(int segn, AmpValue lvl)
	{
		if (segn < numSeg)
			segRLT[segn].level = lvl; 
	}

	/// Set the fixed duration flag.
	/// When fix is 1 the time for the indicated segment
	/// is a fixed duration. When fix is 0, the time
	/// is relative to the duration of a note. When fix
	/// is 2, the duration is the total duration minus
	/// all other segments.
	/// @param segn segment number
	/// @param fix fixed or relative duration
	inline void SetFixed(int segn, int fix)
	{
		if (segn < numSeg)
			segRLT[segn].fixed = fix; 
	}


	/// Get the current value for the sustain on flag.
	inline int GetSusOn()
	{ 
		return susOn; 
	}

	/// Get the current start value for envelope.
	inline AmpValue GetStart()
	{ 
		return segStart; 
	}

	/// Get the rate for a segment.
	/// @param segn segment number
	inline FrqValue GetRate(int segn)
	{ 
		if (segn < numSeg)
			return segRLT[segn].rate;
		return 0;
	}

	/// Get the end level for a segment.
	/// @param segn segment number
	inline AmpValue GetLevel(int segn)
	{ 
		if (segn < numSeg)
			return segRLT[segn].level; 
		return 0;
	}

	/// Get the type of a segment. The type defines
	/// the curve, linear, exponential, logarithmic,
	/// or sustain.
	/// @param segn segment number
	inline EGSegType GetType(int segn)
	{ 
		if (segn < numSeg)
			return segRLT[segn].type; 
		return nulSeg;
	}

	/// Get the value for the fixed duration flag
	inline int GetFixed(int segn)
	{
		if (segn < numSeg)
			return segRLT[segn].fixed; 
		return 0;
	}


	/// Set the type of a segment. The type defines
	/// the curve, linear, exponential, logarithmic,
	/// or sustain.
	/// @param segn segment number
	/// @param ty segment type, one of EGSegType
	void SetType(int segn, EGSegType ty)
	{
		if (segn >= numSeg)
			return;
		segRLT[segn].type = ty;
		switch(ty)
		{
		default:
		case linSeg:
			segObj[segn] = &egsLin;
			break;
		case expSeg:
			segObj[segn] = &egsExp;
			break;
		case logSeg:
			segObj[segn] = &egsLog;
			break;
		case susSeg:
			segObj[segn] = &egsSus;
			break;
		case sqrSeg:
			segObj[segn] = &egsSqr;
			break;
		}
	}

	/// Get all values for a segment. This method copies all three
	/// values into the appropriate arguments.
	/// @param segn segment number
	/// @param rt time in seconds for the segment
	/// @param lvl end level for the segment
	/// @param typ curve type for the segment
	/// @param fix fixed duration flag for the segment
	void GetSegN(int segn, FrqValue& rt, AmpValue& lvl, EGSegType& typ, int& fix)
	{
		if (segn >= numSeg)
			return;
		rt  = segRLT[segn].rate;
		lvl = segRLT[segn].level;
		typ = segRLT[segn].type;
		fix = segRLT[segn].fixed;
	}

	/// Set all values for a segment. This method copies all three
	/// values from the appropriate arguments.
	/// @param segn segment number
	/// @param rt time in seconds for the segment
	/// @param lvl end level for the segment
	/// @param typ curve type for the segment
	/// @param fix fixed time or relative to duration
	void SetSegN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg, int fix = 1)
	{
		if (segn >= numSeg)
			return;
		SetRate(segn, rt);
		SetLevel(segn, lvl);
		SetType(segn, typ);
		SetFixed(segn, fix);
	}

	/// Reset the envelope. The segment number is moved back to the first
	/// segment and the current value is set back to the start value. If
	/// the phase is < 0, no change to the envelope happens.
	/// @param initPhs phase value
	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			int n1, n2;
			FrqValue rt;
			SegVals *rlt = &segRLT[0];
			for (n1 = 0; n1 < numSeg; n1++)
			{
				switch (rlt->fixed)
				{
				case 0:
					// relative to duration
					rt = duration * rlt->rate;
					break;
				case 1:
					// fixed rate
					rt = rlt->rate;
					break;
				case 2:
					// duration minus all other rates
					rt = duration;
					for (n2 = 0; n2 < numSeg; n2++)
					{
						if (n2 != n1)
							rt -= segRLT[n2].rate;
					}
					if (rt < 0)
						rt = 0;
					break;
				default:
					rt = 0;
					break;
				}
				rlt->crt = rt;
				rlt++;
			}
			index = 0;
			lastVal = segStart;
			NextSeg();
		}
	}

	/// Begin the envelope release.
	/// For a multi-segment envelope without sustain
	/// this method does nothing.
	virtual void Release()
	{
		// NOOP for the base class...
		// derived classes should transition to release state
	}

	/// Move to the next segment.
	/// This is called automatically
	/// by Gen when the end of the current segment is reached. Derived
	/// classes can also call this directly to jump to another segment.
	/// Discontinuities are avoided by using the current level as the
	/// starting value for the segment.
	virtual void NextSeg()
	{
		if (index < numSeg)
		{
			seg = segObj[index];
			if (seg != NULL)
			{
				SegVals *rlt = &segRLT[index];
				seg->InitSeg(rlt->crt, lastVal, rlt->level);
			}
			index++;
		}
		else
			seg = segObj[numSeg-1];
	}

	/// @copydoc EnvGenUnit::Gen()
	virtual AmpValue Gen()
	{
		lastVal = seg->Gen();
		if (seg->IsFinished())
			NextSeg();
		return lastVal;
	}

	/// @copydoc EnvGen::IsFinished()
	virtual int IsFinished()
	{
		return (index >= numSeg && seg->IsFinished());
	}
}; 

///////////////////////////////////////////////////////////
/// Multi-attack, sustain, single release segment envelope generator.
/// This envelope generator implements a configurable number
/// of attack segments, an optional sustain of indeterminate length,
/// and a single release segment. The ADSR and AR envelopes
/// derive from this class.
/// @sa EnvGenSeg
///////////////////////////////////////////////////////////
class EnvGenSegSus : public EnvGenSeg
{
protected:
	int state;
	int relSeg;

public:
	EnvGenSegSus()
	{
		state = 3;
		relSeg = 0;
	}

	/// @copydoc EnvGenSeg::Reset
	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
		{
			state = 0;
			relSeg = numSeg - 1;
			EnvGenSeg::Reset(initPhs);
		}
	}

	/// @copydoc EnvGenUnit::Gen()
	virtual AmpValue Gen()
	{
		switch (state)
		{
		case 0:
			lastVal = seg->Gen();
			if (seg->IsFinished())
			{
				if (index == relSeg)
					state = 1;
				else
					NextSeg();
			}
			break;
		case 1:
			if (!susOn)
			{
				NextSeg();
				state = 2;
				lastVal = seg->Gen();
			}
			break;
		case 2:
			lastVal = seg->Gen();
			if (seg->IsFinished())
				state = 3;
			break;
		}
		return lastVal;
	}

	/// Begin the envelope release.
	/// If the envelope is already in the release phase no action is taken. Otherwise,
	/// the segment is moved to the release segment regardless of the current segment.
	virtual void Release()
	{
		if (state < 2)
		{
			state = 2;
			index = relSeg;
			NextSeg();
		}
	}

	/// @copydoc EnvGen::IsFinished()
	virtual int IsFinished()
	{
		return state == 3;
	}
};

///////////////////////////////////////////////////////////
// AR - attack, [sustain,] and release
/// AR envelope generator with variable sustain.
/// This envelope generator provides a single attack
/// and single release segment with a variable length sustain.
/// The release segment begins when the Release() method is called.
/// @sa EnvGenSegSus EnvGenSeg
///////////////////////////////////////////////////////////
class EnvGenAR : public EnvGenSegSus
{
protected:

public:
	EnvGenAR()
	{
		susOn = 1;
		EnvGenSeg::SetSegs(2);
	}

	/// Set the attack rate.
	/// @param val rate in seconds from 0 to peak level
	inline void SetAtkRt(FrqValue val)
	{ 
		SetRate(0, val); 
	}
	/// Set the release rate.
	/// @param val rate in seconds from peak to 0 level.
	inline void SetRelRt(FrqValue val)
	{ 
		SetRate(1, val); 
	}
	/// Set the sustain level.
	inline void SetSus(AmpValue val)
	{ 
		SetLevel(0, val); 
	}
	/// Get the attack rate
	inline FrqValue GetAtkRt()
	{ 
		return GetRate(0); 
	}
	/// Get the release rate.
	inline FrqValue GetRelRt()
	{ 
		return GetRate(1); 
	}
	/// Get the sustain level.
	inline AmpValue GetSus()
	{ 
		return GetLevel(0); 
	}

	/// Set the number of segments.
	/// For AR generators the number of segments is always two.
	void SetSegs(int count) { }
	/// Get the number of segments.
	/// For AR generators the number of segments is always two.
	int GetSegs() { return 2; }

	/// Initialize all segments.
	/// This method sets all values with one function call.
	/// @param ar attack rate
	/// @param sl sustain level
	/// @param rr release rate
	/// @param son sustain on (1) or off (0)
	/// @param t curve type (\see EGSegType)
	/// @param fix fixed time or relative to duration
	virtual void InitAR(FrqValue ar, AmpValue sl, FrqValue rr, int son, EGSegType t, int fix = 1)
	{
		SetSusOn(son);
		SetStart(0);
		SetSegN(0, ar, sl, t, fix);
		SetSegN(1, rr, 0, t, fix);
		Reset();
	}
};

///////////////////////////////////////////////////////////
// ADSR - attack, decay, sustain, and release
/// ADSR envelope generator with variable sustain.
/// This envelope generator provides a typical attack,
/// decay, sustain, release envelope. The length of the sustain
/// segment is variable. The release segment begins when the
/// Release() method is called.
/// @sa EnvGenSegSus EnvGenSeg
///////////////////////////////////////////////////////////
class EnvGenADSR : public EnvGenSegSus
{
protected:

public:
	EnvGenADSR()
	{
		susOn = 1;
		EnvGenSeg::SetSegs(3);
	}

	/// Set the attack rate.
	/// @param val rate in seconds to transtion from start to initial peak level
	inline void SetAtkRt(FrqValue val)
	{ 
		SetRate(0, val); 
	}
	/// Set the initial peak level.
	/// @param val level at end of attack segment
	inline void SetAtkLvl(AmpValue val)
	{ 
		SetLevel(0, val); 
	}
	/// Set the decay rate.
	/// @param val rate in seconds to transition from attack peak to sustain level
	inline void SetDecRt(FrqValue val)
	{
		SetRate(1, val); 
	}
	/// Set the sustain level.
	/// @param val level at end of decay segment
	inline void SetSusLvl(AmpValue val)
	{ 
		SetLevel(1, val); 
	}
	/// Set the release rate.
	/// @param val rate in seconds to transition from sustain level to end level
	inline void SetRelRt(FrqValue val)  
	{ 
		SetRate(2, val); 
	}
	/// Set the release level.
	/// @param val level at the end of the envelope
	inline void SetRelLvl(AmpValue val) 
	{ 
		SetLevel(2, val); 
	}
	/// Set the envelope curve type. All segments in an ADSR envelope
	/// have the same type.
	/// @param ty envelope curve type
	inline void SetType(EGSegType ty)   
	{
		EnvGenSeg::SetType(0, ty);
		EnvGenSeg::SetType(1, ty);
		EnvGenSeg::SetType(2, ty);
	}

	/// Get the attack rate.
	inline FrqValue GetAtkRt()
	{ 
		return GetRate(0); 
	}
	/// Get the attack level.
	inline AmpValue GetAtkLvl()
	{ 
		return GetLevel(0); 
	}
	/// Get the decay rate.
	inline FrqValue GetDecRt()
	{ 
		return GetRate(1); 
	}
	/// Get the sustain level.
	inline AmpValue GetSusLvl()
	{ 
		return GetLevel(1); 
	}
	/// Get the release rate.
	inline FrqValue GetRelRt()
	{ 
		return GetRate(2); 
	}
	/// Get the release (final) level
	inline AmpValue GetRelLvl()
	{ 
		return GetLevel(2); 
	}
	/// Get the curve type.
	inline EGSegType GetType()
	{ 
		return EnvGenSeg::GetType(0); 
	}

	/// Set the number of segments.
	/// For ADSR generators the number of segments is always three.
	void SetSegs(int count) { }
	/// Get the number of segments.
	/// For ADSR generators the number of segments is always three.
	int GetSegs() {	return 3; }

	/// Initialize all segments.
	/// This method sets all rate and level values from the arguments.
	/// @param st start level
	/// @param ar attack rate
	/// @param al attack level
	/// @param dr decay rate
	/// @param sl sustain level
	/// @param rr release rate
	/// @param rl release (final) level
	/// @param t curve type (\see EGSegType)
	/// @param fix fixed or relative duration
	virtual void InitADSR(AmpValue st, FrqValue ar, AmpValue al, FrqValue dr, 
	                      AmpValue sl, FrqValue rr, AmpValue rl, EGSegType t = linSeg, int fix = 1)
	{
		SetStart(st);
		SetSegN(0, ar, al, t, fix);
		SetSegN(1, dr, sl, t, fix);
		SetSegN(2, rr, rl, t, fix);
		Reset();
	}
};

///////////////////////////////////////////////////////////
/// Multi-attack, sustain, multi-decay segment envelope generator.
/// EnvGenMulSus implements a multi-segment envelope with a
/// variable number of attack and decay segments and 
/// indeterminate sustain length. This aggregates two copies of EnvGenSeg.
/// The first object contains the attack segments while the 
/// second contains the decay segments. When the end of all 
/// attack segments has been reached, EnvGenSegSus waits for
/// the Release() method call to begin processing the decay segments.
/// @sa EnvGenSeg
///////////////////////////////////////////////////////////
class EnvGenMulSus : public EnvGenUnit
{
private:
	EnvGenSeg atk;
	EnvGenSeg dec;
	int state;

	AmpValue lastVal;
	AmpValue segStart;

public:
	EnvGenMulSus()
	{
		state = 0;
		lastVal = 0;
		segStart = 0;
	}

	/// @copydoc EnvGenUnit::Copy
	virtual void Copy(EnvGenUnit *tp)
	{
		EnvGenMulSus *ap = (EnvGenMulSus *)tp;
		atk.Copy(&ap->atk);
		dec.Copy(&ap->dec);
		state = ap->state;
		lastVal = ap->lastVal;
		segStart = ap->segStart;
	}

	/// @copydoc EnvGenSeg::Init
	virtual void Init(int n, float *v)
	{
		if (n > 0)
		{
			AmpValue start = *v++;
			atk.SetStart(start);
			int atksegs = (int) *v++;
			int decsegs = (int) *v++;
			SetSegs(atksegs, decsegs);
			n += 4;
			int i;
			for (i = 0; i < atksegs; i++)
			{
				atk.SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2], (int) v[3]);
				v += 4;
				start = v[1];
			}
			dec.SetStart(start);
			for (i = 0; i < decsegs; i++)
			{
				dec.SetSegN(i, FrqValue(v[0]), AmpValue(v[1]), (EGSegType) (int) v[2], (int) v[4]);
				v += 4;
			}
		}
		Reset();
	}

	/// Set the starting level for the envelope.
	/// @param sv starting level
	void SetStart(AmpValue sv)
	{
		atk.SetStart(sv);
	}

	/// Set the number of attack and decay segments
	/// @param atks number of attack segments
	/// @param decs number of decay segments
	void SetSegs(int atks, int decs)
	{
		atk.SetSegs(atks);
		dec.SetSegs(decs);
	}

	/// Set values for one attack segment.
	/// @param segn segment number
	/// @param rt rate in seconds
	/// @param lvl end level for the segment
	/// @param typ curve type for the segment
	/// @param fix fixed time or relative to duration
	void SetAtkN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg, int fix = 1)
	{
		atk.SetSegN(segn, rt, lvl, typ, fix);
	}

	/// Set values for one decay segment.
	/// @param segn segment number
	/// @param rt rate in seconds
	/// @param lvl end level for the segment
	/// @param typ curve type for the segment
	/// @param fix fixed time or relative to duration
	void SetDecN(int segn, FrqValue rt, AmpValue lvl, EGSegType typ = linSeg, int fix = 1)
	{
		dec.SetSegN(segn, rt, lvl, typ, fix);
	}

	/// @copydoc EnvGenSeg::Reset
	virtual void Reset(float initPhs = 0)
	{
		state = 0;
		atk.Reset(initPhs);
	}

	/// @copydoc EnvGenUnit::Sample
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Move to the first release segment.
	virtual void Release()
	{
		dec.SetStart(lastVal);
		dec.Reset();
		state = 2;
	}

	/// @copydoc EnvGenUnit::Gen()
	virtual AmpValue Gen()
	{
		switch (state)
		{
		case 0:
			lastVal = atk.Gen();
			if (atk.IsFinished())
				state = 1;
			break;
		case 1:
			break;
		case 2:
			lastVal = dec.Gen();
			if (dec.IsFinished())
				state = 3;
			break;
		case 3:
			break;
		}
		return lastVal;
	}

	/// @copydoc EnvGenSeg::IsFinished
	virtual int IsFinished()
	{
		return state == 3;
	}
};

///////////////////////////////////////////////////////////
/// Table lookup envelope generator. The entire envelope
/// is calculated once and stored in a lookup table. 
/// This is more efficient when the envelope rarely changes.
/// Note that this derives directly from GenUnit and does
/// not include the Release method of EnvGenUnit. 
///////////////////////////////////////////////////////////
class EnvGenTable : public GenUnit
{
private:
	AmpValue *egTable;
	bsInt32 count;
	bsInt32 index;

public:
	EnvGenTable()
	{
		egTable = NULL;
		count = 0;
		index = 0;
	}

	~EnvGenTable()
	{
		delete egTable;
	}

	/// Return the next value multiplied by the current sample amplitude.
	virtual AmpValue Sample(AmpValue in)
	{
		return Gen() * in;
	}

	/// Generate the next value. When the end of the calculated table
	/// is reached, the final value is returned.
	virtual AmpValue Gen()
	{
		if (index < count)
			return egTable[index++];
		return egTable[count];
	}

	/// Check to see if the envelope is finished.
	/// Returns true when all values have been returned.
	virtual int IsFinished()
	{
		return index >= count;
	}

	// segs, start, [rate, level]*
	/// Initialize the envelope. 
	/// v[0] contains the number of segments; v[1] contains
	/// the starting level. The remaining values contains pairs of
	/// rate,level for each segment.
	/// @param n number of values
	/// @param v array of initialization values
	virtual void Init(int n, float *v)
	{
		if (n >= 2)
		{
			int segs = (int) *v++;
			if (segs < 1)
				segs = 1;
			AmpValue start = AmpValue(*v++);
			FrqValue *rts  = new FrqValue[segs];
			AmpValue *amps = new AmpValue[segs];
			n -= 2;
			int i = 0;
			while (n >= 2)
			{
				rts[i]  = FrqValue(*v++);
				amps[i] = AmpValue(*v++);
				n -= 2;
				i++;
			}

			InitSegs(segs, start, rts, amps, NULL);
			delete rts;
			delete amps;
		}
	}

	/// Reset the envelope.
	/// @param initPhs time position to move to
	virtual void Reset(float initPhs = 0)
	{
		if (initPhs >= 0)
			index = (bsInt32) (initPhs * synthParams.sampleRate);
	}

	/// Initialize the envelope.
	/// Values are taken from the arguments:
	/// @param segs  number of segments
	/// @param start starting value
	/// @param rt array of segment lengths in seconds
	/// @param amp array of levels at end of segments
	/// @param typ array of segment types, if NULL, all segs are linear
	void InitSegs(int segs, AmpValue start, FrqValue *rt, AmpValue *amp, EGSegType *typ)
	{
		if (egTable)
			delete egTable;

		FrqValue dcount = 0;
		int segn;
		for (segn = 0; segn < segs; segn++)
			dcount += rt[segn];

		count = (long) ((synthParams.sampleRate * dcount) + 0.5);
		egTable = new AmpValue[count+1];

		FrqValue ndxf = 0;
		AmpValue vbeg = start;
		AmpValue vend = 0;

		EnvSeg *segp;
		EnvSegLin egsLin;
		EnvSegExp egsExp;
		EnvSegLog egsLog;
		EnvSeg    egsSus;

		for (segn = 0; segn < segs; segn++)
		{
			vend = amp[segn];
			if (vend == vbeg)
				segp = &egsSus;
			else if (typ == NULL)
				segp = &egsLin;
			else switch(typ[segn])
			{
			case linSeg:
				segp = &egsLin;
				break;
			case expSeg:
				segp = &egsExp;
				break;
			case logSeg:
				segp = &egsLog;
				break;
			default:
				segp = &egsSus;
				break;
			}
			segp->InitSeg(rt[segn], vbeg, vend);
			FrqValue seglen = synthParams.sampleRate * rt[segn];
			FrqValue segend = ndxf + seglen;
			while (ndxf < segend)
			{
				egTable[(int)ndxf] = segp->Gen();
				ndxf += 1;
			}
			vbeg = vend;
		}
		index = (int)ndxf;
		while (index <= count)
			egTable[index++] = vend;

		index = 0;
	}
};
/*@}*/
#endif
