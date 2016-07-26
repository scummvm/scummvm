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

#ifndef SCUMM_HE_MOONBASE_AI_NODE_H
#define SCUMM_HE_MOONBASE_AI_NODE_H

#include "common/array.h"

namespace Scumm {

const float SUCCESS = -1;
const float FAILURE = 1e20f;

class IContainedObject {
private:
	int _objID;
	float _valueG;

protected:
	virtual float getG() const { return _valueG; }
	virtual float calcH() { return 0; }

public:
	IContainedObject() { _valueG = 0; _objID = -1; }
	IContainedObject(float inG) { _valueG = inG; _objID = -1; }
	IContainedObject(IContainedObject &sourceContainedObject);
	virtual ~IContainedObject() {}

	virtual IContainedObject *duplicate() = 0;

	void setValueG(float inG) { _valueG = inG; }
	float getValueG() { return _valueG; }

	int getObjID() const { return _objID; }
	void setObjID(int inputObjID) { _objID = inputObjID; }

	virtual int numChildrenToGen() = 0;
	virtual IContainedObject *createChildObj(int index, int &completionFlag) = 0;

	virtual int checkSuccess() = 0;
	virtual float calcT() { return getG(); }

	float returnG() const { return getG(); }
};

class Node {
private:
	Node *_parent;
	Common::Array<Node *> _children;

	int _depth;
	static int _nodeCount;

	IContainedObject *_contents;

public:
	Node();
	Node(Node *sourceNode);
	~Node();

	void setParent(Node *parentPtr) { _parent = parentPtr; }
	Node *getParent() const { return _parent; }

	void setDepth(int depth) { _depth = depth; }
	int getDepth() const { return _depth; }

	static int getNodeCount() { return _nodeCount; }

	void setContainedObject(IContainedObject *value) { _contents = value; }
	IContainedObject *getContainedObject() { return _contents; }

	Common::Array<Node *> getChildren() const { return _children; }
	int generateChildren();
	int generateNextChild();
	Node *popChild();

	float getObjectT() { return _contents->calcT(); }

	Node *getFirstStep();
};

} // End of namespace Scumm

#endif
