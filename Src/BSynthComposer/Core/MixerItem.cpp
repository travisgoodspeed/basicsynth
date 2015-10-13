//////////////////////////////////////////////////////////////////////
// BasicSynth - Project items that represent a mixer or effects unit.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "MixerEdit.h"

void MixerItem::InitMixer()
{
//	int kbdon = theProject->Stop();
	if (mixChnl < 1)
		SetMixerInputs(1, 0);

	theProject->mix.SetChannels((int)mixChnl);
	theProject->mix.MasterVolume(mixVolLft, mixVolRgt);
	short chnl;
	for (chnl = 0; chnl < mixChnl; chnl++)
		inputs[chnl].InitMixer();

	theProject->mix.SetFxChannels((int)fxUnits);
	short fxn;
	for (fxn = 0; fxn < fxUnits; fxn++)
		effects[fxn]->InitMixer();
//	if (kbdon)
//		theProject->Start();
}

void MixerItem::ResetMixer()
{
	theProject->mix.Reset();
	int fxn;
	for (fxn = 0; fxn < fxUnits; fxn++)
		effects[fxn]->Reset();
}

void MixerItem::SetMasterVol(AmpValue lft, AmpValue rgt, int imm)
{
	mixVolLft = lft;
	mixVolRgt = rgt;
	if (imm)
		theProject->mix.MasterVolume(mixVolLft, mixVolRgt);
}

void MixerItem::SetChannelOn(int ndx, short on, int imm)
{
	if (ndx < mixChnl)
		inputs[ndx].SetOn(on, imm);
}

short MixerItem::GetChannelOn(int ndx)
{
	if (ndx < mixChnl)
		return inputs[ndx].GetOn();
	return 0;
}
void MixerItem::SetChannelVol(int ndx, AmpValue vol, int imm)
{
	if (ndx < mixChnl)
		inputs[ndx].SetVol(vol, imm);
}

AmpValue MixerItem::GetChannelVol(int ndx)
{
	if (ndx < mixChnl)
		return inputs[ndx].GetVol();
	return 0.0;
}

void MixerItem::SetChannelPan(int ndx, AmpValue pan, int imm)
{
	if (ndx < mixChnl)
		inputs[ndx].SetPan(pan, imm);
}

AmpValue MixerItem::GetChannelPan(int ndx)
{
	if (ndx < mixChnl)
		return inputs[ndx].GetPan();
	return 0.0;
}

void MixerItem::SetEffectsVol(int fx, AmpValue vol, int imm)
{
	if (fx < fxUnits)
		effects[fx]->SetVol(vol, imm);
}

AmpValue MixerItem::GetEffectsVol(int fx)
{
	if (fx < fxUnits)
		return effects[fx]->GetVol();
	return 0.0;
}

void MixerItem::SetEffectsPan(int fx, AmpValue pan, int imm)
{
	if (fx < fxUnits)
		effects[fx]->SetPan(pan, imm);
}

AmpValue MixerItem::GetEffectsPan(int fx)
{
	if (fx < fxUnits)
		return effects[fx]->GetPan();
	return 0.0;
}

void MixerItem::SetEffectsSend(int fx, int cn, AmpValue lvl, int imm)
{
	if (fx < fxUnits && cn < mixChnl)
		effects[fx]->SetSend(cn, lvl, imm);
}

AmpValue MixerItem::GetEffectsSend(int fx, int cn)
{
	if (fx < fxUnits && cn < mixChnl)
		return effects[fx]->GetSend(cn);
	return 0;
}

FxItem *MixerItem::GetEffectsItem(int fx)
{
	if (fx < fxUnits)
		return effects[fx];
	return 0;
}

void MixerItem::SetMixerInputs(int num, int keep)
{
	if (num == mixChnl && keep)
		return;

	int n;
	ChannelItem *newin = new ChannelItem[num];
	for (n = 0; n < num; n++)
	{
		if (keep && n < mixChnl)
		{
			newin[n].SetChnl(inputs[n].GetChnl());
			newin[n].SetVol(inputs[n].GetVol());
			newin[n].SetPan(inputs[n].GetPan());
			newin[n].SetOn(inputs[n].GetOn());
		}
		else
		{
			newin[n].SetChnl(n);
			newin[n].SetOn(1);
			newin[n].SetVol(0.5);
			newin[n].SetPan(0.0);
		}
	}
	delete[] inputs;
	inputs = newin;
	mixChnl = num;
	if (effects)
	{
		for (n = 0; n < fxUnits; n++)
		{
			if (effects[n])
				effects[n]->SetChannels(mixChnl);
		}
	}
}

void MixerItem::SetMixerEffects(int num, int keep)
{
	if (num == fxUnits && keep)
		return;

	FxItem **items = new FxItem*[num];
	int n;
	for (n = 0; n < num; n++)
	{
		items[n] = 0;
		if (n < fxUnits)
		{
			if (effects && effects[n])
			{
				if (keep)
					items[n] = effects[n];
				else if (effects[n])
					effects[n]->Release();
			}
		}
	}
	if (effects)
	{
		while (n < fxUnits)
		{
			if (effects[n])
				effects[n]->Release();
			n++;
		}
		delete effects;
	}
	effects = items;
	fxUnits = num;
}

FxItem *MixerItem::AddEffect(const char *type)
{
	FxItem *fx = 0;
	if (strcmp(type, "reverb") == 0)
		fx = new ReverbItem;
	else if (strcmp(type, "flanger") == 0)
		fx = new FlangerItem;
	else if (strcmp(type, "echo") == 0)
		fx = new EchoItem;
	else
		return 0;
	fx->SetParent(this);
	fx->SetChannels(mixChnl);
	fx->AddRef();
	return fx;
}

int MixerItem::Load(XmlSynthElem *node)
{
	if (node->GetAttribute("chnls", mixChnl) || mixChnl < 1)
		mixChnl = 1;
	node->GetAttribute("fxunits", fxUnits);
	node->GetAttribute("lft", mixVolLft);
	node->GetAttribute("rgt", mixVolRgt);
	if (node->GetAttribute("pan", mixPanType))
		mixPanType = panTrig;
	short cn;
	SetMixerInputs(mixChnl, 0);
	SetMixerEffects(fxUnits, 0);

	XmlSynthElem *mixElem = node->FirstChild();
	while (mixElem)
	{
		if (mixElem->TagMatch("chnl"))
		{
			cn = -1;
			mixElem->GetAttribute("cn", cn);
			if (cn >= 0 && cn < mixChnl)
				inputs[cn].Load(mixElem);
		}
		else
		{
			FxItem *fx = AddEffect(mixElem->TagName());
			if (fx != 0)
			{
				mixElem->GetAttribute("unit", cn);
				if (cn >= fxUnits)
				{
					fxUnits = cn+1;
					SetMixerEffects(fxUnits, 1);
				}
				effects[cn] = fx;
				fx->Load(mixElem);
				prjTree->AddNode(fx);
			}
		}
		XmlSynthElem *sib = mixElem->NextSibling();
		delete mixElem;
		mixElem = sib;
	}
	return 0;
}

int MixerItem::Save(XmlSynthElem *node)
{
	XmlSynthElem *mixnode = node->AddChild("mixer");
	XmlSynthElem *child;

	mixnode->SetAttribute("chnls", mixChnl);
	mixnode->SetAttribute("fxunits", fxUnits);
	mixnode->SetAttribute("lft", mixVolLft);
	mixnode->SetAttribute("rgt", mixVolRgt);
	mixnode->SetAttribute("pan", mixPanType);
	int cn;
	for (cn = 0; cn < mixChnl; cn++)
	{
		child = mixnode->AddChild("chnl");
		inputs[cn].Save(child);
		delete child;
	}

	FxItem *fx;
	int fn;
	for (fn = 0; fn < fxUnits; fn++)
	{
		fx = effects[fn];
		child = mixnode->AddChild(fx->GetFxType());
		fx->Save(child);
		delete child;
	}
	delete mixnode;
	return 0;
}

void MixerItem::EditorClosed()
{
	editor = 0;
	mixEdit = 0;
}

WidgetForm *MixerItem::CreateForm(int xo, int yo)
{
	if (!mixEdit)
	{
		mixEdit = new MixerEdit;
		mixEdit->Setup(xo, yo);
		editX = xo;
		editY = yo;
	}
	return mixEdit;
}

int MixerItem::EditItem()
{
	prjFrame->CreateFormEditor(this);
	return 1;
}

int MixerItem::ItemProperties()
{
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		pb->Activate(1);
		delete pb;
		prjFrame->MixerChanged();
	}

	return 1;
}

int MixerItem::ItemActions()
{
	int actEnable = actions;
	if (mixEdit)
		actEnable |= ITM_ENABLE_CLOSE|ITM_ENABLE_SAVE;
	return actEnable;
}

int MixerItem::SaveItem()
{
	if (mixEdit)
		mixEdit->SetParams();
	return 1; 
}

int MixerItem::CloseItem()
{
	if (mixEdit)
	{
		mixEdit->SetParams();
		prjFrame->CloseEditor(this);
	}
	return 1;
}

int MixerItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_MIX_CHNL, (long)mixChnl, 0);
	pb->SetState(PROP_MIX_PLIN, mixPanType == panLin);
	pb->SetState(PROP_MIX_PTRG, mixPanType == panTrig);
	pb->SetState(PROP_MIX_PSQR, mixPanType == panSqr);
	pb->ListChildren(PROP_MIX_FX, this);
	return 1;
}

int MixerItem::SaveProperties(PropertyBox *pb)
{
	long chnls = 0;
	short panl = 0;
	short pant = 0;
	short pans = 0;
	pb->GetValue(PROP_MIX_CHNL, chnls);
	pb->GetState(PROP_MIX_PLIN, panl);
	pb->GetState(PROP_MIX_PTRG, pant);
	pb->GetState(PROP_MIX_PSQR, pans);
	if (pans)
		mixPanType = panSqr;
	else if (pant)
		mixPanType = panTrig;
	else if (panl)
		mixPanType = panLin;
	else
		mixPanType = panOff;
	if (chnls < 1)
		chnls = 1;

	ProjectItem *pi;
	while ((pi = prjTree->FirstChild(this)) != NULL)
	{
		prjTree->RemoveNode(pi);
	}

	int oldUnits = fxUnits;
	int newUnits = 0;
	FxItem **oldFx = effects;
	FxItem **newFx = 0;

	pb->GetListCount(PROP_MIX_FX, newUnits);
	if (newUnits)
		newFx = new FxItem*[newUnits];

	int fxn;
	for (fxn = 0; fxn < newUnits; fxn++)
	{
		FxItem *fx = (FxItem*)pb->GetListItem(PROP_MIX_FX, fxn);
		fx->SetUnit(fxn);
		fx->SetParent(this);
		//fx->SetChannels(mixChnl); this will be done in SetChannels()
		prjTree->AddNode(fx);
		for (int n = 0; n < oldUnits; n++)
		{
			if (oldFx[n] == fx)
			{
				oldFx[n] = 0;
				break;
			}
		}
		newFx[fxn] = fx;
	}

	for (fxn = 0; fxn < oldUnits; fxn++)
	{
		if (oldFx[fxn])
			oldFx[fxn]->Release();
	}
	delete oldFx;

	fxUnits = newUnits;
	effects = newFx;
	SetMixerInputs(chnls, 1);

	theProject->mix.SetFxChannels(fxUnits);

	InitMixer();
	if (mixEdit)
	{
		mixEdit->Setup(editX, editY);
		mixEdit->Resize();
	}

	theProject->SetChange(1);

	return 1;
}

void ChannelItem::InitMixer()
{
	theProject->mix.ChannelOn(cn, on);
	theProject->mix.ChannelVolume(cn, vol);
	theProject->mix.ChannelPan(cn, panTrig, pan);
}

void ChannelItem::SetVol(AmpValue v, int imm)
{
	vol = v; 
	if (imm)
		theProject->mix.ChannelVolume(cn, v);
}

void ChannelItem::SetPan(AmpValue p, int imm)
{
	pan = p;
	if (imm)
		theProject->mix.ChannelPan(cn, panTrig, p);
}

void ChannelItem::SetOn(short o, int imm)
{
	on = o; 
	if (imm)
		theProject->mix.ChannelOn(cn, o);
}

int ChannelItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("cn", cn);
	node->GetAttribute("on", on);
	node->GetAttribute("vol", vol);
	node->GetAttribute("pan", pan);
	return 0;
}

int ChannelItem::Save(XmlSynthElem *node)
{
	node->SetAttribute("cn", cn);
	node->SetAttribute("on", on);
	node->SetAttribute("vol", vol);
	node->SetAttribute("pan", pan);
	return 0;
}

void FxItem::SetChannels(short n)
{
	if (n == nchnl)
		return;

	AmpValue *newVals = new AmpValue[n];
	for (int m = 0; m < n; m++)
	{
		if (m < nchnl)
			newVals[m] = send[m];
		else
			newVals[m] = 0.0;
	}
	nchnl = n;
	delete send;
	send = newVals;
}

void FxItem::InitMixer()
{
	theProject->mix.FxReceive(unit, vol);
	theProject->mix.FxPan(unit, panTrig, pan);
	int chnl;
	for (chnl = 0; chnl < nchnl; chnl++)
		theProject->mix.FxLevel(unit, chnl, send[chnl]);
}

void FxItem::SetVol(AmpValue val, int imm)
{
	vol = val; 
	if (imm)
		theProject->mix.FxReceive(unit, val);
}

void FxItem::SetPan(AmpValue val, int imm) 
{ 
	pan = val;
	if (imm)
		theProject->mix.FxPan(unit, panTrig, val);
}

void FxItem::SetSend(int ndx, AmpValue val, int imm)
{
	if (ndx < nchnl)
	{
		send[ndx] = val;
		if (imm)
			theProject->mix.FxLevel(unit, ndx, val);
	}
}

int FxItem::Load(XmlSynthElem *node)
{
	ProjectItem::Load(node);
	node->GetAttribute("unit", unit);
	node->GetAttribute("vol", vol);
	node->GetAttribute("pan", pan);
	if (name.Length() == 0)
	{
		//char nmbuf[80];
		//snprintf(nmbuf, 80, "%s #%d", (const char *)fxType, unit);
		//name = nmbuf;
		name = fxType;
		name += ' ';
		name += (long)unit;
	}

	if (send)
	{
		XmlSynthElem *child = node->FirstChild();
		while (child)
		{
			if (child->TagMatch("send"))
			{
				short cn = 0;
				if (child->GetAttribute("chnl", cn) == 0 && cn < nchnl)
					child->GetAttribute("amt", send[cn]);
			}
			XmlSynthElem *sib = child->NextSibling();
			delete child;
			child = sib;
		}
	}

	return 0;
}

int FxItem::Save(XmlSynthElem *node)
{
	ProjectItem::Save(node);
	node->SetAttribute("unit", unit);
	node->SetAttribute("vol", vol);
	node->SetAttribute("pan", pan);

	short cn;
	for (cn = 0; cn < nchnl; cn++)
	{
		XmlSynthElem *fxSend = node->AddChild("send");
		fxSend->SetAttribute("chnl", cn);
		fxSend->SetAttribute("amt", send[cn]);
		delete fxSend;
	}

	return 0;
}

int FxItem::ItemProperties()
{
	int ok = 0;
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		int play = theProject->Stop();
		ok = pb->Activate(1);
		delete pb;
		if (ok)
			InitMixer();
		if (play)
			theProject->Start();
	}
	return ok;
}

int FxItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_NAME, name, "Name");
	pb->SetValue(PROP_FX_VOL, vol, "Volume");
	pb->SetValue(PROP_FX_PAN, pan, "Pan");
	return 1;
}

int FxItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_NAME, name);
	pb->GetValue(PROP_FX_VOL, vol);
	pb->GetValue(PROP_FX_PAN, pan);
	return 1;
}

static float rvrbDefaults[] = { 0.0437, 0.0411, 0.0371, 0.0297 };

ReverbItem::ReverbItem() : FxItem(PRJNODE_REVERB)
{
	fxType = "reverb";
	rvt = 1.0;
	rvrb = new Reverb2;
	rvrb->InitReverb(0.1, 1.0);
	for (int n = 0; n < 4; n++)
		lt[n] = rvrbDefaults[n];
};

void ReverbItem::InitMixer()
{
	rvrb->InitDelay(0, lt[0], rvt);
	rvrb->InitDelay(1, lt[1], rvt);
	rvrb->InitDelay(2, lt[2], rvt);
	rvrb->InitDelay(3, lt[3], rvt);
	rvrb->InitDelay(4, 0.09683, 0.0050);
	rvrb->InitDelay(5, 0.03292, 0.0017);
	rvrb->Reset();
	rvrb->Clear();
	theProject->mix.FxInit(unit, rvrb, vol);
	theProject->mix.FxPan(unit, panTrig, pan);
	FxItem::InitMixer();
}

int ReverbItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("rvt", rvt);
	if (node->GetAttribute("lt1", lt[0]))
		lt[0] = rvrbDefaults[0];
	if (node->GetAttribute("lt2", lt[1]))
		lt[1] = rvrbDefaults[1];
	if (node->GetAttribute("lt3", lt[2]))
		lt[2] = rvrbDefaults[2];
	if (node->GetAttribute("lt4", lt[3]))
		lt[3] = rvrbDefaults[3];
	return FxItem::Load(node);
}

int ReverbItem::Save(XmlSynthElem *node)
{
	node->SetAttribute("rvt", rvt);
	node->SetAttribute("lt1", lt[0]);
	node->SetAttribute("lt2", lt[1]);
	node->SetAttribute("lt3", lt[2]);
	node->SetAttribute("lt4", lt[3]);
	return FxItem::Save(node);
}


int ReverbItem::LoadProperties(PropertyBox *pb)
{
	pb->SetCaption("Reverb Setup");
	pb->SetValue(PROP_FX_V1, rvt, "Reverb Time");
	pb->SetValue(PROP_FX_V2, lt[0], "Loop Time 1");
	pb->SetValue(PROP_FX_V3, lt[1], "Loop Time 2");
	pb->SetValue(PROP_FX_V4, lt[2], "Loop Time 3");
	pb->SetValue(PROP_FX_V5, lt[3], "Loop Time 4");
	FxItem::LoadProperties(pb);
	return 1;
}

int ReverbItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_FX_V1, rvt);
	pb->GetValue(PROP_FX_V2, lt[0]);
	pb->GetValue(PROP_FX_V3, lt[1]);
	pb->GetValue(PROP_FX_V4, lt[2]);
	pb->GetValue(PROP_FX_V5, lt[3]);
	FxItem::SaveProperties(pb);
	return 1;
}

void FlangerItem::InitMixer()
{
	flngr->InitFlanger(1.0, mix, fb, cntr, depth, sweep);
	theProject->mix.FxInit(unit, flngr, vol);
	theProject->mix.FxPan(unit, panTrig, pan);
	FxItem::InitMixer();
}

int FlangerItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("mix", mix);
	node->GetAttribute("fb", fb);
	node->GetAttribute("cntr", cntr);
	node->GetAttribute("depth", depth);
	node->GetAttribute("sweep", sweep);
	return FxItem::Load(node);
}

int FlangerItem::Save(XmlSynthElem *node)
{
	node->SetAttribute("mix", mix);
	node->SetAttribute("fb", fb);
	node->SetAttribute("cntr", cntr);
	node->SetAttribute("depth", depth);
	node->SetAttribute("sweep", sweep);
	return FxItem::Save(node);
}

int FlangerItem::LoadProperties(PropertyBox *pb)
{
	pb->SetCaption("Flanger Setup");
	pb->SetValue(PROP_FX_V1, mix, "Mix");
	pb->SetValue(PROP_FX_V2, fb, "Feedback");
	pb->SetValue(PROP_FX_V3, cntr, "Center delay");
	pb->SetValue(PROP_FX_V4, depth, "Depth");
	pb->SetValue(PROP_FX_V5, sweep, "Sweep freq.");
	FxItem::LoadProperties(pb);
	return 1;
}

int FlangerItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_FX_V1, mix);
	pb->GetValue(PROP_FX_V2, fb);
	pb->GetValue(PROP_FX_V3, cntr);
	pb->GetValue(PROP_FX_V4, depth);
	pb->GetValue(PROP_FX_V5, sweep);
	FxItem::SaveProperties(pb);
	return 1;
}

void EchoItem::InitMixer()
{
//	dlr->InitDL(dly, dec);
	dlr->InitDLR(dly, dec, 0.001, 1.0);
	theProject->mix.FxInit(unit, dlr, vol);
	theProject->mix.FxPan(unit, panTrig, pan);
	FxItem::InitMixer();
}

int EchoItem::Load(XmlSynthElem *node)
{
	node->GetAttribute("dly", dly);
	node->GetAttribute("dec", dec);
	return FxItem::Load(node);
}

int EchoItem::Save(XmlSynthElem *node)
{
	node->SetAttribute("dly", dly);
	node->SetAttribute("dec", dec);
	return FxItem::Save(node);
}

int EchoItem::LoadProperties(PropertyBox *pb)
{
	pb->SetCaption("Echo Setup");
	pb->SetValue(PROP_FX_V1, dly, "Delay");
	pb->SetValue(PROP_FX_V2, dec, "Decay");
	pb->SetValue(PROP_FX_V3, 0.f, "");
	pb->SetValue(PROP_FX_V4, 0.f, "");
	pb->SetValue(PROP_FX_V5, 0.f, "");
	pb->EnableValue(PROP_FX_V3, 0);
	pb->EnableValue(PROP_FX_V4, 0);
	pb->EnableValue(PROP_FX_V5, 0);
	FxItem::LoadProperties(pb);
	return 1;
}

int EchoItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_FX_V1, dly);
	pb->GetValue(PROP_FX_V2, dec);
	FxItem::SaveProperties(pb);
	return 1;
}
