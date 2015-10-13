/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file DLSFile.cpp MIDI file loader implementation.
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SynthDefs.h>
#include <WaveFile.h>
#include <SynthList.h>
#include <SoundBank.h>
#include <DLSFile.h>

/*static bsUint16 Swap(bsUint16 n)
{
#if SYNTH_BIG_ENDIAN
	return ((n << 8) & 0xff00) | ((n >> 8) & 0x00ff);
#else
	return n;
#endif
}*/

static bsUint32 Swap(bsUint32 n)
{
#if SYNTH_BIG_ENDIAN
	return ((n << 24) & 0xff000000)
	     | ((n << 8)  & 0x00ff0000)
	     | ((n >> 8)  & 0x0000ff00)
	     | ((n >> 24) & 0x000000ff);
#else
	return n;
#endif
}

static int ReadChunk(FileReadBuf& file, dlsChunk& chk)
{
	if (file.FileRead(&chk, 8) != 8)
		return -1;
	chk.cksz = Swap(chk.cksz);
	if (chk.cksz & 1)
		chk.cksz++;
	return 0;
}

DLSFile::DLSFile()
{
	preload = 1;
}

DLSFile::~DLSFile()
{
}

int DLSFile::IsDLSFile(const char *fname)
{
	FileReadBuf file;
	int isDLS = 0;
	if (file.FileOpen(fname) == 0)
	{
		// read the RIFF chunk.
		dlsChunk rchk;
		rchk.ckid = 0;
		if (ReadChunk(file, rchk) == 0 && rchk.ckid == DLS_RIFF_CHUNK)
		{
			// Read the format
			bsInt32 id = 0;
			file.FileRead(&id, 4);
			if (id == DLS_FILE_FORMAT)
				isDLS = 1;
		}
		file.FileClose();
	}
	return isDLS;
}

SoundBank *DLSFile::LoadSoundBank(const char *fname, int pre)
{
	preload = pre;

	file.SetBufSize(0x10000);
	if (file.FileOpen(fname))
		return 0;

	dlsChunk rchk;
	rchk.ckid = 0;
	rchk.cksz = 0;
	// read the RIFF chunk.
	if (ReadChunk(file, rchk) || rchk.ckid != DLS_RIFF_CHUNK)
	{
		file.FileClose();
		return 0;
	}

	// Read the format
	bsInt32 id = 0;
	file.FileRead(&id, 4);
	if (id != DLS_FILE_FORMAT)
	{
		file.FileClose();
		return 0;
	}
	SoundBank *sb = 0;
	if (info.Read(file, rchk.cksz - 4) == 0)
		sb = BuildSoundBank(fname);
	file.FileClose();
	return sb;
}


int DLSFileInfo::Read(FileReadBuf& file, bsUint32 riffSize)
{
	dlsChunk chk;
	int err = 0;
	while (riffSize > 0 && err == 0)
	{
		// read the next chunk header
		if (ReadChunk(file, chk))
			break;
		riffSize -= 8;
		if (chk.ckid == DLS_VERS_CHUNK)
		{
			if (file.FileRead(&vers, 8) != 8)
				err = -1;
		}
		else if (chk.ckid == DLS_DLID_CHUNK)
		{
			if (file.FileRead(&id, 16) != 16)
				err = -1;
		}
		else if (chk.ckid == DLS_COLH_CHUNK)
		{
			if (file.FileRead(&colh, 4) != 4)
				err = -1;
		}
		else if (chk.ckid == DLS_PTBL_CHUNK)
		{
			if (file.FileRead(&ptbl, sizeof(ptbl)) != sizeof(ptbl))
				err = -1;
			else
			{
				if (ptbl.size > 8)
					file.FileSkip(ptbl.size - 8);
				cues = new bsInt32[ptbl.cues];
				file.FileRead(cues, ptbl.cues*4);
			}
		}
		else if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_LINS_FORMAT)
				err = ins.Read(file, chk.cksz - 4);
			else if (fmt == DLS_WVPL_FORMAT)
				err = wvpl.Read(file, chk.cksz - 4);
			else if (fmt == DLS_INFO_FORMAT)
				err = inflist.Read(file, chk.cksz-4);
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		riffSize -= chk.cksz;
	}

	return err;
}

int DLSWvplInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" WVPLChunk(%d)\n", chksz);
	filepos = file.FilePosition();
	int err = 0;
	bsInt32 index = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			if (file.FileRead(&fmt, 4) != 4)
				err = -1;
			else if (fmt == DLS_WAVE_FORMAT)
			{
				DLSWaveInfo *wi = AddItem();
				wi->index = index++;
				err = wi->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSWaveInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("  WAVEChunk(%d)\n", chksz);
	int err = 0;
	bsUint32 fmt = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_DLID_CHUNK:
			file.FileRead(&id, sizeof(id));
			break;
		case DLS_FMT__CHUNK:
			if (file.FileRead(&wvfmt, 16) != 16)
				err = -1;
			else if (chk.cksz > 16)
				file.FileSkip(chk.cksz - 16);
			break;
		case DLS_DATA_CHUNK:
			err = ReadSamples(file, chk.cksz);
			break;
		case DLS_WSMP_CHUNK:
			if (file.FileRead(&wvsmp, sizeof(wvsmp)) != sizeof(wvsmp))
			{
				err = -1;
				break;
			}
			if (wvsmp.size > sizeof(wvsmp))
				file.FileSkip(wvsmp.size - sizeof(wvsmp));
			if (wvsmp.sampleLoops > 0)
			{
				file.FileRead(&loop, sizeof(loop));
				if (loop.size > sizeof(loop))
					file.FileSkip(loop.size - sizeof(loop));
				// just-in-case
				for (bsUint32 n = 1; n < wvsmp.sampleLoops; n++)
					file.FileSkip(loop.size);
			}
			break;
		case DLS_LIST_CHUNK:
			file.FileRead(&fmt, 4);
			if (fmt == DLS_INFO_FORMAT)
				err = info.Read(file, chk.cksz - 4);
			else
				file.FileSkip(chk.cksz - 4);
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSWaveInfo::ReadSamples(FileReadBuf& file, bsUint32 chksz)
{
	//printf("DATAChunk(%d)\n", chksz);

	// save file position - we will load samples later.
	filepos = file.FilePosition();

	file.FileSkip(chksz);
	sampsize = chksz / (bsUint32) (wvfmt.bitsPerSamp / 8);
	return 0;
}

int DLSInsList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("LINSChunk(%d)\n", chksz);

	int index = 0;
	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_INS__FORMAT)
			{
				DLSInsInfo *ii = AddItem();
				ii->index = index++;
				err = ii->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return 0;
}

int DLSInsInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" INSChunk(%d)\n", chksz);
	int err = 0;

	bsUint32 fmt;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_DLID_CHUNK:
			if (file.FileRead(&id, 16) != 16)
				err = -1;
			break;
		case DLS_INSH_CHUNK:
			if (file.FileRead(&hdr, sizeof(hdr)) != sizeof(hdr))
				err = -1;
			break;
		case DLS_LIST_CHUNK:
			fmt = 0;
			if (file.FileRead(&fmt, 4) != 4)
			{
				err = -1;
				break;
			}
			switch (fmt)
			{
			case DLS_LRGN_FORMAT:
				err = rgnlist.Read(file, chk.cksz - 4);
				break;
			case DLS_LART_FORMAT:
			case DLS_LAR2_FORMAT:
				err = lart.Read(file, chk.cksz - 4);
				break;
			case DLS_INFO_FORMAT:
				err = info.Read(file, chk.cksz - 4);
				break;
			default:
				file.FileSkip(chk.cksz - 4);
				break;
			}
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSRgnList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("  LRGNChunk(%d)\n", chksz);
	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_LIST_CHUNK)
		{
			bsUint32 fmt = 0;
			file.FileRead(&fmt, 4);
			if (fmt == DLS_RGN__FORMAT || fmt == DLS_RGN2_FORMAT)
			{
				DLSRgnInfo *ri = AddItem();
				ri->rgn2 = fmt == DLS_RGN2_FORMAT;
				err = ri->Read(file, chk.cksz - 4);
			}
			else
				file.FileSkip(chk.cksz - 4);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSRgnInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("   RGNChunk(%d)\n", chksz);
	int err = 0;

	bsUint32 fmt = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		switch (chk.ckid)
		{
		case DLS_RGNH_CHUNK:
			//printf("    RGNHChunk(%d) %d\n", chk.cksz, sizeof(rgnh));
			if (chk.cksz > sizeof(rgnh))
			{
				file.FileRead(&rgnh, sizeof(rgnh));
				file.FileSkip(chk.cksz-sizeof(rgnh));
			}
			else
				file.FileRead(&rgnh, chk.cksz);
			break;
		case DLS_WSMP_CHUNK:
			//printf("    WSMPChunk(%d)\n", chk.cksz);
			wsmpValid = 1;
			file.FileRead(&wsmp, sizeof(wsmp));
			if (wsmp.size > sizeof(wsmp))
				file.FileSkip(wsmp.size - sizeof(wsmp));
			if (wsmp.sampleLoops > 0)
			{
				file.FileRead(&loop, sizeof(loop));
				if (loop.size > sizeof(loop))
					file.FileSkip(loop.size - sizeof(loop));
				// just-in-case
				for (bsUint32 n = 1; n < wsmp.sampleLoops; n++)
					file.FileSkip(loop.size);
			}
			break;
		case DLS_WLNK_CHUNK:
			//printf("    WLNKChunk(%d)\n", chk.cksz);
			file.FileRead(&wlnk, sizeof(wlnk));
			break;
		case DLS_LIST_CHUNK:
			//printf("    LISTChunk(%d)\n", chk.cksz);
			file.FileRead(&fmt, 4);
			if (fmt == DLS_LART_FORMAT || fmt == DLS_LAR2_FORMAT)
				err = lart.Read(file, chk.cksz - 4);
			else if (fmt == DLS_INFO_FORMAT)
				file.FileSkip(chk.cksz - 4);
			else
			{
				/*char nmbuf[5];
				memcpy(nmbuf, &fmt, 4);
				nmbuf[4] = 0;
				//printf("List format %s\n", nmbuf);*/
				file.FileSkip(chk.cksz - 4);
			}
			break;
		default:
			file.FileSkip(chk.cksz);
			break;
		}
		chksz -= chk.cksz;
	}
	return err;
}

int DLSArtList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("    LARTChunk(%d)\n", chksz);
	int err = 0;

	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		if (chk.ckid == DLS_ART1_CHUNK || chk.ckid == DLS_ART2_CHUNK)
		{
			DLSArtInfo *ci = AddItem();
			ci->art2 = chk.ckid == DLS_ART2_CHUNK;
			ci->Read(file, chk.cksz);
		}
		else
			file.FileSkip(chk.cksz);
		chksz -= chk.cksz;
	}
	return err;
}

int DLSArtInfo::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf("     ArtNChunk(%d)\n", chksz);

	bsUint32 sz = 0;
	file.FileRead(&sz, 4);
	file.FileRead(&connections, 4);
	if (sz > 8)
		file.FileSkip(sz - 8);
	if (connections == 0)
		return 0;

	info = new dlsConnection[connections];
	for (bsUint32 ndx = 0; ndx < connections; ndx++)
	{
		if (file.FileRead(&info[ndx], 12) != 12)
			return -1;
		if (art2 && info[ndx].destination == CONN_DST_ATTENUATION)
			info[ndx].scale = -info[ndx].scale;
	}
	return 0;
}


int DLSInfoList::Read(FileReadBuf& file, bsUint32 chksz)
{
	//printf(" InfoList(%d)\n", chksz);

	int err = 0;
	dlsChunk chk;
	while (chksz > 0 && err == 0)
	{
		if (ReadChunk(file, chk))
			return -1;
		chksz -= 8;
		if (chk.cksz > 0)
		{
			if (chk.cksz & 1)
				chk.cksz++;
			DLSInfoStr *str = AddItem();
			str->id = chk.ckid;
			str->Read(file, chk.cksz);
			chksz -= chk.cksz;
		}
	}
	return err;
}

int DLSInfoStr::Read(FileReadBuf& file, bsInt32 cksz)
{
	//printf("  InfoStr(%d)\n", cksz);
	char *s = (char *)malloc(cksz+1);
	if (file.FileRead(s, cksz) == cksz)
	{
		s[cksz] = '\0';
		while (--cksz >= 0)
		{
			if (s[cksz] && s[cksz] != ' ')
				break;
			s[cksz] = '\0';
		}
		str.Attach(s);
		return 0;
	}
	free(s);
	return -1;
}

float DLSFile::DLSScale(bsInt32 scl)
{
	return (AmpValue) scl / 65536.0;
}

SoundBank *DLSFile::BuildSoundBank(const char *fname)
{
	SoundBank *sbnk = new SoundBank;

	sbnk->file = fname;
	sbnk->info.wMajorFile = info.vers.dwVersionMS >> 16;
	sbnk->info.wMinorFile = info.vers.dwVersionMS & 0xffff;
	sbnk->info.wMajorVer = info.vers.dwVersionLS >> 16;
	sbnk->info.wMinorVer = info.vers.dwVersionLS & 0xffff;
	sbnk->info.szName = info.inflist.GetInfo(DLS_INAM_CHUNK);
	sbnk->info.szDate = info.inflist.GetInfo(DLS_ICRD_CHUNK);
	sbnk->info.szEng = info.inflist.GetInfo(DLS_IENG_CHUNK);
	sbnk->info.szProduct = info.inflist.GetInfo(DLS_IPRD_CHUNK);
	sbnk->info.szCopyright = info.inflist.GetInfo(DLS_ICOP_CHUNK);
	sbnk->info.szComment = info.inflist.GetInfo(DLS_ICMT_CHUNK);

	DLSWaveInfo *wvi = 0;
	while ((wvi = info.wvpl.EnumWave(wvi)) != 0)
	{
		SBSample *sp = sbnk->AddSample(wvi->index);
		//sp->filepos = info.cues[wvi->index] + info.wvpl.filepos;
		//if (sp->filepos != wvi->filepos) // testing
		sp->filepos = wvi->filepos;
		sp->sampleLen = wvi->sampsize;
		sp->rate = wvi->wvfmt.sampleRate;
		sp->channels = wvi->wvfmt.channels;
		if (wvi->wvfmt.fmtTag == 1)
			sp->format = wvi->wvfmt.bitsPerSamp == 8 ? 0 : 1;
		else if (wvi->wvfmt.fmtTag == 2)
			sp->format = 2;
		if (preload)
			sbnk->LoadSample(sp, file);
	}

	bsUint32 insIndx = 0;
	DLSInsInfo *ins = 0;
	DLSRgnInfo *rgn;
	DLSArtInfo *art;
	while ((ins = info.ins.EnumIns(ins)) != 0)
	{
		SBInstr *instr = sbnk->AddInstr(ins->GetBank(), ins->GetProg());
		if (instr == 0)
			continue;

		SBZoneGroup *grp = instr->AddGroup();

		insIndx++;
		instr->instrName = ins->info.GetInfo(DLS_INAM_CHUNK);

		rgn = 0;
		while ((rgn = ins->rgnlist.EnumRgn(rgn)) != 0)
		{
			SBZone *zone = instr->AddZone();

			// alter some defaults...
			zone->vibLfo.rate = -851.3179; // 5Hz: pc = 1200 * log2(5/440) + 6900
			zone->modLfo.rate = -851.3179;
			zone->vibLfo.delay = -7972.6274; // 10ms: tc = 1200 * log2(0.01)
			zone->modLfo.delay = -7972.6274;

			zone->lowKey = rgn->rgnh.rangeKey.low;
			zone->highKey = rgn->rgnh.rangeKey.high;
			zone->lowVel = rgn->rgnh.rangeVel.low;
			zone->highVel = rgn->rgnh.rangeVel.high;
			zone->exclNote = (bsUint32) rgn->rgnh.keyGroup;

			// Connect to sample
			DLSWaveInfo *wv = info.wvpl.FindWave(rgn->wlnk.tableIndex);
			if (wv)
			{
				dlsSample *wsmp;
				dlsLoop *loop;
				if (rgn->wsmpValid)
				{
					wsmp = &rgn->wsmp;
					loop = &rgn->loop;
				}
				else
				{
					wsmp = &wv->wvsmp;
					loop = &wv->loop;
				}
				zone->keyNum = wsmp->unityNote;
				zone->cents = -wsmp->fineTune;
				zone->initAtten = MapScale(SBGEN_VOLUME, wsmp->attenuation);
				if (wsmp->sampleLoops == 0)
					zone->mode = 0;
				else if (loop->type == WLOOP_TYPE_FORWARD)
					zone->mode = 1;
				else if (loop->type == WLOOP_TYPE_RELEASE)
					zone->mode = 3;
				zone->rate = wv->wvfmt.sampleRate;
				zone->loopStart = loop->start;
				zone->loopLen = loop->length;
				zone->loopEnd = zone->loopStart + zone->loopLen;
				zone->tableEnd = wv->sampsize;
				zone->sample = sbnk->GetSample(wv->index, 0);
				zone->sampleNdx = wv->index;
				zone->name = wv->info.GetInfo(DLS_INAM_CHUNK);
				// validate loop points
				if (zone->loopStart < 0)
					zone->loopStart = 0;
				else if (zone->loopStart >= zone->tableEnd)
					zone->loopStart = zone->tableEnd;
				if (zone->loopEnd < zone->loopStart)
					zone->loopEnd = zone->loopStart;
				else if (zone->loopEnd > zone->tableEnd)
					zone->loopEnd = zone->tableEnd;
			}

			// Apply instrument articulation
			art = 0;
			while ((art = ins->lart.EnumArt(art)) != 0)
				ApplyArt(zone, art);

			// Apply zone articulation
			art = 0;
			while ((art = rgn->lart.EnumArt(art)) != 0)
				ApplyArt(zone, art);

			if (rgn->wlnk.phaseGroup != 0 && !(rgn->wlnk.options & F_WAVELINK_PHASE_MASTER))
			{
				// TODO: Set up a link to the phase master zone and take
				// oscillator phase from that other zone at playback.
				// find phase master and get tuning information
/*				DLSInsInfo *ins2 = 0;
				DLSRgnInfo *rgn2 = 0;
				int found = 0;
				while (!found && (ins2 = info.ins.EnumIns(ins2)) != 0)
				{
					rgn2 = 0;
					while (!found && (rgn2 = ins->rgnlist.EnumRgn(rgn2)) != 0)
					{
						if (rgn->wlnk.phaseGroup == rgn2->wlnk.phaseGroup)
						{
							if (rgn2->wlnk.options & F_WAVELINK_PHASE_MASTER)
							{
								DLSWaveInfo *wv2 = info.wvpl.FindWave(rgn2->wlnk.tableIndex);
								zone->rate = wv2->wvfmt.sampleRate;
								if (rgn->wsmpValid)
								{
									zone->keyNum = rgn2->wsmp.unityNote;
									zone->cents = -rgn2->wsmp.fineTune;
								}
								else
								{
									zone->keyNum = wv2->wvsmp.unityNote;
									zone->cents = -wv2->wvsmp.fineTune;
								}
								// Replace articulations to DST_PITCH.
								art = 0;
								while ((art = rgn2->lart.EnumArt(art)) != 0)
								{
									dlsConnection *ci = art->info;
									for (bsUint32 n = 0; n < art->connections; n++)
									{
										if (ci->destination == CONN_DST_PITCH)
											AddModulator(zone, ci);
										ci++;
									}
								}
								found = 1;
							}
						}
					}
				}*/
			}

			// DLS2 2.11 - this is done after articulators since
			// the spec says this overrides "any parameters in the articulation data"
			/*if (rgn->wlnk.options & F_WAVELINK_MULTICHANNEL)
			{
				switch (rgn->wlnk.channel)
				{
				case 0: // left
					zone->pan = = -500;
					break;
				case 1: // right
					zone->pan = 500;
					break;
				case 6: // left of center
					zone->pan = -200;
					break;
				case 7: // right of center
					zone->pan = 200;
					break;
				}
			}*/

			zone->recCents = (zone->keyNum * 100) + zone->cents;
			zone->recFreq = 440.0 * SoundBank::GetPow2n1200(zone->recCents - 6900);
			zone->recPeriod = (bsInt32) (zone->rate / zone->recFreq);

			grp->AddZone(zone);
		}
		instr->loaded = preload;
	}

	sbnk->Optimize();
	return sbnk;
}

void DLSFile::ApplyArt(SBZone *zone, DLSArtInfo *art)
{
	dlsConnection *ci = art->info;

	for (bsUint32 n = 0; n < art->connections; n++)
	{
		float scale = DLSScale(ci->scale);
		switch (ci->source)
		{
		case CONN_SRC_NONE:
			switch (ci->destination)
			{
			case CONN_DST_RESERVED:
			case CONN_DST_NONE:
			case CONN_DST_PITCH:
				break;
			case CONN_DST_ATTENUATION:
				if (scale < 0.0)
					scale = -scale;
				zone->initAtten = scale;
				break;
			case CONN_DST_EG1_DELAYTIME:
				zone->volEg.delay = scale;
				break;
			case CONN_DST_EG1_ATTACKTIME:
				zone->volEg.attack = scale;
				break;
			case CONN_DST_EG1_HOLDTIME:
				zone->volEg.hold = scale;
				break;
			case CONN_DST_EG1_DECAYTIME:
				zone->volEg.decay = scale;
				break;
			case CONN_DST_EG1_RELEASETIME:
				zone->volEg.release = scale;
				break;
			case CONN_DST_EG1_SUSTAINLEVEL:
				zone->volEg.sustain = scale;
				break;
			case CONN_DST_EG1_RESERVED:
				break;
			case CONN_DST_EG1_SHUTDOWNTIME:
				zone->volEg.shutdown = SoundBank::EnvRate(ci->scale);
				break;

			case CONN_DST_EG2_DELAYTIME:
				zone->modEg.delay = scale;
				break;
			case CONN_DST_EG2_ATTACKTIME:
				zone->modEg.attack = scale;
				break;
			case CONN_DST_EG2_HOLDTIME:
				zone->modEg.hold = scale;
				break;
			case CONN_DST_EG2_DECAYTIME:
				zone->modEg.decay = scale;
				break;
			case CONN_DST_EG2_RELEASETIME:
				zone->modEg.release = scale;
				break;
			case CONN_DST_EG2_SUSTAINLEVEL:
				zone->modEg.sustain = scale;
				break;
			case CONN_DST_EG2_RESERVED:
				break;

			case CONN_DST_LFO_FREQUENCY:
				if (scale < -12000)
					scale = -12000;
				zone->modLfo.rate = scale;
				break;
			case CONN_DST_VIB_FREQUENCY:
				if (scale < -12000)
					scale = -12000;
				zone->vibLfo.rate = scale;
				break;
			case CONN_DST_LFO_STARTDELAY:
				zone->modLfo.delay = scale;
				break;
			case CONN_DST_VIB_STARTDELAY:
				zone->vibLfo.delay = scale;
				break;

			case CONN_DST_PAN:
				zone->pan = scale;
				break;
			case CONN_DST_FLT_CUTOFF:
				if (scale >= 5535 && scale <= 11921.0)
				{
					zone->genFlags |= SBGEN_FILTERX;
					zone->filtFreq = scale;
				}
				break;
			case CONN_DST_FLT_Q:
				if (scale >= 0 && scale <= 22.5)
					zone->filtQ = scale;
				break;
			}
			break;
		case CONN_SRC_LFO:
			if (ci->control == CONN_SRC_NONE)
			{
				if (ci->destination == CONN_DST_FLT_CUTOFF)
				{
					zone->modLfoFlt = scale;
					zone->genFlags |= SBGEN_FILTERX|SBGEN_LFO2X;
				}
				else if (ci->destination == CONN_DST_GAIN)
				{
					zone->modLfoVol = scale;
					zone->genFlags |= SBGEN_LFO2X;
				}
				else if (ci->destination == CONN_DST_PITCH)
				{
					zone->modLfoFrq = scale;
					zone->genFlags |= SBGEN_LFO2X;
				}
				else
					AddModulator(zone, ci);
			}
			else
				AddModulator(zone, ci);
			break;
		case CONN_SRC_VIBRATO:
			if (ci->control == CONN_SRC_NONE)
			{
				if (ci->destination == CONN_DST_PITCH)
				{
					zone->vibLfoFrq = scale;
					zone->genFlags |= SBGEN_LFO1X;
				}
				else
					AddModulator(zone, ci);
			}
			else
				AddModulator(zone, ci);
			break;
		case CONN_SRC_EG2:
			if (ci->control == CONN_SRC_NONE)
			{
				if (ci->destination == CONN_DST_FLT_CUTOFF)
				{
					zone->modEnvFlt = scale;
					zone->genFlags |= SBGEN_FILTERX|SBGEN_EG2X;
				}
				else if (ci->destination == CONN_DST_PITCH)
				{
					zone->modEnvFrq = scale;
					zone->genFlags |= SBGEN_EG2X;
				}
				else
					AddModulator(zone, ci);
			}
			else
				AddModulator(zone, ci);
			break;

		case CONN_SRC_KEYONVELOCITY:
			switch (ci->destination)
			{
			case CONN_DST_ATTENUATION:
				zone->velScale = scale;
				break;
			case CONN_DST_EG1_ATTACKTIME:
				zone->volEg.velAttack = scale;
				break;
			case CONN_DST_EG2_ATTACKTIME:
				zone->modEg.velAttack = scale;
				break;
			}
			break;

		case CONN_SRC_KEYNUMBER:
			if (ci->destination == CONN_DST_EG1_DECAYTIME)
				zone->volEg.keyDecay = scale;
			else if (ci->destination == CONN_DST_EG2_DECAYTIME)
				zone->modEg.keyDecay = scale;
			break;

		case CONN_SRC_EG1:
			if (ci->destination != CONN_DST_ATTENUATION)
				AddModulator(zone, ci);
			break;

		case CONN_SRC_PITCHWHEEL:
			if (ci->destination != CONN_DST_PITCH)
				AddModulator(zone, ci);
			break;
		case CONN_SRC_CC1:
			// this is considered a control, not a source.
			// The CC value is normalized and multiplied
			// by a scale appropriate to the destination
			// then added to a scaled LFO output.
			//AddModulator(zone, ci);
			break;
		default:
			AddModulator(zone, ci);
			break;
		}
		ci++;
	}
}

void DLSFile::AddModulator(SBModList *list, dlsConnection *ci)
{
	SBModInfo *mi = list->GetModInfo(MapSource(ci->source), MapSource(ci->control), MapDestination(ci->destination));
	if (mi != NULL)
	{
		if (ci->scale == 0.0)
		{
			mi->Remove();
			delete mi;
			return;
		}
	}
	else
		mi = list->AddModInfo();
	InitModulator(mi, ci);
}

void DLSFile::InitModulator(SBModInfo *mi, dlsConnection *ci)
{
	mi->srcOp = MapSource(ci->source);;
	mi->srcNf = (ci->transform >> 10) & 0x3F;
	mi->ctlOp = MapSource(ci->control);
	mi->ctlNf = (ci->transform >> 4) & 0x3f;
	mi->dstOp = MapDestination(ci->destination);;
	mi->trnOp = 0;
	mi->scale = MapScale(mi->dstOp, ci->scale);
}

float DLSFile::MapScale(short destination, bsInt32 scale)
{
	switch (destination)
	{
	case SBGEN_VOLUME:
		if (scale < 0.0)
			scale = -scale;
		break;
	case SBGEN_REVERB:
		return DLSScale(scale) * 0.5;
	}
	return DLSScale(scale);
}

short DLSFile::MapDestination(short destination)
{
	switch (destination)
	{
	case CONN_DST_PITCH:
		return SBGEN_PITCH;
	case CONN_DST_ATTENUATION:
		return SBGEN_VOLUME;
	case CONN_DST_EG1_DELAYTIME:
		return SBGEN_EG1DLY;
	case CONN_DST_EG1_ATTACKTIME:
		return SBGEN_EG1ATK;
	case CONN_DST_EG1_HOLDTIME:
		return SBGEN_EG1HOLD;
	case CONN_DST_EG1_DECAYTIME:
		return SBGEN_EG1DEC;
	case CONN_DST_EG1_RELEASETIME:
		return SBGEN_EG1REL;
	case CONN_DST_EG1_SUSTAINLEVEL:
		return SBGEN_EG1SUS;
	case CONN_DST_EG2_DELAYTIME:
		return SBGEN_EG2DLY;
	case CONN_DST_EG2_ATTACKTIME:
		return SBGEN_EG2ATK;
	case CONN_DST_EG2_HOLDTIME:
		return SBGEN_EG2HOLD;
	case CONN_DST_EG2_DECAYTIME:
		return SBGEN_EG2DEC;
	case CONN_DST_EG2_RELEASETIME:
		return SBGEN_EG2REL;
	case CONN_DST_EG2_SUSTAINLEVEL:
		return SBGEN_EG2SUS;
	case CONN_DST_LFO_FREQUENCY:
		return SBGEN_LFO1FRQ;
	case CONN_DST_VIB_FREQUENCY:
		return SBGEN_LFO2FRQ;
	case CONN_DST_LFO_STARTDELAY:
		return SBGEN_LFO1DLY;
	case CONN_DST_VIB_STARTDELAY:
		return SBGEN_LFO2DLY;
	case CONN_DST_PAN:
		return SBGEN_PAN;
	case CONN_DST_FLT_CUTOFF:
		return SBGEN_FILTER;
	case CONN_DST_FLT_Q:
		return SBGEN_FILTQ;
	case CONN_DST_CHORUS:
		return SBGEN_CHORUS;
	case CONN_DST_REVERB:
		return SBGEN_REVERB;
	}
	return SBGEN_NONE;
}

short DLSFile::MapSource(short source)
{
	switch (source)
	{
	case CONN_SRC_NONE:
		return SBGEN_NONE;
	case CONN_SRC_KEYONVELOCITY:
		return SBGEN_VELN;
	case CONN_SRC_KEYNUMBER:
		return SBGEN_KEYN;
	case CONN_SRC_LFO:
		return SBGEN_LFO1;
	case CONN_SRC_VIBRATO:
		return SBGEN_LFO2;
	case CONN_SRC_EG1:
		return SBGEN_EG1;
	case CONN_SRC_EG2:
		return SBGEN_EG2;
	case CONN_SRC_PITCHWHEEL:
		return SBGEN_PITCHWHL;
	case CONN_SRC_CHANNELPRESSURE:
		return SBGEN_CHNLAT;
	case CONN_SRC_POLYPRESSURE:
		return SBGEN_PKEYAT;
	case CONN_SRC_RPN0:
		return SBGEN_RPN0;
//	case CONN_SRC_RPN1:
//		return SBGEN_RPN1;
//	case CONN_SRC_RPN2:
//		return SBGEN_RPN2;
	case CONN_SRC_CC1:  // Modulation wheel
	case CONN_SRC_CC7:  // Volume
	case CONN_SRC_CC10: // Pan
	case CONN_SRC_CC11: // Expression
	case CONN_SRC_CC91: // Reverb
	case CONN_SRC_CC93: // Chorus
	default:
		if (source & SBGEN_CHCTL)
			return source;
		break;
	}
	return SBGEN_NONE;
}

