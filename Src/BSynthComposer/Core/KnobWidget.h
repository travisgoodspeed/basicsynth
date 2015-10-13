//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _KNOBWIDGET_H_
#define _KNOBWIDGET_H_

#pragma once
#define pi2 (3.141592654*2)
// PI * 1.25
#define pi125 3.926990817
// PI * 1.50
#define pi150 4.71238898

enum KnobImageType
{
	nullKnob = 0,
	plainKnob = 1,
	blackKnob = 2,
	baseKnob = 3
};

/// Image cache data for a knob
struct KnobData : public ImageData
{
	long highClr;
	long faceClr;
	long fgClr;
	long bgClr;
	int tick;
	int tmaj;
	int tmin;
	int style;
	float angle;
	void *bm;

	KnobData()
	{
		bm = 0;
	}

	virtual ~KnobData()
	{
		DestroyImage();
	}

	virtual int Compare(ImageData *id)
	{
		if (ImageData::Compare(id))
		{
			KnobData *kd = (KnobData *)id;
			return style == kd->style
				&& angle == kd->angle
				&& faceClr == kd->faceClr
				&& highClr == kd->highClr
				&& fgClr == kd->fgClr
				&& bgClr == kd->bgClr
				&& tick == kd->tick
				&& tmaj == kd->tmaj
				&& tmin == kd->tmin;
		}
		return 0;
	}

	virtual void *GetImage(int n)
	{
		return bm;
	}

	// platform specific image creation functions
	virtual void CreateImage();
	virtual void DestroyImage();
};

extern WidgetImageCache *knobCache;

/// The knob widget displays a circle with an indicator.
/// The knob represents a variable value, with a minimum,
/// maximum, and scaling factor. In addition, the digits
/// of precision (right of the decimal point) can be set
/// so that "round" values are returned. The knob value
/// is changed with the mouse by moving the mouse in a 
/// circular motion. Each pixel of movement is scaled to a
/// change in the value based on the range of values.
/// Change in x and y mouse position results in
/// increase or decrease of value based on the position of
/// the mouse relative to the center of the widget. If the
/// mouse is above center, an increase in x produces an
/// increase in value and a decrease in x produces a
/// decrease in value. If the mouse is below center,
/// the opposite occurs. For the y position, when
/// the mouse is to the right of center, an increase
/// in y produces a decrease in value, etc.
/// This emulates the action of a physical knob
/// where the knob is moved by setting a finger against
/// the knob and moving back and forth. Holding down
/// the shift and control keys while moving the mouse
/// "warps" the speed of the knob so that a pixel movement
/// is multiplied or divided. When shift and control are up,
/// a one pixel movement moves the value by the next to least
/// digit.
///
/// The graphic allows for a gradient fill on the face of the
/// knob (where the platform supports it) producing a 3D shading effect.
class KnobWidget : public SynthWidget
{
protected:
	float value;
	float minval;
	float maxval;
	float range;
	float scale;
	float warp[4];
	short logScale;
	int prec;
	int moving;
	wdgPoint centerPt;
	wdgPoint valuePt;
	wdgPoint lastPt;
	wdgRect dial;
	wdgColor faceClr;
	wdgColor highClr;
	float faceAng;
	float faceAmt[3];
	float facePos[3];
	ImageCacheItem *knbImg;

	int tickMaj;
	int tickMin;
	int tickLen;
	int style;

public:
	KnobWidget();
	virtual ~KnobWidget();
	virtual void SetArea(wdgRect& r);
	virtual void SetValue(float v);
	virtual float GetValue();

	virtual int Tracking();
	virtual int SetFocus();
	virtual int LoseFocus();

	float Round(float v);
	void SetScale(float s);
	void SetRange(float lo, float hi, int s = 3);
	void SetFace(wdgColor& fc, wdgColor& hc, float ang = 135.0, int sty = 0);
	void SetTicks(int maj, int min, int len);
	void SetColors(wdgColor& bg, wdgColor& fg);
	int CalcIndicator();
	int GetKnobImage(KnobImageType t);
	int ChangeValue(int x, int y, int ctrl, int shift);

	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Load(XmlSynthElem *elem);

	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
	virtual void DrawTicks(DrawContext dc, wdgRect& bk, wdgRect& fc);
};

/// The knob black widget displays a black, knurled knob with an indicator.
class KnobBlack : public KnobWidget
{
private:
	wdgRect shaft;
public:
	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
};

/// The based knob widget displays a three-part knob with gray base, shaft and face.
class KnobBased : public KnobWidget
{
private:
	wdgRect base;
	wdgRect shaft;
public:
	virtual void CalcCenter();
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
};

/// A frequency group combines two knobs and two labels.
/// The value of the group is the sum of the two knobs.
/// One knob represents the integer frequency multiple (course tuning)
/// while the other knob represents the fractional multiple (fine tuning).
class FrequencyGroup : public WidgetGroup
{
protected:
	SynthWidget *track;
	SynthWidget *fine;
	SynthWidget *course;
	float value;

public:
	FrequencyGroup();
	virtual ~FrequencyGroup();
	virtual float GetValue();
	virtual void SetValue(float v);
	virtual void SetPairs(int knbCourse, int knbFine, int lblCourse, int lblFine);
	virtual int Load(XmlSynthElem *elem);
	virtual SynthWidget *HitTest(int x, int y);
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
