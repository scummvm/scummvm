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
#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/messages.h"

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
	for (uint i = 0; i < _motionControllers.size(); i++)
		_motionControllers[i]->_motionControllerObj->freeItems();
}

MessageQueue *MctlCompound::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MctlCompound::method34()");

	return 0;
}

MessageQueue *MctlCompound::doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
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
		return _motionControllers[match1]->_motionControllerObj->doWalkTo(subj, xpos, ypos, fuzzyMatch, staticsId);

	MctlConnectionPoint *closestP = findClosestConnectionPoint(subj->_ox, subj->_oy, match1, xpos, ypos, match2, &match2);

	if (!closestP)
		return 0;

	MessageQueue *mq = _motionControllers[match1]->_motionControllerObj->doWalkTo(subj, closestP->_connectionX, closestP->_connectionY, 1, closestP->_field_14);

	ExCommand *ex;

	if (mq) {
		for (uint i = 0; i < closestP->_messageQueueObj->getCount(); i++) {
			ex = new ExCommand(closestP->_messageQueueObj->getExCommandByIndex(i));
			ex->_excFlags |= 2;
			mq->addExCommandToEnd(ex);
		}

		ex = new ExCommand(subj->_id, 51, 0, xpos, ypos, 0, 1, 0, 0, 0);

		ex->_field_20 = fuzzyMatch;
		ex->_keyCode = subj->_okeyCode;
		ex->_excFlags |= 2;

		mq->addExCommandToEnd(ex);
	}

	return mq;
}

MctlCompoundArrayItem::~MctlCompoundArrayItem() {
	delete _movGraphReactObj;
	delete _motionControllerObj;
}

MctlLadder::MctlLadder() {
	_width = 0;
	_ladderX = 0;
	_height = 0;
	_ladderY = 0;
	_ladder_field_14 = 0;

	_ladder_field_20 = 0;
	_ladder_field_24 = 0;
}

MctlLadder::~MctlLadder() {
	freeItems();
}

int MctlLadder::collisionDetection(StaticANIObject *man) {
	if (findObjectPos(man) < 0)
		return 0;

	double delta;

	if ((double)(man->_oy - _ladderY) / (double)_height < 0.0)
		delta = -0.5;
	else 
		delta = 0.5;

	int res = (int)((double)(man->_oy - _ladderY) / (double)_height + delta);

	if (res < 0)
		return 0;

	return res;
}

void MctlLadder::addObject(StaticANIObject *obj) {
	if (findObjectPos(obj) < 0) {
		MctlLadderMovement *movement = new MctlLadderMovement;
		
		if (initMovement(obj, movement)) {
			_mgm.addItem(obj->_id);
			_movements.push_back(movement);
		} else {
			delete movement;
		}
	}
}

int MctlLadder::findObjectPos(StaticANIObject *obj) {
	int res = -1;

	for (Common::List<MctlLadderMovement *>::iterator it = _movements.begin(); it != _movements.end(); ++it, ++res)
		if ((*it)->objId == obj->_id)
			break;

	return res;
}

bool MctlLadder::initMovement(StaticANIObject *ani, MctlLadderMovement *movement) {
	GameVar *v = g_fullpipe->getGameLoaderGameVar()->getSubVarByName(ani->getName());

	if (!v)
		return false;

	v = v->getSubVarByName("Test_Ladder");

	if (!v)
		return false;

	movement->staticIdsSize = 6;
	movement->movVars = new MctlLadderMovementVars;
	movement->staticIds = new int[movement->staticIdsSize];

	v = v->getSubVarByName("Up");

	if (!v)
		return false;

	movement->movVars->varUpStart = v->getSubVarAsInt("Start");
	movement->movVars->varUpGo = v->getSubVarAsInt("Go");
	movement->movVars->varUpStop = v->getSubVarAsInt("Stop");

	movement->staticIds[0] = ani->getMovementById(movement->movVars->varUpStart)->_staticsObj1->_staticsId;
	movement->staticIds[2] = ani->getMovementById(movement->movVars->varUpGo)->_staticsObj1->_staticsId;

	v = v->getSubVarByName("Down");

	if (!v)
		return false;

	movement->movVars->varDownStart = v->getSubVarAsInt("Start");
	movement->movVars->varDownGo = v->getSubVarAsInt("Go");
	movement->movVars->varDownStop = v->getSubVarAsInt("Stop");

	movement->staticIds[1] = ani->getMovementById(movement->movVars->varDownStart)->_staticsObj1->_staticsId;
	movement->staticIds[3] = ani->getMovementById(movement->movVars->varDownGo)->_staticsObj1->_staticsId;

	movement->objId = ani->_id;

	return true;
}

void MctlLadder::freeItems() {
	_mgm.clear();

	for (Common::List<MctlLadderMovement *>::iterator it = _movements.begin(); it != _movements.end(); ++it) {
		delete (*it)->movVars;
		delete [] (*it)->staticIds;
	}

	_movements.clear();
}

MessageQueue *MctlLadder::method34(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	MessageQueue *mq = doWalkTo(subj, xpos, ypos, fuzzyMatch, staticsId);

	if (mq) {
		if (mq->chain(subj))
			return mq;

	} else {
		delete mq;
	}

	return 0;
}

MessageQueue *MctlLadder::doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MctlLadder::doWalkTo()");

	return 0;
}

MessageQueue *MctlLadder::controllerWalkTo(StaticANIObject *ani, int off) {
	return doWalkTo(ani, _ladderX + off * _width, _ladderY + off * _height, 1, 0);
}

MctlConnectionPoint *MctlCompound::findClosestConnectionPoint(int ox, int oy, int destIndex, int connectionX, int connectionY, int sourceIndex, int *minDistancePtr) {
	warning("STUB: MctlCompound::findClosestConnectionPoint()");

	return 0;
}

MctlConnectionPoint::MctlConnectionPoint() {
	_connectionX = 0;
	_connectionY = 0;
	_field_C = 0;
	_field_10 = 0;
	_field_14 = 0;
	_field_16 = 0;
	_messageQueueObj = 0;
	_motionControllerObj = 0;
}

MctlConnectionPoint::~MctlConnectionPoint() {
	delete _messageQueueObj;
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

MessageQueue *MovGraph::doWalkTo(StaticANIObject *subj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	warning("STUB: MovGraph::doWalkTo()");

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

void MovGraph::calcNodeDistancesAndAngles() {
	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		lnk->_flags &= 0x7FFFFFFF;

		lnk->calcNodeDistanceAndAngle();
	}
}

int MovGraph2::getItemIndexByGameObjectId(int objectId) {
	for (uint i = 0; i < _items2.size(); i++)
		if (_items2[i]->_objectId == objectId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByStaticsId(int idx, int staticsId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]->_subItems[i]._staticsId1 == staticsId || _items2[idx]->_subItems[i]._staticsId2 == staticsId)
			return i;

	return -1;
}

int MovGraph2::getItemSubIndexByMovementId(int idx, int movId) {
	for (int i = 0; i < 4; i++)
		if (_items2[idx]->_subItems[i]._walk[0]._movementId == movId || _items2[idx]->_subItems[i]._turn[0]._movementId == movId ||
			_items2[idx]->_subItems[i]._turnS[0]._movementId == movId)
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
		_items2[id]->_obj = obj;
	} else {
		MovGraph2Item *item = new MovGraph2Item;

		if (initDirections(obj, item)) {
			_items2.push_back(item);
		} else {
			delete item;
		}
	}
}

void MovGraph2::buildMovInfo1SubItems(MovInfo1 *movinfo, Common::Array<MovGraphLink *> *linkList, LinkInfo *lnkSrc, LinkInfo *lnkDst) {
	MovInfo1Sub *elem;
	Common::Point point;
	Common::Rect rect;

	int subIndex = movinfo->subIndex;

	movinfo->items.clear();

	elem = new MovInfo1Sub;
	elem->subIndex = subIndex;
	elem->x = movinfo->pt1.x;
	elem->y = movinfo->pt1.y;
	elem->distance = -1;

	movinfo->items.push_back(elem);

	int prevSubIndex = movinfo->subIndex;

	for (uint i = 0; i < linkList->size(); i++) {
		int idx1;

		if (linkList->size() <= 1) {
			if (linkList->size() == 1)
				idx1 = getShortSide((*linkList)[0], movinfo->pt2.x - movinfo->pt1.x, movinfo->pt2.y - movinfo->pt1.y);
			else
				idx1 = getShortSide(0, movinfo->pt2.x - movinfo->pt1.x, movinfo->pt2.y - movinfo->pt1.y);

			point.y = -1;
			rect.bottom = -1;
			rect.right = -1;
			rect.top = -1;
			rect.left = -1;
		} else {
			idx1 = findLink(linkList, i, &rect, &point);
		}

		if (idx1 != prevSubIndex) {
			prevSubIndex = idx1;
			subIndex = idx1;

			elem = new MovInfo1Sub;
			elem->subIndex = subIndex;
			elem->x = rect.left;
			elem->y = rect.top;
			elem->distance = -1;

			movinfo->items.push_back(elem);
		}

		if (i != linkList->size() - 1) {
			while (1) {
				i++;
				if (findLink(linkList, i, &rect, 0) != prevSubIndex) {
					i--;
					findLink(linkList, i, &rect, &point);

					break;
				}

				if (i == linkList->size() - 1)
					break;
			}
		}

		if (movinfo->items.back()->subIndex != 10) {
			subIndex = prevSubIndex;

			elem = new MovInfo1Sub;
			elem->subIndex = 10;
			elem->x = -1;
			elem->y = -1;
			elem->distance = -1;

			movinfo->items.push_back(elem);

			if (i == linkList->size()) {
				elem = new MovInfo1Sub;
				elem->subIndex = prevSubIndex;
				elem->x = movinfo->pt2.x;
				elem->y = movinfo->pt2.y;
				elem->distance = movinfo->distance2;

				movinfo->items.push_back(elem);
			} else {
				elem = new MovInfo1Sub;
				elem->subIndex = prevSubIndex;
				elem->x = rect.right;
				elem->y = rect.bottom;
				elem->distance = point.y;

				movinfo->items.push_back(elem);
			}
		}
	}

	if (subIndex != movinfo->item1Index) {
		elem = new MovInfo1Sub;
		elem->subIndex = movinfo->item1Index;
		elem->x = movinfo->pt2.x;
		elem->y = movinfo->pt2.y;
		elem->distance = movinfo->distance2;

		movinfo->items.push_back(elem);
	}

	movinfo->itemsCount = movinfo->items.size();
}

MessageQueue *MovGraph2::buildMovInfo1MessageQueue(MovInfo1 *movInfo) {
	MovInfo1 movinfo;

	memcpy(&movinfo, movInfo, sizeof(movinfo));

	int curX = movInfo->pt1.x;
	int curY = movInfo->pt1.y;
	int curDistance = movInfo->distance1;

	MessageQueue *mq = new MessageQueue(g_fullpipe->_globalMessageQueueList->compact());

	for (int i = 0; i < movInfo->itemsCount - 1; i++) {
		if (movInfo->items[i + 1]->subIndex != 10) {
			MG2I *mg2i;

			if (i >= movInfo->itemsCount - 2 || movInfo->items[i + 2]->subIndex != 10) {
				movinfo.flags = 0;
				mg2i = &_items2[movInfo->field_0]->_subItems[movInfo->items[i]->subIndex]._turnS[movInfo->items[i + 1]->subIndex];
			} else {
				movinfo.flags = 2;
				mg2i = &_items2[movInfo->field_0]->_subItems[movInfo->items[i]->subIndex]._turn[movInfo->items[i + 1]->subIndex];
			}
			if (i < movInfo->itemsCount - 2
				|| (movInfo->items[i]->x == movInfo->items[i + 1]->x
					&& movInfo->items[i]->y == movInfo->items[i + 1]->y)
				 || movInfo->items[i]->x == -1
				 || movInfo->items[i]->y == -1
				 || movInfo->items[i + 1]->x == -1
				 || movInfo->items[i + 1]->y == -1) {

				ExCommand *ex = new ExCommand(_items2[movInfo->field_0]->_objectId, 1, mg2i->_movementId, 0, 0, 0, 1, 0, 0, 0);

				ex->_excFlags |= 2;
				ex->_keyCode = _items2[movInfo->field_0]->_obj->_okeyCode;
				ex->_field_24 = 1;
				ex->_field_14 = -1;
				mq->addExCommandToEnd(ex);

				curX += mg2i->_mx;
				curY += mg2i->_my;
			} else {
				MGMInfo mgminfo;

				memset(&mgminfo, 0, sizeof(mgminfo));

				mgminfo.ani = _items2[movInfo->field_0]->_obj;
				mgminfo.staticsId2 = mg2i->_mov->_staticsObj2->_staticsId;
				mgminfo.x1 = movInfo->items[i + 1]->x;
				mgminfo.y1 = movInfo->items[i + 1]->y;
				mgminfo.field_1C = movInfo->items[i + 1]->distance;
				mgminfo.staticsId1 = mg2i->_mov->_staticsObj1->_staticsId;

				mgminfo.x2 = movInfo->items[i]->x;
				mgminfo.y2 = movInfo->items[i]->y;
				mgminfo.field_10 = 1;
				mgminfo.flags = 0x7f;
				mgminfo.movementId = mg2i->_movementId;

				MessageQueue *mq2 = _mgm.genMovement(&mgminfo);
				mq->transferExCommands(mq2);

				delete mq2;

				curX = movInfo->items[i + 1]->x;
				curY = movInfo->items[i + 1]->y;
			}
		} else {
			movinfo.item1Index = movInfo->items[i]->subIndex;
			movinfo.subIndex = movinfo.item1Index;
			movinfo.pt1.y = curY;
			movinfo.pt1.x = curX;

			movinfo.distance1 = curDistance;
			movinfo.pt2.x = movInfo->items[i + 2]->x;
			movinfo.pt2.y = movInfo->items[i + 2]->y;
			movinfo.distance2 = movInfo->items[i + 2]->distance;

			if (i >= movInfo->itemsCount - 4
				 || movInfo->items[i + 2]->subIndex == 10
				 || movInfo->items[i + 3]->subIndex == 10
				 || movInfo->items[i + 2]->subIndex == movInfo->items[i + 3]->subIndex
				 || movInfo->items[i + 4]->subIndex != 10) {
				if (i >= movInfo->itemsCount - 3
					 || movInfo->items[i + 2]->subIndex == 10
					 || movInfo->items[i + 3]->subIndex == 10
					 || movInfo->items[i + 2]->subIndex == movInfo->items[i + 3]->subIndex) {
					movinfo.flags &= 3;
				} else {
					MG2I *m = &_items2[movInfo->field_0]->_subItems[movInfo->items[i + 2]->subIndex]._turnS[movInfo->items[i + 3]->subIndex];
					movinfo.pt2.x -= m->_mx;
					movinfo.pt2.y -= m->_my;
					movinfo.flags &= 3;
				}
			} else {
				MG2I *m = &_items2[movInfo->field_0]->_subItems[movInfo->items[i + 2]->subIndex]._turn[movInfo->items[i + 3]->subIndex];

				if (movinfo.item1Index && movinfo.item1Index != 1) {
					movinfo.pt2.y -= m->_my;
					movinfo.flags = (movinfo.flags & 2) | 1;
				} else {
					movinfo.pt2.x -= m->_mx;
					movinfo.flags = (movinfo.flags & 2) | 1;
				}
			}
			i++; // intentional

			MessageQueue *mq2 = genMovement(&movinfo);

			if (!mq2) {
				delete mq;
				return 0;
			}

			mq->transferExCommands(mq2);

			delete mq2;

			curX = movinfo.pt2.x;
			curY = movinfo.pt2.y;
			curDistance = movinfo.distance2;
		}
	}

	movInfo->pt2.x = movinfo.pt2.x;
	movInfo->pt2.y = movinfo.pt2.y;

	return mq;
}

int MovGraph2::removeObject(StaticANIObject *obj) {
	warning("STUB: MovGraph2::removeObject()");

	return 0;
}

void MovGraph2::freeItems() {
	warning("STUB: MovGraph2::freeItems()");
}

MessageQueue *MovGraph2::method34(StaticANIObject *ani, int xpos, int ypos, int fuzzyMatch, int staticsId) {
	if (!ani->isIdle())
		return 0;

	if (ani->_flags & 0x100)
		return 0;

	MessageQueue *mq = doWalkTo(ani, xpos, ypos, fuzzyMatch, staticsId);

	if (!mq)
		return 0;

	if (ani->_movement) {
		if (mq->getCount() <= 1 || mq->getExCommandByIndex(0)->_messageKind != 22) {
			PicAniInfo picAniInfo;

			ani->getPicAniInfo(&picAniInfo);
			ani->updateStepPos();
			MessageQueue *mq1 = doWalkTo(ani, xpos, ypos, fuzzyMatch, staticsId);

			ani->setPicAniInfo(&picAniInfo);

			if (mq1) {
				delete mq;

				mq = mq1;
			}
		} else {
			ani->_movement = 0;
		}
	}

	if (!mq->chain(ani)) {
		delete mq;

		return 0;
	}

	return mq;
}

MessageQueue *MovGraph2::doWalkTo(StaticANIObject *obj, int xpos, int ypos, int fuzzyMatch, int staticsId) {
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
			obj->changeStatics2(_items2[idx]->_subItems[idxsub]._staticsId1);
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

				delete mq;

				return 0;
			}

			ExCommand *ex = new ExCommand(picAniInfo.objectId, 1, _items2[idx]->_subItems[idxsub]._walk[idxwalk]._movementId, 0, 0, 0, 1, 0, 0, 0);

			ex->_field_24 = 1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 2;

			mq->addExCommandToEnd(ex);
		} else {
			ExCommand *ex = new ExCommand(picAniInfo.objectId, 22, obj->_statics->_staticsId, 0, 0, 0, 1, 0, 0, 0);

			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);

			ex = new ExCommand(picAniInfo.objectId, 5, -1, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);

			ex->_field_14 = -1;
			ex->_keyCode = picAniInfo.field_8;
			ex->_excFlags |= 3;
			mq->addExCommandToEnd(ex);
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
	double minPath = findMinPath(&linkInfoSource, &linkInfoDest, &tempLinkList);

	debug(0, "MovGraph2::doWalkTo(): path: %g  parts: %d", minPath, tempLinkList.size());

	if (minPath < 0.0 || ((linkInfoSource.node != linkInfoDest.node || !linkInfoSource.node) && !tempLinkList.size()))
		return 0;

	memset(&movInfo1, 0, sizeof(movInfo1));

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
		double dst1 = sqrt((double)((ypos - nod->_y) * (ypos - nod->_y) + (xpos - nod->_x) * (xpos - nod->_x)));
		int dst = linkInfoDest.link->_movGraphNode2->_distance - nod->_distance;

		movInfo1.distance2 = nod->_distance + (dst1 * (double)dst / linkInfoDest.link->_distance);

		calcDistance(&movInfo1.pt2, linkInfoDest.link, 1);

		dx1 = movInfo1.pt1.x;
		dy1 = movInfo1.pt1.y;
		dx2 = movInfo1.pt2.x;
		dy2 = movInfo1.pt2.y;
	}

	if (staticsId) {
		movInfo1.item1Index = getItemSubIndexByStaticsId(idx, staticsId);
	} else if (tempLinkList.size() <= 1) {
		if (tempLinkList.size() == 1)
			movInfo1.item1Index = getShortSide(tempLinkList[0], dx2 - dx1, dy2 - dy1);
		else
			movInfo1.item1Index = getShortSide(0, dx2 - dx1, dy2 - dy1);
	} else {
		movInfo1.item1Index = findLink(&tempLinkList, tempLinkList.size() - 1, 0, 0);
	}

	movInfo1.flags = fuzzyMatch != 0;

	if (_items2[idx]->_subItems[idxsub]._staticsId1 != obj->_statics->_staticsId)
		movInfo1.flags |= 2;

	// FIXME: This somehow corrupts the heap (reported by MSVC)
	buildMovInfo1SubItems(&movInfo1, &tempLinkList, &linkInfoSource, &linkInfoDest);

	MessageQueue *mq = buildMovInfo1MessageQueue(&movInfo1);

	linkInfoDest.node = findNode(movInfo1.pt2.x, movInfo1.pt2.y, fuzzyMatch);

	if (!linkInfoDest.node)
		linkInfoDest.link = findLink1(movInfo1.pt2.x, movInfo1.pt2.y, movInfo1.item1Index, fuzzyMatch);

	if (fuzzyMatch || linkInfoDest.link || linkInfoDest.node) {
		if (mq && mq->getCount() > 0 && picAniInfo.movementId) {
			ExCommand *ex = mq->getExCommandByIndex(0);

			if (ex && (ex->_messageKind == 1 || ex->_messageKind == 20)
					&& picAniInfo.movementId == ex->_messageNum
					&& picAniInfo.someDynamicPhaseIndex == ex->_field_14) {
				mq->deleteExCommandByIndex(0, 1);
			} else {
				ex = new ExCommand(picAniInfo.objectId, 5, ex->_messageNum, obj->_ox, obj->_oy, 0, 1, 0, 0, 0);
				ex->_field_14 = -1;
				ex->_keyCode = picAniInfo.field_8;
				ex->_excFlags |= 2;
				mq->addExCommand(ex);

				ex = new ExCommand(picAniInfo.objectId, 22, _items2[idx]->_subItems[idxsub]._staticsId1, 0, 0, 0, 1, 0, 0, 0);

				ex->_keyCode = picAniInfo.field_8;
				ex->_excFlags |= 3;
				mq->addExCommand(ex);
			}
		}
	} else {
		if (mq)
			delete mq;
		mq = 0;
	}

	obj->setPicAniInfo(&picAniInfo);

	return mq;
}

MovGraphNode *MovGraph2::findNode(int x, int y, int fuzzyMatch) {
	for (ObList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = (MovGraphNode *)*i;

		if (fuzzyMatch) {
			if (abs(node->_x - x) < 15 && abs(node->_y - y) < 15)
				return node;
		} else {
			if (node->_x == x && node->_y == y)
				return node;
		}
	}

	return 0;
}

int MovGraph2::getShortSide(MovGraphLink *lnk, int x, int y) {
	bool cond;

	if (lnk)
		cond = abs(lnk->_movGraphNode2->_x - lnk->_movGraphNode1->_x) > abs(lnk->_movGraphNode2->_y - lnk->_movGraphNode1->_y);
	else
		cond = abs(x) > abs(y);

	if (cond)
		return x <= 0;
	else
		return ((y > 0) + 2);
}

int MovGraph2::findLink(Common::Array<MovGraphLink *> *linkList, int idx, Common::Rect *rect, Common::Point *point) {
	MovGraphNode *node1 = (*linkList)[idx]->_movGraphNode1;
	MovGraphNode *node2 = (*linkList)[idx]->_movGraphNode2;
	MovGraphNode *node3 = node1;

	if (idx != 0) {
		MovGraphLink *lnk = (*linkList)[idx - 1];

		if (lnk->_movGraphNode2 != node1) {
			if (lnk->_movGraphNode1 != node1) {
				if (lnk->_movGraphNode2 == node2 || lnk->_movGraphNode1 == node2) {
					node3 = node2;
					node2 = node1;
				}
				goto LABEL_7;
			}
		}
		node3 = node1;
	} else if (idx != (int)(linkList->size() - 1)) {
		MovGraphLink *lnk = (*linkList)[idx + 1];

		if (lnk->_movGraphNode2 == node1 || lnk->_movGraphNode1 == node1) {
			node3 = node2;
			node2 = node1;
		} else if (lnk->_movGraphNode2 == node2 || lnk->_movGraphNode1 == node2) {
			node3 = node1;
		}
	}

 LABEL_7:
	if (rect) {
		rect->left = node3->_x;
		rect->top = node3->_y;
		rect->right = node2->_x;
		rect->bottom = node2->_y;
	}
	if (point) {
		point->x = node3->_distance;
		point->y = node2->_distance;
	}

	if (abs(node3->_x - node2->_x) <= abs(node3->_y - node2->_y))
		return (node3->_y < node2->_x) + 2;
	else
		return node3->_x >= node2->_x;
}

MessageQueue *MovGraph2::genMovement(MovInfo1 *movinfo) {
	warning("STUB: MovGraph2::genMovement()");

	return 0;
}

MovGraphLink *MovGraph2::findLink1(int x, int y, int idx, int fuzzyMatch) {
	Common::Point point;
	MovGraphLink *res = 0;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		if (fuzzyMatch) {
			point.x = x;
			point.y = y;
			double dst = calcDistance(&point, lnk, 0);

			if (dst >= 0.0 && dst < 2.0)
				return lnk;
		} else if (!(lnk->_flags & 0x20000000)) {
			if (lnk->_movGraphReact->pointInRegion(x, y)) {
				if (abs(lnk->_movGraphNode1->_x - lnk->_movGraphNode2->_x) <= abs(lnk->_movGraphNode1->_y - lnk->_movGraphNode2->_y)) {
					if (idx == 2 || idx == 3)
						return lnk;
					res = lnk;
				} else {
					if (idx == 1 || !idx)
						return lnk;
					res = lnk;
				}
			}
		}
	}

	return res;
}

MovGraphLink *MovGraph2::findLink2(int x, int y) {
	double mindist = 1.0e20;
	MovGraphLink *res = 0;

	for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphLink);

		MovGraphLink *lnk = (MovGraphLink *)*i;

		if (!(lnk->_flags & 0x20000000)) {
			double n1x = lnk->_movGraphNode1->_x;
			double n1y = lnk->_movGraphNode1->_y;
			double n2x = lnk->_movGraphNode2->_x;
			double n2y = lnk->_movGraphNode2->_y;
			double n1dx = n1x - x;
			double n1dy = n1y - y;
			double dst1 = sqrt(n1dy * n1dy + n1dx * n1dx);
			double coeff1 = ((n1y - n2y) * n1dy + (n2x - n1x) * n1dx) / lnk->_distance / dst1;
			double dst3 = coeff1 * dst1;
			double dst2 = sqrt(1.0 - coeff1 * coeff1) * dst1;

			if (coeff1 * dst1 < 0.0) {
				dst3 = 0.0;
				dst2 = sqrt(n1dy * n1dy + n1dx * n1dx);
			}
			if (dst3 > lnk->_distance) {
				dst3 = lnk->_distance;
				dst2 = sqrt((n2x - x) * (n2x - x) + (n2y - y) * (n2y - y));
			}
			if (dst3 >= 0.0 && dst3 <= lnk->_distance && dst2 < mindist) {
				mindist = dst2;
				res = lnk;
			}
		}
	}

	if (mindist < 1.0e20)
		return res;
	else
		return 0;
}

double MovGraph2::findMinPath(LinkInfo *linkInfoSource, LinkInfo *linkInfoDest, Common::Array<MovGraphLink *> *listObj) {
	LinkInfo linkInfoWorkSource;

	if (linkInfoSource->link != linkInfoDest->link || linkInfoSource->node != linkInfoDest->node) {
		double minDistance = -1.0;

		if (linkInfoSource->node) {
			for (ObList::iterator i = _links.begin(); i != _links.end(); ++i) {
				MovGraphLink *lnk = (MovGraphLink *)*i;

				if ((lnk->_movGraphNode1 == linkInfoSource->node || lnk->_movGraphNode2 == linkInfoSource->node) && !(lnk->_flags & 0xA0000000)) {
					linkInfoWorkSource.node = 0;
					linkInfoWorkSource.link = lnk;

					Common::Array<MovGraphLink *> tmpList;

					lnk->_flags |= 0x80000000;

					double newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

					if (newDistance >= 0.0 && (minDistance < 0.0 || newDistance + lnk->_distance < minDistance)) {
						listObj->clear();
						listObj->push_back(tmpList);

						minDistance = newDistance + lnk->_distance;
					}

					lnk->_flags &= 0x7FFFFFFF;
				}
			}
		} else if (linkInfoSource->link) {
			linkInfoWorkSource.node = linkInfoSource->link->_movGraphNode1;
			linkInfoWorkSource.link = 0;
			
			Common::Array<MovGraphLink *> tmpList;

			double newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0.0) {
				listObj->clear();

				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}

			linkInfoWorkSource.link = 0;
			linkInfoWorkSource.node = linkInfoSource->link->_movGraphNode2;
			
			tmpList.clear();
			
			newDistance = findMinPath(&linkInfoWorkSource, linkInfoDest, &tmpList);

			if (newDistance >= 0 && (minDistance < 0.0 || newDistance < minDistance)) {
				listObj->push_back(linkInfoSource->link);
				listObj->push_back(tmpList);

				minDistance = newDistance;
			}
		}

		return minDistance;
	} else {
		if (linkInfoSource->link)
			listObj->push_back(linkInfoSource->link);

		return 0.0;
	}
}

MovGraphNode *MovGraph::calcOffset(int ox, int oy) {
	MovGraphNode *res = 0;
	double mindist = 1.0e10;

	for (ObList::iterator i = _nodes.begin(); i != _nodes.end(); ++i) {
		assert(((CObject *)*i)->_objtype == kObjTypeMovGraphNode);

		MovGraphNode *node = (MovGraphNode *)*i;

		double dist = sqrt((double)((node->_x - oy) * (node->_x - oy) + (node->_x - ox) * (node->_x - ox)));
		if (dist < mindist) {
			mindist = dist;
			res = node;
		}
	}

	return res;
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

MessageQueue *MGM::genMovement(MGMInfo *mgminfo) {
	warning("STUB: MGM::genMovement()");

	return 0;
}

void MGM::updateAnimStatics(StaticANIObject *ani, int staticsId) {
	if (getItemIndexById(ani->_id) == -1)
		return;

	if (ani->_movement) {
		ani->queueMessageQueue(0);
		ani->_movement->gotoLastFrame();
		ani->_statics = ani->_movement->_staticsObj2;
		ani->_movement = 0;

		ani->setOXY(ani->_movement->_ox, ani->_movement->_oy);
	}

	if (ani->_statics) {
		Common::Point point;

		getPoint(&point, ani->_id, ani->_statics->_staticsId, staticsId);

		ani->setOXY(ani->_ox + point.x, ani->_oy + point.y);

		ani->_statics = ani->getStaticsById(staticsId);
	}
}

Common::Point *MGM::getPoint(Common::Point *point, int objectId, int staticsId1, int staticsId2) {
	int idx = getItemIndexById(objectId);

	if (idx == -1) {
		point->x = -1;
		point->y = -1;
	} else {
		int st1idx = getStaticsIndexById(idx, staticsId1);
		int st2idx = getStaticsIndexById(idx, staticsId2);

		if (st1idx == st2idx) {
			point->x = 0;
			point->y = 0;
		} else {
			int subidx = st1idx + st2idx * _items[idx]->statics.size();

			if (!_items[idx]->subItems[subidx]->movement) {
				clearMovements2(idx);
				recalcOffsets(idx, st1idx, st2idx, false, true);

				if (!_items[idx]->subItems[subidx]->movement) {
					clearMovements2(idx);
					recalcOffsets(idx, st1idx, st2idx, true, false);
				}
			}

			MGMSubItem *sub = _items[idx]->subItems[subidx];

			if (sub->movement) {
				point->x = sub->x;
				point->y = sub->y;
			} else {
				point->x = 0;
				point->y = 0;
			}
		}
	}

	return point;
}

int MGM::getStaticsIndexById(int idx, int16 id) {
	for (uint i = 0; i < _items[idx]->statics.size(); i++) {
		if (_items[idx]->statics[i]->_staticsId == id)
			return i;
	}

	return 0;
}

void MGM::clearMovements2(int idx) {
	_items[idx]->movements2.clear();
}

int MGM::recalcOffsets(int idx, int st1idx, int st2idx, bool flip, bool flop) {
	warning("STUB: MGM::recalcOffsets()");

	return 0;
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

	_objtype = kObjTypeMovGraphLink;
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

void MovGraphLink::calcNodeDistanceAndAngle() {
	if (_movGraphNode1) {
		double dx = _movGraphNode2->_x - _movGraphNode1->_x;
		double dy = _movGraphNode2->_y - _movGraphNode1->_y;

		_distance = sqrt(dy * dy + dx * dx);
		_angle = atan2(dx, dy);
	}
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
