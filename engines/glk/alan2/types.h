/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_ALAN2_TYPES
#define GLK_ALAN2_TYPES

#include "glk/alan2/sysdep.h"
#include "glk/alan2/acode.h"
#include "common/serializer.h"
#include "common/stream.h"

namespace Glk {
namespace Alan2 {

/* CONSTANTS */

#ifndef TRUE
#define TRUE (0==0)
#endif
#ifndef FALSE
#define FALSE (!TRUE)
#endif

#define ACTMIN (header->actmin)
#define ACTMAX (header->actmax)
#define OBJMIN (header->objmin)
#define OBJMAX (header->objmax)
#define LOCMIN (header->locmin)
#define LOCMAX (header->locmax)
#define CNTMIN (header->cntmin)
#define CNTMAX (header->cntmax)
#define LITMIN (header->locmax+1)
#define LITMAX (header->locmax+1+litCount)
#define EVTMIN (header->evtmin)
#define EVTMAX (header->evtmax)

#define HERO ACTMIN


#define addrTo(x) (&memory[x])

/* The word classes are represented as numbers but in the dictonary they are generated as bits */
#define isVerb(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_VRB))!=0)
#define isConj(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_CONJ))!=0)
#define isBut(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_BUT))!=0)
#define isThem(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_THEM))!=0)
#define isIt(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_IT))!=0)
#define isNoun(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_NOUN))!=0)
#define isAdj(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_ADJ))!=0)
#define isPrep(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_PREP))!=0)
#define isAll(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_ALL))!=0)
#define isDir(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_DIR))!=0)
#define isNoise(word) ((int)word < dictsize && (dict[word]._class&((Aword)1L<<WRD_NOISE))!=0)
#define isLiteral(word) ((int)word >= dictsize)


/* TYPES */

typedef int Boolean;        /* Boolean values within interpreter */

/* Amachine variables */
struct CurVars {
	int
	vrb,
	obj,
	loc,
	act,
	tick,
	score,
	visits;

	/**
	 * Read or write data to/from a save file
	 */
	void synchronize(Common::Serializer &s);
};

#include "common/pack-start.h"  // START STRUCT PACKING

/* The various tables */
struct WrdElem {    /* Dictionary */
	Aaddr wrd;            /* ACODE address to string */
	Aword _class;         /* Word class */
	Aword code;
	Aaddr adjrefs;        /* Address to reference list */
	Aaddr nounrefs;       /* Address to reference list */
} PACKED_STRUCT;

struct ActElem {    /* ACTOR TABLE */
	Aword loc;            /* Location */
	Abool describe;       /* Description flag */
	Aaddr nam;            /* Address to name printing code */
	Aaddr atrs;           /* Address to attribute list */
	Aword cont;           /* Code for the container props if any */
	Aword script;         /* Which script is he using */
	Aaddr scradr;         /* Address to script table */
	Aword step;
	Aword count;
	Aaddr vrbs;
	Aaddr dscr;           /* Address of description code */
} PACKED_STRUCT;

struct ScrElem {    /* SCRIPT TABLE */
	Aword code;           /* Script number */
	Aaddr dscr;           /* Optional description statements */
	Aaddr steps;          /* Address to steps */
} PACKED_STRUCT;

struct StepElem {   /* STEP TABLE */
	Aword after;          /* After how many ticks? */
	Aaddr exp;            /* Address to expression saying when */
	Aaddr stm;            /* Address to the actual code */
} PACKED_STRUCT;

struct LocElem {    /* LOCATION TABLE */
	Aaddr nams;           /* Address of name printing code */
	Aaddr dscr;           /* Address of description code */
	Aaddr does;           /* Address of does code */
	Aword describe;       /* Description flag & counter */
	Aaddr atrs;           /* Address of attribute list */
	Aaddr exts;           /* Address of exit list */
	Aaddr vrbs;           /* Address of local verb list */
} PACKED_STRUCT;

struct ExtElem {    /* EXIT TABLE structure */
	Abool done;           /* Flag for reverse/convert process */
	Aword code;           /* Direction code */
	Aaddr checks;         /* Address of check table */
	Aaddr action;         /* Address of action code */
	Aword next;           /* Number of next location */
} PACKED_STRUCT;

struct ChkElem {    /* CHECK TABLE */
	Aaddr exp;            /* ACODE address to expression code */
	Aaddr stms;           /* ACODE address to statement code */
} PACKED_STRUCT;

struct VrbElem {    /* VERB TABLE */
	Aword code;           /* Code for the verb */
	Aaddr alts;           /* Address to alternatives */
} PACKED_STRUCT;

struct StxElem {    /* SYNTAX TABLE */
	Aword code;           /* Code for verb word */
	Aaddr elms;           /* Address to element tables */
} PACKED_STRUCT;

struct ElmElem26 {  /* ELEMENT TABLES */
	Aword code;           /* Code for this element, 0 -> parameter */
	Abool multiple;       /* May be multiple (if parameter) */
	Aaddr next;           /* Address to next element table ... */
	/* ... or class check if EOS */
} PACKED_STRUCT;

struct ElmElem {    /* ELEMENT TABLES */
	Aword code;           /* Code for this element, 0 -> parameter */
	Aword flags;          /* Flags for multiple/omni (if parameter) */
	/* CHANGED: v2.7 from Abool for multiple */
	Aaddr next;           /* Address to next element table ... */
	/* ... or class check if EOS */
} PACKED_STRUCT;

struct ClaElem {    /* CLASS DEFINITION TABLE */
	Aword code;           /* Parameter number */
	Aword classes;        /* Parameter classes */
	Aaddr stms;           /* Exception statements */
} PACKED_STRUCT;

struct AltElem {    /* VERB ALTERNATIVE TABLE */
	Abool done;           /* Flag for patching (reverse/convert) process */
	Aword param;          /* Parameter number */
	Aword qual;           /* Verb execution qualifier */
	Aaddr checks;         /* Address of the check table */
	Aaddr action;         /* Address of the action code */
} PACKED_STRUCT;

struct AtrElem {    /* ATTRIBUTE LIST */
	Aword val;            /* Its value */
	Aaddr stradr;         /* Address to the name */
} PACKED_STRUCT;

struct ObjElem25 {  /* OBJECT TABLE of 2.5 format*/
	Aword loc;            /* Current location */
	Abool describe;       /* Describe flag */
	Aaddr atrs;           /* Address of attribute list */
	Aword cont;           /* Index to container properties if any */
	Aaddr vrbs;           /* Address to local verb table */
	Aaddr dscr1;          /* Address to Aword description code */
	Aaddr dscr2;          /* Address to short description code */
} PACKED_STRUCT;

struct ObjElem {    /* OBJECT TABLE */
	Aword loc;            /* Current location */
	Abool describe;       /* Describe flag */
	Aaddr atrs;           /* Address of attribute list */
	Aword cont;           /* Index to container properties if any */
	Aaddr vrbs;           /* Address to local verb table */
	Aaddr dscr1;          /* Address to Aword description code */
	Aaddr art;            /* Article printing code? Else use default */
	/* INTRODUCED: v2.6 */
	Aaddr dscr2;          /* Address to short description code */
} PACKED_STRUCT;

struct CntElem {    /* CONTAINER TABLE */
	Aaddr lims;           /* Address to limit check code */
	Aaddr header;         /* Address to header code */
	Aaddr empty;          /* Address to empty code */
	Aword parent;         /* Object or actor index */
	Aaddr nam;            /* Address to statement printing name */
} PACKED_STRUCT;

struct LimElem {    /* LIMIT Type */
	Aword atr;            /* Attribute that limits */
	Aword val;            /* And the limiting value */
	Aaddr stms;           /* Statements if fail */
} PACKED_STRUCT;

struct RulElem {    /* RULE TABLE */
	Abool run;            /* Is rule already run? */
	Aaddr exp;            /* Address to expression code */
	Aaddr stms;           /* Address to run */
} PACKED_STRUCT;

struct EvtElem {    /* EVENT TABLE */
	Aaddr stradr;         /* Address to name string */
	Aaddr code;           /* Address of code to run */
} PACKED_STRUCT;

struct EvtqElem {   /* EVENT QUEUE ELEMENT */
	int time;
	int event;
	int where;

	/**
	 * Read or write data to/from a save file
	 */
	void synchronize(Common::Serializer &s);
} PACKED_STRUCT;

struct IniElem {    /* STRING INITIALISATION TABLE */
	Aword fpos;           /* File position */
	Aword len;            /* Length */
	Aword adr;            /* Where to store the string */
} PACKED_STRUCT;

struct MsgElem26 {  /* MESSAGE TABLE */
	Aword fpos;           /* File position */
	Aword len;            /* Length of message */
} PACKED_STRUCT;

struct MsgElem {    /* MESSAGE TABLE */
	Aaddr stms;           /* Address to statements*/
	/* Changed v2.7 from fpos+len in .dat */
} PACKED_STRUCT;


struct ParamElem {  /* PARAMETER */
	Aword code;           /* Code for this parameter (0=multiple) */
	Aword firstWord;      /* Index to first word used by player */
	Aword lastWord;           /* d:o to last */
} PACKED_STRUCT;

enum Type { TYPNUM, TYPSTR };

struct LitElem {    /* LITERAL */
	Type type;
	Aptr value;
} PACKED_STRUCT;

#include "common/pack-end.h"    // END STRUCT PACKING

#define MAXPARAMS 9
#define MAXENTITY (header->actmax)

} // End of namespace Alan2
} // End of namespace Glk

#endif
