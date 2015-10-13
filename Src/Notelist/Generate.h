//////////////////////////////////////////////////////////////////////
/// @file Generate.h Notelist generator pass.
// Generator classes.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#if !defined(_GENERATE_H_)
#define _GENERATE_H_

#pragma once

/// @brief Values for builtin functions
/// @details Builtin functions are line, exponential, logarithmic, and randome.
/// Each time the function is invoked, the next value in the series is returned.
/// This structure holds the values used to calculate the series.
/// For LINE:
/// @code
/// y = a * x + c
/// @endcode
/// For EXP or LOG:
/// @code
/// y = a * x^b + c
/// x = n * 1/d
/// b = flatness of curve, exp or log
/// @code
/// For both:
/// @code
/// a = end - start
/// c = start
/// @endcode
class nlFunctionData
{
public:
	double beginVal;
	double endVal;
	double range;
	double offset;
	double count;
	double curVal;
	int    fnType;
	int    pos;

	nlFunctionData()
	{
		beginVal = 0;
		endVal = 1;
		range = 1.0;
		offset = 0.0;
		curVal = 0.0;
		count = 0.0;
		fnType = T_LINE;
		pos = 1;
	}

	int IsActive()
	{
		return curVal < count;
	}

	double Iterate(double dDur);

	void Init(double dFrom, double dTo, double dIn, int nFn = T_LINE)
	{
		fnType = nFn;
		beginVal = dFrom;
		endVal = dTo;
		range = dTo - dFrom;
		offset = dFrom;
		count = dIn;
		curVal = 0;
		pos = (dTo >= dFrom) ? 1 : 0;
	}
};

enum vType
{
	vtNull = 0,
	vtText,
	vtNum,
	vtReal
};

/// @brief variant value class.
/// @details The nlVarValue can store string, integer or real
/// and provides automatic type conversion. The expression
/// stack uses this type. All script nodes are also value nodes.
class nlVarValue
{
public:
	vType  vt;
	union 
	{
		char *txtVal;
		long  lVal;
		double dblVal;
	};

	nlVarValue()
	{
		vt = vtNull;
		txtVal = 0;
	}
	virtual ~nlVarValue()
	{
		ClearValue();
	}
	inline vType GetType() { return vt; }
	inline const char *RefStr() { return txtVal; }
	virtual void ClearValue();
	virtual void ChangeType(vType vnew);
	virtual void SetValue(const char *p);
	virtual void SetValue(long n);
	virtual void SetValue(double d);
	virtual void SetValue(nlVarValue *v);
	virtual void GetValue(char **p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	virtual void CopyValue(nlVarValue *v);
	virtual int  Compare(nlVarValue *v);
	virtual int  Match(nlVarValue *v);
};

class nlNamedVal : public nlVarValue
{
public:
	nlVarValue id;
	nlNamedVal *next;

	nlNamedVal()
	{
		next = 0;
	}

	void SetID(nlVarValue *p)
	{
		p->CopyValue(&id);
	}

	int MatchID(nlVarValue *p)
	{
		return id.Match(p);
	}
};

/// A symbol table entry.
/// A symbol consists of a name and a value.
/// The symbol can be an associative array with
/// a list of values. The Index function selects
/// or creates the value.
class nlSymbol : public nlVarValue
{
public:
	char *name;
	nlSymbol *next;
	nlNamedVal *arrayVal;

	nlSymbol()
	{
		arrayVal = NULL;
		next = NULL;
		name = NULL;
	}

	nlSymbol(const char *n)
	{
		if (n)
			name = StrMakeCopy(n);
		else
			name = NULL;
		arrayVal = NULL;
		next = NULL;
	}

	nlVarValue *Index(nlVarValue *ndx);
	void GetValue(nlVarValue *v, nlVarValue *ndx);
	void SetValue(nlVarValue *v, nlVarValue *ndx);
};


/// A holder for a sync/mark location
struct nlSyncMark
{
	nlVarValue id;
	double t;
	nlSyncMark *next;
};

/// Holder for note data: pitch, rhythm, volume, and parameter
/// The value is a dynamic array to support the group notation {...}
class nlNoteData
{
public:
	long alloc;
	long count;
	long index;
	int simul;
	nlVarValue *values;

	nlNoteData()
	{
		alloc = 0;
		count = 0;
		index = 0;
		simul = 0;
		values = NULL;
	}

	virtual ~nlNoteData()
	{
		for (long n = 0; n < alloc; n++)
			values[n].ClearValue();

		delete[] values;
	}

	/// Initialize with a single integer value
	void InitSingle(long n)
	{
		simul = 0;
		count = 1;
		index = 0;
		if (alloc == 0)
		{
			values = new nlVarValue[1];
			alloc = 1;
		}
		values->SetValue(n);
	}

	/// Initialize with a single floating point value
	void InitSingle(double n)
	{
		simul = 0;
		count = 1;
		index = 0;
		if (alloc == 0)
		{
			values = new nlVarValue[1];
			alloc = 1;
		}
		values->SetValue(n);
	}

	/// Exec fills in the note data.
	/// If the script node is a group, an array
	/// of the group size is allocated and filled in.
	/// Otherwise, a single value is set. On exit,
	/// the index is reset to 0.
	nlScriptNode *Exec(nlScriptNode *p);
	/// Return the next value.
	/// This increments the index until
	/// all values are processed and then
	/// continues to return the last value.
	int GetNextValue(double *d);
	int GetNextValue(long *n);
};

/// @brief A sequence of statements.
/// @details A sequence is a linked list of script nodes to be
/// executed in order. Sequence is used in several places
/// by script nodes. Each voice has a sequence of statements.
/// A PLAY object has a callable sequence. IF and WHILE
/// statements use a sequence to contain the nodes controlled by the condition.
class nlSequence
{
	nlVarValue id;
	nlScriptNode *head;
	nlScriptNode *tail;
	nlSequence *next;

	void Init();

public:
	nlSequence();
	nlSequence(const char *name);
	nlSequence(int n);
	~nlSequence();
	void Play();
	void Append(nlSequence **list);
	nlScriptNode *AddNode(nlScriptNode *seq);
	nlSequence *FindSequence(nlVarValue *find);
};

/// A nlVoice object holds all pertinent information about a voice.
class nlVoice
{
public:
	nlVoice();
	virtual ~nlVoice();

	nlVoice *next;
	nlGenerate *genPtr;
	int    voiceNum;
	double curTime;
	double volMul;
	double lastPit;
	double lastDur;
	double lastVol;
	double lastArtic;
	double *lastParam;
	double *paramVal;
	long   maxParam;
	long   cntParam;
	long   numParam;

	long   instr;
	long   chnl;
	long   articType;
	long   articParam;
	long   transpose;
	long   doublex;
	long   track;
	double doublev;
	long   loopCount;
	char   *instname;

	nlNoteData pitch;
	nlNoteData duration;
	nlNoteData volume;
	nlNoteData artic;
	nlNoteData *params;

	void ClearLast()
	{
		for (long n = 0; n < maxParam; n++)
		{
			lastParam[n] = 0.0;
			paramVal[n] = 0.0;
		}
	}

	void SetMaxParam(long n)
	{
		if (n < 0)
			return;
		if (params)
			delete params;
		if (lastParam)
			delete lastParam;
		if (paramVal)
			delete paramVal;
		params = new nlNoteData[n+1];
		lastParam = new double[n+1];
		paramVal = new double[n+1];
		maxParam = n;
		ClearLast();
	}

	void SetInstName(const char *newname)
	{
		if (instname)
			delete instname;
		instname = StrMakeCopy(newname);
	}
};

/// A nlScriptNode represents one parsed node in the script.
/// In general, each keyword or operator is implemented
/// by a class derived from nlScriptNode. The derived class
/// need only implement the Exec function in most cases.
/// For constants, no implementation is needed since a 
/// script node is also a value node.
/// The nlNote node and nlExpr nodes do most of the 
/// work. Other nodes typically set values on the current voice.
class nlScriptNode : public nlVarValue
{
protected:
	int token;
	nlScriptNode *next;
	nlGenerate *genPtr;

public:
	nlScriptNode()
	{
		token = -1;
		next = NULL;
		genPtr = NULL;
	}
	virtual ~nlScriptNode()
	{
	}

	virtual void SetGen(nlGenerate *p)
	{ 
		genPtr = p; 
	}

	virtual nlScriptNode *Exec()
	{
		return next;
	}

	void Append(nlScriptNode *p)
	{
		p->next = this;
	}

	inline void SetToken(int n)
	{
		token = n;
	}

	inline int GetToken()
	{
		return token;
	}

	inline nlScriptNode *GetNext()
	{
		return next;
	}
};

/// A node representing a change in voice.
class nlVoiceNode : public nlScriptNode
{
public:
	nlVoiceNode()
	{
		token = T_VOICE;
	}

	virtual nlScriptNode *Exec();
};

/// A node representing a BEGIN...END block
class nlBlockNode : public nlScriptNode
{
public:
	nlBlockNode()
	{
		token = T_BEGIN;
	}
	virtual nlScriptNode *Exec();
};

/// Tempo nodes set the current tempo.
/// The tempo values are stored in the voice
/// and used to convert rhythm values into
/// time values.
class nlTempoNode : public nlScriptNode
{
public:
	nlTempoNode()
	{
		token = T_TEMPO;
	}

	virtual nlScriptNode *Exec();
};

/// The time node moves the current time for the current voice.
class nlTimeNode : public nlScriptNode
{
public:
	nlTimeNode()
	{
		token = T_TIME;
	}
	virtual nlScriptNode *Exec();
};

/// Mark a place in the score. 
/// The mark value is stored in a list
/// in the current voice (synclist). 
class nlMarkNode : public nlScriptNode
{
public:
	nlMarkNode()
	{
		token = T_MARK;
	}
	virtual nlScriptNode *Exec();
};

/// Sync to a mark in the score
class nlSyncNode : public nlScriptNode
{
public:
	nlSyncNode()
	{
		token = T_SYNC;
	}
	virtual nlScriptNode *Exec();
};

// Change the instrument for the current voice.
class nlInstnumNode : public nlScriptNode
{
public:
	nlInstnumNode()
	{
		token = T_INSTNUM;
	}
	virtual nlScriptNode *Exec();
};

// Change the channel for the current voice.
class nlChnlNode : public nlScriptNode
{
public:
	nlChnlNode()
	{
		token = T_CHNL;
	}
	virtual nlScriptNode *Exec();
};

/// Generate a note event.
/// A "note" can be a single event, a group of events, or a chord.
/// The note node calculates the start time and duration for the
/// event, builds the parameter list, and then calls the converter
/// to output the event.
class nlNoteNode : public nlScriptNode
{
private:
	int    sus;
	int    add;

public:
	nlNoteNode()
	{
		sus = 0;
		add = 0;
		token = T_NOTE;
	}
	void SetSus(int n)
	{
		sus = n;
	}
	void SetAdd(int n)
	{
		add = n;
	}

	virtual nlScriptNode *Exec();
};

/// Evaluate an expression.
class nlExprNode : public nlScriptNode
{
public:
	nlExprNode()
	{
		token = T_EXPR;
	}

	virtual nlScriptNode *Exec();
};

/// A rhythm value. The value stored in the node
/// is converted to time using the current tempo.
class nlDurNode : public nlScriptNode
{
private:
	int isDotted;
public:
	nlDurNode()
	{
		isDotted = 0;
		token = T_DUR;
	}
	void SetDotted(int d) { isDotted = d; }
	virtual void CopyValue(nlVarValue *p);
	virtual void GetValue(long *n);
	virtual void GetValue(double *d);
	void GetBeatValue(double *n)
	{
		nlVarValue::GetValue(n);
	}
};

/// Conditionally execute a sequence.
/// The condition is the expression node
/// immediately following the IF node.
class nlIfNode : public nlScriptNode
{
private:
	nlSequence *ifPart;
	nlSequence *elPart;
public:
	nlIfNode()
	{
		token = T_IF;
		ifPart = NULL;
		elPart = NULL;
	}
	virtual ~nlIfNode();
	void SetIfSequence(nlSequence *p)
	{
		ifPart = p;
	}
	void SetElseSequence(nlSequence *p)
	{
		elPart = p;
	}
	virtual nlScriptNode* Exec();
};

/// Execute a sequence while a condition is true.
/// The condition is the expression node
/// immediately following the WHILE node.
class nlWhileNode : public nlScriptNode
{
private:
	nlSequence *wseq;
public:
	nlWhileNode()
	{
		token = T_WHILE;
		wseq = NULL;
	}
	virtual ~nlWhileNode();
	void SetSequence(nlSequence *p)
	{
		wseq = p;
	}

	virtual nlScriptNode* Exec();
};

/// Loop a sequence.
class nlLoopNode : public nlScriptNode
{
private:
	nlSequence *lseq;
public:
	nlLoopNode()
	{
		lseq = NULL;
		token = T_LOOP;
	}
	virtual ~nlLoopNode();
	void SetSequence(nlSequence *p)
	{
		lseq = p;
	}
	virtual nlScriptNode *Exec();
};

/// Write a message to the console during generation.
class nlWriteNode: public nlScriptNode
{
public:
	nlWriteNode()
	{
		token = T_WRITE;
	}
	virtual nlScriptNode *Exec();
};

/// Set the volume level for the voice.
/// The voice volume is combined with the note
/// volume to produce the volume level for each event.
class nlVolumeNode : public nlScriptNode
{
public:
	nlVolumeNode()
	{
		token = T_VOL;
	}
	virtual nlScriptNode *Exec();
};

/// Transpose all notes following.
class nlTransposeNode : public nlScriptNode
{
public:
	nlTransposeNode()
	{
		token = T_XPOSE;
	}
	virtual nlScriptNode *Exec();
};

/// Double all notes at the given transposition.
class nlDoubleNode : public nlScriptNode
{
public:
	nlDoubleNode()
	{
		token = T_DOUBLE;
	}
	virtual nlScriptNode *Exec();
};

/// Play a sequence.
/// A sequence holds a list of statements
/// that can be invoked by name, 
/// similar to a subroutine call. The class holds
/// the script nodes that make up the sequence. When
/// the sequence is played, the object pushes the current
/// note list and plays its own note list, then pops the
/// original notelist back. The push/pop is effected by
/// calling Play on the sequence.
class nlPlayNode : public nlScriptNode
{
public:
	nlPlayNode()
	{
		token = T_PLAY;
	}
	virtual nlScriptNode *Exec();
};

/// Initialize a function generator.
class nlInitFnNode : public nlScriptNode
{
public:
	nlInitFnNode()
	{
		token = T_INIT;
	}
	virtual nlScriptNode *Exec();
};

/// Set the articulation for the current voice.
class nlArticNode : public nlScriptNode
{
public:
	nlArticNode()
	{
		token = T_ART;
	}
	virtual nlScriptNode *Exec();
};

class nlParamNode : public nlScriptNode
{
public:
	nlParamNode()
	{
		token = T_PARAM;
	}
	virtual nlScriptNode *Exec();
};

/// Map instrument parameters.
class nlMapNode : public nlScriptNode
{
public:
	nlMapNode()
	{
		token = T_MAP;
	}
	virtual nlScriptNode *Exec();
};

/// Set the maximum number of retained parameters.
class nlMaxParamNode : public nlScriptNode
{
public:
	nlMaxParamNode()
	{
		token = T_MAXPARAM;
	}
	virtual nlScriptNode *Exec();
};

/// Call a script engine.
class nlCallNode : public nlScriptNode
{
public:
	nlCallNode()
	{
		token = T_CALL;
	}
	virtual nlScriptNode *Exec();
};

/// Assign a value to a variable.
class nlSetNode : public nlScriptNode
{
private:
	nlSymbol *symb;
public:
	nlSetNode()
	{
		token = T_SET;
		symb = NULL;
	}

	void SetSymbol(nlSymbol *s)
	{
		symb = s;
	}

	virtual nlScriptNode *Exec();
};

/// A variable.
class nlVarNode : public nlScriptNode
{
private:
	nlSymbol *symb;
public:
	nlVarNode()
	{
		token = T_VAR;
		symb = NULL;
	}

	void SetSymbol(nlSymbol *s)
	{
		symb = s;
	}

	virtual nlScriptNode *Exec();
};

/// Set options for the generator.
class nlOptNode : public nlScriptNode
{
public:
	nlOptNode()
	{
		token = T_OPTION;
	}
	virtual nlScriptNode *Exec();
};

/// Generate events for dynamic mixer operations.
class nlMixerNode : public nlScriptNode
{
private:
	int mixFn;
public:
	nlMixerNode()
	{
		mixFn = mixNoFunc;
		token = T_MIX;
	}
	void SetFunction(int f)
	{
		mixFn = f;
	}
	virtual nlScriptNode *Exec();
};

/// Generate MIDI control change event
class nlMidiNode : public nlScriptNode
{
private:
	short mmsg;
public:
	nlMidiNode(short m)
	{
		token = T_MIDICC;
		mmsg = m;
	}
	virtual nlScriptNode *Exec();
};

/// Generate start or stop track command
class nlTrackNode : public nlScriptNode
{
public:
	nlTrackNode(short f)
	{
		token = f;
	}
	virtual nlScriptNode *Exec();
};


/// @brief The Notelist generator class.
/// @details nlGenerate is the second pass of the interpreter.
/// It steps through each parsed node in turn, executes each.
/// This class contains the evaluation stack used by expression
/// evaluator, a list of sequences, and a list of voices.
class nlGenerate  
{
private:
	nlSequence *curSeq;
	nlConverter *cvtPtr;
	nlVoice *curVoice;
	nlVoice *voiceList;
	nlSequence *mainSeq;

	nlVarValue *vStack;
	nlVarValue *spEnd;
	nlVarValue *spCur;
	nlSyncMark *synclist;

	long maxParam;
	long freqmode;
	long voldbmode;
	long velocity;
	double beat;
	double secBeat;
	double nlVersion;

	void Clear();

public:
	nlGenerate();
	virtual ~nlGenerate();


	void SetConverter(nlConverter *p) {cvtPtr = p; }
	nlConverter *GetConverter() { return cvtPtr; }
	nlVoice *GetCurVoice() { return curVoice; }
	nlVoice *SetCurVoice(int n);

	void SyncTo(nlVarValue *v);
	void MarkTo(nlVarValue *v);

	void SetMaxParam(long n) { maxParam = n; }
	long GetMaxParam() { return maxParam; }
	void SetVersion(double v) { nlVersion = v; }
	double GetVersion() { return nlVersion; }

	void SetFrequencyMode(long n) { freqmode = n; }
	long GetFrequencyMode() { return freqmode; }
	void SetVoldbMode(long n) { voldbmode = n; }
	long GetVoldbMode() { return voldbmode; }
	void SetVelocityOn(long n) { velocity = n; }
	long GetVelocityOn() { return velocity; }

	void SetTempo(double b, double t)
	{
		beat = b;
		secBeat = t;
	}

	double ConvertRhythm(double d)
	{
		return beat / d * secBeat;
	}

	nlFunctionData *iFnGen[MAXFGEN];

	nlSequence *FindSequence(nlVarValue *id);
	void Reset();
	int Run();
	void InitStack();
	void PushStack(long n);
	void PushStack(double d);
	void PushStack(const char *s);
	void PushStack(nlVarValue *v);
	void PopStack(long *n);
	void PopStack(double *d);
	void PopStack(char **s);
	void PopStack(nlVarValue *v);
	nlSequence *AddSequence(const char *id);
	nlSequence *AddSequence(int id);
	nlSequence *SetCurSeq(nlSequence *p);
	nlScriptNode *AddNode(nlScriptNode *pn);
	nlScriptNode *AddNode(int token, const char *text);
	nlScriptNode *AddNode(int token, long val);
	nlScriptNode *AddNode(int token, short v1, short v2);
	nlScriptNode *AddNode(int token, double val);
};

#endif // !defined(_GENERATE_H_)
