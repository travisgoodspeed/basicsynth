//////////////////////////////////////////////////////////////////////
// BasicSynth - Widgets that act as switches.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "SynthWidget.h"
#include "SwitchWidget.h"
#include "WidgetForm.h"

WidgetImageCache *switchCache;

int SwitchData::Compare(ImageData *id)
{
	if (ImageData::Compare(id))
	{
		SwitchData *sd = (SwitchData *)id;
		return fgClr == sd->fgClr && bgClr == sd->bgClr;
	}
	return 0;
}

int LampData::Compare(ImageData *id)
{
	if (SwitchData::Compare(id))
	{
		LampData *sd = (LampData *)id;
		return loClr == sd->loClr && hiClr == sd->hiClr && style == sd->style;
	}
	return 0;
}

wdgColor SwitchWidget::swuphi = 0;
wdgColor SwitchWidget::swuplo = 0;
wdgColor SwitchWidget::swdnhi = 0;
wdgColor SwitchWidget::swdnlo = 0;

SwitchWidget::SwitchWidget() : SynthWidget(wdgSwitch)
{
	sip = 0;
	value = 0;
	track = 0;
	swOn = 0;
	act = 1;
	//lbl = 0;
	lblHeight = 12;
	bold = 0;
	italic = 0;
	shadow = 0;
	align = 1; // center text
	if (swuphi == 0)
	{
		if (!SynthWidget::colorMap.Find("swuphi", swuphi))
			swuphi = 0xffb4b4b4;
		if (!SynthWidget::colorMap.Find("swuplo", swuplo))
			swuplo = 0xff5a5a55;
		if (!SynthWidget::colorMap.Find("swdnhi", swdnhi))
			swdnhi = 0xff808078;
		if (!SynthWidget::colorMap.Find("swdnlo", swdnlo))
			swdnlo = 0xff424242;
	}
}

SwitchWidget::~SwitchWidget()
{
	if (sip)
		sip->Release();
}

void SwitchWidget::SetArea(wdgRect& r)
{
	SynthWidget::SetArea(r);
	lblRect = area;
	lblRect.Shrink(1, 1);
	if (sip)
	{
		sip->Release();
		sip = 0;
	}
}

void SwitchWidget::SetText(const char *s)
{
	lbl = s;
}

void SwitchWidget::SetToggle(int t)
{
	act = t;
	if (t)
		type = wdgSwitch;
	else
		type = wdgCmd;
}


void SwitchWidget::SetState(int n)
{
	swOn = n;
}

int SwitchWidget::GetState()
{
	return swOn;
}

float SwitchWidget::GetValue() 
{ 
	return value;
}

void SwitchWidget::SetValue(float v)
{
	value = v;
}

int SwitchWidget::GetSwitchImage(SwitchImageType t)
{
	SwitchData* sd = new SwitchData;
	sd->type = t;
	sd->cx = area.w;
	sd->cy = area.h;
	sd->fgClr = fgClr;
	sd->bgClr = bgClr;
	return GetSwitchImage(sd);
}

int SwitchWidget::GetSwitchImage(SwitchData *sd)
{
	if (sip != NULL)
	{
		if (sip->Match(sd))
		{
			delete sd;
			return 1;
		}
		sip->Release();
		sip = 0;
	}

	sip = switchCache->FindImage(sd);
	if (sip != NULL)
	{
		delete sd;
		sip->AddRef();
		return 1;
	}
	sip = switchCache->AddImage(sd);
	return 0; // blank image
}

int SwitchWidget::BtnDown(int x, int y, int ctrl, int shift)
{
	track = 1;
	if (act == 1)
		swOn = !swOn;
	else
		swOn = 1;
	form->Redraw(this);
	return 1; 
}

int SwitchWidget::MouseMove(int x, int y, int ctrl, int shift)
{
	if (track && type == wdgCmd)
	{
		int on = area.Inside(x, y);
		if (on != swOn)
		{
			swOn = on;
			form->Redraw(this);
			return 1;
		}
	}
	return 0;
}

int SwitchWidget::BtnUp(int x, int y, int ctrl, int shift)
{ 
	if (track)
	{
		track = 0;
		if (type == wdgCmd)
		{
			if (swOn)
			{
				swOn = 0;
				form->Redraw(this);
				if (area.Inside(x, y))
					ValueChanged(this);
			}
		}
		else
			ValueChanged(this);
	}
	return 1;
}

int SwitchWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	char *txt;
	if (elem->GetAttribute("lbl", &txt) == 0)
		lbl.Attach(txt);
	float v;
	if (elem->GetAttribute("val", v) == 0)
		value = v;
	short th;
	if (elem->GetAttribute("th", th) == 0 && th)
		lblHeight = th;
	if (elem->GetAttribute("bold", th) == 0 && th)
		bold = 1;
	if (elem->GetAttribute("italic", th) == 0 && th)
		italic = 1;
	if (elem->GetAttribute("shadow", th) == 0)
		shadow = th;

	return 0;
}

SlideSwitchWidget::SlideSwitchWidget()
{
	style = 0; // horizontal
}

SlideSwitchWidget::~SlideSwitchWidget()
{
}

int SlideSwitchWidget::Load(XmlSynthElem *elem)
{
	int r = SwitchWidget::Load(elem);
	short sty;
	if (elem->GetAttribute("style", sty) == 0)
		SetStyle(sty);
	return r;
}

ImageWidget::ImageWidget()
{
	onImg = 0;
	offImg = 0;
	sipImg = 0;
}

ImageWidget::~ImageWidget()
{
	delete onImg;
	delete offImg;
	if (sipImg)
		sipImg->Release();
}

int ImageWidget::Load(XmlSynthElem *elem)
{
	SwitchWidget::Load(elem);
	elem->GetAttribute("on", &onImg);
	elem->GetAttribute("off", &offImg);

	return 0;
}

//////////////////////////////////////////////////////////////
LampWidget::LampWidget()
{
	align = 0;
	style = 0;
	hiColor = 0xfff0f0f0;
	loColor = 0xff808080;
}

LampWidget::~LampWidget()
{
}

void LampWidget::SetArea(wdgRect& r)
{
	SwitchWidget::SetArea(r);
	lblRect = area;
	if (!(style & 2))
	{
		int ring;
		if (area.w > area.h)
			ring = area.h;
		else
			ring = area.w;
		lblRect.x = area.x + ring + 1;
		lblRect.w = area.w - ring - 1;
	}
}

int LampWidget::GetSwitchImage(SwitchImageType t)
{
	LampData* sd = new LampData;
	sd->type = t;
	sd->cx = area.w;
	sd->cy = area.h;
	sd->fgClr = fgClr;
	sd->bgClr = bgClr;
	sd->loClr = loColor;
	sd->hiClr = hiColor;
	sd->style = style;
	return SwitchWidget::GetSwitchImage((SwitchData*)sd);
}

int LampWidget::Load(XmlSynthElem *elem)
{
	SwitchWidget::Load(elem);
	SynthWidget::GetColorAttribute(elem, "hi", hiColor);
	SynthWidget::GetColorAttribute(elem, "lo", loColor);

	short v;
	if (elem->GetAttribute("style", v) == 0)
		style = v;

	return 0;
}

//////////////////////////////////////////////////////////////

SwitchGroup::SwitchGroup()
{
	type = wdgGroup;
	track = 0;
	defID = -1;
}

SwitchGroup::~SwitchGroup()
{
}

SynthWidget *SwitchGroup::AddWidget(const char *type, short x, short y, short w, short h)
{
	SynthWidget *wdg = WidgetGroup::AddWidget(type, x, y, w, h);
	if (wdg && wdg->GetType() == wdgSwitch)
		((SwitchWidget*)wdg)->SetToggle(2); // don't toggle
	return wdg;
}

int SwitchGroup::EnumEnable(SynthWidget *wdg, void *id)
{
	wdg->SetEnable((int)(long)id);
	return 1;
}

void SwitchGroup::SetEnable(int n)
{
	EnumList(EnumEnable, (void*)n);
}

float SwitchGroup::GetValue()
{ 
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if (wdg->GetState())
			return wdg->GetValue();
		wdg = wdg->next;
	}
	return 0; 
}

void SwitchGroup::SetValue(float v)
{
	int found = 0;
	SynthWidget *def = 0;
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if (wdg->GetID() == defID)
			def = wdg;
		if (!found && wdg->GetValue() == v)
		{
			found = 1;
			wdg->SetState(1);
		}
		else
			wdg->SetState(0);
		wdg = wdg->next;
	}
	if (!found && def)
	{
		def->SetValue(v);
		def->SetState(1);
	}
}

int SwitchGroup::Load(XmlSynthElem *elem)
{
	int r = WidgetGroup::Load(elem);
	float v = 0;
	if (elem->GetAttribute("value", v) == 0)
		SetValue(v);
	short d = 0;
	if (elem->GetAttribute("def", d) == 0)
		defID = d;

	return r;
}

SynthWidget *SwitchGroup::HitTest(int x, int y)
{
	track = WidgetGroup::HitTest(x, y);
	if (track)
	{
		if (track->GetEnable())
			return this;
		track = 0;
	}
	return 0;
}

int SwitchGroup::BtnDown(int x, int y, int ctrl, int shift)
{
	int d = 0;
	if (track)
	{
		d = track->BtnDown(x, y, ctrl, shift);
	}

	return d;
}

int SwitchGroup::BtnUp(int x, int y, int ctrl, int shift)
{
	int d = 0;
	if (track)
	{
		d = track->BtnUp(x, y, ctrl, shift);
		track = 0;
	}
	return d;
}

int SwitchGroup::MouseMove(int x, int y, int ctrl, int shift)
{
	int d = 0;
	if (track)
		d = track->MouseMove(x, y, ctrl, shift);
	return d;
}

int SwitchGroup::Tracking()
{ 
	if (track)
		return track->Tracking();
	return 0; 
}

void SwitchGroup::ValueChanged(SynthWidget *wdg)
{
	SynthWidget *wdg2 = wdgHead.next;
	while (wdg2 != &wdgTail)
	{
		if (wdg2 != wdg && wdg2->GetState())
		{
			wdg2->SetState(0);
			form->Redraw(wdg2);
		}
		wdg2 = wdg2->next;
	}
	SynthWidget::ValueChanged(this);
}

/////////////////////////////////////////////////////////////////

SwitchSet::SwitchSet() 
{
}

SwitchSet::~SwitchSet()
{
}

int SwitchSet::EnumEnable(SynthWidget *wdg, void *id)
{
// stoopid G++
	wdg->SetEnable((int)(long)id);
	return 0;
}

void SwitchSet::SetEnable(int n)
{
	EnumList(EnumEnable, (void*)n);
}

float SwitchSet::GetValue()
{
	float val = 0;
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if (wdg->GetState())
			val += wdg->GetValue();
		wdg = wdg->next;
	}
	return val;
}

void SwitchSet::SetValue(float v)
{
	int bits = (int) v;
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if ((int) wdg->GetValue() & bits)
			wdg->SetState(1);
		else
			wdg->SetState(0);
		wdg = wdg->next;
	}
}

void SwitchSet::ValueChanged(SynthWidget *wdg)
{
	SynthWidget::ValueChanged(wdg);
	SynthWidget::ValueChanged(this);
}
