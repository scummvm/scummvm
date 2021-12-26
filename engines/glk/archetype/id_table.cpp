/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/archetype/id_table.h"
#include "glk/archetype/array.h"
#include "glk/archetype/archetype.h"

namespace Glk {
namespace Archetype {

ClassifyType DefaultClassification;

// Static variables
static IdRecPtr hash[BUCKETS];

int add_ident(const String &id_str) {
	int hasher;
	IdRecPtr p, new_rec;

	hasher = (int)(toupper(id_str[1])) - 65;	// A..Z => 65..90 => 0..25
	if (hasher < 0 || hasher > 25)
		hasher = 26;

	p = hash[hasher];
	while (p->next && *p->next->id_name < id_str)
		p = p->next;

	if (p->next == nullptr || *p->next->id_name > id_str) {
		new_rec = new IdRecType();
		append_to_xarray(g_vm->h_index, new_rec);

		new_rec->id_kind    = DefaultClassification;
		new_rec->id_index   = g_vm->h_index.size();
		new_rec->id_integer = new_rec->id_index;
		new_rec->id_name    = NewConstStr(id_str);
		new_rec->next       = p->next;

		p->next = new_rec;
		return g_vm->h_index.size();
	} else {
		// found existing identifier
		return p->next->id_index;
	}
}

bool index_ident(int index, IdRecPtr &id_ptr) {
	void *p;
	bool result = index_xarray(g_vm->h_index, index, p);
	id_ptr = (IdRecPtr)p;
	return result;
}

} // End of namespace Archetype
} // End of namespace Glk
