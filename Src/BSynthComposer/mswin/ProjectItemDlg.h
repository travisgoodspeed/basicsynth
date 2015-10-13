//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

template<class PD> class ProjectItemDlg : 
	public PropertyBox
{
protected:
	ProjectItem *pi;

public:
	ProjectItemDlg()
	{
		pi = 0;
	}
	
	virtual ~ProjectItemDlg<PD>()
	{
	}

	virtual int GetFieldID(int id, int& idval)
	{
		idval = -1;
		return 0;
	}

	virtual int GetLabelID(int id, int& idlbl)
	{
		idlbl = -1;
		return 0;
	}

	virtual void EnableValue(int id, int enable)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
			EnableWindow(pt->GetDlgItem(idval), enable);
	}

	virtual void SetCaption(const char *text)
	{
		PD *pt = static_cast<PD*>(this);
		pt->SetWindowText(text);
	}

	virtual void SetState(int id, short onoff)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
			pt->CheckDlgButton(idval, onoff);
	}

	virtual int GetState(int id, short& onoff)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			onoff = pt->IsDlgButtonChecked(idval);
			return 1;
		}
		return 0;
	}

	virtual void SetValue(int id, float val, const char *lbl)
	{
		char txt[40];
		snprintf(txt, 40, "%f", val);
		SetValue(id, txt, lbl);
	}

	virtual void SetValue(int id, long val, const char *lbl)
	{
		char txt[40];
		snprintf(txt, 40, "%d", val);
		SetValue(id, txt, lbl);
	}

	virtual wchar_t *MakeWideString(const char *val)
	{
		int slen = (int) strlen(val) + 1;
		int wlen = ::MultiByteToWideChar(CP_UTF8, 0, val, slen, NULL, 0);
		wchar_t *wstr = new wchar_t[wlen+1];
		if (wstr)
		{
			memset(wstr, 0, (wlen+1)*sizeof(wchar_t));
			::MultiByteToWideChar(CP_UTF8, 0, val, slen, wstr, wlen);
		}
		return wstr;
	}

	virtual void SetTextUTF8(HWND w, const char *val)
	{
		PD *pt = static_cast<PD*>(this);
		wchar_t *wstr = MakeWideString(val);
		if (wstr)
		{
			::SetWindowTextW(w, wstr);
			delete wstr;
		}
	}

	virtual void SetTextUTF8(int id, const char *val)
	{
		PD *pt = static_cast<PD*>(this);
		pt->SetTextUTF8(pt->GetDlgItem(id), val);
	}

	virtual void SetValue(int id, const char *val, const char *lbl)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		int idlbl = -1;
		if (GetFieldID(id, idval))
		{
			//pt->SetDlgItemText(idval, val);
			pt->SetTextUTF8(idval, val);
			if (lbl)
			{
				if (GetLabelID(id, idval))
					pt->SetTextUTF8(idval, lbl);
					//pt->SetDlgItemText(idval, lbl);
			}
		}
	}

	virtual int GetValue(int id, float& val)
	{
		char txt[40];
		if (GetValue(id, txt, 40))
		{
			val = (float)bsString::StrToFlp(txt);
			return 1;
		}
		return 0;
	}

	virtual int GetValue(int id, long& val)
	{
		char txt[40];
		if (GetValue(id, txt, 40))
		{
			val = bsString::StrToNum(txt);
			return 1;
		}
		return 0;
	}

	virtual char *GetTextUTF8(HWND w, char *cbuf, int blen)
	{
		int wlen = ::GetWindowTextLengthW(w)+1;
		wchar_t *wbuf = new wchar_t[wlen];
		if (wbuf)
		{
			memset(wbuf, 0, wlen*sizeof(wchar_t));
			::GetWindowTextW(w, wbuf, wlen);
			int clen = ::WideCharToMultiByte(CP_UTF8, 0, wbuf, wlen, NULL, 0, NULL, NULL);
			if (blen && clen > blen)
				clen = blen;
			if (cbuf == NULL)
				cbuf = new char[clen+1];
			if (cbuf)
			{
				memset(cbuf, 0, clen+1);
				::WideCharToMultiByte(CP_UTF8, 0, wbuf, wlen, cbuf, clen, NULL, NULL);
			}
			delete wbuf;
		}
		else if (cbuf)
		{
			::GetWindowText(w, cbuf, blen);
		}
		return cbuf;
	}

	virtual char *GetTextUTF8(int id, char *cbuf, int blen)
	{
		PD *pt = static_cast<PD*>(this);
		return GetTextUTF8(pt->GetDlgItem(id), cbuf, blen);
	}

	virtual int GetValue(int id, char *val, int len)
	{
		//PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			//pt->GetDlgItemText(idval, val, len);
			if (GetTextUTF8(idval, val, len))
				return 1;
			return 1;
		}
		*val = 0;
		return 0;
	}

	virtual int GetValue(int id, bsString& val)
	{
		//PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			//int len = ::GetWindowTextLengthW(pt->GetDlgItem(idval));
			//char *buf = new char[len+1];
			//pt->GetDlgItemText(idval, buf, len+1);
			val.Attach(GetTextUTF8(idval, NULL, 0));
			return 1;
		}
		return 0;
	}

	void SetLBTextW(HWND w, int index, wchar_t *str)
	{
		::SendMessageW(w, LB_INSERTSTRING, (WPARAM)index, (LPARAM)str);
	}

	virtual int GetLBTextUTF8(HWND w, int index, char *str)
	{
		wchar_t wpath[MAX_PATH];
		memset(wpath, 0, sizeof(wpath));
		int r = GetLBTextW(w, index, wpath);
		::WideCharToMultiByte(CP_UTF8, 0, wpath, (int)wcslen(wpath)+1, str, MAX_PATH, NULL, NULL);
		return r;
	}

	virtual int GetLBTextW(HWND w, int index, const wchar_t *str)
	{
		return (int)::SendMessageW(w, LB_GETTEXT, (WPARAM)index, (LPARAM)str);
	}

	virtual void InsertLBTextUTF8(HWND w, int index, const char *str)
	{
		wchar_t wpath[MAX_PATH];
		::MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wpath, MAX_PATH);
		InsertLBTextW(w, index, wpath);
	}

	virtual void InsertLBTextW(HWND w, int index, const wchar_t *str)
	{
		::SendMessageW(w, LB_INSERTSTRING, (WPARAM)index, (LPARAM)str);
	}

	virtual int AddLBTextUTF8(HWND w, const char *str)
	{
		wchar_t wstr[MAX_PATH];
		memset(wstr, 0, sizeof(wstr));
		::MultiByteToWideChar(CP_UTF8, 0, str, (int)strlen(str)+1, wstr, MAX_PATH);
		return AddLBTextW(w, wstr);
	}

	virtual int AddLBTextW(HWND w, const wchar_t *str)
	{
		return ::SendMessageW(w, LB_ADDSTRING, 0, (LPARAM)str);
	}

	virtual int GetSelection(int id, short& sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// get item selection from list box
			sel = (short) pt->SendDlgItemMessage(idval, LB_GETCURSEL);
			if (sel != LB_ERR)
				return 1;
		}
		return 0;
	}

	virtual void SetSelection(int id, short sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// set item selection for list box
			pt->SendDlgItemMessage(idval, LB_SETCURSEL, (WPARAM)sel);
		}			
	}

	virtual int GetSelection(int id, void **sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		*sel = 0;
		if (GetFieldID(id, idval))
		{
			CListBox lb = pt->GetDlgItem(idval);
			int index = lb.GetCurSel();
			if (index != LB_ERR)
			{
				*sel = lb.GetItemDataPtr(index);
				return 1;
			}
		}
		return 0;
	}

	virtual void SetSelection(int id, void *sel)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			// set item selection for list box
			CListBox lb = pt->GetDlgItem(idval);
			int count = lb.GetCount();
			int index;
			for (index = 0; index < count; index++)
			{
				if (lb.GetItemDataPtr(index) == sel)
				{
					lb.SetCurSel(index);
					break;
				}
			}
		}			
	}

	virtual int ListChildren(int id, ProjectItem *parent)
	{
		PD *pt = static_cast<PD*>(this);
		int count = 0;
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				ProjectItem *itm = prjTree->FirstChild(parent);
				while (itm)
				{
					int ndx = AddLBTextUTF8(list, itm->GetName());
					list.SetItemDataPtr(ndx, itm);
					itm = prjTree->NextSibling(itm);
					count++;
				}
			}
		}
		return count;
	}

	virtual int GetListCount(int id, int& count)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				count = list.GetCount();
				return 1;
			}
		}
		count = 0;
		return 0;
	}

	virtual ProjectItem *GetListItem(int id, int ndx)
	{
		PD *pt = static_cast<PD*>(this);
		int idval = -1;
		if (GetFieldID(id, idval))
		{
			CListBox list = pt->GetDlgItem(idval);
			if (list.IsWindow())
			{
				if (ndx < list.GetCount())
					return (ProjectItem*)list.GetItemDataPtr(ndx);
			}
		}
		return 0;
	}

	ProjectItem *GetItem()
	{
		return pi;
	}

	void SetItem(ProjectItem *p)
	{
		pi = p;
	}

	virtual int Activate(int modal)
	{
		PD *pt = static_cast<PD*>(this);
		if (modal)
			return pt->DoModal();
		if (pt->Create(::GetActiveWindow()))
			pt->ShowWindow(SW_SHOW);
		return 0;
	}
};