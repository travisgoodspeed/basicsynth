//////////////////////////////////////////////////////////////////////
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "QueryValueDlg.h"

BEGIN_EVENT_TABLE(QueryValueDlg, wxDialog)
	EVT_BUTTON(wxID_OK, QueryValueDlg::OnOK)
	EVT_BUTTON(wxID_CANCEL, QueryValueDlg::OnCancel)
END_EVENT_TABLE()

QueryValueDlg::QueryValueDlg(wxWindow *parent, const char *p, char *v, int l)
{
	wxXmlResource::Get()->LoadDialog(this, parent, "DLG_GET_VALUE");
	CenterOnParent();
	value = v;
	vlen = l;
	SetLabel(p);
	wxButton *b = (wxButton*)FindWindow(wxID_OK);
	if (b)
		b->SetDefault();
	wxWindow *w = FindWindow("IDC_VALUE");
    if (w->IsKindOf(CLASSINFO(wxTextCtrl)))
	{
		txtwnd = static_cast<wxTextCtrl*>(w);
		txtwnd->ChangeValue(v);
		txtwnd->SetFocus();
	}
	else
		txtwnd = 0;
}

QueryValueDlg::~QueryValueDlg(void)
{
}

void QueryValueDlg::OnOK(wxCommandEvent &evt)
{
	if (txtwnd)
		strncpy(value, txtwnd->GetValue().data(), vlen);
	EndModal(1);
}

void QueryValueDlg::OnCancel(wxCommandEvent &evt)
{
	EndModal(0);
}
