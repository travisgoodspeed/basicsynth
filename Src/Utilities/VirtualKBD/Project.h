/////////////////////////////////////////////////////////////////////////////
// Project file load class.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////////////////////
#ifndef _PROJECT_H_
#define _PROJECT_H_
#pragma once

class ProjectFileList : 
	public SynthList<ProjectFileList>
{
public:
	char *str;

	ProjectFileList() {	str = 0; }
	~ProjectFileList() { delete str; }
};

class SynthProject
{
public:
	ProjectFileList *libPath;
	long sampleRate;
	long wtSize;
	long wtUser;

	SynthProject()
	{
		sampleRate = 44100;
		wtSize = 16384;
		wtUser = 0;
		libPath = 0;
	}

	~SynthProject()
	{
		ProjectFileList *p;
		while ((p = libPath) != NULL)
		{
			libPath = p->Remove();
			delete p;
		}
	}

	int LoadSynth(XmlSynthElem *root, InstrManager& mgr);
	int LoadProject(char *prjFname, InstrManager& mgr);
	int FindOnPath(bsString& fullPath, char *fname);
};

#endif
