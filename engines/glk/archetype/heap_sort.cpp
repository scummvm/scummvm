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

#include "glk/archetype/heap_sort.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

const char *const CANT_PEEK = "Internal error:  cannot peek into heap";
const char *const CANT_POKE = "Internal error:  cannot poke into heap";

// FIXME: This requires global constructor
HeapType H;

void heap_sort_init() {
	new_xarray(H);
}

static bool lighter(const Element one, const Element two) {
	return *static_cast<StringPtr>(one) < *static_cast<StringPtr>(two);
}

static void heapup() {
	int L, parent;
	Element Lp, parentp = nullptr;
	Element temp;

	L = H.size();
	while (L > 1) {
		if ((L % 2) == 0)
			parent = L / 2;
		else
			parent = (L - 1) / 2;

		if (!(access_xarray(H, L, Lp, PEEK_ACCESS) && access_xarray(H, parent, parentp, PEEK_ACCESS)))
			g_vm->writeln(CANT_PEEK);

		if (lighter(Lp, parentp)) {
			temp = parentp;
			if (!(access_xarray(H, parent, Lp, POKE_ACCESS) && access_xarray(H, L, temp, POKE_ACCESS)))
				g_vm->writeln(CANT_POKE);
			L = parent;
		} else {
			L = 0;
		}
	}
}

static void heapdown() {
	uint L, compare, lc, rc;
	Element lp;
	Element lcp, rcp;
	Element comparep;
	Element temp;

	L = 1;
	while (L < H.size()) {
		lc = L * 2;
		if (lc > H.size()) {
			L = lc;
		} else {
			rc = lc + 1;
			if (!access_xarray(H, lc, lcp, PEEK_ACCESS))
				g_vm->writeln(CANT_PEEK);

			if (rc > H.size()) {
				compare = lc;
				comparep = lcp;
			} else {
				if (!access_xarray(H, rc, rcp, PEEK_ACCESS))
					g_vm->writeln(CANT_PEEK);
				if (lighter(lcp, rcp)) {
					compare = lc;
					comparep = lcp;
				} else {
					compare = rc;
					comparep = rcp;
				}
			}

			if (!access_xarray(H, L, lp, PEEK_ACCESS))
				g_vm->writeln(CANT_PEEK);
			if (!lighter(comparep, lp)) {
				temp = comparep;
				if (!(access_xarray(H, compare, lp, POKE_ACCESS) && access_xarray(H, L, temp, POKE_ACCESS)))
					g_vm->writeln(CANT_POKE);
				L = compare;
			} else {
				L = H.size() + 1;
			}
		}
	}
}

bool pop_heap(Element &e) {
	Element temp;

	if (H.empty()) {
		return false;
	} else {
		if (!(access_xarray(H, 0, e, PEEK_ACCESS) && access_xarray(H, H.size() - 1, temp, PEEK_ACCESS)
				&&  access_xarray(H, 0, temp, POKE_ACCESS)))
			g_vm->writeln(CANT_PEEK);

		shrink_xarray(H);
		heapdown();
		return true;
	}
}

void drop_on_heap(Element e) {
	append_to_xarray(H, e);
	heapup();
}

void drop_str_on_heap(const String &s) {
	StringPtr sp = NewDynStr(s);
	void *p = (void *)sp;
	drop_on_heap(p);
}

void reinit_heap() {
	dispose_xarray(H);
	new_xarray(H);
}

} // End of namespace Archetype
} // End of namespace Glk
