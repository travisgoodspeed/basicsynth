///////////////////////////////////////////////////////////
// BasicSynth - command-line synthesizer with Notelist
//
// use: BSynth project
///////////////////////////////////////////////////////////
#include "BSynth.h"
#include <SFFile.h>
#include <DLSFile.h>

class BSynthError : public nlErrOut
{
public:

	virtual void OutputDebug(const char *s)
	{
		fputs(s, stderr);
		fputc('\n', stderr);
	}

	virtual void OutputError(const char *s)
	{
		fputs(s, stderr);
		fputc('\n', stderr);
	}

	virtual void OutputMessage(const char *s)
	{
		fputs(s, stdout);
		fputc('\n', stdout);
	}
};

class ProjectFileList : 
	public SynthList<ProjectFileList>
{
public:
	char *str;

	ProjectFileList() {	str = 0; }
	~ProjectFileList() { delete str; }
};

class SynthProject
{
public:
	char *name; // name of the project
	char *author; // author/composer
	char *descr;
	char *cpyrgt; // copyright
	char *title;
	char *outFile;
	ProjectFileList *libPath;
	long sampleRate;
	long sampleFormat;
	long wtSize;
	long wtUser;
	long mixChnl;
	long fxChnl;
	float mixVolLft;
	float mixVolRgt;
	AmpValue tail;
	AmpValue lead;
	int silent;

	long outType;
	long lastOOR;

	WaveOut *wvp;
	WaveFile wvf;
	WaveFileIEEE wvf32;
	Sequencer seq;
	Mixer mix;
	InstrManager mgr;
	BSynthError err;
	nlConverter cvt;

	static void Monitor(bsInt32 cnt, Opaque arg)
	{
		((SynthProject *)arg)->Update(cnt);
	}

	static void DestroyTemplate(Opaque tp)
	{
		Instrument *ip = (Instrument *)tp;
		delete ip;
	}

	void Update(bsInt32 cnt)
	{
		AmpValue lftPk, rgtPk;
		mix.Peak(lftPk, rgtPk);
		long oor = wvp->GetOOR();
		if (oor > lastOOR)
		{
			fprintf(stdout, " %ld samples out-of-range, peak: left=%f, right=%f\n", oor-lastOOR, lftPk, rgtPk);
			lastOOR = oor;
		}
		fprintf(stdout, "\r%d:%02d", cnt / 60, cnt % 60);
		fflush(stdout);
	}

	SynthProject()
	{
		silent = 0;
		name = 0;
		author = 0;
		descr = 0;
		cpyrgt = 0;
		title = 0;
		outFile = 0;
		libPath = 0;
		sampleRate = 44100;
		sampleFormat = 0; // PCM
		wtSize = 16384;
		wtUser = 0;
		mixChnl = 0;
		fxChnl = 0;
		mixVolLft = 1.0;
		mixVolRgt = 1.0;
		lead = 0.0;
		tail = 0.0;
		wvp = &wvf;
	}
	~SynthProject()
	{
	}

	void Init()
	{
		InstrMapEntry *im = 0;
		im = mgr.AddType("Tone", ToneInstr::ToneFactory, ToneInstr::ToneEventFactory);
		im->paramToID = ToneInstr::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("ToneFM", ToneFM::ToneFMFactory, ToneFM::ToneFMEventFactory);
		im->paramToID = ToneFM::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("AddSynth", AddSynth::AddSynthFactory, AddSynth::AddSynthEventFactory);
		im->paramToID = AddSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("SubSynth", SubSynth::SubSynthFactory, SubSynth::SubSynthEventFactory);
		im->paramToID = SubSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("FMSynth", FMSynth::FMSynthFactory, FMSynth::FMSynthEventFactory);
		im->paramToID = FMSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("MatrixSynth", MatrixSynth::MatrixSynthFactory, MatrixSynth::MatrixSynthEventFactory);
		im->paramToID = MatrixSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("WFSynth", WFSynth::WFSynthFactory, WFSynth::WFSynthEventFactory);
		im->paramToID = WFSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("Chuffer", Chuffer::ChufferFactory, Chuffer::ChufferEventFactory);
		im->paramToID = Chuffer::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("ModSynth", ModSynth::ModSynthFactory, ModSynth::ModSynthEventFactory);
		im->paramToID = ModSynth::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("SoundBank", SFPlayerInstr::SFPlayerInstrFactory, SFPlayerInstr::SFPlayerEventFactory);
		im->paramToID = SFPlayerInstr::MapParamID;
		im->dumpTmplt = DestroyTemplate;
		im = mgr.AddType("GMPlayer", GMPlayer::InstrFactory, GMPlayer::EventFactory);
		im->paramToID = GMPlayer::MapParamID;
		im->dumpTmplt = DestroyTemplate;
	}

	int LoadProject(char *prjFname)
	{
		int errcnt = 0;
		bsString fullPath;
		char *fname;
		XmlSynthDoc doc;
		XmlSynthElem *root;

		if ((root = doc.Open(prjFname)) == NULL)
		{
			fprintf(stderr, "Cannot open project %s\n", prjFname);
			return -1;
		}

		if (!root->TagMatch("synthprj"))
		{
			fprintf(stderr, "No synthprj node for %s\n", prjFname);
			delete root;
			return -1;
		}

		// three passes - 
		// 1) global info
		// 2) load instruments 
		// 3) score files
		int gotSynth = 0;
		XmlSynthElem *child = root->FirstChild();
		XmlSynthElem *sib;
		while (child != NULL)
		{
			if (child->TagMatch("name"))
				child->GetContent(&name);
			else if (child->TagMatch("author"))
				child->GetContent(&author);
			else if (child->TagMatch("desc"))
				child->GetContent(&descr);
			else if (child->TagMatch("cpyrgt"))
				child->GetContent(&cpyrgt);
			else if (child->TagMatch("out"))
			{
				child->GetAttribute("type", outType);
				child->GetAttribute("fmt", sampleFormat);
				child->GetAttribute("lead", lead);
				child->GetAttribute("tail", tail);
				child->GetContent(&outFile);
			}
			else if (child->TagMatch("wvdir"))
			{
				char *file = 0;
				if (child->GetContent(&file) == 0)
				{
					if (*file)
						synthParams.wvPath = file;
					delete file;
				}
			}
			else if (child->TagMatch("wvfile"))
			{
				char *file = 0;
				short id = -1;
				child->GetContent(&file);
				child->GetAttribute("id", id);
				if (file)
				{
					if (!silent)
						printf("Load wavefile '%s' as ID %d\n", file, id);
					if (WFSynth::AddToCache(file, id) == -1)
						fprintf(stderr, "Error loading wave file '%s'\n", file);
					delete file;
				}
			}
			else if (child->TagMatch("sndbnk") || child->TagMatch("sf2") || child->TagMatch("dls"))
			{
				// sf2 and dls types are for backward compatibility; use sndbnk from now on...
				short inc = 1;
				child->GetAttribute("inc", inc);
				if (inc)
				{
					char *file = 0;
					child->GetContent(&file);
					if (file)
					{
						SoundBank *bnk = 0;
						bsInt16 pre = 0;
						float nrm = 1.0;
						child->GetAttribute("pre", pre);
						//child->GetAttribute("nrm", nrm);
						if (SFFile::IsSF2File(file))
						{
							SFFile sndfile;
							bnk = sndfile.LoadSoundBank(file, pre);
						}
						else if (DLSFile::IsDLSFile(file))
						{
							DLSFile sndfile;
							bnk = sndfile.LoadSoundBank(file, pre);
						}
						if (bnk)
						{
							bnk->Lock();
							char *name = 0;
							child->GetAttribute("name", &name);
							if (name)
								bnk->name.Attach(name);
							else
								bnk->name = file;
							SoundBank::SoundBankList.Insert(bnk);
							if (!silent)
								fprintf(stdout, "SoundBank: %s\n%s\n%s\n\n", 
									(const char *)bnk->name, 
									(const char *)bnk->info.szComment,
									(const char *)bnk->info.szCopyright);
						}
						else
						{
							fprintf(stderr, "Cannot load SoundBank '%s'\n", file);
							errcnt++;
						}
						delete file;
					}
				}
			}
			else if (child->TagMatch("libpath"))
			{
				ProjectFileList *lib = new ProjectFileList;
				child->GetContent(&lib->str);
				if (libPath)
					libPath->Insert(lib);
				else
					libPath = lib;
			}
			else if (child->TagMatch("synth"))
			{
				gotSynth = 1;
				child->GetAttribute("sr", sampleRate);
				child->GetAttribute("wt", wtSize);
				child->GetAttribute("usr", wtUser);
				InitSynthesizer((bsInt32)sampleRate, (bsInt32)wtSize, (bsInt32)wtUser);
				int wvCount = 0;
				XmlSynthElem *wvnode = child->FirstChild();
				while (wvnode)
				{
					if (wvnode->TagMatch("wvtable"))
					{
						wvCount++;
						if (mgr.LoadWavetable(wvnode))
						{
							fprintf(stderr, "Error loading wavetable %d\n", wvCount);
							errcnt++;
						}
					}
					sib = wvnode->NextSibling();
					delete wvnode;
					wvnode = sib;
				}
				if (wvCount < wtUser)
				{
					fprintf(stderr, "Not all waveforms are initialized (%d of %d)\n", wvCount, wtUser);
					errcnt++;
				}
			}
			else if (child->TagMatch("mixer"))
			{
				child->GetAttribute("chnls", mixChnl);
				child->GetAttribute("fxunits", fxChnl);
				int fxCount = 0;
				int mixCount = 0;

				if (mixChnl > 0)
				{
					mix.SetChannels(mixChnl);
					mix.SetFxChannels(fxChnl);
					child->GetAttribute("lft", mixVolLft);
					child->GetAttribute("rgt", mixVolRgt);
					mix.MasterVolume(mixVolLft, mixVolRgt);
					XmlSynthElem *mixElem = child->FirstChild();
					while (mixElem)
					{
						short cn;
						short on;
						float pan;
						float vol;
						if (mixElem->TagMatch("chnl"))
						{
							cn = -1;
							mixElem->GetAttribute("cn", cn);
							if (cn >= 0 && cn < mixChnl)
							{
								if (mixElem->GetAttribute("on", on) == 0)
									mix.ChannelOn(cn, on);
								if (mixElem->GetAttribute("vol", vol) == 0)
									mix.ChannelVolume(cn, vol);
								if (mixElem->GetAttribute("pan", pan) == 0)
									mix.ChannelPan(cn, panTrig, pan);
							}
						}
						else if (mixElem->TagMatch("reverb"))
						{
							fxCount++;
							float rvt;
							mixElem->GetAttribute("rvt", rvt);
							Reverb2 *rvb = new Reverb2;
							rvb->InitReverb(1.0, FrqValue(rvt));
							LoadFX(mixElem, rvb);
						}
						else if (mixElem->TagMatch("flanger"))
						{
							fxCount++;
							float flngMix = 0.5;
							float flngFb = 0.0;
							float flngCenter = 0.005;
							float flngDepth = 0.001;
							float flngSweep = 0.15;
							mixElem->GetAttribute("mix", flngMix);
							mixElem->GetAttribute("fb", flngFb);
							mixElem->GetAttribute("cntr", flngCenter);
							mixElem->GetAttribute("depth", flngDepth);
							mixElem->GetAttribute("sweep", flngSweep);
							Flanger *flng = new Flanger;
							flng->InitFlanger(1.0, flngMix, flngFb, flngCenter, flngDepth, flngSweep);
							LoadFX(mixElem, flng);
						}
						else if (mixElem->TagMatch("echo"))
						{
							fxCount++;
							float dly = 0;
							float dec = 0;
							mixElem->GetAttribute("dly", dly);
							mixElem->GetAttribute("dec", dec);
							DelayLineR *dl = new DelayLineR;
							dl->InitDLR(dly, dec, 0.001, 1.0);
							LoadFX(mixElem, dl);
						}
						sib = mixElem->NextSibling();
						delete mixElem;
						mixElem = sib;
					}
					if (fxCount < fxChnl)
					{
						fprintf(stderr, "Not all fx units are configured (only %d of %d)\n", fxCount, fxChnl);
						errcnt++;
					}
				}
			}
			else if (child->TagMatch("midi"))
			{
				short chnl;
				bsInt16 val;
				AmpValue vol;

				XmlSynthElem *chnlNode = child->FirstChild();
				while (chnlNode)
				{
					if (chnlNode->TagMatch("chnl"))
					{
						chnlNode->GetAttribute("cn", chnl);
						chnlNode->GetAttribute("bnk", val);
						mgr.ProcessMessage(MIDI_CTLCHG|chnl, MIDI_CTRL_BANK, val);
						chnlNode->GetAttribute("prg", val);
						mgr.ProcessMessage(MIDI_PRGCHG|chnl, val, 0);
						chnlNode->GetAttribute("vol", vol);
						mgr.ProcessMessage(MIDI_CTLCHG|chnl, MIDI_CTRL_VOL, val);
						chnlNode->GetAttribute("pan", vol);
						mgr.ProcessMessage(MIDI_CTLCHG|chnl, MIDI_CTRL_PAN, val);
					}
					XmlSynthElem *n = chnlNode->NextSibling();
					delete chnlNode;
					chnlNode = n;
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}
		if (!gotSynth)
		{
			fprintf(stderr, "The project does not contain a <synth> tag.\n");
			errcnt++;
		}
		if (mixChnl < 1)
		{
			fprintf(stderr, "The project does not have any mixer channels\n", mixChnl);
			errcnt++;
		}
		if (!silent)
		{
			if (name)
				fprintf(stdout, "Project %s\n", name);
			if (title)
				fprintf(stdout, "%s\n", title);
			if (author)
				fprintf(stdout, "%s\n", author);
			if (cpyrgt)
				fprintf(stdout, "%s\n", cpyrgt);
			if (descr)
				fprintf(stdout, "%s\n", descr);
		}

		child = root->FirstChild();
		while (child != NULL)
		{
			if (child->TagMatch("libfile"))
			{
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					if (!silent)
						fprintf(stdout, "Load library %s\n", fname);
					if (FindOnPath(fullPath, fname))
					{
						if (LoadInstrLib(mgr, fname))
						{
							fprintf(stderr, "Error loading %s\n", (const char*)fullPath);
							errcnt++;
						}
					}
					else
					{
						fprintf(stderr, "Cannot find instr. file %s\n", (const char*)fullPath);
						errcnt++;
					}
					delete fname;
				}
			}
			else if (child->TagMatch("instrlib"))
			{
				if (LoadInstrLib(mgr, child))
				{
					fprintf(stderr, "Error loading instrLib\n");
					errcnt++;
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}

		cvt.SetErrorCallback(&err);
		cvt.SetInstrManager(&mgr);
		cvt.SetSequencer(&seq);
		cvt.SetSampleRate(sampleRate);

		child = root->FirstChild();
		while (child)
		{
			if (child->TagMatch("score"))
			{
				long dbg = 0;
				child->GetAttribute("dbg", dbg);
				cvt.SetDebugLevel((int)dbg);
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					if (FindOnPath(fullPath, fname))
					{
						if (!silent)
							fprintf(stdout, "Convert %s\n", fname);
						if (cvt.Convert(fullPath, NULL))
							errcnt++;
					}
					else
					{
						fprintf(stderr, "Cannot find score file: %s\n", (const char *)fullPath);
						errcnt++;
					}
					delete fname;
				}
			}
			else if (child->TagMatch("seq"))
			{
				fname = 0;
				child->GetContent(&fname);
				if (fname)
				{
					if (FindOnPath(fullPath, fname))
					{
						if (!silent)
							fprintf(stdout, "Load %s\n", fname);
						SequenceFile seqFileLoad;
						seqFileLoad.Init(&mgr, &seq);
						if (seqFileLoad.LoadFile(fullPath))
						{
							bsString ebuf;
							seqFileLoad.GetError(ebuf);
							fprintf(stderr, "Error loading sequence %s\n%s\n", (const char*)fullPath, (const char*)ebuf);
							errcnt++;
						}
					}
					else
					{
						fprintf(stderr, "Cannot find sequence file: %s\n", (const char *)fullPath);
						errcnt++;
					}
					delete fname;
				}
			}
			sib = child->NextSibling();
			delete child;
			child = sib;
		}
		delete root;
		doc.Close();

		return errcnt;
	}

	int FindOnPath(bsString& fullPath, char *fname)
	{
		int gotFile = 0;
		fullPath = fname;
		ProjectFileList *libs = libPath;
		while (!(gotFile = SynthFileExists(fullPath)) && libs)
		{
			fullPath = libs->str;
			fullPath += "/";
			fullPath += fname;
			libs = libs->next;
		}
		return gotFile;
	}

	void LoadFX(XmlSynthElem *mixElem, GenUnit *gen)
	{
		short fxu;
		short cn;
		float vol;
		float pan;

		mixElem->GetAttribute("unit", fxu);
		mixElem->GetAttribute("vol", vol);
		mix.FxInit(fxu, gen, vol);

		if (mixElem->GetAttribute("pan", pan) == 0)
			mix.FxPan(fxu, panTrig, pan);

		XmlSynthElem *sib;
		XmlSynthElem *fxElem = mixElem->FirstChild();
		while (fxElem)
		{
			if (fxElem->TagMatch("send"))
			{
				fxElem->GetAttribute("chnl", cn);
				fxElem->GetAttribute("amt", vol);
				mix.FxLevel(fxu, cn, vol);
			}
			sib = fxElem->NextSibling();
			delete fxElem;
			fxElem = sib;
		}						
	}

	int Generate()
	{
		AmpValue lv, rv;
		long pad;
		if (!silent)
			fprintf(stdout, "Generate sequence\n");
		int errcnt = cvt.Generate();
		if (errcnt == 0 && outFile)
		{
			if (!silent)
				fprintf(stdout, "Generate wavefile %s\n", outFile);
			if (sampleFormat == 1)
			{
				wvf32.SetBufSize(30);
				wvf32.OpenWaveFile(outFile, 2);
				wvp = &wvf32;
			}
			else
			{
				wvf.SetBufSize(30);
				wvf.OpenWaveFile(outFile, 2);
				wvp = &wvf;
			}
			mix.Reset();
			mgr.Init(&mix, wvp);
			pad = (long) (synthParams.isampleRate * lead);
			while (pad-- > 0)
				wvp->Output2(0.0, 0.0);
			lastOOR = 0;
			if (!silent)
				seq.SetCB(Monitor, synthParams.isampleRate, (Opaque)this);
			int n = seq.Sequence(mgr);
			pad = (long) (synthParams.isampleRate * tail);
			while (pad-- > 0)
			{
				mix.Out(&lv, &rv);
				wvp->Output2(lv, rv);
			}
			if (sampleFormat == 1)
				wvf32.CloseWaveFile();
			else
				wvf.CloseWaveFile();
			if (!silent)
			{
				lastOOR = wvp->GetOOR() - lastOOR;
				if (lastOOR > 0)
					fprintf(stdout, " %ld samples out-of-range\r", lastOOR);
				fprintf(stdout, "\nDone.\n");
			}
		}
		return errcnt;
	}
};

SynthProject prj;

void GetDefault()
{
	char *env = getenv("BSYNTHWAVEIN");
	if (env)
		synthParams.wvPath = env;

#ifdef _WIN32
	HKEY rk;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\BasicSynth", 0, KEY_ALL_ACCESS, &rk) == ERROR_SUCCESS)
	{
		DWORD len = MAX_PATH;
		DWORD type = REG_SZ;
		char path[MAX_PATH];
		if (RegQueryValueEx(rk, "WaveIn", 0, &type, (LPBYTE) path, &len) == ERROR_SUCCESS)
			synthParams.wvPath = path;
		RegCloseKey(rk);
	}
#endif
}

int main(int argc, char *argv[])
{
#if defined(USE_MSXML)
	CoInitialize(0);
#endif

	GetDefault();

	if (argc < 2)
	{
		fprintf(stderr, "use: BSynth [-s] project\n");
	}
	else
	{
		int i = 1;
		if (strcmp(argv[1], "-s") == 0)
		{
			prj.silent = 1;
			i++;
		}
		prj.Init();
		int errcnt = prj.LoadProject(argv[i]);
		if (errcnt == 0)
			errcnt = prj.Generate();
	}

#if defined(USE_MSXML)
	CoUninitialize();
#endif
	return 0;
}
