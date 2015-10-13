//////////////////////////////////////////////////////////////////////
// BasicSynth Composer
//
/// @file FindReplDlgFltk.h Find/Replace dialog
//
// Copyright 2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

class FindReplDlgFltk : public Fl_Window
{
private:
	Fl_Input  *findText;
	Fl_Hold_Browser *findList;
	Fl_Button *findListOpen;
	Fl_Input  *replText;
	Fl_Hold_Browser *replList;
	Fl_Button *replListOpen;
	Fl_Button *findBtn;
	Fl_Button *replBtn;
	Fl_Button *replAllBtn;
	Fl_Button *replSelBtn;
	Fl_Check_Button *matchBtn;
	Fl_Check_Button *fullWordBtn;
	Fl_Check_Button *startWordBtn;
	Fl_Check_Button *regexpBtn;
	Fl_Output *statText;
	Fl_Button *closeBtn;

	TextEditor  *ed;

	int GetFlags();
	int FindNext();
	void DoReplace(int inSel);
	int GetFindText(Fl_Input *txt, Fl_Hold_Browser *lst, bsString& text);
public:
	FindReplDlgFltk();
	~FindReplDlgFltk();

	int handle(int evt);

	TextEditor *GetEditor()
	{
		return ed;
	}

	void SetEditor(TextEditor *p)
	{
		if (p != ed)
		{
			ed = p;
			SetEnable();
		}
	}

	void SetEnable();
	void SelFindText();
	void SelReplText();
	void OnActivate();
	void OnDestroy();
	void OnFindNext();
	void OnReplace();
	void OnReplaceAll();
	void OnReplaceSel();
	void OnCloseCmd();
	void OnTextChanged();
};
