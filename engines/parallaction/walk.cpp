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
#if 0
static int16	walkData3 = -1000; 	// unused
#endif

int32 dotProduct(const Common::Point &p1, const Common::Point &p2) {
	return p1.x * p2.x + p1.y * p2.y;
}

// adjusts position towards nearest walkable point
//
void correctPathPoint(Common::Point &to) {

	if (queryPath(to.x, to.y)) return;

	int16 right = to.x;
	int16 left = to.x;
	do {
		right++;
	} while ((queryPath(right, to.y) == 0) && (right < SCREEN_WIDTH));
	do {
		left--;
	} while ((queryPath(left, to.y) == 0) && (left > 0));
	right = (right == SCREEN_WIDTH) ? 1000 : right - to.x;
	left = (left == 0) ? 1000 : to.x - left;


	int16 top = to.y;
	int16 bottom = to.y;
	do {
		top--;
	} while ((queryPath(to.x, top) == 0) && (top > 0));
	do {
		bottom++;
	} while ((queryPath(to.x, bottom) == 0) && (bottom < SCREEN_HEIGHT));
	top = (top == 0) ? 1000 : to.y - top;
	bottom = (bottom == SCREEN_HEIGHT) ? 1000 : bottom - to.y;


	int16 closeX = (right >= left) ? left : right;
	int16 closeY = (top >= bottom) ? bottom : top;
	int16 close = (closeX >= closeY) ? closeY : closeX;
	if (close == right) {
		to.x += right;
#if 0
		walkData3 = (_vm->_char._ani.getFrameNum() == 20) ? 7 : 9;
#endif
	} else
	if (close == left) {
		to.x -= left;
#if 0
		walkData3 = 0;
#endif
	} else
	if (close == top) {
		to.y -= top;
	} else
	if (close == bottom) {
		to.y += bottom;
#if 0
		walkData3 = (_vm->_char._ani.getFrameNum() == 20) ? 17 : 21;
#endif
	}

	return;

}

uint32 buildSubPath(const Common::Point& pos, const Common::Point& stop, WalkNode* root) {

	WalkNode *v48 = root;

	uint32 v28 = 0;
	uint32 v2C = 0;
	uint32 v34 = pos.sqrDist(stop);				// square distance from current position and target
	uint32 v30 = v34;

	Common::Point v20(pos);

	while (true) {

		WalkNode *nearestNode = NULL;
		WalkNode *locNode = (WalkNode*)_vm->_location._walkNodes._next;

		// scans location path nodes searching for the nearest Node
		// which can't be farther than the target position
		// otherwise no _closest_node is selected
		while (locNode != NULL) {

			Common::Point v8;
			locNode->getPoint(v8);
			v2C = v8.sqrDist(stop);
			v28 = v8.sqrDist(v20);

			if (v2C < v34 && v28 < v30) {
				v30 = v28;
				nearestNode = locNode;
			}

			locNode = (WalkNode*)locNode->_next;
		}

		if (nearestNode == NULL) break;

		nearestNode->getPoint(v20);
		v34 = v30 = v20.sqrDist(stop);

		addNode(v48, new WalkNode(*nearestNode));
		v48 = (WalkNode*)v48->_next;
	}

	return v34;

}

//
//	x, y: mouse click (foot) coordinates
//
WalkNode *buildWalkPath(uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "buildWalkPath to (%i, %i)", x, y);

	Common::Point to(x, y);
	correctPathPoint(to);
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

	WalkNode	dummy;
	addNode(&dummy, v48);

	Common::Point stop(v48->_x, v48->_y);
	Common::Point pos(_vm->_char._ani._left, _vm->_char._ani._top);

	v48 = &dummy;
	uint32 v34 = buildSubPath(pos, stop, v48);
	if (v38 != 0 && v34 > v38) {
		// no alternative path (gap?)
		freeNodeList(dummy._next);
		return v44;
	}

	((WalkNode*)(dummy._next))->getPoint(stop);

	v48 = &dummy;
	buildSubPath(pos, stop, v48);

	delete v44;
	return (WalkNode*)dummy._next;
}


//
//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable in a straight line
//	other values: square distance to target (point not reachable in a straight line)
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

void clipMove(Common::Point& pos, const WalkNode* from) {

	if ((pos.x < from->_x) && (pos.x < SCREEN_WIDTH) && (queryPath(_vm->_char._ani.width()/2 + pos.x + 2, _vm->_char._ani.height() + pos.y) != 0)) {
		pos.x = (pos.x + 2 < from->_x) ? pos.x + 2 : from->_x;
	}

	if ((pos.x > from->_x) && (pos.x > -20) && (queryPath(_vm->_char._ani.width()/2 + pos.x - 2, _vm->_char._ani.height() + pos.y) != 0)) {
		pos.x = (pos.x - 2 > from->_x) ? pos.x - 2 : from->_x;
	}

	if ((pos.y < from->_y) && (pos.y < (SCREEN_HEIGHT - _vm->_char._ani.height())) && (queryPath(_vm->_char._ani.width()/2 + pos.x, _vm->_char._ani.height() + pos.y + 2) != 0)) {
		pos.y = (pos.y + 2 <= from->_y) ? pos.y + 2 : from->_y;
	}

	if ((pos.y > from->_y) && (pos.y > -20) && (queryPath(_vm->_char._ani.width()/2 + pos.x, _vm->_char._ani.height() + pos.y- 2) != 0)) {
		pos.y = (pos.y - 2 >= from->_y) ? pos.y - 2 :from->_y;
	}

	return;
}

int16 selectWalkFrame(const Common::Point& pos, const WalkNode* from) {

	Common::Point dist(from->_x - pos.x, from->_y - pos.y);

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	walkData1++;

	// walk frame selection
	int16 v16;
	if (_vm->_char._ani.getFrameNum() == 20) {

		if (dist.x > dist.y) {
			walkData2 = (from->_x > pos.x) ? 0 : 7;
			walkData1 %= 12;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (from->_y > pos.y) ? 14 : 17;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	} else {

		if (dist.x > dist.y) {
			walkData2 = (from->_x > pos.x) ? 0 : 9;
			walkData1 %= 16;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (from->_y > pos.y) ? 18 : 21;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	}

	return v16;
}

WalkNode* getNextPathNode(const Common::Point& pos, WalkNode* curNode) {

	if ((curNode->_x == pos.x) && (curNode->_y == pos.y)) {

		if (curNode->_next == NULL) return NULL;

		WalkNode *tmp = curNode;
		curNode = (WalkNode*)curNode->_next;
		free(tmp);

		debugC(1, kDebugWalk, "jobWalk moving to next node (%i, %i)", tmp->_x, tmp->_y);
	}

	return curNode;

}

void jobWalk(void *parm, Job *j) {
	WalkNode *node = (WalkNode*)parm;

	Common::Point pos(_vm->_char._ani._left, _vm->_char._ani._top);

	_vm->_char._ani._oldPos = pos;

	node = getNextPathNode(pos, node);
	if (node == NULL) {
		debugC(1, kDebugWalk, "jobWalk reached last node");
		j->_finished = 1;
		checkDoor();
		free(node);
		return;
	}
	j->_parm = node;

	// selectWalkFrame must be performed before position is changed by clipMove
	int16 v16 = selectWalkFrame(pos, node);

	clipMove(pos, node);

	_vm->_char._ani._left = pos.x;
	_vm->_char._ani._top = pos.y;

	if (pos == _vm->_char._ani._oldPos) {
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
			_vm->runCommands(z->_commands, z);
		}
	}

	z = _vm->hitZone(kZoneTrap, _vm->_char._ani._left + _vm->_char._ani.width() / 2, _vm->_char._ani._top + _vm->_char._ani.height());

	if (z != NULL) {
		_localFlags[_vm->_currentLocationIndex] |= kFlagsEnter;
		_vm->runCommands(z->_commands, z);
		_localFlags[_vm->_currentLocationIndex] &= ~kFlagsEnter;
		_zoneTrap = z;
	} else
	if (_zoneTrap != NULL) {
		_localFlags[_vm->_currentLocationIndex] |= kFlagsExit;
		_vm->runCommands(_zoneTrap->_commands, _zoneTrap);
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



