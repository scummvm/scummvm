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

class TTtreeBuffer {
public:
	int _field0;
	const char *_strP;
public:
	TTtreeBuffer() : _field0(0), _strP(nullptr) {}
};

class TTquotesTree {
	struct TTquotesTreeEntry {
		uint _id;
		TTquotesTreeEntry *_subTable;
		CString _string;

		TTquotesTreeEntry() : _id(0), _subTable(nullptr) {}
	};
private:
	TTquotesTreeEntry _entries[QUOTES_TREE_COUNT];
private:
	/**
	 * Inner search method
	 */
	void search(const char **str, TTquotesTreeEntry *bTree, TTtreeBuffer *buffer,
		int quoteId);

	/**
	 * Compare the current word in the string against a specified word
	 */
	bool compareWord(const char **str, const char *refStr);
public:
	/**
	 * Load data for the quotes tree
	 */
	void load();

};

} // End of namespace Titanic

#endif /* TITANIC_TT_QUOTES_TREE_H */
