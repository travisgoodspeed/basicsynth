//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item that represents a notelist file.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

void ErrCB::OutputDebug(const char *s)
{
	if (prjGenerate)
		prjGenerate->AddMessage(s);
}

void ErrCB::OutputError(const char *s)
{
	if (prjGenerate)
		prjGenerate->AddMessage(s);
}

void ErrCB::OutputError(nlSyntaxErr *se)
{
	if (itm)
		itm->AddError(se);
	nlErrOut::OutputError(se);
}

void ErrCB::OutputMessage(const char *s)
{
	if (prjGenerate)
		prjGenerate->AddMessage(s);
}

void NotelistItem::AddError(nlSyntaxErr *e)
{
	ScoreError *p = new ScoreError(e);
	if (errLast)
		errLast->Insert(p);
	else
		errFirst = p;
	errLast = p;
}

void NotelistItem::ClearErrors()
{
	while (errFirst)
	{
		errLast = errFirst->Remove();
		delete errFirst;
		errFirst = errLast;
	}
	errLast = 0;
}

int NotelistItem::Convert(nlConverter& cvt)
{
	ErrCB cb;
	cb.itm = this;
	bsString msg;

	ClearErrors();
	if (!useThis)
	{
		msg = "Skipping ";
		msg += name;
		msg += " ...";
		cb.OutputMessage(msg);
		return 0;
	}

	msg = "Converting ";
	msg += name;
	msg += " ...";
	cb.OutputMessage(msg);

	cvt.SetDebugLevel(dbgLevel);
	cvt.SetErrorCallback(&cb);
	int ret = 0;
	if (editor)
	{
		bsString text;
		((TextEditor*)editor)->GetText(text);
		nlLexFileMem lex(text, text.Length());
		ret = cvt.Convert(name, &lex);
	}
	else
	{
		theProject->FindOnPath(fullPath, file);
		nlLexFileIn lf(fullPath);
		ret = cvt.Convert(name, &lf);
	}
	cvt.SetErrorCallback(0);
	return ret;
}

int NotelistItem::Load(XmlSynthElem *node)
{
	FileItem::Load(node);
	node->GetAttribute("dbg", dbgLevel);
	if (name.Length() == 0)
		name = file;
	return 0;
}

int NotelistItem::Save(XmlSynthElem *node)
{
	FileItem::Save(node);
	node->SetAttribute("dbg", dbgLevel);
	return 0;
}

int NotelistItem::CopyItem()
{
	return 0; 
}

int NotelistItem::SyntaxCheck()
{
	ClearErrors();
	ErrCB cb;
	cb.itm = this;
	nlConverter cvt;
	cvt.SetSampleRate(synthParams.sampleRate);
	cvt.SetInstrManager(&theProject->mgr);
	cvt.SetDebugLevel(dbgLevel);
	cvt.SetErrorCallback(&cb);
	int ret = 0;
	if (editor)
	{
		bsString text;
		((TextEditor*)editor)->GetText(text);
		nlLexFileMem lex(text, text.Length());
		ret = cvt.Convert(name, &lex);
	}
	else
	{
		theProject->FindOnPath(fullPath, file);
		nlLexFileIn lf(fullPath);
		ret = cvt.Convert(name, &lf);
	}
	return ret;
}

ScoreError* NotelistItem::EnumErrors(ScoreError *e)
{
	if (e == 0)
		return errFirst;
	return e->next;
}

/////////////////////////////////////////////////////////////////////

int NotelistList::Convert(nlConverter& cvt)
{
	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_NOTEFILE)
		{
			NotelistItem *nl = (NotelistItem *) pi;
			err |= nl->Convert(cvt);
		}
		pi = prjTree->NextSibling(pi);
	}

	return err;
}

int SeqItem::Convert(SequenceFile& seqf)
{
	if (!useThis)
		return 0;
	int ret = 0;
	if (editor)
	{
		bsString text;
		((TextEditor*)editor)->GetText(text);
		ret = seqf.LoadMem(text);
	}
	else
	{
		bsString fullPath;
		theProject->FindOnPath(fullPath, GetFile());
		ret = seqf.LoadFile(fullPath);
	}
	if (ret)
	{
		if (prjGenerate)
		{
			bsString errMsg("Sequence ");
			errMsg += GetName();
			errMsg += ": ";
			bsString errLin;
			seqf.GetError(errLin);
			errMsg += errLin;
			prjGenerate->AddMessage(errMsg);
		}
	}
	return ret;
}

int SeqList::LoadSequences(InstrManager *mgr, Sequencer *seq)
{
	SequenceFile seqf;
	seqf.Init(mgr, seq);

	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_SEQFILE)
		{
			SeqItem *sf = (SeqItem *) pi;
			err |= sf->Convert(seqf);
		}
		pi = prjTree->NextSibling(pi);
	}

	return err;
}

int ScriptList::LoadScripts(nlConverter& cvt)
{
	nlScriptEngine *engine = cvt.GetScriptEngine();
	if (!engine)
		return 0;

	int err = 0;
	ProjectItem *pi = prjTree->FirstChild(this);
	while (pi)
	{
		if (pi->GetType() == PRJNODE_SCRIPT)
		{
			FileItem *sf = (FileItem *) pi;
			err |= engine->LoadScript(sf->GetFile());
		}
		pi = prjTree->NextSibling(pi);
	}

	return err;
}
