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
#include "parallaction/walk.h"
#include "parallaction/zone.h"

namespace Parallaction {

uint16 walkFunc1(int16, int16, WalkNode *);


WalkNode _NULL_WALKNODE();

static byte		*_buffer;

static uint16 _doorData1 = 1000;
static Zone *_zoneTrap = NULL;

static uint16	walkData1 = 0;
static uint16	walkData2 = 0; 	// next walk frame
static int16	walkData3 = -1000; 	// unused


int32 dotProduct(const Common::Point &p1, const Common::Point &p2) {
	return p1.x * p2.x + p1.y * p2.y;
}

//
//	x, y: mouse click (foot) coordinates
//
WalkNode *buildWalkPath(uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "buildWalkPath to (%i, %i)", x, y);

	Common::Point to(x, y);

	int16 left, bottom, right, top, close, closeY, closeX;

	// looks for closest usable path Point
	if (queryPath(to.x, to.y) == 0) {

		right = left = to.x;

		do {
			right++;
		} while ((queryPath(right, to.y) == 0) && (right < SCREEN_WIDTH));

		do {
			left--;
		} while ((queryPath(left, to.y) == 0) && (left > 0));

		right = (right == SCREEN_WIDTH) ? 1000 : right - to.x;
		left = (left == 0) ? 1000 : to.x - left;

		top = bottom = to.y;

		do {
			top--;
		} while ((queryPath(to.x, top) == 0) && (top > 0));

		do {
			bottom++;
		} while ((queryPath(to.x, bottom) == 0) && (bottom < SCREEN_HEIGHT));

		top = (top == 0) ? 1000 : to.y - top;
		bottom = (bottom == SCREEN_HEIGHT) ? 1000 : bottom - to.y;

		closeX = (right >= left) ? left : right;
		closeY = (top >= bottom) ? bottom : top;

		close = (closeX >= closeY) ? closeY : closeX;

		if (close == right) {
			to.x += right;
			walkData3 = (_vm->_char._ani.getFrameNum() == 20) ? 7 : 9;
		} else
		if (close == left) {
			to.x -= left;
			walkData3 = 0;
		} else
		if (close == top) {
			to.y -= top;
		} else
		if (close == bottom) {
			to.y += bottom;
			walkData3 = (_vm->_char._ani.getFrameNum() == 20) ? 17 : 21;
		}

	}
	debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to.x, to.y);

	WalkNode *v48 = new WalkNode(to.x - _vm->_char._ani.width() / 2, to.y - _vm->_char._ani.height());
	WalkNode *v44 = new WalkNode(*v48);

	uint16 v38 = walkFunc1(to.x, to.y, v44);
	if (v38 == 1) {
		// destination directly reachable
		debugC(1, kDebugWalk, "direct move to (%i, %i)", to.x, to.y);
		delete v44;
		return v48;
	}

	// path is obstructed: find alternative
	debugC(1, kDebugWalk, "trying to build walk path to (%i, %i)", to.x, to.y);

	WalkNode	v58;

	int16 _si = v48->_x;						// _si, _di: target top left coordinates
	int16 _di = v48->_y;
	addNode(&v58, v48);

	WalkNode *_closest_node = NULL;

	int32 v30, v34, v2C, v28;

	byte _closest_node_found = 1;
	bool emptyList = true;

	do {

		v48 = &v58;

		Common::Point v20(_vm->_char._ani._left, _vm->_char._ani._top);
		Common::Point v8(_si - _vm->_char._ani._left, _di - _vm->_char._ani._top);

		v34 = v30 = dotProduct(v8, v8);				// square distance from current position and target

		while (_closest_node_found != 0) {

			_closest_node_found = 0;
			WalkNode *location_node = (WalkNode*)_vm->_location._walkNodes._next;

			// scans location path nodes searching for the nearest Node
			// which can't be farther than the target position
			// otherwise no _closest_node is selected
			while (location_node != NULL) {
				v8.x = location_node->_x - _si;
				v8.y = location_node->_y - _di;
				v2C = dotProduct(v8, v8); 			// square distance from Node to target position

				v8.x = location_node->_x - v20.x;
				v8.y = location_node->_y - v20.y;
				v28 = dotProduct(v8, v8); 			// square distance from Node to current position

				if (v2C < v34 && v28 < v30) {
					_closest_node_found = 1;
					v30 = v28;
					_closest_node = location_node;
				}

				location_node = (WalkNode*)location_node->_next;
			}

			if (_closest_node_found == 0) break;

			WalkNode *_newnode = new WalkNode(*_closest_node);
			_newnode->getPoint(v20);

			Common::Point tmp(_si - v20.x, _di - v20.y);

			v34 = v30 = dotProduct(tmp, tmp);


			debugC(1, kDebugWalk, "adding walk node (%i, %i) to path", _newnode->_x, _newnode->_y);

			addNode(v48, _newnode);
			v48 = _newnode;
		}

		if (!emptyList) break;

		if (v38 != 0 && v34 > v38) {
			// no alternative path (gap?)
			freeNodeList(v58._next);
			debugC(1, kDebugWalk, "can't find a path node: rejecting partial path");
			return v44;
		} else {
			_si = ((WalkNode*)(v58._next))->_x;
			_di = ((WalkNode*)(v58._next))->_y;
			emptyList = false;
			_closest_node_found = 1;
		}

	} while (true);

	debugC(1, kDebugWalk, "walk path completed");

	WalkNode* tmp = &v58;
	uint16 i = 1;
	while (tmp->_next) {
		debugC(1, kDebugWalk, "node %i: %i, %i", i, tmp->_x, tmp->_y);
		tmp = (WalkNode*)tmp->_next;
		i++;
	}


	delete v44;
	return (WalkNode*)v58._next;
}


//
//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable
//	other values: square distance to target (not reachable)
//
uint16 walkFunc1(int16 x, int16 y, WalkNode *Node) {

	Common::Point arg(x, y);

	Common::Point v4(0, 0);

	Common::Point foot(
		_vm->_char._ani._left + _vm->_char._ani.width()/2,
		_vm->_char._ani._top + _vm->_char._ani.height()
	);

	Common::Point v8(foot);

	while (foot != arg) {

		if (foot.x < x && queryPath(foot.x + 1, foot.y) != 0) foot.x++;
		if (foot.x > x && queryPath(foot.x - 1, foot.y) != 0) foot.x--;
		if (foot.y < y && queryPath(foot.x, foot.y + 1) != 0) foot.y++;
		if (foot.y > y && queryPath(foot.x, foot.y - 1) != 0) foot.y--;


		if (foot == v8 && foot != arg) {
			// foot couldn't move and still away from target

			v4 = foot;

			while (foot != arg) {

				if (foot.x < x && queryPath(foot.x + 1, foot.y) == 0) foot.x++;
				if (foot.x > x && queryPath(foot.x - 1, foot.y) == 0) foot.x--;
				if (foot.y < y && queryPath(foot.x, foot.y + 1) == 0) foot.y++;
				if (foot.y > y && queryPath(foot.x, foot.y - 1) == 0) foot.y--;

				if (foot == v8 && foot != arg)
					return 0;

				v8 = foot;
			}

			Node->_x = v4.x - _vm->_char._ani.width() / 2;
			Node->_y = v4.y - _vm->_char._ani.height();

			return (x - v4.x) * (x - v4.x) + (y - v4.y) * (y - v4.y);
		}

		v8 = foot;

	}

	// there exists an unobstructed path
	return 1;
}


void jobWalk(void *parm, Job *j) {
	WalkNode *node = (WalkNode*)parm;

	int16 _si = _vm->_char._ani._left;
	int16 _di = _vm->_char._ani._top;

//	debugC(1, kDebugWalk, "jobWalk to (%i, %i)", node->_x + _vm->_char._ani.width() / 2, node->_y + _vm->_char._ani.height());

	_vm->_char._ani._oldLeft = _si;
	_vm->_char._ani._oldTop = _di;

	if ((node->_x == _si) && (node->_y == _di)) {
		if (node->_next == NULL) {

			debugC(1, kDebugWalk, "jobWalk reached last node");

			j->_finished = 1;
			checkDoor();
			free(node);
			return;
		}


		WalkNode *tmp = (WalkNode*)node->_next;
		j->_parm = node->_next;
		free(node);

		debugC(1, kDebugWalk, "jobWalk moving to next node (%i, %i)", tmp->_x, tmp->_y);

		node = (WalkNode*)tmp;
	}

	Common::Point dist(node->_x - _vm->_char._ani._left, node->_y - _vm->_char._ani._top);

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	walkData1++;

	// walk frame selection
	int16 v16;
	if (_vm->_char._ani.getFrameNum() == 20) {

		if (dist.x > dist.y) {
			walkData2 = (node->_x > _si) ? 0 : 7;
			walkData1 %= 12;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (node->_y > _di) ? 14 : 17;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	} else {

		if (dist.x > dist.y) {
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
//	v14._width = _vm->_char._ani.width();
//	v14._height = _vm->_char._ani.height();
//	v14._data0 = _vm->_char._ani._cnv._array[_vm->_char._ani._frame];
//	v14._data1 = _vm->_char._ani._cnv.field_8[_vm->_char._ani._frame];

	if ((_si < node->_x) && (_si < SCREEN_WIDTH) && (queryPath(_vm->_char._ani.width()/2 + _si + 2, _vm->_char._ani.height() + _di) != 0)) {
//		printf("walk right\n");
		_si = (_si + 2 < node->_x) ? _si + 2 : node->_x;
	}

	if ((_si > node->_x) && (_si > -20) && (queryPath(_vm->_char._ani.width()/2 + _si - 2, _vm->_char._ani.height() + _di) != 0)) {
//		printf("walk left\n");
		_si = (_si - 2 > node->_x) ? _si - 2 :node->_x;
	}

	if ((_di < node->_y) && (_di < (SCREEN_HEIGHT - _vm->_char._ani.height())) && (queryPath(_vm->_char._ani.width()/2 + _si, _vm->_char._ani.height() + _di + 2) != 0)) {
//		printf("walk down\n");
		_di = (_di + 2 <= node->_y) ? _di + 2 : node->_y;
	}

	if ((_di > node->_y) && (_di > -20) && (queryPath(_vm->_char._ani.width()/2 + _si, _vm->_char._ani.height() + _di - 2) != 0)) {
//		printf("walk up\n");
		_di = (_di - 2 >= node->_y) ? _di - 2 : node->_y;
	}

//	printf("hitZone: %i, %i\n", _si, _di);
	_vm->_char._ani._left = _si;
	_vm->_char._ani._top = _di;

	if ((_si == _vm->_char._ani._oldLeft) && (_di == _vm->_char._ani._oldTop)) {

		j->_finished = 1;
		checkDoor();
		freeNodeList(node);

	} else {

		_vm->_char._ani._frame = v16 + walkData2 + 1;

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
	Zone *z = _vm->hitZone(kZoneDoor, _vm->_char._ani._left + _vm->_char._ani.width() / 2,	_vm->_char._ani._top + _vm->_char._ani.height());

	if (z != NULL) {

		if ((z->_flags & kFlagsClosed) == 0) {
			_vm->_location._startPosition.x = z->u.door->_startPos.x;
			_vm->_location._startPosition.y = z->u.door->_startPos.y;
			_vm->_location._startFrame = z->u.door->_startFrame;
			strcpy( _vm->_location._name, z->u.door->_location );

			_engineFlags |= kEngineChangeLocation;
			_zoneTrap = NULL;

		} else {
			runCommands(z->_commands, z);
		}
	}

	z = _vm->hitZone(kZoneTrap, _vm->_char._ani._left + _vm->_char._ani.width() / 2, _vm->_char._ani._top + _vm->_char._ani.height());

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

	_vm->_char._ani._frame = walkData2;
	return _vm->_char._ani._frame;
}

uint16 queryPath(uint16 x, uint16 y) {

	byte _al = _buffer[y*40 + x/8];
	byte _dl = 1 << (x % 8);

	return _al & _dl;

}

void setPath(byte *path) {
	memcpy(_buffer, path, SCREENPATH_WIDTH*SCREEN_HEIGHT);
}

void initWalk() {
	_buffer = (byte*)malloc(SCREENPATH_WIDTH * SCREEN_HEIGHT);
}

} // namespace Parallaction


