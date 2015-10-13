//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef KEYBOARD_DLG_2
#define KEYBOARD_DLG_2

class KeyboardDlg2 : 
	public CWindowImpl<KeyboardDlg2>,
	public FormEditor
{
private:
	KeyboardForm *form;
	Color bgColor;
	//CListBox instrList;
	utf8ListBox instrList;
	HFONT listFont;

public:
	KeyboardDlg2();
	~KeyboardDlg2();

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
	virtual void Focus() { if (IsWindow()) SetFocus(); }
	virtual void SetForm(WidgetForm *frm) { }

	virtual WidgetForm *GetForm()
	{
		return form;
	}

	virtual void Capture()
	{
		SetCapture();
	}

	virtual void Release()
	{
		ReleaseCapture();
	}

	virtual void Redraw(SynthWidget *wdg)
	{
		if (wdg)
		{
			Graphics *gr = new Graphics(m_hWnd);
			wdg->Paint((DrawContext)gr);
			SynthWidget *bud = wdg->GetBuddy2();
			if (bud)
				bud->Paint((DrawContext)gr);
			delete gr;
		}
		else
			InvalidateRect(NULL, 0);
	}

	virtual void Resize()
	{
	}

	virtual SynthWidget *SystemWidget(const char *type) { return 0; }

	BEGIN_MSG_MAP(KeyboardDlg2)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONDOWN , OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnBtnUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		COMMAND_HANDLER(101, LBN_SELCHANGE, OnInstrChange)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInstrChange(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

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
	int InsertInstrument(int ndx, InstrConfig *ic);
	void UpdateChannels();
};

#endif
