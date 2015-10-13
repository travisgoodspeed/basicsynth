//////////////////////////////////////////////////////////////////////
// BasicSynth - Base class for widgets used to edit instruments.
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
#include "SliderWidget.h"
#include "SwitchWidget.h"
#include "TextWidget.h"
#include "EnvelopeWidget.h"
#include "KeyboardWidget.h"
#include "WaveWidget.h"
#include "WidgetForm.h"

WidgetColorMap SynthWidget::colorMap;

SynthWidget *SynthWidget::MakeWidget(const char *type)
{
	SynthWidget *wdg = 0;
	if (strcmp(type, "dial") == 0)
		wdg = new KnobWidget;
	else if (strcmp(type, "knob") == 0)
		wdg = new KnobBlack;
	else if (strcmp(type, "baseknob") == 0)
		wdg = new KnobBased;
	else if (strcmp(type, "slider") == 0)
		wdg = new SliderWidget;
	else if (strcmp(type, "pushswitch") == 0)
		wdg = new SwitchWidget;
	else if (strcmp(type, "slideswitch") == 0)
		wdg = new SlideSwitchWidget;
	else if (strcmp(type, "pushbutton") == 0)
	{
		wdg = new SwitchWidget;
		wdg->type = wdgCmd;
	}
	else if (strcmp(type, "label") == 0)
		wdg = new TextWidget;
	else if (strcmp(type, "group") == 0)
		wdg = new WidgetGroup;
	else if (strcmp(type, "switchgroup") == 0)
		wdg = new SwitchGroup;
	else if (strcmp(type, "switchset") == 0)
		wdg = new SwitchSet;
	else if (strcmp(type, "envgraph") == 0)
		wdg = new EnvelopeWidget;
	else if (strcmp(type, "graph") == 0)
		wdg = new GraphWidget;
	else if (strcmp(type, "frqgroup") == 0)
		wdg = new FrequencyGroup;
	else if (strcmp(type, "imageswitch") == 0)
		wdg = new ImageWidget;
	else if (strcmp(type, "imagebutton") == 0)
	{
		wdg = new ImageWidget;
		wdg->type = wdgCmd;
	}
	else if (strcmp(type, "waveform") == 0)
		wdg = new WaveWidget;
	else if (strcmp(type, "lamp") == 0)
		wdg = new LampWidget;
	else if (strcmp(type, "keyboard") == 0)
		wdg = new KeyboardWidget;
	else if (strcmp(type, "box") == 0)
		wdg = new BoxWidget;
    //else
	//	printf("Unknown widget %s\n", type);
	return wdg;
}

SynthWidget::SynthWidget(wdgType t) : type(t)
{
	enable = 1;
	show = 1;
	focus = 0;
	buddy1 = 0;
	buddy2 = 0;
	parent = 0;
	id = -1;
	ip = -1;
	fgClr = 0xffffffff;
	bgClr = 0xff000000;
	form = 0;
	psdata = 0;
}

SynthWidget::~SynthWidget()
{
}

void SynthWidget::ValueChanged(SynthWidget *wdg)
{
	if (parent)
		parent->ValueChanged(wdg);
	else if (form)
		form->ValueChanged(wdg);
}

int SynthWidget::Load(XmlSynthElem *elem)
{
	short v;
	if (elem->GetAttribute("id", v) == 0)
		id = v;
	if (elem->GetAttribute("ip", v) == 0)
		ip = v;

	wdgColor fg = fgClr;
	wdgColor bg = bgClr;
	GetColorAttribute(elem, "fg", fg);
	GetColorAttribute(elem, "bg", bg);
	SetColors(bg,fg);
	if (elem->GetAttribute("show", v) == 0)
		show = v;
	if (elem->GetAttribute("enable", v) == 0)
		enable = v;
	return 0;
}

int SynthWidget::GetColorAttribute(XmlSynthElem *elem, const char *attr, wdgColor& clr)
{
	char *clrstr;
	if (elem->GetAttribute(attr, &clrstr) == 0)
	{
		if (*clrstr == '#')
		{
			char *digits = &clrstr[1];
			size_t len = strlen(digits);
			unsigned red = 0;
			unsigned grn = 0;
			unsigned blu = 0;
			unsigned alpha = 255;
			if (len == 8) // #aarrggbb
			{
				alpha = HexDig(digits[0]) << 4;
				alpha += HexDig(digits[1]);
				len = 6;
				digits += 2;
			}
			else if (len == 4) // #argb
			{
				alpha = HexDig(digits[0]);
				alpha += alpha << 4;
				len = 3;
				digits += 1;
			}
			else
				alpha = 0xff;
			if (len == 6) // #rrggbb
			{
				red = (HexDig(digits[0]) << 4) + HexDig(digits[1]);
				grn = (HexDig(digits[2]) << 4) + HexDig(digits[3]);
				blu = (HexDig(digits[4]) << 4) + HexDig(digits[5]);
			}
			else if (len == 3) // #rgb
			{
				red = HexDig(digits[0]);
				red += red << 4;
				grn = HexDig(digits[1]);
				grn += grn << 4;
				blu = HexDig(digits[2]);
				blu += blu << 4;
			}
			else if (len == 2) // two digit gray-scale level
			{
				red = (HexDig(digits[0]) << 4) + HexDig(digits[1]);
				grn = red;
				blu = red;
			}
			else //if (len == 1) // single digit gray-scale level
			{
				red = HexDig(digits[0]);
				red += red << 4;
				grn = red;
				blu = red;
			}

			clr = (wdgColor) ((alpha << 24) + (red << 16) + (grn << 8) + blu);
		}
		else if (*clrstr >= '0' && *clrstr <= '9')
			clr = (wdgColor) atol(clrstr) | 0xff000000;
		else if (!colorMap.Find(clrstr, clr))
			clr = 0xff000000;
		delete clrstr;
		return 0;
	}
	return -1;
}

unsigned int SynthWidget::HexDig(int ch)
{
	if (ch >= '0' && ch <= '9')
		return (unsigned)ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return (unsigned)ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return (unsigned)ch - 'a' + 10;
	return 0;
}


///////////////////////////////////////////////////////////////////////////////////////

int WidgetGroup::EnumFindID(SynthWidget *wdg, void *id)
{
	FindStruct *ff = (FindStruct *)id;
	if ((ff->hit = wdg->FindID(ff->id)) == 0)
		return 1;
	return 0;
}

int WidgetGroup::EnumFindIP(SynthWidget *wdg, void *id)
{
	FindStruct *ff = (FindStruct *)id;
	if ((ff->hit = wdg->FindIP(ff->id)) == 0)
		return 1;
	return 0;
}

int WidgetGroup::EnumWindow(SynthWidget *wdg, void *arg)
{
	wdg->WidgetWindow(arg);
	return 1;
}

int WidgetGroup::EnumPaint(SynthWidget *wdg, void *arg)
{
	wdg->Paint((DrawContext)arg);
	return 1;
}

int WidgetGroup::EnumHitTest(SynthWidget *wdg, void *arg)
{
	HitStruct *h = (HitStruct *)arg;
	h->hit = wdg->HitTest(h->x, h->y);
	if (h->hit)
		return 0;
	return 1;
}

int WidgetGroup::EnumGetSize(SynthWidget *wdg, void *arg)
{
	int x = 0;
	int y = 0;
	wdg->GetSize(x, y);
	int *sz = (int*)arg;
	sz[0] = x;
	sz[1] = y;
	return 1;
}

int WidgetGroup::EnumMove(SynthWidget *wdg, void *arg)
{
	int *offs = (int*)arg;
	wdgRect a = wdg->GetArea();
	a.x += offs[0];
	a.y += offs[1];
	wdg->SetArea(a);
	return 1;
}

WidgetGroup::WidgetGroup() : SynthWidget(wdgGroup)
{
	frClr = 0;
	border = 0;
	style = 0;
	wdgHead.Insert(&wdgTail);
}

WidgetGroup::~WidgetGroup()
{
	Clear();
}

void WidgetGroup::Clear()
{
	SynthWidget *wdg;
	while ((wdg = wdgHead.next) != &wdgTail)
	{
		wdg->Remove();
		delete wdg;
	}
}

void WidgetGroup::WidgetWindow(void *w)
{
	EnumList(EnumWindow, w);
}

SynthWidget *WidgetGroup::EnumList(WGCallback cb, void *arg)
{
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if (cb(wdg, arg) == 0)
			return wdg;
		wdg = wdg->next;
	}
	return 0;
}

void WidgetGroup::SetArea(wdgRect& r)
{
	int offs[2];
	offs[0] = r.x - area.x;
	offs[1] = r.y - area.y;
	EnumList(EnumMove, offs);
	area = r;
}

SynthWidget *WidgetGroup::FindID(int n)
{
	if (n == id)
		return this;
	FindStruct ff;
	ff.id = n;
	ff.hit = 0;
	EnumList(EnumFindID, (void*)&ff);
	return ff.hit;
}

SynthWidget *WidgetGroup::FindIP(int n)
{
	if (n == ip)
		return this;
	FindStruct ff;
	ff.id = n;
	ff.hit = 0;
	EnumList(EnumFindIP, (void*)&ff);
	return ff.hit;
}

SynthWidget *WidgetGroup::HitTest(int x, int y)
{
	HitStruct h;
	h.x = x;
	h.y = y;
	h.hit = 0;
	EnumList(EnumHitTest, (void*)&h);
	return h.hit;

	/*SynthWidget *hit = 0;
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if ((hit = wdg->HitTest(x, y)) != 0)
			return hit;
		wdg = wdg->next;
	}
	return 0;*/
}

void WidgetGroup::GetSize(int& cx, int& cy)
{
	int sz[2];
	sz[0] = cx;
	sz[1] = cy;
	SynthWidget::GetSize(sz[0], sz[1]);
	EnumList(EnumGetSize, (void*)sz);
	cx = sz[0];
	cy = sz[1];
}

int WidgetGroup::Load(XmlSynthElem *elem)
{
	SynthWidget::Load(elem);
	float w;
	if (elem->GetAttribute("bord", w) == 0)
		border = w;
	elem->GetAttribute("style", style);
	GetColorAttribute(elem, "fr", frClr);

	XmlSynthElem wdgNode(elem->Document());
	if (!elem->FirstChild(&wdgNode))
		return 0;
	do
	{
		if (wdgNode.TagMatch("pair"))
		{
			short knbID;
			short lblID;
			wdgNode.GetAttribute("knb", knbID);
			wdgNode.GetAttribute("lbl", lblID);
			SynthWidget *knb = FindID(knbID);
			SynthWidget *lbl = FindID(lblID);
			if (knb && lbl)
			{
				knb->SetBuddy2(lbl);
				lbl->SetBuddy1(knb);
			}
		}
		else if (wdgNode.TagMatch("color"))
		{
			colorMap.Add(&wdgNode);
		}
		else
		{
			bsString wdgtype;
			if (wdgNode.TagMatch("wdg"))
			{
				char *type = 0;
				wdgNode.GetAttribute("ty", &type);
				wdgtype.Attach(type);
			}
			else
				wdgtype = wdgNode.TagName();
			if (wdgtype.Length() > 0)
			{
				short x = 0;
				short y = 0;
				short w = 0;
				short h = 0;
				wdgNode.GetAttribute("x", x);
				wdgNode.GetAttribute("y", y);
				wdgNode.GetAttribute("w", w);
				wdgNode.GetAttribute("h", h);
				SynthWidget *wdg = AddWidget(wdgtype, x, y, w, h);
				if (wdg)
				{
					if (wdg->Load(&wdgNode))
					{
						wdg->Remove();
						delete wdg;
					}
				}
			}
		}
	} while (wdgNode.NextSibling(&wdgNode));

	return 0;
}

SynthWidget *WidgetGroup::AddWidget(SynthWidget *wdg)
{
	wdg->SetForm(form);
	wdg->SetParent(this);
	wdg->SetColors(bgClr, fgClr);
	wdgTail.InsertBefore(wdg);
	return wdg;
}

SynthWidget *WidgetGroup::AddWidget(const char *type, short x, short y, short w, short h)
{
	SynthWidget *wdg = MakeWidget(type);
	if (wdg == 0)
	{
		// not a builtin type. Maybe a system widget...
		if (form)
		{
			FormEditor *ed = form->GetFormEditor();
			if (ed)
				wdg = ed->SystemWidget(type);
		}
		if (wdg == 0)
			return 0;
		//	wdg = new BoxWidget;
	}
	wdgRect rc(x + area.x, y + area.y, w, h);
	wdg->SetParent(this);
	wdg->SetForm(form);
	wdg->SetArea(rc);
	wdg->SetColors(bgClr, fgClr);
	wdgTail.InsertBefore(wdg);
	return wdg;
}

// top-down traversal
int WidgetGroup::EnumDn(WGCallback cb, void *arg)
{
	int rv;
	if ((rv = cb(this, arg)) != 0)
		return rv;
	SynthWidget *wdg = wdgHead.next;
	while (wdg != &wdgTail)
	{
		if ((rv = wdg->EnumDn(cb, arg)) != 0)
			return rv;
		wdg = wdg->next;
	}
	return 0;
}

/// bottom-up (depth-first) traversal
int WidgetGroup::EnumUp(WGCallback cb, void *arg)
{
	int rv;
	SynthWidget *wdg = wdgTail.prev;
	while (wdg != &wdgHead)
	{
		if ((rv = wdg->EnumUp(cb, arg)) != 0)
			return rv;
		wdg = wdg->prev;
	}
	return cb(this, arg);
}

/////////////////////////////////////////////////////////////

WidgetColorEntry *WidgetColorEntry::Match(const char *nm)
{
	if (name.CompareNC(nm) == 0)
		return this;
	if (next)
		return next->Match(nm);
	return 0;
}

WidgetColorMap::WidgetColorMap()
{
	head.name = "black";
	head.clrVal = 0xff000000;
	tail.name = "white";
	tail.clrVal = 0xff000000;
	head.Insert(&tail);
}

WidgetColorMap::~WidgetColorMap()
{
	DeleteAll();
	tail.Remove();
}

void WidgetColorMap::DeleteAll()
{
	WidgetColorEntry *ent;
	while ((ent = head.next) != &tail)
	{
		head.next = ent->Remove();
		delete ent;
	}
}

int WidgetColorMap::Find(const char *name, wdgColor &val)
{
	if (head.next == &tail)
		return 0;
	WidgetColorEntry *ent = head.Match(name);
	if (ent)
	{
		val = ent->clrVal;
		return 1;
	}
	return 0;
}

// load a colors file.
// 'file' can be a full path or relative to 'path'
// 'top' should be 1 for the top level file. 
// Included files set 'top' to 0.
// a colors file is XML format with two valid tag values:
// <color name="name" val="color" />
// <include>file</include>
// a color value can be hex format or another color name, i.e.:
// <color name="black" val="#ff000000" />
// <color name="background" val="black" />
void WidgetColorMap::Load(const char *path, const char *file, int top)
{
	int found = 0;
	bsString fullPath;
	if ((found = SynthFileExists(file)) != 0)
		fullPath = file;
	else if (path)
	{
		fullPath = path;
		fullPath += '/';
		fullPath += file;
		found = SynthFileExists(fullPath);
	}

	if (found)
	{
		if (top)
			DeleteAll();
		XmlSynthDoc doc;
		XmlSynthElem *root = doc.Open(fullPath);
		if (root)
		{
			XmlSynthElem node(&doc);
			XmlSynthElem *child = root->FirstChild(&node);
			while (child)
			{
				if (child->TagMatch("color"))
					Add(child);
				else if (child->TagMatch("include"))
				{
					char *data = 0;
					child->GetContent(&data);
					if (data)
					{
						Load(path, data, 0);
						delete data;
					}
				}
				child = child->NextSibling(&node);
			}
			delete root;
			doc.Close();
		}
	}

	if (top)
	{
		// Set the global switch shadings
		if (!Find("swuphi", SwitchWidget::swuphi))
			SwitchWidget::swuphi = 0xffb4b4b4;
		if (!Find("swuplo", SwitchWidget::swuplo))
			SwitchWidget::swuplo = 0xff5a5a55;
		if (!Find("swdnhi", SwitchWidget::swdnhi))
			SwitchWidget::swdnhi = 0xff808078;
		if (!Find("swdnlo", SwitchWidget::swdnlo))
			SwitchWidget::swdnlo = 0xff424242;
	}
}

void WidgetColorMap::Add(const char *name, wdgColor clr)
{
	WidgetColorEntry *ent = head.Match(name);
	if (!ent)
	{
		ent = new WidgetColorEntry;
		tail.InsertBefore(ent);
		ent->name = name;
	}
	ent->clrVal = clr;
}

void WidgetColorMap::Add(XmlSynthElem *node)
{
	char *nm = 0;
	if (node->GetAttribute("name", &nm) == 0)
	{
		wdgColor val;
		SynthWidget::GetColorAttribute(node, "val", val);
		Add(nm, val);
		delete nm;
	}
}
