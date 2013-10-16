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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "common/file.h"
#include "common/array.h"
#include "common/list.h"

#include "fullpipe/objects.h"
#include "fullpipe/statics.h"
#include "fullpipe/motion.h"
#include "fullpipe/messages.h"
#include "fullpipe/gameloader.h"

namespace Fullpipe {

bool MotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();

	debug(5, "MotionController::load()");

	return true;
}

bool MctlCompound::load(MfcArchive &file) {
	debug(5, "MctlCompound::load()");

	int count = file.readUint32LE();

	debug(6, "MctlCompound::count = %d", count);

	for (int i = 0; i < count; i++) {
		debug(6, "CompoundArray[%d]", i);
		MctlCompoundArrayItem *obj = new MctlCompoundArrayItem();

		obj->_motionControllerObj = (MotionController *)file.readClass();

		int count1 = file.readUint32LE();

		debug(6, "ConnectionPoint::count: %d", count1);
		for (int j = 0; j < count1; j++) {
			debug(6, "ConnectionPoint[%d]", j);
			MctlConnectionPoint *obj1 = (MctlConnectionPoint *)file.readClass();

			obj->_connectionPoints.push_back(obj1);
		}

		obj->_field_20 = file.readUint32LE();
		obj->_field_24 = file.readUint32LE();

		debug(6, "graphReact");
		obj->_movGraphReactObj = (MovGraphReact *)file.readClass();

		_motionControllers.push_back(obj);
	}

	return true;
}

void MctlCompound::addObject(StaticANIObject *obj) {
	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->addObject(obj);
}

int MctlCompound::removeObject(StaticANIObject *obj) {
	warning("STUB: MctlCompound::removeObject()");

	return 0;
}

void MctlCompound::initMovGraph2() {
	if (_objtype != kObjTypeMctlCompound)
		return;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_motionControllerObj->_objtype != kObjTypeMovGraph)
			continue;

		MovGraph *gr = (MovGraph *)_motionControllers[i]->_motionControllerObj;

		MovGraph2 *newgr = new MovGraph2();

		newgr->_links = gr->_links;
		newgr->_nodes = gr->_nodes;

		gr->_links.clear();
		gr->_nodes.clear();

		delete gr;

		_motionControllers[i]->_motionControllerObj = newgr;
	}
}

void MctlCompound::freeItems() {
	warning("STUB: MctlCompound::freeItems()");
}

MessageQueue *MctlCompound::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MctlCompound::method34()");

	return 0;
}

MessageQueue *MctlCompound::method4C(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	int match1 = -1;
	int match2 = -1;

	if (!subj)
		return 0;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(subj->_ox, subj->_oy)) {
				match1 = i;
				break;
			}
		}
	}

	if (match1 == -1)
		return 0;

	for (uint i = 0; i < _motionControllers.size(); i++) {
		if (_motionControllers[i]->_movGraphReactObj) {
			if (_motionControllers[i]->_movGraphReactObj->pointInRegion(xpos, ypos)) {
				match2 = i;
				break;
			}
		}
	}

	if (match2 == -1)
		return 0;

	if (match1 == match2)
		return _motionControllers[match1]->_motionControllerObj->method4C(subj, xpos, ypos, fuzzyMatch, staticsId);

	MctlConnectionPoint *closestP = findClosestConnectionPoint(subj->_ox, subj->_oy, match1, xpos, ypos, match2, &match2);

	if (!closestP)
		return 0;

	MessageQueue *mq = _motionControllers[match1]->_motionControllerObj->method4C(subj, closestP->_connectionX, closestP->_connectionY, 1, closestP->_field_14);

	ExCommand *ex;

	if (mq) {
		for (uint i = 0; i < closestP->_messageQueueObj->getCount(); i++) {
			ex = new ExCommand(closestP->_messageQueueObj->getExCommandByIndex(i));
			ex->_excFlags |= 2;
			mq->_exCommands.push_back(ex);
		}

		ex = new ExCommand(subj->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);

		ex->_field_20 = fuzzyMatch;
		ex->_keyCode = subj->_okeyCode;
		ex->_excFlags |= 2;

		mq->_exCommands.push_back(ex);
	}

	return mq;
}

MctlConnectionPoint *MctlCompound::findClosestConnectionPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIndex, int *minDistancePtr) {
	warning("STUB: MctlCompound::findClosestConnectionPoint()");

	return 0;
}

bool MctlCompoundArray::load(MfcArchive &file) {
	debug(5, "MctlCompoundArray::load()");

	int count = file.readUint32LE();

	debug(0, "MctlCompoundArray::count = %d", count);

	assert(0);

	return true;
}

MovGraphItem::MovGraphItem() {
	ani = 0;
	field_4 = 0;
	field_8 = 0;
	field_C = 0;
	field_10 = 0;
	field_14 = 0;
	field_18 = 0;
	field_1C = 0;
	field_20 = 0;
	field_24 = 0;
	items = 0;
	count = 0;
	field_30 = 0;
	field_34 = 0;
	field_38 = 0;
	field_3C = 0;
}

int MovGraph_messageHandler(ExCommand *cmd);

int MovGraphCallback(int a1, int a2, int a3) {
	warning("STUB: MovgraphCallback");

	return 0;
}

MovGraph::MovGraph() {
	_callback1 = MovGraphCallback;
	_field_44 = 0;
	insertMessageHandler(MovGraph_messageHandler, getMessageHandlersCount() - 1, 129);

	_objtype = kObjTypeMovGraph;
}

bool MovGraph::load(MfcArchive &file) {
	debug(5, "MovGraph::load()");

	_links.load(file);
	_nodes.load(file);

	return true;
}

void MovGraph::addObject(StaticANIObject *obj) {
	_mgm.clear();
	_mgm.addItem(obj->_id);

	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->ani == obj)
			return;

	MovGraphItem *item = new MovGraphItem();

	item->ani = obj;

	_items.push_back(item);

	_mgm.addItem(obj->_id); // FIXME: Is it really needed?
}

int MovGraph::removeObject(StaticANIObject *obj) {
	warning("STUB: MovGraph::removeObject()");

	return 0;
}

void MovGraph::freeItems() {
	warning("STUB: MovGraph::freeItems()");
}

int MovGraph::method28() {
	warning("STUB: MovGraph::method28()");

	return 0;
}

int MovGraph::method2C() {
	warning("STUB: MovGraph::method2C()");

	return 0;
}

MessageQueue *MovGraph::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MovGraph::method34()");

	return 0;
}

int MovGraph::changeCallback() {
	warning("STUB: MovGraph::changeCallback()");

	return 0;
}

int MovGraph::method3C() {
	warning("STUB: MovGraph::method3C()");

	return 0;
}

int MovGraph::method44() {
	warning("STUB: MovGraph::method44()");

	return 0;
}

MessageQueue *MovGraph::method4C(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MovGraph::method4C()");

	return 0;
}

int MovGraph::method50() {
	warning("STUB: MovGraph::method50()");

	return 0;
}

double MovGraph::calcDistance(Common::Point *point, MovGraphLink *link, int fuzzyMatch) {
	int n1x = link->_movGraphNode1->_x;
	int n1y = link->_movGraphNode1->_y;
	int n2x = link->_movGraphNode2->_x;
	int n2y = link->_movGraphNode2->_y;
	double dist1x = (double)(point->x - n1x);
	double dist1y = (double)(n1y - point->y);
	double dist2x = (double)(n2x - n1x);
	double dist2y = (double)(n2y - n1y);
	double dist1 = sqrt(dist1y * dist1y + dist1x * dist1x);
	double dist2 = ((double)(n1y - n2y) * dist1y + dist2x * dist1x) / link->_distance / dist1;
	double distm = dist2 * dist1;
	double res = sqrt(1.0 - dist2 * dist2) * dist1;

	if (dist2 <= 0.0 || distm >= link->_distance) {
		if (fuzzyMatch) {
			if (dist2 > 0.0) {
				if (distm >= link->_distance) {
					point->x = n2x;
					point->y = n2y;
				}
			} else {
				point->x = n1x;
				point->y = n1y;
			}
		} else {
			return -1.0;
		}
	} else {
		point->x = n1x + (dist2x * distm / link->_distance);
		point->y = n1y + (dist2y * distm / link->_distance);
	}

	return res;
}

int MovGraph2::getItemIndexByGameObjectId(int objectId) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->_objectId == objectId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByStaticsId(int idx, int staticsId) {
	for (int i = 0; i < 4; i++)
		if (_items[idx]->_subItems[i]._staticsId1 == staticsId || _items[idx]->_subItems[i]._staticsId2 == staticsId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByMovementId(int idx, int movId) {
	for (int i = 0; i < 4; i++)
		if (_items[idx]->_subItems[i]._walk[0]._movementId == movId || _items[idx]->_subItems[i]._turn[0]._movementId == movId ||
			_items[idx]->_subItems[i]._turnS[0]._movementId == movId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByMGM(int idx, StaticANIObject *ani) {
	warning("STUB: MovGraph2::getItemSubIndexByMGM()");

	return -1;
}

bool MovGraph2::initDirections(StaticANIObject *obj, MovGraph2Item *item) {
	item->_obj = obj;
	item->_objectId = obj->_id;

	GameVar *var = g_fullpipe->getGameLoaderGameVar()->getSubVarByName(obj->_objectName);
	if (!var)
		return false;

	var = var->getSubVarByName("Test_walk");

	if (!var)
		return false;

	GameVar *varD = 0;
	Common::Point point;

	for (int dir = 0; dir < 4; dir++) {
		switch (dir) {
		case 0:
			varD = var->getSubVarByName("Right");
			break;
		case 1:
			varD = var->getSubVarByName("Left");
			break;
		case 2:
			varD = var->getSubVarByName("Up");
			break;
		case 3:
			varD = var->getSubVarByName("Down");
			break;
		}

		if (!varD)
			return false;

		for (int act = 0; act < 3; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("Start");
				break;
			case 1:
				idx = varD->getSubVarAsInt("Go");
				break;
			case 2:
				idx = varD->getSubVarAsInt("Stop");
				break;
			}

			item->_subItems[dir]._walk[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._walk[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0);
				item->_subItems[dir]._walk[act]._mx = point.x;
				item->_subItems[dir]._walk[act]._my = point.y;
			}
		}

		for (int act = 0; act < 4; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("TurnR");
				break;
			case 1:
				idx = varD->getSubVarAsInt("TurnL");
				break;
			case 2:
				idx = varD->getSubVarAsInt("TurnU");
				break;
			case 3:
				idx = varD->getSubVarAsInt("TurnD");
				break;
			}

			item->_subItems[dir]._turn[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._turn[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0);
				item->_subItems[dir]._turn[act]._mx = point.x;
				item->_subItems[dir]._turn[act]._my = point.y;
			}
		}

		for (int act = 0; act < 4; act++) {
			int idx = 0;

			switch(act) {
			case 0:
				idx = varD->getSubVarAsInt("TurnSR");
				break;
			case 1:
				idx = varD->getSubVarAsInt("TurnSL");
				break;
			case 2:
				idx = varD->getSubVarAsInt("TurnSU");
				break;
			case 3:
				idx = varD->getSubVarAsInt("TurnSD");
				break;
			}

			item->_subItems[dir]._turnS[act]._movementId = idx;

			Movement *mov = obj->getMovementById(idx);

			item->_subItems[dir]._turnS[act]._mov = mov;
			if (mov) {
				mov->calcSomeXY(point, 0);
				item->_subItems[dir]._turnS[act]._mx = point.x;
				item->_subItems[dir]._turnS[act]._my = point.y;
			}
		}

		item->_subItems[dir]._staticsId1 = item->_subItems[dir]._walk[0]._mov->_staticsObj1->_staticsId;
		item->_subItems[dir]._staticsId2 = item->_subItems[dir]._walk[0]._mov->_staticsObj2->_staticsId;

	}
	return true;
}

void MovGraph2::addObject(StaticANIObject *obj) {
	MovGraph::addObject(obj);

	int id = getItemIndexByGameObjectId(obj->_id);

	if (id >= 0) {
		_items[id]->_obj = obj;
	} else {
		MovGraph2Item *item = new MovGraph2Item;

		if (initDirections(obj, item)) {
			_items.push_back(item);
		} else {
			delete item;
		}
	}
}

int MovGraph2::removeObject(StaticANIObject *obj) {
	warning("STUB: MovGraph2::removeObject()");

	return 0;
}

void MovGraph2::freeItems() {
	warning("STUB: MovGraph2::freeItems()");
}

MessageQueue *MovGraph2::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MovGraph2::method34()");

	return 0;
}

MessageQueue *MovGraph2::method4C(StaticANIObject *obj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	LinkInfo linkInfoDest;
	LinkInfo linkInfoSource;
	MovInfo1 movInfo1;
	PicAniInfo picAniInfo;
	Common::Point point;

	int idx = getItemIndexByGameObjectId(obj->_id);

	if (idx < 0)
		return 0;

	linkInfoSource.link = 0;
	linkInfoSource.node = 0;

	linkInfoDest.link = 0;
	linkInfoDest.node = 0;

	point.x = 0;

	obj->getPicAniInfo(&picAniInfo);

	int idxsub;

	if (obj->_movement)
		idxsub = getItemSubIndexByMovementId(idx, obj->_movement->_id);
	else
		idxsub = getItemSubIndexByStaticsId(idx, obj->_statics->_staticsId);

	bool subMgm = false;

	if (idxsub == -1) {
		idxsub = getItemSubIndexByMGM(idx, obj);
		subMgm = true;

		if (idxsub == -1)
			return 0;
	}

	if (obj->_movement) {
		int newx, newy;

		if (subMgm) {
			obj->_messageQueueId = 0;
			obj->changeStatics2(_items[idx]->_subItems[idxsub]._staticsId1);
			newx = obj->_ox;
			newy = obj->_oy;
		} else {
			obj->_movement->calcSomeXY(point, 0);
			newx = obj->_movement->_ox - point.x;
			newy = obj->_movement->_oy - point.y;
			if (idxsub != 1 && idxsub) {
				if (idxsub == 2 || idxsub == 3) {
					newy = obj->_movement->_oy;
				}
			} else {
				newx = obj->_movement->_ox;
			}
		}

		obj->_movement = 0;
		obj->setOXY(newx, newy);
	}

	if (obj->_ox == xpos && obj->_oy == ypos) {
		g_fullpipe->_globalMessageQueueList->compact();

		MessageQueue *mq = new MessageQueue();

		if (staticsId && obj->_statics->_staticsId != staticsId) {
			int idxwalk = getItemSubIndexByStaticsId(idx, staticsId);
			if (idxwalk == -1) {
				obj->setPicAniInfo(&picAniInfo);

				return 0;
			}

			ExCommand *ex = new ExCommand(picAniInfo.objectId, 1, _items[idx]->_subItems[idxsub]._walk[idxwalk]._movementId, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_24 = 1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 2;

			mq->_exCommands.push_back(ex);
		} else {
			ExCommand *ex = new ExCommand(picAniInfo.objectId, 22, obj->_statics->_staticsId, 0, 0, 0, 1, 0, 0, 0);

			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->_exCommands.push_back(ex);

			ex = new ExCommand(picAniInfo.objectId, 5, -1, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);

			ex->_field_14 = -1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->_exCommands.push_back(ex);
		}

		obj->setPicAniInfo(&picAniInfo);

		return mq;
	}

	linkInfoSource.node = findNode(obj->_ox, obj->_oy, 0);

	if (!linkInfoSource.node) {
		linkInfoSource.link = findLink1(obj->_ox, obj->_oy, idxsub, 0);

		if (!linkInfoSource.link) {
			linkInfoSource.link = findLink2(obj->_ox, obj->_oy);

			if (!linkInfoSource.link) {
				obj->setPicAniInfo(&picAniInfo);

				return 0;
			}
		}
	}

	linkInfoDest.node = findNode(xpos, ypos, fuzzyMatch);

	if (!linkInfoDest.node) {
		linkInfoDest.link = findLink1(xpos, ypos, idxsub, fuzzyMatch);
		if (!linkInfoDest.link) {
			obj->setPicAniInfo(&picAniInfo);

			return 0;
		}
	}

	Common::Array<MovGraphLink *> tempLinkList;

	if (findMinPath(&linkInfoSource, &linkInfoDest, &tempLinkList) < 0.0 || 
		((linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.size()))
		return 0;

	movInfo1.subIndex = idxsub;
	movInfo1.pt1.x = obj->_ox;
	movInfo1.pt1.y = obj->_oy;

	int dx1 = obj->_ox;
	int dy1 = obj->_oy;
	int dx2, dy2;

	if (linkInfoSource.node)
		movInfo1.distance1 = linkInfoSource.node->_distance;
	else
		movInfo1.distance1 = linkInfoSource.link->_movGraphNode1->_distance;

	if (linkInfoDest.node) {
		dx2 = linkInfoDest.node->_x;
		dy2 = linkInfoDest.node->_y;

		movInfo1.pt2.x = linkInfoDest.node->_x;
		movInfo1.pt2.y = linkInfoDest.node->_y;

		movInfo1.distance2 = linkInfoDest.node->_distance;
	} else {
		movInfo1.pt2.x = xpos;
		movInfo1.pt2.y = ypos;

		MovGraphNode *nod = linkInfoDest.link->_movGraphNode1;
		double dst1 = sqrt((ypos - nod->_y) * (ypos - nod->_y) + (xpos - nod->_x) * (xpos - nod->_x));
		int dst = linkInfoDest.link->_movGraphNode2->_distance - nod->_distance;

		movInfo1.distance2 = nod->_distance + (dst1 * (double)dst / linkInfoDest.link->_distance);

		calcDistance(&movInfo1.pt2, linkInfoDest.link, 1);

		dx1 = movInfo1.pt1.x;
		dy1 = movInfo1.pt1.y;
		dx2 = movInfo1.pt2.x;
		dy2 = movInfo1.pt2.y;
	}

	warning("STUB: MovGraph2::method4C()");

#if 0

	if (staticsId) {
		v47 = MovGraph2_getItem1IndexByStaticsId(ex, staticsId);
	} else if (tempLinkList.m_nCount <= 1) {
		if (tempLinkList.m_nCount == 1)
			LOBYTE(v47) = MovGraph2_sub_456690(
											   this,
											   (int)&tempLinkList.m_pNodeHead->data->GameObject.CObject.vmt,
											   dx2 - dx1,
											   dy2 - dy1);
		else
			LOBYTE(v47) = MovGraph2_sub_456690(this, 0, dx2 - dx1, dy2 - dy1);
	} else {
		LOBYTE(v47) = MovGraph2_sub_456300(this, (int)&tempLinkList, tempLinkList.m_pNodeTail, 0, 0);
	}
	movInfo1.flags = fuzzyMatch != 0;
	movInfo1.item1Index = v47;
	if (*((_DWORD *)this->items.CObArray.m_pData[offsetof(MovGraph2, movGraph)]
		   + 186 * movInfo1.field_0
		   + 46 * movInfo1.subIndex
		   + 3) != (unsigned __int16)v62) {
		v48 = movInfo1.flags;
		LOBYTE(v48) = LOBYTE(movInfo1.flags) | 2;
		movInfo1.flags = v48;
	}
	MovGraph2_buildMovInfo1SubItems(this, (int)&movInfo1, (int)&tempLinkList, (int)&linkInfoSource, (int)&linkInfoDest);
	v49 = MovGraph2_buildMovInfo1MessageQueue(this, (int)&movInfo1);
	v50 = (MessageQueue *)v49;
	v62 = (MessageQueue *)v49;
	CObjectFree((void *)movInfo1.items);
	v51 = MovGraph2_findNode(this, movInfo1.pt2.x, movInfo1.pt2.y, fuzzyMatch);
	linkInfoDest.node = v51;
	if (!v51) {
		linkInfoDest.link = MovGraph2_findLink1(this, movInfo1.pt2.x, movInfo1.pt2.y, movInfo1.item1Index, fuzzyMatch);
		v51 = linkInfoDest.node;
	}
	if (fuzzyMatch || (_DWORD)linkInfoDest.link || v51) {
		if (v50 && MessageQueue_getCount(v50) > 0 && picAniInfo.movementId) {
			v52 = MessageQueue_getExCommandByIndex(v50, 0);
			point.x = (int)v52;
			if (v52
				 && ((v53 = v52->msg.messageKind, v53 == 1) || v53 == 20)
				 && picAniInfo.movementId == LOWORD(v52->messageNum)
				 && picAniInfo.someDynamicPhaseIndex == v52->msg.field_14) {
				MessageQueue_deleteExCommandByIndex(v50, 0, 1);
			} else {
				ExCommand *ex = new ExCommand(picAniInfo.objectId, 5, v52->_messageNum, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);
				ex->_field_14 = -1;
				ex->_keyCode = picAniInfo.field_8;
				ex->_excFlags |= 2;
				v50->addExCommand(ex);

				v57 = (ExCommand *)operator new(sizeof(ExCommand));
				v63 = v57;
				LOBYTE(v71.state) = 6;
				if (v57) {
					v58 = ExCommand_ctor(
										 v57,
										 picAniInfo.objectId,
										 22,
										 *((_DWORD *)this->items.CObArray.m_pData[offsetof(MovGraph2, movGraph)]
										   + 186 * ex
										   + 46 * movInfo1.subIndex
										   + 3),
										 0,
										 0,
										 0,
										 1,
										 0,
										 0,
										 0);
					v50 = v62;
				}
				else
					{
						v58 = 0;
					}
				v58->msg.keyCode = picAniInfo.field_8;
				v59 = v58->excFlags | 3;
				LOBYTE(v71.state) = 4;
				v58->excFlags = v59;
				MessageQueue_addExCommand(v50, v58);
			}
		}
	} else {
		if (v50)
			(*(void (__thiscall **)(MessageQueue *, signed int))(v50->CObject.vmt + 4))(v50, 1);
		v50 = 0;
	}
	GameObject_setPicAniInfo(obj, &picAniInfo);
	v71.state = -1;
	ObList_dtor(&tempLinkList);
	return v50;
#endif

	return 0;
}

MovGraphNode *MovGraph2::findNode(int x, int y, int fuzzyMatch) {
	warning("STUB: MovGraphLink *MovGraph2::findNode()");

	return 0;
}

MovGraphLink *MovGraph2::findLink1(int x, int y, int idx, int fuzzyMatch) {
	warning("STUB: MovGraphLink *MovGraph2::findLink1()");

	return 0;
}

MovGraphLink *MovGraph2::findLink2(int x, int y) {
	warning("STUB: MovGraphLink *MovGraph2::findLink2()");

	return 0;
}

double MovGraph2::findMinPath(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, Common::Array<MovGraphLink *> *listObj) {
	warning("STUB: MovGraph2::findMinPath()");

	return 0.0;
}

MovGraphNode *MovGraph::calcOffset(int ox, int oy) {
	warning("STUB: MovGraph::calcOffset()");

	return 0;
}

void MGM::clear() {
	_items.clear();
}

MGMItem::MGMItem() {
	objId = 0;
}

MGMSubItem::MGMSubItem() {
	movement = 0;
	staticsIndex = 0;
	field_8 = 0;
	field_C = 0;
	x = 0;
	y = 0;
}

void MGM::addItem(int objId) {
	if (getItemIndexById(objId) == -1) {
		MGMItem *item = new MGMItem();

		item->objId = objId;
		_items.push_back(item);
	}
	rebuildTables(objId);
}

void MGM::rebuildTables(int objId) {
	int idx = getItemIndexById(objId);

	if (idx == -1)
		return;

	_items[idx]->subItems.clear();
	_items[idx]->statics.clear();
	_items[idx]->movements1.clear();
	_items[idx]->movements2.clear();

	StaticANIObject *obj = g_fullpipe->_currentScene->getStaticANIObject1ById(objId, -1);

	if (!obj)
		return;

	for (uint i = 0; i < obj->_staticsList.size(); i++)
		_items[idx]->statics.push_back((Statics *)obj->_staticsList[i]);

	for (uint i = 0; i < obj->_movements.size(); i++)
		_items[idx]->movements1.push_back((Movement *)obj->_movements[i]);

	_items[idx]->subItems.clear();
}

int MGM::getItemIndexById(int objId) {
	for (uint i = 0; i < _items.size(); i++)
		if (_items[i]->objId == objId)
			return i;

	return -1;
}

MovGraphLink::MovGraphLink() {
	_distance = 0;
	_angle = 0;
	_flags = 0x10000000;
	_movGraphNode2 = 0;
	_movGraphNode1 = 0;
	_field_3C = 0;
	_field_38 = 0;
	_movGraphReact = 0;
	_name = 0;
}

bool MovGraphLink::load(MfcArchive &file) {
	debug(5, "MovGraphLink::load()");

	_dwordArray1.load(file);
	_dwordArray2.load(file);

	_flags = file.readUint32LE();

	debug(8, "GraphNode1");
	_movGraphNode1 = (MovGraphNode *)file.readClass();
	debug(8, "GraphNode2");
	_movGraphNode2 = (MovGraphNode *)file.readClass();

	_distance = file.readDouble();
	_angle = file.readDouble();

	debug(8, "distance: %g, angle: %g", _distance, _angle);

	_movGraphReact = (MovGraphReact *)file.readClass();
	_name = file.readPascalString();

	return true;
}

bool MovGraphNode::load(MfcArchive &file) {
	debug(5, "MovGraphNode::load()");

	_field_14 = file.readUint32LE();
	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_distance = file.readUint32LE();

	return true;
}

ReactParallel::ReactParallel() {
	_x1 = 0;
	_x2 = 0;
	_dy = 0;
	_dx = 0;
	_y1 = 0;
	_y2 = 0;
}

bool ReactParallel::load(MfcArchive &file) {
	debug(5, "ReactParallel::load()");

	_x1 = file.readUint32LE();
	_y1 = file.readUint32LE();
	_x2 = file.readUint32LE();
	_y2 = file.readUint32LE();
	_dx = file.readUint32LE();
	_dy = file.readUint32LE();

	createRegion();

	return true;
}

void ReactParallel::createRegion() {
	_points = (Common::Point **)malloc(sizeof(Common::Point *) * 4);

	for (int i = 0; i < 4; i++)
		_points[i] = new Common::Point;

	double at = atan2((double)(_x1 - _x2), (double)(_y1 - _y2)) + 1.570796;
	double sn = sin(at);
	double cs = cos(at);

	_points[0]->x = (int16)(_x1 - _dx * cs);
	_points[0]->y = (int16)(_y1 - _dx * sn);

	_points[1]->x = (int16)(_x2 - _dx * cs);
	_points[1]->y = (int16)(_y2 - _dx * sn);

	_points[2]->x = (int16)(_x2 + _dy * cs);
	_points[2]->y = (int16)(_y2 + _dy * sn);

	_points[3]->x = (int16)(_x1 + _dy * cs);
	_points[3]->y = (int16)(_y1 + _dy * sn);

	_pointCount = 4;
	// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, 4, 2);
}

void ReactParallel::method14() {
	warning("STUB: ReactParallel::method14()");
}

ReactPolygonal::ReactPolygonal() {
	_field_C = 0;
	_field_10 = 0;
}

bool ReactPolygonal::load(MfcArchive &file) {
	debug(5, "ReactPolygonal::load()");

	_field_C = file.readUint32LE();
	_field_10 = file.readUint32LE();
	_pointCount = file.readUint32LE();

	if (_pointCount > 0) {
		_points = (Common::Point **)malloc(sizeof(Common::Point *) * _pointCount);

		for (int i = 0; i < _pointCount; i++) {
			_points[i] = new Common::Point;

			_points[i]->x = file.readUint32LE();
			_points[i]->y = file.readUint32LE();
		}

	}

	createRegion();

	return true;
}

void ReactPolygonal::createRegion() {
	if (_points) {

		// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, _pointCount, 2);
	}
}

void ReactPolygonal::method14() {
	warning("STUB: ReactPolygonal::method14()");
}

bool MovGraphReact::pointInRegion(int x, int y) {
	if (_pointCount < 3) {
		return false;
	}

	int counter = 0;
	double xinters;
	Common::Point p, p1, p2;

	p.x = (double)x;
	p.y = (double)y;

	p1.x = (double)_points[0]->x;
	p1.y = (double)_points[0]->y;

	for (int i = 1; i <= _pointCount; i++) {
		p2.x = (double)_points[i % _pointCount]->x;
		p2.y = (double)_points[i % _pointCount]->y;

		if (p.y > MIN(p1.y, p2.y)) {
			if (p.y <= MAX(p1.y, p2.y)) {
				if (p.x <= MAX(p1.x, p2.x)) {
					if (p1.y != p2.y) {
						xinters = (p.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
						if (p1.x == p2.x || p.x <= xinters) {
							counter++;
						}
					}
				}
			}
		}
		p1 = p2;
	}

	if (counter % 2 == 0) {
		return false;
	} else {
		return true;
	}
}

int startWalkTo(int objId, int objKey, int x, int y, int a5) {
	MctlCompound *mc = getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId);

	if (mc)
		return (mc->method34(g_fullpipe->_currentScene->getStaticANIObject1ById(objId, objKey), x, y, a5, 0) != 0);

	return 0;
}

int doSomeAnimation(int objId, int objKey, int a3) {
	warning("STUB: doSomeAnimation(%d, %d, %d)", objId, objKey, a3);

	return 0;
}

int doSomeAnimation2(int objId, int objKey) {
	return doSomeAnimation(objId, objKey, 0);
}

} // End of namespace Fullpipe
