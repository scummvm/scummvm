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

#ifndef GLK_ALAN2_MAIN
#define GLK_ALAN2_MAIN

/* Header file for main unit of ARUN Alan System interpreter */

#include "common/file.h"
#include "glk/alan2/types.h"
#include "glk/jumps.h"

namespace Glk {
namespace Alan2 {

/* DATA */

#define MEMORYSIZE 1000L

extern Aaddr memTop;        // Top of memory

extern int conjWord;        // First conjunction in dictionary

/* The Amachine memory */
extern Aword *memory;
extern AcdHdr *header;

/* Amachine variables */
extern CurVars cur;

/* Amachine data structures */
extern WrdElem *dict;       /* Dictionary pointer */
extern ActElem *acts;       /* Actor table pointer */
extern LocElem *locs;       /* Location table pointer */
extern VrbElem *vrbs;       /* Verb table pointer */
extern StxElem *stxs;       /* Syntax table pointer */
extern ObjElem *objs;       /* Object table pointer */
extern CntElem *cnts;       /* Container table pointer */
extern RulElem *ruls;       /* Rule table pointer */
extern EvtElem *evts;       /* Event table pointer */
extern MsgElem *msgs;       /* Message table pointer */
extern Aword *scores;       /* Score table pointer */
extern Aword *freq;     /* Cumulated frequencies */

extern int dictsize;        /* Number of entries in dictionary */

/* The text and message file */
extern Common::File *txtfil;
extern Common::WriteStream *logfil;
extern Common::SeekableReadStream *codfil;

#undef ftell
#undef fgetc
#undef getc
#undef fseek
#undef fclose
#define ftell(FP) FP->pos()
#define fgetc(FP) (FP->pos() >= FP->size()) ? EOD : FP->readByte()
#define getc(FP) (FP->pos() >= FP->size()) ? EOD : FP->readByte()
#define fseek(FP, OFS, WHENCE) FP->seek(OFS, WHENCE)
#define fclose(FP) delete FP


/* File names */
extern const char *advnam;

/* Screen formatting info */
extern int col, lin;

extern Boolean verbose, errflg, trcflg, dbgflg, stpflg, logflg, statusflg;
extern Boolean fail;
extern Boolean anyOutput;
extern Boolean needsp;

#define endOfTable(x) eot((Aword *) x)

extern void *allocate(unsigned long len);
extern void terminate(CONTEXT, int code);
extern void usage(void);
extern void error(CONTEXT, MsgKind msg);
extern void syserr(const char *msg);
extern void statusline(void);
extern void output(const char string[]);
extern void prmsg(MsgKind msg);
extern void para(void);
extern void newline(void);

extern Boolean checklim(Aword cnt, Aword obj);
extern Boolean possible(void);
extern Boolean exitto(int to, int from);
extern void action(CONTEXT, ParamElem *plst);
extern void go(CONTEXT, int dir);

extern Boolean eot(Aword *adr);
extern Boolean isObj(Aword x);
extern Boolean isCnt(Aword x);
extern Boolean isAct(Aword x);
extern Boolean isLoc(Aword x);
extern Boolean isLit(Aword x);
extern Boolean isNum(Aword x);
extern Boolean isStr(Aword x);

/* Run the game! */
extern void run(void);

} // End of namespace Alan2
} // End of namespace Glk

#endif
