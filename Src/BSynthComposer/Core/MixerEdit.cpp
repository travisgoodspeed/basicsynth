//////////////////////////////////////////////////////////////////////
// BasicSynth - Mixer and effects units editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "MixerEdit.h"

MixerEdit::MixerEdit()
{
	mixItem = 0;
	changed = 0;
	numChnl = 0;
	numFx = 0;
}

MixerEdit::~MixerEdit()
{
	if (mixItem)
		mixItem->EditorClosed();
}

int MixerEdit::Load(XmlSynthElem *node, int xo, int yo)
{
	return 0;
}

int MixerEdit::Load(const char *fileName, int xo, int yo)
{
	return 0;
}

void MixerEdit::GetArea(XmlSynthElem *node, wdgRect& rc)
{
	short c;
	node->GetAttribute("x", c);
	rc.x = c;
	node->GetAttribute("y", c);
	rc.y = c;
	node->GetAttribute("w", c);
	rc.w = c;
	node->GetAttribute("h", c);
	rc.h = c;
}

void MixerEdit::Setup(int xo, int yo)
{
	int masterH;
	int cn;
	int fn;
	//char lbl[40];
	bsString lbl;
	wdgRect rca;
	wdgRect rcin;
	wdgRect sndrc;
	XmlSynthElem *masterNode = 0;
	XmlSynthElem *labelsNode = 0;
	XmlSynthElem *inputNode = 0;
	XmlSynthElem *sendNode = 0;
	XmlSynthElem *effectNode = 0;
	SynthWidget *wdg;
	TextWidget *txt;
	WidgetGroup *grp;
	bsString fileName;

	track = 0;
	mainGroup->Clear();
	mainGroup->SetForm(this);
	numFx = 0;
	numChnl = 0;

	rca.x = 0;
	rca.y = 0;
	rca.w = 150;
	rca.h = 250; // actual w, h is set at the bottom
	mainGroup->SetArea(rca);
	SynthWidget::colorMap.Find("bg", bgColor);
	SynthWidget::colorMap.Find("fg", fgColor);
	mainGroup->SetColors(bgColor, fgColor);
	mainGroup->SetID(0);

	if (theProject == 0)
		return;

	mixItem = theProject->mixInfo;
	if (mixItem == 0)
		return;
	numChnl = mixItem->GetMixerInputs();
	numFx = mixItem->GetMixerEffects();

	theProject->FindForm(fileName, "MixerEd.xml");
	XmlSynthDoc doc;
	XmlSynthElem *rootNode = doc.Open((char*)(const char*)fileName);
	if (!rootNode)
	{
		doc.Close();
		return;
	}

	XmlSynthElem *child = rootNode->FirstChild();
	while (child)
	{
		XmlSynthElem *sib = child->NextSibling();

		char *name = 0;
		child->GetAttribute("name", &name);
		if (name)
		{
			if (strcmp(name, "master") == 0)
				masterNode = child;
			else if (strcmp(name, "labels") == 0)
				labelsNode = child;
			else if (strcmp(name, "input") == 0)
				inputNode = child;
			else if (strcmp(name, "send") == 0)
				sendNode = child;
			else if (strcmp(name, "effect") == 0)
				effectNode = child;
			else
				delete child;
			delete name;
		}
		else
			delete child;
		child = sib;
	}

	grp = new WidgetGroup;
	mainGroup->AddWidget(grp);
	GetArea(masterNode, rca);
	grp->SetArea(rca);
	grp->Load(masterNode);
	masterH = rca.h;

	grp = new WidgetGroup;
	mainGroup->AddWidget(grp);
	GetArea(labelsNode, rca);
	GetArea(sendNode, sndrc);
	rca.h += (sndrc.h + 5) * numFx;
	if (rca.h < masterH)
		rca.h = masterH;
	grp->SetArea(rca);
	grp->Load(labelsNode);
	wdg = grp->FindID(12);
	rcin = wdg->GetArea();
	rcin.h = rca.h;
	wdg->SetArea(rcin);

	txt = (TextWidget*) grp->FindID(11);
	wdgColor lblFg;
	wdgColor lblBg;
	txt->GetColors(lblBg, lblFg);
	int th = txt->GetTextHeight();

	wdg = grp->FindID(11);
	rcin = wdg->GetArea();
	rcin.y += sndrc.h + 5;
	for (fn = 0; fn < numFx; fn++)
	{
		//snprintf(lbl, 40, "Fx %d", fn);
		lbl = "Fx ";
		lbl += (long)fn;
		txt = new TextWidget;
		grp->AddWidget(txt);
		txt->SetArea(rcin);
		txt->SetText(lbl);
		txt->SetTextHeight(th);
		txt->SetID(-1);
		txt->SetColors(lblBg, lblFg);
		rcin.y += sndrc.h + 5;
	}

	GetArea(inputNode, rca);
	int inx = rca.x;
	int iny = rca.y;
	int inID = 20;
	for (cn = 0; cn < numChnl; cn++)
	{
		grp = new WidgetGroup;
		mainGroup->AddWidget(grp);
		rcin.x = inx;
		rcin.y = rca.y;
		rcin.w = rca.w;
		rcin.h = rca.h;
		grp->SetArea(rcin);
		grp->Load(inputNode);
		//snprintf(lbl, 40, "%d", cn);
		lbl = (long)cn;
		wdg = grp->FindID(21);
		wdg->SetText(lbl);
		wdg->SetID(inID+1);
		wdg = grp->FindID(22);
		wdg->SetID(inID+2);
		wdg = grp->FindID(23);
		wdg->SetID(inID+3);
		wdg = grp->FindID(24);
		wdg->SetID(inID+4);
		wdg = grp->FindID(25);
		wdg->SetID(inID+5);

		rcin = sndrc;
		iny = sndrc.y;
		for (fn = 0; fn < numFx; fn++)
		{
			grp = new WidgetGroup;
			mainGroup->AddWidget(grp);
			rcin.x = inx;
			rcin.y = iny;
			grp->SetArea(rcin);
			grp->Load(sendNode);
			wdg = grp->FindID(26);
			wdg->SetID(inID+6+(fn*2));
			wdg = grp->FindID(27);
			wdg->SetID(inID+7+(fn*2));
			iny += sndrc.h + 5;
		}
		inx += rca.w + 5;
		inID += 20;
	}
	//inx += 10;
	//wdg = new BoxWidget;
	//mainGroup->AddWidget(wdg);
	inx += 10;
	GetArea(effectNode, rcin);
	for (fn = 0; fn < numFx; fn++)
	{
		grp = new WidgetGroup;
		mainGroup->AddWidget(grp);
		rcin.x = inx;
		grp->SetArea(rcin);
		grp->Load(effectNode);
		//snprintf(lbl, 40, "Fx %d", fn);
		lbl = "Fx ";
		lbl += (long)fn;
		wdg = grp->FindID(21);
		wdg->SetID(inID+1);
		wdg->SetText(lbl);
		wdg = grp->FindID(22);
		wdg->SetID(inID+2);
		wdg = grp->FindID(23);
		wdg->SetID(inID+3);
		wdg = grp->FindID(24);
		wdg->SetID(inID+4);
		wdg = grp->FindID(25);
		wdg->SetID(inID+5);
		wdg = grp->FindID(26);
		wdg->SetID(inID+6);
		inx += rcin.w + 5;
		inID += 20;
	}

	rca.x = 0;
	rca.y = 0;
	rca.w = inx + 10;
	if (iny < masterH)
		iny = masterH;
	rca.h = iny + 10;
	mainGroup->SetArea(rca);

	delete masterNode;
	delete labelsNode;
	delete inputNode;
	delete sendNode;
	delete effectNode;
	delete rootNode;
	doc.Close();

	// This is for toolkits like FLTK that don't use real windows.
	if (xo != 0 || yo != 0)
		MoveTo(xo, yo);
	GetParams();
}

void MixerEdit::GetParams()
{
	if (mixItem == 0)
		return;

	AmpValue lft;
	AmpValue rgt;
	mixItem->GetMasterVol(lft, rgt);
	SetWidgetValue(4, (lft+rgt)/2.0);

	int n;
	int id = 20;
	for (n = 0; n < numChnl; n++)
	{
		SetWidgetState(id+1, mixItem->GetChannelOn(n));
		SetWidgetValue(id+2, mixItem->GetChannelVol(n));
		SetWidgetValue(id+4, mixItem->GetChannelPan(n));
		id += 20;
	}

	int fn;
	for (fn = 0; fn < numFx; fn++)
	{
		FxItem *fx = mixItem->GetEffectsItem(fn);
		if (!fx)
			continue;
		id = (numChnl + fn + 1) * 20;
		SetWidgetValue(id+1, fn);
		SetWidgetValue(id+2, fx->GetVol());
		SetWidgetValue(id+4, fx->GetPan());
		SetWidgetText(id+6, fx->GetName());
		id = 20;
		for (n = 0; n < numChnl; n++)
		{
			id = ((n + 1) * 20) + 6 + (fn * 2);
			SetWidgetValue(id, fx->GetSend(n));
		}
	}
	changed = 0;
}

void MixerEdit::SetParams()
{
	if (mixItem == 0)
		return;

	float vol = GetWidgetValue(4);
	mixItem->SetMasterVol(vol, vol);

	int n;
	int id = 20;
	for (n = 0; n < numChnl; n++)
	{
		mixItem->SetChannelOn(n, GetWidgetState(id+1));
		mixItem->SetChannelVol(n, GetWidgetValue(id+2));
		mixItem->SetChannelPan(n, GetWidgetValue(id+4));
		id += 20;
	}

	for (int fxn = 0; fxn < numFx; fxn++)
	{
		FxItem *fx = mixItem->GetEffectsItem(fxn);
		if (!fx)
			continue;
		id = (numChnl + fxn + 1) * 20;
		fx->SetVol(GetWidgetValue(id+2));
		fx->SetPan(GetWidgetValue(id+4));
		id = 20;
		for (n = 0; n < numChnl; n++)
		{
			id = ((n + 1) * 20) + 6 + (fxn * 2);
			fx->SetSend(n, GetWidgetValue(id));
		}
	}

	changed = 0;
}

void MixerEdit::ValueChanged(SynthWidget *wdg)
{
	theProject->SetChange(1);
	int id = wdg->GetID();
	if (id == 4)
	{
		float val = wdg->GetValue();
		mixItem->SetMasterVol(val, val, 1);
		return;
	}
	if (id == 6)
	{
		SetParams();
		int kbdon = prjFrame->StopPlayer();
		mixItem->InitMixer();
		if (kbdon)
			prjFrame->StartPlayer();
		return;
	}
	if (id == 7)
	{
		GetParams();
		return;
	}
	if (id == 8)
	{
		int kbdon = prjFrame->StopPlayer();
		mixItem->ResetMixer();
		if (kbdon)
			prjFrame->StartPlayer();
		return;
	}

	int fxn;
	int chn = (id / 20) - 1;
	int ndx = id % 20;
	//ATLTRACE("Mixer Change: chn=%d, ndx=%d\n", chn, ndx);
	if (chn < numChnl)
	{
		switch (ndx)
		{
		case 1:
			mixItem->SetChannelOn(chn, wdg->GetState(), 1);
			break;
		case 2:
			mixItem->SetChannelVol(chn, wdg->GetValue(), 1);
			break;
		case 4:
			mixItem->SetChannelPan(chn, wdg->GetValue(), 1);
			break;
		default:
			fxn = (ndx - 6) / 2;
			mixItem->SetEffectsSend(fxn, chn, wdg->GetValue(), 1);
			break;
		}
	}
	else if ((fxn = chn - numChnl) < numFx)
	{
		FxItem *fx = mixItem->GetEffectsItem(fxn);
		switch (ndx)
		{
		case 1:
			if (fx->ItemProperties())
			{
				SetWidgetValue(id+1, fx->GetVol());
				SetWidgetValue(id+3, fx->GetPan());
				SetWidgetText(id+5, fx->GetName());
				Redraw(mainGroup->FindID(id-1));
			}
			break;
		case 2:
			fx->SetVol(wdg->GetValue(), 1);
			break;
		case 4:
			fx->SetPan(wdg->GetValue(), 1);
			break;
		}
	}
}
