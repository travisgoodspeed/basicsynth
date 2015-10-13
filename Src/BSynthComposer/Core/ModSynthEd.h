//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MODSYNTH_EDIT_H
#define MODSYNTH_EDIT_H

class ModSynthEdit : public SynthEdit
{
private:
	XmlSynthDoc doc;
	XmlSynthElem *templates;
	ModSynth *msobj;
	int formXo;
	int formYo;

	void CreateLayout();
	XmlSynthElem *FindTemplate(const char *type);

	void OnConfig();
	void OnConnect();

public:
	ModSynthEdit();
	~ModSynthEdit();

	virtual void SetInstrument(InstrConfig *ip);
	virtual void GetParams();
	virtual void LoadValues();
	virtual void SaveValues();
	virtual void ValueChanged(SynthWidget *wdg);
	virtual int Load(const char *fileName, int xo, int yo);
	virtual int Load(XmlSynthElem *root, int xo, int yo);
};

#endif
