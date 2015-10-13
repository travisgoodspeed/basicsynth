//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file Text editor window implementation
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "globinc.h"
#include "FindReplDlgFltk.h"
#include "TextEditorFltk.h"
#include "MainFrm.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <io.h>

static void TextChangedCB(int pos, int nInserted, int nDeleted, int nRestyled,
						  const char* deletedText, void* cbArg)
{
	TextEditorFltk *ed = reinterpret_cast<TextEditorFltk*>(cbArg);
	ed->TextChanged(pos, nInserted, nDeleted, nRestyled);
}

static void TextEditorCB(Fl_Widget *wdg, void *arg)
{
	TextEditorFltk *ed = reinterpret_cast<TextEditorFltk*>(arg);
	ed->CallbackEvent(wdg);
}

TextEditorFltk::TextEditorFltk(int X, int Y, int W, int H) : Fl_Text_Editor(X, Y, W, H, 0)
{
	findFlags = 0;
	memset(&re, 0, sizeof(re));
	pi = 0;
	Fl_Text_Buffer *buf = new Fl_Text_Buffer;
	buffer(buf);
	buf->add_modify_callback(TextChangedCB, reinterpret_cast<void*>(this));
}

TextEditorFltk::~TextEditorFltk()
{
	regfree(&re);
}

void TextEditorFltk::CallbackEvent(Fl_Widget *wdg)
{
	int evt = Fl::event();
//	LogDebug("TextEditor callback: %d\r\n", evt);
	switch (evt)
	{
	case FL_ACTIVATE:
	case FL_SHOW:
		if (mainWnd && mainWnd->findDlg)
			mainWnd->findDlg->SetEditor(this);
		break;
	case FL_CLOSE:
	case FL_HIDE:
	case FL_DEACTIVATE:
		if (mainWnd && mainWnd->findDlg)
			mainWnd->findDlg->SetEditor(NULL);
		break;
	}
}

void TextEditorFltk::TextChanged(int pos, int inserted, int deleted, int restyled)
{
	if (inserted || deleted)
	{
		changed = 1;
		theProject->SetChange(1);
		//Restyle(pos);
	}
	mainWnd->EditStateChanged();
}


ProjectItem *TextEditorFltk::GetItem()
{
	return pi;
}

void TextEditorFltk::SetItem(ProjectItem *p)
{
	if (pi)
		pi->SetEditor(0);
	pi = p;
	if (pi)
		pi->SetEditor(this);
}

void TextEditorFltk::Undo()
{
	buffer()->undo();
}

void TextEditorFltk::Redo()
{

}

void TextEditorFltk::Cut()
{
	kf_cut(0, this);
}

void TextEditorFltk::Copy()
{
	kf_copy(0, this);
}

void TextEditorFltk::Paste()
{
	kf_paste(0, this);
}


void TextEditorFltk::SelectAll()
{
	Fl_Text_Buffer *buf = buffer();
	buf->select(0, buf->length());
}

void TextEditorFltk::GotoLine(int ln)
{
	if (ln < 0)
	{
		char value[40];
		value[0] = 0;
		if (!prjFrame->QueryValue("Line number:", value, 40))
			return;
		ln = atoi(value);
	}
	Fl_Text_Buffer *buf = buffer();
	buf->skip_lines(0, ln);
}

void TextEditorFltk::GotoPosition(int pos)
{
	Fl_Text_Buffer *buf = buffer();
	buf->skip_displayed_characters(0, pos);
}

void TextEditorFltk::Cancel()
{
}

long TextEditorFltk::EditState()
{
	// I don't see a FLTK function to detect when there is "pasteable" text.
	// or an editor function to detect when "Undo" is valid.
	// No "Redo"
	long flags = VW_ENABLE_FILE | VW_ENABLE_FIND | VW_ENABLE_GOTO
		       | VW_ENABLE_SELALL | VW_ENABLE_UNDO | VW_ENABLE_PASTE;
	int startPos = 0;
	int endPos = 0;
	buffer()->selection_position(&startPos, &endPos);
	if (startPos != endPos)
		flags |= VW_ENABLE_COPY | VW_ENABLE_CUT;
	return flags;
}

int TextEditorFltk::IsChanged()
{
	return changed;
}

int TextEditorFltk::IsKeyword(char *txt)
{
	int kw = 0;
	switch (*txt)
	{
	case 'A':
		if ( strcmp(txt, "ARTIC") == 0
		  || strcmp(txt, "AND") == 0
		  || strcmp(txt, "ADD") == 0)
			kw = 1;
		break;
	case 'B':
		kw = strcmp(txt, "BEGIN") == 0;
		break;
	case 'C':
		if ( strcmp(txt, "CHANNEL") == 0
		  || strcmp(txt, "CHNL") == 0
		  || strcmp(txt, "COUNT") == 0
		  || strcmp(txt, "CALL") == 0
		  || strcmp(txt, "CURPIT") == 0
		  || strcmp(txt, "CURDUR") == 0
		  || strcmp(txt, "CURVOL") == 0
		  || strcmp(txt, "CURTIME") == 0)
			kw = 1;
		break;
	case 'D':
		if (strcmp(txt, "DOUBLE") == 0
		 || strcmp(txt, "DO") == 0
		 || strcmp(txt, "VAR") == 0)
			kw = 1;
		break;
	case 'E':
		if (strcmp(txt, "END") == 0
		 || strcmp(txt, "EXP") == 0
		 || strcmp(txt, "EVAL") == 0
		 || strcmp(txt, "ELSE") == 0)
			kw = 1;
		break;
	case 'F':
		if (strcmp(txt, "FIXED") == 0
		 || strcmp(txt, "FGEN") == 0
		 || strcmp(txt, "FREQUENCY") == 0)
			kw = 1;
		break;
	case 'I':
		if (strcmp(txt, "INSTR") == 0
		 || strcmp(txt, "INSTRUMENT") == 0
		 || strcmp(txt, "INIT") == 0
		 || strcmp(txt, "INCLUDE") == 0
		 || strcmp(txt, "IF") == 0)
			kw = 1;
		break;
	case 'L':
		if (strcmp(txt, "LOOP") == 0
		 || strcmp(txt, "LINE") == 0
		 || strcmp(txt, "LOG") == 0)
			kw = 1;
		break;
	case 'M':
		if (strcmp(txt, "MARK") == 0
		 || strcmp(txt, "MAP") == 0
		 || strcmp(txt, "MAXPARAM") == 0
		 || strcmp(txt, "MIXER") == 0
		 || strcmp(txt, "MIDDLEC") == 0)
			kw = 1;
		break;
	case 'N':
		if (strcmp(txt, "NOT") == 0
		 || strcmp(txt, "NOTE") == 0)
			kw = 1;
		break;
	case 'O':
		if (strcmp(txt, "ON") == 0
		 || strcmp(txt, "OFF") == 0
		 || strcmp(txt, "OR") == 0
		 || strcmp(txt, "OPTION") == 0)
			kw = 1;
		break;
	case 'P':
		if (strcmp(txt, "PARAM") == 0
		 || strcmp(txt, "PLAY") == 0
		 || strcmp(txt, "PERCENT") == 0)
			kw = 1;
		break;
	case 'R':
		if (strcmp(txt, "RAND") == 0
		 || strcmp(txt, "REPEAT") == 0)
			kw = 1;
		break;
	case 'S':
		if (strcmp(txt, "SUS") == 0
		 || strcmp(txt, "SUSTAIN") == 0
		 || strcmp(txt, "SEQ") == 0
		 || strcmp(txt, "SEQUENCE") == 0
		 || strcmp(txt, "SYNC") == 0
		 || strcmp(txt, "SET") == 0
		 || strcmp(txt, "SCRIPT") == 0
		 || strcmp(txt, "SYS") == 0
		 || strcmp(txt, "SYSTEM") == 0)
			kw = 1;
		break;
	case 'T':
		if (strcmp(txt, "TIE") == 0
		 || strcmp(txt, "TRANSPOSE") == 0
		 || strcmp(txt, "TIME") == 0
		 || strcmp(txt, "TEMPO") == 0
		 || strcmp(txt, "THEN") == 0)
			kw = 1;
		break;
	case 'V':
		if (strcmp(txt, "VOICE") == 0
		 || strcmp(txt, "VOL") == 0
		 || strcmp(txt, "VOLUME") == 0
		 || strcmp(txt, "VAR") == 0
		 || strcmp(txt, "VARIABLE") == 0
		 || strcmp(txt, "VERSION") == 0
		 || strcmp(txt, "VER") == 0)
			kw = 1;
		break;
	case 'W':
		if (strcmp(txt, "WHILE") == 0
		 || strcmp(txt, "WRITE") == 0)
			kw = 1;
		break;
	}
	return kw;
}

// TODO: read the line into a buffer, then set the styles
void TextEditorFltk::Restyle(int position)
{
}

int TextEditorFltk::OpenFile(const char *fname)
{
	if (pi == NULL)
		return -1;
	if (!theProject->FullPath(fname))
		theProject->FindOnPath(file, fname);
	else
		file = fname;

	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->loadfile(file);
	changed = 0;

	return 0;
}

int TextEditorFltk::SaveFile(const char *fname)
{
	if (pi == NULL)
		return -1;

	if (fname == 0 || *fname == 0)
	{
		if (file.Length() == 0)
		{
			const char *spc = ProjectItem::GetFileSpec(pi->GetType());
			if (!prjFrame->BrowseFile(0, file, spc, 0))
				return -1;
		}
	}
	else
		file = fname;

	Fl_Text_Buffer *buf = buffer();
	if (buf != 0)
	{
		if (buf->outputfile(file, 0, buf->length()))
		{
			prjFrame->Alert("Error saving file.", "Ooops...");
			return -1;
		}
		changed = 0;
		return 0;
	}
	return -1;
}

int TextEditorFltk::GetText(bsString& text)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf != 0)
		text.Attach(buf->text());
	else
		text = "";
	return text.Length();
}

int TextEditorFltk::SetText(bsString& text)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->text(text);
	return 0;
}

void TextEditorFltk::Find()
{
	mainWnd->ShowFind(this);
}

void TextEditorFltk::FindNext()
{
	if (findText.Length() == 0)
		mainWnd->ShowFind(this);
	else
		Find(-1, 0);
}

bool TextEditorFltk::InitFind(int flags, const char *text, bool insel)
{
	Fl_Text_Buffer *buf = buffer();
	int sel = buf->selection_position(&findStart, &findEnd);
	if (insel)
	{
		if (!sel)
		{
			findStart = insert_position();
			findEnd = findStart;
			return false;
		}
	}
	else
	{
		if (!sel)
			findStart = insert_position();
		else
			findStart = findEnd+1;
		findEnd = buf->length();
		if (findStart > findEnd)
			findStart = findEnd;
	}
	if (flags != -1)
		findFlags = flags;
	if (text)
	{
		findText = text;
		if (findFlags & TXTFIND_REGEXP)
		{
			int reflags = REG_EXTENDED|REG_NEWLINE;
			if (!(flags & TXTFIND_MATCHCASE))
				reflags |= REG_ICASE;
			regfree(&re);
			if (regcomp(&re, text, reflags) != 0)
				return false;
		}
	}
	return true;
}

bool TextEditorFltk::DoFind()
{
	if (findStart == findEnd)
		return false;

	Fl_Text_Buffer *buf = buffer();
	if (findFlags & TXTFIND_REGEXP)
	{
		char *matchText = buf->text_range(findStart, findEnd);
		if (regexec(&re, matchText, RESUBS, resubs, 0) == 0)
		{
			for (int s = 0; s < RESUBS; s++)
			{
				if (resubs[s].rm_so != -1)
					resubs[s].rm_so += findStart;
				if (resubs[s].rm_eo != -1)
					resubs[s].rm_eo += findStart;
			}
			matchStart = resubs[0].rm_so;
			matchEnd = resubs[0].rm_eo;
			free(matchText);
		}
		else
		{
			free(matchText);
			return false;
		}
	}
	else
	{
		if (!buf->search_forward(findStart, findText, &matchStart, findFlags & TXTFIND_MATCHCASE))
			return false;
		matchEnd = matchStart + findText.Length();
	}
	// if we get here we have matched the text
	if (findFlags & (TXTFIND_WHOLEWORD|TXTFIND_WORDSTART))
	{
		if (buf->word_start(matchStart) != matchStart)
			return false;
		if (findFlags & TXTFIND_WHOLEWORD)
		{
			if (buf->word_end(matchStart) != matchEnd)
				return false;
		}
	}
	return true;
}

int TextEditorFltk::Find(int flags, const char *text)
{
	Fl_Text_Buffer *buf = buffer();
	if (!InitFind(flags, text, false))
		return -1;

	int wrap = 0;
	while (1)
	{
		if (DoFind())
		{
			buf->select(matchStart, matchEnd);
			insert_position(matchStart);
			show_insert_position();
			return wrap;
		}
		if (wrap || findStart == 0)
			return -1;
		findEnd = findStart-1;
		findStart = 0;
		wrap = 1;
	}
	return -1;
}


int TextEditorFltk::MatchSel(int flags, const char *ftext)
{
	if (InitFind(flags, ftext, true))
	{
		if (DoFind())
		{
			buffer()->select(matchStart, matchEnd);
			show_insert_position();
			return 1;
		}
		return 0;
	}
	return -1;
}

int TextEditorFltk::DoReplace(const char *rtext)
{
	if (rtext == 0)
		return 0;

	int len = 0;
	Fl_Text_Buffer *buf = buffer();
	if (findFlags & TXTFIND_REGEXP)
	{
		bsString rbuf;
		while (*rtext)
		{
			if (*rtext == '\\')
			{
				rtext++;
				if (isdigit(*rtext))
				{
					int grp = *rtext++ - '0';
					int st = resubs[grp].rm_so;
					int end = resubs[grp].rm_eo;
					if (st != -1)
					{
						while (st <= end)
							rbuf += buf->character(st++);
					}
				}
				else
					rbuf += '\\';
			}
			else
				rbuf += *rtext++;
		}
		buf->replace(matchStart, matchEnd, rbuf);
		len = rbuf.Length();
	}
	else
	{
		buf->replace(matchStart, matchEnd, rtext);
		len = strlen(rtext);
	}
	return len;
}

void TextEditorFltk::Replace(const char *rtext)
{
	int len = DoReplace(rtext);
	buffer()->select(matchStart, matchStart+len);
}

int TextEditorFltk::ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();

	if (!InitFind(flags, ftext, false))
		return -1;

	int count = 0;
	findStart = info.startPos;
	findEnd = info.endPos;
	if (findStart == findEnd)
		findEnd = buf->length();

	int rlen;
	do
	{
		if (!DoFind())
			break;
		count++;
		rlen = DoReplace(rtext);
		findStart = matchStart + rlen;
	} while (findStart < findEnd);
	return count;
}

int TextEditorFltk::GetSelection(SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf == 0)
		return 0;
	int r = buf->selection_position(&info.startPos, &info.endPos);
	if (r)
	{
		info.startLn = buf->line_start(info.startPos);
		info.endLn = buf->line_start(info.endPos);
	}
	return r;
}

void TextEditorFltk::SetSelection(SelectInfo& info)
{
	Fl_Text_Buffer *buf = buffer();
	if (buf)
		buf->select(info.startPos, info.endPos);
}

void TextEditorFltk::Resize(wdgRect& r)
{
}

