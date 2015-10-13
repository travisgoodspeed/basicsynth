//////////////////////////////////////////////////////////////////////
// BasicSynth - Widgets that display a knob
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "SynthWidget.h"
#include "KnobWidget.h"
#include "WidgetForm.h"

WidgetImageCache *knobCache;

KnobWidget::KnobWidget() : SynthWidget(wdgValue)
{
	style = 0;
	value = 0.0;
	minval = 0.0;
	maxval = 1.0f;
	scale = 1.0f;
	warp[0] = 0.01f;
	warp[1] = 0.1f;
	warp[2] = 0.001f;
	warp[3] = 0.1f;
	prec = 3;
	logScale = 0;
	range = maxval - minval;
	moving = 0;
	faceAmt[0] = 0.0f;
	faceAmt[1] = 1.0f;
	faceAmt[2] = 0.0f;
	facePos[0] = 0.0f;
	facePos[1] = 0.5f;
	facePos[2] = 1.0f;
	tickMaj = 0;
	tickMin = 0;
	tickLen = 0;
	knbImg = 0;
}

KnobWidget::~KnobWidget()
{
	if (knbImg)
		knbImg->Release();
}

void KnobWidget::SetArea(wdgRect& r)
{
	SynthWidget::SetArea(r);
	CalcCenter();
	CalcIndicator();
	if (knbImg)
	{
		knbImg->Release();
		knbImg = 0;
	}
}

void KnobWidget::SetValue(float v)
{
	if (logScale)
	{
		if (v > 0)
			value = scale * log10(v);
		else
			value = 0;
		if (buddy2)
			buddy2->SetValue(v);
	}
	else
	{
		value = Round(v / scale);
		if (buddy2)
			buddy2->SetValue(value);
	}
	CalcIndicator();
}

float KnobWidget::GetValue()
{
	if (logScale)
		return pow(10.0f, value / scale);
	return scale * Round(value);
}

int KnobWidget::Tracking()
{
	return moving;
}

int KnobWidget::SetFocus()
{
	focus = 1;
	return 1;
}

int KnobWidget::LoseFocus()
{
	focus = 0;
	return 1;
}

// Note: using floor(v+0.5) creates problems, 
// especially with prec=0, so we actually "truncate"
float KnobWidget::Round(float v)
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
	return floor(v * scl) / scl;
}

void KnobWidget::SetScale(float s)
{
	scale = s;
}

void KnobWidget::SetRange(float lo, float hi, int s)
{
	minval = lo;
	maxval = hi;
	prec = s; // digits to the right of the decimal point
	range = maxval - minval;
	// set "warp" 
	// [0] = normal, [1] = shift, [2] = ctrl, [3] = shift+ctrl
	if (logScale)
	{
		warp[0] = 1.0;
		warp[1] = 2.0;
		warp[2] = 0.5;
		warp[3] = 0.25;
	}
	else if (prec == 0)
	{
		float div = (float) (area.w+area.h);
		if (div < 1)
			div = 1;
		warp[0] = range / div;
		warp[2] = warp[0] / 2.0;
		warp[1] = warp[0] * 2.0;
		warp[3] = warp[0] * 4.0;
	}
	else
	{
		int digits = prec + (int) log10(range);
		int fact = -prec;
		warp[2] = pow(10.0f, (float)fact); // Ctrl - move smallest digit
		if (++fact < digits)
			warp[0] = warp[2] * 10.0;
		else
			warp[0] = warp[2];
		if (++fact < digits)
			warp[1] = warp[0] * 10.0;
		else
			warp[1] = warp[0];
		if (++fact < digits)
			warp[3] = warp[1] * 10.0;
		else
			warp[3] = warp[1];
	}
	CalcIndicator();
}

void KnobWidget::SetFace(wdgColor& fc, wdgColor& hc, float ang, int sty)
{
	faceAng = ang;
	faceClr = fc;
	highClr = hc;
	style = sty;
	switch (sty)
	{
	case 0:
		// center highlight
		faceAmt[0] = 0.2f;
		faceAmt[1] = 1.0f;
		faceAmt[2] = 0.0f;
		facePos[0] = 0.0f;
		facePos[1] = 0.5f;
		facePos[2] = 1.0f;
		break;
	case 1:
		// bump highlight
		faceAmt[0] = 1.0f;
		faceAmt[1] = 0.1f;
		faceAmt[2] = 0.0f;
		facePos[0] = 0.0f;
		facePos[1] = 0.4f;
		facePos[2] = 1.0f;
		break;
	case 2:
		// dimple highlight
		faceAmt[0] = 0.0f;
		faceAmt[1] = 0.15f;
		faceAmt[2] = 1.0f;
		facePos[0] = 0.0f;
		facePos[1] = 0.4f;
		facePos[2] = 1.0f;
		break;
	}
}

void KnobWidget::SetTicks(int maj, int min, int len)
{
	tickMaj = maj;
	tickMin = min;
	tickLen = len;
	CalcCenter();
}

void KnobWidget::SetColors(wdgColor& bg, wdgColor& fg)
{
	bgClr = bg;
	fgClr = fg;
}

void KnobWidget::CalcCenter()
{
	int step = 2 + tickLen;
	dial = area;
	dial.Shrink(step, step);
	centerPt.x = dial.x + (dial.w / 2);
	centerPt.y = dial.y + (dial.h / 2);
}

int KnobWidget::CalcIndicator()
{
	int x = valuePt.x;
	int y = valuePt.y;

	double ang = pi125 - ((double)((value - minval)/ range) * pi150);
	double rx = (double) dial.w / 2.0;
	double ry = (double) dial.h / 2.0;
	double ca = cos(ang);
	double sa = sin(ang);
	valuePt.x = centerPt.x + (int) (ca * rx);
	valuePt.y = centerPt.y - (int) (sa * ry);
	return valuePt.x != x || valuePt.y != y;
}

int KnobWidget::GetKnobImage(KnobImageType t)
{
	KnobData *kd = new KnobData;
	kd->type = t;
	kd->cx = area.w;
	kd->cy = area.h;
	kd->tick = tickLen;
	kd->tmaj = tickMaj;
	kd->tmin = tickMin;
	kd->angle = faceAng;
	kd->style = style;
	kd->fgClr = fgClr;
	kd->bgClr = bgClr;
	kd->faceClr = faceClr;
	kd->highClr = highClr;

	if (knbImg != NULL)
	{
		if (knbImg->Match(kd))
		{
			delete kd;
			return 1;
		}
		knbImg->Release();
		knbImg = 0;
	}

	knbImg = knobCache->FindImage(kd);
	if (knbImg != NULL)
	{
		knbImg->AddRef();
		delete kd;
		return 1;
	}
	knbImg = knobCache->AddImage(kd);
	return 0;
}

int KnobWidget::BtnDown(int x, int y, int ctrl, int shift)
{
	moving = 1;
	lastPt.x = x;
	lastPt.y = y;
	return 0;
}

int KnobWidget::BtnUp(int x, int y, int ctrl, int shift)
{
	int chng = 0;
	if (moving)
	{
		moving = 0;
		chng = ChangeValue(x, y, ctrl, shift);
	}
	return chng;
}

int KnobWidget::MouseMove(int x, int y, int ctrl, int shift)
{
	if (moving)
		return ChangeValue(x, y, ctrl, shift);
	return 0;
}

int KnobWidget::ChangeValue(int x, int y, int ctrl, int shift)
{
	float dx = (float) (lastPt.x - x);
	float dy = (float) (lastPt.y - y);
	lastPt.x = x;
	lastPt.y = y;
	if (y < centerPt.y)
		dx = -dx;
	if (x > centerPt.x)
		dy = -dy;
	float lastval = value;
	int wndx = 0;
	if (ctrl)
		wndx |= 2;
	if (shift)
		wndx |= 1;
	value = value + (warp[wndx] * (dx+dy));

	if (value < minval)
		value = minval;
	else if (value > maxval)
		value = maxval;
	if (lastval == value)
		return 0;

	int r = CalcIndicator();
	if (buddy2)
	{
//		buddy2->SetValue(GetValue());
		buddy2->SetValue(Round(value));
		form->Redraw(buddy2);
	}
	if (r)
	{
		form->Redraw(this);
		ValueChanged(this);
	}
	return r;
}

int KnobWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);

	wdgColor hiclr = 0xc0c0c0;
	wdgColor loclr = 0x505050;
	GetColorAttribute(elem, "hiclr", hiclr);
	GetColorAttribute(elem, "loclr", loclr);

	short style;
	if (elem->GetAttribute("style", style))
		style = 0;

	float angle;
	if (elem->GetAttribute("angle", angle))
		angle = 135.0;
	SetFace(loclr, hiclr, angle, style);

	float scl;
	float lo;
	float hi;
	short prec;
	if (elem->GetAttribute("lo", lo))
		lo = 0.0;
	if (elem->GetAttribute("hi", hi))
		hi = 1.0;
	if (elem->GetAttribute("prec", prec))
		prec = 3;
	if (elem->GetAttribute("scl", scl) == 0)
		SetScale(scl);
	elem->GetAttribute("log", logScale);
	SetRange(lo, hi, prec);

	float v;
	if (elem->GetAttribute("val", v))
		v = 0.0;
	SetValue(v);

	short tmaj = 0;
	short tmin = 0;
	short tlen = 0;
	elem->GetAttribute("tickmaj", tmaj);
	elem->GetAttribute("tickmin", tmin);
	elem->GetAttribute("ticklen", tlen);
	SetTicks(tmaj, tmin, tlen);
	return 0;
}

void KnobBlack::CalcCenter()
{
	int step = (area.w / 10) + tickLen;
	shaft = area;
	shaft.Shrink(step, step);
	dial = shaft;
	shaft.Offset(0, -(step/2));
	dial.Offset(0, step/2);
	centerPt.x = dial.x + (dial.w / 2);
	centerPt.y = dial.y + (dial.h / 2);
}

void KnobBased::CalcCenter()
{
	int step = tickLen + 1;
	base = area;
	base.Shrink(step, step);
	step = base.w / 10;
	shaft = base;
	shaft.Shrink(step, step);
	step = shaft.w / 12;
	dial = shaft;
	dial.Shrink(step, step);
	centerPt.x = dial.x + (dial.w / 2);
	centerPt.y = dial.y + (dial.h / 2);
}

FrequencyGroup::FrequencyGroup()
{
	type = wdgGroup;
	track = 0;
	fine = 0;
	course = 0;
	value = 0;
}

FrequencyGroup::~FrequencyGroup()
{
}

float FrequencyGroup::GetValue()
{ 
	float vc;
	float vf;
	if (course)
		vc = course->GetValue();
	else
		vc = 0;
	if (fine)
		vf = fine->GetValue();
	else
		vf = 0;
	return vc + vf;
}

void FrequencyGroup::SetValue(float v)
{
	float vc = floor(v);
	float vf = v - vc;
	if (fine)
		fine->SetValue(vf);
	if (course)
		course->SetValue(vc);
}

void FrequencyGroup::SetPairs(int knbCourse, int knbFine, int lblCourse, int lblFine)
{
	SynthWidget *lbl;
	if (knbCourse >= 0)
	{
		course = FindID(knbCourse);
		if (course && lblCourse >= 0)
		{
			if ((lbl = FindID(lblCourse)) != 0 && lbl != course)
			{
				course->SetBuddy2(lbl);
				lbl->SetBuddy1(course);
			}
		}
	}
	if (knbFine >= 0)
	{
		fine = FindID(knbFine);
		if (fine && lblFine >= 0)
		{
			if ((lbl = FindID(lblFine)) != 0 && lbl != fine)
			{
				fine->SetBuddy2(lbl);
				lbl->SetBuddy1(fine);
			}
		}
	}
}

int FrequencyGroup::Load(XmlSynthElem *elem)
{
	fine = 0;
	course = 0;
	int r = WidgetGroup::Load(elem);
	short idkc = -1;
	short idlc = -1;
	short idkf = -1;
	short idlf = -1;
	short id;
	if (elem->GetAttribute("kc", id) == 0)
		idkc = id;
	if (elem->GetAttribute("lc", id) == 0)
		idlc = id;
	if (elem->GetAttribute("kf", id) == 0)
		idkf = id;
	if (elem->GetAttribute("lf", id) == 0)
		idlf = id;
	SetPairs(idkc, idkf, idlc, idlf);

	return r;
}

SynthWidget *FrequencyGroup::HitTest(int x, int y)
{
	SynthWidget *lbl;
	SynthWidget *track = 0;
	if (course)
	{
		track = course->HitTest(x, y);
		if (!track && (lbl = course->GetBuddy2()) != 0)
			track = lbl->HitTest(x, y);
	}
	if (!track && fine)
	{
		track = fine->HitTest(x, y);
		if (!track && (lbl = fine->GetBuddy2()) != 0)
			track = lbl->HitTest(x, y);
	}
	return track;
}

void FrequencyGroup::ValueChanged(SynthWidget *wdg)
{
	SynthWidget::ValueChanged(this);
}

