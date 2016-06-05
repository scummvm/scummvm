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

#ifndef TITANIC_TT_QUOTES_TREE_H
#define TITANIC_TT_QUOTES_TREE_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "titanic/support/string.h"

namespace Titanic {

#define QUOTES_TREE_COUNT 1022

enum TreeEntryType { ET_END = 0, ET_TABLE = 1, ET_STRING = 2 };

class TTquotesTree {
	struct TTquotesTreeEntry {
		uint _id;
		TreeEntryType _type;
		TTquotesTreeEntry *_subTable;
		CString _string;

		TTquotesTreeEntry() : _id(0), _type(ET_END), _subTable(nullptr) {}
	};
private:
	TTquotesTreeEntry _entries[QUOTES_TREE_COUNT];
public:
	/**
	 * Load data for the quotes tree
	 */
	void load();
};

} // End of namespace Titanic

#endif /* TITANIC_TT_QUOTES_TREE_H */
