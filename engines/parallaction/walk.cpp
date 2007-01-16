/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "parallaction/defs.h"
#include "parallaction/parallaction.h"
#include "parallaction/commands.h"
#include "parallaction/graphics.h"
#include "parallaction/zone.h"

namespace Parallaction {

uint16 walkFunc1(int16, int16, WalkNode *);
uint16 queryPath(uint16 x, uint16 y);

WalkNode _NULL_WALKNODE = { {NULL, NULL}, 0, 0 };

static uint16 _doorData1 = 1000;
static Zone *_zoneTrap = NULL;

static uint16	walkData1 = 0;
static uint16	walkData2 = 0; 	// next walk frame
static int16	walkData3 = -1000; 	// unused


int32 dotProduct(Point *p1, Point *p2) {
	return p1->_x * p2->_x + p1->_y * p2->_y;
}

//
//	x, y: mouse click (foot) coordinates
//
WalkNode *buildWalkPath(uint16 x, uint16 y) {
    debugC(1, kDebugWalk, "buildWalkPath to (%i, %i)", x, y);

	int16 to_x = x;
	int16 to_y = y;

	int16 left, bottom, right, top, close, closeY, closeX;

	// looks for closest usable path Point
	if (queryPath(to_x, to_y) == 0) {

		right = left = to_x;

		do {
			right++;
		} while ((queryPath(right, to_y) == 0) && (right < SCREEN_WIDTH));

		do {
			left--;
		} while ((queryPath(left, to_y) == 0) && (left > 0));

		right = (right == SCREEN_WIDTH) ? 1000 : right - to_x;
		left = (left == 0) ? 1000 : to_x - left;

		top = bottom = to_y;

		do {
			top--;
		} while ((queryPath(to_x, top) == 0) && (top > 0));

		do {
			bottom++;
		} while ((queryPath(to_x, bottom) == 0) && (bottom < SCREEN_HEIGHT));

		top = (top == 0) ? 1000 : to_y - top;
		bottom = (bottom == SCREEN_HEIGHT) ? 1000 : bottom - to_y;

		closeX = (right >= left) ? left : right;
		closeY = (top >= bottom) ? bottom : top;

		close = (closeX >= closeY) ? closeY : closeX;

		if (close == right) {
			to_x += right;
			walkData3 = (_yourself._cnv._count == 20) ? 7 : 9;
		} else
		if (close == left) {
			to_x -= left;
			walkData3 = 0;
		} else
		if (close == top) {
			to_y -= top;
		} else
		if (close == bottom) {
			to_y += bottom;
			walkData3 = (_yourself._cnv._count == 20) ? 17 : 21;
		}

	}
    debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to_x, to_y);

	WalkNode *v48 = (WalkNode*)memAlloc(sizeof(WalkNode));
	WalkNode *v44 = (WalkNode*)memAlloc(sizeof(WalkNode));

	v48->_x = to_x - _yourself._cnv._width / 2; 		// target top left coordinates
	v48->_y = to_y - _yourself._cnv._height;
	v48->_node._next = NULL;
	memcpy(v44, v48, sizeof(WalkNode));

	uint16 v38 = walkFunc1(to_x, to_y, v44);
	if (v38 == 1) {
		// destination directly reachable
        debugC(1, kDebugWalk, "direct move to (%i, %i)", to_x, to_y);
		memFree(v44);
		return v48;
	}

	// path is obstructed: find alternative
    debugC(1, kDebugWalk, "trying to build walk path to (%i, %i)", to_x, to_y);

	WalkNode	v58;
	memset(&v58, 0, sizeof(WalkNode));

	int16 _si = v48->_x;						// _si, _di: target top left coordinates
	int16 _di = v48->_y;
	addNode(&v58._node, &v48->_node);

	WalkNode *_closest_node = NULL;

	Point v20;
	Point v8;

	int32 v30, v34, v2C, v28;

	byte _closest_node_found = 1;
	bool emptyList = true;

	do {

		v48 = &v58;

		v20._x = _yourself._zone.pos._position._x;
		v20._y = _yourself._zone.pos._position._y;

		v8._x = _si - _yourself._zone.pos._position._x;
		v8._y = _di - _yourself._zone.pos._position._y;
		v34 = v30 = dotProduct(&v8, &v8);				// square distance from current position and target

		while (_closest_node_found != 0) {

			_closest_node_found = 0;
			WalkNode *location_node = (WalkNode*)_locationWalkNodes._next;

			// scans location path nodes searching for the nearest Node
			// which can't be farther than the target position
			// otherwise no _closest_node is selected
			while (location_node != NULL) {
				v8._x = location_node->_x - _si;
				v8._y = location_node->_y - _di;
				v2C = dotProduct(&v8, &v8); 			// square distance from Node to target position

				v8._x = location_node->_x - v20._x;
				v8._y = location_node->_y - v20._y;
				v28 = dotProduct(&v8, &v8); 			// square distance from Node to current position

				if (v2C < v34 && v28 < v30) {
					_closest_node_found = 1;
					v30 = v28;
					_closest_node = location_node;
				}

				location_node = (WalkNode*)location_node->_node._next;
			}

			if (_closest_node_found == 0) break;

			WalkNode *_newnode = (WalkNode*)memAlloc(sizeof(WalkNode));
			memcpy( _newnode, _closest_node, sizeof(WalkNode));
			v20._x = _newnode->_x;
			v20._y = _newnode->_y;

			v34 = v30 = (_si - v20._x) * (_si - v20._x) + (_di - v20._y) * (_di - v20._y);


            debugC(1, kDebugWalk, "adding walk node (%i, %i) to path", _newnode->_x, _newnode->_y);

			addNode(&v48->_node, &_newnode->_node);
			v48 = _newnode;
		}

		if (!emptyList) break;

		if (v38 != 0 && v34 > v38) {
			// no alternative path (gap?)
			freeNodeList(v58._node._next);
            debugC(1, kDebugWalk, "can't find a path node: rejecting partial path");
			return v44;
		} else {
			_si = ((WalkNode*)(v58._node._next))->_x;
			_di = ((WalkNode*)(v58._node._next))->_y;
			emptyList = false;
			_closest_node_found = 1;
		}

	} while (true);

    debugC(1, kDebugWalk, "walk path completed");

	memFree(v44);
	return (WalkNode*)v58._node._next;
}


uint16 queryPath(uint16 x, uint16 y) {
	return _vm->_graphics->queryPath(x, y);
}


//
//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable
//	other values: square distance to target (not reachable)
//
uint16 walkFunc1(int16 x, int16 y, WalkNode *Node) {

	Point v4 = { 0, 0 };

	Point foot = {
		_yourself._zone.pos._position._x + _yourself._cnv._width/2,
		_yourself._zone.pos._position._y + _yourself._cnv._height
	};

	Point v8 = {
		foot._x,
		foot._y
	};

	while (foot._x != x || foot._y != y) {

		if (foot._x < x) {
			if (queryPath(foot._x + 1, foot._y) != 0) foot._x++;
		}
		if (foot._x > x) {
			if (queryPath(foot._x - 1, foot._y) != 0) foot._x--;
		}
		if (foot._y < y) {
			if (queryPath(foot._x, foot._y + 1) != 0) foot._y++;
		}
		if (foot._y > y) {
			if (queryPath(foot._x, foot._y - 1) != 0) foot._y--;
		}

		if ((foot._x == v8._x) && (foot._y == v8._y) && ((foot._x != x) || (foot._y != y))) {
			// foot couldn't move and still away from target

			v4._x = foot._x;
			v4._y = foot._y;

			while (foot._x != x || foot._y != y) {

				if (foot._x < x) {
					if (queryPath(foot._x + 1, foot._y) == 0) foot._x++;
				}
				if (foot._x > x) {
					if (queryPath(foot._x - 1, foot._y) == 0) foot._x--;
				}
				if (foot._y < y) {
					if (queryPath(foot._x, foot._y + 1) == 0) foot._y++;
				}
				if (foot._y > y) {
					if (queryPath(foot._x, foot._y - 1) == 0) foot._y--;
				}

				if ((foot._x == v8._x) && (foot._y == v8._y) && (foot._x != x || foot._y != y))
					return 0;

				v8._x = foot._x;
				v8._y = foot._y;
			}

			Node->_x = v4._x - _yourself._cnv._width / 2;
			Node->_y = v4._y - _yourself._cnv._height;

			return (x - v4._x) * (x - v4._x) + (y - v4._y) * (y - v4._y);
		}

		v8._x = foot._x;
		v8._y = foot._y;

	}

	// there exists an unobstructed path
	return 1;
}


void jobWalk(void *parm, Job *j) {
	WalkNode *node = (WalkNode*)parm;

	int16 _si = _yourself._zone.pos._position._x;
	int16 _di = _yourself._zone.pos._position._y;

    debugC(1, kDebugWalk, "jobWalk to (%i, %i)", node->_x + _yourself._cnv._width / 2, node->_y + _yourself._cnv._height);

	_yourself._zone.pos._oldposition._x = _si;
	_yourself._zone.pos._oldposition._y = _di;

	if ((node->_x == 0) && (node->_y == 0)) {
		if (node->_node._next == NULL) {
			j->_finished = 1;
			checkDoor();
			memFree(node);
			return;
		}

		j->_parm = node->_node._next;
		memFree(node);
		node = (WalkNode*)node->_node._next;
	}

	Point dist = {
		node->_x - _yourself._zone.pos._position._x,
		node->_y - _yourself._zone.pos._position._y
	};

	if (dist._x < 0) dist._x = -dist._x;
	if (dist._y < 0) dist._y = -dist._y;

	walkData1++;

	// walk frame selection
	int16 v16;
	if (_yourself._cnv._count == 20) {

		if (dist._x > dist._y) {
			walkData2 = (node->_x > _si) ? 0 : 7;
			walkData1 %= 12;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (node->_y > _di) ? 14 : 17;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	} else {

		if (dist._x > dist._y) {
			walkData2 = (node->_x > _si) ? 0 : 9;
			walkData1 %= 16;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (node->_y > _di) ? 18 : 21;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	}

//	StaticCnv v14;
//	v14._width = _yourself._cnv._width;
//	v14._height = _yourself._cnv._height;
//	v14._data0 = _yourself._cnv._array[_yourself._frame];
//	v14._data1 = _yourself._cnv.field_8[_yourself._frame];
//	v14._data2 = &_yourself.field_54;

	if ((_si < node->_x) && (_si < SCREEN_WIDTH) && (queryPath(_yourself._cnv._width/2 + _si + 2, _yourself._cnv._height + _di) != 0)) {
//		printf("walk right\n");
		_si = (_si + 2 < node->_x) ? _si + 2 : node->_x;
	}

	if ((_si > node->_x) && (_si > -20) && (queryPath(_yourself._cnv._width/2 + _si - 2, _yourself._cnv._height + _di) != 0)) {
//		printf("walk left\n");
		_si = (_si - 2 > node->_x) ? _si - 2 :node->_x;
	}

	if ((_di < node->_y) && (_di < (SCREEN_HEIGHT - _yourself._cnv._height)) && (queryPath(_yourself._cnv._width/2 + _si, _yourself._cnv._height + _di + 2) != 0)) {
//		printf("walk down\n");
		_di = (_di + 2 <= node->_y) ? _di + 2 : node->_y;
	}

	if ((_di > node->_y) && (_di > -20) && (queryPath(_yourself._cnv._width/2 + _si, _yourself._cnv._height + _di - 2) != 0)) {
//		printf("walk up\n");
		_di = (_di - 2 >= node->_y) ? _di - 2 : node->_y;
	}

//	printf("hitZone: %i, %i\n", _si, _di);
	_yourself._zone.pos._position._x = _si;
	_yourself._zone.pos._position._y = _di;

	if ((_si == _yourself._zone.pos._oldposition._x) && (_di == _yourself._zone.pos._oldposition._y)) {

		j->_finished = 1;
		checkDoor();
		freeNodeList(&node->_node);

	} else {

		_yourself._frame = v16 + walkData2 + 1;

	}

	return;
}


uint16 checkDoor() {
//	printf("checkDoor()...");

	if (_vm->_currentLocationIndex != _doorData1) {
		_doorData1 = _vm->_currentLocationIndex;
		_zoneTrap = NULL;
	}

	_engineFlags &= ~kEngineWalking;
	Zone *z = hitZone(kZoneDoor, _yourself._zone.pos._position._x + _yourself._cnv._width / 2,	_yourself._zone.pos._position._y + _yourself._cnv._height);

	if (z != NULL) {

		if ((z->_flags & kFlagsClosed) == 0) {
			_firstPosition._x = z->u.door->_startPos._x;
			_firstPosition._y = z->u.door->_startPos._y;
			_firstFrame = z->u.door->_startFrame;
			strcpy( _location, z->u.door->_location );

			_engineFlags |= kEngineChangeLocation;
			_zoneTrap = NULL;

		} else {
			runCommands(z->_commands, z);
		}
	}

	z = hitZone(kZoneTrap, _yourself._zone.pos._position._x + _yourself._cnv._width / 2, _yourself._zone.pos._position._y + _yourself._cnv._height);

	if (z != NULL) {
		_localFlags[_vm->_currentLocationIndex] |= kFlagsEnter;
		runCommands(z->_commands, z);
		_localFlags[_vm->_currentLocationIndex] &= ~kFlagsEnter;
		_zoneTrap = z;
	} else
	if (_zoneTrap != NULL) {
		_localFlags[_vm->_currentLocationIndex] |= kFlagsExit;
		runCommands(_zoneTrap->_commands, _zoneTrap);
		_localFlags[_vm->_currentLocationIndex] &= ~kFlagsExit;
		_zoneTrap = NULL;
	}

//	printf("done\n");

	_yourself._frame = walkData2;
	return _yourself._frame;
}

} // namespace Parallaction

