//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file TabsView.cpp Editor tab window implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "TabsView.h"
#include "MainFrm.h"

static void tvCB(Fl_Widget *w, void *arg)
{
	mainWnd->EditorSelected((EditorView *)arg);
}

TabsView::TabsView(int X, int Y, int W, int H) 
	: Fl_Group(X, Y, W, H, 0), 
	chooser(0)
{
	chooser = new Fl_Choice(X, Y+5, W, H-10);
	end();
}

TabsView::~TabsView()
{
}

void TabsView::DeleteEditor(const Fl_Menu_Item *mnuItm)
{
	EditorView *ed = (EditorView *)mnuItm->user_data();
	if (ed)
	{
//		mnuItm->user_data(0);
		ed->SetItem(0);

		Fl_Widget *wdg = (Fl_Widget *) ed->GetPSData();
		ed->SetPSData(0);
		if (wdg)
		{
			Fl_Group *wdgparent = wdg->parent();
			if (wdgparent)
				wdgparent->remove(wdg);
			Fl::delete_widget(wdg);
		}
	}
}

void TabsView::AddItem(EditorView *ed)
{
	const char *name;
	ProjectItem *prjItm = ed->GetItem();
	if (prjItm)
		name = prjItm->GetName();
	else
	{
		static int id = 1;
		static char buf[20];
		snprintf(buf, 20, "%d", id++);
		name = buf;
	}
	int ndx = chooser->add(name, (int)0, tvCB, (void *)ed, 0);
	chooser->value(ndx);
	chooser->redraw();
}

void TabsView::RemoveItem(EditorView *ed)
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (mnuItm == 0)
		return;
	const Fl_Menu_Item* mnuSel = chooser->mvalue();
	int ndx = 0;
	while (mnuItm->label())
	{
		if (mnuItm->user_data() == (void*)ed)
		{
			DeleteEditor(mnuItm);
			chooser->remove(ndx);
			if (mnuItm != mnuSel)
				chooser->value(mnuSel);
			else
				chooser->value(chooser->menu());
			break;
		}
		mnuItm++;
		ndx++;
	}
	chooser->redraw();
	redraw();
}

int TabsView::CloseAll(int query)
{
	int ret = 1;
	int ndx = 0;
	while (ndx < chooser->size())
	{
		const Fl_Menu_Item *mnuItm = chooser->menu();
		if (mnuItm == NULL)
			break;
		if (mnuItm[ndx].label() == NULL)
			break;
		EditorView *ed = (EditorView *)mnuItm[ndx].user_data();
		if (ed)
		{
			ProjectItem *pi = ed->GetItem();
			if (query && pi)
			{
				if (!pi->CloseItem())
				{
					ndx++;
					ret = 0;
				}
			}
			else
			{
				DeleteEditor(&mnuItm[ndx]);
				chooser->remove(ndx);
			}
		}
		else
			chooser->remove(ndx);
	}
	redraw();
	return ret;
}

int TabsView::SaveAll(int query)
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (!mnuItm)
		return 1;
	while (mnuItm->label())
	{
		EditorView *ed = (EditorView *)mnuItm->user_data();
		if (ed)
		{
			if (ed->IsChanged())
			{
				ProjectItem *itm = ed->GetItem();
				if (itm)
				{
					int res = 1;
					if (query)
					{
						bsString msg;
						msg = mnuItm->label();
						msg += " has changed. Save?";
						res = prjFrame->Verify(msg, "Wait...");
						if (res == -1) // cancel
							return 0;
					}
					if (res)
						itm->SaveItem();
				}
			}
		}
		mnuItm++;
	}
	return 1;
}

void TabsView::RemoveAll()
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (!mnuItm)
		return;
	int cnt = 0;
	while (mnuItm->label())
	{
		DeleteEditor(mnuItm);
		cnt++;
		mnuItm++;
	}
	chooser->clear();
	redraw();
}

void TabsView::SetActiveItem(EditorView *ed)
{
	const Fl_Menu_Item *mnuItm = chooser->menu();
	if (!mnuItm)
		return;
	while (mnuItm->label())
	{
		if (mnuItm->user_data() == (void*)ed)
		{
			chooser->value(mnuItm);
			break;
		}
		mnuItm++;
	}
}

EditorView *TabsView::GetActiveItem()
{
	const Fl_Menu_Item* mnuItm = chooser->mvalue();
	if (!mnuItm || !mnuItm->label())
	{
		mnuItm = chooser->menu();
		if (mnuItm && mnuItm->label())
			chooser->value(mnuItm);
	}
	if (mnuItm && mnuItm->label())
		return (EditorView *) mnuItm->user_data();
	return 0;
}
