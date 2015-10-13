/// @file BasicSynth.h Master include file for BasicSynth common library.
#include <SynthDefs.h>
#include <SynthList.h>
#include <SynthFile.h>
#include <SynthMutex.h>
#include <WaveTable.h>
#include <WaveFile.h>

#include <GenWave.h>
#include <GenWaveWT.h>
#include <GenNoise.h>
#include <GenWaveX.h>
#include <GenWaveDSF.h>

#include <EnvGen.h>
#include <EnvGenSeg.h>

#include <BiQuad.h>
#include <AllPass.h>
#include <Filter.h>
#include <DynFilter.h>

#include <DelayLine.h>
#include <Flanger.h>
#include <Reverb.h>

#include <Mixer.h>
#include <XmlWrap.h>
#include <SeqEvent.h>
#include <MIDIDefs.h>
#include <MIDIControl.h>
#include <Instrument.h>
#include <Sequencer.h>
#include <SequenceFile.h>
#include <MIDIInput.h>

//////////////////////////////////////////
// These two files are not included by
// default since they are used only for live 
// playback of sound
// Version 2 - Player is deprecated, use Sequencer::Play()
//////////////////////////////////////////
//#include <Player.h>
//#include <WaveOutDirect.h>
//////////////////////////////////////////


