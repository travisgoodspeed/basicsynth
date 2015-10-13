//////////////////////////////////////////////////////////////////////
// BasicSynth - Base class for widget form editors.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

WidgetForm::WidgetForm()
{
	edwin = 0;
	track = 0;
	focus = 0;
	mainGroup = new WidgetGroup;
	mainGroup->SetForm(this);
	bgColor = 0xff808080;
	fgColor = 0xff000000;
	formFont = prjOptions.formsFont;
}

WidgetForm::~WidgetForm()
{
	delete mainGroup;
}

int WidgetForm::CanCopy()
{
	if (focus)
		return focus->GetType() == wdgText || focus->GetType() == wdgValue;
	return 0;
}

int WidgetForm::CanPaste()
{
	if (focus)
		return focus->GetType() == wdgText || focus->GetType() == wdgValue;
	return 0;
}

void WidgetForm::PasteText(const char *text)
{
	if (focus)
		focus->SetText(text);
}

void WidgetForm::CopyText(bsString& text)
{
	if (focus)
	{
		if (focus->GetType() == wdgText)
			text = focus->GetText();
		else if (focus->GetType() == wdgValue)
		{
			//char valbuf[40];
			//snprintf(valbuf, 40, "%f", focus->GetValue());
			//text = valbuf;
			text = (double) focus->GetValue();
		}
	}
}

void WidgetForm::BtnUp(int x, int y, int shft, int ctl)
{
	if (track && track->Tracking())
	{
		if (edwin)
			edwin->Release();
		track->BtnUp(x, y, shft, ctl);
		track = 0;
	}
}

void WidgetForm::MouseMove(int x, int y, int shft, int ctl)
{
	if (track && track->Tracking())
		track->MouseMove(x, y, shft, ctl);
}

void WidgetForm::BtnDn(int x, int y, int shft, int ctl)
{
	SynthWidget *wdg = mainGroup->HitTest(x, y);
/*	if (focus != wdg)
	{
		SynthWidget *tmp = focus;
		if (wdg && wdg->SetFocus())
		{
			focus = wdg;
		}
		else
			focus = 0;
		if (tmp)
		{
			if (tmp->LoseFocus())
				ValueChanged(tmp);
			else
				Redraw(tmp);
		}
	}*/

	if (wdg)
	{
		if (wdg->GetEnable())
		{
			track = wdg;
			track->BtnDown(x, y, shft, ctl);
			if (track->Tracking() && edwin)
				edwin->Capture();
		}
		else
			track = 0;
	}
	else
		track = 0;
}

int WidgetForm::Load(const char *fileName, int xo, int yo)
{
	XmlSynthDoc doc;
	XmlSynthElem root(&doc);
	if (doc.Open((char*)fileName, &root) == 0)
	{
#ifndef _WIN32
		fprintf(stderr, "WidgetForm: failed to load %s\n", fileName);
#endif
		return -1;
	}

	int r = Load(&root, xo, yo);
	root.Clear();
	doc.Close();
	return r;
}

int WidgetForm::Load(XmlSynthElem *root, int xo, int yo)
{
	SynthWidget::GetColorAttribute(root, "fg", fgColor);
	SynthWidget::GetColorAttribute(root, "bg", bgColor);

	mainGroup->SetForm(this);
	mainGroup->SetColors(bgColor, fgColor);
	long val;
	wdgRect rc(0,0,0,0);
	if (root->GetAttribute("x", val) == 0)
		rc.x = val;
	if (root->GetAttribute("y", val) == 0)
		rc.y = val;
	if (root->GetAttribute("w", val) == 0)
		rc.w = val;
	if (root->GetAttribute("h", val) == 0)
		rc.h = val;
	char *data = 0;
	if (root->GetAttribute("font", &data) == 0)
		formFont.Attach(data);
	rc.x += xo;
	rc.y += yo;
	mainGroup->SetArea(rc);
	return mainGroup->Load(root);
}

void WidgetForm::Command(int id, int cmd)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
	{
		if (wdg->CommandMsg(cmd))
			ValueChanged(wdg);
	}
}

void WidgetForm::ValueChanged(SynthWidget *wdg)
{
}

void WidgetForm::Redraw(SynthWidget *wdg)
{
	if (edwin)
		edwin->Redraw(wdg);
/*	if (wdg)
	{
		DrawContext dc = edwin->GetDrawContext();
		wdg->Paint(dc);
		SynthWidget *bud = wdg->GetBuddy2();
		if (bud)
			bud->Paint(dc);
		edwin->ReleaseDrawContext(dc);
	}
	else
		edwin->Redraw();
*/
}

void WidgetForm::RedrawForm(DrawContext dc)
{
	mainGroup->Paint(dc);
}

void WidgetForm::Resize()
{
	if (edwin)
		edwin->Resize();
}

void WidgetForm::GetSize(int& cx, int& cy)
{
	if (mainGroup)
	{
		wdgRect a = mainGroup->GetArea();
		if (a.w == 0 || a.h == 0)
		{
			mainGroup->GetSize(cx, cy);
			a.w = cx;
			a.h = cy;
			mainGroup->SetArea(a);
		}
		else
		{
			cx = a.w;
			cy = a.h;
		}
	}
}

float WidgetForm::GetWidgetValue(int id, float def)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
		return wdg->GetValue();
	return def;
}

void WidgetForm::SetWidgetValue(int id, float val)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
	{
		wdg->SetValue(val);
		SynthWidget *bud = wdg->GetBuddy2();
		if (bud)
			bud->SetValue(val);
	}
}

int WidgetForm::GetWidgetState(int id, int def)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
		return wdg->GetState();
	return def;
}

void WidgetForm::SetWidgetState(int id, int st)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
		wdg->SetState(st);
}

void WidgetForm::SetWidgetText(int id, const char *txt)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
		wdg->SetText(txt);
}

int WidgetForm::GetWidgetText(int id, char *txt, size_t len)
{
	SynthWidget *wdg = mainGroup->FindID(id);
	if (wdg)
	{
		const char *p = wdg->GetText();
		if (p)
		{
			strncpy(txt, p, len);
			return 1;
		}
	}
	return 0;
}

void WidgetForm::SetFocus()
{
}

void WidgetForm::KillFocus()
{
}

int WidgetForm::EnumUp(WGCallback cb, void *arg)
{
	return mainGroup->EnumUp(cb, arg);
}

int WidgetForm::EnumDn(WGCallback cb, void *arg)
{
	return mainGroup->EnumDn(cb, arg);
}

void WidgetForm::MoveTo(int x, int y)
{
	wdgRect a = mainGroup->GetArea();
	a.x += x - a.x;
	a.y += y - a.y;
	mainGroup->SetArea(a);
}

const char *WidgetForm::LabelFont()
{
	return formFont;
}

const char *WidgetForm::FormsDir()
{
	return prjOptions.formsDir;
}
