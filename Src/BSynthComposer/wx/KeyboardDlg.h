//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#pragma once

/// @brief Virtual Keyboard window
/// @details The virtual keyboard window displays a piano keyboard
/// and allows playing notes by clicking on the keyboard. 
class KeyboardDlg : 
	public wxWindow,
	public FormEditor
{
private:
    DECLARE_EVENT_TABLE()
	wxColour bgColor;
	KeyboardForm *form;
	wxListBox *instrList;
	static wxBitmap *offscrn;
	static wxRect offsSize;
	int CheckBuffer(int w, int h);
	wxMBConvUTF8 cutf8;

public:
	KeyboardDlg(wxWindow *parent, int w, int h);
	virtual ~KeyboardDlg();

	virtual ProjectItem *GetItem() { return 0; }
	virtual void SetItem(ProjectItem *p) { }
	virtual void Undo() { }
	virtual void Redo() { }
	virtual void Cut() { }
	virtual void Copy() { }
	virtual void Paste() { }
	virtual void Find() { }
	virtual void FindNext() { }
	virtual void SelectAll() { }
	virtual void GotoLine(int ln) { }
	virtual void GotoPosition(int n) { }
	virtual void SetMarker() { }
	virtual void SetMarkerAt(int line, int on) { }
	virtual void NextMarker() { }
	virtual void PrevMarker() { }
	virtual void ClearMarkers() { }
	virtual void Cancel() { }
	virtual long EditState() { return 0; }
	virtual int IsChanged()  { return 0; }
	virtual void Focus() { SetFocus(); }
	virtual void SetForm(WidgetForm *frm) { }
	virtual WidgetForm *GetForm() { return form; }

	virtual void Capture()  { CaptureMouse(); }
	virtual void Release()  { ReleaseMouse(); }
	virtual void Redraw(SynthWidget *wdg);
	virtual void Resize() { }

	virtual SynthWidget *SystemWidget(const char *type) { return 0; }


	void OnPaint(wxPaintEvent& evt);
	void OnErase(wxEraseEvent& evt);
	void OnBtnDown(wxMouseEvent&);
	void OnBtnUp(wxMouseEvent&);
	void OnMouseMove(wxMouseEvent&);
	void OnInstrChange(wxCommandEvent&);

	void Load();
	int IsRunning();
	int Stop();
	int Start();
	void Clear();
	void InitInstrList();
	int FindInstrument(InstrConfig *ic);
	int AddInstrument(InstrConfig *ic);
	int RemoveInstrument(InstrConfig *ic);
	int UpdateInstrument(InstrConfig *ic);
	int SelectInstrument(InstrConfig *ic);
	void UpdateChannels();
};
