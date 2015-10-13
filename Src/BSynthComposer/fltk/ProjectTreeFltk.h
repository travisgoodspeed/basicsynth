//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Project tree window class declaration.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef PROJECT_TREE_FLTK_H
#define PROJECT_TREE_FLTK_H

class ProjectTreeFltk :
#if USE_TREE_CTL
	public Fl_Tree,
#else
	public Fl_Hold_Browser,
#endif
	public ProjectTree
{
private:

public:
	ProjectTreeFltk(int X, int Y, int W, int H);
	virtual ~ProjectTreeFltk();
	virtual int handle(int e);
	virtual void AddNode(ProjectItem *itm, ProjectItem *sib = 0);
	virtual void SelectNode(ProjectItem *itm);
	virtual void RemoveNode(ProjectItem *itm);
	virtual void UpdateNode(ProjectItem *itm);
	virtual void MoveNode(ProjectItem *itm, ProjectItem *prev);
	virtual void RemoveAll();
	virtual ProjectItem *FirstChild(ProjectItem *itm);
	virtual ProjectItem *NextSibling(ProjectItem *itm);
	virtual ProjectItem *GetSelectedNode();

	int CheckButton(int e);
	int FindItem(ProjectItem *itm);
};

#endif
