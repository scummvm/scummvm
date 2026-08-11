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
#ifndef TOT_FOREST_H
#define TOT_FOREST_H

#include "tot/util.h"

namespace Tot {

struct nodeElement {
	char spoken;
	uint index;
};

typedef struct TreeDef *Tree;

struct TreeDef {
	nodeElement element;
	Tree parent, sibling, child;

	~TreeDef() {
		delete sibling;
		delete child;
	}
};

void initTree(Tree &a, nodeElement data);

bool isRoot(Tree node);

Tree rightSibling(Tree node);

Tree parent(Tree node);

Tree leftChild(Tree node);

int depth(Tree node);

void expandNode(Tree &node, nodeElement data);

void preOrder(Tree a, Common::String &string_);

void saveConversations(Common::SeekableWriteStream *s, Tree a, uint location);

void readTree(Common::SeekableReadStream &f, Tree &a, uint location);
void readTree(Common::String f, Tree &a, uint location);

} // End of namespace Tot
#endif
