//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _TEXTWIDGET_H_
#define _TEXTWIDGET_H_

#define TEXTFMT_LEN 10

class TextWidget : public SynthWidget
{
protected:
	bsString text;
	int textHeight;
	int textBold;
	int textItalic;
	int filled;
	int shadow;
	int inset;
	int align;
	int editable;
	int track;
	wdgRect textRect;
	bsString fmt;

public:
	TextWidget();
	virtual ~TextWidget();

	void SetFormat(const char *s);
	void SetTextHeight(int h);
	int GetTextHeight() { return textHeight; }
	void SetAlign(int al);
	void SetBold(int b);
	void SetItalic(int it);
	void SetShadow(int sh);
	void SetInset(int in);
	void SetFilled(int fi);
	void SetEdit(int e);
	virtual void SetValue(float v);
	virtual float GetValue();
	virtual void SetText(const char *s);
	virtual const char *GetText();
	virtual int Tracking() { return track; }
	virtual int BtnDown(int x, int y, int ctrl, int shift);
	virtual int BtnUp(int x, int y, int ctrl, int shift);
	virtual int MouseMove(int x, int y, int ctrl, int shift);
	virtual void Paint(DrawContext dc);

	virtual int Load(XmlSynthElem *elem);
};

class BoxWidget : public SynthWidget
{
protected:
	float thick;
	int style; // 0 = single line, 1 = inset, 2 = outset
	int filled;
	wdgColor hiClr;
	wdgColor loClr;
public:
	BoxWidget();
	~BoxWidget();

	void SetStyle(int n);
	virtual void Paint(DrawContext gr);
	virtual int Load(XmlSynthElem *elem);
};

class GraphWidget : public SynthWidget
{
protected:
	int bar;
	int numVals;
	float *vals;
	float range;
	float barWidth;
	float thick;
	wdgColor grClr;

public:
	GraphWidget();
	~GraphWidget();
	void SetNumVals(int n);
	void SetRange(float r);
	void SetVal(int n, float v);

	virtual void Paint(DrawContext dc);
	virtual int Load(XmlSynthElem *elem);
};


/*class EditWidget : public SynthWidget
{
private:
	CEdit edWnd;
	int changed;
	int prec;
	char *txtbuf;

public:
	EditWidget() : SynthWidget(wdgText)
	{
		changed = 0;
		prec = 4;
		txtbuf = 0;
	}

	~EditWidget()
	{
		delete txtbuf;
		if (edWnd.IsWindow())
			edWnd.DestroyWindow();
	}

	void SetValue(float v)
	{
		char txt[40];
		if (prec == 0)
			snprintf(txt, 40, "%d", (int) v);
		else
			snprintf(txt, 40, "%f", Round(v));
		edWnd.SetWindowText(txt);
		changed = 0;
	}

	float GetValue()
	{
		char txt[40];
		edWnd.GetWindowText(txt, 40);
		changed = 0;
		return Round(atof(txt));
	}

	void SetText(const char *p)
	{
		edWnd.SetWindowText(p);
		changed = 0;
	}

	const char *GetText()
	{
		delete txtbuf;
		int len = edWnd.GetWindowTextLength();
		txtbuf = new char[len+1];
		edWnd.GetWindowText(txtbuf, len+1);
		changed = 0;
		return txtbuf;
	}

	float Round(float v)
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

	void Paint(Graphics *gr)
	{
		if (edWnd.IsWindow())
			return;

		SolidBrush bk(bgClr);
		gr->FillRectangle(&bk, area);
	}

	int CommandMsg(short code)
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

	void WidgetWindow(void *w)
	{
		RECT rc;
		rc.left = area.GetLeft();
		rc.top = area.GetTop();
		rc.right = area.GetRight();
		rc.bottom = area.GetBottom();
		edWnd.Create((HWND)w, rc, NULL, WS_CHILD|WS_BORDER|WS_VISIBLE|ES_AUTOHSCROLL, 0, id, 0);
	}

	int Load(XmlSynthElem *elem)
	{
		SynthWidget::Load(elem);
		short v;
		if (elem->GetAttribute("prec", v) == 0)
			prec = v;
		return 0;
	}
};
*/

#endif
