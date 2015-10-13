//////////////////////////////////////////////////////////////////////
// BasicSynth - Widgets to display text.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "SynthWidget.h"
#include "TextWidget.h"
#include "WidgetForm.h"

TextWidget::TextWidget() : SynthWidget(wdgText)
{
	textHeight = 12;
	textBold = 0;
	textItalic = 0;
	filled = 0;
	shadow = 0;
	inset = 0;
	align = 1;
	editable = 0;
	fmt = "%6.3f";
}

TextWidget::~TextWidget()
{
}

void TextWidget::SetValue(float v)
{
	char txt[40];
	txt[0] = 0;
	snprintf(txt, 40, fmt, v);
	text = txt;
	if (buddy2)
		buddy2->SetValue(v);
}

float TextWidget::GetValue()
{
	return (float) text.ToFloat();
}

void TextWidget::SetText(const char *s)
{
	text = s;
	if (buddy2)
		buddy2->SetText(s);
}

const char *TextWidget::GetText()
{
	return text;
}

void TextWidget::SetFormat(const char *s)
{
	fmt = s;
}

void TextWidget::SetTextHeight(int h)
{
	textHeight = h;
}

void TextWidget::SetAlign(int al)
{
	align = al;
}

void TextWidget::SetBold(int b)
{
	textBold = b;
}

void TextWidget::SetItalic(int it)
{
	textItalic = it;
}

void TextWidget::SetShadow(int sh)
{
	shadow = sh;
}

void TextWidget::SetInset(int in)
{
	inset = in;
}

void TextWidget::SetFilled(int fi)
{
	filled = fi;
}

void TextWidget::SetEdit(int e)
{
	editable = e;
}

int TextWidget::BtnDown(int x, int y, int ctrl, int shift)
{
	return track = editable;
}

int TextWidget::MouseMove(int x, int y, int ctrl, int shift)
{
	return 0;
}

int TextWidget::BtnUp(int x, int y, int ctrl, int shift)
{ 
	if (track)
	{
		track = 0;
		if (enable && area.Inside(x, y))
		{
			char buf[80];
			if (buddy1)
			{
				//snprintf(buf, 80, "%f", buddy1->GetValue());
				bsString::FlpToStr(buddy1->GetValue(), buf, sizeof(buf));
			}
			else
				strncpy(buf, text, sizeof(buf));
			if (prjFrame->QueryValue("Enter value", buf, sizeof(buf)))
			{
				if (buddy1)
				{
					buddy1->SetText(buf);
					form->Redraw(buddy1);
					buddy1->ValueChanged(buddy1);
				}
				else
				{
					SetText(buf);
					form->Redraw(this);
					ValueChanged(this);
				}
			}
		}
	}
	return 1;
}

int TextWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	char *txt;
	if (elem->GetAttribute("lbl", &txt) == 0)
	{
		SetText(txt);
		delete txt;
	}
	if (elem->GetAttribute("fmt", &txt) == 0)
	{
		SetFormat(txt);
		delete txt;
	}
	short th;
	if (elem->GetAttribute("th", th) == 0)
		SetTextHeight(th);
	if (elem->GetAttribute("bold", th) == 0)
		SetBold(th);
	if (elem->GetAttribute("italic", th) == 0)
		SetItalic(th);
	if (elem->GetAttribute("filled", th) == 0)
		SetFilled(th);
	if (elem->GetAttribute("align", th) == 0)
		SetAlign(th);
	if (elem->GetAttribute("shadow", th) == 0)
		SetShadow(th);
	if (elem->GetAttribute("inset", th) == 0)
		SetInset(th);
	if (elem->GetAttribute("edit", th) == 0)
		editable = (int) th;
	return 0;
}

BoxWidget::BoxWidget() : SynthWidget(wdgUnk)
{
	filled = 0;
	style = 0;
	thick = 1;
	hiClr = 0xffffffff;
	loClr = 0xff000000;
}

BoxWidget::~BoxWidget()
{
}

void BoxWidget::SetStyle(int n)
{
	style = n;
}

int BoxWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	short w;
	if (elem->GetAttribute("wid", w) == 0)
		thick = w;
	short st;
	if (elem->GetAttribute("style", st) == 0)
		style = st;
	if (elem->GetAttribute("filled", st) == 0)
		filled = st;
	GetColorAttribute(elem, "hiclr", hiClr);
	GetColorAttribute(elem, "loclr", loClr);
	return 0;
}

GraphWidget::GraphWidget() : SynthWidget(wdgUnk)
{
	thick = 1;
	bar = 0;
	numVals = 0;
	vals = 0;
	range = 1.0;
	barWidth = 0.9;
	grClr = 0;
}

GraphWidget::~GraphWidget()
{
	delete[] vals;
}

void GraphWidget::SetNumVals(int n)
{
	float *newVals = new float[n];
	int i;
	for (i = 0; i < n && i < numVals; i++)
		newVals[i] = vals[i];
	numVals = n;
	if (vals)
		delete[] vals;
	vals = newVals;
	for (; i < numVals; i++)
		vals[i] = 0;
}

void GraphWidget::SetRange(float r)
{
	range = r;
}

void GraphWidget::SetVal(int n, float v)
{
	if (n < numVals)
		vals[n] = v;
}

int GraphWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	float w;
	short ival;
	if (elem->GetAttribute("wid", w) == 0)
		thick = w;
	if (elem->GetAttribute("bar", ival) == 0)
		bar = ival;
	if (elem->GetAttribute("bw", w) == 0)
		barWidth = w;
	if (elem->GetAttribute("num", ival) == 0)
		SetNumVals(ival);
	if (elem->GetAttribute("range", w) == 0)
		range = w;
	SynthWidget::GetColorAttribute(elem, "grclr", grClr);
	return 0;
}

/*
EditWidget::EditWidget() : SynthWidget(wdgText)
{
	changed = 0;
	prec = 4;
	txtbuf = 0;
}

EditWidget::~EditWidget()
{
	delete txtbuf;
	if (edWnd.IsWindow())
		edWnd.DestroyWindow();
}

void EditWidget::SetValue(float v)
{
	char txt[40];
	if (prec == 0)
		snprintf(txt, 40, "%d", (int) v);
	else
		snprintf(txt, 40, "%f", Round(v));
	edWnd.SetWindowText(txt);
	changed = 0;
}

float EditWidget::GetValue()
{
	char txt[40];
	edWnd.GetWindowText(txt, 40);
	changed = 0;
	return Round(atof(txt));
}

void EditWidget::SetText(const char *p)
{
	edWnd.SetWindowText(p);
	changed = 0;
}


const char *EditWidget::GetText()
{
	delete txtbuf;
	int len = edWnd.GetWindowTextLength();
	txtbuf = new char[len+1];
	edWnd.GetWindowText(txtbuf, len+1);
	changed = 0;
	return txtbuf;
}


float EditWidget::Round(float v)
{
	float scl;
	switch (prec)
	{
	case 0:
		return floor(v);
	case 1:
		scl = 10.0f;
		break;
	case 2:
		scl = 100.0f;
		break;
	case 3:
		scl = 1000.0f;
		break;
	default:
		scl = pow(10.0f, (float) prec);
		break;
	}
	return floor((v * scl)+0.5) / scl;
}

void EditWidget::Paint(Graphics *gr)
{
	if (edWnd.IsWindow())
		return;

	SolidBrush bk(bgClr);
	gr->FillRectangle(&bk, area);
}

int EditWidget::CommandMsg(short code)
{
	int rv = 0;
	if (code == EN_KILLFOCUS)
	{
		rv = changed;
		changed = 0;
	}
	else if (code == EN_CHANGE)
	{
		changed = 1;
	}
	return rv;
}

void EditWidget::WidgetWindow(void *w)
{
	RECT rc;
	rc.left = area.GetLeft();
	rc.top = area.GetTop();
	rc.right = area.GetRight();
	rc.bottom = area.GetBottom();
	edWnd.Create((HWND)w, rc, NULL, WS_CHILD|WS_BORDER|WS_VISIBLE|ES_AUTOHSCROLL, 0, id, 0);
}

int EditWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	short v;
	if (elem->GetAttribute("prec", v) == 0)
		prec = v;
	return 0;
}
*/

