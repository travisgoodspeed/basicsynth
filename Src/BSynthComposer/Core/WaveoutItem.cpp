//////////////////////////////////////////////////////////////////////
// BasicSynth - Project item to represent the output wave file.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
#include "ComposerGlobal.h"
#include "WindowTypes.h"
#include "ProjectItem.h"

int WaveoutItem::Load(XmlSynthElem *node)
{
	// TODO: for now we only allow WAV files and so
	// ignore the type attribute.
	// When support for other output formats is included,
	// we will need to load the type as well.
//	node->GetAttribute("type", outType);
	node->GetAttribute("fmt", sampleFmt);
	node->GetAttribute("lead", leadIn);
	node->GetAttribute("tail", tailOut);
	char *content;
	node->GetContent(&content);
	outFile.Attach(content);
	return 0;
}

int WaveoutItem::Save(XmlSynthElem *node)
{
	XmlSynthElem *child = node->AddChild("out");
	child->SetAttribute("type", (short)1);
	child->SetAttribute("fmt", sampleFmt);
	child->SetAttribute("lead", leadIn);
	child->SetAttribute("tail", tailOut);
	child->SetContent(outFile);
	delete child;
	return 0;
}

int WaveoutItem::ItemProperties()
{
	return theProject->ItemProperties();
}

int WaveoutItem::LoadProperties(PropertyBox *pb)
{
	pb->SetValue(PROP_PRJ_OUTF, outFile, 0);
	pb->SetValue(PROP_PRJ_LEAD, leadIn, 0);
	pb->SetValue(PROP_PRJ_TAIL, tailOut, 0);
	pb->SetSelection(PROP_PRJ_SFMT, sampleFmt);
	return 1;
}

int WaveoutItem::SaveProperties(PropertyBox *pb)
{
	pb->GetValue(PROP_PRJ_OUTF, outFile);
	pb->GetValue(PROP_PRJ_LEAD, leadIn);
	pb->GetValue(PROP_PRJ_TAIL, tailOut);
	pb->GetSelection(PROP_PRJ_SFMT, sampleFmt);
	return 1;
}

WaveOut *WaveoutItem::GetOutput()
{
	if (sampleFmt == 1)
		return &wvf32;
	return &wvf16;
}

WaveOut *WaveoutItem::InitOutput()
{
	wvOut = 0;
	if (outFile.Length())
	{
		bsString outPath;
		if (theProject->FullPath(outFile) || outFile[(size_t)0] == '.')
			outPath = outFile;
		else 
		{
			theProject->GetProjectDir(outPath);
			outPath += '/';
			outPath += outFile;
		}
		if (sampleFmt == 1)
		{
			if (wvf32.OpenWaveFile((char*)(const char*)outPath, 2) == 0)
				wvOut = &wvf32;
		}
		else
		{
			if (wvf16.OpenWaveFile((char*)(const char*)outPath, 2) == 0)
				wvOut = &wvf16;
		}
		if (wvOut)
		{
			long pad = (long) (leadIn * synthParams.sampleRate);
			while (pad-- > 0)
				wvOut->Output(0.0);
		}
	}
	return wvOut;
}

int WaveoutItem::CloseOutput(WaveOut *wvOut, Mixer *mix)
{
	AmpValue lv = 0;
	AmpValue rv = 0;
	long pad = (long) (tailOut * synthParams.sampleRate);
	while (pad-- > 0)
	{
		if (mix)
			mix->Out(&lv, &rv);
		wvOut->Output2(lv, rv);
	}

	int err = -1;
	if (sampleFmt == 0)
		err = wvf16.CloseWaveFile();
	else if (sampleFmt == 1)
		err = wvf32.CloseWaveFile();
	return err;
}


