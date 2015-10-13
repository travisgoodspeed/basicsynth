//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents a file.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

int FileItem::ItemActions()
{
	int actEnable = actions;
	if (editor)
		actEnable |= ITM_ENABLE_CLOSE | ITM_ENABLE_SAVE;
	return actEnable;
}

int FileItem::EditItem()
{
	if (actions & ITM_ENABLE_EDIT)
	{
		TextEditor *ed = prjFrame->CreateTextEditor(this);
		if (ed)
		{
			if (SynthProject::FullPath(file))
				fullPath = file;
			else
				theProject->FindOnPath(fullPath, file);
			ed->OpenFile(fullPath);
		}
		return 1; 
	}
	return 0;
}

int FileItem::SaveItem() 
{ 
	if (editor)
	{
		TextEditor *te = (TextEditor*)editor;
		te->SaveFile(fullPath);
	}
	return 0; 
}

int FileItem::ItemProperties()
{
	int ok = 0;
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		ok = pb->Activate(1);
		delete pb;
	}
	return ok;
}

int FileItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_NAME, name, 0);
	pb->SetValue(PROP_DESC, desc, 0);
	pb->SetValue(PROP_FILE, file, 0);
	pb->SetState(PROP_INCL, useThis);
	return 1;
}

int FileItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_NAME, name);
	pb->GetValue(PROP_DESC, desc);
	pb->GetValue(PROP_FILE, file);
	pb->GetState(PROP_INCL, useThis);
	if (SynthProject::FullPath(file))
		fullPath = file;
	else
		theProject->FindOnPath(fullPath, file);
	return 1;
}

int FileItem::Load(XmlSynthElem *node)
{
	ProjectItem::Load(node);
	node->GetAttribute("inc", useThis);
	char *content = 0;
	node->GetContent(&content);
	if (content)
	{
		SynthProject::NormalizePath(content);
		file.Attach(content);
	}
	return 0;
}

int FileItem::Save(XmlSynthElem *node)
{
	ProjectItem::Save(node);
	node->SetAttribute("inc", useThis);
	node->SetContent(file);
	return 0;
}

int FileItem::CopyFile(const char *srcDir, const char *dstDir)
{
	if (theProject->FullPath(file))
		return 0;
	bsString op;
	bsString np;
	op = srcDir;
	op += "/";
	op += file;
	np = dstDir;
	np += "/";
	np += file;
	if (SynthFileExists(op))
		return SynthCopyFile(op, np);
	return 0;
}

int FileItem::CreateNew()
{
	if (fullPath.Length() == 0)
		return -1;
	if (SynthFileExists(fullPath))
		return 0;
	return SynthCreateFile(fullPath, NULL, 0);
}

///////////////////////////////////////////////////////////////

int FileList::Load(XmlSynthElem *node)
{
	XmlSynthElem *child = node->FirstChild();
	while (child)
	{
		if (child->TagMatch(xmlChild))
		{
			FileItem *fi = NewChild();
			fi->SetParent(this);
			fi->Load(child);
			prjTree->AddNode(fi);
		}
		XmlSynthElem *sib = child->NextSibling();
		delete child;
		child = sib;
	}
	return 0;
}

int FileList::Save(XmlSynthElem *node)
{
	int err = 0;
	XmlSynthElem *child;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		child = node->AddChild(xmlChild);
		if (child)
		{
			err |= pi->Save(child);
			delete child;
		}
		pi = prjTree->NextSibling(pi);
	}
	return err;
}

FileItem* FileList::NewAdd(const char *file)
{
	FileItem *ni = NewChild();
	if (!ni)
		return 0;
	ni->SetParent(this);
	if (file)
	{
		const char *sl = strrchr(file, '/');
		if (sl)
			ni->SetName(sl+1);
		else
			ni->SetName(file);
		ni->SetFile(file);
	}
	else
		ni->SetName(xmlChild);
	prjTree->AddNode(ni);
	theProject->SetChange(1);
	if (ni->ItemActions() & ITM_ENABLE_PROPS)
	{
		if (ni->ItemProperties())
		{
			if (file == NULL)
				ni->CreateNew();
			prjTree->UpdateNode(ni);
		}
	}
	if (ni->ItemActions() & ITM_ENABLE_EDIT)
		ni->EditItem();
	return ni;
}

int FileList::NewItem()
{
	return NewAdd(0) != 0;
}

int FileList::AddItem()
{ 
	bsString newFile;
	if (prjFrame->BrowseFile(1, newFile, GetFileSpec(type), GetFileExt(type)))
		return NewAdd(newFile) != 0;
	return 0; 
}

int FileList::SaveItem()
{
	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		err |= pi->SaveItem();
		pi = prjTree->NextSibling(pi);
	}
	return err;
}

int FileList::CopyFiles(const char *srcDir, const char *dstDir)
{
	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		err |= ((FileItem*)pi)->CopyFile(srcDir, dstDir);
		pi = prjTree->NextSibling(pi);
	}
	return err;
}

int FileList::ItemProperties()
{
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		pb->Activate(1);
		delete pb;
	}
	return 1;
}
