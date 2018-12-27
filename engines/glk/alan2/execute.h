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

#ifndef GLK_ALAN2_EXECUTE
#define GLK_ALAN2_EXECUTE

#include "glk/alan2/acode.h"
#include "glk/alan2/rules.h"
#include "common/list.h"

namespace Glk {
namespace Alan2 {

class Execute {
public:
	Execute() {}

	void sys(Aword fpos, Aword len);
	bool confirm(MsgKind msgno);
	Aptr attribute(Aword item, Aword atr);
	void say(Aword item);
	void saynum(Aword num);
	void saystr(char *str);
	Aptr strattr(Aword id, Aword atr);
	void setstr(Aword id, Aword atr, Aword str);
	void getstr(Aword fpos, Aword len);
	void print(Aword fpos, Aword len);
	void look();
	void make(Aword id, Aword atr, Aword val);
	void set(Aword id, Aword atr, Aword val);
	void incr(Aword id, Aword atr, Aword step);
	void decr(Aword id, Aword atr, Aword step);
	void use(Aword act, Aword scr);
	void describe(Aword id);
	void list(Aword cnt);
	void locate(Aword id, Aword whr);
	void empty(Aword cnt, Aword whr);
	void score(Aword sc);
	void visits(Aword v);

	void eventchk();
	void schedule(Aword evt, Aword whr, Aword aft);
	void cancl(Aword evt);

	void quit();
	void restart();
	void sayint(Aword val);
	Aword rnd(Aword from, Aword to);
	Abool btw(Aint val, Aint from, Aint to);
	Aword contains(Aptr string, Aptr substring);
	Abool streq(char a[], char b[]);
	Abool in(Aword obj, Aword cnt);
	Aword where(Aword item);
	Aint agrmax(Aword atr, Aword whr);
	Aint agrsum(Aword atr, Aword whr);
	Aint agrcount(Aword whr);
	Abool isHere(Aword item);
	Abool isNear(Aword item);

private:
	bool exitto(int to, int from);
	int count(int cnt);
	int sumatr(Aword atr, Aword cnt);
	bool checklim(Aword cnt, Aword obj);
	Aptr getatr(Aaddr atradr, Aaddr atr);
	void setatr(Aaddr atradr, Aword atr, Aword val);
	void makloc(Aword loc, Aword atr, Aword val);
	void makobj(Aword obj, Aword atr, Aword val);
	void makact(Aword act, Aword atr, Aword val);
	void setloc(Aword loc, Aword atr, Aword val);
	void setobj(Aword obj, Aword atr, Aword val);
	void setact(Aword act, Aword atr, Aword val);
	void incratr(Aaddr atradr, Aword atr, Aword step);
	void incrloc(Aword loc, Aword atr, Aword step);
	void incrobj(Aword obj, Aword atr, Aword step);
	void incract(Aword act, Aword atr, Aword step);
	Aword objloc(Aword obj);
	Aword actloc(Aword act);
	void locobj(Aword obj, Aword whr);
	void locact(Aword act, Aword whr);
	Abool objhere(Aword obj);
	Aword acthere(Aword act);
	Aword objnear(Aword obj);
	Aword actnear(Aword act);
	void sayloc(Aword loc);
	void sayobj(Aword obj);
	void sayact(Aword act);
	void saylit(Aword lit);
	void sayarticle(Aword id);
	void dscrloc(Aword loc);
	void dscrobj(Aword obj);
	void dscract(Aword act);
	void dscrobjs();
	void dscracts();
	Aptr locatr(Aword loc, Aword atr);
	Aptr objatr(Aword obj, Aword atr);
	Aptr actatr(Aword act, Aword atr);
	Aptr litatr(Aword lit, Aword atr);

	// The event queue
	EvtqElem *eventq;	// Event queue
	int etop;			// Event queue top pointer

	// Amachine data structures
	ActElem *_acts;		// Actor table pointer
	LocElem *_locs;		// Location table pointer
	ObjElem *_objs;		// Object table pointer
	CntElem *_cnts;		// Container table pointer
	Common::Stack<Aword> _describeStack;
};

} // End of namespace Alan2
} // Engine of namespace GLK

#endif
