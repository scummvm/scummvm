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

#ifndef TITANIC_TT_STRING_NODE_H
#define TITANIC_TT_STRING_NODE_H

#include "titanic/true_talk/tt_node.h"
#include "titanic/true_talk/tt_string.h"
#include "titanic/support/exe_resources.h"

namespace Titanic {

class TTstringNode : public TTnode {
protected:
	/**
	 * Initializes state for the node
	 */
	void initialize(int mode);

	/**
	 * Initializes state for the node
	 */
	void initialize(TTstringNode *oldNode);
public:
	TTstring _string;
	FileHandle _file;
	int _mode;
	int _field1C;
public:
	TTstringNode();

	/**
	 * Find a string node in the linked chain by name
	 */
	TTstringNode *findByName(const TTstring &str, VocabMode mode);
};

} // End of namespace Titanic

#endif /* TITANIC_TT_STRING_NODE_H */
