//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents the whole project.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

// TODO: Need better error checking and reporting for load/save.

// Global variables
SynthProject *theProject;
ProjectTree  *prjTree;
GenerateWindow *prjGenerate;
ProjectFrame *prjFrame;
ProjectOptions prjOptions;

// Default instrument template cleanup.
// This is used when the instrument template
// is a copy of the instrument object.
// When an instrument has a custom template,
// the instrument template destroy is used instead.
static void DestroyTemplate(Opaque tp)
{
	Instrument *ip = (Instrument *)tp;
	delete ip;
}

// Initialization - adds all instrument types to the instrument manager.
// Typically the inclInstr flags are set to include all types. For a
// custom version of BasicSynth the flags can be set so that only
// some instruments are made available. N.B. - disabling instruments
// can cause problems when loading a project that contains disabled
// instrument types... This needs some more work.
void SynthProject::Init()
{
	synthInfo = 0;
	midiInfo = 0;
	nlInfo = 0;
	seqInfo = 0;
	txtInfo = 0;
	scriptInfo = 0;
	mixInfo = 0;
	instrInfo = 0;
	wvoutInfo = 0;
	wfInfo = 0;
	libInfo = 0;
	libPath = 0;
	change = 0;
	cvtActive = 0;
	wop = 0;
	playFrom = 0;
	playTo = 0;
	playMode = 0;
	cbRate = 1.0;

	prjMidiIn.SetSequenceInfo(&seq, &mgr);
	prjMidiIn.SetDevice(prjOptions.midiDevice, prjOptions.midiDeviceName);
	seq.SetResolution(prjOptions.tickRes);

	InstrMapEntry *ime;
	if (prjOptions.inclInstr & 0x001)
	{
		ime = mgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
		ime->paramToID = ToneInstr::MapParamID;
		ime->paramToName = ToneInstr::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}

	if (prjOptions.inclInstr & 0x002)
	{
		ime = mgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
		ime->paramToID = ToneFM::MapParamID;
		ime->paramToName = ToneFM::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x004)
	{
		ime = mgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
		ime->paramToID = AddSynth::MapParamID;
		ime->paramToName = AddSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x008)
	{
		ime = mgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
		ime->paramToID = SubSynth::MapParamID;
		ime->paramToName = SubSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}

	if (prjOptions.inclInstr & 0x010)
	{
		ime = mgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
		ime->paramToID = FMSynth::MapParamID;
		ime->paramToName = FMSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x020)
	{
		ime = mgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
		ime->paramToID = MatrixSynth::MapParamID;
		ime->paramToName = MatrixSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x040)
	{
		ime = mgr.AddType("ModSynth", ModSynth::ModSynthFactory, ModSynth::ModSynthEventFactory);
		ime->paramToID = ModSynth::MapParamID;
		ime->paramToName = ModSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x080)
	{
		ime = mgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
		ime->paramToID = WFSynth::MapParamID;
		ime->paramToName = WFSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x100)
	{
		ime = mgr.AddType("Chuffer", Chuffer::ChufferFactory, Chuffer::ChufferEventFactory);
		ime->paramToID = Chuffer::MapParamID;
		ime->paramToName = Chuffer::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x200)
	{
		ime = mgr.AddType("MixerControl", MixerControl::MixerControlFactory, MixerControl::MixerControlEventFactory);
		ime->paramToID = MixerControl::MapParamID;
		ime->paramToName = MixerControl::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
		InstrConfig *mi = mgr.AddInstrument(-1, ime, 0);
		mi->SetName("[mixer]");
	}
	if (prjOptions.inclInstr & 0x400)
	{
		ime = mgr.AddType("SoundBank", SFPlayerInstr::SFPlayerInstrFactory, SFPlayerInstr::SFPlayerEventFactory);
		ime->paramToID = SFPlayerInstr::MapParamID;
		ime->paramToName = SFPlayerInstr::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
	if (prjOptions.inclInstr & 0x800)
	{
		ime = mgr.AddType("GMPlayer", GMPlayer::InstrFactory, GMPlayer::EventFactory);
		ime->paramToID = GMPlayer::MapParamID;
		ime->paramToName = GMPlayer::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
		InstrConfig *mi = mgr.AddInstrument(-1, ime, GMPlayer::InstrFactory(&mgr, 0));
		mi->SetName("[midiplayer]");
	}
	if (prjOptions.inclInstr & 0x1000)
	{
		ime = mgr.AddType("BuzzSynth", BuzzSynth::InstrFactory, BuzzSynth::EventFactory);
		ime->paramToID = BuzzSynth::MapParamID;
		ime->paramToName = BuzzSynth::MapParamName;
		ime->dumpTmplt = DestroyTemplate;
	}
}

// Initialize the project tree information
void SynthProject::InitProject()
{
	name = "Project";
	parent = 0;
	prjTree->AddNode(this);

	synthInfo = new SynthItem;
	synthInfo->AddRef();
	synthInfo->SetParent(this);
	prjTree->AddNode(synthInfo);

	wvoutInfo = new WaveoutItem;
	wvoutInfo->AddRef();
	wvoutInfo->SetParent(this);
	prjTree->AddNode(wvoutInfo);

	mixInfo = new MixerItem;
	mixInfo->AddRef();
	mixInfo->SetParent(this);
	prjTree->AddNode(mixInfo);

	nlInfo = new NotelistList;
	nlInfo->AddRef();
	nlInfo->SetParent(this);
	if (prjOptions.inclNotelist)
		prjTree->AddNode(nlInfo);

	scriptInfo = new ScriptList;
	scriptInfo->AddRef();
	scriptInfo->SetParent(this);
	if (prjOptions.inclScripts)
		prjTree->AddNode(scriptInfo);

	seqInfo = new SeqList;
	seqInfo->AddRef();
	seqInfo->SetParent(this);
	if (prjOptions.inclSequence)
		prjTree->AddNode(seqInfo);

	txtInfo = new FileList;
	txtInfo->AddRef();
	txtInfo->SetParent(this);
	if (prjOptions.inclTextFiles)
		prjTree->AddNode(txtInfo);

	instrInfo = new InstrList;
	instrInfo->AddRef();
	instrInfo->SetParent(this);
	prjTree->AddNode(instrInfo);

	wfInfo = new WavefileList;
	wfInfo->AddRef();
	wfInfo->SetParent(this);
	prjTree->AddNode(wfInfo);

	libInfo = new LibfileList;
	libInfo->AddRef();
	libInfo->SetParent(this);
	if (prjOptions.inclLibraries)
		prjTree->AddNode(libInfo);

	midiInfo = new MidiItem;
	midiInfo->AddRef();
	midiInfo->SetParent(this);
	if (prjOptions.inclMIDI)
		prjTree->AddNode(midiInfo);

	sblInfo = new SoundBankList;
	sblInfo->AddRef();
	sblInfo->SetParent(this);
	if (prjOptions.inclSoundFonts)
		prjTree->AddNode(sblInfo);

	libPath = new PathList;
	libPath->AddRef();
	libPath->SetParent(this);

}

// Edit the project properties.
int SynthProject::ItemProperties()
{
	if (prjFrame)
		prjFrame->StopPlayer();
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		if (pb->Activate(1))
			change = 1;
		delete pb;
	}
	return change;
}

// Load properties into the property box
int SynthProject::LoadProperties(PropertyBox *pb)
{
	pb->SetCaption("Project Properties");
	pb->SetValue(PROP_PRJ_NAME, name, 0);
	pb->SetValue(PROP_PRJ_AUTH, author, 0);
	pb->SetValue(PROP_PRJ_CPYR, cpyrgt, 0);
	pb->SetValue(PROP_PRJ_DESC, desc, 0);
	pb->SetValue(PROP_PRJ_PATH, prjPath, 0);
	wvoutInfo->LoadProperties(pb);
	synthInfo->LoadProperties(pb);
	pb->SetValue(PROP_PRJ_WVIN, wvInPath, 0);
	return 1;
}

// Save properties from the property box
int SynthProject::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_PRJ_NAME, name);
	pb->GetValue(PROP_PRJ_AUTH, author);
	pb->GetValue(PROP_PRJ_CPYR, cpyrgt);
	pb->GetValue(PROP_PRJ_DESC, desc);
	pb->GetValue(PROP_PRJ_PATH, prjPath);
	wvoutInfo->SaveProperties(pb);
	synthInfo->SaveProperties(pb);
	pb->GetValue(PROP_PRJ_WVIN, wvInPath);
	return 1;
}

// Load the project file.
int SynthProject::Load(XmlSynthElem *node)
{
	int err = 0;
	int itmerr = 0;
	char *content;
	XmlSynthElem *child = node->FirstChild();
	while (child)
	{
		itmerr = 0;
		if (child->TagMatch("name"))
		{
			child->GetContent(&content);
			name.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("descr"))
		{
			child->GetContent(&content);
			desc.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("author"))
		{
			child->GetContent(&content);
			author.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("cpyrgt"))
		{
			child->GetContent(&content);
			cpyrgt.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("wvdir"))
		{
			child->GetContent(&content);
			wvInPath.Attach(content);
			content = 0;
		}
		else if (child->TagMatch("synth"))
		{
			itmerr = synthInfo->Load(child);
		}
		else if (child->TagMatch("out"))
		{
			itmerr = wvoutInfo->Load(child);
		}
		else if (child->TagMatch("mixer"))
		{
			itmerr = mixInfo->Load(child);
		}
		else if (child->TagMatch("wvfile"))
		{
			WavefileItem *wv = new WavefileItem;
			wv->SetParent(wfInfo);
			itmerr = wv->Load(child);
			prjTree->AddNode(wv);
		}
		else if (child->TagMatch("libdir"))
		{
			char *path = 0;
			if (child->GetContent(&path) == 0)
			{
				libPath->AddItem(path);
				delete path;
			}
		}
		else if (child->TagMatch("libfile"))
		{
			LibfileItem *lib = new LibfileItem;
			lib->SetParent(libInfo);
			itmerr = lib->Load(child);
			if (prjOptions.inclLibraries)
				prjTree->AddNode(lib);
		}
		else if (child->TagMatch("score"))
		{
			NotelistItem *ni = new NotelistItem;
			ni->SetParent(nlInfo);
			itmerr = ni->Load(child);
			if (prjOptions.inclNotelist)
				prjTree->AddNode(ni);
		}
		else if (child->TagMatch("seq"))
		{
			FileItem *fi = seqInfo->NewChild();
			fi->SetParent(seqInfo);
			itmerr = fi->Load(child);
			if (prjOptions.inclSequence)
				prjTree->AddNode(fi);
		}
		else if (child->TagMatch("text"))
		{
			FileItem *fi = txtInfo->NewChild();
			fi->SetParent(txtInfo);
			itmerr = fi->Load(child);
			if (prjOptions.inclTextFiles)
				prjTree->AddNode(fi);
		}
		else if (child->TagMatch("script"))
		{
			FileItem *fi = scriptInfo->NewChild();
			fi->SetParent(scriptInfo);
			itmerr = fi->Load(child);
			if (prjOptions.inclScripts)
				prjTree->AddNode(fi);
		}
		else if (child->TagMatch("sndbnk") || child->TagMatch("sf2") || child->TagMatch("dls"))
		{
			SoundBankItem *sbi = new SoundBankItem;
			sbi->SetParent(sblInfo);
			itmerr = sbi->Load(child);
			if (prjOptions.inclSoundFonts)
				prjTree->AddNode(sbi);
		}
		else if (child->TagMatch("midi"))
		{
			itmerr = midiInfo->Load(child);
		}
		if (itmerr)
		{
			lastError += "Error loading ";
			lastError += child->TagName();
			lastError += "\r\n";
			err++;
		}
		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}

	if (wvInPath.Length())
		synthParams.wvPath = wvInPath;
	else
		synthParams.wvPath = prjDir;
	synthInfo->InitSynth();
	mixInfo->InitMixer();
	wfInfo->LoadFiles();
	if (prjOptions.inclLibraries)
		libInfo->LoadLibs();
	if (prjOptions.inclSoundFonts)
		sblInfo->LoadFiles();

	// We need to wait to do instrument loading until after the synth is initialized
	child = node->FirstChild();
	while (child)
	{
		if (child->TagMatch("instrlib"))
		{
			if (instrInfo->Load(child))
			{
				lastError += "Error loading instrlib\r\n";
				err++;
			}
		}
		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}

//	bsString colorFile;
//	if (FindForm(colorFile, prjOptions.colorsFile))
//		SynthWidget::colorMap.Load(colorFile);
	return err;
}

int SynthProject::Save(XmlSynthElem *node)
{
	// node points to the document root

	XmlSynthElem *child;

	child = node->AddChild("name");
	child->SetContent(name);
	delete child;

	child = node->AddChild("descr");
	child->SetContent(desc);
	delete child;

	child = node->AddChild("author");
	child->SetContent(author);
	delete child;

	child = node->AddChild("cpyrgt");
	child->SetContent(cpyrgt);
	delete child;

	child = node->AddChild("wvdir");
	child->SetContent(wvInPath);
	delete child;

	libPath->Save(node);

	synthInfo->Save(node);
	wvoutInfo->Save(node);
	mixInfo->Save(node);
	wfInfo->Save(node);
	libInfo->Save(node);
	sblInfo->Save(node);
	instrInfo->Save(node);
	nlInfo->Save(node);
	seqInfo->Save(node);
	txtInfo->Save(node);
	scriptInfo->Save(node);
	midiInfo->Save(node);

	return 0;
}

// Set defaults for a new project.
int SynthProject::NewProject(const char *fname)
{
	InitProject();
	SetProjectPath(fname);
	SetAuthor(prjOptions.defAuthor);
	SetCopyright(prjOptions.defCopyright);
	int ok = ItemProperties();
	prjTree->UpdateNode(this);
	synthInfo->NewProject();
	mixInfo->SetMixerInputs(1, 0);
	mixInfo->SetChannelOn(0, 1);
	mixInfo->SetChannelVol(0, 0.5);
	mixInfo->SetChannelPan(0, 0.0);
	mixInfo->InitMixer();
	if (ok)
		SaveProject();
	return 1;
}

int SynthProject::LoadProject(const char *fname)
{
	InitProject();

	lastError = "";

	XmlSynthDoc doc;
	XmlSynthElem *root;
	if ((root = doc.Open((char*)fname)) == NULL)
	{
		lastError = "Cannot open file: ";
		lastError += fname;
		lastError += "Project files must be valid XML.";
		return -1;
	}

	if (!root->TagMatch("synthprj"))
	{
		lastError = "The project file has the wrong root tag.";
		delete root;
		return -2;
	}

	SetProjectPath(fname);

	int err = Load(root);

	delete root;
	doc.Close();

	prjTree->UpdateNode(this);

	change = 0;
	return err;
}

int SynthProject::SaveProject(const char *fname)
{
	if (fname == 0)
	{
		if (prjPath.Length() == 0)
		{
			lastError = "Project name was not specified";
			return -1;
		}
		fname = prjPath;
	}

	XmlSynthDoc doc;
	XmlSynthElem *root;
	if ((root = doc.NewDoc("synthprj")) == NULL)
	{
		lastError = "Cannot create new project document";
		return -1;
	}

	int err = Save(root);

	delete root;
	if (err == 0)
	{
		if (doc.Save((char*)fname))
		{
			lastError = "Cannot write output file ";
			lastError += fname;
			return -1;
		}
	}
	change = 0;
	return err;
}

// Copy files that are associated with the project
int SynthProject::CopyFiles(const char *oldDir, const char *newDir)
{
	int err = 0;
	if (nlInfo)
		err |= nlInfo->CopyFiles(oldDir, newDir);
	if (seqInfo)
		err |= seqInfo->CopyFiles(oldDir, newDir);
	if (txtInfo)
		err |= txtInfo->CopyFiles(oldDir, newDir);
	if (scriptInfo)
		err |= scriptInfo->CopyFiles(oldDir, newDir);
	if (libInfo)
		err |= libInfo->CopyFiles(oldDir, newDir);
	return err;
}

// Make all the path delimiters the same.
char *SynthProject::NormalizePath(char *path)
{
	char *sl = path;
	while ((sl = strchr(sl, '\\')) != NULL)
		*sl++ = '/';
	return path;
}

// If the path doesn't have an extension, add the default.
char *SynthProject::CheckExtension(char *path, const char *ext)
{
	if (ext == NULL)
		return path;

	char *slsh = strrchr(path, '/');
	if (slsh == NULL)
		slsh = path;
	char *dot = strrchr(slsh, '.');
	if (dot == NULL)
	{
		strcat(path, ".");
		strcat(path, ext);
	}
	return path;
}

// Find the part of the path past the project directory.
char *SynthProject::SkipProjectDir(char *path)
{
	size_t len = theProject->prjDir.Length();
	if (strncmp(theProject->prjDir, path, len) == 0 && path[len] == '/')
		return path+len+1;
	return path;
}

// Exampand to a full path if needed.
int SynthProject::FullPath(const char *s)
{
	return PathList::FullPath(s);
}

// Search for a file in the lib path list.
int SynthProject::FindOnPath(bsString& fullPath, const char *fname)
{
	return libPath->FindOnPath(fullPath, fname);
}

// Find an editor form
int SynthProject::FindForm(bsString& fullPath, const char *fname)
{
	fullPath = prjOptions.formsDir;
	fullPath += '/';
	fullPath += fname;
	if (SynthFileExists(fullPath))
		return 1;
	return FindOnPath(fullPath, fname);
}

// Handle callbacks from the sequencer.
// If the project generate dialog is visible
// the time and peak amplitude levels are updated.
void SynthProject::SeqCallback(bsInt32 count, Opaque arg)
{
	((SynthProject*)arg)->UpdateGenerator(count);
}

void SynthProject::UpdateGenerator(bsInt32 count)
{
	if (prjGenerate)
	{
		AmpValue lftPeak;
		AmpValue rgtPeak;
		mix.Peak(lftPeak, rgtPeak);
		prjGenerate->UpdatePeak(lftPeak, rgtPeak);
		prjGenerate->UpdateTime(count);
		if (prjGenerate->WasCanceled())
			seq.Halt();
	}
}

// Add sequencer events from the Notelist files.
int SynthProject::GenerateSequence(nlConverter& cvt)
{
	cvtActive = &cvt;

	if (prjGenerate)
		prjGenerate->AddMessage("Generate sequences...");

	// clear existing sequence
	seq.Reset();

	int err = 0;
	if (seqInfo)
		err |= seqInfo->LoadSequences(&mgr, &seq);
	if (nlInfo)
		err |= nlInfo->Convert(cvt);
	if (err == 0)
	{
		ErrCB ecb;
		ecb.itm = 0;
		cvt.SetErrorCallback(&ecb);
		err = cvt.Generate();
	}
	cvtActive = 0;
	if (midiInfo)
		err |= midiInfo->Generate(&mgr, &seq);
	return err;
}

int SynthProject::GenerateToFile()
{
	if (!wvoutInfo)
		return -1;
	mixInfo->InitMixer();
	mix.Reset();
	mgr.Init(&mix, wvoutInfo->GetOutput());

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	if (GenerateSequence(cvt))
		return -1;

	WaveOut *wvOut = wvoutInfo->InitOutput();
	if (!wvOut)
	{
		if (prjGenerate)
			prjGenerate->AddMessage("Could not initialize output.");
		return -1;
	}
	if (prjGenerate)
		prjGenerate->AddMessage("Start sequencer...");
	seq.SetCB(SeqCallback, (bsInt32)(synthParams.sampleRate * cbRate), (Opaque)this);
	seq.Sequence(mgr, playFrom*synthParams.isampleRate, playTo*synthParams.isampleRate);
	seq.SetCB(0, 0, 0);
	wvoutInfo->CloseOutput(wvOut, &mix);

	// reset in case of dynamic mixer control changes
	mixInfo->InitMixer();
	return 0;
}

int SynthProject::Finished(int ret)
{
	if (prjGenerate)
		prjGenerate->Finished();
	return ret;
}

int SynthProject::Generate()
{
	if (SetupSoundDevice(prjOptions.playBuf))
		return -1;

	mixInfo->InitMixer();
	mgr.Init(&mix, wop);

	nlConverter cvt;
	cvt.SetInstrManager(&mgr);
	cvt.SetSequencer(&seq);
	cvt.SetSampleRate(synthParams.sampleRate);

	int ok = 0;
	if ((ok = GenerateSequence(cvt)) == 0)
	{
		if (prjGenerate)
			prjGenerate->AddMessage("Start sequencer...");
		seq.SetCB(SeqCallback, (bsInt32)(synthParams.sampleRate * cbRate), (Opaque)this);

		// For now, if MIDI is currently active, allow playing along
		// with the sequence. This might need to be changed to a separate
		// option.
		SeqState ss = seqSeqOnce;
		if (prjMidiIn.IsOn())
			ss |= seqPlay;

		// Generate the output...
		bsInt32 fromSamp = (playFrom * synthParams.isampleRate) / 10;
		bsInt32 toSamp = (playTo * synthParams.isampleRate) / 10;
		if (seq.GetTrackCount() > 1 || ss & seqPlay)
			seq.SequenceMulti(mgr, fromSamp, toSamp, ss);
		else // optimal single track playback
			seq.Sequence(mgr, fromSamp, toSamp);
		seq.SetCB(0, 0, 0);

		int cancel = 0;
		if (prjGenerate)
			cancel = prjGenerate->WasCanceled();
		if (!cancel)
		{
			AmpValue lv, rv;
			long pad;
			// Drain all output.
			if (wvoutInfo)
				pad = (long) (wvoutInfo->GetTailOut() * synthParams.sampleRate);
			else
				pad = (long) (synthParams.sampleRate * prjOptions.playBuf);

			while (pad-- > 0 && !cancel)
			{
				mix.Out(&lv, &rv);
				wop->Output2(lv, rv);
				if (prjGenerate)
					cancel = prjGenerate->WasCanceled();
			}
		}
	}

	wop->Shutdown();
	delete wop;
	wop = NULL;

	// re-initialize in case of dynamic mixer control changes
	mixInfo->InitMixer();

	return ok;
}

// Start live playback from keyboard (virtual and/or MIDI).
// This runs as a background thread. (See Start/Stop below)
int SynthProject::Play()
{
	// select minimum latency
	if (SetupSoundDevice(0.0f))
		return -1;
	mix.Reset();
	mgr.Init(&mix, wop);
	seq.SetCB(0, 0, 0);
	seq.Play(mgr);
//	wop->Stop();
	wop->Shutdown();
	delete wop;
	wop = 0;
	return 0;
}

int SynthProject::ThreadProc()
{
	int ret = -1;
	switch (playMode)
	{
	case 0:
		ret = GenerateToFile();
		break;
	case 1:
		ret = Generate();
		break;
	case 2:
		ret = Play();
		break;
	}
	return Finished(ret);
}

int SynthProject::Start()
{
	if (seq.GetState() == seqOff)
		return StartThread(15);
	return seq.GetState() != seqOff;
}

int SynthProject::Stop()
{
	SeqState wasRunning = seq.GetState();
	if (wasRunning != seqOff)
	{
		seq.Halt();
		WaitThread();
	}
	return wasRunning != seqOff;
}

// Pause sequencer playback. First signal the sequencer
// and then halt the live output buffer. It's important
// to wait for the sequencer to enter the pause
// state before stopping output. Otherwise you can get
// into a deadlock where the sequencer is waiting on the
// output buffer.
int SynthProject::Pause()
{
	if (seq.GetState() != seqPaused)
	{
		seq.Pause();
		while (seq.GetState() != seqPaused)
			ShortWait();
		if (wop)
			wop->Stop();
		return 1;
	}
	return 0;
}

// Resume sequencer playback. First restart the output
// buffer then signal the sequencer to continue.
int SynthProject::Resume()
{
	if (seq.GetState() == seqPaused)
	{
		if (wop)
			wop->Restart();
		seq.Resume();
		return 1;
	}
	return 0;
}

int SynthProject::PlayEvent(SeqEvent *evt)
{
	if (seq.GetState() & seqPlay)
	{
		seq.AddImmediate(evt);
		return 1;
	}

	delete evt;
	return 0;
}

int SynthProject::IsPlaying()
{
	return seq.GetState() & seqPlay;
}
