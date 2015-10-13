//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef SLIDER_WIDGET_H
#define SLIDER_WIDGET_H

class SliderWidget : public SynthWidget
{
protected:
	float value;
	float minval;
	float maxval;
	float range;
	float scale;
	float warp[4];
	int prec;
	int moving;
	int vertical;
//	wdgPoint centerPt;
//	wdgPoint valuePt;
//	wdgPoint lastPt;
	wdgRect slideKnob;
	wdgRect slideTrack;
	wdgRect knobPos;
	wdgColor faceClr;
	wdgColor highClr;
//	ImageCacheItem *knbImg;

	int tickMaj;
	int tickMin;
	int tickLen;

public:
	SliderWidget();
	virtual ~SliderWidget();
	virtual void SetArea(wdgRect& r);
	virtual void SetValue(float v);
	virtual float GetValue();

	virtual int Tracking();
	virtual int SetFocus();
	virtual int LoseFocus();

	float Round(float v);
	void SetScale(float s);
	void SetRange(float lo, float hi, int s = 3);
	void SetTicks(int maj, int min, int len);
	void SetColors(wdgColor& bg, wdgColor& fg);
	int CalcPosition();
	int ChangeValue(int x, int y, int ctrl, int shift);

	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Load(XmlSynthElem *elem);

	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
//	virtual void DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc);
};
#endif
