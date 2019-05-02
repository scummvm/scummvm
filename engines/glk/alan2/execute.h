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

	/**
	 * Check if any events are pending. If so execute them.
	 */
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
	
	/**
	 * Count the number of items in a container.
	 * 
	 * @param cnt	The container to count
	 */
	int count(int cnt);
	
	/**
	 * Sum the values of one attribute in a container. Recursively.
	 * 
	 * @param atr	The attribute to sum over
	 * @param cnt	the container to sum
	 */
	int sumAttributes(Aword atr, Aword cnt);
	
	/**
	 * Checks if a limit for a container is exceeded.
	 * 
	 * @param cnt	Container code
	 * @param obj	The object to add
	 */
	bool checkContainerLimit(Aword cnt, Aword obj);

	/**
	 * Get an attribute value from an attribute list
	 * 
	 * @param atradr	ACODE address to attribute table
	 * @param atr		The attribute to read
	 */	
	Aptr getAttribute(Aaddr atradr, Aaddr atr);
	
	/**
	 * Set a particular attribute to a value.
	 *
	 * @param atradr	ACODE address to attribute table
	 * @param atr		Attribute code
	 * @param val		New value
	 */
	void setAttribute(Aaddr atradr, Aword atr, Aword val);

	/**
	 * Increment a particular attribute by a value.
	 * 
	 * @param atradr	ACODE address to attribute table
	 * @param atr		Attribute code
	 * @param step		Step to increment by
	 */
	void incAttribute(Aaddr atradr, Aword atr, Aword step);
	void incLocation(Aword loc, Aword atr, Aword step);
	void incObject(Aword obj, Aword atr, Aword step);
	void incract(Aword act, Aword atr, Aword step);
	Aword objloc(Aword obj);
	Aword actloc(Aword act);
	void locobj(Aword obj, Aword whr);
	void locact(Aword act, Aword whr);
	Abool objhere(Aword obj);
	Aword acthere(Aword act);
	Aword objnear(Aword obj);
	Aword actnear(Aword act);
	void sayarticle(Aword id);
	void dscrloc(Aword loc);
	void dscrobj(Aword obj);
	void dscract(Aword act);
	
	/**
	 * Description of current location
	 */
	void dscrobjs();
	
	void dscracts();

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
} // End of namespace Glk

#endif
