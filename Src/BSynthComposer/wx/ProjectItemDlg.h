//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#ifndef _PROJECTITEMDLG_
#define _PROJECTITEMDLG_

#if _MSC_VER
#pragma warning(disable : 4800)
#endif

/// @brief Template for property box dialogs.
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

	ProjectItemDlg(wxWindow *parent, ProjectItem *p, const char *res)
	{
		PD *pdlg = static_cast<PD*>(this);
		pi = p;
		wxXmlResource::Get()->LoadDialog(pdlg, parent, res);
		pdlg->CenterOnParent();
	}

	virtual ~ProjectItemDlg()
	{
	}

	virtual int GetFieldID(int id, wxWindow** idval)
	{
		*idval = 0;
		return 0;
	}

	virtual int GetLabelID(int id, wxWindow** idlbl)
	{
		*idlbl = 0;
		return 0;
	}

	virtual void EnableValue(int id, int enable)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
			child->Enable((bool)enable);
	}

	virtual void SetCaption(const char *text)
	{
		PD *p = static_cast<PD*>(this);
		p->SetLabel(text);
	}

	virtual void SetState(int id, short onoff)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
			((wxCheckBox*)child)->SetValue((bool)onoff);
	}

	virtual int GetState(int id, short& onoff)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			onoff = ((wxCheckBox*)child)->GetValue();
			return 1;
		}
		return 0;
	}

	virtual void SetValue(int id, float val, const char *lbl)
	{
		//char txt[40];
		//snprintf(txt, 40, "%f", val);
		bsString txt((double)val);
		SetValue(id, txt, lbl);
	}

	virtual void SetValue(int id, long val, const char *lbl)
	{
		//char txt[40];
		//snprintf(txt, 40, "%ld", val);
		bsString txt(val);
		SetValue(id, txt, lbl);
	}

	virtual void SetValue(int id, const char *val, const char *lbl)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			// explicitly convert from utf8
			wxMBConvUTF8 conv;
			wxString wval(val, conv);
		    if (child->IsKindOf(CLASSINFO(wxTextCtrl)))
                ((wxTextCtrl*)child)->ChangeValue(wval);
			else if (child->IsKindOf(CLASSINFO(wxListBox))
				||   child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				((wxControlWithItems*)child)->SetStringSelection(wval);
			}
			//else if (child->IsKindOf(CLASSINFO(wxControl)))
            //    ((wxControl*)child)->SetLabel(wval);
			else
				child->SetLabel(wval);
			if (lbl)
			{
				wxString wlbl(lbl, conv);
				if (GetLabelID(id, &child))
					child->SetLabel(wlbl);
			}
		}
	}

	int GetValue(wxWindow *child, wxString& wval)
	{
		if (child)
		{
			if (child->IsKindOf(CLASSINFO(wxTextCtrl)))
				wval = ((wxTextCtrl*)child)->GetValue();
			else if (child->IsKindOf(CLASSINFO(wxListBox))
				||   child->IsKindOf(CLASSINFO(wxComboBox)) )
				wval  = ((wxControlWithItems*)child)->GetStringSelection();
			//else if (child->IsKindOf(CLASSINFO(wxControl)))
			//	wval = ((wxControl*)child)->GetLabel();
			else
				wval = child->GetLabel();
			return 1;
		}
		return 0;
	}

	virtual int GetValueLength(int id)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			wxString wval;
			if (GetValue(child, wval))
				return wval.Len();
		}
		return 0;
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
			//val = atol(txt);
			val = bsString::StrToNum(txt);
			return 1;
		}
		return 0;
	}

	virtual int GetValue(int id, char *val, int len)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			wxString wval;
			if (GetValue(child, wval))
			{
				bsString::utf8(wval.wc_str(), val, len);
				return 1;
			}
		}
		*val = 0;
		return 0;
	}


	virtual int GetValue(int id, bsString& val)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			wxString wval;
			if (GetValue(child, wval))
			{
				val.Assign(wval.wc_str());
				return 1;
			}
		}
		return 0;
	}

	virtual int GetSelection(int id, short& sel)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				// get item selection from list box
				sel = (short) ((wxControlWithItems*)child)->GetSelection();
				if (sel != wxNOT_FOUND)
					return 1;
			}
		}
		return 0;
	}

	virtual void SetSelection(int id, short sel)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				wxControlWithItems* lst = (wxControlWithItems*)child;
				if (lst->GetCount() > (unsigned int)sel)
					lst->SetSelection(sel);
			}
		}
	}

	virtual int GetSelection(int id, void **sel)
	{
		*sel = 0;
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				wxControlWithItems *lb = (wxControlWithItems*)child;
				int index = lb->GetSelection();
				if (index != wxNOT_FOUND)
				{
					*sel = lb->GetClientData(index);
					return 1;
				}
			}
		}
		return 0;
	}

	virtual void SetSelection(int id, void *sel)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				wxControlWithItems *lb = (wxControlWithItems*)child;
				unsigned int count = lb->GetCount();
				unsigned int index;
				for (index = 0; index < count; index++)
				{
					if (lb->GetClientData() == sel)
					{
						lb->SetSelection(index);
						break;
					}
				}
			}
		}
	}

	virtual int ListChildren(int id, ProjectItem *parent)
	{
		int count = 0;
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				wxControlWithItems *lb = (wxControlWithItems*)child;
				ProjectItem *itm = prjTree->FirstChild(parent);
				while (itm)
				{
					lb->Append(itm->GetName(), (void*)itm);
					itm = prjTree->NextSibling(itm);
					count++;
				}
			}
		}
		return count;
	}

	virtual int GetListCount(int id, int& count)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				count = (int) ((wxControlWithItems*)child)->GetCount();
				return 1;
			}
		}
		count = 0;
		return 0;
	}

	virtual ProjectItem *GetListItem(int id, int ndx)
	{
		wxWindow *child = 0;
		if (GetFieldID(id, &child))
		{
			if (child->IsKindOf(CLASSINFO(wxListBox))
			 || child->IsKindOf(CLASSINFO(wxComboBox)) )
			{
				wxControlWithItems *lb = (wxControlWithItems*)child;
				if (ndx < (int)lb->GetCount())
					return (ProjectItem*) lb->GetClientData((unsigned int)ndx);
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
		PD *p = static_cast<PD*>(this);
		if (modal)
			return p->ShowModal();
		p->Show(true);
		return 0;
	}
};
#endif
