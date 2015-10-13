//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Main frame menu class declaration
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

class MainMenu : public Fl_Menu_Bar
{
public:
	MainMenu(int w);
	~MainMenu();

	void ItemSelected(ProjectItem *pi);
	void EditorSelected(EditorView *vw);
	void UpdateEditState(long flags);
	void UpdateRecentList();
	void EnableProject(int e);
	void EnableFile(int e);
	void EnableItem(int ndx, int e);
	void CheckProject(int ck);
	void CheckKeyboard(int ck);
};

#endif

