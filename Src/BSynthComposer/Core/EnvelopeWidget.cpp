//////////////////////////////////////////////////////////////////////
// BasicSynth - Widget to display an envelope graph
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "SynthWidget.h"
#include "EnvelopeWidget.h"

EnvelopeWidget::EnvelopeWidget() : SynthWidget(wdgGraph)
{
	ampRange = 1.0;
	tmRange = 2.0;
	border = 1;
	numSegs = 0;
	vals = 0;
	start = 0;
	susOn = 0;
	frClr = 0xff000000;
}

EnvelopeWidget::~EnvelopeWidget()
{
	delete vals;
}

void EnvelopeWidget::SetSegs(int n)
{
	SegVals *newVals = new SegVals[n];
	int i;
	for (i = 0; i < n && i < numSegs; i++)
	{
		newVals[i].level = vals[i].level;
		newVals[i].rate = vals[i].rate;
		newVals[i].type = vals[i].type;
	}
	numSegs = n;
	if (vals)
		delete[] vals;
	vals = newVals;
	for (; i < numSegs; i++)
	{
		vals[i].level = 0;
		vals[i].rate = 0;
		vals[i].type = linSeg;
	}
}

void EnvelopeWidget::SetVal(int n, float rt, float lvl)
{
	if (n >= 0 && n < numSegs)
	{
		vals[n].level = lvl;
		vals[n].rate = rt;
	}
}

int EnvelopeWidget::Load(XmlSynthElem *elem)
{
	int err = SynthWidget::Load(elem);
	float w;
	if (elem->GetAttribute("bord", w) == 0)
		border = (int)w;
	GetColorAttribute(elem, "fr", frClr);
	return err;
}
