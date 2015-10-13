//////////////////////////////////////////////////////////////////////
/// @file NlConvert.h  Notelist Converter definitions
//
// This file contains the definitions for tokens and global functions
// used by the notelist parser and sequence generators.
// Since it includes the other header files, only this file needs to be 
// included by programs that use Notelist
//
// Copyright 2008, Daniel R. Mitchell
// License: Creative Commons/GNU-GPL 
// (http://creativecommons.org/licenses/GPL/2.0/)
// (http://www.gnu.org/licenses/gpl.html)
//////////////////////////////////////////////////////////////////////

#define NOTELIST_VERSION 1.1

// generator tokens

#define T_ENDOF 0
#define	T_START	1		// first instruction

#define T_ENDSTMT ';'
#define T_OPAREN  '('
#define T_CPAREN  ')'
#define T_OBRACE  '{'
#define T_CBRACE  '}'
#define T_OBRACK  '['
#define T_CBRACK  ']'
#define T_EQ      '='
#define T_MULOP   '*'
#define T_DIVOP   '/'
#define T_ADDOP   '+'
#define T_SUBOP   '-'
#define T_COMMA   ','
#define T_STRLIT  '"'
#define T_QUERY   '?'
#define T_EXPOP   '^'
#define T_AND     '&'
#define T_OR      '|'
#define T_NOT     '~'
#define T_COL     ':'

#define T_LTOP    367
#define T_LEOP    368
#define T_GTOP    369
#define T_GEOP    370
#define T_NEOP    371
#define T_EQOP    372

#define T_PIT   301
#define T_DUR   302
#define T_NUM   303
#define T_VOICE 304
#define T_TIME  305
#define T_TEMPO 306
#define T_BEGIN 307
#define T_END   308
#define T_LOOP  309
#define T_SYSTEM 310
#define T_SCRIPT 311
#define T_CALL  312
#define T_EVAL  313
#define T_WRITE 315
#define T_INSTNUM 316
#define T_NOTE  317
#define T_EXPR  318
#define T_SUS   319
#define T_TIE   320
#define T_VOL   324
#define T_XPOSE 325
#define T_NEG   326
#define T_SEQ   327
#define T_PLAY  328
#define T_VALS  329
#define T_INC   330
#define T_INIT  331
#define T_FGEN  332
#define T_LINE  333
#define T_EXP   334
#define T_RAND  335
#define T_LOG   336
#define T_ART   337
#define T_PCNT  338
#define T_FIXED 339
#define T_ADD   340
#define T_MIX   341
#define T_MIDC  354
#define T_CHNL  355
#define T_VER   356
#define T_ON    357
#define T_OFF   358
#define T_PARAM 359
#define T_MARK  360
#define T_SYNC  361
#define T_COUNT 364
#define T_MAP 365
#define T_MAXPARAM 366
#define T_DOUBLE  373
#define T_CURPIT 377
#define T_CURDUR 378
#define T_CURVOL 379
#define T_CURTIME 380
#define T_IF 374
#define T_ELSE 375
#define T_THEN 381
#define T_DO 382
#define T_WHILE 376
#define T_DECLARE 383
#define T_VAR 384
#define T_SET 385
#define T_OPTION 386
#define T_FREQ 387
#define T_VOLDB 388
#define T_CATOP 389
#define T_MIXIN 390
#define T_MIXPAN 391
#define T_MIXSEND 392
#define T_FXRCV 393
#define T_FXPAN 394
#define T_RAMP 395
#define T_OSC  396
#define T_MIDICC 397
#define T_MIDIPRG 398
#define T_MIDIPW 399
#define T_MIDIAT 400
#define T_TRACK 401
#define T_STARTTRK 402
#define T_STOPTRK 403
#define T_VELOCITY 404
#define T_NLVER 405

#define MAXPARAM 10
#define MAXFGEN  10

class nlLex;
class nlGenerate;
class nlConverter;
class nlVoice;
class nlInstr;
class nlScriptEngine;
class nlSequence;
class nlScriptNode;
class nlVariable;

extern int CompareToken(const char *s1, const char *s2);
extern char *StrMakeCopy(const char *);
extern char *StrPaste(const char *s1, const char *s2);

#include "Lex.h"
#include "Generate.h"
#include "Parser.h"
#include "ScriptEngine.h"
#include "Converter.h"
