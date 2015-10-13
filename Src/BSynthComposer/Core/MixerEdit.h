//////////////////////////////////////////////////////////////////////
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#ifndef MIXER_EDIT_H
#define MIXER_EDIT_H

#include "WidgetForm.h"

class MixerEdit : public WidgetForm
{
private:
	int changed;
	int numChnl;
	int numFx;
	MixerItem *mixItem;

	void GetArea(XmlSynthElem *node, wdgRect& rc);

public:
	MixerEdit();
	virtual ~MixerEdit();

	virtual int IsChanged() { return changed; }

	void Setup(int xo = 0, int yo = 0);
	void GetParams();
	void SetParams();
	virtual void ValueChanged(SynthWidget *wdg);
	virtual int Load(const char *fileName, int xo, int yo);
	virtual int Load(XmlSynthElem *root, int xo, int yo);
};

#endif
