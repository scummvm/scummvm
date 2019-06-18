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
#ifdef _PROTOTYPES_
Aword reversed(Aword w) /* IN - The ACODE word to swap bytes of */
#else
Aword reversed(w)
     Aword w;                   /* IN - The ACODE word to swap bytes of */
#endif
{
  Aword s;                      /* The swapped ACODE word */
  char *wp, *sp;
  int i;
  
  wp = (char *) &w;
  sp = (char *) &s;

  for (i = 0; i < sizeof(Aword); i++)
    sp[sizeof(Aword)-1 - i] = wp[i];

  return s;
}


#ifdef _PROTOTYPES_
void reverse(Aword *w)          /* IN - The ACODE word to reverse bytes in */
#else
void reverse(w)
     Aword *w;                  /* IN - The ACODE word to reverse bytes in */
#endif
{
  *w = reversed(*w);
}


#ifdef _PROTOTYPES_
static void reverseTable(Aword adr, int len)
#else
static void reverseTable(adr, len)
     Aword adr;
     int len;
#endif
{
  Aword *e = &memory[adr];
  int i;

  if (adr != 0)
    while (!endOfTable(e)) {
      for (i = 0; i < len/(int)sizeof(Aword); i++) {
	reverse(e);
	e++;
      }
    }
}


#ifdef _PROTOTYPES_
static void reverseStms(Aword adr)
#else
static void reverseStms(adr)
     Aword adr;
#endif
{
  Aword *e = &memory[adr];

  if (adr != 0)
    while (TRUE) {
      reverse(e);
      if (*e == ((Aword)C_STMOP<<28|(Aword)I_RETURN)) break;
      e++;
    }
}


#ifdef _PROTOTYPES_
static void reverseMsgs(Aword adr)
#else
static void reverseMsgs(adr)
     Aword adr;
#endif
{
  MsgElem *e = (MsgElem *) &memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(MsgElem));
    while (!endOfTable(e)) {
      reverseStms(e->stms);
      e++;
    }
  }
}    


#ifdef _PROTOTYPES_
static void reverseWrds(Aword adr)
#else
static void reverseWrds(adr)
     Aword adr;
#endif
{
  WrdElem *e = (WrdElem *) &memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(WrdElem));
    while (!endOfTable(e)) {
      if ((e->_class & (1L<<WRD_SYN)) == 0) { /* Do not do this for synonyms */
	reverseTable(e->adjrefs, sizeof(Aword));
	reverseTable(e->nounrefs, sizeof(Aword));
      }
      e++;
    }
  }
}    

#ifdef _PROTOTYPES_
static void reverseChks(Aword adr)
#else
static void reverseChks(adr)
     Aword adr;
#endif
{
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

#ifdef _PROTOTYPES_
static void reverseAlts(Aword adr)
#else
static void reverseAlts(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseVrbs(Aword adr)
#else
static void reverseVrbs(adr)
     Aword adr;
#endif
{
  VrbElem *e = (VrbElem *)&memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(VrbElem));
    while (!endOfTable(e)) {
      reverseAlts(e->alts);
      e++;
    }
  }
}    


#ifdef _PROTOTYPES_
static void reverseSteps(Aword adr)
#else
static void reverseSteps(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseScrs(Aword adr)
#else
static void reverseScrs(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseActs(Aword adr)
#else
static void reverseActs(adr)
     Aword adr;
#endif
{
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

#ifdef _PROTOTYPES_
static void reverseObjs(Aword adr, Boolean v2_5)
#else
static void reverseObjs(adr, v2_5)
     Aword adr;
     Boolean v2_5;              /* TRUE if it's a v2.5 format game */
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseExts(Aword adr)
#else
static void reverseExts(adr)
     Aword adr;
#endif
{
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

#ifdef _PROTOTYPES_
static void reverseLocs(Aword adr)
#else
static void reverseLocs(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseClas(Aword adr)
#else
static void reverseClas(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseElms(Aword adr)
#else
static void reverseElms(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseStxs(Aword adr)
#else
static void reverseStxs(adr)
     Aword adr;
#endif
{
  StxElem *e = (StxElem *) &memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(StxElem));
    while (!endOfTable(e)) {
      reverseElms(e->elms);
      e++;
    }
  }
}    

#ifdef _PROTOTYPES_
static void reverseEvts(Aword adr)
#else
static void reverseEvts(adr)
     Aword adr;
#endif
{
  EvtElem *e = (EvtElem *) &memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(EvtElem));
    while (!endOfTable(e)) {
      reverseStms(e->code);
      e++;
    }
  }
}    



#ifdef _PROTOTYPES_
static void reverseLims(Aword adr)
#else
static void reverseLims(adr)
     Aword adr;
#endif
{
  LimElem *e = (LimElem *) &memory[adr];

  if (adr != 0 && !endOfTable(e)) {
    reverseTable(adr, sizeof(LimElem));
    while (!endOfTable(e)) {
      reverseStms(e->stms);
      e++;
    }
  }
}    


#ifdef _PROTOTYPES_
static void reverseCnts(Aword adr)
#else
static void reverseCnts(adr)
     Aword adr;
#endif
{
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


#ifdef _PROTOTYPES_
static void reverseRuls(Aword adr)
#else
static void reverseRuls(adr)
     Aword adr;
#endif
{
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
#ifdef _PROTOTYPES_
void reverseHdr(AcdHdr *hdr)
#else
void reverseHdr(hdr)
     AcdHdr *hdr;
#endif
{
  int i;

  /* Reverse all words in the header except the first (version marking) */
  for (i = 1; i < sizeof(AcdHdr)/sizeof(Aword); i++)
    reverse(&((Aword *)hdr)[i]);
}

/*----------------------------------------------------------------------

  reverseACD()

  Traverse all the data structures and reverse all integers.
  Only performed in architectures with reversed byte ordering, which
  makes the .ACD files fully compatible across architectures

  */
#ifdef _PROTOTYPES_
void reverseACD(Boolean v2_5)
#else
void reverseACD(v2_5)
     Boolean v2_5;
#endif
{
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
