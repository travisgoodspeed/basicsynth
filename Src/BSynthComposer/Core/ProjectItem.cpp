//////////////////////////////////////////////////////////////////////
// BasicSynth - Base class for project items.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"

int ProjectItem::RemoveItem()
{
/*	bsString prompt;
	prompt = "Remove item '";
	prompt += name;
	prompt += "'?";
	if (prjFrame->Verify(prompt, 0))
	{
		theProject->SetChange(1);
		return 1;
	}
	return 0;*/
	return 1;
}

int ProjectItem::CloseItem()
{
	if (editor)
	{
		if (editor->IsChanged())
		{
			bsString prompt;
			prompt = name;
			prompt += " has changed. Save changes?";
			int res = prjFrame->Verify(prompt, "Wait...");
			if (res < 0)
				return 0;
			if (res > 0)
				SaveItem();
		}
		prjFrame->CloseEditor(this);
	}
	return 1;
}

int ProjectItem::Load(XmlSynthElem *node)
{
	char *p = 0;
	if (node->GetAttribute("name", &p) == 0)
		name.Attach(p);
	if (node->GetAttribute("desc", &p) == 0)
		desc.Attach(p);
	return 0;
}

int ProjectItem::Save(XmlSynthElem *node)
{
	if (name.Length() > 0)
		node->SetAttribute("name", name);
	if (desc.Length() > 0)
		node->SetAttribute("desc", desc);
	return 0;
}

WidgetForm *ProjectItem::CreateForm(int xo, int yo)
{
	WidgetForm *wf = new WidgetForm;
	wf->MoveTo(xo, yo);
	return wf;
}

const char *ProjectItem::GetFileSpec(int type)
{
	switch(type)
	{
	case PRJNODE_PROJECT:
		return "Project Files|*.bsprj|Xml Files|*.xml|";
	case PRJNODE_WAVEOUT:
	case PRJNODE_WVFLIST:
	case PRJNODE_WVFILE:
		return "Wave Files|*.wav|";
	case PRJNODE_NOTELIST:
	case PRJNODE_NOTEFILE:
		return "Notelist Files|*.nl|";
	case PRJNODE_TEXTLIST:
	case PRJNODE_TEXTFILE:
		return "Text Files|*.txt|";
	case PRJNODE_SEQLIST:
	case PRJNODE_SEQFILE:
		return "Sequencer Files|*.seq|";
	case PJRNODE_SCRIPTLIST:
	case PRJNODE_SCRIPT:
		return "Script Files|*.js|";
	case PRJNODE_LIBLIST:
	case PRJNODE_LIB:
		return "Library Files|*.bslib|";
	case PRJNODE_SOUNDBANK:
	case PRJNODE_SBLIST:
		return "SoundFont Files|*.sf2|";
	case PRJNODE_MIDICTRL:
	case PRJNODE_MIDIFILE:
		return "MIDI Files|*.mid|";
	}
	return "All Files|*.*|";
}

const char *ProjectItem::GetFileExt(int type)
{
	switch(type)
	{
	case PRJNODE_PROJECT:
		return "bsprj";
	case PRJNODE_WAVEOUT:
	case PRJNODE_WVFILE:
		return "wav";
	case PRJNODE_NOTELIST:
	case PRJNODE_NOTEFILE:
		return "nl";
	case PRJNODE_TEXTLIST:
	case PRJNODE_TEXTFILE:
		return "txt";
	case PRJNODE_SEQLIST:
	case PRJNODE_SEQFILE:
		return "seq";
	case PJRNODE_SCRIPTLIST:
	case PRJNODE_SCRIPT:
		return "js";
	case PRJNODE_LIBLIST:
	case PRJNODE_LIB:
		return "bslib";
	case PRJNODE_SOUNDBANK:
	case PRJNODE_SBLIST:
		return "sf2";
	case PRJNODE_MIDICTRL:
	case PRJNODE_MIDIFILE:
		return "mid";
	}
	return "";
}
