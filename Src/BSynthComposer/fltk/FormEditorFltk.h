//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file FormEditorFltk.h Form Window implementation
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _FORM_EDITOR_H
#define _FORM_EDITOR_H

class FormEditorFltk : 
	public Fl_Group,
	public FormEditor
{
protected:
	ProjectItem *item;
	WidgetForm *form;
	int setCapture;
	int formW;
	int formH;
	Fl_Scrollbar *vscrl;
	Fl_Scrollbar *hscrl;
	wdgColor bgColor;
	wdgColor fgColor;

public:
	FormEditorFltk(int x, int y, int w, int h);
	virtual ~FormEditorFltk();

	void draw();
	int handle(int e);
	void resize(int X, int Y, int W, int H);

	void CopyToClipboard();
	void PasteFromClipboard();

	ProjectItem *GetItem()
	{
		return item;
	}

	void SetItem(ProjectItem *p);
	void SetForm(WidgetForm *wf);
	
	WidgetForm *GetForm()
	{ 
		return form; 
	}

	void GetSize(int& cx, int& cy)
	{
		if (form)
			form->GetSize(cx, cy);
		else
		{
			cx = 100;
			cy = 100;
		}
	}

	void Undo() 
	{
		if (form)
			form->Cancel();
	}

	void Redo() { }
	void SelectAll() { }
	void GotoLine(int ln) { }
	void GotoPosition(int pos) { }
	void Find() { }
	void FindNext() { }
	void SetMarker() { }
	void SetMarkerAt(int line, int on) { }
	void NextMarker() { }
	void PrevMarker() { }
	void ClearMarkers() { }

	void Cancel()
	{
		if (form)
			form->Cancel();
	}

	int IsChanged()
	{
		if (form)
			return form->IsChanged();
		return 0;
	}

	void Focus() { take_focus(); }

	long EditState();

	void Copy() { CopyToClipboard(); }
	void Cut()  { CopyToClipboard(); }
	void Paste() { PasteFromClipboard(); }
	void Capture();
	void Release();
	void Resize();
	void Redraw(SynthWidget *wdg);
	SynthWidget *SystemWidget(const char *) { return 0; }
	void DrawWidget(SynthWidget *wdg);
	void OnScroll();
};

#endif
