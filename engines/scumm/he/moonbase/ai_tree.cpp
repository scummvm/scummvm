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

#include "scumm/he/moonbase/ai_tree.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

Tree::Tree() {
	pBaseNode = new Node;
	_maxDepth = MAX_DEPTH;
	_maxNodes = MAX_NODES;
}

Tree::Tree(IContainedObject *contents) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = MAX_DEPTH;
	_maxNodes = MAX_NODES;
}

Tree::Tree(IContainedObject *contents, int maxDepth) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = maxDepth;
	_maxNodes = MAX_NODES;
}

Tree::Tree(IContainedObject *contents, int maxDepth, int maxNodes) {
	pBaseNode = new Node;
	pBaseNode->setContainedObject(contents);
	_maxDepth = maxDepth;
	_maxNodes = maxNodes;
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

Tree::Tree(const Tree *sourceTree) {
	pBaseNode = new Node(sourceTree->getBaseNode());
	_maxDepth = sourceTree->getMaxDepth();
	_maxNodes = sourceTree->getMaxNodes();

	duplicateTree(sourceTree->getBaseNode(), pBaseNode);
}

Tree::~Tree() {
	// Delete all nodes
	Node *pNodeItr = pBaseNode;

	// Depth first traversal of nodes to delete them
	while (pNodeItr != NULL) {
		// If any children are left, move to one of them
		if (!(pNodeItr->getChildren().empty())) {
			int size = (pNodeItr->getChildren()).size();
			pNodeItr = pNodeItr->popChild();
		} else {
			// Delete this node, and move up to the parent for further processing
			Node *pTemp = pNodeItr;
			pNodeItr = pNodeItr->getParent();
			delete pTemp;
			pTemp = NULL;
		}
	}
}


Node *Tree::aStarSearch() {
	return NULL;
#if 0
	fnpMMap mmfpOpen;

	Node *currentNode = NULL;
	float currentT;

	Node *retNode = NULL;

	float temp = pBaseNode->getContainedObject()->calcT();

	if (static_cast<int>(temp) != SUCCESS) {

		mmfpOpen.insert(fnpMMap::value_type(pBaseNode->getObjectT(), pBaseNode));

		while (mmfpOpen.size() && (retNode == NULL)) {
			currentNode = mmfpOpen.begin()->second;
			mmfpOpen.erase(mmfpOpen.begin());

			if ((currentNode->getDepth() < _maxDepth) && (Node::getNodeCount() < _maxNodes)) {
				// Generate nodes
				int numChildren = currentNode->generateChildren();
				Common::Array<Node *> vChildren = currentNode->getChildren();

				for (Common::Array<Node *>::iterator i = vChildren.begin(); i != vChildren.end(); i++) {
					IContainedObject *pTemp = (*i)->getContainedObject();
					currentT = pTemp->calcT();

					if (currentT == SUCCESS) retNode = *i;
					else mmfpOpen.insert(fnpMMap::value_type(currentT, (*i)));
				}
			} else {
				retNode = currentNode;
			}
		}
	} else {
		retNode = pBaseNode;
	}

	return retNode;
#endif
}


Node *Tree::aStarSearch_singlePassInit() {
	Node *retNode = NULL;

	currentChildIndex = 1;

	float temp = pBaseNode->getContainedObject()->calcT();

	if (static_cast<int>(temp) != SUCCESS) {
		//_currentMap.insert(fnpMMap::value_type(pBaseNode->getObjectT(), pBaseNode));
		//assert(_currentMap.size());
	} else {
		retNode = pBaseNode;
	}

	return retNode;
}

Node *Tree::aStarSearch_singlePass(Node **currentNode) {
	currentNode = NULL;
	float currentT;

	Node *retNode = NULL;

#if 0
	static int maxTime = 0;

	if (currentChildIndex == 1) {
		maxTime = getPlayerMaxTime();
	}

	if (currentChildIndex) {
		if (!(_currentMap.size())) {
			retNode = _currentNode;
			return retNode;
		}

		_currentNode = _currentMap.begin()->second;
		_currentMap.erase(_currentMap.begin());
	}

	if ((_currentNode->getDepth() < _maxDepth) && (Node::getNodeCount() < _maxNodes) && ((!maxTime) || (getTimerValue(3) < maxTime))) {
		// Generate nodes
		currentChildIndex = _currentNode->generateChildren();

		if (currentChildIndex) {
			Common::Array<Node *> vChildren = _currentNode->getChildren();

			if (!vChildren.size() && !_currentMap.size()) {
				currentChildIndex = 0;
				retNode = _currentNode;
			}

			for (Common::Array<Node *>::iterator i = vChildren.begin(); i != vChildren.end(); i++) {
				IContainedObject *pTemp = (*i)->getContainedObject();
				currentT = pTemp->calcT();

				if (currentT == SUCCESS) {
					retNode = *i;
					i = vChildren.end() - 1;
				} else {
					_currentMap.insert(fnpMMap::value_type(currentT, (*i)));
				}
			}

			if (!(_currentMap.size()) && (currentT != SUCCESS)) {
				assert(_currentNode != NULL);
				retNode = _currentNode;
			}
		}
	} else {
		retNode = _currentNode;
	}
#endif

	return retNode;
}

int Tree::IsBaseNode(Node *thisNode) {
	return (thisNode == pBaseNode);
}

} // End of namespace Scumm
