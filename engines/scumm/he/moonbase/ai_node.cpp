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

#include "scumm/he/moonbase/ai_node.h"

namespace Scumm {

IContainedObject::IContainedObject(IContainedObject &sourceContainedObject) {
	_objID = sourceContainedObject.getObjID();
	_valueG = sourceContainedObject.getG();
}

int Node::_nodeCount = 0;

Node::Node() {
	_parent = nullptr;
	_depth = 0;
	_nodeCount++;
	_contents = nullptr;
}

Node::Node(Node *sourceNode) {
	_parent = nullptr;
	_children = sourceNode->getChildren();

	_depth = sourceNode->getDepth();

	_contents = sourceNode->getContainedObject()->duplicate();
}

Node::~Node() {
	if (_contents != nullptr) {
		delete _contents;
		_contents = nullptr;
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
		assert(!(thisContObj != nullptr && completionFlag == 0));

		if (!completionFlag) {
			_children.pop_back();
			delete tempNode;
			return 0;
		}

		i++;

		if (thisContObj != nullptr) {
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

	if (thisContObj != nullptr) {
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

	if (currentNode->getParent() == nullptr)
		return currentNode;

	while (currentNode->getParent()->getParent() != nullptr)
		currentNode = currentNode->getParent();

	assert(currentNode->getDepth() == 1);

	return currentNode;
}

} // End of namespace Scumm
