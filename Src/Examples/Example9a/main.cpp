/////////////////////////////////////////////////////////////////////
// BasicSynth Example09a (Chapter 16)
//
// MIDI file playback
//
// This program loads a GM soundbank (SF2 or DLS) and a MIDI file (.mid)
// and produces a wave file.
//
// use: Example09 [-gn] [-vn] [-s] [-d] soundfont.sf2 infile.mid outfile.wav
// -g = GM level 1 or 2, or 0 for not GM.
// -v = master volume level
// -s = silent output
// -d = dump MIDI events to console
//
// Copyright 2009, Daniel R. Mitchell
/////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <BasicSynth.h>
#include <Instruments.h>
#include <SFFile.h>
#include <DLSFile.h>
#include <SMFFile.h>
#include <GMPlayer.h>

extern const char *midiCCName[128];
extern const char *midiMsgName[8];
extern const char *midiSysName[16];

void useage()
{
	fprintf(stderr, "MIDI File Player\n");
	fprintf(stderr, "use: example9a [-gn] [-vn] [-s] [-d] soundbank midifile wavefile\n");
	fprintf(stderr, "    -vn = master volume level, default 1.0\n");
	fprintf(stderr, "    -nn = maximum voices, default 32\n");
//	fprintf(stderr, "    -gn = GM level n=1 or n=2 (default 1), n=0 use bank select as is\n");
	fprintf(stderr, "    -s  = don't print info or time\n");
	fprintf(stderr, "    -d  = dump MIDI event list to console\n");
	exit(1);
}

int verbose = 1;
time_t clkTime = 0;
long genTime = 0;
void GenCallback(bsInt32 count, Opaque arg)
{
	if (verbose)
		fprintf(stderr, "Time %02d:%02d\r", (genTime / 60), (genTime %60));
	genTime = count;
}

int main(int argc, char *argv[])
{
	InitSynthesizer();

	int dump = 0;
	int gmbank = 1;
	bsString midFile;
	bsString sbFile;
	bsString wavFile;
	int maxNotes = 32;
	AmpValue vol = 2.0;

	WaveFile wvf;
	Sequencer seq;
	Mixer mix;
	InstrManager inmgr;
	SFFile sf;
	DLSFile dls;
	SMFFile smf;
	SMFInstrMap map[16];
	int i;
	int nchnls;
	bsInt32 chnls[16];

	int argn = 1;
	char *ap = argv[argn];
	while (argn < argc && *ap == '-')
	{
		if (ap[1] == 'v')
			vol = atof(&ap[2]);
		else if (ap[1] == 's')
			verbose = 0;
		else if (ap[1] == 'd')
			dump = 1;
		else if (ap[1] == 'g')
			gmbank = atoi(&ap[2]);
		else if (ap[1] == 'n')
			maxNotes = atoi(&ap[2]);
		else
			useage();
		argn++;
		ap = argv[argn];
	}

	if (argn < argc)
		sbFile = argv[argn++];
	if (argn < argc)
		midFile = argv[argn++];
	if (argn < argc)
		wavFile = argv[argn];
	if (sbFile.Length() == 0 || midFile.Length() == 0 || wavFile.Length() == 0)
		useage();

	SoundBank *sb = 0;
	if (SFFile::IsSF2File(sbFile))
		sb = sf.LoadSoundBank(sbFile, 0);
	else if (DLSFile::IsDLSFile(sbFile))
		sb = dls.LoadSoundBank(sbFile, 0);
	if (!sb)
	{
		fprintf(stderr, "Failed to load soundbank %s\n", (const char *)sbFile);
		exit(1);
	}
	sb->Lock();

	if (verbose)
	{
		printf("Soundbank:\n%s\nVersion %d.%d.%d.%d\n%s\n\n", (const char*)sb->info.szName,
			sb->info.wMajorFile, sb->info.wMinorFile,
			sb->info.wMajorVer, sb->info.wMinorVer,
			(const char*)sb->info.szCopyright);
	}

	smf.GMBank(gmbank);

	if (smf.LoadFile(midFile))
	{
		fprintf(stderr, "Errror loading .mid file %s\n", (const char *)midFile);
		exit(1);
	}

	if (verbose)
		printf("Song:\n%s%s%s\n%s\n", smf.SeqName(), smf.Copyright(), smf.TimeSignature(), smf.KeySignature());

	if (dump)
	{
		SMFTrack *trk = 0;
		while ((trk = smf.Enum(trk)) != 0)
		{
			char fmtbuf[80];
			printf("======== Track %d ========\n", trk->trkNum+1);
			printf("delta-t  mm ch v1  v2\n");
			MIDIEvent *evt = 0;
			while ((evt = trk->Enum(evt)) != 0)
			{
				const char *evtName;
				short mm = evt->mevent & 0xf0;
				if (mm == 0xf0)
				{
					evtName = midiSysName[evt->mevent & 0x0f];
					if (evt->mevent == MIDI_SYSEX)
					{
						sprintf(fmtbuf, "%s %04x", evtName, evt->val2);
						evtName = fmtbuf;
					}
				}
				else if (mm == MIDI_CTLCHG)
					evtName = midiCCName[evt->val1];
				else if (mm == MIDI_NOTEON && evt->val2 == 0)
					evtName = "NOTE ON-OFF";
				else
				{
					evtName = midiMsgName[(evt->mevent >> 4)&7];
					if (evt->mevent == MIDI_PWCHG)
					{
						sprintf(fmtbuf, "%s %d", evtName, (int) (evt->val1 | evt->val2 << 7) - 8192);
						evtName = fmtbuf;
					}
				}
				printf("%08d %02x %02d %-3d %-3d = %s\n",
					evt->deltat, evt->mevent, evt->chan,
					evt->val1, evt->val2, evtName);
			}
		}
		exit(0);
	}

	// This shows using one player config for all channels.
	// When	using the GMManager, instruments get the patch
	// directly from the MIDIChannelControl object. Envelope
	// is set from the SF2/DLS file.

	// Add GMManager type to instrument manager
	InstrMapEntry *ime = inmgr.AddType("GMPlayer",
		GMPlayer::InstrFactory,
		GMPlayer::EventFactory,
		NULL);

	// Create the template instrument
	GMPlayer *instr = new GMPlayer;
	//instr->SetSoundFile(sb->name);
	instr->SetSoundBank(sb);
	instr->SetParam(GMPLAYER_FLAGS, (float)(GMPLAYER_LOCAL_PAN|GMPLAYER_LOCAL_VOL));

	// Add it as template to the available instrument list.
	InstrConfig *inc = inmgr.AddInstrument(0, ime, instr);
	for (i = 0; i < 16; i++)
	{
		map[i].inc = inc;
		// The bank and patch params can be used to send
		// the preset number to the instrument in the START event.
		map[i].bnkParam = GMPLAYER_BANK;
		map[i].preParam = GMPLAYER_PROG;
	}

	smf.GenerateSeq(&seq, &map[0], sb, 0xffff);
	nchnls = smf.GetChannelMap(chnls);
	if (nchnls == 0)
	{
		fprintf(stderr, "No notes found!\n");
		exit(0);
	}

	// Initialize the mixer
	mix.MasterVolume(vol, vol);
	mix.SetChannels(nchnls);
	for (i = 0; i < nchnls; i++)
	{
		mix.ChannelVolume(i, 1.0);
		mix.ChannelOn(i, chnls[i] > 0);
	}

	// run the sequencer
	wvf.OpenWaveFile(wavFile, 2);
	inmgr.Init(&mix, &wvf);
	seq.SetCB(GenCallback, synthParams.isampleRate, 0);
	genTime = 0;
	time(&clkTime);
	seq.SetMaxNotes(maxNotes);
	seq.Sequence(inmgr, 0, 0);
	wvf.CloseWaveFile();

	if (verbose)
	{
		long clkTimeDiff = (long) (time(0) - clkTime);
		printf("\n%02d:%02d in %02ld:%02ld %d%%\n",
			genTime / 60, genTime % 60, clkTimeDiff / 60, clkTimeDiff % 60,
			(clkTimeDiff * 100) / genTime);
	}

	sb->Unlock();

	return 0;
}
