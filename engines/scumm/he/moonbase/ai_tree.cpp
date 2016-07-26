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

#include "scumm/he/intern_he.h"

#include "scumm/he/moonbase/moonbase.h"
#include "scumm/he/moonbase/ai_tree.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

static int compareTreeNodes(const void *a, const void *b) {
	if (((const TreeNode *)a)->value < ((const TreeNode *)b)->value)
		return -1;
	else if (((const TreeNode *)a)->value > ((const TreeNode *)b)->value)
		return 1;
	else
		return 0;
}

Tree::Tree(AI *ai) : _ai(ai) {
	pBaseNode = new Node;
	_maxDepth = MAX_DEPTH;
	_maxNodes = MAX_NODES;
	_currentNode = 0;
	_currentChildIndex = 0;

	_currentMap = new Common::SortedArray<TreeNode *>(compareTreeNodes);
}

Tree::Tree(IContainedObject *contents, AI *ai) : _ai(ai) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = MAX_DEPTH;
	_maxNodes = MAX_NODES;
	_currentNode = 0;
	_currentChildIndex = 0;

	_currentMap = new Common::SortedArray<TreeNode *>(compareTreeNodes);
}

Tree::Tree(IContainedObject *contents, int maxDepth, AI *ai) : _ai(ai) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = maxDepth;
	_maxNodes = MAX_NODES;
	_currentNode = 0;
	_currentChildIndex = 0;

	_currentMap = new Common::SortedArray<TreeNode *>(compareTreeNodes);
}

Tree::Tree(IContainedObject *contents, int maxDepth, int maxNodes, AI *ai) : _ai(ai) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = maxDepth;
	_maxNodes = maxNodes;
	_currentNode = 0;
	_currentChildIndex = 0;

	_currentMap = new Common::SortedArray<TreeNode *>(compareTreeNodes);
}

void Tree::duplicateTree(Node *sourceNode, Node *destNode) {
	Common::Array<Node *> vUnvisited = sourceNode->getChildren();

	while (vUnvisited.size()) {
		Node *newNode = new Node(*(vUnvisited.end()));
		newNode->setParent(destNode);
		(destNode->getChildren()).push_back(newNode);
		duplicateTree(*(vUnvisited.end()), newNode);
		vUnvisited.pop_back();
	}
}

Tree::Tree(const Tree *sourceTree, AI *ai) : _ai(ai) {
	pBaseNode = new Node(sourceTree->getBaseNode());
	_maxDepth = sourceTree->getMaxDepth();
	_maxNodes = sourceTree->getMaxNodes();
	_currentMap = new Common::SortedArray<TreeNode *>(compareTreeNodes);
	_currentNode = 0;
	_currentChildIndex = 0;

	duplicateTree(sourceTree->getBaseNode(), pBaseNode);
}

Tree::~Tree() {
	// Delete all nodes
	Node *pNodeItr = pBaseNode;

	// Depth first traversal of nodes to delete them
	while (pNodeItr != NULL) {
		// If any children are left, move to one of them
		if (!(pNodeItr->getChildren().empty())) {
			pNodeItr = pNodeItr->popChild();
		} else {
			// Delete this node, and move up to the parent for further processing
			Node *pTemp = pNodeItr;
			pNodeItr = pNodeItr->getParent();
			delete pTemp;
			pTemp = NULL;
		}
	}

	delete _currentMap;
}

Node *Tree::aStarSearch() {
	Common::SortedArray<TreeNode *> mmfpOpen(compareTreeNodes);

	Node *currentNode = NULL;
	float currentT;

	Node *retNode = NULL;

	float temp = pBaseNode->getContainedObject()->calcT();

	if (static_cast<int>(temp) != SUCCESS) {
		mmfpOpen.insert(new TreeNode(pBaseNode->getObjectT(), pBaseNode));

		while (mmfpOpen.size() && (retNode == NULL)) {
			currentNode = mmfpOpen.front()->node;
			mmfpOpen.erase(mmfpOpen.begin());

			if ((currentNode->getDepth() < _maxDepth) && (Node::getNodeCount() < _maxNodes)) {
				// Generate nodes
				Common::Array<Node *> vChildren = currentNode->getChildren();

				for (Common::Array<Node *>::iterator i = vChildren.begin(); i != vChildren.end(); i++) {
					IContainedObject *pTemp = (*i)->getContainedObject();
					currentT = pTemp->calcT();

					if (currentT == SUCCESS)
						retNode = *i;
					else
						mmfpOpen.insert(new TreeNode(currentT, (*i)));
				}
			} else {
				retNode = currentNode;
			}
		}
	} else {
		retNode = pBaseNode;
	}

	return retNode;
}


Node *Tree::aStarSearch_singlePassInit() {
	Node *retNode = NULL;

	_currentChildIndex = 1;

	float temp = pBaseNode->getContainedObject()->calcT();

	if (static_cast<int>(temp) != SUCCESS) {
		_currentMap->insert(new TreeNode(pBaseNode->getObjectT(), pBaseNode));
	} else {
		retNode = pBaseNode;
	}

	return retNode;
}

Node *Tree::aStarSearch_singlePass() {
	float currentT = 0.0;
	Node *retNode = NULL;

	static int maxTime = 0;

	if (_currentChildIndex == 1) {
		maxTime = _ai->getPlayerMaxTime();
	}

	if (_currentChildIndex) {
		if (!(_currentMap->size())) {
			retNode = _currentNode;
			return retNode;
		}

		_currentNode = _currentMap->front()->node;
		_currentMap->erase(_currentMap->begin());
	}

	if ((_currentNode->getDepth() < _maxDepth) && (Node::getNodeCount() < _maxNodes) && ((!maxTime) || (_ai->getTimerValue(3) < maxTime))) {
		// Generate nodes
		_currentChildIndex = _currentNode->generateChildren();

		if (_currentChildIndex) {
			Common::Array<Node *> vChildren = _currentNode->getChildren();

			if (!vChildren.size() && !_currentMap->size()) {
				_currentChildIndex = 0;
				retNode = _currentNode;
			}

			for (Common::Array<Node *>::iterator i = vChildren.begin(); i != vChildren.end(); i++) {
				IContainedObject *pTemp = (*i)->getContainedObject();
				currentT = pTemp->calcT();

				if (currentT == SUCCESS) {
					retNode = *i;
					i = vChildren.end() - 1;
				} else {
					_currentMap->insert(new TreeNode(currentT, (*i)));
				}
			}

			if (!(_currentMap->size()) && (currentT != SUCCESS)) {
				assert(_currentNode != NULL);
				retNode = _currentNode;
			}
		}
	} else {
		retNode = _currentNode;
	}

	return retNode;
}

int Tree::IsBaseNode(Node *thisNode) {
	return (thisNode == pBaseNode);
}

} // End of namespace Scumm
