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

			obj->_connectionPoints.push_back(*obj1);
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

MessageQueue *MctlCompound::method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: MctlCompound::method34()");

	return 0;
}

MessageQueue *MctlCompound::method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: MctlCompound::method4C()");

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

MessageQueue *MovGraph::method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
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

MessageQueue *MovGraph::method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: MovGraph::method4C()");

	return 0;
}

int MovGraph::method50() {
	warning("STUB: MovGraph::method50()");

	return 0;
}

double MovGraph::calcDistance(Common::Point *point, MovGraphLink *link, int flag) {
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
		if (flag) {
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
			int idx;

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
			int idx;

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
			int idx;

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

MessageQueue *MovGraph2::method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: MovGraph2::method34()");

	return 0;
}

MessageQueue *MovGraph2::method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: MovGraph2::method4C()");

	return 0;
}

MovGraphNode *MovGraph::calcOffset(int ox, int oy) {
	warning("STUB: MovGraph::calcOffset()");

	return 0;
}

void MGM::clear() {
	warning("STUB: MGM:clear()");
}

void MGM::addItem(int objId) {
	warning("STUB: MGM:addItem(%d)", objId);
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
	_points = 0;
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

	_points[2]->x = (int16)(_x1 + _dy * cs);
	_points[2]->y = (int16)(_y2 + _dy * sn);

	_points[3]->x = (int16)(_x1 + _dy * cs);
	_points[3]->y = (int16)(_y1 + _dy * sn);

	// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, 4, 2);
}

ReactPolygonal::ReactPolygonal() {
	_field_C = 0;
	_points = 0;
	_pointCount = 0;
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
