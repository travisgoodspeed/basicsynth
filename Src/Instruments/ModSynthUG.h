//////////////////////////////////////////////////////////////////////
/// @file ModSynthUG.h Interface and template class for ModSynth unit generators
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpInstrument
//@{
#ifndef MODSYNTH_UG_H
#define MODSYNTH_UG_H

// These values allow for 64 inputs per gen, and 1024 gens per instrument.
// That should be enough...
#define UGID_SHIFT 6
#define UGID_MASK 0x3F

// A UG can have more than 32 inputs, but they cannot monitored for
// change with the "anyChange" member on 32-bit compilers where a long
// is only 32-bits. A UG can do this by forcing use of a 64 bit variable,
// or by defining a change bits array, override the SetInput function
// and use these:
#define UGIP_SET(a,n)   a[(n)>>5] |= 1 << ((n)&0x1F)
#define UGIP_CLR(a,n)   a[(n)>>5] &= ~(1 << ((n)&0x1F))
#define UGIP_TST(a,n)   a[(n)>>5] & (1 << ((n)&0x1F))

#define UGP_NEVER 0
#define UGP_INIT  0x01
#define UGP_GEN   0x02
#define UGP_DEF   0x08
#define UGP_SAVE  0x10
#define UGP_LOAD  0x20
#define UGP_RUN  (UGP_INIT|UGP_GEN)
#define UGP_ALL  (UGP_RUN|UGP_SAVE|UGP_LOAD)

/// UGParam defines one unit generator parameter.
/// The index value specifies which parameter.
/// The when value specifies when the parameter is updated.
class UGParam
{
public:
	short index;
	short when;
	const char *name;
	float minval;
	float maxval;
};

class ModSynthUG;

/// ModSynthConn defines a connection between a ug output and input.
class ModSynthConn : public SynthList<ModSynthConn>
{
public:
	short index;
	short when;
	ModSynthUG *ug;
	ModSynthConn()
	{
		ug = 0;
		index = -1;
		when = 0;
	}
	ModSynthConn(ModSynthUG *u, int n, int w)
	{
		ug = u;
		index = n;
		when = w;
	}
};

/// ModSynthUG is the interface to any unit generator
/// that is part of the ModSynth instrument. This is
/// a pure-virtual base class.
class ModSynthUG : public SynthList<ModSynthUG>
{
public:
	/// Get the UG type, a string that matches the table of UG objects.
	virtual const char *GetType() = 0;
	/// Get the ID. This is only needed if the UG represents
	/// a settable parameter.
	virtual bsInt16 GetID() = 0;
	virtual void SetID(bsInt16 i) = 0;
	/// Set an input value.
	virtual void SetInput(short index, float value) = 0;
	/// Get an input value
	virtual float GetInput(short index) = 0;
	/// Return the number of input values
	virtual short GetNumInputs() = 0;
	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void Tick() = 0;
	virtual void Send(float value, short mask) = 0;
	virtual int IsFinished() = 0;
	virtual AmpValue GetOutput() = 0;
	virtual const UGParam *GetParamList() = 0;
	virtual const char *GetName()  = 0;
	virtual void SetName(const char *n) = 0;
	virtual int MatchName(const char *n) = 0;
	virtual const UGParam *FindParam(const char *p) = 0;
	virtual const UGParam *FindParam(int index) = 0;
	virtual void AddConnect(ModSynthUG *dst, short index, short when) = 0;
	virtual void RemoveConnect(ModSynthUG *ug, int index = -1) = 0;
	virtual ModSynthConn *ConnectList(ModSynthConn *last) = 0;
	virtual ModSynthUG *Copy() = 0;
	virtual void InitDefault() = 0;
	virtual int Load(XmlSynthElem *elem) = 0;
	virtual int Save(XmlSynthElem *elem) = 0;
	virtual void DumpUnit(void (*fn)(const char*)) = 0;
	virtual void DumpConnect(void (*fn)(const char*)) = 0;
};

/// Template for standard unit generators.
/// A unit generator typically inhertis from
/// this implementation class, specifying the
/// GenUnit class and number of input parameters.
template <class DT, class UG, short IP = 1>
class ModSynthUGImpl : public ModSynthUG
{
public:
	UG gen;
	AmpValue out;
	float inputs[IP];
	bsInt32 anyChange;
	bsInt16 id;
	bsString name;
	ModSynthConn chead;
	ModSynthConn ctail;

	static ModSynthUG *Construct()
	{
		return new DT;
	}

	ModSynthUGImpl()
	{
		for (int n = 0; n < IP; n++)
			inputs[n] = 0;
		anyChange = 0;
		out = 0;
		id = -1;
		chead.Insert(&ctail);
	}

	virtual ~ModSynthUGImpl()
	{
		ModSynthConn *conn;
		while ((conn = chead.next) != &ctail)
		{
			conn->Remove();
			delete conn;
		}
	}

	ModSynthUG *Copy()
	{
		DT *p = new DT;
		p->SetName(name);
		p->id = id;
		//for (int n = 0; n < IP; n++)
		//	p->SetInput(n, inputs[n]); <== this can have side-effects
		memcpy(p->inputs, inputs, IP*sizeof(float));
		return (ModSynthUG*)p;
	}

	virtual void SetID(bsInt16 i) { id = i; }
	virtual bsInt16 GetID() { return id; }

	virtual const char *GetName() { return name; }
	virtual void SetName(const char *n) { name = n; }
	virtual int MatchName(const char *n)
	{
		return name.CompareNC(n) == 0;
	}

	virtual void SetInput(short index, float val)
	{
		if (index < IP)
		{
			inputs[index] = val;
			anyChange |= 1 << index;
		}
	}

	virtual float GetInput(short index)
	{
		if (index < IP)
			return inputs[index];
		return 0;
	}

	virtual short GetNumInputs()
	{
		return IP;
	}

	virtual const UGParam *FindParam(const char *p)
	{
		if (isdigit(*p))
			return FindParam(atoi(p));

		const UGParam *list = GetParamList();
		while (list->index >= 0)
		{
			if (strcmp(list->name, p) == 0)
				return list;
			list++;
		}
		return 0;
	}

	virtual const UGParam *FindParam(int index)
	{
		const UGParam *list = GetParamList();
		while (list->index >= 0)
		{
			if (list->index == index)
				return list;
			list++;
		}
		return 0;
	}

	virtual ModSynthConn *ConnectList(ModSynthConn *last)
	{
		if (last == 0)
			last = chead.next;
		else
			last = last->next;
		if (last == &ctail)
			return 0;
		return last;
	}

	virtual void AddConnect(ModSynthUG *dst, short index, short when)
	{
		ModSynthConn *cp = new ModSynthConn(dst, index, when);
		ctail.InsertBefore(cp);
	}

	virtual void RemoveConnect(ModSynthUG *ug, int index = -1)
	{
		ModSynthConn *conn = chead.next;
		while (conn != &ctail)
		{
			if (conn->ug == ug && (index == -1 || index == conn->index))
			{
				conn->Remove();
				delete conn;
				break;
			}
			conn = conn->next;
		}
	}

	virtual void DumpUnit(void (*fn)(const char *))
	{
		fn(GetType());
		fn(" ");
		fn(GetName());
		char valbuf[40];
		const UGParam *list = GetParamList();
		while (list->index >= 0)
		{
//			snprintf(valbuf, 40, " %s=%f", list->name, GetInput(list->index));
			strcpy(valbuf, list->name);
			fn(valbuf);
			list++;
		}
		fn("\n");
	}

	virtual void DumpConnect(void (*fn)(const char*))
	{
		const UGParam *p;
		ModSynthConn *conn = chead.next;
		while (conn != &ctail)
		{
			fn("connect ");
			fn(GetName());
			fn(" -> ");
			fn(conn->ug->GetName());
			p = conn->ug->FindParam(conn->index);
			if (p)
			{
				fn(".");
				fn(p->name);
			}
			fn(" ");
			conn = conn->next;
		}
		fn("\n");
	}

	virtual AmpValue GetOutput()
	{
		return out;
	}

	virtual int IsFinished()
	{
		return gen.IsFinished();
	}

	virtual void Send(float value, short mask)
	{
		ModSynthConn *conn = chead.next;
		while (conn != &ctail)
		{
			if (conn->when & mask)
				conn->ug->SetInput(conn->index, value);
			conn = conn->next;
		}
	}

	virtual void Start()
	{
		anyChange = 0;
	}

	virtual void Stop() { }

	virtual void Tick()
	{
		// derived class must initialize gen from any changed inputs
		out = gen.Sample(AmpValue(inputs[0]));
		Send(out, UGP_GEN);
	}

	virtual void InitDefault()
	{
	}

	virtual int Load(XmlSynthElem *elem)
	{
		DT *pthis = static_cast<DT*>(this);
		float val;
		const UGParam *list = pthis->GetParamList();
		while (list->index >= 0)
		{
			if (list->when & UGP_LOAD)
			{
				if (elem->GetAttribute(list->name, val) == 0)
					pthis->SetInput(list->index, val);
			}
			list++;
		}
		return 0;
	}

	virtual int Save(XmlSynthElem *elem)
	{
		DT *pthis = static_cast<DT*>(this);
		const UGParam *list = pthis->GetParamList();
		while (list->index >= 0)
		{
			if (list->when & UGP_SAVE)
				elem->SetAttribute(list->name, pthis->GetInput(list->index));
			list++;
		}
		return 0;
	}
};
//@}
#endif
