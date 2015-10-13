//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item to represent synthesis parameters.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "MIDIControlEd.h"
#include <SMFFile.h>

///////////////////////////////////////////////
// FIXME: Wavetables can also be allocated
// in library files. we need to discover the
// total number of wavetables and init
// appropriately. In an initial load from the
// project file, libraries are loaded after this
// point and will extend the wavetable set as
// needed. BUT - if InitSynth() is called again,
// the library tables will get discarded and then...
// crash.
///////////////////////////////////////////////

void SynthItem::InitSynth()
{
	loaded = 1;
	InitSynthesizer(sampleRate, wtSize, wtUser);

	WavetableItem *wv = (WavetableItem *)prjTree->FirstChild(this);
	while (wv)
	{
		if (wv->GetType() == PRJNODE_WVTABLE)
		{
			if (wv->GetParts() == 0)
			{
				wv->AllocParts(1);
				wv->SetPart(0, 1, 1.0, 0.0);
			}
			wv->InitWaveform();
		}
		wv = (WavetableItem *)prjTree->NextSibling(wv);
	}
}

int SynthItem::ItemProperties()
{
	return theProject->ItemProperties();
}

WavetableItem *SynthItem::AddWavetable(int ndx)
{
	WavetableItem *wvitm = new WavetableItem;
	//char nm[40];
	//snprintf(nm, 40, "#%d", WT_USR(ndx));
	bsString nm("wt");
	nm += (long)WT_USR(ndx);
	wvitm->SetName(nm);
	wvitm->SetSum(1);
	wvitm->SetGibbs(0);
	wvitm->SetParent(this);
	wvitm->SetID(WT_USR(ndx));
	wvitm->SetIndex(wtSet.wavTblMax);
	prjTree->AddNode(wvitm);
	return wvitm;
}

int SynthItem::NewItem()
{
	WavetableItem *wnew;
	wnew = AddWavetable(wtUser++);
	wnew->AllocParts(1);
	wnew->SetPart(0, 1, 1.0, 0.0);
	wnew->InitWaveform();
	wnew->ItemProperties();
	prjTree->UpdateNode(wnew);
	theProject->SetChange(1);
	wnew->EditItem();
	return 1;
}

int SynthItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_PRJ_SRT, sampleRate, 0);
	pb->SetValue(PROP_PRJ_WTSZ, newSize, 0);
	pb->SetValue(PROP_PRJ_WTU, wtUser, 0);
	if (loaded)
		pb->EnableValue(PROP_PRJ_WTU, 0);
	return 1;
}

int SynthItem::SaveProperties(PropertyBox *pb)
{
	long newRate = 0;
	pb->GetValue(PROP_PRJ_SRT, newRate);
	if (newRate != sampleRate)
	{
		sampleRate = newRate;
		if (loaded)
			synthParams.Init(sampleRate, wtSize);
	}

	// wtUser is calculated as the project is saved/loaded
	// except for a new project where we let the user
	// specify the number of extra wavetables to start with.
	if (!loaded)
		pb->GetValue(PROP_PRJ_WTU, wtUser);

	// If wtSize gets changed, we need to re-initialize the synthesizer.
	// At present we can't do that reliably. So, the change is stored and
	// will be applied on the next project open. As with wtUser, for a
	// new project, the user can specify a value that will be applied
	// for the first init.
	pb->GetValue(PROP_PRJ_WTSZ, newSize);
	if (newSize != wtSize)
	{
		if (loaded)
			prjFrame->Alert("Change to the wavetable size will be applied the next time the project is loaded.", "Note!");
		else
			wtSize = newSize;
	}
	return 1;
}

void SynthItem::NewProject()
{
	if (wtUser > 0)
	{
		for (long ndx = 0; ndx < wtUser; ndx++)
			AddWavetable(ndx);
		wtSet.SetMax(wtSet.wavTblMax + wtUser);
	}
	InitSynth();
}

int SynthItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("sr", sampleRate);
	node->GetAttribute("wt", wtSize);
	newSize = wtSize;
	wtUser = 0;
	int wvIndex = WT_USR(0);

	WavetableItem *wvitm;
	XmlSynthElem *wvnode = node->FirstChild();
	while (wvnode)
	{
		if (wvnode->TagMatch("wvtable"))
		{
			wvitm = new WavetableItem;
			wvitm->SetParent(this);
			wvitm->SetIndex(wvIndex++);
			wvitm->Load(wvnode);
			prjTree->AddNode(wvitm);
			short n = wvitm->GetID() - WT_USR(0);
			if (n >= wtUser)
				wtUser = n+1;
		}
		XmlSynthElem *sib = wvnode->NextSibling();
		delete wvnode;
		wvnode = sib;
	}
	return 0;
}

int SynthItem::Save(XmlSynthElem *node)
{
	XmlSynthElem *synth = node->AddChild("synth");
	if (!synth)
		return -1;

	int err = 0;
	synth->SetAttribute("sr", sampleRate);
	synth->SetAttribute("wt", newSize);

	short usr = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVTABLE)
		{
			XmlSynthElem *child = synth->AddChild("wvtable");
			if (child)
			{
				usr++;
				pi->Save(child);
				delete child;
			}
			else
				err++;
		}
		pi = prjTree->NextSibling(pi);
	}
	synth->SetAttribute("usr", usr);
	delete synth;
	return err;
}

//////////////////////////////////////////////

int MidiItem::Load(XmlSynthElem *node)
{
	short chnl;
	bsInt16 val;

	XmlSynthElem *chnlNode = node->FirstChild();
	while (chnlNode)
	{
		if (chnlNode->TagMatch("chnl"))
		{
			chnlNode->GetAttribute("cn", chnl);
			chnlNode->GetAttribute("bnk", val);
			theProject->mgr.SetBank(chnl, val);
			chnlNode->GetAttribute("prg", val);
			theProject->mgr.SetPatch(chnl, val);
			chnlNode->GetAttribute("vol", val);
			theProject->mgr.SetVolume(chnl, val);
			chnlNode->GetAttribute("pan", val);
			theProject->mgr.SetPan(chnl, val);
		}
		else if (chnlNode->TagMatch("sbfile"))
		{
			char *fname = 0;
			chnlNode->GetContent(&fname);
			sbFile.Attach(fname);
		}

		XmlSynthElem *n = chnlNode->NextSibling();
		delete chnlNode;
		chnlNode = n;
	}

	return FileList::Load(node);
}

int MidiItem::Save(XmlSynthElem *node)
{
	XmlSynthElem *midi = node->AddChild("midi");
	if (!midi)
		return -1;

	XmlSynthElem *chnlNode;
	short chnl;

	for (chnl = 0; chnl < 16; chnl++)
	{
		chnlNode = midi->AddChild("chnl");
		if (!chnlNode)
			return -1;
		chnlNode->SetAttribute("cn", chnl);
		chnlNode->SetAttribute("bnk", theProject->mgr.GetBank(chnl));
		chnlNode->SetAttribute("prg", theProject->mgr.GetPatch(chnl));
		chnlNode->SetAttribute("vol", theProject->mgr.GetVolume(chnl));
		chnlNode->SetAttribute("pan", theProject->mgr.GetPan(chnl));
		delete chnlNode;
	}
	chnlNode = midi->AddChild("sbfile");
	if (chnlNode)
	{
		chnlNode->SetContent(sbFile);
		delete chnlNode;
	}

	return FileList::Save(midi);
}

int MidiItem::Generate(InstrManager *mgr, Sequencer *seq)
{
	SMFFile smf;
	InstrConfig *inc = mgr->FindInstr("[midiplayer]");
	if (inc == NULL)
		return 0;

	GMPlayer *ins = (GMPlayer*)inc->instrTmplt;
	if (ins == NULL)
		return 0;

	SoundBank *sb = SoundBank::FindBank(sbFile);
	if (sb == NULL)
		sb = SoundBank::FindBankFile(sbFile);
	ins->SetSoundBank(sb);

	SMFInstrMap map[16];
	for (int ch = 0; ch < 16; ch++)
	{
		map[ch].inc = inc;
		map[ch].bnkParam = -1;
		map[ch].preParam = -1;
	}


	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_MIDIFILE)
		{
			MidiFile *mf = (MidiFile *)pi;
			if (mf->GetUse())
			{
				smf.Reset();
				bsString fullPath;
				theProject->FindOnPath(fullPath, mf->GetFile());
				if (smf.LoadFile(fullPath) == 0)
					err |= smf.GenerateSeq(seq, map, sbnk);
			}
		}
		pi = prjTree->NextSibling(pi);
	}
	return err;
}

WidgetForm *MidiItem::CreateForm(int xo, int yo)
{
	bsString path;
	theProject->FindForm(path, "MIDIControlEd.xml");

	MIDIControlEd *ed = new MIDIControlEd;
	ed->Load(path, xo, yo);
	return ed;
}

int MidiItem::EditItem()
{
	FormEditor *fe = prjFrame->CreateFormEditor(this);
	if (fe)
        return 1;
    return 0;
}
