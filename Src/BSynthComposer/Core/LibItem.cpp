//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents a library
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

// added is TRUE when we are adding a library after project load.
// This signals we need to add all instruments to the keyboard.
int LibfileItem::LoadLib(int added)
{
	if (loaded)
		return 0;

	XmlSynthDoc doc;
	XmlSynthElem *root = doc.Open((char *)(const char *)file);
	if (!root)
	{
		bsString msg;
		msg = "Cannot open library ";
		msg += name;
		prjFrame->Alert(msg, "Ooops...");
		return -1;
	}

	baseNum = 0;
	root->GetAttribute("baseid", baseNum);

	char *content = 0;
	if (root->GetAttribute("name", &content) == 0)
	{
		name.Attach(content);
		content = 0;
	}
	if (root->GetAttribute("desc", &content) == 0)
		desc.Attach(content);
	XmlSynthElem *next;
	XmlSynthElem *inst = root->FirstChild();
	while (inst != NULL)
	{
		if (inst->TagMatch("instr"))
		{
			InstrConfig *inc = theProject->mgr.LoadInstr(inst);
			if (inc)
			{
				// Possible: don't put the instrument in the global instrument list.
				// This would allow libraries to be loaded for the purpose of
				// copy in/out to the project, but avoid possible name/inum
				// conflicts. This has not been tested, so don't do it yet.
				//if (!useThis)
				//	inc->Remove();
				if (baseNum == 0 || inc->inum < baseNum)
					baseNum = inc->inum;
				InstrItem *iip = new InstrItem;
				if (iip)
				{
					iip->SetParent(this);
					iip->SetConfig(inc);
					iip->SetType(PRJNODE_LIBINSTR);
					iip->SetActions(ITM_ENABLE_PROPS | ITM_ENABLE_EDIT);
					prjTree->AddNode(iip);
				}
				if (added && useThis)
					prjFrame->InstrAdded(inc);
			}
		}
		next = inst->NextSibling();
		delete inst;
		inst = next;
	}
	SetChange(0);
	loaded = 1;
	doc.Close();
	return 0;
}

int LibfileItem::ItemActions()
{
	int actEnable = actions;
	if (GetChange())
		actEnable |= ITM_ENABLE_SAVE;
	return actEnable;
}

// copy from the project
int LibfileItem::AddItem()
{
	return AddCopy(1);
}

// copy to the project
int LibfileItem::CopyItem()
{
	return AddCopy(2);
}


int LibfileItem::RemoveItem()
{
	ProjectItem *instr = 0;
	while ((instr = prjTree->FirstChild(this)) != 0)
	{
		if (instr->GetType() == PRJNODE_INSTR)
			((InstrItem*)instr)->RemoveInstr();
		prjTree->RemoveNode(instr);
	}
	return 1;
}

int LibfileItem::SaveItem()
{
	XmlSynthDoc doc;
	XmlSynthElem *root;
	if ((root = doc.NewDoc("instrlib")) == NULL)
	{
		prjFrame->Alert("Cannot create the library doc.", "Ooops...");
		return 0;
	}

	root->SetAttribute("name", name);
	root->SetAttribute("desc", desc);
	root->SetAttribute("baseid", baseNum);

	ProjectItem *instr = prjTree->FirstChild(this);
	while (instr)
	{
		XmlSynthElem *child = root->AddChild("instr");
		instr->Save(child);
		delete child;
		instr = prjTree->NextSibling(instr);
	}
	delete root;

	int ret = doc.Save((char*)(const char*)file);
	doc.Close();
	if (ret != 0)
	{
		prjFrame->Alert("Error writing the library file.", "Ooops...");
		return 0;
	}
	SetChange(0);
	return 1;
}

int LibfileItem::ItemProperties()
{
	propFn = 0;
	return FileItem::ItemProperties();
}

int LibfileItem::AddCopy(int f)
{
	propFn = f;
	int ok = 0;
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, PRJNODE_SELINSTR);
	if (pb)
	{
		ok = pb->Activate(1);
		delete pb;
		if (GetChange())
			SaveItem();
	}
	return ok;
}

int LibfileItem::LoadProperties(PropertyBox *pb)
{
	if (propFn == 1) // copy from the project
	{
		pb->ListChildren(PROP_ILST, theProject->instrInfo);
		pb->SetValue(PROP_INUM, (long)NextInum(), 0);
	}
	else if (propFn == 2) // copy to the project
	{
		pb->ListChildren(PROP_ILST, this);
		pb->SetValue(PROP_INUM, (long)InstrList::NextInum(), 0);
	}
	else
	{
		FileItem::LoadProperties(pb);
		pb->SetValue(PROP_INUM, (long)baseNum, "Start id");
		if (prjTree->FirstChild(this) == 0)
			pb->EnableValue(PROP_REN, 0);
	}
	return 1;
}

int LibfileItem::SaveProperties(PropertyBox *pb)
{
	InstrConfig *inc;
	InstrConfig *inc2;
	if (propFn == 1 || propFn == 2)
	{
		long inum = -1;
		InstrItem *itm = 0;
		pb->GetSelection(PROP_ILST, (void**)&itm);
		if (!itm)
		{
			prjFrame->Alert("You must select an instrument!", "Wait...");
			return 0;
		}
		pb->GetValue(PROP_INUM, inum);
		inc = itm->GetConfig();
		InstrItem *iip = new InstrItem;
		if (iip)
		{
			InstrMapEntry *instType = inc->instrType;
			Opaque tp = 0;
			if (instType->manufTmplt)
				tp = instType->manufTmplt(0);
			else if (instType->manufInstr)
				tp = (Opaque) instType->manufInstr(0, inc->instrTmplt);
			// It's legit to create an instrument without a template.
			// When instantiated, you will get a "blank" instrument.
			// NB: the editors may not correctly edit such an entity!
			inc2 = theProject->mgr.AddInstrument(inum, instType, tp);
			if (!inc2)
			{
				if (instType->dumpTmplt)
					instType->dumpTmplt(tp);
				prjFrame->Alert("Could not create new instrument instance", "Ooops...");
				return 0;
			}
			if (propFn == 2)
			{
				iip->SetParent(theProject->instrInfo);
				inc2->SetName(inc->GetName());
				theProject->SetChange(1);
			}
			else
			{
				bsString inm;
				inm = name;
				inm += inc->GetName();
				inc2->SetName(inm);
				iip->SetParent(this);
				iip->SetType(PRJNODE_LIBINSTR);
				iip->SetActions(ITM_ENABLE_PROPS | ITM_ENABLE_EDIT);
				SetChange(1);
			}
			inc2->SetDesc(inc->GetDesc());
			iip->SetConfig(inc2);
			prjTree->AddNode(iip);
		}
	}
	else
	{
		long start = 0;
		pb->GetValue(PROP_INUM, start);
		baseNum = (bsInt16) start;
		FileItem::SaveProperties(pb);
		short renumber = 0;
		pb->GetState(PROP_REN, renumber);
		if (renumber)
		{
			if (start == 0)
			{
				// Maybe forgot to enter base number?
				if (prjFrame->Verify("Renumber from 0?", "Wait...") != 1)
					return 0;
			}
			InstrItem *itm = (InstrItem *) prjTree->FirstChild(this);
			while (itm)
			{
				inc = itm->GetConfig();
				inc->inum = start++;
				itm = (InstrItem *) prjTree->NextSibling(itm);
			}
			SetChange(1);
		}
	}
	return 1;
}

int LibfileItem::NextInum()
{
	int inum = baseNum;
	InstrItem *itm = (InstrItem *)prjTree->FirstChild(this);
	while (itm)
	{
		InstrConfig *ic = itm->GetConfig();
		if (ic && ic->inum >= inum)
			inum = ic->inum+1;
		itm = (InstrItem *)prjTree->NextSibling(itm);
	}
	return inum;
}

//////////////////////////////////////////////////////////////////

int LibfileList::LoadLibs()
{
	LibfileItem *itm = (LibfileItem*)prjTree->FirstChild(this);
	while (itm)
	{
		if (itm->GetType() == PRJNODE_LIB)
		{
			itm->LoadLib(0);
			prjTree->UpdateNode(itm);
		}
		itm = (LibfileItem*) prjTree->NextSibling(itm);
	}

	return 0;
}

int LibfileList::AddItem()
{
	bsString file;
	if (prjFrame->BrowseFile(1, file, GetFileSpec(type), GetFileExt(type)))
	{
		LibfileItem *itm = (LibfileItem*)NewAdd(file);
		if (itm)
		{
			itm->LoadLib(1);
			return 1;
		}
	}
	return 0;
}

int LibfileList::NewItem()
{
	if (NewAdd(0))
		return 1;
	return 0;
}

