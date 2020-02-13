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

#ifndef GLK_ALAN2_EXE
#define GLK_ALAN2_EXE

/* Header file for instruction execution unit in Alan interpreter
 */

#include "glk/alan2/types.h"
#include "glk/jumps.h"

namespace Glk {
namespace Alan2 {

/* The event queue */
extern EvtqElem eventq[];   /* Event queue */
extern int etop;        /* Event queue top pointer */
extern Boolean looking;     /* LOOKING? flag */
extern int dscrstkp;        /* Point into describe stack */

extern void sys(Aword fpos, Aword len);
extern Boolean confirm(MsgKind msgno);
extern Aptr attribute(Aword item, Aword atr);
extern void say(Aword item);
extern void saynum(Aword num);
extern void saystr(char *str);
extern Aptr strattr(Aword id, Aword atr);
extern void setstr(Aword id, Aword atr, Aword str);
extern void getstr(Aword fpos, Aword len);
extern void print(Aword fpos, Aword len);
extern void look(void);
extern void make(Aword id, Aword atr, Aword val);
extern void set(Aword id, Aword atr, Aword val);
extern void incr(Aword id, Aword atr, Aword step);
extern void decr(Aword id, Aword atr, Aword step);
extern void use(Aword act, Aword scr);
extern void describe(Aword id);
extern void list(Aword cnt);
extern void locate(Aword id, Aword whr);
extern void empty(Aword cnt, Aword whr);
extern void score(Aword sc);
extern void visits(Aword v);
extern void schedule(Aword evt, Aword whr, Aword aft);
extern void cancl(Aword evt);
extern void quit(CONTEXT);
extern void restart(void);
extern void save(void);
extern void restore(void);
extern void say(Aword id);
extern void sayint(Aword val);
extern Aword rnd(Aword from, Aword to);
extern Abool btw(Aint val, Aint from, Aint to);
extern Aword contains(Aptr string, Aptr substring);
extern Abool streq(char a[], char b[]);
extern Abool in(Aword obj, Aword cnt);
extern Aword where(Aword item);
extern Aint agrmax(Aword atr, Aword whr);
extern Aint agrsum(Aword atr, Aword whr);
extern Aint agrcount(Aword whr);
extern Abool isHere(Aword item);
extern Abool isNear(Aword item);

} // End of namespace Alan2
} // End of namespace Glk

#endif
