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

#include "scumm/he/moonbase/ai_node.h"

namespace Scumm {

IContainedObject::IContainedObject(IContainedObject &sourceContainedObject) {
	_objID = sourceContainedObject.getObjID();
	_valueG = sourceContainedObject.getG();
}

int Node::_nodeCount = 0;

Node::Node() {
	_parent = NULL;
	_depth = 0;
	_nodeCount++;
	_contents = NULL;
}

Node::Node(Node *sourceNode) {
	_parent = NULL;
	_children = sourceNode->getChildren();

	_depth = sourceNode->getDepth();

	_contents = sourceNode->getContainedObject()->duplicate();
}

Node::~Node() {
	if (_contents != NULL) {
		delete _contents;
		_contents = NULL;
	}

	_nodeCount--;
}

int Node::generateChildren() {
	int numChildren = _contents->numChildrenToGen();

	int numChildrenGenerated = numChildren;
	int errorCode = -1;
	static int i = 0;

	while (i < numChildren) {
		Node *tempNode = new Node;
		_children.push_back(tempNode);
		tempNode->setParent(this);
		tempNode->setDepth(_depth + 1);

		int completionFlag;

		IContainedObject *thisContObj = _contents->createChildObj(i, completionFlag);
		assert(!(thisContObj != NULL && completionFlag == 0));

		if (!completionFlag) {
			_children.pop_back();
			delete tempNode;
			return 0;
		}

		i++;

		if (thisContObj != NULL) {
			tempNode->setContainedObject(thisContObj);
		} else {
			_children.pop_back();
			delete tempNode;
			numChildrenGenerated--;
		}
	}

	i = 0;

	if (numChildrenGenerated > 0)
		return numChildrenGenerated;

	return errorCode;
}


int Node::generateNextChild() {
	int numChildren = _contents->numChildrenToGen();

	static int i = 0;

	Node *tempNode = new Node;
	_children.push_back(tempNode);
	tempNode->setParent(this);
	tempNode->setDepth(_depth + 1);

	int compFlag;
	IContainedObject *thisContObj = _contents->createChildObj(i, compFlag);

	if (thisContObj != NULL) {
		tempNode->setContainedObject(thisContObj);
	} else {
		_children.pop_back();
		delete tempNode;
	}

	++i;

	if (i > numChildren)
		i = 0;

	return i;
}

Node *Node::popChild() {
	Node *temp;

	temp = _children.back();
	_children.pop_back();
	return temp;
}

Node *Node::getFirstStep() {
	Node *currentNode = this;

	if (currentNode->getParent() == NULL)
		return currentNode;

	while (currentNode->getParent()->getParent() != NULL)
		currentNode = currentNode->getParent();

	assert(currentNode->getDepth() == 1);

	return currentNode;
}

} // End of namespace Scumm
