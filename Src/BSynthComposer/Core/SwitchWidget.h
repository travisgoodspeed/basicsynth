//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _SWITCHWIDGET_H_
#define _SWITCHWIDGET_H_

enum SwitchImageType
{
	nullSwitch = 0,
	pushSwitch = 1,
	slideSwitch = 2,
	lampSwitch = 3
};

class SwitchData : public ImageData
{
public:
	wdgColor fgClr;
	wdgColor bgClr;
	void *bm[2];

	SwitchData()
	{
		bm[0] = 0;
		bm[1] = 0;
		type = nullSwitch;
	}

	virtual ~SwitchData()
	{
		DestroyImage();
	}

	virtual int Compare(ImageData *id);

	virtual void *GetImage(int n)
	{
		if (n < 2)
			return bm[n];
		return 0;
	}

	virtual void DestroyImage();
	virtual void CreateImage();
};

class LampData : public SwitchData
{
public:
	wdgColor hiClr;
	wdgColor loClr;
	int style;

	LampData()
	{
		type = lampSwitch;
	}

	virtual ~LampData()	{ }
	virtual int Compare(ImageData *id);
};

extern WidgetImageCache *switchCache;

class SwitchWidget : public SynthWidget
{
protected:
	ImageCacheItem *sip;
	float value;
	int swOn;
	int act; // 0 = momentary, 1 = toggle, 2 = radio
	int track;
	bsString lbl;
	int lblHeight;
	int bold;
	int italic;
	int align;
	int shadow;
	wdgRect lblRect;
	void *psdata;

	virtual int GetSwitchImage(SwitchData *sd);
	virtual int GetSwitchImage(SwitchImageType t);

public:
	static wdgColor swuphi;
	static wdgColor swuplo;
	static wdgColor swdnhi;
	static wdgColor swdnlo;

	SwitchWidget();
	virtual ~SwitchWidget();

	virtual void SetArea(wdgRect& r);
	void SetText(const char *s);
	void SetToggle(int t);
	virtual void SetState(int n);
	virtual int GetState();
	virtual float GetValue();
	virtual void SetValue(float v);
	virtual int Tracking() { return track; }
	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);

	// base class is In/Out "pressed" type button
	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
	virtual void DrawLabel(DrawContext dc);

	virtual int Load(XmlSynthElem *elem);
};

class SlideSwitchWidget : public SwitchWidget
{
private:
	int style;
public:
	SlideSwitchWidget();
	virtual ~SlideSwitchWidget();

	void SetStyle(int s)
	{
		style = s;
	}

	virtual void CreateImage();
	virtual void Paint(DrawContext dc);
	virtual int Load(XmlSynthElem *elem);
};

class ImageWidget : public SwitchWidget
{
private:
	char *onImg;
	char *offImg;
	ImageCacheItem *sipImg;

public:
	ImageWidget();
	virtual ~ImageWidget();

	void *LoadImage(char *file);
	virtual void Paint(DrawContext dc);
	virtual void CreateImage();

	virtual int Load(XmlSynthElem *elem);
};

class LampWidget : public SwitchWidget
{
private:
	wdgColor hiColor;
	wdgColor loColor;
	int style;

	virtual int GetSwitchImage(SwitchImageType t);

public:
	LampWidget();
	virtual ~LampWidget();

	virtual void SetArea(wdgRect& r);
	virtual void Paint(DrawContext dc);
	virtual void CreateImage();

	virtual int Load(XmlSynthElem *elem);
};

class SwitchGroup : public WidgetGroup
{
protected:
	SynthWidget *track;
	int defID;
	static int EnumEnable(SynthWidget *wdg, void *arg);
public:

	SwitchGroup();
	virtual ~SwitchGroup();

	virtual void SetEnable(int n);
	virtual float GetValue();
	virtual void SetValue(float v);
	virtual void ValueChanged(SynthWidget *wdg);
	virtual int Load(XmlSynthElem *elem);
	virtual SynthWidget *HitTest(int x, int y);
	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual int Tracking();
	SynthWidget *AddWidget(const char *type, short x, short y, short w, short h);
};

class SwitchSet : public WidgetGroup
{
private:
	static int EnumEnable(SynthWidget *wdg, void *arg);
public:
	SwitchSet();
	virtual ~SwitchSet();

	virtual void SetEnable(int n);
	virtual float GetValue();
	virtual void SetValue(float v);
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
