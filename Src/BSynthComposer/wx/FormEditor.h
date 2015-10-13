//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Implementation of the forms editor window.
/// @details The form window implements the FormEditor interface
/// for a specific windowing framework.
class FormEditorWin : 
	public wxWindow,
	public FormEditor
{
private:
    DECLARE_EVENT_TABLE()
protected:
	ProjectItem *item;
	WidgetForm *form;
	wxColor bgColor;
	wxColor fgColor;

	static wxBitmap *offscrn;
	static wxRect offsSize;

	int CheckBuffer(int w, int h);
	void UpdateScrollBars();

public:
	FormEditorWin(wxWindow *parent);
	virtual ~FormEditorWin();

	void CopyToClipboard();
	void PasteFromClipboard();

	ProjectItem *GetItem()
	{
		return item;
	}

	void SetItem(ProjectItem *p)
	{
		item = p;
		item->SetEditor(this);
		form = item->CreateForm(0,0);
		if (form)
		{
			form->SetFormEditor(this);
			form->GetParams();
		}
	}

	void SetForm(WidgetForm *wf)
	{
		form = wf;
		wf->SetFormEditor(this);
	}

	WidgetForm *GetForm()
	{
		return form;
	}

	void GetSize(wxSize& sz)
	{
		int x = 100;
		int y = 100;
		if (form)
			form->GetSize(x, y);
		sz.Set(x, y);
	}

	void Undo() 
	{
		if (form)
			form->Cancel();
	}

	void Redo() { }
	void SelectAll() { }
	void GotoLine(int n) { }
	void GotoPosition(int n) { }
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

	void Save() 
	{
		if (form)
			form->SetParams();
	}

	int IsChanged()
	{
		if (form)
			return form->IsChanged();
		return 0;
	}

	long EditState();
	void Focus() { SetFocus(); }

	void Copy() { CopyToClipboard(); }
	void Cut()  { CopyToClipboard(); }
	void Paste() { PasteFromClipboard(); }
	void Capture()  { CaptureMouse(); }
	void Release()  { ReleaseMouse(); }
	void Redraw(SynthWidget *wdg);
	void Resize();
	SynthWidget *SystemWidget(const char *type);

	void OnPaint(wxPaintEvent& evt);
	void OnEraseBackground(wxEraseEvent& evt);
	void OnSize(wxSizeEvent& evt);
	void OnScroll(wxScrollWinEvent& evt);
	void OnBtnDown(wxMouseEvent&);
	void OnBtnUp(wxMouseEvent&);
	void OnMouseMove(wxMouseEvent&);
	void OnCommand(wxCommandEvent&);
	void OnSetFocus(wxFocusEvent&);
	void OnKillFocus(wxFocusEvent&);
};
