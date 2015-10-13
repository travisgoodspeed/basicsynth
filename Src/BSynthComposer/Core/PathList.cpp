//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents the file search path.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

void PathList::AddItem(const char *name)
{
	AddItem(new PathListItem(name));
}

void PathList::AddItem(PathListItem *itm)
{
	tail.InsertBefore(itm);
}

PathListItem* PathList::EnumList(PathListItem *itm)
{
	if (itm == 0)
		itm = &head;
	if (itm->next != &tail)
		return itm->next;
	return 0;
}

void PathList::RemoveItem(const char *name)
{
	PathListItem *itm = head.next;
	while (itm != &tail)
	{
		if (itm->path.CompareNC(name) == 0)
		{
			itm->Remove();
			delete itm;
			break;
		}
	}
}

int PathList::FindOnPath(bsString& fullPath, const char *fname)
{
	if (FullPath(fname))
	{
		fullPath = fname;
		return SynthFileExists(fullPath);
	}

	if (SynthFileExists(fname))
	{
		fullPath = fname;
		return 1;
	}

	theProject->GetProjectDir(fullPath);
	fullPath += '/';
	fullPath += fname;
	if (SynthFileExists(fullPath))
		return 1;

	if (fname[1] == ':')
		fname += 2;

	int gotFile = 0;
	PathListItem *itm = 0;
	while (!gotFile && (itm = EnumList(itm)) != 0)
	{
		fullPath = itm->path;
		fullPath += '/';
		fullPath += fname;
		gotFile = SynthFileExists(fullPath);
	}
	return gotFile;
}

int PathList::ResolvePath(FileItem *fi)
{
	return FindOnPath(fi->PathBuffer(), fi->GetFile());
}

int PathList::FullPath(const char *fname)
{
	if (fname == 0 || *fname == '\0')
		return 0;
	if (fname[1] == ':')
		fname += 2;
	if (*fname == '/' || *fname == '\\')
		return 1;
	return 0;
}

int PathList::Load(XmlSynthElem *node)
{
	char *data = 0;
	if (node->GetContent(&data) == 0)
	{
		AddItem(data);
		delete data;
	}
	return 0;
}

int PathList::Save(XmlSynthElem *node)
{
	XmlSynthElem *child;
	PathListItem *itm = 0;
	while ((itm = EnumList(itm)) != 0)
	{
		child = node->AddChild("libdir");
		child->SetContent(itm->path);
		delete child;
	}
	return 0;
}
