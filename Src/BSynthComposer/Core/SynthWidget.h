//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
// type indcates how the value is stored:
// unk - a null valued item
// Text - a text string
// Value - a floating point number
// Switch - a binary (On/Off) value
// Cmd - a push button
#ifndef _SYNTHWIDGET_H_
#define _SYNTHWIDGET_H_

enum wdgType
{
	wdgUnk,
	wdgText,
	wdgValue,
	wdgSwitch,
	wdgCmd,
	wdgGroup,
	wdgGraph
};

/// Platform independent rectangle
struct wdgRect
{
	int x;
	int y;
	int w;
	int h;

	wdgRect()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	wdgRect(int ix, int iy, int iw, int ih)
	{
		x = ix;
		y = iy;
		w = iw;
		h = ih;
	}

	wdgRect& operator=(const wdgRect& r)
	{
		x = r.x;
		y = r.y;
		w = r.w;
		h = r.h;
		return *this;
	}

	inline int Height() { return h; }
	inline int Width()  { return w; }
	inline int GetLeft()   { return x; }
	inline int GetTop()    { return y; }
	inline int GetRight()  { return w + x; }
	inline int GetBottom() { return h + y; }

	/// Inflate the rectangle.
	void Inflate(int byx, int byy)
	{
		w += byx*2;
		h += byy*2;
		x -= byx;
		y -= byy;
	}

	/// Shrink the rectangle.
	inline void Shrink(int byx, int byy)
	{
		Inflate(-byx, -byy);
	}

	/// Offset the x,y position (move)
	inline void Offset(int dx, int dy)
	{
		x += dx;
		y += dy;
	}

	/// Test a point to see if it is inside the rectangle.
	inline int Inside(int x1, int y1)
	{
		return (x1 >= GetLeft() && x1 <= GetRight()
		     && y1 >= GetTop()  && y1 <= GetBottom());
	}

	/// Test for overlap with another rectangle.
	inline int Intersects(wdgRect& r)
	{
		if (r.x > GetRight())
			return 0;
		if (r.y > GetBottom())
			return 0;
		if (x > r.GetRight())
			return 0;
		if (y > r.GetBottom())
			return 0;
		return 1;
	}

	/// Combine with another rectangle (union).
	void Combine(wdgRect& r)
	{
		if (r.IsEmpty())
			return;
		if (IsEmpty())
		{
			x = r.x;
			y = r.y;
			w = r.w;
			h = r.h;
			return;
		}
		int lft = x;
		int top = y;
		int rgt = x + w;
		int bot = y + h;
		if (r.x < x)
			lft = r.x;
		if (r.y < y)
			top = r.y;
		if (r.GetRight() > GetRight())
			rgt = r.GetRight();
		if (r.GetBottom() > GetBottom())
			bot = r.GetBottom();
		x = lft;
		y = top;
		w = rgt - x;
		h = bot - y;
	}

	/// Test a rectangle for zero width or height.
	inline int IsEmpty()
	{
		return w == 0 && h == 0;
	}

	/// Set the rectangle to zero width and height.
	inline void SetEmpty()
	{
		w = h = 0;
	}
};

/// Platform independent graphics point (x,y coordinate)
struct wdgPoint
{
	int x;
	int y;
	wdgPoint()
	{
		x = y = 0;
	}
};

/// ImageData holds information to identify an image in the image cache.
class ImageData
{
public:
	int type;
	int cx;
	int cy;

	ImageData()
	{
		type = -1;
		cx = 0;
		cy = 0;
	}

	virtual ~ImageData() { }

	virtual int Compare(ImageData *id)
	{
		return type == id->type && cx == id->cx && cy == id->cy;
	}

	// Image specific
	virtual void *GetImage(int n) { return 0; }
	virtual void CreateImage() { }
	virtual void DestroyImage() { }
};

/// One item in an image cache.
class ImageCacheItem : public SynthList<ImageCacheItem>
{
public:
	ImageData *data;
	int refs;

	ImageCacheItem()
	{
		data = 0;
		refs = 0;
	}

	~ImageCacheItem()
	{
		delete data;
	}

	int AddRef()
	{
		return ++refs;
	}

	int Release()
	{
		int r = --refs;
		if (r == 0)
		{
			Remove();
			delete this;
		}
		return r;
	}

	int Match(ImageData *id)
	{
		if (data && id)
			return id->Compare(data);
		return 0;
	}

	void SetData(ImageData *id)
	{
		data = id;
	}

	ImageData *GetData()
	{
		return data;
	}

	void *GetImage(int n)
	{
		if (data)
			return data->GetImage(n);
		return 0;
	}
};

/// An image cache.
/// The image cache is used to hold pre-drawn versions of a widget.
/// Using a bitmap image significantly improves performance. Since
/// many instances of a given image are likely to appear on a form,
/// we create one bitmap and then share it amongst all identical
/// widgets. For example, a knob with a given size, style, and color
/// only needs to be imaged once for all instances of the knob.
class WidgetImageCache
{
private:
	ImageCacheItem *head;
	ImageCacheItem *tail;
public:
	WidgetImageCache()
	{
		head = new ImageCacheItem;
		tail = new ImageCacheItem;
		head->Insert(tail);
	}

	~WidgetImageCache()
	{
		Clear();
		delete tail;
		delete head;
	}

	void Clear()
	{
		ImageCacheItem *id;
		while ((id = head->next) != tail)
			id->Release();
	}

	ImageCacheItem *FindImage(ImageData *id)
	{
		ImageCacheItem *img = head->next;
		while (img != tail)
		{
			if (img->Match(id))
				return img;
			img = img->next;
		}
		return 0;
	}

	ImageCacheItem *AddImage(ImageData *id)
	{
		ImageCacheItem *img = new ImageCacheItem;
		img->AddRef();
		img->SetData(id);
		id->CreateImage();
		tail->InsertBefore(img);
		return img;
	}
};

typedef long wdgColor;

/// An entry in the color map.
class WidgetColorEntry : public SynthList<WidgetColorEntry>
{
public:
	bsString name;
	wdgColor clrVal;

	WidgetColorEntry()
	{
		clrVal = 0;
	}

	void Load(XmlSynthElem *node);
	WidgetColorEntry *Match(const char *nm);
};

/// The color map is used to translate text names to binary RGB values.
/// Color names are typically functional, not descriptive, e.g.,
/// volume, frequency, etc. Using color names in the widget form
/// files allows the color scheme to be changed globally by changing
/// the color map file.
class WidgetColorMap
{
private:
	WidgetColorEntry head;
	WidgetColorEntry tail;

public:
	WidgetColorMap();
	~WidgetColorMap();
	void DeleteAll();
	void Load(const char *path, const char *file, int top=1);
	int Find(const char *name, wdgColor &val);
	void Add(const char *name, wdgColor val);
	void Add(XmlSynthElem *node);
};


class WidgetForm;

// callback function for enumeration
typedef int (*WGCallback)(SynthWidget *wdg, void *arg);

/// SynthWidget is the base class for all other widgets.
/// A widget has a display area (rectangle), style, colors, and
/// widget specific parameters. Widgets are arranged in a heirarchy
/// with the GroupWidget the base class for aggregates. Widgets within
/// a group have a position relative to the position of the group in
/// the form file, but are given an absolute position as they are loaded.
///
/// A widget that handles mouse interaction must implement the BtnDn(), BtnUp(),
/// MouseMove(), and Tracking() functions. Tracking should return true if
/// the widget is responding to mouse movements (mouse capture).
///
/// A widget has an ID and optional IP. IP is the instrument parameter
/// number. Widgets can be located by ID or IP as needed.
///
/// When the value or state of a widget changes it invokes ValueChanged()
/// on its parent object. If the widget has no parent, the form method is called.
/// I.E., the events "bubble" to the top allowing each group widget to
/// provide specialized handling of child events.
///
/// The "buddy system" is used to link knobs (buddy1) with labels (buddy2)
/// but could be used for other things as well. The idea is that buddy1
/// is the master, buddy2 the follower. If a widget has a "buddy1" and
/// that widget receives direct input, it should set buddy1's value and wait to
/// recive the new value indirectly. If a widget has a buddy2, it should
/// pass all values and individual redraws to buddy2.
class SynthWidget : public SynthList<SynthWidget>
{
public:
	wdgRect area;
	wdgColor fgClr;
	wdgColor bgClr;
	wdgType type;
	SynthWidget *parent;
	SynthWidget *buddy1;
	SynthWidget *buddy2;
	WidgetForm  *form;
	int id; // widget id
	int ip; // instrument parameter number
	int enable;
	int show;
	int focus;
	void *psdata; // platform-specific data

	SynthWidget(wdgType t = wdgUnk);
	virtual ~SynthWidget();

	static WidgetColorMap colorMap;
	static SynthWidget *MakeWidget(const char *type);

	void SetForm(WidgetForm *f) { form = f; }
	void *GetPSData() { return psdata; }
	void SetPSData(void *p) { psdata = p; }

	SynthWidget *GetParent() { return parent; }
	void SetParent(SynthWidget *p) { parent = p; }
	SynthWidget *GetBuddy1() { return buddy1; }
	SynthWidget *GetBuddy2() { return buddy2; }
	void SetBuddy1(SynthWidget *p) { buddy1 = p; }
	void SetBuddy2(SynthWidget *p) { buddy2 = p; }

	wdgType GetType() { return type; }

	virtual void SetShow(int n) { show = n; }
	virtual int GetShow() { return show; }
	virtual void SetEnable(int n) { enable = n; if (buddy1) buddy1->SetEnable(n);}
	virtual int GetEnable() { return enable; }
	virtual int SetFocus() { focus = 1; return 0; }
	virtual int LoseFocus() { focus = 0; return 0; }
	virtual int OnKey(int key) { return 0; }
	virtual int OnEnter() { return 0; }
	virtual int OnEscape() { return 0; }

//	virtual int Copy() { return 0; }
//	virtual int Cut() { return 0; }
//	virtual int Paste() { return 0; }
//	virtual int Undo() { return 0; }
//	virtual int Redo() { return 0; }

	virtual void SetID(int i) { id = i; }
	virtual int GetID() { return id; }
	virtual SynthWidget *FindID(int i)
	{
		if (i == id)
			return this;
		return 0;
	}

	virtual void SetIP(int i) { ip = i; }
	virtual int GetIP() { return ip; }
	virtual SynthWidget *FindIP(int i)
	{
		if (i == ip)
			return this;
		return 0;
	}

	const wdgRect& GetArea()
	{
		return area;
	}

	virtual void SetArea(wdgRect& r)
	{
		area = r;
	}

	virtual void GetSize(int& cx, int& cy)
	{
		int n = area.GetRight();
		if (n > cx)
			cx = n;
		n = area.GetBottom();
		if (n > cy)
			cy = n;
	}

	virtual void SetColors(wdgColor bg, wdgColor fg)
	{
		bgClr = bg;
		fgClr = fg;
	}

	virtual void GetColors(wdgColor& bg, wdgColor& fg)
	{
		bg = bgClr;
		fg = fgClr;
	}

	virtual SynthWidget *HitTest(int x, int y)
	{
		if (x >= area.x && x <= area.GetRight()
		 && y >= area.y && y <= area.GetBottom())
		{
			return this;
		}
		return 0;
	}

	virtual const char *GetText() { return ""; }
	virtual void SetText(const char *p) { SetValue((float)bsString::StrToFlp(p)); }
	virtual float GetValue() { return 0; }
	virtual void SetValue(float v) { }
	virtual int GetState() { return 0; }
	virtual void SetState(int s) { }
	virtual void ValueChanged(SynthWidget *wdg);

	virtual void WidgetWindow(void *w) { }
	virtual void Paint(DrawContext gr) { }

	virtual int NotifyMsg(void *nm) { return 0; }
	virtual int CommandMsg(short code) { return 0; }
	virtual int BtnDown(int x, int y, int ctrl, int shift) { return 0; }
	virtual int BtnUp(int x, int y, int ctrl, int shift) { return 0; }
	virtual int MouseMove(int x, int y, int ctrl, int shift) { return 0; }
	virtual int Tracking() { return 0; }

	virtual int EnumUp(WGCallback cb, void *arg) { return cb(this, arg); }
	virtual int EnumDn(WGCallback cb, void *arg) { return cb(this, arg); }

	inline int Red(wdgColor c)   { return (int)((c >> 16) & 0xff); }
	inline int Green(wdgColor c) { return (int)((c >> 8) & 0xff); }
	inline int Blue(wdgColor c)  { return (int)(c & 0xff); }
	inline int Alpha(wdgColor c) { return (int)((c >> 24) & 0xff); }
	inline long ClrRGBA(int r, int g, int b, int a = 255) 
	{
		return ((long)(a&0xff) << 24)
			 | ((long)(r&0xff) << 16) 
			 | ((long)(g&0xff) << 8)
			 | ((long)(b&0xff));
	}

	/// Load parameters.
	/// @param elem XML node containing position and options
	virtual int Load(XmlSynthElem *elem);
	static int GetColorAttribute(XmlSynthElem *elem, const char *attr, wdgColor& clr);
	static unsigned int HexDig(int ch);
};

/// A dynamic aggregate of widgets.
class WidgetGroup : public SynthWidget
{
protected:
	SynthWidget wdgHead;
	SynthWidget wdgTail;
	wdgColor frClr;
	float border;
	short style;

	struct HitStruct
	{
		int x;
		int y;
		SynthWidget *hit;
	};

	struct FindStruct
	{
		int id;
		SynthWidget *hit;
	};

	static int EnumMove(SynthWidget *wdg, void *arg);
	static int EnumFindID(SynthWidget *wdg, void *id);
	static int EnumFindIP(SynthWidget *wdg, void *id);
	static int EnumWindow(SynthWidget *wdg, void *arg);
	static int EnumPaint(SynthWidget *wdg, void *arg);
	static int EnumHitTest(SynthWidget *wdg, void *arg);
	static int EnumGetSize(SynthWidget *wdg, void *arg);
	SynthWidget *EnumList(WGCallback cb, void *arg);

public:
	WidgetGroup();
	virtual ~WidgetGroup();

	virtual void SetArea(wdgRect& r);
	void SetBorder(float n) { border = n; }
	void Clear();
	virtual void Paint(DrawContext gr);
	virtual void WidgetWindow(void *w);
	virtual SynthWidget *FindID(int n);
	virtual SynthWidget *FindIP(int n);
	virtual SynthWidget *HitTest(int x, int y);
	virtual int EnumUp(WGCallback cb, void *arg);
	virtual int EnumDn(WGCallback cb, void *arg);
	virtual void GetSize(int& cx, int& cy);
	virtual int Load(XmlSynthElem *elem);
	virtual SynthWidget *AddWidget(SynthWidget *wdg);
	virtual SynthWidget *AddWidget(const char *type, short x, short y, short w, short h);
};

#endif
