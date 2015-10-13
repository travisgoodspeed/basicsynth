//////////////////////////////////////////////////////////////////////
// BasicSynth - Modular Synthesis instrument editor
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "SynthEdit.h"
#include "ModSynthEd.h"

/// Specialized widget group that understands modsynth UG.
class ModSynthEditGroup : public WidgetGroup
{
public:
	ModSynthUG *ug;

	ModSynthEditGroup()
	{
		ug = 0;
	}

	void LoadValues(ModSynthUG *p)
	{
		KnobWidget *knb;
		SynthWidget *wdg;

		ug = p;
		wdg = FindID(id+2);
		if (wdg)
			wdg->SetText(ug->GetName());

		// If this is an ENVSEGN, setup #segs, level and time ranges
		if (strcmp(ug->GetType(), "ENVSEGN") == 0)
		{
			int nseg = (int)ug->GetInput(UGEG_SEGN);
			EnvelopeWidget *envGr = (EnvelopeWidget *)FindID(838);
			envGr->SetSegs(nseg);
			envGr->SetStart(ug->GetInput(UGEG_ST));
			envGr->SetSus((int)ug->GetInput(UGEG_SUS));
			//float maxLvl = 1.0;
			float maxRt = 1.0;
			for (int n = 0; n < nseg; n++)
			{
				float lvl = ug->GetInput(UGEG_SEGLVL(n));
				float rt = ug->GetInput(UGEG_SEGRTE(n));
				envGr->SetVal(n, rt, lvl);
				//if (lvl > maxLvl)
				//	maxLvl = lvl;
				if (rt > maxRt)
					maxRt = rt;
			}
			//if (maxLvl > 50.0f)
			//	maxLvl = 10000.0f;
			//if (maxLvl > 1.0f)
			//	maxLvl = 50.0f;
			//else
			//	maxLvl = 1.0f;
			//wdg = FindID(806);
			//wdg->SetValue(maxLvl);
			//knb = (KnobWidget *) FindID(825);
			//knb->SetScale(maxLvl);

			if (maxRt > 5.0)
				maxRt = 10.0;
			else if (maxRt > 1.0)
				maxRt = 5.0;
			else
				maxRt = 1.0;
			envGr->SetTime(maxRt*(float)nseg);
			wdg = FindID(810);
			wdg->SetValue(maxRt);
			knb = (KnobWidget *) FindID(828);
			knb->SetRange(0, maxRt);
			wdg = FindID(816);
			wdg->SetValue(1.0); // segment #1
		}
		else if (strcmp(ug->GetType(), "ENVADSR") == 0)
		{
			int ips[] = { UGADSR_ATK, UGADSR_DEC, UGADSR_REL };
			int n;
			float maxRt = 1.0;
			float val;
			for (n = 0; n < 3; n++)
			{
				val = ug->GetInput(ips[n]);
				if (val > maxRt)
					maxRt = val;
			}
			if (maxRt > 5.0f)
				maxRt = 10.0f;
			else if (maxRt > 1.0f)
				maxRt = 5.0f;
			else
				maxRt = 1.0f;
			wdg = FindID(336);
			wdg->SetValue(maxRt);
			for (n = 0; n < 3; n++)
			{
				knb = (KnobWidget *) FindIP(ips[n]);
				if (knb)
					knb->SetRange(0, maxRt, 4);
			}
		}
		// If this is a 2-pole lowpass set range for resonance
		else if (strcmp(ug->GetType(), "LOWPASSR") == 0)
		{
			wdg = FindID(606);
			if (wdg)
				wdg->SetText("Res");
			knb = (KnobWidget *)FindID(607);
			if (knb)
				knb->SetRange(0, 50.0, 3);
		}
		else if (strcmp(ug->GetType(), "LOWPASS") == 0
		      || strcmp(ug->GetType(), "HIPASS") == 0)
		{
			wdg = FindID(606);
			if (wdg)
				wdg->SetText("Gain");
			knb = (KnobWidget *)FindID(607);
			if (knb)
				knb->SetRange(0, 10.0, 3);
		}

		int count = ug->GetNumInputs();
		int index = 0;
		while (index < count)
		{
			wdg = FindIP(index);
			if (wdg)
			{
				if (wdg->GetType() == wdgSwitch)
					wdg->SetState((int) ug->GetInput(index));
				else
					wdg->SetValue(ug->GetInput(index));
			}
			index++;
		}
	}

	void ValueChanged(SynthWidget *wdg)
	{
		char numtxt[40];

		ModSynthEdit *ed = (ModSynthEdit *)form;
		int id = wdg->GetID();
		if ((id % 50) == 2) // generator name
			ug->SetName(wdg->GetText());
		else if (id == 126)
			ed->SelectWavetable(FindID(120), 1);
		else if (id == 223)
			ed->SelectWavetable(FindID(220), 1);
		else if (id == 273)
			ed->SelectWavetable(FindID(270), 1);
		else if (id == 455)
			ed->SelectWavetable(FindID(454), 1);
		else if (id == 336) // ADSR rate scale
		{
			float val = wdg->GetValue();
			int ips[] = { UGADSR_ATK, UGADSR_DEC, UGADSR_REL, -1 };
			KnobWidget *knb;
			for (int n = 0; ips[n] >= 0; n++)
			{
				knb = (KnobWidget *) FindIP(ips[n]);
				if (knb)
				{
					knb->SetRange(0, val, 4);
					form->ValueChanged(knb);
					form->Redraw(knb);
				}
			}
			// TODO: put a group box around the knobs and then draw that group!
		}
		else if (id >= 800 && id <= 899)
		{
			EnvelopeWidget *envGr = (EnvelopeWidget *)FindID(838);
			if (id == 803) // ENVSEGN # segments
			{
				int nsegs = (int) wdg->GetValue();
				wdg = FindID(815);
				if ((int)wdg->GetValue() > nsegs)
				{
					wdg->SetText(numtxt);
					form->Redraw(wdg);
				}
				ug->SetInput(1, (float)nsegs);
				envGr->SetSegs(nsegs);
				form->Redraw(envGr);
			}
			/*else if (id == 806) // level range
			{
				float lvl = wdg->GetValue();
				KnobWidget *knb = (KnobWidget *) FindID(828);
				if (knb)
				{
					knb->SetScale(lvl);
					form->ValueChanged(knb);
					form->Redraw(knb);
				}
				//envGr->SetLevel(lvl);
				form->Redraw(envGr);
			}*/
			else if (id == 810) // rate range
			{
				float rt = wdg->GetValue();
				KnobWidget *knb = (KnobWidget *) FindID(825);
				if (knb)
				{
					knb->SetRange(0, rt);
					form->Redraw(knb);
				}
				envGr->SetTime(rt*ug->GetInput(UGEG_SEGN));
				form->Redraw(envGr);
			}
			else if (id == 815 || id == 839)
			{
				wdg = FindID(816);
				float seg = wdg->GetValue();
				if (id == 815)
					seg += 1.0;
				else
					seg -= 1.0;
				float segn = ug->GetInput(UGEG_SEGN);
				if (seg > segn)
					seg = 1.0;
				else if (seg < 1.0)
					seg = segn;
				wdg->SetValue(seg);
				form->Redraw(wdg);
				short index = (short) seg - 1;
				wdg = FindIP(UGEG_SEGLVL(0));
				if (wdg)
				{
					wdg->SetValue(ug->GetInput(UGEG_SEGLVL(index)));
					form->Redraw(wdg);
				}
				wdg = FindIP(UGEG_SEGRTE(0));
				if (wdg)
				{
					wdg->SetValue(ug->GetInput(UGEG_SEGRTE(index)));
					form->Redraw(wdg);
				}
				wdg = FindIP(UGEG_SEGTYP(0));
				if (wdg)
				{
					wdg->SetValue(ug->GetInput(UGEG_SEGTYP(index)));
					form->Redraw(wdg);
				}
				wdg = FindIP(UGEG_SEGFIX(0));
				if (wdg)
				{
					wdg->SetValue(ug->GetInput(UGEG_SEGFIX(index)));
					form->Redraw(wdg);
				}
			}
			else if (id == 823)
			{
				// sustain on/off
				int on = wdg->GetState();
				ug->SetInput(3, (float)on);
				envGr->SetSus(on);
				form->Redraw(envGr);
			}
			else if (id >= 817)
			{
				bsInt16 ip = wdg->GetIP();
				if (ip >= 0)
				{
					// get currently selected segment
					SynthWidget *seg = FindID(816);
					short segn = (short) seg->GetValue() - 1;
					int index = ip;
					if (index > UGEG_SCL)
						index += segn * 4;
					if (wdg->GetType() == wdgSwitch)
						ug->SetInput(index, (float) wdg->GetState());
					else
						ug->SetInput(index, wdg->GetValue());
					if (ip == UGEG_SEGLVL(0) || ip == UGEG_SEGRTE(0))
					{
						float lvl = ug->GetInput(UGEG_SEGLVL(segn));
						float rt = ug->GetInput(UGEG_SEGRTE(segn));
						envGr->SetVal(segn, rt, lvl);
						form->Redraw(envGr);
					}
				}
			}
		}
		else
		{
			bsInt16 ip = wdg->GetIP();
			if (ip >= 0)
			{
				if (wdg->GetType() == wdgSwitch)
					ug->SetInput(ip, (float) wdg->GetState());
				else
					ug->SetInput(ip, wdg->GetValue());
			}
		}
		theProject->SetChange(1);
	}
};

ModSynthEdit::ModSynthEdit()
{
	templates = 0;
	formXo = 0;
	formYo = 0;
}

ModSynthEdit::~ModSynthEdit()
{
	delete templates;
	doc.Close();
}


void ModSynthEdit::SetInstrument(InstrConfig *ip)
{
	SynthEdit::SetInstrument(ip);
	msobj = (ModSynth *) tone;
	CreateLayout();
}

void ModSynthEdit::GetParams()
{
}

void ModSynthEdit::LoadValues()
{
}

void ModSynthEdit::SaveValues()
{
}

void ModSynthEdit::ValueChanged(SynthWidget *wdg)
{
	int id = wdg->GetID();
	if (id == 1003)
		OnConfig();
	else if (id == 1004)
		OnConnect();
	else
		SynthEdit::ValueChanged(wdg);
}

// The form for ModSynth is a set of templates. We have to
// construct the actual form based on the instrument content.
// Here, we load the form file but do not actually load
// widgets into the mainGroup object. Since Load()
// may (will) be called before SetInstrument, we will add the
// actual form widgets when the instrument gets set.
int ModSynthEdit::Load(const char *fileName, int xo, int yo)
{
	templates = doc.Open((char*)fileName);
	if (templates == 0)
		return -1;
	Load(templates, xo, yo);
	return 0;
}

int ModSynthEdit::Load(XmlSynthElem *root, int xo, int yo)
{
	formXo = xo;
	formYo = yo;
	SynthWidget::GetColorAttribute(root, "fg", fgColor);
	SynthWidget::GetColorAttribute(root, "bg", bgColor);
	mainGroup->SetColors(fgColor, bgColor);
	mainGroup->SetForm(this);
	return 0;
}

void ModSynthEdit::CreateLayout()
{
	mainGroup->SetColors(bgColor, fgColor);

	wdgRect a;
	int ypos = 0;
	int xpos = 5;
	int xmax = 805;
	int slotH = 300;
//	int slotW = 100;
	XmlSynthElem *elem;
	ModSynthEditGroup *grp;
	WidgetGroup *wgrp;
	SynthWidget *wdg;


	int maxX = 0;
	int maxY = 0;
	short x, y, w, h;
	templates->GetAttribute("w", x);
	xmax = (int) x;
	elem = FindTemplate("EDIT");
	if (elem)
	{
		wgrp = new WidgetGroup;
		mainGroup->AddWidget(wgrp);
		elem->GetAttribute("x", x);
		elem->GetAttribute("y", y);
		elem->GetAttribute("w", w);
		elem->GetAttribute("h", h);
		a.x = xpos;
		a.y = ypos;
		a.w = w;
		a.h = h;
		wgrp->SetArea(a);
		wgrp->Load(elem);
		elem->GetAttribute("slh", h);
		elem->GetAttribute("slw", w);
//		slotW = w;
		slotH = h;
		ypos += a.h;
	}

	ModSynthUG *ug = msobj->FirstUnit();
	while (ug)
	{
		if (*ug->GetName() != '@')
		{
			elem = FindTemplate(ug->GetType());
			if (elem)
			{
				if (xpos >= xmax)
				{
					xpos = 5;
					ypos += slotH;
				}
				short x, y, w, h;
				grp = new ModSynthEditGroup;
				mainGroup->AddWidget(grp);
				elem->GetAttribute("x", x);
				elem->GetAttribute("y", y);
				elem->GetAttribute("w", w);
				elem->GetAttribute("h", h);
				a.x = xpos;
				a.y = ypos;
				a.w = w;
				a.h = h;
				grp->SetArea(a);
				grp->Load(elem);
				grp->LoadValues(ug);
				// We need to get the display type from the UG since
				// some widgets are shared by different types (like filters)
				wdg = grp->FindID(grp->GetID()+1);
				if (wdg)
				{
					ModSynthUGType *ugt = msobj->FindType(ug->GetType());
					if (ugt)
						wdg->SetText(ugt->shortName);
				}
				xpos += w;
				if (a.GetRight() > maxX)
					maxX = a.GetRight();
				if (a.GetBottom() > maxY)
					maxY = a.GetBottom();

				delete elem;
			}
		}
		ug = msobj->NextUnit(ug);
	}

	a.x = formXo;
	a.y = formYo;
	if (maxX >= xmax)
		a.w = maxX + 10;
	else
		a.w = xmax + 10;
	a.h = maxY + 10;
	mainGroup->SetArea(a);
}

XmlSynthElem *ModSynthEdit::FindTemplate(const char *type)
{
	if (templates == 0)
		return 0;

	XmlSynthElem *grp = templates->FirstChild();
	while (grp)
	{
		char *content = 0;
		if (grp->GetAttribute("name", &content) == 0)
		{
			int match = 0;
			char *ftyp = content;
			while (ftyp && !match)
			{
				char *pipe = strchr(ftyp, '|');
				if (pipe)
					*pipe++ = '\0';
				match = strcmp(type, ftyp) == 0;
				ftyp = pipe;
			}
			delete content;
			if (match)
				return grp;
		}
		XmlSynthElem *sib = grp->NextSibling();
		delete grp;
		grp = sib;
	}
	return 0;
}
