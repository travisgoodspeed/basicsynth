//////////////////////////////////////////////////////////////////////
/// @file GMPlayer.cpp Implementation of the General MIDI player
//
// This instrument provides an emulation of a MIDI keyboard using
// SoundFont or DLS GM file.
//
// The implementation is:
//  - Vol EG affects attenuation
//  - Vib LFO affects pitch
//  - Mod EG affects pitch or filter
//  - Mod LFO affects pitch, filter or attenuation
//  - Mod Wheel is applied to LFOs
//  - Pitch bend affects pitch, scaling set by RPN0 or fixed at 200 cents
//  - Scaling for volume and expression is fixed at 960cb
//  - Pitchbend, CC7 (volume) and CC1 (expression) apply at the instrument, not each zone.
//  - Sustain, sostenuto and soft pedas are applied.
//  - Pan is applied at the zone level, but may be disabled.
//  - Only default transforms for the above are used.
//  - Other modulators with MIDI inputs and non-default transforms are ignored
//  - Unit generators are disabled if the associated modulation scale is 0.
//
// See SFFile and DLSFile for more info.
//
// Copyright 2009-2010, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#include "Includes.h"
#include "GMPlayer.h"

/// Create an instance of the GMPlayer instrument
Instrument *GMPlayer::InstrFactory(InstrManager *m, Opaque tmplt)
{
	return new GMPlayer((GMPlayer *)tmplt, m);
}

/// Create an event object for GMManager
SeqEvent *GMPlayer::EventFactory(Opaque tmplt)
{
	VarParamEvent *ep = new VarParamEvent;
	ep->maxParam = 23;
	return (SeqEvent*)ep;
}


/// Allocate a variable parameters object (needed for editor)
VarParamEvent *GMPlayer::AllocParams()
{
	return (VarParamEvent*) EventFactory(0);
}

void GMPlayer::SetSoundFile(const char *b)
{
	SetSoundBank(SoundBank::FindBank(b));
}

void GMPlayer::SetSoundBank(SoundBank *b)
{
	if (sndbnk)
		sndbnk->Unlock();
	sndbnk = b;
	if (sndbnk)
	{
		sndbnk->Lock();
		sndFile = sndbnk->name;
	}
	else
		sndFile = "";
}

/////////////////////////////////////////////////////////////////

int GMPlayer::Load(XmlSynthElem *parent)
{
	char *cval;

	XmlSynthElem elem(parent->Document());
	XmlSynthElem *next = parent->FirstChild(&elem);
	while (next != NULL)
	{
		if (elem.TagMatch("gm"))
		{
			elem.GetAttribute("local", localVals);
			elem.GetAttribute("bank", bankValue);
			elem.GetAttribute("prog", progValue);
			if (elem.GetAttribute("attn", attnScale) == 0)
				attnScale = 1.0;
			elem.GetContent(&cval);
			if (cval)
				sndFile.Attach(cval);
		}
		next = elem.NextSibling(&elem);
	}
	if (sndFile.Length() > 0)
	{
		sndbnk = SoundBank::FindBank(sndFile);
		if (sndbnk)
			sndbnk->Lock();
	}

	return 0;
}

int GMPlayer::Save(XmlSynthElem *parent)
{
	XmlSynthElem elem(parent->Document());

	if (!parent->AddChild("gm", &elem))
		return -1;

	elem.SetAttribute("local", localVals);
	elem.SetAttribute("bank", bankValue);
	elem.SetAttribute("prog", progValue);
	elem.SetAttribute("attn", attnScale);
	elem.SetContent(sndFile);

	return 0;
}

int GMPlayer::SetParams(VarParamEvent *params)
{
	int err = 0;
	bsInt16 *id = params->idParam;
	float *valp = params->valParam;
	int n = params->numParam;
	while (n-- > 0)
		err += SetParam(*id++, *valp++);
	return err;
}

int GMPlayer::SetParam(bsInt16 idval, float val)
{
	switch (idval)
	{
	case GMPLAYER_FLAGS:
		localVals = (bsInt32) val;
		break;
	case GMPLAYER_BANK:
		bankValue = (bsInt16) val;
		break;
	case GMPLAYER_PROG:
		progValue = (bsInt16) val;
		break;
	case GMPLAYER_ATTN:
		attnScale = val;
		break;
	default:
		return 1;
	}
	return 0;
}

int GMPlayer::GetParams(VarParamEvent *params)
{
	params->SetParam(GMPLAYER_FLAGS, (float) localVals);
	params->SetParam(GMPLAYER_BANK, (float) bankValue);
	params->SetParam(GMPLAYER_PROG, (float) progValue);
	params->SetParam(GMPLAYER_ATTN, (float) attnScale);
	return 0;
}

int GMPlayer::GetParam(bsInt16 idval, float *val)
{
	switch (idval)
	{
	case GMPLAYER_FLAGS:
		*val = (float) localVals;
		break;
	case GMPLAYER_BANK:
		*val = (float) bankValue;
		break;
	case GMPLAYER_PROG:
		*val = (float) progValue;
		break;
	case GMPLAYER_ATTN:
		*val = (float) attnScale;
		break;
	default:
		return 1;
	}
	return 0;
}

static InstrParamMap gmPlayerParams[] =
{
	{"attn",  GMPLAYER_ATTN },
	{"bank",  GMPLAYER_BANK },
	{"local", GMPLAYER_FLAGS },
	{"prog",  GMPLAYER_PROG },
};

bsInt16 GMPlayer::MapParamID(const char *name, Opaque tmplt)
{
	return InstrParamMap::SearchParamID(name, gmPlayerParams, sizeof(gmPlayerParams)/sizeof(InstrParamMap));
}

const char *GMPlayer::MapParamName(bsInt16 id, Opaque tmplt)
{
	return InstrParamMap::SearchParamName(id, gmPlayerParams, sizeof(gmPlayerParams)/sizeof(InstrParamMap));
}

///////////////////////////////////////////////////////////////////////////////

GMPlayer::GMPlayer()
{
	im = 0;
	instr = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	sostenuto = 0;
	zoneList = 0;
	pendingStop = 0;
	sndbnk = 0;
	pitchBend = 0;
	ctrlAtten = 0;
	rvrbAmnt = 0;
	attnScale = 1.0;
	localVals = 0;
	bankValue = 0;
	progValue = 0;
}

GMPlayer::GMPlayer(GMPlayer *tmplt, InstrManager *m)
{
	im = m;
	instr = 0;
	chnl = 0;
	mkey = 69;
	novel = 0;
	sostenuto = 0;
	zoneList = 0;
	pendingStop = 0;
	pitchBend = 0;
	ctrlAtten = 0;
	rvrbAmnt = 0;
	if (tmplt)
	{
		sndbnk = tmplt->sndbnk;
		if (sndbnk != 0)
			sndbnk->Lock();
		localVals = tmplt->localVals;
		bankValue = tmplt->bankValue;
		progValue = tmplt->progValue;
		attnScale = tmplt->attnScale;
	}
	else
	{
		sndbnk = 0;
		localVals = 0;
		bankValue = 0;
		progValue = 0;
		attnScale = 1.0;
	}
}

GMPlayer::~GMPlayer()
{
	if (sndbnk)
		sndbnk->Unlock();
	ClearZones();
}

void GMPlayer::ClearZones()
{
	GMPlayerZone *pz;
	while ((pz = zoneList) != 0)
	{
		zoneList = pz->next;
		delete pz;
	}
	zoneList = 0;
}

/// Start playing a note.
/// Locate the sound bank instrument, then the zone(s).
/// Initialize oscillators and envelopes.
void GMPlayer::Start(SeqEvent *se)
{
	VarParamEvent *evt = (VarParamEvent *)se;
	chnl = evt->chnl;
	mkey = evt->pitch + 12;
	novel = evt->noteonvel;
	if (novel == 0)
		novel = 100;
	sostenuto = im->GetSwitch(chnl, MIDI_CTRL_SOS_ON);
	exclNote = 0;

	bsInt16 bnk;
	bsInt16 prg;
	if (localVals & GMPLAYER_LOCAL_BANK)
		bnk = bankValue;
	else
		bnk = im->GetBank(chnl);
	if (localVals & GMPLAYER_LOCAL_PROG)
		prg = progValue;
	else
		prg = im->GetPatch(chnl);

	SetVolume();
	rvrbAmnt = (AmpValue) im->GetCCN(chnl, MIDI_CTRL_FX1) / 127.0;

	if (sndbnk)
		instr = sndbnk->GetInstr(bnk, prg);
	if (instr)
	{
		SBZoneGroup *grp = 0;
		while ((grp = instr->EnumGroups(grp)) != 0)
		{
			if (grp->Match(mkey, novel))
			{
				SBZone *zone;
				SBZoneRef *ref = grp->map[mkey];
				while (ref)
				{
					zone = ref->zone;
					if (zone->Match(mkey, novel))
					{
						GMPlayerZone *pz = new GMPlayerZone(zone, im, this);
						if (zoneList)
							pz->next = zoneList;
						zoneList = pz;
						pz->Initialize(chnl, mkey, novel);
						exclNote |= zone->exclNote;
					}
					ref = ref->next;
				}
			}
		}
		if (exclNote)
		{
			im->ExclNoteOn((chnl << 4) | (exclNote & 0xf), this);
		}
	}
}

void GMPlayer::Param(SeqEvent *se)
{
	GMPlayerZone *pz;
	if (se->type == SEQEVT_CONTROL)
	{
		ControlEvent *evt = (ControlEvent *)se;
		//bsInt16 ch = evt->mmsg & 0x0f;
		switch (evt->mmsg & 0xf0)
		{
		case MIDI_CTLCHG:
			switch (evt->ctrl)
			{
			case MIDI_CTRL_SUS_ON:
			case MIDI_CTRL_SOS_ON:
				if (pendingStop)
					Stop();
				break;
			case MIDI_CTRL_VOL:
			case MIDI_CTRL_EXPR:
			case MIDI_CTRL_SFT_ON:
				SetVolume();
				break;
			case MIDI_CTRL_MOD:
				pz = zoneList;
				while (pz)
				{
					pz->SetLFO();
					pz = pz->next;
				}
				break;
			case MIDI_CTRL_PAN:
				if (localVals & GMPLAYER_LOCAL_PAN)
				{
					pz = zoneList;
					while (pz)
					{
						pz->SetPanning();
						pz = pz->next;
					}
				}
				break;
			case MIDI_CTRL_FX1:
				rvrbAmnt = (AmpValue) im->GetCCN(chnl, MIDI_CTRL_FX1);
				break;
			}
			break;
		case MIDI_PWCHG:
			pitchBend = im->GetPitchbendC(chnl);
			break;
		}
	}
	else
	{
		VarParamEvent *evt = (VarParamEvent *)se;
		if (mkey != (evt->pitch + 12))
		{
			ClearZones();
			Start(se);
		}
	}
}

void GMPlayer::Stop()
{
	if (im->GetSwitch(chnl, MIDI_CTRL_SUS_ON)
     || (!sostenuto && im->GetSwitch(chnl, MIDI_CTRL_SOS_ON)))
	{
		pendingStop = 1;
		return;
	}
	GMPlayerZone *pz = zoneList;
	while (pz)
	{
		pz->osc.Release();
		pz->volEnv.AdjustRelease();
		pz->modEnv.Release();
		pz = pz->next;
	}
	pendingStop = 0;
}

void GMPlayer::Cancel()
{
	GMPlayerZone *pz = zoneList;
	while (pz)
	{
		pz->volEnv.SetRelease(pz->zone->volEg.shutdown);
		pz->volEnv.AdjustRelease();
		pz->modEnv.Release();
		pz->osc.Release();
		pz = pz->next;
	}
	pendingStop = 0;
}

int  GMPlayer::IsFinished()
{
//	if (pendingStop)
//		return 0;
	GMPlayerZone *pz = zoneList;
	while (pz)
	{
		if (!pz->volEnv.IsFinished())
		{
			if (!pz->osc.IsFinished())
				return 0;
		}
		pz = pz->next;
	}
	return 1;
}


/// Produce the next sample.
void GMPlayer::Tick()
{
	GMPlayerZone *pz = zoneList;
	if (pz == 0)
		return;

	// process each zone
	do
	{
		pz->Gen();
	} while ((pz = pz->next) != 0);
}

void GMPlayer::Destroy()
{
	if (exclNote)
		im->ExclNoteOff((chnl << 4) | (exclNote & 0xf), this);
	delete this;
}

void GMPlayer::GMPlayerZone::Initialize(bsInt16 ch, bsInt16 key, bsInt16 vel)
{
	chnl = ch;
	if (zone->fixedKey != -1)
		noKey = zone->fixedKey;
	else
		noKey = key & 0x7f;
	if (zone->fixedVel != -1)
		noVel = zone->fixedVel;
	else
		noVel = vel & 0x7f;
	if (player->localVals & GMPLAYER_LOCAL_PAN)
		localPan = 1;
	else
		localPan = 0;
	if (player->localVals & GMPLAYER_LOCAL_VOL)
		localVol = 1;
	else
		localVol = 0;

	genFlags = zone->genFlags;

	// Initialize parameter values
	float veln = SoundBank::posLinear[noVel];
	float keyn = SoundBank::posLinear[noKey];
	initAtten = zone->initAtten * player->attnScale;
	initAtten += (SoundBank::posConcave[127 - noVel] * zone->velScale);

	// Initialize oscillator.
	// Calculate cents ratio for phase increment updates.
	FrqValue smpl;
	if (zone->rate != synthParams.isampleRate)
	{
		double wsrCents = 1200.0 * SoundBank::log2((double)zone->rate/440.0);
		double srCents = 1200.0 * SoundBank::log2((double)synthParams.sampleRate/440.0);
		smpl = FrqValue(wsrCents - srCents);
	}
	else
		smpl = 0;
	FrqValue adjKey = FrqValue(noKey + zone->coarseTune - zone->keyNum);
	FrqValue adjCents = FrqValue(zone->fineTune) * 0.01;
	initPitch = (FrqValue(zone->scaleTune) * (adjKey + adjCents)) - zone->cents + smpl;
	osc.InitSB(zone, SoundBank::GetPow2n1200(initPitch));

	// Initialize LFO
	vibLfo.InitWT(SoundBank::Frequency(zone->vibLfo.rate), WT_SIN);
	vibDelay = (bsInt32) (SoundBank::EnvRate(zone->vibLfo.delay) * synthParams.sampleRate);

	modLfo.InitWT(SoundBank::Frequency(zone->modLfo.rate), WT_SIN);
	modDelay = (bsInt32) (SoundBank::EnvRate(zone->modLfo.delay) * synthParams.sampleRate);

	// Initialize volume envelope
	FrqValue km;
	if (zone->genFlags & SBGEN_SF2)
		km = FrqValue(60.0) - keyn;
	else if (zone->genFlags & SBGEN_DLS)
		km = keyn;
	else
		km = 0;

	volEnv.SetDelay(SoundBank::EnvRate(zone->volEg.delay));
	volEnv.SetAttack(SoundBank::EnvRate(zone->volEg.attack + (veln * zone->volEg.velAttack)));
	volEnv.SetHold(SoundBank::EnvRate(zone->volEg.hold + (km * zone->volEg.keyHold)));
	volEnv.SetDecay(SoundBank::EnvRate(zone->volEg.decay + (km * zone->volEg.keyDecay)));
	if (genFlags & SBGEN_SF2)
	{
		if (zone->volEg.sustain <= 0)
			volEnv.SetSustain(1.0);
		else if (zone->volEg.sustain < 960.0)
			volEnv.SetSustain((960.0 - zone->volEg.sustain) / 960.0);
		else
			volEnv.SetSustain(0.0);
	}
	else
		volEnv.SetSustain(zone->volEg.sustain * 0.001);
	volEnv.SetRelease(SoundBank::EnvRate(zone->volEg.release));
	volEnv.Reset(0);

	// Initialize modulation envelope
	if (genFlags & SBGEN_EG2X)
	{
		modEnv.SetDelay(SoundBank::EnvRate(zone->modEg.delay));
		modEnv.SetAttack(SoundBank::EnvRate(zone->modEg.attack + (veln * zone->modEg.velAttack)));
		modEnv.SetHold(SoundBank::EnvRate(zone->modEg.hold + (km * zone->modEg.keyHold)));
		modEnv.SetDecay(SoundBank::EnvRate(zone->modEg.decay + (km * zone->modEg.keyDecay)));
		if (genFlags & SBGEN_SF2)
		{
			if (zone->modEg.sustain <= 0)
				modEnv.SetSustain(1.0);
			else if (zone->modEg.sustain >= 1000.0)
				modEnv.SetSustain(0.0);
			else
				modEnv.SetSustain(1.0f - (zone->modEg.sustain * 0.001));
		}
		else
			modEnv.SetSustain(zone->modEg.sustain * 0.001);
		modEnv.SetRelease(SoundBank::EnvRate(zone->modEg.release));
		modEnv.Reset(0);
	}

	// Initialize filter
	// Filters are "expensive" - only init if used.
	// the flag gets set if there is a modulator
	// applied to the filter and the frequency
	// is below the maximum.
	float initFilter = zone->filtFreq;// + (SoundBank::posLinear[127-noVel] * zone->velFlt);
	float dynFilter = zone->modLfoFlt + zone->modLfoMwFlt + zone->modEnvFlt;
	if (initFilter > SoundBank::maxFilter)
		genFlags &= ~SBGEN_FILTERX;
	else if ((initFilter + dynFilter) > SoundBank::minFilter)
		genFlags |= SBGEN_FILTERX;
	if (genFlags & SBGEN_FILTERX)
	{
		if (dynFilter > 0)
			genFlags |= SBGEN_FILTERD;
		fcFlt = (bsInt16)initFilter;
		gainQ = SoundBank::Gain(zone->filtQ)/2.0f;
		filt.CalcCoef(SoundBank::Frequency((bsInt16)initFilter), gainQ);
		filt.Reset();
		// Reduce amplitude for High 'Q' values.
		// See DLS 2.2, sec 1.5.2
		initAtten += zone->filtQ / 2.0f;
	}

	SetLFO();
	SetPanning();
}

void GMPlayer::SetVolume()
{
	ctrlAtten = SoundBank::posConcave[127 - im->GetCC(chnl, MIDI_CTRL_EXPR)];
	if (localVals & GMPLAYER_LOCAL_VOL)
		ctrlAtten += SoundBank::posConcave[127 - im->GetCC(chnl, MIDI_CTRL_VOL)];
	ctrlAtten *= 960.0;
	if (im->GetSwitch(chnl, MIDI_CTRL_SFT_ON))
		ctrlAtten += 30; // 3dB additial attenuation
}

void GMPlayer::GMPlayerZone::SetPanning()
{
	float pan = (float)(im->GetCC(chnl, MIDI_CTRL_PAN) - 64) / 128.0; // normalize [-0.5,+0.5]
	pan += zone->pan / 1000.0;
	if (pan < -0.5)
		pan = -0.5;
	else if (pan > 0.5)
		pan = 0.5;
	panLft = synthParams.sinquad[(int)((0.5 - pan) * synthParams.sqNdx)];
	panRgt = synthParams.sinquad[(int)((0.5 + pan) * synthParams.sqNdx)];
}

void GMPlayer::GMPlayerZone::SetLFO()
{
	float mwval = im->GetCC(chnl, MIDI_CTRL_MOD) / 127.0;
	vibLfoFrq = zone->vibLfoFrq + (zone->vibLfoMwFrq * mwval);
	modLfoFrq = zone->modLfoFrq + (zone->modLfoMwFrq * mwval);
	modLfoFlt = zone->modLfoFlt + (zone->modLfoMwFlt * mwval);
	modLfoVol = zone->modLfoVol + (zone->modLfoMwVol * mwval);
}

void GMPlayer::GMPlayerZone::Gen()
{
	FrqValue pitchVal = initPitch + player->pitchBend;
	AmpValue attenVal = initAtten + player->ctrlAtten;
	FrqValue filtVal = zone->filtFreq;

	if (genFlags & SBGEN_EG2X)
	{
		float eg2 = modEnv.Gen();
		pitchVal += eg2 * zone->modEnvFrq;
		filtVal += eg2 * zone->modEnvFlt;
	}

	if (vibDelay == 0 || --vibDelay == 0)
	{
		pitchVal += vibLfo.Gen() * vibLfoFrq;
	}

	if (genFlags & SBGEN_LFO2X && (modDelay == 0 || --modDelay == 0))
	{
		float lfo = modLfo.Gen();
		pitchVal += lfo * modLfoFrq;
		filtVal += lfo * modLfoFlt;
		attenVal += (1.0 + lfo) * 0.5 * modLfoVol;
	}

#ifdef _DEBUG
	// set breakpoints here to catch bad frequency calculations...
	if (pitchVal < -1200)
		pitchVal = -1200;
	else if (pitchVal >= 25501)
		pitchVal = 25500;
#endif

	// run the oscillator
	osc.UpdatePhaseIncr(SoundBank::GetPow2n1200(pitchVal));
	AmpValue out = osc.Gen();

	// filter
	if (genFlags & SBGEN_FILTERX)
	{
		if (genFlags & SBGEN_FILTERD)
		{
			if (filtVal > SoundBank::maxFilter)
				filtVal = SoundBank::maxFilter;
			bsInt32 fcCents = (bsInt32)filtVal;
			if (fcCents != fcFlt)
			{
				fcFlt = fcCents;
				filt.CalcCoef(SoundBank::Frequency(filtVal), gainQ);
			}
		}
		out = filt.Sample(out);
	}

	// apply envelope and attenuation
	AmpValue eg = volEnv.Gen();
	if (volEnv.GetSegment() > 2)
		eg = SoundBank::Attenuation((1.0 - eg) * 960);

	out *= SoundBank::Attenuation(attenVal) * eg;

	// output
	if (localPan)
	{
		// bypass mixer panning and effects
		im->Output2(chnl, out * panLft, out * panRgt);
		im->FxSend(0, out * player->rvrbAmnt);
	}
	else
		im->Output(chnl, out);
}
