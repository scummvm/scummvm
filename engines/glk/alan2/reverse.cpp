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

#include "glk/alan2/types.h"
#include "glk/alan2/main.h"
#include "glk/alan2/reverse.h"

namespace Glk {
namespace Alan2 {

/*----------------------------------------------------------------------

  reversed()

  Return the reversed bytes in the Aword

*/
Aword reversed(Aword w /* IN - The ACODE word to swap bytes of */) {
	Aword s;                      /* The swapped ACODE word */
	char *wp, *sp;

	wp = (char *) &w;
	sp = (char *) &s;

	for (uint i = 0; i < sizeof(Aword); i++)
		sp[sizeof(Aword) - 1 - i] = wp[i];

	return s;
}

void reverse(Aword *w /* IN - The ACODE word to reverse bytes in */) {
	*w = reversed(*w);
}

static void reverseTable(Aword adr, int len) {
	Aword *e = &memory[adr];
	int i;

	if (adr != 0)
		while (!endOfTable(e)) {
			for (i = 0; i < len / (int)sizeof(Aword); i++) {
				reverse(e);
				e++;
			}
		}
}

static void reverseStms(Aword adr) {
	Aword *e = &memory[adr];

	if (adr != 0)
		while (TRUE) {
			reverse(e);
			if (*e == ((Aword)C_STMOP << 28 | (Aword)I_RETURN)) break;
			e++;
		}
}

static void reverseMsgs(Aword adr) {
	MsgElem *e = (MsgElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(MsgElem));
		while (!endOfTable(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
}

static void reverseWrds(Aword adr) {
	WrdElem *e = (WrdElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(WrdElem));
		while (!endOfTable(e)) {
			if ((e->_class & (1L << WRD_SYN)) == 0) { /* Do not do this for synonyms */
				reverseTable(e->adjrefs, sizeof(Aword));
				reverseTable(e->nounrefs, sizeof(Aword));
			}
			e++;
		}
	}
}

static void reverseChks(Aword adr) {
	ChkElem *e = (ChkElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ChkElem));
		while (!endOfTable(e)) {
			reverseStms(e->exp);
			reverseStms(e->stms);
			e++;
		}
	}
}

static void reverseAlts(Aword adr) {
	AltElem *e = (AltElem *)&memory[adr];

	if (adr != 0 && !endOfTable(e) && !e->done) {
		reverseTable(adr, sizeof(AltElem));
		e->done = TRUE;
		while (!endOfTable(e)) {
			reverseChks(e->checks);
			reverseStms(e->action);
			e++;
		}
	}
}

static void reverseVrbs(Aword adr) {
	VrbElem *e = (VrbElem *)&memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(VrbElem));
		while (!endOfTable(e)) {
			reverseAlts(e->alts);
			e++;
		}
	}
}

static void reverseSteps(Aword adr) {
	StepElem *e = (StepElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(StepElem));
		while (!endOfTable(e)) {
			reverseStms(e->exp);
			reverseStms(e->stm);
			e++;
		}
	}
}

static void reverseScrs(Aword adr) {
	ScrElem *e = (ScrElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ScrElem));
		while (!endOfTable(e)) {
			reverseStms(e->dscr);
			reverseSteps(e->steps);
			e++;
		}
	}
}

static void reverseActs(Aword adr) {
	ActElem *e = (ActElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ActElem));
		while (!endOfTable(e)) {
			reverseStms(e->nam);
			reverseTable(e->atrs, sizeof(AtrElem));
			reverseScrs(e->scradr);
			reverseVrbs(e->vrbs);
			reverseStms(e->dscr);
			e++;
		}
	}
}

static void reverseObjs(Aword adr, Boolean v2_5) {
	ObjElem *e = (ObjElem *) &memory[adr];
	ObjElem25 *e25 = (ObjElem25 *) &memory[adr];

	if (v2_5) {
		if (adr != 0 && !endOfTable(e25)) {
			reverseTable(adr, sizeof(ObjElem25));
			while (!endOfTable(e25)) {
				reverseTable(e25->atrs, sizeof(AtrElem));
				reverseVrbs(e25->vrbs);
				reverseStms(e25->dscr1);
				reverseStms(e25->dscr2);
				e25++;
			}
		}
	} else {
		if (adr != 0 && !endOfTable(e)) {
			reverseTable(adr, sizeof(ObjElem));
			while (!endOfTable(e)) {
				reverseTable(e->atrs, sizeof(AtrElem));
				reverseVrbs(e->vrbs);
				reverseStms(e->art);
				reverseStms(e->dscr1);
				reverseStms(e->dscr2);
				e++;
			}
		}
	}
}

static void reverseExts(Aword adr) {
	ExtElem *e = (ExtElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ExtElem));
		while (!endOfTable(e)) {
			if (!e->done) {
				reverseChks(e->checks);
				reverseStms(e->action);
			}
			e++;
		}
	}
}

static void reverseLocs(Aword adr) {
	LocElem *e = (LocElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(LocElem));
		while (!endOfTable(e)) {
			reverseStms(e->nams);
			reverseStms(e->dscr);
			reverseStms(e->does);
			reverseTable(e->atrs, sizeof(AtrElem));
			reverseExts(e->exts);
			reverseVrbs(e->vrbs);
			e++;
		}
	}
}

static void reverseClas(Aword adr) {
	ClaElem *e = (ClaElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ClaElem));
		while (!endOfTable(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
	if (adr)
		reverse(&((Aword *)e)[1]);  /* The verb code is stored after the table */
}

static void reverseElms(Aword adr) {
	ElmElem *e = (ElmElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(ElmElem));
		while (!endOfTable(e)) {
			if (e->code == EOS) reverseClas(e->next);
			else reverseElms(e->next);
			e++;
		}
	}
}

static void reverseStxs(Aword adr) {
	StxElem *e = (StxElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(StxElem));
		while (!endOfTable(e)) {
			reverseElms(e->elms);
			e++;
		}
	}
}

static void reverseEvts(Aword adr) {
	EvtElem *e = (EvtElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(EvtElem));
		while (!endOfTable(e)) {
			reverseStms(e->code);
			e++;
		}
	}
}

static void reverseLims(Aword adr) {
	LimElem *e = (LimElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(LimElem));
		while (!endOfTable(e)) {
			reverseStms(e->stms);
			e++;
		}
	}
}

static void reverseCnts(Aword adr) {
	CntElem *e = (CntElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(CntElem));
		while (!endOfTable(e)) {
			reverseLims(e->lims);
			reverseStms(e->header);
			reverseStms(e->empty);
			reverseStms(e->nam);
			e++;
		}
	}
}

static void reverseRuls(Aword adr) {
	RulElem *e = (RulElem *) &memory[adr];

	if (adr != 0 && !endOfTable(e)) {
		reverseTable(adr, sizeof(RulElem));
		while (!endOfTable(e)) {
			reverseStms(e->exp);
			reverseStms(e->stms);
			e++;
		}
	}
}


/*----------------------------------------------------------------------

  reverseHdr()

  Reverse the header structure.

*/
void reverseHdr(AcdHdr *hdr) {
	// Reverse all words in the header except the first (version marking)
	for (uint i = 1; i < sizeof(AcdHdr) / sizeof(Aword); i++)
		reverse(&((Aword *)hdr)[i]);
}

/*----------------------------------------------------------------------

  reverseACD()

  Traverse all the data structures and reverse all integers.
  Only performed in architectures with reversed byte ordering, which
  makes the .ACD files fully compatible across architectures

  */
void reverseACD(Boolean v2_5) {
	reverseHdr(header);
	reverseWrds(header->dict);
	reverseTable(header->oatrs, sizeof(AtrElem));
	reverseTable(header->latrs, sizeof(AtrElem));
	reverseTable(header->aatrs, sizeof(AtrElem));
	reverseActs(header->acts);
	reverseObjs(header->objs, v2_5);
	reverseLocs(header->locs);
	reverseStxs(header->stxs);
	reverseVrbs(header->vrbs);
	reverseEvts(header->evts);
	reverseCnts(header->cnts);
	reverseRuls(header->ruls);
	reverseTable(header->init, sizeof(IniElem));
	reverseStms(header->start);
	reverseMsgs(header->msgs);

	reverseTable(header->scores, sizeof(Aword));
	reverseTable(header->freq, sizeof(Aword));
}

} // End of namespace Alan2
} // End of namespace Glk
