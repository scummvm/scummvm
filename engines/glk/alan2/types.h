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

#include "glk/alan2/acode.h"

namespace Glk {
namespace Alan2 {

// CONSTANTS

#define ACTMIN (_vm->header->actmin)
#define ACTMAX (_vm->header->actmax)
#define OBJMIN (_vm->header->objmin)
#define OBJMAX (_vm->header->objmax)
#define LOCMIN (_vm->header->locmin)
#define LOCMAX (_vm->header->locmax)
#define CNTMIN (_vm->header->cntmin)
#define CNTMAX (_vm->header->cntmax)
#define LITMIN (_vm->header->locmax + 1)
#define LITMAX (_vm->header->locmax + 1 + litCount)
#define EVTMIN (_vm->header->evtmin)
#define EVTMAX (_vm->header->evtmax)

#define HERO ACTMIN


#define addrTo(x) (&_vm->memory[x])

// The word classes are represented as numbers but in the dictonary they are generated as bits
#define isVerb(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_VRB))!=0)
#define isConj(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_CONJ))!=0)
#define isBut(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_BUT))!=0)
#define isThem(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_THEM))!=0)
#define isIt(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_IT))!=0)
#define isNoun(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_NOUN))!=0)
#define isAdj(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_ADJ))!=0)
#define isPrep(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_PREP))!=0)
#define isAll(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_ALL))!=0)
#define isDir(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_DIR))!=0)
#define isNoise(word) (word < dictsize && (dict[word].wordClass&((Aword)1L<<WRD_NOISE))!=0)
#define isLiteral(word) (word >= dictsize)


// TYPES

// Amachine variables 
struct CurVars {
	int vrb;
	int obj;
	int loc;
	int act;
	int tick;
	int score;
	int visits;
};

// The various tables 
struct WrdElem {			// Dictionary 
	Aaddr wrd;				// ACODE address to string 
	Aword wordClass;		// Word class 
	Aword code;
	Aaddr adjrefs;			// Address to reference list 
	Aaddr nounrefs;			// Address to reference list 
};

struct ActElem {			// ACTOR TABLE 
	Aword loc;				// Location 
	Abool describe;			// Description flag 
	Aaddr nam;				// Address to name printing code 
	Aaddr atrs;				// Address to attribute list 
	Aword cont;				// Code for the container props if any 
	Aword script;			// Which script is he using 
	Aaddr scradr;			// Address to script table 
	Aword step;
	Aword count;
	Aaddr vrbs;
	Aaddr dscr;				// Address of description code 
};

struct ScrElem {			// SCRIPT TABLE 
	Aword code;				// Script number 
	Aaddr dscr;				// Optional description statements 
	Aaddr steps;			// Address to steps 
};

struct StepElem {			// STEP TABLE 
	Aword after;			// After how many ticks? 
	Aaddr exp;				// Address to expression saying when 
	Aaddr stm;				// Address to the actual code 
};

struct LocElem {			// LOCATION TABLE 
	Aaddr nams;				// Address of name printing code 
	Aaddr dscr;				// Address of description code 
	Aaddr does;				// Address of does code 
	Aword describe;			// Description flag & counter 
	Aaddr atrs;				// Address of attribute list 
	Aaddr exts;				// Address of exit list 
	Aaddr vrbs;				// Address of local verb list 
};

struct ExtElem {			// EXIT TABLE structure 
	Abool done;				// Flag for reverse/convert process 
	Aword code;				// Direction code 
	Aaddr checks;			// Address of check table 
	Aaddr action;			// Address of action code 
	Aword next;				// Number of next location 
};

struct ChkElem {			// CHECK TABLE 
	Aaddr exp;				// ACODE address to expression code 
	Aaddr stms;				// ACODE address to statement code 
};

struct VrbElem {			// VERB TABLE 
	Aword code;				// Code for the verb 
	Aaddr alts;				// Address to alternatives 
};

struct StxElem {			// SYNTAX TABLE 
	Aword code;				// Code for verb word 
	Aaddr elms;				// Address to element tables 
};

struct ElmElem26 {			// ELEMENT TABLES 
	Aword code;				// Code for this element, 0 -> parameter 
	Abool multiple;			// May be multiple (if parameter) 
	Aaddr next;				// Address to next element table ... 
							// ... or class check if EOS 
};

struct ElmElem {			// ELEMENT TABLES 
	Aword code;				// Code for this element, 0 -> parameter 
	Aword flags;			// Flags for multiple/omni (if parameter) 
							// CHANGED: v2.7 from Abool for multiple 
	Aaddr next;				// Address to next element table ... 
							// ... or class check if EOS 
};

struct ClaElem {			// CLASS DEFINITION TABLE 
	Aword code;				// Parameter number 
	Aword classes;			// Parameter classes 
	Aaddr stms;				// Exception statements 
};

struct AltElem {			// VERB ALTERNATIVE TABLE 
	Abool done;				// Flag for patching (reverse/convert) process 
	Aword param;			// Parameter number 
	Aword qual;				// Verb execution qualifier 
	Aaddr checks;			// Address of the check table 
	Aaddr action;			// Address of the action code 
};

struct AtrElem {			// ATTRIBUTE LIST 
	Aword val;				// Its value 
	Aaddr stradr;			// Address to the name 
};

struct ObjElem25 {			// OBJECT TABLE of 2.5 format
	Aword loc;				// Current location 
	Abool describe;			// Describe flag 
	Aaddr atrs;				// Address of attribute list 
	Aword cont;				// Index to container properties if any 
	Aaddr vrbs;				// Address to local verb table 
	Aaddr dscr1;			// Address to Aword description code 
	Aaddr dscr2;			// Address to short description code 
};

struct ObjElem {			// OBJECT TABLE 
	Aword loc;				// Current location 
	Abool describe;			// Describe flag 
	Aaddr atrs;				// Address of attribute list 
	Aword cont;				// Index to container properties if any 
	Aaddr vrbs;				// Address to local verb table 
	Aaddr dscr1;			// Address to Aword description code 
	Aaddr art;				// Article printing code? Else use default 
							// INTRODUCED: v2.6 
	Aaddr dscr2;			// Address to short description code 
};

struct CntElem {			// CONTAINER TABLE 
	Aaddr lims;				// Address to limit check code 
	Aaddr header;			// Address to header code 
	Aaddr empty;			// Address to empty code 
	Aword parent;			// Object or actor index 
	Aaddr nam;				// Address to statement printing name 
};

struct LimElem {			// LIMIT Type 
	Aword atr;				// Attribute that limits 
	Aword val;				// And the limiting value 
	Aaddr stms;				// Statements if fail 
};

struct RulElem {			// RULE TABLE 
	Abool run;				// Is rule already run? 
	Aaddr exp;				// Address to expression code 
	Aaddr stms;				// Address to run 
};

struct EvtElem {			// EVENT TABLE 
	Aaddr stradr;			// Address to name string 
	Aaddr code;				// Address of code to run 
};

struct EvtqElem {			// EVENT QUEUE ELEMENT 
	int time;
	int event;
	int where;
};

struct IniElem {			// STRING INITIALISATION TABLE 
	Aword fpos;				// File position 
	Aword len;				// Length 
	Aword adr;				// Where to store the string 
};

struct MsgElem26 {			// MESSAGE TABLE 
	Aword fpos;				// File position 
	Aword len;				// Length of message 
};

struct MsgElem {			// MESSAGE TABLE 
	Aaddr stms;				// Address to statements
							// Changed v2.7 from fpos+len in .dat 
};

struct ParamElem {			// PARAMETER 
	Aword code;				// Code for this parameter (0=multiple) 
	Aword firstWord;		// Index to first word used by player 
	Aword lastWord;			// d:o to last 
};

enum Type {
	TYPNUM,
	TYPSTR
};

struct LitElem {			// LITERAL 
	Type type;
	Aptr value;
};

#define MAXPARAMS 9
#define MAXENTITY (_vm->header->actmax)

} // End of namespace Alan2
} // End of namespace Glk

#endif
