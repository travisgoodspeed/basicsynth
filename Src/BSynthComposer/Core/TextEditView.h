/////////////////////////////////////////////////////////////////////////////

#pragma once

class TextFileItem;

struct SelectInfo
{
	int startPos;
	int startLn;
	int startCh;
	int endPos;
	int endLn;
	int endCh;
};

class EditorView : public CWindow
{
public:
	virtual ~EditorView() { }
	virtual unsigned long GetEditState() { return 0; }
	virtual int IsChanged() { return 0; }
	virtual int IsFile() { return 0; }
	virtual void Undo() { }
	virtual void Redo() { }
	virtual void Cut() { }
	virtual void Copy() { }
	virtual void Paste() { }
	virtual void SelectAll() { }
	virtual void GotoLine(int line) { }
	virtual int GetSelection(SelectInfo& info) { return 0; }
	virtual void SetSelection(SelectInfo& info) { }
	virtual int Find(int flags, const char *text) { return -1; }
	virtual int Replace(int flags, const char *ftext, const char *rtext) { return -1; }
	virtual int ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info) { return 1; }
	virtual int Load() { return 0; }
	virtual int Save() { return 0; }
	virtual const char *GetTitle() { return ""; }
	virtual char *GetText(int *sizRet) { *sizRet = 0; return 0; }
	virtual void SetText(const char *text) { }
};

class TextEditView : public EditorView
{
private:
	TextFileItem *pi;
	int findStart;
	int findEnd;

	int IsKeyword(char *txt);

public:
	TextEditView();
	virtual ~TextEditView();

	virtual int IsFile() { return 1; }

	virtual unsigned long GetEditState();
	virtual int IsChanged();
	virtual void Undo();
	virtual void Redo();
	virtual void Cut();
	virtual void Copy();
	virtual void Paste();
	virtual void SelectAll();
	virtual void GotoLine(int line);

	void SetFileItem(TextFileItem *p) { pi = p; }
	TextFileItem *GetFileItem() { return pi; }

	const char *GetFilename();
	const char *GetTitle();
	char *GetText(int *sizRet);
	void SetText(const char *text);
	void Colorize(int position);
	HWND CreateEditor(HWND parent, int id);
	int Load();
	int Save();
	int GetSelection(SelectInfo& info);
	void SetSelection(SelectInfo& info);
	virtual int Find(int flags, const char *text);
	virtual int Replace(int flags, const char *ftext, const char *rtext);
	virtual int ReplaceAll(int flags, const char *ftext, const char *rtext, SelectInfo& info);
};

