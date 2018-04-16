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

#include "titanic/true_talk/tt_quotes_tree.h"
#include "titanic/support/files_manager.h"
#include "titanic/titanic.h"
#include "common/algorithm.h"

namespace Titanic {

/**
 * Specifies the starting index for each of the three main trees
 */
static uint TABLE_INDEXES[3] = { 922, 1015, 1018 };

void TTquotesTree::load() {
	Common::SeekableReadStream *r = g_vm->_filesManager->getResource("TEXT/TREE");

	for (int idx = 0; idx < QUOTES_TREE_COUNT; ++idx) {
		TTquotesTreeEntry &rec = _entries[idx];
		assert(r->pos() < r->size());

		rec._id = r->readUint32LE();
		if (rec._id == 0) {
			// Nothing needed
		} else {
			byte type = r->readByte();
			if (type == 0) {
				// Index to sub-table
				rec._subTable = &_entries[0] + r->readUint32LE();
			} else {
				// Read in string for entry
				char c;
				while ((c = r->readByte()) != '\0')
					rec._string += c;
			}
		}
	}

	assert(r->pos() == r->size());
	delete r;
}

int TTquotesTree::search(const char *str, QuoteTreeNum treeNum,
		TTtreeResult *buffer, uint tagId, uint *remainder) {
	const TTquotesTreeEntry *bTree = &_entries[TABLE_INDEXES[treeNum]];
	if (!search1(&str, bTree, buffer, tagId) || !buffer->_treeItemP)
		return -1;

	if (remainder) {
		for (; *str; ++str) {
			if (*str >= 'a' && *str != 's')
				*remainder += *str;
		}
	}

	return buffer->_treeItemP->_id & 0xffffff;
}

bool TTquotesTree::search1(const char **str, const TTquotesTreeEntry *bTree,
		TTtreeResult *buffer, uint tagId) {
	buffer->_treeItemP = nullptr;
	(buffer + 1)->_treeItemP = nullptr;

	const char *strP = *str;
	bool flag = false;

	for (uint mode = bTree->_id >> 24; mode != 0;
			++bTree, mode = bTree->_id >> 24) {

		switch (mode) {
		case 1:
			if (compareWord(str, bTree->_string.c_str()))
				flag = true;
			break;

		case 2:
			compareWord(str, bTree->_string.c_str());
			break;

		case 5:
			if (READ_LE_UINT32(bTree->_string.c_str()) == tagId)
				flag = true;
			break;

		case 7:
			if (search1(str, bTree->_subTable, buffer + 1, tagId))
				flag = true;
			break;

		case 8:
			if (search2(str, bTree->_subTable, buffer + 1, tagId))
				flag = true;
			break;

		default:
			break;
		}

		if (flag) {
			buffer->_treeItemP = bTree;
			return true;
		}
	}

	*str = strP;
	return false;
}

bool TTquotesTree::search2(const char **str, const TTquotesTreeEntry *bTree,
		TTtreeResult *buffer, uint tagId) {
	buffer->_treeItemP = bTree;
	(buffer + 1)->_treeItemP = nullptr;

	const char *strP = *str;
	bool flag = false;
	for (uint mode = bTree->_id >> 24; mode != 0;
			++bTree, mode = bTree->_id >> 24) {
		switch (mode) {
		case 0:
			return true;

		case 1:
			if (compareWord(str, bTree->_string.c_str()))
				flag = true;
			break;

		case 2:
			compareWord(str, bTree->_string.c_str());
			break;

		case 5:
			if (READ_LE_UINT32(bTree->_string.c_str()) == tagId)
				flag = true;
			break;

		case 7:
			if (search1(str, bTree->_subTable, buffer + 1, tagId))
				flag = true;
			break;

		case 8:
			if (search2(str, bTree->_subTable, buffer + 1, tagId))
				flag = true;
			break;

		default:
			break;
		}

		if (flag) {
			buffer->_treeItemP = nullptr;
			*str = strP;
			return false;
		}
	}

	return true;
}

bool TTquotesTree::compareWord(const char **str, const char *refStr) {
	// Skip over any spaces
	const char *strP = *str;
	while (*strP && *strP == ' ')
		++strP;
	*str = strP;

	// Compare against the reference string
	while (*strP && *refStr && *refStr != '*') {
		if (*refStr == '-') {
			if (*strP == ' ')
				++strP;
		} else if (*strP == *refStr) {
			++strP;
		} else {
			return false;
		}
	}

	if (*refStr && *refStr != '*')
		return false;
	if (!*refStr && *strP && *strP != ' ')
		return false;

	if (*refStr == '*') {
		// Skip over to the end of the word
		while (*strP && *strP != ' ')
			++strP;
	}

	// Pass out the new updated string position
	*str = strP;
	return true;
}

} // End of namespace Titanic
