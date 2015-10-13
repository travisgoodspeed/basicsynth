//////////////////////////////////////////////////////////////////////
/// @file GMSynthDLL.h General MIDI Synthesizer
/// This is a prototype and may change significantly in future versions.
//
// Copyright 2009, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
///@{
#ifndef GMSYNTHDLL_H
#define GMSYNTHDLL_H

#ifdef _WIN32
# ifndef STATIC_GMSYNTH
#  ifdef _USRDLL
#   define EXPORT __declspec(dllexport)
#  else
#   define EXPORT __declspec(dllimport)
#  endif
# else
#  define EXPORT
# endif
#endif
#ifdef UNIX
# define EXPORT
#endif

// ID values for MetaText
#define GMSYNTH_META_SEQTEXT  0  ///< MIDI file text
#define GMSYNTH_META_SEQCPYR  1  ///< MIDI file copyright
#define GMSYNTH_META_SEQNAME  2  ///< MIDI file sequence name
#define GMSYNTH_META_TIMESIG  3  ///< MIDI file time signature
#define GMSYNTH_META_KEYSIG   4  ///< MIDI file key signature
#define GMSYNTH_META_SBKNAME  6  ///< SoundBank name (INFO text from file)
#define GMSYNTH_META_SBKCPYR  7  ///< SoundBank copyright
#define GMSYNTH_META_SBKCMNT  8  ///< SoundBank comment
#define GMSYNTH_META_SBKVER   9  ///< SoundBank version

#define GMSYNTH_EVENT_TICK    1
#define GMSYNTH_EVENT_START   2  ///< Sequencer started
#define GMSYNTH_EVENT_STOP    3  ///< Sequencer stopped
#define GMSYNTH_EVENT_PAUSE   4  ///< Sequencer paused
#define GMSYNTH_EVENT_RESUME  5  ///< Sequencer resumed
#define GMSYNTH_EVENT_NOTEON  6  ///< Note start
#define GMSYNTH_EVENT_NOTEOFF 7  ///< Note end
#define GMSYNTH_EVENT_CTLCHG  8  ///< MIDI Control change
#define GMSYNTH_EVENT_TRKON   9  ///< Start a sequencer track
#define GMSYNTH_EVENT_TRKOFF  10  ///< Stop a sequencer track

#define GMSYNTH_MODE_PLAY     0 ///< Play immediate events
#define GMSYNTH_MODE_SEQUENCE 1 ///< Play the sequence once
#define GMSYNTH_MODE_SEQPLAY  2 ///< Play both

#define GMSYNTH_NOERROR       0
#define GMSYNTH_ERR_BADHANDLE 1
#define GMSYNTH_ERR_FILETYPE  2
#define GMSYNTH_ERR_FILEOPEN  3
#define GMSYNTH_ERR_BADID     4
#define GMSYNTH_ERR_GENERATE  5
#define GMSYNTH_ERR_SOUNDBANK 6

/// Function to receive events
typedef void (*GMSYNTHCB)(bsInt32 evtid, bsInt32 count, Opaque arg);

#ifdef __cplusplus

/// Wrapper class for GMSynthDlL.
class EXPORT GMSynth
{
public:
	GMSynth(Opaque w);
	virtual ~GMSynth();

	virtual void SetVolume(float db, float rv);
	virtual void SetCallback(GMSYNTHCB cb, bsInt32 cbRate, Opaque arg);
	virtual void SetWaveDevice(Opaque dev);
	virtual int Unload();
	virtual int LoadSoundBank(const char *alias, const char *fileName, int preload, float scl);
	virtual int GetMetaText(int id, char *txt, size_t len);
	virtual int GetMetaData(int id, long *vals);
	virtual int GetBanks(short *banks, size_t len);
	virtual int GetPreset(short bank, short preset, char *txt, size_t len);
	virtual int LoadSequence(const char *fileName, const char *sbnkName, unsigned short mask);
	virtual int Start(int mode, void *dev);
	virtual int Stop();
	virtual int Pause();
	virtual int Resume();
	virtual int Generate(const char *fileName);
	virtual int MidiIn(int onoff, int device);
	virtual void ImmediateEvent(short mmsg, short val1, short val2);
};

extern "C" {
#endif

int EXPORT GMSynthOpen(Opaque w, bsInt32 sr);
int EXPORT GMSynthClose();
int EXPORT GMSynthUnload();
int EXPORT GMSynthLoadSoundBank(const char *fileName, const char *alias, int preload, float scale);
int EXPORT GMSynthLoadSequence(const char *fileName, const char *sbnkName, unsigned short mask);
int EXPORT GMSynthStart(int mode, float start, float end, void *dev);
int EXPORT GMSynthStop();
int EXPORT GMSynthPause();
int EXPORT GMSynthResume();
int EXPORT GMSynthGenerate(const char *fileName);
int EXPORT GMSynthMetaText(int id, char *txt, size_t len);
int EXPORT GMSynthMetaData(int id, long *vals);
int EXPORT GMSynthSetCallback(GMSYNTHCB cb, bsUint32 cbRate, Opaque arg);
int EXPORT GMSynthMIDIKbdIn(int onoff, int device);
int EXPORT GMSynthMIDIEvent(short mmsg, short val1, short val2);
int EXPORT GMSynthSetVolume(float db, float rv);
int EXPORT GMSynthGetBanks(short *banks, size_t len);
int EXPORT GMSynthGetPreset(short bank, short preset, char *txt, size_t len);
#ifdef __cplusplus
// end extern "C"
}
#endif

#endif
//@}
