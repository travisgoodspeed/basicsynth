
const char *midiMsgName[8] =
{
	"NOTE OFF",
	"NOTE ON",
	"KEY AFTER TOUCH",
	"CONTROL CHANGE",
	"PROGRAM CHANGE",
	"CHANNEL AFTER TOUCH",
	"PITCH WHEEL CHANGE",
	"SYSTEM"
};

const char *midiSysName[16] =
{
	"SYSTEM EXCLUSIVE",
	"TIME CODE",
	"SONG POSITION",
	"SONG SELECT",
	"",
	"",
	"TUNE REQUEST",
	"END EXCLUSIVE",
	"TIME CLOCK",
	""
	"START",
	"CONTINUE",
	"STOP",
	"",
	"ACTIVE SENSE",
	"RESET/META DATA"
};

const char *midiCCName[128] =
{
/* 0*/	"BANK SELECT", "MOD WHEEL", "BREATH", "",
/* 4*/	"FOOT", "PORTAMENTO TIME", "DATA ENTRY MSB", "VOLUME",
/* 8*/	"BALANCE", "", "PAN", "EXPRESSION",
/*12*/	"EFFECT 1", "EFFECT 2", "", "",
/*16*/	"GENERAL PURPOSE 1", "GENERAL PURPOSE 2", "GENERAL PURPOSE 3", "GENERAL PURPOSE 4",
/*20*/	"", "", "", "",
/*24*/	"", "", "", "",
/*28*/	"", "", "", "",
/*32*/	"BANK SELECT LSB", "MOD WHEEL LSB", "BREATH LSB", "",
/*36*/	"FOOT LSB", "PORTAMENTO TIME LSB", "DATA ENTRY LSB", "VOLUME LSB",
/*40*/	"BALANCE LSB", "", "PAN LSB", "EXPRESSION LSB",
/*44*/	"EFFECT 1 LSB", "EFFECT 2 LSB", "", "",
/*48*/	"GENERAL PURPOSE 1 LSB", "GENERAL PURPOSE 2 LSB", "GENERAL PURPOSE 3 LSB", "GENERAL PURPOSE 4 LSB",
/*52*/  "", "", "", "",
/*56*/  "", "", "", "",
/*60*/  "", "", "", "",
/*64*/	"SUSTAIN PEDAL", "PORTAMENTO ON/OFF", "SOSTENUTO", "SOFT PEDAL",
/*68*/	"LEGATO", "HOLD 2", "SOUND CONTROL 1", "SOUND CONTROL 2",
/*72*/	"SOUND CONTROL 3", "SOUND CONTROL 4", "SOUND CONTROL 5", "SOUND CONTROL 6",
/*76*/	"SOUND CONTROL 7", "SOUND CONTROL 8", "SOUND CONTROL 9", "SOUND CONTROL 10",
/*80*/	"GENERAL PURPOSE 5", "GENERAL PURPOSE 6", "GENERAL PURPOSE 7", "GENERAL PURPOSE 8",
/*84*/	"PROTAMENTO CONTROL", "", "", "",
/*88*/  "", "", "", "EFFECTS 1 DEPTH",
/*92*/  "EFFECTS 2 DEPTH", "EFFECTS 3 DEPTH", "EFFECTS 4 DEPTH", "EFFECTS 5 DEPTH",
/*96*/	"DATA INCREMENT", "DATA DECREMENT", "NRPN LSB", "NRPN MSB",
/*100*/ "RPN LSB", "RPN MSB", "", "",
/*104*/  "", "", "", "",
/*108*/  "", "", "", "",
/*112*/  "", "", "", "",
/*116*/  "", "", "", "",
/*120*/  "", "", "", "",
/*124*/  "", "", "", ""
};
