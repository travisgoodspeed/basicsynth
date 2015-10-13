//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _WVWIDGET_H_
#define _WVWIDGET_H_

#ifndef WFI_MAXPART
#define WFI_MAXPART 16
#endif

class WaveWidget  : public SynthWidget
{
public:
	int wvType;
	int gibbs;
	int on[WFI_MAXPART];
	float amps[WFI_MAXPART];
	float phs[WFI_MAXPART];
	wdgColor frClr;
	int border;

	WaveWidget() : SynthWidget(wdgGraph)
	{
		memset(on, 0, sizeof(on));
		memset(amps, 0, sizeof(amps));
		memset(phs, 0, sizeof(phs));
		gibbs = 0;
		wvType = 1;
		border = 1;
		fgClr = 0xffffffff;
		frClr = 0xff000000;
	}

	void SetWaveType(int v)
	{
		wvType = v;
	}

	void SetGibbs(int v)
	{
		gibbs = v;
	}

	void SetOn(int n, int v)
	{
		on[n] = v;
	}

	void SetAmp(int n, float v)
	{
		amps[n] = v;
	}

	void SetPhs(int n, float v)
	{
		phs[n] = v;
	}

	virtual int Load(XmlSynthElem *elem)
	{
		int err = SynthWidget::Load(elem);
		float w;
		if (elem->GetAttribute("bord", w) == 0)
			border = (int)w;
		GetColorAttribute(elem, "fr", frClr);
		return err;
	}

	virtual void Paint(DrawContext dc);
	void PlotSum(DrawContext dc);
	void PlotSeg(DrawContext dc);
};

#endif
