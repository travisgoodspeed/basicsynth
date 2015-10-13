//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef PROJECT_ITEM_H
#define PROJECT_ITEM_H


/// Project item types.
enum PIType
{
	 PRJNODE_UNKNOWN = 0,
	 PRJNODE_PROJECT,
	 PRJNODE_WAVEOUT,
	 PRJNODE_FILELIST,
	 PRJNODE_SYNTH,
	 PRJNODE_NOTELIST,
	 PRJNODE_NOTEFILE,
	 PRJNODE_SEQLIST,
	 PRJNODE_SEQFILE,
	 PRJNODE_TEXTLIST,
	 PRJNODE_TEXTFILE,
	 PJRNODE_SCRIPTLIST,
	 PRJNODE_SCRIPT,
	 PRJNODE_LIBLIST,
	 PRJNODE_LIB,
	 PRJNODE_LIBINSTR,
	 PRJNODE_INSTRLIST,
	 PRJNODE_INSTR,
	 PRJNODE_WVTABLE,
	 PRJNODE_WVFLIST,
	 PRJNODE_WVFILE,
	 PRJNODE_MIXER,
	 PRJNODE_CHANNEL,
	 PRJNODE_REVERB,
	 PRJNODE_FLANGER,
	 PRJNODE_ECHO,
	 PRJNODE_FXITEM,
	 PRJNODE_SELINSTR,
	 PRJNODE_SOUNDBANK,
	 PRJNODE_SBLIST,
	 PRJNODE_SELSOUND,
	 PRJNODE_MIDICTRL,
	 PRJNODE_MIDIFILE
};

#define ITM_ENABLE_NEW   0x0001
#define ITM_ENABLE_ADD   0x0002
#define ITM_ENABLE_COPY  0x0004
#define ITM_ENABLE_REM   0x0008
#define ITM_ENABLE_EDIT  0x0010
#define ITM_ENABLE_PROPS 0x0020
#define ITM_ENABLE_SAVE  0x0040
#define ITM_ENABLE_CLOSE 0x0080
#define ITM_ENABLE_ERRS  0x0100

/// Base class for all project items.
/// Entries in the project file generally match to one class derived
/// from ProjectItem. ProjectItem defines the common properties and
/// functions. Every item has a type, name, and description. The
/// item action variable defines which editor actions are allowed
/// on this item. An item can have a property box, an editor,
/// or both. Items with editors will set the editor member only
/// when the editor is open.
/// @sa See the descriptions for PropertyBox and EditorView for more
/// information.
class ProjectItem
{
protected:
	PIType   type;
	bsString name;
	bsString desc;
	EditorView *editor;
	ProjectItem *parent;
	int leaf;
	int change;
	int actions;
	int refcnt;
	void *psdata;

public:
	ProjectItem(PIType t = PRJNODE_UNKNOWN)
	{
		type = t;
		psdata = 0;
		editor = 0;
		parent = 0;
		leaf = 1;
		change = 0;
		actions = 0;
		refcnt = 0;
	}

	virtual ~ProjectItem()
	{
	}

	inline int AddRef()
	{
		return ++refcnt;
	}

	inline int Release()
	{
		int r = --refcnt;
		if (r == 0)
			delete this;
		return r;
	}

	/// Indicates the item is a leaf (bottom node of the tree).
	/// Leaf nodes cannot have child nodes.
	inline int IsLeaf() { return leaf; }

	// Accessor functions.
	/// Get the item type.
	inline void SetType(PIType t) { type = t; }
	/// Set the item type, typically set during construction.
	inline PIType GetType() { return type; }
	/// Set the parent item.
	inline void SetParent(ProjectItem *p) { parent = p; }
	/// Get the item parent item.
	inline ProjectItem *GetParent() { return parent; }
	/// Set the item name.
	inline void SetName(const char *s) { name = s; }
	/// Get the item name.
	inline const char *GetName() { return name; }
	/// Set the item description.
	inline void SetDesc(const char *s) { desc = s; }
	/// Get the item description.
	inline const char *GetDesc() { return desc; }
	/// Set the opened editor for this item.
	inline void SetEditor(EditorView *p) { editor = p; }
	/// Get the editor for this item if open.
	inline EditorView *GetEditor() { return editor; }
	/// Set the platform-specific data for this item.
	inline void SetPSData(void *p) { psdata = p; }
	/// Get the platform-specific data for this item.
	inline void *GetPSData() { return psdata; }

	/// A set of flags indicating which project functions are applicable to this item.
	/// The values are the constants defined as ITM_ENABLE_* and are combined
	/// into a single flag word. Some actions, such as SAVE and CLOSE are
	/// set dynamically based on the state of the object.
	virtual int ItemActions() { return actions; }
	/// Create a new instance of this item.
	virtual int NewItem() { return 0; }
	/// Add an existing instance of this item.
	virtual int AddItem() { return 0; }
	/// Open the editor for this item.
	virtual int EditItem() { return 0; }
	/// Save the item (only for items with external files.)
	virtual int SaveItem() { return 0; }
	/// Close the editor for this item.
	virtual int CloseItem();
	/// Make a copy of this item.
	virtual int CopyItem() { return 0; }
	/// Remove this item from the project.
	virtual int RemoveItem();
	/// Open the property box for this item.
	virtual int ItemProperties() { return 0; }
	/// Get the changed flag for this item.
	virtual int GetChange() { return change; }
	virtual void SetChange(int c) { change = c; }

	/// Load the properties into the property box.
	virtual int LoadProperties(PropertyBox *pb) { return 1; }
	/// Save the properties from the property box.
	virtual int SaveProperties(PropertyBox *pb) { return 1; }

	/// Load the item from the project file.
	virtual int Load(XmlSynthElem *node);
	/// Save the item to the project file.
	virtual int Save(XmlSynthElem *node);
	/// Create the editor form (if applicable)
	virtual WidgetForm *CreateForm(int xo, int yo);

	/// Get the wildcard for file search.
	/// For file items only, this returns a pattern
	/// in the form File|*.ext| that is used in
	/// the file browser dialog.
	static const char *GetFileSpec(int type);
	/// Get the file extension.
	static const char *GetFileExt(int type);

};


/// A project item that references an external file.
/// A score file (Notelist)
/// or a Sequencer file
/// or a Script file
/// or a Plain text file
/// or a Instrument library file
/// or a MIDI file
class FileItem : public ProjectItem
{
protected:
	bsString file;
	bsString fullPath; // fully resolved path
	short useThis;
	short loaded;
public:

	FileItem(PIType t = PRJNODE_TEXTFILE) : ProjectItem(t)
	{
		useThis = 1;
		loaded = 0;
		actions = ITM_ENABLE_EDIT
				| ITM_ENABLE_CLOSE
			    | ITM_ENABLE_PROPS
				| ITM_ENABLE_REM;
	}
	virtual int ItemActions();
	inline const char *GetFile() { return file; }
	inline void SetFile(const char *f) { file = f; }
	inline void SetUse(short u) { useThis = u; }
	inline short GetUse() { return useThis; }
	inline bsString& PathBuffer() { return fullPath; }
	inline const char *GetFullPath() { return fullPath; }
	inline void SetFullPath(const char *p) { fullPath = p; }

	virtual int ItemProperties();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int CopyFile(const char *srcDir, const char *dstDir);
	virtual int CreateNew();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);

	int Load(XmlSynthElem *node);
	int Save(XmlSynthElem *node);
};

/// ScoreError holds one syntax error for a notelist score.
class ScoreError : public SynthList<ScoreError>
{
private:
	nlSyntaxErr err;

public:
	ScoreError(nlSyntaxErr *e)
	{
		err.file = e->file;
		err.msg = e->msg;
		err.token = e->token;
		err.lineno = e->lineno;
		err.position = e->position;
	}

	inline const char *GetFile() { return err.file; }
	inline const char *GetMsg()  { return err.msg;  }
	inline const char *GetToken() { return err.token; }
	inline long GetLine() { return err.lineno; }
	inline long GetPosition() { return err.position; }
};

class NotelistItem;

/// ErrCB implements the nlErrOut class.
/// This is used to receive callbacks during parsing a Notelist file.
class ErrCB : public nlErrOut
{
public:
	NotelistItem *itm;

	virtual void OutputDebug(const char *s);
	virtual void OutputError(const char *s);
	virtual void OutputError(nlSyntaxErr *se);
	virtual void OutputMessage(const char *s);
};

/// A notelist score file.
class NotelistItem : public FileItem
{
private:
	ScoreError *errFirst;
	ScoreError *errLast;
	short dbgLevel;

public:
	NotelistItem() : FileItem(PRJNODE_NOTEFILE)
	{
		errFirst = 0;
		errLast = 0;
		dbgLevel = 0;
		actions  = ITM_ENABLE_EDIT
				 | ITM_ENABLE_CLOSE
				 | ITM_ENABLE_SAVE
				 | ITM_ENABLE_REM
				 | ITM_ENABLE_ERRS
				 | ITM_ENABLE_PROPS;
	}

	~NotelistItem()
	{
		ClearErrors();
	}

	inline void SetDebug(short d) { dbgLevel = d; }
	inline short GetDebug() { return dbgLevel; }

	virtual int CopyItem();

	ScoreError* EnumErrors(ScoreError *e);
	void AddError(nlSyntaxErr *e);
	void ClearErrors();
	int SyntaxCheck();
	int Convert(nlConverter& cvt);
	int Load(XmlSynthElem *node);
	int Save(XmlSynthElem *node);
};

class SeqItem : public FileItem
{
public:
	SeqItem() : FileItem(PRJNODE_SEQFILE)
	{
		actions  = ITM_ENABLE_EDIT
				 | ITM_ENABLE_CLOSE
				 | ITM_ENABLE_SAVE
				 | ITM_ENABLE_REM
				 /*| ITM_ENABLE_ERRS*/
				 | ITM_ENABLE_PROPS;
	}

	int Convert(SequenceFile& seqf);
};


/// A list of files.
class FileList : public ProjectItem
{
protected:
	bsString xmlChild;
	virtual FileItem *NewAdd(const char *file);

public:
	FileList(PIType t = PRJNODE_TEXTLIST) : ProjectItem(t)
	{
		xmlChild = "text";
		name = "Text Files";
		leaf = 0;
		actions  = ITM_ENABLE_NEW
				 | ITM_ENABLE_ADD
				 | ITM_ENABLE_PROPS
				 | ITM_ENABLE_SAVE;
	}
	virtual FileItem *NewChild()
	{
		return new FileItem(PRJNODE_TEXTFILE);
	}
	virtual const char *FileSpec()
	{
		return GetFileSpec(type);
	}
	virtual const char *FileExt()
	{
		return GetFileExt(type);
	}
	virtual int NewItem();
	virtual int AddItem();
	virtual int SaveItem();
	virtual int CopyFiles(const char *srcDir, const char *dstDir);
	virtual int ItemProperties();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// The list of notelist files.
class NotelistList : public FileList
{
public:
	NotelistList() : FileList(PRJNODE_NOTELIST)
	{
		xmlChild = "score";
		name = "Notelists";
		leaf = 0;
	}

	int Convert(nlConverter& cvt);
	virtual FileItem *NewChild()
	{
		return (FileItem*) new NotelistItem;
	}
};

/// A list of sequence files.
class SeqList : public FileList
{
public:
	SeqList() : FileList(PRJNODE_SEQLIST)
	{
		xmlChild = "seq";
		name = "Sequences";
		leaf = 0;
	}

	int LoadSequences(InstrManager *mgr, Sequencer *seq);

	virtual FileItem *NewChild()
	{
		return (FileItem*) new SeqItem();
	}
};



/// The list of script files.
class ScriptList : public FileList
{
public:
	ScriptList() : FileList(PJRNODE_SCRIPTLIST)
	{
		xmlChild = "script";
		name = "Script Files";
		leaf = 0;
	}

	virtual FileItem *NewChild()
	{
		return new FileItem(PRJNODE_SCRIPT);
	}

	int LoadScripts(nlConverter& cvt);
};

/// An instrument item.
/// This item contains a reference to the actual instrument configuration.
class InstrItem : public ProjectItem
{
private:
	InstrConfig *inc;
	InstrItem *copyOf;

public:
	InstrItem() : ProjectItem(PRJNODE_INSTR)
	{
		copyOf = 0;
		inc = 0;
		actions = ITM_ENABLE_COPY
				| ITM_ENABLE_EDIT
				| ITM_ENABLE_CLOSE
				| ITM_ENABLE_REM
				| ITM_ENABLE_PROPS;
	}

	void SetActions(int a) { actions = a; }
	virtual int ItemActions();

	inline void SetConfig(InstrConfig *e)
	{
		inc = e;
		if (inc)
		{
			name = inc->GetName();
			desc = inc->GetDesc();
		}
	}

	inline InstrConfig *GetConfig()
	{
		return inc;
	}

	void RemoveInstr();

	virtual int CopyItem();
	virtual int EditItem();
	virtual int RemoveItem();
	virtual int SaveItem();
	virtual int CloseItem();
	virtual int ItemProperties();
	virtual WidgetForm *CreateForm(int xo, int yo);

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

};

/// The list of project specific instruments.
class InstrList : public ProjectItem
{
public:
	InstrList() : ProjectItem(PRJNODE_INSTRLIST)
	{
		name = "Instruments";
		leaf = 0;
		actions = ITM_ENABLE_NEW;
	}
	static int NextInum();

	virtual int NewItem();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

};

/// A library of instrument configurations.
class LibfileItem : public FileItem
{
private:
	int propFn;
	bsInt16 baseNum;
	int AddCopy(int f);

public:
	LibfileItem() : FileItem(PRJNODE_LIB)
	{
		propFn = 0;
		leaf = 0;
		baseNum = 0;
		actions = ITM_ENABLE_COPY  // copy to project
			    | ITM_ENABLE_ADD   // copy from project
				| ITM_ENABLE_REM   // remove library from project
				| ITM_ENABLE_PROPS; // edit library properties
	}

	int LoadLib(int added);
	int NextInum();
	int GetFn() { return propFn; }

	virtual int ItemActions();
	virtual int CopyItem();
	virtual int AddItem();
	virtual int RemoveItem();
	virtual int ItemProperties();
	virtual int SaveItem();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);

};

/// A list of instrument libraries.
class LibfileList : public FileList
{
protected:

public:
	LibfileList() : FileList(PRJNODE_LIBLIST)
	{
		xmlChild = "libfile";
		name = "Libraries";
		leaf = 0;
		actions = ITM_ENABLE_NEW  // create new library
			    | ITM_ENABLE_ADD; // add existing library
	}

	virtual FileItem *NewChild()
	{
		return new LibfileItem;
	}

	int LoadLibs();

	virtual int AddItem();
	virtual int NewItem();
};

/// A .sf2 or .dls file
class SoundBankItem : public FileItem
{
private:
	bsInt16 preload; // load samples on file open
	float  normalize; // rescale samples

public:
	SoundBankItem() : FileItem(PRJNODE_SOUNDBANK)
	{
		preload = 0;
		normalize = 1.0;
		actions = ITM_ENABLE_ADD    // add an instrument
			    | ITM_ENABLE_REM   // remove from project
				| ITM_ENABLE_PROPS; // edit properties
	}
	virtual ~SoundBankItem();
	virtual int ItemActions() { return actions; }
	virtual int AddItem();
	virtual int LoadFile();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// A list of instrument libraries.
class SoundBankList : public FileList
{
protected:

public:
	SoundBankList() : FileList(PRJNODE_SBLIST)
	{
		xmlChild = "sndbnk";
		name = "SoundBank";
		leaf = 0;
		actions = ITM_ENABLE_ADD; // add existing library
	}

	virtual FileItem *NewChild()
	{
		return new SoundBankItem;
	}

	int LoadFiles();

	virtual int AddItem();
};

/// A wavetable.
class WavetableItem : public ProjectItem
{
private:
	short wvNdx;
	short wvID;
	short wvParts;
	short gibbs;
	short sumParts;
	bsInt32 *mult;
	double *amps;
	double *phs;

public:

	WavetableItem() : ProjectItem(PRJNODE_WVTABLE)
	{
		wvNdx = 0;
		wvID = 0;
		wvParts = 0;
		gibbs = 0;
		mult = 0;
		amps = 0;
		phs = 0;
		sumParts = 1;
		actions = ITM_ENABLE_EDIT
			    | ITM_ENABLE_PROPS
				| ITM_ENABLE_COPY
				| ITM_ENABLE_REM
				| ITM_ENABLE_CLOSE
				| ITM_ENABLE_SAVE;
	}

	~WavetableItem()
	{
		DeleteParts();
	}

	inline short GetParts() { return wvParts; }
	inline void SetIndex(short n) { wvNdx = n; }
	inline short GetIndex() { return wvNdx; }
	inline void SetID(short n) { wvID = n; }
	inline short GetID() { return wvID; }
	inline void SetGibbs(short n) { gibbs = n; }
	inline short GetGibbs() { return gibbs; }
	inline void SetSum(short n) { sumParts = n; }
	inline short GetSum() { return sumParts; }

	inline int SetPart(int n, bsInt32 m, double a, double p)
	{
		if (n < wvParts)
		{
			mult[n] = m;
			amps[n] = a;
			phs[n] = p;
			return 0;
		}
		return -1;
	}

	inline int GetPart(int n, bsInt32 &m, double& a, double& p)
	{
		if (n < wvParts)
		{
			m = mult[n];
			a = amps[n];
			p = phs[n];
			return 0;
		}
		return -1;
	}

	int AllocParts(short n);
	void DeleteParts();
	int InitWaveform();

	virtual int CopyItem();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
	virtual WidgetForm *CreateForm(int xo, int yo);
};

/// A wave file.
class WavefileItem : public FileItem
{
private:
	bsInt16 wvid;
	long loopStart;
	long loopEnd;
public:
	WavefileItem() : FileItem(PRJNODE_WVFILE)
	{
		loopStart = 0;
		loopEnd = 0;
		wvid = -1;
		actions = ITM_ENABLE_PROPS | ITM_ENABLE_REM;
	}

	inline void SetID(short id) { wvid = id; }
	inline short GetID() { return wvid; }

	WaveFileIn *FindFile(bsInt16 id);
	int LoadFile();

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// A list of wave files.
class WavefileList : public ProjectItem
{
private:

public:
	WavefileList() : ProjectItem(PRJNODE_WVFLIST)
	{
		name = "Wavefiles";
		leaf = 0;
		actions = ITM_ENABLE_ADD | ITM_ENABLE_EDIT;
	}

	int LoadFiles();

	virtual int AddItem();
	virtual int EditItem();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};


/// Base class for effects items.
class FxItem : public ProjectItem
{
protected:
	bsString fxType;
	short nchnl;
	short unit;
	AmpValue vol;
	AmpValue pan;
	AmpValue *send;
public:
	FxItem(PIType t = PRJNODE_FXITEM) : ProjectItem(t)
	{
		nchnl = 0;
		unit = -1;
		vol = 0;
		pan = 0;
		send = 0;
		actions = ITM_ENABLE_PROPS;
	}

	~FxItem()
	{
		delete send;
	}

	const char *GetFxType() { return fxType; }
	inline short GetUnit() { return unit; }
	inline void SetUnit(short n) { unit = n; }

	inline AmpValue GetVol() { return vol; }
	inline AmpValue GetPan() { return pan; }
	inline AmpValue GetSend(int ndx) { return (ndx < nchnl) ? send[ndx] : 0.0; }

	void SetVol(AmpValue v, int imm = 0);
	void SetPan(AmpValue p, int imm = 0);
	void SetSend(int ndx, AmpValue v, int imm = 0);

	virtual void SetChannels(short n);
	virtual void InitMixer();
	virtual void Reset() { };

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// A reverb unit.
class ReverbItem : public FxItem
{
private:
	AmpValue rvt;
	AmpValue lt[6];
	Reverb2 *rvrb;
public:
	ReverbItem();
	~ReverbItem()
	{
		delete rvrb;
	}

	virtual void Reset()
	{
		if (rvrb)
			rvrb->Clear();
	}

	virtual void InitMixer();

	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// A flanger unit.
class FlangerItem : public FxItem
{
private:
	AmpValue mix;
	AmpValue fb;
	AmpValue cntr;
	AmpValue depth;
	FrqValue sweep;
	Flanger *flngr;
public:
	FlangerItem() : FxItem(PRJNODE_FLANGER)
	{
		fxType = "flanger";
		mix = 0.7;
		fb = 0.0;
		cntr = 0.004;
		depth = 0.004;
		sweep = 0.15;
		flngr = new Flanger;
		// actions |= ITM_ENABLE_NEW | ITM_ENABLE_COPY | ITM_ENABLE_REM;
	}

	~FlangerItem()
	{
		delete flngr;
	}

	virtual void Reset()
	{
		if (flngr)
			flngr->Clear();
	}

	virtual void InitMixer();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// An echo unit.
class EchoItem : public FxItem
{
private:
	FrqValue dly;
	AmpValue dec;
	DelayLineR *dlr;
public:
	EchoItem() : FxItem(PRJNODE_ECHO)
	{
		fxType = "echo";
		dly = 0;
		dec = 0;
		dlr = new DelayLineR;
		// actions |= ITM_ENABLE_NEW | ITM_ENABLE_COPY | ITM_ENABLE_REM;
	}

	~EchoItem()
	{
		delete dlr;
	}

	virtual void Reset()
	{
		if (dlr)
			dlr->Clear();
	}

	virtual void InitMixer();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// A mixer input channel.
class ChannelItem : public ProjectItem
{
private:
	AmpValue vol;
	AmpValue pan;
	short on;
	short cn;
public:
	ChannelItem() : ProjectItem(PRJNODE_CHANNEL)
	{
		vol = 0;
		pan = 0;
		on = 0;
		cn = 0;
	}

	inline void SetChnl(int c) { cn = c; }
	inline short GetChnl() { return cn; }
	inline AmpValue GetVol() { return vol; }
	inline AmpValue GetPan() { return pan; }
	inline short GetOn() { return on; }
	void SetVol(AmpValue v, int imm = 0);
	void SetPan(AmpValue p, int imm = 0);
	void SetOn(short o, int imm = 0);

	virtual void InitMixer();
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class MixerEdit;

/// The global mixer.
class MixerItem : public ProjectItem
{
private:
	short mixChnl;
	short fxUnits;
	short mixPanType;
	int editX;
	int editY;
	AmpValue mixVolLft;
	AmpValue mixVolRgt;
	ChannelItem *inputs;
	FxItem **effects;
	MixerEdit *mixEdit;

public:
	MixerItem() : ProjectItem(PRJNODE_MIXER)
	{
		name = "Mixer";
		mixChnl = 0;
		fxUnits = 0;
		mixPanType = panTrig;
		mixVolLft = 1.0;
		mixVolRgt = 1.0;
		inputs = 0;
		effects = 0;
		leaf = 0;
		mixEdit = 0;
		editX = 0;
		editY = 0;
		actions = ITM_ENABLE_EDIT
			    | ITM_ENABLE_PROPS;
	}

	~MixerItem()
	{
		delete[] inputs;
		//for (int n = 0; n < fxUnits; n++)
		//	delete effects[n];
		delete effects;
	}

	inline void SetPanType(short n) { mixPanType = n; }
	inline short GetPanType() { return mixPanType; }

	void SetMasterVol(AmpValue lft, AmpValue rgt, int imm = 0);
	inline void GetMasterVol(AmpValue& lft, AmpValue& rgt)
	{
		lft = mixVolLft;
		rgt = mixVolRgt;
	}

	short GetMixerInputs() { return mixChnl; }
	short GetMixerEffects() { return fxUnits; }
	void SetMixerInputs(int num, int keep);
	void SetMixerEffects(int num, int keep);

	void SetChannelOn(int ndx, short on, int imm = 0);
	short GetChannelOn(int ndx);
	void SetChannelVol(int ndx, AmpValue vol, int imm = 0);
	AmpValue GetChannelVol(int ndx);
	void SetChannelPan(int ndx, AmpValue pan, int imm = 0);
	AmpValue GetChannelPan(int ndx);

	void SetEffectsVol(int ndx, AmpValue vol, int imm = 0);
	AmpValue GetEffectsVol(int ndx);
	void SetEffectsPan(int ndx, AmpValue pan, int imm = 0);
	AmpValue GetEffectsPan(int ndx);
	void SetEffectsSend(int fx, int cn, AmpValue lvl, int imm = 0);
	AmpValue GetEffectsSend(int fx, int cn);

	FxItem *GetEffectsItem(int fx);
	FxItem *AddEffect(const char *type);

	void InitMixer();
	void ResetMixer();
	void EditorClosed();

	virtual int ItemActions();
	virtual int ItemProperties();
	virtual int EditItem();
	virtual int SaveItem();
	virtual int CloseItem();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
	virtual WidgetForm *CreateForm(int xo, int yo);
};

///////////////////////////////////////////////////////////////////////////

/// Ouput file information.
class WaveoutItem : public ProjectItem
{
private:
	bsString outFile;
	float leadIn;
	float tailOut;
	short sampleFmt;
	WaveFile wvf16;
	WaveFileIEEE wvf32;
	WaveOut *wvOut;

public:
	WaveoutItem() : ProjectItem(PRJNODE_WAVEOUT)
	{
		name = "Output";
		leadIn = 0;
		tailOut = 0;
		sampleFmt = 0; // 16-bit PCM
		wvOut = 0;
		actions = ITM_ENABLE_PROPS;
	}

	inline void SetOutfile(const char *s) { outFile = s; }
	inline const char *GetOutfile() { return outFile; }
	inline void SetLeadIn(double n) { leadIn = n; }
	inline double GetLeadIn() { return leadIn; }
	inline void SetTailOut(double n) { tailOut = n; }
	inline double GetTailOut() { return tailOut; }
	inline void SetSampleFmt(short f) { sampleFmt = f; }
	inline short GetSampleFmt() { return sampleFmt; }

	WaveOut *GetOutput();
	WaveOut *InitOutput();
	int CloseOutput(WaveOut *wvout, Mixer *mix);

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/////////////////////////////////////////////////////////////////////

/// Syntheiszer setup information.
class SynthItem : public ProjectItem
{
private:
	long sampleRate;
	long wtSize;
	long wtUser;
	long newSize;
	int loaded;

	WavetableItem *AddWavetable(int ndx);

public:
	SynthItem() : ProjectItem(PRJNODE_SYNTH)
	{
		name = "Synthesizer";
		desc = "Synthesizer parameters";
		sampleRate = 44100;
		wtSize = 16384;
		newSize = wtSize;
		loaded = 0;
		wtUser = 0;
		leaf = 0;
		actions = ITM_ENABLE_PROPS | ITM_ENABLE_NEW;
	}

	inline void SetSampleRate(long r) { sampleRate = r; }
	inline long GetSampleRate() { return sampleRate; }
	inline void SetWTSize(short n) { wtSize = n; }
	inline short GetWTSize() { return wtSize; }
	inline void SetWTUser(short n) { wtUser = n; }
	inline short GetWTUser() { return wtUser; }

	void InitSynth();
	void NewProject();

	virtual int NewItem();
	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

class MidiFile : public FileItem
{
public:
	MidiFile() : FileItem(PRJNODE_MIDIFILE)
	{
		actions  = ITM_ENABLE_REM | ITM_ENABLE_PROPS;
	}
};

class MidiItem : public FileList
{
private:
	bsString sbFile;
	SoundBank *sbnk;
public:
	MidiItem() : FileList(PRJNODE_MIDICTRL)
	{
		actions = ITM_ENABLE_EDIT | ITM_ENABLE_CLOSE | ITM_ENABLE_ADD;
		name = "MIDI";
		xmlChild = "smf";
		leaf = 0;
		sbnk = 0;
	}

	virtual int EditItem();
	virtual WidgetForm *CreateForm(int xo, int yo);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

	virtual FileItem *NewChild()
	{
		return (FileItem*) new MidiFile();
	}

	int Generate(InstrManager *mgr, Sequencer *seq);

	const char *GetSoundBankFile() 
	{
		return sbFile; 
	}
	
	void SetSoundBankFile(const char *file)
	{
		sbFile = file;
		if (sbnk)
			sbnk->Unlock();
		sbnk = SoundBank::FindBankFile(file);
		if (sbnk)
			sbnk->Lock();
	}
};

/// A list of files/directories, e.g., a search path.
class PathListItem  : public SynthList<PathListItem >
{
public:
	bsString path;
	PathListItem(const char *p = 0)
	{
		path = p;
	}
};

/// The file search path.
class PathList : public ProjectItem
{
private:
	PathListItem head;
	PathListItem tail;

public:
	PathList() : ProjectItem(PRJNODE_FILELIST)
	{
		head.Insert(&tail);
	}

	~PathList()
	{
		RemoveAll();
	}

	void RemoveAll()
	{
		PathListItem *itm;
		while ((itm = head.next) != &tail)
		{
			itm->Remove();
			delete itm;
		}
	}

	static int FullPath(const char *fname);
	virtual void AddItem(const char *name);
	virtual void AddItem(PathListItem *itm);
	virtual void RemoveItem(const char *name);
	PathListItem* EnumList(PathListItem *itm);
	int ResolvePath(FileItem *fi);
	int FindOnPath(bsString& fullPath, const char *fname);

	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);
};

/// The synthesis project.
/// There is exactly one of these whenever a project is open
/// and it is referenced by the global variable theProject.
class SynthProject : public ProjectItem, public SynthThread
{
private:
	bsString author;
	bsString cpyrgt;
	bsString prjPath;
	bsString prjDir;
	float latency;
	bsInt32 playFrom;
	bsInt32 playTo;
	bsInt32 playMode;  ///< 0 = to disk, 1 = sequence live, 2 = keyboard
	float cbRate;
	bsString lastError;
	bsString wvInPath;

	void Init();

	void InitProject();

	static void SeqCallback(bsInt32 count, Opaque arg);
	void UpdateGenerator(bsInt32 count);
	int GenerateSequence(nlConverter& cvt);
	int GenerateToFile();
	int Generate();
	int Play();
	int Finished(int ret);

public:
	SynthItem     *synthInfo;
	NotelistList  *nlInfo;
	SeqList       *seqInfo;
	FileList      *txtInfo;
	ScriptList    *scriptInfo;
	MixerItem     *mixInfo;
	InstrList     *instrInfo;
	WaveoutItem   *wvoutInfo;
	WavefileList  *wfInfo;
	LibfileList   *libInfo;
	SoundBankList *sblInfo;
	PathList      *libPath;
	MidiItem      *midiInfo;
	MIDIInput     prjMidiIn;

	Mixer mix;
	InstrManager mgr;
	Sequencer seq;
	nlConverter *cvtActive;
	WaveOut *wop;    ///< Sound device when playing live.

	SynthProject() : ProjectItem(PRJNODE_PROJECT)
	{
		leaf = 0;
		Init();
		actions = ITM_ENABLE_PROPS;
	}

	~SynthProject() 
	{
		synthInfo->Release();
		nlInfo->Release();
		seqInfo->Release();
		txtInfo->Release();
		scriptInfo->Release();
		mixInfo->Release();
		instrInfo->Release();
		wvoutInfo->Release();
		wfInfo->Release();
		libInfo->Release();
		sblInfo->Release();
		libPath->Release();
		midiInfo->Release();
	}

	inline void SetAuthor(const char *s) { author = s; }
	inline const char *GetAuthor() { return author; }
	inline void SetCopyright(const char *s) { cpyrgt = s; }
	inline const char *GetCopyright() { return cpyrgt; }
	inline void SetWavePath(const char *s) { wvInPath = s; }
	inline const char *GetWavePath() { return wvInPath; }
	inline const char *WhatHappened() { return lastError; }

	static char *NormalizePath(char *path);
	static char *CheckExtension(char *path, const char *ext);
	static char *SkipProjectDir(char *path);
	static char *SaveStringCopy(const char *s);
	static int FullPath(const char *s);

	int NewProject(const char *fname = 0);
	int LoadProject(const char *fname);
	int SaveProject(const char *fname = 0);
	int IsPlaying();
	int PlayEvent(SeqEvent *evt);
	int Start();
	int Stop();
	int Pause();
	int Resume();
	int ThreadProc();
	int SetupSoundDevice(float latency);

	void StartTime(bsInt32 t) { playFrom = t; }
	void EndTime(bsInt32 t)   { playTo = t; }
	void PlayMode(bsInt32 m)  { playMode = m; }
	void CallbackRate(float r) { cbRate = r; }
	float CallbackRate() { return cbRate; }

	void SetProjectPath(const char *path)
	{
		prjPath = path;
		int sl = prjPath.FindReverse(0, '/');
		prjPath.SubString(prjDir, 0, sl);
	}

	void GetProjectPath(bsString& path)
	{
		path = prjPath;
	}

	void GetProjectDir(bsString& path)
	{
		path = prjDir;
	}

	int FindOnPath(bsString& fullPath, const char *fname);
	int FindForm(bsString& fullPath, const char *fname);

	virtual int ItemProperties();
	virtual int LoadProperties(PropertyBox *pb);
	virtual int SaveProperties(PropertyBox *pb);
	virtual int Load(XmlSynthElem *node);
	virtual int Save(XmlSynthElem *node);

	int CopyFiles(const char *oldDir, const char *newDir);
};

extern SynthProject *theProject;

#endif
