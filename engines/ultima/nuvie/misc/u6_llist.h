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

#ifndef NUVIE_MISC_U6_LIST_H
#define NUVIE_MISC_U6_LIST_H

namespace Ultima {
namespace Nuvie {

#define U6LLIST_FREE_DATA true

struct U6Link {
	U6Link *next;
	U6Link *prev;
	void *data;
	uint8 ref_count;
	U6Link() {
		next = NULL;
		prev = NULL;
		data = NULL;
		ref_count = 1;
	}
};

void retainU6Link(U6Link *link);
void releaseU6Link(U6Link *link);

class U6LList {
	U6Link *head;
	U6Link *tail;
	U6Link *cur;

public:

	U6LList();
	~U6LList();

	bool add(void *data);
	bool addAtPos(uint32 pos, void *data);

	uint32 findPos(void *data);
	bool replace(void *old_data, void *new_data);

	bool remove(void *data);
	bool removeAll();

	uint32 count();

	U6Link *start();
	U6Link *end();

	U6Link *next();
	U6Link *prev();
	U6Link *gotoPos(uint32 pos);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
