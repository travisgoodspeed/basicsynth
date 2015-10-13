#ifndef PROJECT_TREE_FLU_H
#define PROJECT_TREE_FLU_H

class ProjectTreeFlu : 
	public Flu_Tree_Browser,
	public ProjectTree
{
private:
public:
	ProjectTreeFlu(int w, int h);
	virtual ~ProjectTreeFlu();

	void DeleteAll();
	ProjectItem *GetSelection();
	int FindItem(ProjectItem *itm);

	virtual void AddNode(ProjectItem *itm, ProjectItem *sib = 0);
	virtual void RemoveNode(ProjectItem *itm);
	virtual void UpdateNode(ProjectItem *itm);
	virtual void MoveNode(ProjectItem *itm, ProjectItem *prev);
	virtual ProjectItem *FirstChild(ProjectItem *itm);
	virtual ProjectItem *NextSibling(ProjectItem *itm);
	virtual int ItemEnabled(int id);
};

#endif
