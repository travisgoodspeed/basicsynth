//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item to represent a wavetable.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "ComposerCore.h"
#include "WavetableEdit.h"

int WavetableItem::EditItem()
{
	EditorView *ed = prjFrame->CreateFormEditor(this);
	if (ed)
        return 1;
    return 0;
}

WidgetForm *WavetableItem::CreateForm(int xo, int yo)
{
	WavetableEdit *we = new WavetableEdit(this);
	bsString path;
	if (theProject->FindForm(path, "WaveTableEd.xml"))
		we->Load(path, xo, yo);
	return we;
}

int WavetableItem::SaveItem()
{
	FormEditor *fe = static_cast<FormEditor*>(editor);
	if (fe)
		fe->GetForm()->SetParams();
	return 1;
}

int WavetableItem::CopyItem()
{
	WavetableItem *wnew = new WavetableItem;
	wnew->SetParent(parent);
	wnew->SetSum(sumParts);
	wnew->SetGibbs(gibbs);
	wnew->AllocParts(wvParts);
	short ptndx;
	for (ptndx = 0; ptndx < wvParts; ptndx++)
		wnew->SetPart(ptndx, mult[ptndx], amps[ptndx], phs[ptndx]);

	short newID = theProject->synthInfo->GetWTUser();
	theProject->synthInfo->SetWTUser(newID+1);
	wnew->SetID(WT_USR(newID));
	wnew->SetIndex(wtSet.wavTblMax + 1);
	wnew->InitWaveform();
	//char nm[40];
	//snprintf(nm, 40, "#%d", WT_USR(newID));
	bsString nm("wt");
	nm += (long)WT_USR(newID);
	wnew->SetName(nm);
	prjTree->AddNode(wnew);
	wnew->ItemProperties();
	prjTree->UpdateNode(wnew);
	theProject->SetChange(1);
	return 1;
}

int WavetableItem::ItemProperties()
{
	int ok = 0;
	PropertyBox *pb = prjFrame->CreatePropertyBox(this, 0);
	if (pb)
	{
		ok = pb->Activate(1);
		delete pb;
	}
	return ok;
}

int WavetableItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_NAME, name, 0);
	pb->SetValue(PROP_DESC, desc, 0);
	return 1;
}

int WavetableItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_NAME, name);
	pb->GetValue(PROP_DESC, desc);
	return 1;
}

int WavetableItem::AllocParts(short n)
{
	DeleteParts();
	wvParts = n;
	if ((mult = new bsInt32[wvParts]) == 0
	 || (amps = new double[wvParts]) == 0
	 || (phs  = new double[wvParts]) == 0)
		return -1;
	short ptndx;
	for (ptndx = 0; ptndx < wvParts; ptndx++)
	{
		mult[ptndx] = 0;
		amps[ptndx] = 0.0;
		phs[ptndx] = 0.0;
	}
	return 0;
}

void WavetableItem::DeleteParts()
{
	if (wvParts > 0)
	{
		delete[] mult;
		delete[] amps;
		delete[] phs;
		wvParts = 0;
	}
}

int WavetableItem::InitWaveform()
{
	if (wvNdx >= wtSet.wavTblMax)
		wtSet.SetMax(wvNdx+1);
	wtSet.wavSet[wvNdx].wavID = wvID;
	if (sumParts == 1)
		return wtSet.SetWaveTable(wvNdx, wvParts, mult, amps, phs, gibbs);
	else if (sumParts == 2) // linear segments
		return wtSet.SegWaveTable(wvNdx, wvParts, phs, amps);
	return -1;
}

int WavetableItem::Load(XmlSynthElem *node)
{
	ProjectItem::Load(node); // get name/description

	//node->GetAttribute("ndx", wvNdx);
	if (node->GetAttribute("id", wvID))
		wvID = wvNdx;
	if (node->GetAttribute("type", sumParts))
		sumParts = 1;
	node->GetAttribute("gibbs", gibbs);
	short sval;
	node->GetAttribute("parts", sval);
	if (sval < 1)
		sval = 1;
	AllocParts(sval);
	int ptndx = 0;
	XmlSynthElem *sib;
	XmlSynthElem *ptnode = node->FirstChild();
	while (ptnode && ptndx < wvParts)
	{
		if (ptnode->TagMatch("part"))
		{
			ptnode->GetAttribute("mul", sval);
			mult[ptndx] = sval;
			ptnode->GetAttribute("amp", amps[ptndx]);
			ptnode->GetAttribute("phs", phs[ptndx]);
			ptndx++;
		}
		sib = ptnode->NextSibling();
		delete ptnode;
		ptnode = sib;
	}
	if (name.Length() == 0)
	{
		//char buf[40];
		//snprintf(buf, 40, "#%d", wvID);
		//name = buf;
		name = "wt";
		name += (long)wvID;
	}
	return 0;
}

int WavetableItem::Save(XmlSynthElem *node)
{
	ProjectItem::Save(node); // set name/description
	//node->SetAttribute("ndx", wvNdx);
	node->SetAttribute("id", wvID);
	node->SetAttribute("parts", wvParts);
	node->SetAttribute("type", sumParts);
	node->SetAttribute("gibbs", gibbs);

	int ptndx;
	for (ptndx = 0; ptndx < wvParts; ptndx++)
	{
		XmlSynthElem *ptnode = node->AddChild("part");
		ptnode->SetAttribute("mul", (short)mult[ptndx]);
		ptnode->SetAttribute("amp", amps[ptndx]);
		ptnode->SetAttribute("phs", phs[ptndx]);
		delete ptnode;
	}
	return 0;
}
