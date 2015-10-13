//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef _FORM_EDITOR_H
#define _FORM_EDITOR_H

class ScrollForm;

class FormEditorWin : 
	public CWindowImpl<FormEditorWin>, 
	public CMessageFilter,
	public FormEditor
{
protected:
	ScrollForm *scrl;
	ProjectItem *item;
	WidgetForm *form;
	Color bgColor;
	Color fgColor;

public:
	static Bitmap *offscrn;
	static RECT offsSize;

	FormEditorWin();
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

	void SetScrollWin(ScrollForm *w)
	{
		scrl = w;
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

	void GetSize(Size& sz)
	{
		if (form)
			form->GetSize(sz.Width, sz.Height);
		else
		{
			sz.Width = 100;
			sz.Height = 100;
		}
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

	void Focus() { SetFocus(); }

	long EditState();

	void Copy() { CopyToClipboard(); }
	void Cut()  { CopyToClipboard(); }
	void Paste() { PasteFromClipboard(); }
	void Capture()  { SetCapture(); }
	void Release()  { ReleaseCapture(); }
	void Redraw(SynthWidget *wdg);
	void Resize();
	SynthWidget *SystemWidget(const char *type);

	DECLARE_WND_CLASS_EX("FormEditorWin", CS_HREDRAW|CS_VREDRAW, 0)

	BOOL PreTranslateMessage(MSG* pMsg);

	BEGIN_MSG_MAP(FormEditorWin)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONDOWN , OnBtnDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnBtnUp)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnBtnUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

class ScrollForm : 
	public CWindowImpl<ScrollForm>
{
protected:
	FormEditorWin *formWnd;
	Color bgColor;
	int cmdID;

public:
	ScrollForm(int id = 0)
	{
		formWnd = 0;
		cmdID = id;
		//bgColor.SetFromCOLORREF(GetSysColor(COLOR_APPWORKSPACE));
		bgColor.SetFromCOLORREF(RGB(0x80,0x80,0x80));
	}

	~ScrollForm()
	{
	}

	void SetForm(FormEditorWin *w)
	{
		formWnd = w;
	}
	
	FormEditorWin *GetForm()
	{
		return formWnd;
	}

	void SetColor(COLORREF c)
	{
		bgColor.SetFromCOLORREF(c);
	}

	void Layout();

	DECLARE_WND_CLASS_EX("ScrollForm", CS_HREDRAW|CS_VREDRAW, 0)

	BEGIN_MSG_MAP(ScrollForm)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)
		MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
		MESSAGE_HANDLER(WM_VSCROLL, OnVScroll)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnWheel)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
		FORWARD_NOTIFICATIONS()
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnErase(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnVScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHScroll(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
};

#endif
