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

#include "common/list.h"

namespace Scumm {

const float SUCCESS = -1;
const float FAILURE = 1e20;

class IContainedObject {
private:
	int objID;
	float valueG;

protected:
	virtual float getG() const { return valueG; }
	virtual float calcH() { return 0; }


public:
	IContainedObject() { valueG = 0; }
	IContainedObject(float inG) { valueG = inG; }
	IContainedObject(IContainedObject &sourceContainedObject);
	virtual ~IContainedObject() {}

	virtual IContainedObject *duplicate() = 0;

	void setValueG(float inG) { valueG = inG; }
	float getValueG() { return valueG; }

	int getObjID() const { return objID; }
	void setObjID(int inputObjID) { objID = inputObjID; }

	virtual int numChildrenToGen() = 0;
	virtual IContainedObject *createChildObj(int index, int &completionFlag) = 0;

	virtual int checkSuccess() = 0;
	virtual float calcT() { return getG(); }

	float returnG() const { return getG(); }
};

class Node {
private:
	Node *pParent;
	Common::List<Node *> vpChildren;

	int m_depth;
	static int m_nodeCount;

	IContainedObject *pContents;

public:
	Node();
	Node(Node *sourceNode);
	~Node();

	void setParent(Node *parentPtr) { pParent = parentPtr; }
	Node *getParent() const { return pParent; }

	void setDepth(int depth) { m_depth = depth; }
	int getDepth() const { return m_depth; }

	static int getNodeCount() { return m_nodeCount; }

	void setContainedObject(IContainedObject *pValue) { pContents = pValue; }
	IContainedObject *getContainedObject() { return pContents; }

	Common::List<Node *> getChildren() const { return vpChildren; }
	int generateChildren();
	int generateNextChild();
	Node *popChild();

	float getObjectT() { return pContents->calcT(); }

	Node *getFirstStep();

	void printPath();
};

} // End of namespace Scumm

#endif
