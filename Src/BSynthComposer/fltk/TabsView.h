//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Editor tab window class declaration.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef TABS_VIEW_H
#define TABS_VIEW_H

class TabsView : public Fl_Group
{
private:
	Fl_Choice *chooser;

	void DeleteEditor(const Fl_Menu_Item *mnuItm);

public:
	TabsView(int X, int Y, int W, int H);
	~TabsView();

	void AddItem(EditorView *ed);
	void RemoveItem(EditorView *ed);
	int CloseAll(int query);
	int SaveAll(int query);
	void RemoveAll();
	void SetActiveItem(EditorView *ed);
	EditorView *GetActiveItem();
};

#endif
