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

#include "glk/alan3/types.h"
#include "glk/alan3/lists.h"
#include "glk/alan3/checkentry.h"
#include "glk/alan3/rules.h"
#include "glk/alan3/msg.h"
#include "glk/alan3/utils.h"
#include "glk/alan3/compatibility.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/memory.h"

namespace Glk {
namespace Alan3 {

extern Aword *memory;

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
static Aaddr memorySize = 0;
static Aword *addressesDone = NULL;
static int numberDone = 0;
static int doneSize = 0;

static bool alreadyDone(Aaddr address) {
	int i;

	if (address == 0) return TRUE;

	/* Have we already done it? */
	for (i = 0; i < numberDone; i++)
		if (addressesDone[i] == address)
			return TRUE;

	if (doneSize == numberDone) {
		doneSize += 100;
		addressesDone = (Aword *)realloc(addressesDone, doneSize * sizeof(Aword));
	}
	addressesDone[numberDone] = address;
	numberDone++;

	return FALSE;
}



#define NATIVE(w)   \
	( (((Aword)((w)[3])      ) & 0x000000ff)    \
	  | (((Aword)((w)[2]) <<  8) & 0x0000ff00)    \
	  | (((Aword)((w)[1]) << 16) & 0x00ff0000)    \
	  | (((Aword)((w)[0]) << 24) & 0xff000000))

/*----------------------------------------------------------------------*/
Aword reversed(Aword w) { /* IN - The ACODE word to swap bytes of */
#ifdef TRYNATIVE
	return NATIVE(&w);
#else
	Aword s;                      /* The swapped ACODE word */
	char *wp, *sp;
	uint i;

	wp = (char *) &w;
	sp = (char *) &s;

	for (i = 0; i < sizeof(Aword); i++)
		sp[sizeof(Aword) - 1 - i] = wp[i];

	return s;
#endif
}


void reverseWord(Aword *w) {        /* IN - The ACODE word to reverse bytes in */
	*w = reversed(*w);
}

void reverse(Aword *w) {        /* IN - The ACODE word to reverse bytes in */
	if (w < &memory[0] || w > &memory[memorySize])
		syserr("Reversing address outside of memory");
	reverseWord(w);
}


static void reverseTable(Aword adr, int elementSize) {
	Aword *e = &memory[adr];
	uint i;

	if (elementSize < (int)sizeof(Aword) || elementSize % (int)sizeof(Aword) != 0)
		syserr("***Wrong size in 'reverseTable()' ***");

	if (adr == 0) return;

	while (!isEndOfArray(e)) {
		for (i = 0; i < elementSize / sizeof(Aword); i++) {
			reverse(e);
			e++;
		}
	}
}


static void reverseStms(Aword adr) {
	Aword *e = &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	while (TRUE) {
		reverse(e);
		if (*e == ((Aword)C_STMOP << 28 | (Aword)I_RETURN)) break;
		e++;
	}
}


static void reverseMsgs(Aword adr) {
	MessageEntry *e = (MessageEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(MessageEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseDictionary(Aword adr) {
	DictionaryEntry *e = (DictionaryEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(DictionaryEntry));
		while (!isEndOfArray(e)) {
			if ((e->classBits & SYNONYM_BIT) == 0) { /* Do not do this for synonyms */
				reverseTable(e->adjectiveRefs, sizeof(Aword));
				reverseTable(e->nounRefs, sizeof(Aword));
				reverseTable(e->pronounRefs, sizeof(Aword));
			}
			e++;
		}
	}
}


static void reverseChks(Aword adr) {
	CheckEntry *e = (CheckEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(CheckEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->exp);
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseAlts(Aword adr) {
	AltEntry *e = (AltEntry *)&memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(AltEntry));
		while (!isEndOfArray(e)) {
			reverseChks(e->checks);
			reverseStms(e->action);
			e++;
		}
	}
}


static void reverseVerbs(Aword adr) {
	VerbEntry *e = (VerbEntry *)&memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(VerbEntry));
		while (!isEndOfArray(e)) {
			reverseAlts(e->alts);
			e++;
		}
	}
}


static void reverseSteps(Aword adr) {
	StepEntry *e = (StepEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(StepEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->after);
			reverseStms(e->exp);
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseScrs(Aword adr) {
	ScriptEntry *e = (ScriptEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ScriptEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->description);
			reverseSteps(e->steps);
			e++;
		}
	}
}


static void reverseExits(Aword adr) {
	ExitEntry *e = (ExitEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ExitEntry));
		while (!isEndOfArray(e)) {
			reverseChks(e->checks);
			reverseStms(e->action);
			e++;
		}
	}
}


static void reverseClasses(Aword adr) {
	ClassEntry *e = (ClassEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ClassEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->name);
			reverseStms(e->initialize);
			reverseChks(e->descriptionChecks);
			reverseStms(e->description);
			reverseStms(e->entered);
			reverseStms(e->definite.address);
			reverseStms(e->indefinite.address);
			reverseStms(e->negative.address);
			reverseStms(e->mentioned);
			reverseVerbs(e->verbs);
			e++;
		}
	}
}


static void reverseInstances(Aword adr) {
	InstanceEntry *e = (InstanceEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(InstanceEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->name);
			reverseTable(e->initialAttributes, sizeof(AttributeHeaderEntry));
			reverseStms(e->initialize);
			reverseStms(e->definite.address);
			reverseStms(e->indefinite.address);
			reverseStms(e->negative.address);
			reverseStms(e->mentioned);
			reverseChks(e->checks);
			reverseStms(e->description);
			reverseVerbs(e->verbs);
			reverseStms(e->entered);
			reverseExits(e->exits);
			e++;
		}
	}
}


static void reverseRestrictions(Aword adr) {
	RestrictionEntry *e = (RestrictionEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;
	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(RestrictionEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseElms(Aword adr) {
	ElementEntry *e = (ElementEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ElementEntry));
		while (!isEndOfArray(e)) {
			if ((uint)e->code == EOS) reverseRestrictions(e->next);
			else reverseElms(e->next);
			e++;
		}
	}
}


static void reverseSyntaxTableCurrent(Aword adr) {
	SyntaxEntry *e = (SyntaxEntry *) &memory[adr];

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(SyntaxEntry));
		while (!isEndOfArray(e)) {
			reverseElms(e->elms);
			reverseTable(e->parameterNameTable, sizeof(Aaddr));
			e++;
		}
	}
}


static void reverseSyntaxTablePreBeta2(Aword adr) {
	SyntaxEntryPreBeta2 *e = (SyntaxEntryPreBeta2 *) &memory[adr];

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(SyntaxEntryPreBeta2));
		while (!isEndOfArray(e)) {
			reverseElms(e->elms);
			e++;
		}
	}
}


static void reverseSyntaxTable(Aword adr, byte version[]) {
	if (!adr || alreadyDone(adr)) return;

	if (isPreBeta2(version))
		reverseSyntaxTablePreBeta2(adr);
	else
		reverseSyntaxTableCurrent(adr);
}


static void reverseParameterNames(Aaddr parameterMapAddress) {
	Aaddr *e;
	Aaddr adr;

	adr = addressAfterTable(parameterMapAddress, sizeof(ParameterMapEntry));
	reverse(&memory[adr]);
	adr = memory[adr];

	reverseTable(adr, sizeof(Aaddr));

	e = (Aaddr *) &memory[adr];
	while (!isEndOfArray(e)) {
		reverseTable(*e, sizeof(Aaddr));
		e++;
	}
}


static void reverseParameterTable(Aword adr) {
	ParameterMapEntry *e = (ParameterMapEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ParameterMapEntry));
		while (!isEndOfArray(e)) {
			reverseTable(e->parameterMapping, sizeof(Aword));
			e++;
		}
	}
}


static void reverseEvts(Aword adr) {
	EventEntry *e = (EventEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(EventEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->code);
			e++;
		}
	}
}


static void reverseLims(Aword adr) {
	LimitEntry *e = (LimitEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(LimitEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseContainers(Aword adr) {
	ContainerEntry *e = (ContainerEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(ContainerEntry));
		while (!isEndOfArray(e)) {
			reverseLims(e->limits);
			reverseStms(e->header);
			reverseStms(e->empty);
			reverseChks(e->extractChecks);
			reverseStms(e->extractStatements);
			e++;
		}
	}
}


static void reverseRuls(Aword adr) {
	RuleEntry *e = (RuleEntry *) &memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(RuleEntry));
		while (!isEndOfArray(e)) {
			reverseStms(e->exp);
			reverseStms(e->stms);
			e++;
		}
	}
}


static void reverseSetInitTable(Aaddr adr) {
	SetInitEntry *e = (SetInitEntry *)&memory[adr];

	if (!adr || alreadyDone(adr)) return;

	if (!isEndOfArray(e)) {
		reverseTable(adr, sizeof(SetInitEntry));
		while (!isEndOfArray(e)) {
			reverseTable(e->setAddress, sizeof(Aword));
			e++;
		}
	}
}



/*----------------------------------------------------------------------*/
static void reversePreAlpha5Header(Pre3_0alpha5Header *hdr) {
	uint i;

	/* Reverse all words in the header except the tag */
	for (i = 1; i < sizeof(*hdr) / sizeof(Aword); i++)
		reverseWord(&((Aword *)hdr)[i]);
}


/*----------------------------------------------------------------------*/
static void reversePreAlpha5() {
	/* NOTE that the reversePreXXX() have different header definitions */
	Pre3_0alpha5Header *hdr = (Pre3_0alpha5Header *)memory;

	reversePreAlpha5Header(hdr);
	memorySize = hdr->size;

	reverseDictionary(hdr->dictionary);
	reverseSyntaxTable(hdr->syntaxTableAddress, hdr->version);
	reverseParameterTable(hdr->parameterMapAddress);
	reverseVerbs(hdr->verbTableAddress);
	reverseClasses(hdr->classTableAddress);
	reverseInstances(hdr->instanceTableAddress);
	reverseScrs(hdr->scriptTableAddress);
	reverseContainers(hdr->containerTableAddress);
	reverseEvts(hdr->eventTableAddress);
	reverseRuls(hdr->ruleTableAddress);
	reverseTable(hdr->stringInitTable, sizeof(StringInitEntry));
	reverseSetInitTable(hdr->setInitTable);
	reverseTable(hdr->sourceFileTable, sizeof(SourceFileEntry));
	reverseTable(hdr->sourceLineTable, sizeof(SourceLineEntry));
	reverseStms(hdr->start);
	reverseMsgs(hdr->messageTableAddress);

	reverseTable(hdr->scores, sizeof(Aword));
	reverseTable(hdr->freq, sizeof(Aword));
}


/*----------------------------------------------------------------------*/
static void reversePreBeta2Header(Pre3_0beta2Header *hdr) {
	uint i;

	/* Reverse all words in the header except the tag */
	for (i = 1; i < sizeof(*hdr) / sizeof(Aword); i++)
		reverseWord(&((Aword *)hdr)[i]);
}


/*----------------------------------------------------------------------*/
static void reversePreBeta2() {
	/* NOTE that the reversePreXXX() have different header definitions */
	Pre3_0beta2Header *hdr = (Pre3_0beta2Header *)memory;

	reversePreBeta2Header(hdr);
	memorySize = hdr->size;

	reverseDictionary(hdr->dictionary);
	reverseSyntaxTable(hdr->syntaxTableAddress, hdr->version);
	reverseParameterTable(hdr->parameterMapAddress);
	reverseVerbs(hdr->verbTableAddress);
	reverseClasses(hdr->classTableAddress);
	reverseInstances(hdr->instanceTableAddress);
	reverseScrs(hdr->scriptTableAddress);
	reverseContainers(hdr->containerTableAddress);
	reverseEvts(hdr->eventTableAddress);
	reverseRuls(hdr->ruleTableAddress);
	reverseTable(hdr->stringInitTable, sizeof(StringInitEntry));
	reverseSetInitTable(hdr->setInitTable);
	reverseTable(hdr->sourceFileTable, sizeof(SourceFileEntry));
	reverseTable(hdr->sourceLineTable, sizeof(SourceLineEntry));
	reverseStms(hdr->start);
	reverseMsgs(hdr->messageTableAddress);

	reverseTable(hdr->scores, sizeof(Aword));
	reverseTable(hdr->freq, sizeof(Aword));
}


/*======================================================================*/
void reverseHdr(ACodeHeader *hdr) {
	uint i;

	/* Reverse all words in the header except the tag and the version marking */
	for (i = 1; i < sizeof(*hdr) / sizeof(Aword); i++)
		reverseWord(&((Aword *)hdr)[i]);
}


/*----------------------------------------------------------------------*/
static void reverseInstanceIdTable(ACodeHeader *hdr) {
	reverseTable(hdr->instanceTableAddress + hdr->instanceMax * sizeof(InstanceEntry) / sizeof(Aword) + 1, sizeof(Aword));
}


/*----------------------------------------------------------------------*/
static void reverseNative() {
	/* NOTE that the reversePreXXX() have different hdr definitions */
	ACodeHeader *hdr = (ACodeHeader *)memory;

	reverseHdr(hdr);
	memorySize = hdr->size;

	reverseDictionary(hdr->dictionary);
	reverseSyntaxTable(hdr->syntaxTableAddress, hdr->version);
	if (hdr->debug && !isPreBeta3(hdr->version))
		reverseParameterNames(hdr->parameterMapAddress);
	reverseParameterTable(hdr->parameterMapAddress);
	reverseVerbs(hdr->verbTableAddress);
	reverseClasses(hdr->classTableAddress);
	reverseInstances(hdr->instanceTableAddress);
	if (hdr->debug && !isPreBeta3(hdr->version))
		reverseInstanceIdTable(hdr);
	reverseScrs(hdr->scriptTableAddress);
	reverseContainers(hdr->containerTableAddress);
	reverseEvts(hdr->eventTableAddress);
	reverseRuls(hdr->ruleTableAddress);
	reverseTable(hdr->stringInitTable, sizeof(StringInitEntry));
	reverseSetInitTable(hdr->setInitTable);
	reverseTable(hdr->sourceFileTable, sizeof(SourceFileEntry));
	reverseTable(hdr->sourceLineTable, sizeof(SourceLineEntry));
	reverseStms(hdr->prompt);
	reverseStms(hdr->start);
	reverseMsgs(hdr->messageTableAddress);

	reverseTable(hdr->scores, sizeof(Aword));
	reverseTable(hdr->freq, sizeof(Aword));
}


/*======================================================================

  reverseACD()

  Traverse all the data structures and reverse all integers.
  Only performed in architectures with reversed byte ordering, which
  makes the .ACD files fully compatible across architectures

  */
void reverseACD(void) {
	ACodeHeader *hdr = (ACodeHeader *)memory;
	byte version[4];
	int i;

	/* Make a copy of the version marking to reverse */
	for (i = 0; i <= 3; i++)
		version[i] = hdr->version[i];
	reverseWord((Aword *)&version);

	if (isPreAlpha5(version))
		reversePreAlpha5();
	else if (isPreBeta2(version))
		reversePreBeta2();
	else
		reverseNative();

	free(addressesDone);
}

} // End of namespace Alan3
} // End of namespace Glk
