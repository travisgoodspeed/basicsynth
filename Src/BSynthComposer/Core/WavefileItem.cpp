//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item to represent wave files used by the WFSynth instrument.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

int WavefileItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_NAME, name, 0);
	pb->SetValue(PROP_DESC, desc, 0);
	pb->SetValue(PROP_FILE, file, 0);
	pb->SetValue(PROP_WVID, (long)wvid, 0);
//	pb->SetValue(PROP_LPST, loopStart, 0);
//	pb->SetValue(PROP_LPEND, loopEnd, 0);
	return 1;
}

int WavefileItem::SaveProperties(PropertyBox *pb)
{
	bsString newFile;
	pb->GetValue(PROP_FILE, newFile);

	WaveFileIn *wf = 0;

	long id = 0;
	pb->GetValue(PROP_WVID, id);
	if ((bsInt16)id != wvid)
	{
		wf = FindFile((bsInt16)id);
		if (wf != 0)
		{
			char buf[80];
			snprintf(buf, 80, "ID #%d is already in use, please choose another ID.", (bsInt16)id);
			prjFrame->Alert(buf, 0);
			return 0;
		}
	}

	if (loaded)
		wf = FindFile(wvid);

	int fileChanged = 0;
	if (strcmp(file, newFile) != 0)
		fileChanged = 1;
	else if (wf)
	{
		const char *wfname = wf->GetFilename();
		if (!wfname || strcmp(wfname, newFile) != 0)
			fileChanged = 1;
	}

	if (fileChanged)
	{
		int err = 0;
		if (wf)
			err = wf->LoadWaveFile(newFile, (bsInt16)id);
		else
			err = WFSynth::AddToCache(newFile, (bsInt16)id);
		if (err < 0)
		{
			if (!prjFrame->Verify("The file could not be loaded! Use this file name anyway?", "Ooops..."))
				return 0;
		}
		else
			loaded = 1;
		file = newFile;
	}
	else if (wf)
		wf->SetFileID((bsInt16)id);
	wvid = id;
	pb->GetValue(PROP_NAME, name);
	pb->GetValue(PROP_DESC, desc);
//	pb->GetValue(PROP_LPST, loopStart);
//	pb->GetValue(PROP_LPEND, loopEnd);
//	if (wf)
//		wf->SetLoopPoints(loopStart, loopEnd);
	return 1;
}

WaveFileIn *WavefileItem::FindFile(bsInt16 id)
{
	int count = WFSynth::GetCacheCount();
	for (int ndx = 0; ndx < count; ndx++)
	{
		WaveFileIn *wf = WFSynth::GetCacheEntry(ndx);
		if (wf->GetFileID() == id)
			return wf;
	}
	return 0;
}

int WavefileItem::LoadFile()
{
	loaded = 0;
	int err;
	WaveFileIn *wf = FindFile(wvid);
	if (wf)
	{
		err = wf->LoadWaveFile(file, wvid);
		if (err >= 0)
			loaded = 1;
		return err;
	}

	err = WFSynth::AddToCache(file, wvid);
	if (err >= 0)
	{
		loaded = 1;
		wf = FindFile(wvid);
		if (loopEnd == 0)
			loopEnd = wf->GetInputLength();
		wf->SetLoopPoints(loopStart, loopEnd);
	}
	return err;
}

int WavefileItem::Load(XmlSynthElem *node)
{
	FileItem::Load(node);
	node->GetAttribute("id", wvid);
	if (name.Length() == 0)
	{
		//char buf[80];
		//snprintf(buf, 80, "ID #%d", wvid);
		//name = buf;
		name = "ID #";
		name += (long)wvid;
	}
	node->GetAttribute("lpst", loopStart);
	node->GetAttribute("lpend", loopEnd);
	return 0;
}

int WavefileItem::Save(XmlSynthElem *node)
{
	node->SetAttribute("id", wvid);
	node->SetAttribute("lpst", loopStart);
	node->SetAttribute("lpend", loopEnd);
	return FileItem::Save(node);
	return 0;
}

/////////////////////////////////////////////////////////////////////////

int WavefileList::AddItem()
{
	WavefileItem *itm = new WavefileItem;
	itm->SetParent(this);
	itm->SetName("*New Wavefile");
	prjTree->AddNode(itm);
	if (!itm->ItemProperties())
	{
		prjTree->RemoveNode(itm);
	}
	else
	{
		prjTree->UpdateNode(itm);
		theProject->SetChange(1);
	}
	return 1;
}

int WavefileList::EditItem()
{
	WFSynth::ClearCache();
	LoadFiles();
	return 1;
}

int WavefileList::LoadFiles()
{
	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVFILE)
		{
			WavefileItem *wi = (WavefileItem *)pi;
			err |= wi->LoadFile();
		}
		pi = prjTree->NextSibling(pi);
	}
	return err;
}

int WavefileList::Load(XmlSynthElem *node)
{
	WavefileItem *wi = new WavefileItem;
	wi->SetParent(this);
	int err = wi->Load(node);
	prjTree->AddNode(wi);
	return err;
}

int WavefileList::Save(XmlSynthElem *node)
{
	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_WVFILE)
		{
			WavefileItem *wi = (WavefileItem *)pi;
			XmlSynthElem *child = node->AddChild("wvfile");
			err |= wi->Save(child);
			delete child;
		}
		pi = prjTree->NextSibling(pi);
	}
	return err;
}



