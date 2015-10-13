//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _WIDGET_FORM_H
#define _WIDGET_FORM_H

class WidgetForm
{
protected:
	FormEditor *edwin;
	WidgetGroup *mainGroup;
	SynthWidget *track;
	SynthWidget *focus;
	long bgColor;
	long fgColor;
	bsString formFont; ///< actually, just the face name...
public:
	WidgetForm();
	virtual ~WidgetForm();

	virtual FormEditor *GetFormEditor() { return edwin; }
	virtual void SetFormEditor(FormEditor *e) { edwin = e; }

	virtual void SetFocus();
	virtual void KillFocus();
	virtual void BtnDn(int x, int y, int shft, int ctl);
	virtual void BtnUp(int x, int y, int shft, int ctl);
	virtual void MouseMove(int x, int y, int shft, int ctl);
	virtual void RedrawForm(DrawContext ctx);
	virtual void CopyText(bsString& text);
	virtual void PasteText(const char *text);
	virtual int CanCopy();
	virtual int CanPaste();
	virtual void Command(int id, int cmd);

	virtual int EnumUp(WGCallback cb, void *arg);
	virtual int EnumDn(WGCallback cb, void *arg);
	virtual void ValueChanged(SynthWidget *wdg);
	virtual void Redraw(SynthWidget *wdg);
	virtual void Resize();
	virtual void MoveTo(int x, int y);
	virtual int Load(const char *fileName, int xo, int yo);
	virtual int Load(XmlSynthElem *root, int xo, int yo);
	virtual void GetSize(int& cx, int& cy);
	virtual void GetParams() { }
	virtual void SetParams() { }
	virtual void Cancel() { }
	virtual int IsChanged() { return 0; }
	virtual float GetWidgetValue(int id, float def = 0.0f);
	virtual void SetWidgetValue(int id, float val);
	virtual int GetWidgetState(int id, int def = 0);
	virtual void SetWidgetState(int id, int st);
	virtual void SetWidgetText(int id, const char *txt);
	virtual int GetWidgetText(int id, char *txt, size_t len);
	virtual const char *LabelFont();
	virtual const char *FormsDir();
};

#endif
