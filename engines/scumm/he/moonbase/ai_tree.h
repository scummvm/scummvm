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

#include "common/array.h"
#include "scumm/he/moonbase/ai_node.h"

namespace Scumm {

const int MAX_DEPTH = 100;
const int MAX_NODES = 1000000;

class AI;

struct TreeNode {
	float value;
	Node *node;

	TreeNode(float v, Node *n) { value = v; node = n; }
};

class Tree {
private:
	Node *pBaseNode;

	int _maxDepth;
	int _maxNodes;

	int _currentChildIndex;

	Common::SortedArray<TreeNode *> *_currentMap;
	Node *_currentNode;

	AI *_ai;

public:
	Tree(AI *ai);
	Tree(IContainedObject *contents, AI *ai);
	Tree(IContainedObject *contents, int maxDepth, AI *ai);
	Tree(IContainedObject *contents, int maxDepth, int maxNodes, AI *ai);
	Tree(const Tree *sourceTree, AI *ai);
	~Tree();

	void duplicateTree(Node *sourceNode, Node *destNode);

	Node *getBaseNode() const { return pBaseNode; }
	void setMaxDepth(int maxDepth) { _maxDepth = maxDepth; }
	int getMaxDepth() const { return _maxDepth; }

	void setMaxNodes(int maxNodes) { _maxNodes = maxNodes; }
	int getMaxNodes() const { return _maxNodes; }

	Node *aStarSearch();

	Node *aStarSearch_singlePassInit();
	Node *aStarSearch_singlePass();

	int IsBaseNode(Node *thisNode);
};

} // End of namespace Scumm

#endif
