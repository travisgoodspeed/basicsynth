/////////////////////////////////////////////////////////////
// BasicSynth Library
//
/// @file MIDIDefs.h MIDI Message definitions
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
/////////////////////////////////////////////////////////////
/// @addtogroup grpMIDI
//@{
#ifndef _MIDIDEFS_H_
#define _MIDIDEFS_H_

// Define manifest constants for MIDI messages
// to make things easier to read.
#define MAX_MIDI_CHNL 16
#define MIDI_EVTMSK  0xF0
#define MIDI_CHNMSK  0x0F
#define MIDI_MSGBIT  0x80

#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON  0x90
#define MIDI_KEYAT   0xA0
#define MIDI_CTLCHG  0xB0
#define MIDI_PRGCHG  0xC0
#define MIDI_CHNAT   0xD0
#define MIDI_PWCHG   0xE0
#define MIDI_SYSEX   0xF0
#define MIDI_TMCODE  0xF1
#define MIDI_SNGPOS  0xF2
#define MIDI_SNGSEL  0xF3
#define MIDI_TUNREQ  0xF6
#define MIDI_ENDEX   0xF7
#define MIDI_TMCLK   0xF8
#define MIDI_START   0xFA
#define MIDI_CONT    0xFB
#define MIDI_STOP    0xFC
#define MIDI_ACTSNS  0xFE
#define MIDI_META    0xFF

#define MIDI_CTRL_BANK 0x00
#define MIDI_CTRL_MOD  0x01
#define MIDI_CTRL_BRTH 0x02
#define MIDI_CTRL_FOOT 0x04
#define MIDI_CTRL_PORT 0x05
#define MIDI_CTRL_DATA 0x06
#define MIDI_CTRL_VOL  0x07
#define MIDI_CTRL_BAL  0x08
#define MIDI_CTRL_PAN  0x0A
#define MIDI_CTRL_EXPR 0x0B
#define MIDI_CTRL_FX1  0x0C
#define MIDI_CTRL_FX2  0x0D
#define MIDI_CTRL_BANK_LSB 0x20
#define MIDI_CTRL_MOD_LSB  0x21
#define MIDI_CTRL_BRTH_LSB 0x22
#define MIDI_CTRL_FOOT_LSB 0x24
#define MIDI_CTRL_PORT_LSB 0x25
#define MIDI_CTRL_DATA_LSB 0x26
#define MIDI_CTRL_VOL_LSB  0x27
#define MIDI_CTRL_BAL_LSB  0x28
#define MIDI_CTRL_PAN_LSB  0x2A
#define MIDI_CTRL_EXPR_LSB 0x2B
#define MIDI_CTRL_FX1_LSB  0x2C
#define MIDI_CTRL_FX2_LSB  0x2D

// Foot switches
#define MIDI_CTRL_SUS_ON  0x40
#define MIDI_CTRL_PORT_ON 0x41
#define MIDI_CTRL_SOS_ON  0x42
#define MIDI_CTRL_SFT_ON  0x43
#define MIDI_CTRL_LEG_ON  0x44

#define MIDI_CTRL_FX1DPTH 0x5B
#define MIDI_CTRL_FX2DPTH 0x5C
#define MIDI_CTRL_FX3DPTH 0x5D
#define MIDI_CTRL_FX4DPTH 0x5E
#define MIDI_CTRL_FX5DPTH 0x5F

#define MIDI_CTRL_DATAI  0x60
#define MIDI_CTRL_DATAD  0x61
#define MIDI_CTRL_NRPNMSB 0x62
#define MIDI_CTRL_NRPNLSB 0x63
#define MIDI_CTRL_RPNLSB 0x64
#define MIDI_CTRL_RPNMSB 0x65

#define MIDI_CTRL_ALLSOUNDOFF 0x78
#define MIDI_CTRL_RESET 0x79
#define MIDI_CTRL_LOCALONOFF 0x7A
#define MIDI_CTRL_ALLNOTESOFF 0x7B
#define MIDI_CTRL_OMNI_OFF 0x7C
#define MIDI_CTRL_OMNI_ON 0x7D
#define MIDI_CTRL_POLY_ONOFF 0x7E
#define MIDI_CTRL_POLY_ON 0x7F

#define MIDI_META_TEXT 0x01
#define MIDI_META_CPYR 0x02
#define MIDI_META_TRK  0x03
#define MIDI_META_INST 0x04
#define MIDI_META_LYRK 0x05
#define MIDI_META_MRKR 0x06
#define MIDI_META_CUE  0x07
#define MIDI_META_CHNL 0x20
#define MIDI_META_EOT  0x2F
#define MIDI_META_TMPO 0x51
#define MIDI_META_TMSG 0x58
#define MIDI_META_KYSG 0x59

/////////////////// BEGIN RIFF STRUCTURES //////////////////////////
#ifndef FOURCC
#define FOURCC(a,b,c,d) (a|(b<<8)|(c<<16)|(d<<24))
#endif

// chunk header ids found in the SF2 file
#define SMF_MTHD_CHUNK FOURCC('M','T','h','d')
#define SMF_MTRK_CHUNK FOURCC('M','T','r','k')

//@}
#endif
