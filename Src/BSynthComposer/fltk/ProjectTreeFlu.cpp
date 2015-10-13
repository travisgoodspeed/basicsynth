#include "globinc.h"
#include "MainFrm.h"

static void tvCB(Fl_Widget *w, void *v)
{
	ProjectItem *selItm = 0;
	Flu_Tree_Browser* b = (Flu_Tree_Browser*)w;
	Flu_Tree_Browser::Node *nd = b->callback_node();
	ProjectItem *itm = (ProjectItem *)nd->user_data();
	int why = b->callback_reason();
	if (why == FLU_SELECTED)
	{
		printf("in tvCB SELECTED\n");
		mainWnd->ItemSelected(itm);
	}
	else if (why == FLU_DOUBLE_CLICK)
	{
		printf("in tvCB DOUBLE_CLICK\n");
		mainWnd->ItemDoubleClick(itm);
	}
}

ProjectTreeFlu::ProjectTreeFlu(int w, int h)
: Flu_Tree_Browser(0, 30, w, h, "tree")
{
	prjTree = static_cast<ProjectTree*>(this);
	callback(tvCB);
	selection_mode(FLU_SINGLE_SELECT);
}

ProjectTreeFlu::~ProjectTreeFlu()
{
	prjTree = 0;
}

void ProjectTreeFlu::DeleteAll()
{
	// TODO: enumerate the tree depth-first and delete user_data
	clear();
}

ProjectItem *ProjectTreeFlu::GetSelection()
{
	Node *nd = get_selected(0);
	if (nd)
		return (ProjectItem*)nd->user_data();
	return 0;
}

void ProjectTreeFlu::AddNode(ProjectItem *itm, ProjectItem *sib)
{
	if (!itm)
		return;

	Node *nd;
	ProjectItem *parent = itm->GetParent();
	if (parent)
	{
		nd = (Node *)parent->GetPSData();
		if (nd)
		{
			if (itm->IsLeaf())
				nd = nd->add_leaf(itm->GetName());
			else
				nd = nd->add_branch(itm->GetName());
		}
	}
	else
		nd = set_root(itm->GetName());
	if (nd)
	{
		nd->user_data((void*)itm);
		itm->SetPSData((void*)nd);
	}
}

void ProjectTreeFlu::RemoveNode(ProjectItem *itm)
{
	if (!itm)
		return;
	Node *nd = (Node *)itm->GetPSData();
	itm->SetPSData(0);
	nd->user_data(0);
	remove(nd);
}

void ProjectTreeFlu::UpdateNode(ProjectItem *itm)
{
	if (!itm)
		return;

	Node *nd = (Node *) itm->GetPSData();
	nd->label(itm->GetName());
}

void ProjectTreeFlu::MoveNode(ProjectItem *itm, ProjectItem *prev)
{
}

ProjectItem *ProjectTreeFlu::FirstChild(ProjectItem *itm)
{
	if (!itm)
		return 0;

	Node *nd = (Node *) itm->GetPSData();
	Node *ch = nd->first_leaf();
	if (ch)
		return (ProjectItem *)ch->user_data();
	return 0;
}

ProjectItem *ProjectTreeFlu::NextSibling(ProjectItem *itm)
{
	if (!itm)
		return 0;

	Node *nd = (Node *) itm->GetPSData();
	Node *ch = nd->next();
	if (ch)
		return (ProjectItem *)ch->user_data();
	return 0;
}

int ProjectTreeFlu::ItemEnabled(int id)
{
	return 1;
}
