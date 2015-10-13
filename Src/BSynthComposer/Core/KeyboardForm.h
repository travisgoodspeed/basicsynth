//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef KEYBOARD_FORM_H
#define KEYBOARD_FORM_H

/// Widget form for the virtual keyboard.
class KeyboardForm : public WidgetForm
{
private:
	KeyboardWidget *kbd;
	SynthWidget *startStop;

public:
	KeyboardForm()
	{
		kbd = 0;
		startStop = 0;
	}

	KeyboardWidget *GetKeyboard()
	{
		return kbd;
	}

	SynthWidget *GetInstrList()
	{
		return mainGroup->FindID(1);
	}

	int Start();
	int Stop();

	virtual int Load(const char *fileName, int xo, int yo);
	virtual void ValueChanged(SynthWidget *wdg);
};

#endif
