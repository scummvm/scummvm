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

#ifndef SCUMM_HE_MOONBASE_AI_TREE_H
#define SCUMM_HE_MOONBASE_AI_TREE_H

#include "common/hash-str.h"
#include "scumm/he/moonbase/ai_node.h"

namespace Scumm {

//typedef std::multimap< float, Node *, std::less<float> > fnpMMap;

const int MAX_DEPTH = 100;
const int MAX_NODES = 1000000;

class Tree {
private:
	Node *pBaseNode;

	int _maxDepth;
	int _maxNodes;

	int currentChildIndex;

	Common::StringMap _currentMap;
	Node *_currentNode;

public:
	Tree();
	Tree(IContainedObject *contents);
	Tree(IContainedObject *contents, int maxDepth);
	Tree(IContainedObject *contents, int maxDepth, int maxNodes);
	Tree(const Tree *sourceTree);
	~Tree();

	void duplicateTree(Node *sourceNode, Node *destNode);

	Node *getBaseNode() const { return pBaseNode; }
	void setMaxDepth(int maxDepth) { _maxDepth = maxDepth; }
	int getMaxDepth() const { return _maxDepth; }

	void setMaxNodes(int maxNodes) { _maxNodes = maxNodes; }
	int getMaxNodes() const { return _maxNodes; }

	Node *aStarSearch();

	Node *aStarSearch_singlePassInit();
	Node *aStarSearch_singlePass(Node **currentNode);

	int IsBaseNode(Node *thisNode);
};

} // End of namespace Scumm

#endif
