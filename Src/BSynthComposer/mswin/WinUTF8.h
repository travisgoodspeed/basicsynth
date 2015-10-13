//////////////////////////////////////////////////////////////////////
/// @file WinUTF8.h Classes that set/get UTF-8 text
///
/// Windows controls all support UNICODE internally.
/// but convert 8-bit characters using the current system
/// code page.
/// The ATL/WTL wrappers are either UTF-16 or MB, with
/// no automatic conversion between UTF-16 and UTF-8 either.
/// These window wrappers solve that problem.
//
// Copyright 2012, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#define MAX_WCHAR_LENGTH 1024


class utf8ListBox : public CListBox
{
public:
	utf8ListBox() { }
	utf8ListBox(HWND w)
	{
		m_hWnd = w;
	}
	utf8ListBox(CWindow& w)
	{
		m_hWnd = w.m_hWnd;
	}
	int AddStringUTF8(const char *str)
	{
		wchar_t wstr[MAX_WCHAR_LENGTH];
		bsString::utf16(str, wstr, MAX_WCHAR_LENGTH);
		return ::SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)wstr);
	}

	int InsertStringUTF8(int ndx, const char *str)
	{
		wchar_t wstr[MAX_WCHAR_LENGTH];
		bsString::utf16(str, wstr, MAX_WCHAR_LENGTH);
		return ::SendMessageW(m_hWnd, LB_INSERTSTRING, (WPARAM)ndx, (LPARAM)wstr);
	}
};

class utf8TreeViewCtrl : public CTreeViewCtrl
{
public:
	utf8TreeViewCtrl() { }
	utf8TreeViewCtrl(HWND w)
	{
		m_hWnd = w;
	}
	utf8TreeViewCtrl(CWindow& w)
	{
		m_hWnd = w.m_hWnd;
	}

	HTREEITEM InsertItemUTF8(HTREEITEM parent, HTREEITEM insAfter, const char *text, void *data)
	{
		TVINSERTSTRUCTW tvi;
		memset(&tvi, 0, sizeof(tvi));
		tvi.hInsertAfter = insAfter;
		tvi.hParent = parent;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM;

		wchar_t wbuf[MAX_WCHAR_LENGTH];
		bsString::utf16(text, wbuf, MAX_WCHAR_LENGTH);
		tvi.item.pszText = wbuf;
		tvi.item.lParam = (LPARAM) data;
		return InsertItemUTF8(&tvi);
	}

	HTREEITEM InsertItemUTF8(TVINSERTSTRUCTW *tvi)
	{
		return (HTREEITEM)::SendMessage(m_hWnd, TVM_INSERTITEMW, 0, (LPARAM)tvi);
	}

	void SetItemTextUTF8(HTREEITEM ht, const char *txt)
	{
		TVITEMW item = { 0 };
		item.hItem = ht;
		item.mask = TVIF_TEXT;
		wchar_t wbuf[MAX_PATH];
		bsString::utf16(txt, wbuf, MAX_PATH);
		item.pszText = wbuf;
		::SendMessage(m_hWnd, TVM_SETITEMW, 0, (LPARAM)&item);
	}
};

class utf8Window : public CWindow
{
public:
	utf8Window() { }
	utf8Window(HWND w)
	{
		m_hWnd = w;
	}
	utf8Window(CWindow& w)
	{
		m_hWnd = w.m_hWnd;
	}

	size_t GetTextUTF8(char *cbuf, int clen)
	{
		wchar_t wstr[MAX_WCHAR_LENGTH];
		memset(wstr, 0, sizeof(wstr));
		::GetWindowTextW(m_hWnd, wstr, MAX_WCHAR_LENGTH);
		return bsString::utf8(wstr, cbuf, clen);
	}

	void SetTextUTF8(const char *str)
	{
		wchar_t wstr[MAX_WCHAR_LENGTH];
		bsString::utf16(str, wstr, MAX_WCHAR_LENGTH);
		::SetWindowTextW(m_hWnd, wstr);
	}
};
