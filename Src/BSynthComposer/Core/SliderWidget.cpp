//////////////////////////////////////////////////////////////////////
// BasicSynth - Widget to display a slider.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "SynthWidget.h"
#include "SliderWidget.h"
#include "WidgetForm.h"

SliderWidget::SliderWidget() : SynthWidget(wdgValue)
{
	vertical = 0;
	value = 0.0;
	minval = 0.0;
	maxval = 1.0f;
	scale = 1.0f;
	warp[0] = 0.01f;
	warp[1] = 0.1f;
	warp[2] = 0.001f;
	warp[3] = 0.001f;
	prec = 3;
	range = maxval - minval;
	moving = 0;
	tickMaj = 0;
	tickMin = 0;
	tickLen = 0;
}

SliderWidget::~SliderWidget()
{
}

void SliderWidget::SetArea(wdgRect& r)
{
	int size;
	area = r;
	vertical = area.w < area.h;
	if (vertical)
	{
		// vertical
		size = (area.w * 3) / 5;
		slideKnob.w = size;
		slideKnob.h = size + (size/2);
		slideKnob.x = area.x + ((area.w+1) - slideKnob.w)/2;
		slideKnob.y = area.y + ((area.h+1) - slideKnob.h)/2;
		slideTrack.w = 3;
		slideTrack.y = area.y + (slideKnob.h / 2) + 4;
		slideTrack.x = area.x + (area.w / 2) - 1;
		slideTrack.h = area.h - slideKnob.h - 8;
	}
	else
	{
		size = (area.h * 3) / 5;
		slideKnob.h = size;
		slideKnob.w = size + (size/2);
		slideKnob.x = area.x + ((area.w+1) - slideKnob.w)/2;
		slideKnob.y = area.y + ((area.h+1) - slideKnob.h)/2;
		slideTrack.h = 2;
		slideTrack.y = area.y + (area.h / 2) - 1;
		slideTrack.x = area.x + (slideKnob.w / 2) + 4;
		slideTrack.w = area.w - slideKnob.w - 8;
	}
	knobPos = slideKnob;
	CalcPosition();
}

void SliderWidget::SetValue(float v)
{
	value = Round(v / scale);
	if (buddy2)
		buddy2->SetValue(value);
	CalcPosition();
}

float SliderWidget::GetValue()
{
	return scale * Round(value);
}

int SliderWidget::Tracking()
{
	return moving;
}

int SliderWidget::SetFocus()
{
	focus = 1;
	return 1;
}

int SliderWidget::LoseFocus()
{
	focus = 0;
	return 1;
}

// Note: using floor(v+0.5) creates problems,
// especially with prec=0, so we actually "truncate"
float SliderWidget::Round(float v)
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

void SliderWidget::SetScale(float s)
{
	scale = s;
}

void SliderWidget::SetRange(float lo, float hi, int s)
{
	minval = lo;
	maxval = hi;
	prec = s;
	range = maxval - minval;
	float div = area.w;
	if (div <= 0)
		div = 1;
	warp[0] = range / (float) (div * 2);
	warp[1] = warp[0] * 10.0; // shift
	warp[2] = pow(10.0, -prec); // ctrl
	warp[3] = warp[2] * 10.0; // shift+ctrl
	CalcPosition();
}

void SliderWidget::SetTicks(int maj, int min, int len)
{
	tickMaj = maj;
	tickMin = min;
	tickLen = len;
//	CalcTrack();
}

void SliderWidget::SetColors(wdgColor& bg, wdgColor& fg)
{
	bgClr = bg;
	fgClr = fg;
}

int SliderWidget::CalcPosition()
{
	int x = knobPos.x;
	int y = knobPos.y;
	if (vertical)
	{
		knobPos.y = slideTrack.GetBottom() - (slideKnob.h/2) - (int) (((value - minval) / range) * (double) slideTrack.h);
	}
	else
	{
		knobPos.x = slideTrack.GetLeft() - (slideKnob.w/2) + (int) (((value - minval) / range) * (double) slideTrack.w);
	}
	return x != knobPos.x || y != knobPos.y;
}

int SliderWidget::BtnDown(int x, int y, int ctrl, int shift)
{
	moving = 1;
	ChangeValue(x, y, ctrl, shift);
	return 0;
}

int SliderWidget::BtnUp(int x, int y, int ctrl, int shift)
{
	int chng = 0;
	if (moving)
	{
		moving = 0;
		chng = ChangeValue(x, y, ctrl, shift);
	}
	return chng;
}

int SliderWidget::MouseMove(int x, int y, int ctrl, int shift)
{
	if (moving)
		return ChangeValue(x, y, ctrl, shift);
	return 0;
}

int SliderWidget::ChangeValue(int x, int y, int ctrl, int shift)
{
	double scale;
	if (vertical)
		scale = (double) (slideTrack.GetBottom() - y) / (double) slideTrack.h;
	else
		scale = (double) (x - slideTrack.x) / (double) slideTrack.w;
	value = (scale * range) + minval;
	if (value < minval)
		value = minval;
	else if (value > maxval)
		value = maxval;
//	ATLTRACE("ChangeValue %d,%d, %f (%f)\n", x, y, value, GetValue());
	int r = CalcPosition();
	if (buddy2)
	{
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

int SliderWidget::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);

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
	SetRange(lo, hi, prec);

	float v;
	if (elem->GetAttribute("val", v))
		v = 0.0;
	SetValue(v);

	short tic;
	if (elem->GetAttribute("tickmaj", tic) == 0)
		tickMaj = tic;
	if (elem->GetAttribute("tickmin", tic) == 0)
		tickMin = tic;
	if (elem->GetAttribute("ticklen", tic) == 0)
		tickLen = tic;
	return 0;
}

