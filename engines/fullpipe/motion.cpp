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
#include "fullpipe/motion.h"

namespace Fullpipe {

bool CMotionController::load(MfcArchive &file) {
	// Is originally empty	file.readClass();

	debug(5, "CMotionController::load()");

	return true;
}

bool CMctlCompound::load(MfcArchive &file) {
	debug(5, "CMctlCompound::load()");

	int count = file.readUint32LE();

	debug(6, "CMctlCompound::count = %d", count);

	for (int i = 0; i < count; i++) {
		debug(6, "CompoundArray[%d]", i);
		CMctlCompoundArrayItem *obj = (CMctlCompoundArrayItem *)file.readClass();

		int count1 = file.readUint32LE();

		debug(6, "ConnectionPoint::count: %d", count1);
		for (int j = 0; j < count1; j++) {
			debug(6, "ConnectionPoint[%d]", j);
			CMctlConnectionPoint *obj1 = (CMctlConnectionPoint *)file.readClass();

			obj->_connectionPoints.push_back(*obj1);
		}

		obj->_field_20 = file.readUint32LE();
		obj->_field_24 = file.readUint32LE();

		debug(6, "graphReact");
		obj->_movGraphReactObj = (CMovGraphReact *)file.readClass();

		_motionControllers.push_back(*obj);
	}

	return true;
}

void CMctlCompound::addObject(StaticANIObject *obj) {
	warning("STUB: CMctlCompound::addObject()");
}

void CMctlCompound::initMovGraph2() {
	warning("STUB: CMctlCompound::initMovGraph2()");
}

MessageQueue *CMctlCompound::method34(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: CMctlCompound::method34()");

	return 0;
}

MessageQueue *CMctlCompound::method4C(StaticANIObject *subj, int xpos, int ypos, int flag, int staticsId) {
	warning("STUB: CMctlCompound::method4C()");

	return 0;
}

bool CMctlCompoundArray::load(MfcArchive &file) {
	debug(5, "CMctlCompoundArray::load()");

	int count = file.readUint32LE();

	debug(0, "CMctlCompoundArray::count = %d", count);

	assert(0);

	return true;
}

CMovGraph::CMovGraph() {
	warning("STUB: CMovGraph::CMovGraph()");
	_itemsCount = 0;
	_items = 0;
	//_callback1 = CMovGraphCallback1;  // TODO
	_field_44 = 0;
	// insertMessageHandler(CMovGraph_messageHandler, getMessageHandlersCount() - 1, 129);
}

bool CMovGraph::load(MfcArchive &file) {
	debug(5, "CMovGraph::load()");

	_links.load(file);
	_nodes.load(file);

	return true;
}

void CMovGraph::addObject(StaticANIObject *obj) {
	warning("STUB: CMovGraph::addObject()");
}

CMovGraphLink::CMovGraphLink() {
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

bool CMovGraphLink::load(MfcArchive &file) {
	debug(5, "CMovGraphLink::load()");

	_dwordArray1.load(file);
	_dwordArray2.load(file);

	_flags = file.readUint32LE();

	debug(8, "GraphNode1");
	_movGraphNode1 = (CMovGraphNode *)file.readClass();
	debug(8, "GraphNode2");
	_movGraphNode2 = (CMovGraphNode *)file.readClass();

	_distance = file.readDouble();
	_angle = file.readDouble();

	debug(8, "distance: %g, angle: %g", _distance, _angle);

	_movGraphReact = (CMovGraphReact *)file.readClass();
	_name = file.readPascalString();

	return true;
}

bool CMovGraphNode::load(MfcArchive &file) {
	debug(5, "CMovGraphNode::load()");

	_field_14 = file.readUint32LE();
	_x = file.readUint32LE();
	_y = file.readUint32LE();
	_distance = file.readUint32LE();

	return true;
}

CReactParallel::CReactParallel() {
	_x1 = 0;
	_x2 = 0;
	_dy = 0;
	_dx = 0;
	_points = 0;
	_y1 = 0;
	_y2 = 0;
}

bool CReactParallel::load(MfcArchive &file) {
	debug(5, "CReactParallel::load()");

	_x1 = file.readUint32LE();
	_y1 = file.readUint32LE();
	_x2 = file.readUint32LE();
	_y2 = file.readUint32LE();
	_dx = file.readUint32LE();
	_dy = file.readUint32LE();

	createRegion();

	return true;
}

void CReactParallel::createRegion() {
	_points = (Common::Point **)malloc(sizeof(Common::Point *) * 4);

	for (int i = 0; i < 4; i++)
		_points[i] = new Common::Point;

	double at = atan2(_x1 - _x2, _y1 - _y2) + 1.570796;
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

CReactPolygonal::CReactPolygonal() {
	_field_C = 0;
	_points = 0;
	_pointCount = 0;
	_field_10 = 0;
}

bool CReactPolygonal::load(MfcArchive &file) {
	debug(5, "CReactPolygonal::load()");

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

void CReactPolygonal::createRegion() {
	if (_points) {

		// GdiObject::Attach(_rgn, CreatePolygonRgn(_points, _pointCount, 2);
	}
}

int startWalkTo(int objId, int objKey, int x, int y, int a5) {
	warning("STUB: startWalkTo(%d, %d, %d, %d, %d)", objId, objKey, x, y, a5);

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
