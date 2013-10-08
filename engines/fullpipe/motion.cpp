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

MessageQueue *MctlCompound::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzySearch, int staticsId) {
	warning("STUB: MctlCompound::method34()");

	return 0;
}

MessageQueue *MctlCompound::method4C(StaticANIObject *subj, int xpos, int ypos, int fuzzySearch, int staticsId) {
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
		return _motionControllers[match1]->_motionControllerObj->method4C(subj, xpos, ypos, fuzzySearch, staticsId);

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

		ex->_field_20 = fuzzySearch;
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

MessageQueue *MovGraph::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzySearch, int staticsId) {
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

MessageQueue *MovGraph::method4C(StaticANIObject *subj, int xpos, int ypos, int fuzzySearch, int staticsId) {
	warning("STUB: MovGraph::method4C()");

	return 0;
}

int MovGraph::method50() {
	warning("STUB: MovGraph::method50()");

	return 0;
}

double MovGraph::calcDistance(Common::Point *point, MovGraphLink *link, int fuzzySearch) {
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
		if (fuzzySearch) {
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

MessageQueue *MovGraph2::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzySearch, int staticsId) {
	warning("STUB: MovGraph2::method34()");

	return 0;
}

MessageQueue *MovGraph2::method4C(StaticANIObject *obj, int xpos, int ypos, int fuzzySearch, int staticsId) {
	warning("STUB: MovGraph2::method4C()");
#if 0
	LinkInfo linkInfoDest;
	LinkInfo linkInfoSource;
	MovInfo1 movInfo1;
	PicAniInfo picAniInfo;
	ObList tempLinkList;

	int idx = getItemIndexByGameObjectId(obj->_id);
	ex = idx_;

	if (idx < 0)
		return 0;

	linkInfoSource.link = 0;
	linkInfoSource.node = 0;

	linkInfoDest.link = 0;
	linkInfoDest.node = 0;

	point.x = 0;

	GameObject_getPicAniInfo(obj, &picAniInfo);

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
		if (subMgm) {
			obj->_messageQueueId = 0;
			obj->changeStatics2(_items[idx]->_subItems[idxsub]->_staticsId1);
			v19 = obj->_ox;
			v20 = obj->_oy;
		} else {
			v16 = obj->_movement->calcSomeXY(point, 0);
			v63 = v16->x;
			y = v16->y;
			v17 = obj->movement;
			v18 = v17->GameObject.ox;
			point.x = v17->GameObject.oy - y;
			v19 = v18 - (_DWORD)v63;
			v20 = point.x;
			if (idxsub != 1 && idxsub) {
				if (idxsub == 2 || idxsub == 3) {
					v63 = (ExCommand *)v17->GameObject.ox;
					v20 = v17->GameObject.oy;
				}
			} else {
				v19 = v17->GameObject.ox;
				y = v17->GameObject.oy;
			}
		}
		v24 = obj->GameObject.CObject.vmt;
		obj->movement = 0;
		(*(void (__thiscall **)(GameObject *, int, int))(v24 + offsetof(GameObjectVmt, setOXY)))(&obj->GameObject, v19, v20);
	}
	v25 = obj->GameObject.oy;
	point.x = obj->GameObject.ox;
	v63 = (ExCommand *)point.x;
	v61 = v25;
	y = v25;
	if (point.x == xpos && v25 == ypos) {
		point.x = (int)operator new(sizeof(MessageQueue));
		v71.state = 0;
		if (point.x) {
			v26 = GlobalMessageQueueList_compact(&g_globalMessageQueueList);
			v62 = MessageQueue_ctor1((MessageQueue *)point, v26);
		} else {
			v62 = 0;
		}
		v71.state = -1;
		if (staticsId && obj->statics->staticsId != staticsId) {
			point.x = MovGraph2_getItem1IndexByStaticsId(this, idx, staticsId);
			if (point.x == -1) {
				GameObject_setPicAniInfo(obj, &picAniInfo);
				return 0;
			}
			ex = (int)operator new(sizeof(ExCommand));
			v71.state = 1;
			if (ex)
				v27 = ExCommand_ctor(
									 (ExCommand *)ex,
									 picAniInfo.objectId,
									 1,
									 *((_DWORD *)this->items.CObArray.m_pData[offsetof(MovGraph2, movGraph)]
									   + 186 * idx
									   + 46 * idxsub
									   + 4 * (point.x + 8)),
									 0,
									 0,
									 0,
									 1,
									 0,
									 0,
									 0);
			else
				v27 = 0;
			v28 = picAniInfo.field_8;
			v27->msg.field_24 = 1;
			v27->msg.keyCode = v28;
			v27->excFlags |= 2u;
			v71.state = -1;
			CPtrList::AddTail(&v62->exCommands, v27);
		} else {
			v29 = (ExCommand *)operator new(sizeof(ExCommand));
			point.x = (int)v29;
			v71.state = 2;
			if (v29)
				v30 = ExCommand_ctor(v29, picAniInfo.objectId, 22, obj->statics->staticsId, 0, 0, 0, 1, 0, 0, 0);
			else
				v30 = 0;
			v31 = v62;
			v30->msg.keyCode = picAniInfo.field_8;
			v32 = (int)&v31->exCommands;
			v33 = v30->excFlags | 3;
			v71.state = -1;
			v30->excFlags = v33;
			CPtrList::AddTail(&v31->exCommands, v30);
			v34 = (ExCommand *)operator new(sizeof(ExCommand));
			point.x = (int)v34;
			v71.state = 3;
			if (v34)
				v35 = ExCommand_ctor(v34, picAniInfo.objectId, 5, -1, obj->GameObject.ox, obj->GameObject.oy, 0, 1, 0, 0, 0);
			else
				v35 = 0;
			v36 = v35->excFlags;
			v35->msg.field_14 = -1;
			v35->msg.keyCode = picAniInfo.field_8;
			v71.state = -1;
			v35->excFlags = v36 | 3;
			CPtrList::AddTail(v32, v35);
		}
		GameObject_setPicAniInfo(obj, &picAniInfo);
		return v62;
	}
	linkInfoSource.node = MovGraph2_findNode(this, point.x, v25, 0);
	if (!linkInfoSource.node) {
		v38 = point.x;
		linkInfoSource.link = MovGraph2_findLink1(this, point.x, y, idxsub, 0);
		if (!(_DWORD)linkInfoSource.link) {
			linkInfoSource.link = MovGraph2_findLink2(this, v38, y);
			if (!(_DWORD)linkInfoSource.link) {
				GameObject_setPicAniInfo(obj, &picAniInfo);
				return 0;
			}
		}
	}
	linkInfoDest.node = MovGraph2_findNode(this, xpos, ypos, fuzzyMatch);
	if (!linkInfoDest.node) {
		linkInfoDest.link = MovGraph2_findLink1(this, xpos, ypos, idxsub, fuzzyMatch);
		if (!(_DWORD)linkInfoDest.link) {
			GameObject_setPicAniInfo(obj, &picAniInfo);
			return 0;
		}
	}
	ObList_ctor(&tempLinkList, 10);
	v71.state = 4;
	MovGraph2_findLinks(this, &linkInfoSource, &linkInfoDest, (int)&tempLinkList);
	if (v6 < 0.0 || (linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.m_nCount) {
		v71.state = -1;
		ObList_dtor(&tempLinkList);
		return 0;
	}
	memset(&movInfo1, 0, sizeof(movInfo1));
	v39 = y;
	movInfo1.subIndex = idxsub;
	v40 = point.x;
	movInfo1.pt1.y = y;
	movInfo1.pt1.x = point.x;
	if (linkInfoSource.node)
		v41 = linkInfoSource.node->distance;
	else
		v41 = linkInfoSource.link->movGraphNode1->distance;
	movInfo1.distance1 = v41;
	if (linkInfoDest.node) {
		v42 = linkInfoDest.node->x;
		movInfo1.pt2.x = linkInfoDest.node->x;
		v43 = linkInfoDest.node->y;
		movInfo1.pt2.y = linkInfoDest.node->y;
		movInfo1.distance2 = linkInfoDest.node->distance;
	} else {
		movInfo1.pt2.x = xpos;
		movInfo1.pt2.y = ypos;
		v44 = linkInfoDest.link->movGraphNode1;
		v45 = v44->distance;
		point.x = (ypos - v44->y) * (ypos - v44->y) + (xpos - v44->x) * (xpos - v44->x);
		v46 = sqrt((double)point.x);
		point.x = linkInfoDest.link->movGraphNode2->distance - v45;
		movInfo1.distance2 = v45 + (unsigned __int64)(signed __int64)(v46 * (double)point / linkInfoDest.link->distance);
		MovGraph_calcDistance((int)this, &movInfo1.pt2, linkInfoDest.link, 1);
		v43 = movInfo1.pt2.y;
		v42 = movInfo1.pt2.x;
		v39 = movInfo1.pt1.y;
		v40 = movInfo1.pt1.x;
	}
	if (staticsId) {
		v47 = MovGraph2_getItem1IndexByStaticsId(this, ex, staticsId);
	} else if (tempLinkList.m_nCount <= 1) {
		if (tempLinkList.m_nCount == 1)
			LOBYTE(v47) = MovGraph2_sub_456690(
											   this,
											   (int)&tempLinkList.m_pNodeHead->data->GameObject.CObject.vmt,
											   v42 - v40,
											   v43 - v39);
		else
			LOBYTE(v47) = MovGraph2_sub_456690(this, 0, v42 - v40, v43 - v39);
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
				v54 = (ExCommand *)operator new(sizeof(ExCommand));
				v63 = v54;
				LOBYTE(v71.state) = 5;
				if (v54)
					v55 = ExCommand_ctor(
										 v54,
										 picAniInfo.objectId,
										 5,
										 *(_DWORD *)(point.x + offsetof(ExCommand, messageNum)),
										 obj->GameObject.ox,
										 obj->GameObject.oy,
										 0,
										 1,
										 0,
										 0,
										 0);
				else
					v55 = 0;
				v55->msg.field_14 = -1;
				v55->msg.keyCode = picAniInfo.field_8;
				v56 = v55->excFlags | 2;
				LOBYTE(v71.state) = 4;
				v55->excFlags = v56;
				MessageQueue_addExCommand(v50, v55);
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
