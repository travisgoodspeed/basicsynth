//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Project tree implementation.
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "ProjectTreeFltk.h"
#include "MainFrm.h"

static ProjectItem *selItm = 0;

static void DoItemSelect(void *arg)
{
	mainWnd->ItemSelected((ProjectItem*)arg);
}

static void DoContextMenu(void *arg)
{
	mainWnd->ItemContextMenu((ProjectItem*)arg);
}

static void DoDoubleClick(void *arg)
{
	mainWnd->ItemDoubleClick((ProjectItem*)arg);
}


#if USE_TREE_CTL

static void treeCB(Fl_Widget *w, void *v)
{
	Fl_Tree *t = (Fl_Tree *)w;
	Fl_Tree_Item *node = t->item_clicked();
	if (node)
		selItm = (ProjectItem*)node->user_data();
	else
		selItm = NULL;
//	mainWnd->ItemSelected(selItm);
	Fl::awake(DoItemSelect, (void*)selItm);
}

int ProjectTreeFltk::CheckButton(int e)
{
	int r = 0;
	if (e == FL_PUSH)
	{
		if (Fl::event_button() == FL_RIGHT_MOUSE)
			r = 1;
	}
	else if (e == FL_RELEASE)
	{
		if (Fl::event_button() == FL_RIGHT_MOUSE)
		{
			const Fl_Tree_Item *node = find_clicked();
			if (node)
				Fl::awake(DoContextMenu, (void*)node->user_data());
			r = 1;
		}
		else if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() > 0)
		{
			Fl::awake(DoDoubleClick, (void*)selItm);
			r = 1;
		}
	}
	return r;
}

int ProjectTreeFltk::handle(int e)
{
	return Fl_Tree::handle(e) | CheckButton(e);
}

ProjectTreeFltk::ProjectTreeFltk(int X, int Y, int W, int H)
: Fl_Tree(X, Y, W, H)
{
	prjTree = static_cast<ProjectTree*>(this);
	callback(treeCB);
	connectorstyle(FL_TREE_CONNECTOR_SOLID);
	selectmode(FL_TREE_SELECT_SINGLE);
	sortorder(FL_TREE_SORT_NONE);
	showroot(0);
	root_label("Project");
	when(FL_WHEN_CHANGED);
}

ProjectTreeFltk::~ProjectTreeFltk()
{
	prjTree = 0;
}

static void ClearNode(Fl_Tree_Item *node)
{
	int count = node->children();
	int index;
	for (index = 0; index < count; index++)
		ClearNode(node->child(index));
	node->clear_children();
	ProjectItem *itm = (ProjectItem *)node->user_data();
	node->user_data(0);
	if (itm)
		itm->Release();
}

void ProjectTreeFltk::RemoveAll()
{
	Fl_Tree_Item *node = root();
	if (node)
	{
		// Don't call ClearNode() on root...
		int count = node->children();
		int index;
		for (index = 0; index < count; index++)
			ClearNode(node->child(index));
		node->clear_children();
	}
}

int ProjectTreeFltk::FindItem(ProjectItem *itm)
{
	return -1;
}

ProjectItem *ProjectTreeFltk::GetSelectedNode()
{
	return selItm;
//	const Fl_Tree_Item* node = find_clicked();
//	if (node)
//		return (ProjectItem*)node->user_data();
//	return 0;
}

void ProjectTreeFltk::SelectNode(ProjectItem *itm)
{
	if (itm != selItm)
	{
		selItm = itm;
		select_only((Fl_Tree_Item*)itm->GetPSData(), 0);
	}
}

void ProjectTreeFltk::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (!itm)
		return;

	Fl_Tree_Item *parentNode = 0;
	Fl_Tree_Item *childNode = 0;
	Fl_Tree_Item *sibNode = 0;
	ProjectItem *parent = itm->GetParent();
	if (parent == NULL)
		// N.B. there is no "root" until you add() !
		parentNode = root();
	else
		parentNode = (Fl_Tree_Item*)parent->GetPSData();
	if (sib)
	{
		sibNode = (Fl_Tree_Item*)sib->GetPSData();
		childNode = insert(parentNode, itm->GetName(), parentNode->find_child(sibNode));
	}
	else if (parentNode)
		childNode = add(parentNode, itm->GetName());
	else
		childNode = add(itm->GetName());
	if (childNode)
	{
		childNode->user_data((void*)itm);
		itm->SetPSData((void*)childNode);
		itm->AddRef();
	}
}

void ProjectTreeFltk::RemoveNode(ProjectItem *itm)
{
	Fl_Tree_Item *node = (Fl_Tree_Item*)itm->GetPSData();
	if (node)
	{
		ClearNode(node);
		Fl_Tree_Item *parent = node->parent();
		if (parent)
			parent->remove_child(node);
	}
}

void ProjectTreeFltk::UpdateNode(ProjectItem *itm)
{
	if (!itm)
		return;

	Fl_Tree_Item *node = (Fl_Tree_Item*)itm->GetPSData();
	if (node)
		node->label(itm->GetName());
}

void ProjectTreeFltk::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
	if (!itm)
		return;

	itm->AddRef();
	RemoveNode(itm);
	AddNode(itm, prev);
	itm->Release();
}

ProjectItem *ProjectTreeFltk::FirstChild(ProjectItem *itm)
{
	if (!itm)
		return 0;

	Fl_Tree_Item *node = (Fl_Tree_Item*)itm->GetPSData();
	if (!node)
		return 0;

	if (node->children() < 1)
		return 0;

	Fl_Tree_Item *ch = node->child(0);
	if (!ch)
		return 0;

	return (ProjectItem*)ch->user_data();
}

ProjectItem *ProjectTreeFltk::NextSibling(ProjectItem *itm)
{
	if (!itm)
		return 0;

	Fl_Tree_Item *node = (Fl_Tree_Item*)itm->GetPSData();
	if (!node)
		return 0;

	Fl_Tree_Item *sib = node->next();
	if (!sib)
		return 0;

	if (sib->parent() != node->parent())
		return 0;
	
	return (ProjectItem*)sib->user_data();
}

//////////////////////////////////////////////////////////////////////////////
#else
//////////////////////////////////////////////////////////////////////////////

static void treeCB(Fl_Widget *w, void *v)
{
	Fl_Select_Browser* b = (Fl_Select_Browser*)w;
	int index = b->value();
	if (index)
		selItm = (ProjectItem*)b->data(index);
	else
		selItm = NULL;
	Fl::awake(DoItemSelect, (void*)selItm);
}

int ProjectTreeFltk::CheckButton(int e)
{
	int r = 0;
	if (e == FL_PUSH)
	{
		if (Fl::event_button() == FL_RIGHT_MOUSE)
			r = 1;
	}
	else if (e == FL_RELEASE)
	{
		if (Fl::event_button() == FL_RIGHT_MOUSE)
		{
			int index = lineno(find_item(Fl::event_y()));
			Fl::awake(DoContextMenu, (void*)data(index));
			r = 1;
		}
		else if (Fl::event_button() == FL_LEFT_MOUSE && Fl::event_clicks() > 0)
		{
			Fl::awake(DoDoubleClick, (void*)selItm);
			r = 1;
		}
	}
	return r;
}

int ProjectTreeFltk::handle(int e)
{
	return Fl_Hold_Browser::handle(e) | CheckButton(e);
}

ProjectTreeFltk::ProjectTreeFltk(int X, int Y, int W, int H)
: Fl_Hold_Browser(X, Y, W, H, 0)
{
	prjTree = static_cast<ProjectTree*>(this);
	callback(treeCB);
}

ProjectTreeFltk::~ProjectTreeFltk()
{
	prjTree = 0;
}

void ProjectTreeFltk::RemoveAll()
{
	int count = size();
	int index;
	for (index = count; index > 0; index--)
	{
		ProjectItem *itm = (ProjectItem *)data(index);
		data(index, 0);
		itm->Release();
	}
	clear();
}

int ProjectTreeFltk::FindItem(ProjectItem *itm)
{
	int count = size();
	int index;
	for (index = 1; index <= count; index++)
	{
		if (data(index) == (void*)itm)
			return index;
	}
	return 0;
}

ProjectItem *ProjectTreeFltk::GetSelectedNode()
{
	int count = size();
	int index;
	for (index = 1; index <= count; index++)
	{
		if (selected(index))
			return (ProjectItem *)data(index);
	}
	return 0;
}

void ProjectTreeFltk::SelectNode(ProjectItem *itm)
{
	int index = FindItem(itm);
	value(index);
}

void ProjectTreeFltk::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (!itm)
		return;

	bsString str;
	ProjectItem *parent = itm->GetParent();
	ProjectItem *p2 = parent;
	while (p2)
	{
		str += "  ";
		p2 = p2->GetParent();
	}
	if (!itm->IsLeaf())
		str += '+';
	else
		str += ' ';
	str += itm->GetName();

	int count = size();
	int index = 0;
	if (sib)
		index = FindItem(sib) + 1;
	else if (parent)
	{
		// find the last child of the parent
		index = FindItem(parent);
		if (index)
		{
			while (++index <= count)
			{
				sib = (ProjectItem *) data(index);
				if (sib->GetParent() != parent)
					break;
			}
		}
		else
			index = 1;
	}
	else
		index = size()+1;

	insert(index, str, (void*)itm);
	itm->AddRef();
}

void ProjectTreeFltk::RemoveNode(ProjectItem *itm)
{
	if (!itm)
		return;
	int index = FindItem(itm);
	if (index)
	{
		remove(index);
		itm->Release();
	}
}

void ProjectTreeFltk::UpdateNode(ProjectItem *itm)
{
	if (!itm)
		return;

	int index = FindItem(itm);
	if (index)
	{
		bsString str;
		ProjectItem *parent = itm->GetParent();
		while (parent)
		{
			str += "  ";
			parent = parent->GetParent();
		}
		if (!itm->IsLeaf())
			str += '+';
		else
			str += ' ';
		str += itm->GetName();
		text(index, str);
	}
}

void ProjectTreeFltk::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
	if (!itm)
		return;

	itm->AddRef();
	RemoveNode(itm);
	AddNode(itm, prev);
	itm->Release();
}

ProjectItem *ProjectTreeFltk::FirstChild(ProjectItem *itm)
{
	if (!itm)
		return 0;

	int index = FindItem(itm);
	if (index)
	{
		ProjectItem *ch = (ProjectItem *)data(index+1);
		if (ch && ch->GetParent() == itm)
			return ch;
	}
	return 0;
}

ProjectItem *ProjectTreeFltk::NextSibling(ProjectItem *itm)
{
	if (!itm)
		return 0;

	int index = FindItem(itm);
	ProjectItem *sib = (ProjectItem *)data(index+1);
	if (sib && sib->GetParent() == itm->GetParent())
		return sib;
	return 0;
}

#endif
