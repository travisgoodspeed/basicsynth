//////////////////////////////////////////////////////////////////////
// BasicSynth - Project frame implements much of the functionality
//              of the main window.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

ProjectFrame::ProjectFrame()
{
}

ProjectFrame::~ProjectFrame()
{
}

////////////////// Project Tree Item Functions /////////////////////////////////

void ProjectFrame::EditItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm != NULL)
		OpenEditor(itm);
}

void ProjectFrame::SaveItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->SaveItem();
}

void ProjectFrame::CloseItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->CloseItem();
}

void ProjectFrame::NewItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->NewItem();
}

void ProjectFrame::AddItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->AddItem();
}

void ProjectFrame::CopyItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
		itm->CopyItem();
}

void ProjectFrame::RemoveItem()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if(itm)
	{
		bsString prompt;
		prompt = "Remove item '";
		prompt += itm->GetName();
		prompt += "'?";
		if (prjFrame->Verify(prompt, "Verify..."))
		{
			if (itm->RemoveItem())
			{
				CloseEditor(itm);
				prjTree->RemoveNode(itm);
				theProject->SetChange(1);
			}
		}
	}
}

void ProjectFrame::ItemProperties()
{
	ProjectItem *itm = prjTree->GetSelectedNode();
	if (itm)
	{
		if (itm->ItemProperties())
			prjTree->UpdateNode(itm);
	}
}

////////////////// Edit Functions /////////////////////////////////

void ProjectFrame::EditUndo()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Undo();
}


void ProjectFrame::EditRedo()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Redo();
}


void ProjectFrame::EditCopy()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Copy();
}


void ProjectFrame::EditCut()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Cut();
}

void ProjectFrame::EditPaste()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Paste();
}

void ProjectFrame::EditFind()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->Find();
}

void ProjectFrame::EditFindNext()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->FindNext();
}

void ProjectFrame::EditSelectAll()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->SelectAll();
}

void ProjectFrame::EditGoto()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->GotoLine(-1);
}

void ProjectFrame::MarkerSet()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->SetMarker();
}

void ProjectFrame::MarkerNext()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->NextMarker();
}

void ProjectFrame::MarkerPrev()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->PrevMarker();
}

void ProjectFrame::MarkerClear()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		vw->ClearMarkers();
}

int ProjectFrame::SaveFile()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
	{
		ProjectItem *itm = vw->GetItem();
		if (itm)
			return itm->SaveItem();
	}
	return 0;
}

int ProjectFrame::CloseFile()
{
	EditorView *vw = GetActiveEditor();
	if (vw)
		return CloseEditor(vw->GetItem());
	return 0;
}

////////////////// Project Functions //////////////////////////////
int ProjectFrame::QuerySaveProject()
{
	int res = Verify("Project has unsaved changes. Save?", "Wait...");
	if (res == 1)
		return SaveProject();
	return res;
}

int ProjectFrame::NewProject()
{
	if (!CloseProject(1))
		return 0;

	const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
	bsString path;
	if (!BrowseFile(0, path, spc, ext))
		return 0;

	theProject = new SynthProject;
	if (!theProject)
		return 0;

	theProject->AddRef();
	int rv = theProject->NewProject(path);
	InitPlayer();
	return rv;
}

int ProjectFrame::OpenProject(const char *fname)
{
	bsString file;
	if (fname == 0)
	{
		const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
		const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
		if (!BrowseFile(1, file, spc, ext))
			return 0;
		fname = file;
	}

	if (!CloseProject(1))
		return 0;

	theProject = new SynthProject;
	if (!theProject)
		return 0;

	theProject->AddRef();
	if (theProject->LoadProject(fname))
	{
		bsString msg;
		msg = "Could not load project: ";
		msg += theProject->WhatHappened();
		Alert(msg, "Ooops...");
		prjTree->RemoveAll();
		theProject->Release();
		theProject = 0;
		return 0;
	}
	InitPlayer();
	return 1;
}

int ProjectFrame::CloseProject(int query)
{
	if (theProject)
	{
		if (query && theProject->GetChange())
		{
			if (QuerySaveProject() < 0)
				return 0;
		}

		if (CloseAllEditors() == 0)
			return 0;
		StopPlayer();
		ClearPlayer();
		prjTree->RemoveAll();
		theProject->Release();
		theProject = 0;
	}
	return 1;
}

int ProjectFrame::SaveProject()
{
	if (!theProject)
		return 0;

	bsString path;
	theProject->GetProjectPath(path);
	if (path.Length() == 0)
		return SaveProjectAs();
	SaveBackup();
	if (!SaveAllEditors(0))
		return -1;
	if (theProject->SaveProject())
	{
		bsString msg;
		msg = "Could not save project: ";
		msg += theProject->WhatHappened();
		Alert(msg, "Ooops...");
		return -1;
	}
	return 0;
}

void ProjectFrame::SaveBackup()
{
	if (!theProject)
		return;

	bsString path;
	theProject->GetProjectPath(path);
	if (SynthFileExists(path))
	{
		bsString bak;
		bak = path;
		bak += ".bak";
		SynthCopyFile(path, bak);
	}
}

int ProjectFrame::SaveProjectAs()
{
	if (!theProject)
		return -1;

	bsString path;
	const char *spc = ProjectItem::GetFileSpec(PRJNODE_PROJECT);
	const char *ext = ProjectItem::GetFileExt(PRJNODE_PROJECT);
	if (!BrowseFile(0, path, spc, ext))
		return -1;
	if (theProject->SaveProject(path))
	{
		bsString msg;
		msg = "Could not save project: ";
		msg += theProject->WhatHappened();
		Alert(msg, "Ooops...");
		return -1;
	}
	bsString oldDir;
	bsString newDir;
	theProject->GetProjectDir(oldDir);
	theProject->SetProjectPath(path);
	theProject->GetProjectDir(newDir);
	if (oldDir.Length() > 0 && oldDir.Compare(newDir) != 0)
		theProject->CopyFiles(oldDir, newDir);
	return 0;
}

void ProjectFrame::GenerateStarted()
{
}

void ProjectFrame::GenerateFinished()
{
}
