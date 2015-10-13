/////////////////////////////////////////////////////////////////////////////
// Extract instrument definitions from BSynth project file.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Project.h"

int SynthProject::LoadSynth(XmlSynthElem *root, InstrManager& mgr)
{
	bsString fullPath;
	char *fname;
	int errcnt = 0;
	XmlSynthElem *child = root->FirstChild();
	XmlSynthElem *sib;

	while (child != NULL)
	{
		if (child->TagMatch("synth"))
		{
			child->GetAttribute("sr", sampleRate);
			child->GetAttribute("wt", wtSize);
			child->GetAttribute("usr", wtUser);
			InitSynthesizer((bsInt32)sampleRate, (bsInt32)wtSize, (bsInt32)wtUser);
			XmlSynthElem *wvnode = child->FirstChild();
			while (wvnode)
			{
				if (wvnode->TagMatch("wvtable"))
				{
					if (mgr.LoadWavetable(wvnode))
						errcnt++;
				}
				sib = wvnode->NextSibling();
				delete wvnode;
				wvnode = sib;
			}
		}
		else if (child->TagMatch("wvdir"))
		{
			char *file = 0;
			if (child->GetContent(&file) == 0)
			{
				synthParams.wvPath = file;
				delete file;
			}
		}
		else if (child->TagMatch("wvfile"))
		{
			char *file = 0;
			short id = -1;
			child->GetContent(&file);
			child->GetAttribute("id", id);
			if (file)
			{
				if (WFSynth::AddToCache(file, id) == -1)
					errcnt++;
				delete file;
			}
		}
		else if (child->TagMatch("libpath"))
		{
			ProjectFileList *lib = new ProjectFileList;
			child->GetContent(&lib->str);
			if (libPath)
				libPath->Insert(lib);
			else
				libPath = lib;
		}
		else if (child->TagMatch("libfile"))
		{
			fname = 0;
			child->GetContent(&fname);
			if (fname)
			{
				if (FindOnPath(fullPath, fname))
				{
					if (mgr.LoadInstrLib(fname))
						errcnt++;
				}
				else
					errcnt++;
				delete fname;
			}
		}
		else if (child->TagMatch("instrlib"))
		{
			if (mgr.LoadInstrLib(child))
				errcnt++;
		}
		else if (child->TagMatch("sndbnk") || child->TagMatch("sf2") || child->TagMatch("dls"))
		{
			char *sbpath = 0;
			child->GetContent(&sbpath);
			if (sbpath)
			{
				if (FindOnPath(fullPath, sbpath))
				{
					char *name = 0;
					short preload = 0;
					float normalize = 1;
					child->GetAttribute("name", &name);
					child->GetAttribute("pre", preload);
					if (child->GetAttribute("nrm", normalize))
						normalize = 1.0;
					SoundBank *bnk = 0;
					if (SFFile::IsSF2File(fullPath))
					{
						SFFile sndfile;
						bnk = sndfile.LoadSoundBank(fullPath, preload);
					}
					else if (DLSFile::IsDLSFile(fullPath))
					{
						DLSFile dls;
						bnk = dls.LoadSoundBank(fullPath, preload);
					}
					if (bnk)
					{
						bnk->name = name;
						bnk->Lock();
						SoundBank::SoundBankList.Insert(bnk);
					}
					delete name;
				}
				delete sbpath;
			}
		}

		sib = child->NextSibling();
		delete child;
		child = sib;
	}

	return errcnt;
}

int SynthProject::LoadProject(char *prjFname, InstrManager& mgr)
{
	int errcnt = 0;

	XmlSynthDoc doc;
	XmlSynthElem *root;

	if ((root = doc.Open(prjFname)) == NULL)
		return -1;

	if (root->TagMatch("synthprj"))
	{
		errcnt = LoadSynth(root, mgr);
	}
	else if (root->TagMatch("instrlib"))
	{
		sampleRate = 44100;
		wtSize = 16384;
		wtUser = 0;
		InitSynthesizer(sampleRate, wtSize, wtUser);
		if (mgr.LoadInstrLib(root))
			errcnt++;
	}
	else
		errcnt++;

	delete root;
	doc.Close();

	return errcnt;
}

int SynthProject::FindOnPath(bsString& fullPath, char *fname)
{
	int gotFile = 0;
	fullPath = fname;
	ProjectFileList *libs = libPath;
	while (!(gotFile = SynthFileExists(fullPath)) && libs)
	{
		fullPath = libs->str;
		fullPath += "/";
		fullPath += fname;
		libs = libs->next;
	}
	return gotFile;
}
