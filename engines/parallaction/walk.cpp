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
 * $URL$
 * $Id$
 *
 */

#include "parallaction/parallaction.h"

namespace Parallaction {

// should be reset on location switch
static uint16 _doorData1 = 1000;
static ZonePtr _zoneTrap;

// should be reset on character switch
static uint16	walkData1 = 0;
static uint16	walkData2 = 0;	// next walk frame


inline byte PathBuffer::getValue(uint16 x, uint16 y) {
	byte m = data[(x >> 3) + y * internalWidth];
	uint n = (_vm->getPlatform() == Common::kPlatformPC) ? (x & 7) : (7 - (x & 7));
	return ((1 << n) & m) >> n;
}

// adjusts position towards nearest walkable point
//
void PathBuilder::correctPathPoint(Common::Point &to) {

	if (_vm->_pathBuffer->getValue(to.x, to.y)) return;

	int16 right = to.x;
	int16 left = to.x;
	do {
		right++;
	} while ((_vm->_pathBuffer->getValue(right, to.y) == 0) && (right < _vm->_pathBuffer->w));
	do {
		left--;
	} while ((_vm->_pathBuffer->getValue(left, to.y) == 0) && (left > 0));
	right = (right == _vm->_pathBuffer->w) ? 1000 : right - to.x;
	left = (left == 0) ? 1000 : to.x - left;


	int16 top = to.y;
	int16 bottom = to.y;
	do {
		top--;
	} while ((_vm->_pathBuffer->getValue(to.x, top) == 0) && (top > 0));
	do {
		bottom++;
	} while ((_vm->_pathBuffer->getValue(to.x, bottom) == 0) && (bottom < _vm->_pathBuffer->h));
	top = (top == 0) ? 1000 : to.y - top;
	bottom = (bottom == _vm->_pathBuffer->h) ? 1000 : bottom - to.y;


	int16 closeX = (right >= left) ? left : right;
	int16 closeY = (top >= bottom) ? bottom : top;
	int16 close = (closeX >= closeY) ? closeY : closeX;
	if (close == right) {
		to.x += right;
	} else
	if (close == left) {
		to.x -= left;
	} else
	if (close == top) {
		to.y -= top;
	} else
	if (close == bottom) {
		to.y += bottom;
	}

	return;

}

uint32 PathBuilder::buildSubPath(const Common::Point& pos, const Common::Point& stop) {

	uint32 v28 = 0;
	uint32 v2C = 0;
	uint32 v34 = pos.sqrDist(stop);				// square distance from current position and target
	uint32 v30 = v34;

	_subPath.clear();

	Common::Point v20(pos);

	while (true) {

		WalkNodeList::iterator nearest = _vm->_location._walkNodes.end();
		WalkNodeList::iterator locNode = _vm->_location._walkNodes.begin();

		// scans location path nodes searching for the nearest Node
		// which can't be farther than the target position
		// otherwise no _closest_node is selected
		while (locNode != _vm->_location._walkNodes.end()) {

			Common::Point v8;
			(*locNode)->getPoint(v8);
			v2C = v8.sqrDist(stop);
			v28 = v8.sqrDist(v20);

			if (v2C < v34 && v28 < v30) {
				v30 = v28;
				nearest = locNode;
			}

			locNode++;
		}

		if (nearest == _vm->_location._walkNodes.end()) break;

		(*nearest)->getPoint(v20);
		v34 = v30 = v20.sqrDist(stop);

		_subPath.push_back(WalkNodePtr(new WalkNode(**nearest)));
	}

	return v34;

}
#if 0
void printNodes(WalkNodeList *list, const char* text) {
	printf("%s\n-------------------\n", text);
	for (WalkNodeList::iterator it = list->begin(); it != list->end(); it++)
		printf("node [%p] (%i, %i)\n", *it, (*it)->_x, (*it)->_y);

	return;
}
#endif
//
//	x, y: mouse click (foot) coordinates
//
WalkNodeList *PathBuilder::buildPath(uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "PathBuilder::buildPath to (%i, %i)", x, y);

	Common::Point to(x, y);
	correctPathPoint(to);
	debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to.x, to.y);

	WalkNodePtr v48(new WalkNode(to.x, to.y));
	WalkNodePtr v44 = v48;

	uint16 v38 = walkFunc1(to.x, to.y, v44);
	if (v38 == 1) {
		// destination directly reachable
		debugC(1, kDebugWalk, "direct move to (%i, %i)", to.x, to.y);

		_list = new WalkNodeList;
		_list->push_back(v48);
		return _list;
	}

	// path is obstructed: look for alternative
	_list = new WalkNodeList;
	_list->push_back(v48);
#if 0
	printNodes(_list, "start");
#endif

	Common::Point stop(v48->_x, v48->_y);
	Common::Point pos;
	_vm->_char.getFoot(pos);

	uint32 v34 = buildSubPath(pos, stop);
	if (v38 != 0 && v34 > v38) {
		// no alternative path (gap?)
		_list->clear();
		_list->push_back(v44);
		return _list;
	}
	_list->insert(_list->begin(), _subPath.begin(), _subPath.end());
#if 0
	printNodes(_list, "first segment");
#endif

	(*_list->begin())->getPoint(stop);
	buildSubPath(pos, stop);
	_list->insert(_list->begin(), _subPath.begin(), _subPath.end());
#if 0
	printNodes(_list, "complete");
#endif

	return _list;
}


//
//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable in a straight line
//	other values: square distance to target (point not reachable in a straight line)
//
uint16 PathBuilder::walkFunc1(int16 x, int16 y, WalkNodePtr Node) {

	Common::Point arg(x, y);

	Common::Point v4(0, 0);

	Common::Point foot;
	_vm->_char.getFoot(foot);

	Common::Point v8(foot);

	while (foot != arg) {

		if (foot.x < x && _vm->_pathBuffer->getValue(foot.x + 1, foot.y) != 0) foot.x++;
		if (foot.x > x && _vm->_pathBuffer->getValue(foot.x - 1, foot.y) != 0) foot.x--;
		if (foot.y < y && _vm->_pathBuffer->getValue(foot.x, foot.y + 1) != 0) foot.y++;
		if (foot.y > y && _vm->_pathBuffer->getValue(foot.x, foot.y - 1) != 0) foot.y--;


		if (foot == v8 && foot != arg) {
			// foot couldn't move and still away from target

			v4 = foot;

			while (foot != arg) {

				if (foot.x < x && _vm->_pathBuffer->getValue(foot.x + 1, foot.y) == 0) foot.x++;
				if (foot.x > x && _vm->_pathBuffer->getValue(foot.x - 1, foot.y) == 0) foot.x--;
				if (foot.y < y && _vm->_pathBuffer->getValue(foot.x, foot.y + 1) == 0) foot.y++;
				if (foot.y > y && _vm->_pathBuffer->getValue(foot.x, foot.y - 1) == 0) foot.y--;

				if (foot == v8 && foot != arg)
					return 0;

				v8 = foot;
			}

			Node->_x = v4.x;
			Node->_y = v4.y;

			return (x - v4.x) * (x - v4.x) + (y - v4.y) * (y - v4.y);
		}

		v8 = foot;

	}

	// there exists an unobstructed path
	return 1;
}

void Parallaction::clipMove(Common::Point& pos, const WalkNodePtr to) {

	if ((pos.x < to->_x) && (pos.x < _pathBuffer->w) && (_pathBuffer->getValue(pos.x + 2, pos.y) != 0)) {
		pos.x = (pos.x + 2 < to->_x) ? pos.x + 2 : to->_x;
	}

	if ((pos.x > to->_x) && (pos.x > 0) && (_pathBuffer->getValue(pos.x - 2, pos.y) != 0)) {
		pos.x = (pos.x - 2 > to->_x) ? pos.x - 2 : to->_x;
	}

	if ((pos.y < to->_y) && (pos.y < _pathBuffer->h) && (_pathBuffer->getValue(pos.x, pos.y + 2) != 0)) {
		pos.y = (pos.y + 2 <= to->_y) ? pos.y + 2 : to->_y;
	}

	if ((pos.y > to->_y) && (pos.y > 0) && (_pathBuffer->getValue(pos.x, pos.y - 2) != 0)) {
		pos.y = (pos.y - 2 >= to->_y) ? pos.y - 2 : to->_y;
	}

	return;
}

int16 Parallaction::selectWalkFrame(Character &character, const Common::Point& pos, const WalkNodePtr to) {

	Common::Point dist(to->_x - pos.x, to->_y - pos.y);

	if (dist.x < 0)
		dist.x = -dist.x;
	if (dist.y < 0)
		dist.y = -dist.y;

	walkData1++;

	// walk frame selection
	int16 v16;
	if (character._ani->getFrameNum() == 20) {

		if (dist.x > dist.y) {
			walkData2 = (to->_x > pos.x) ? 0 : 7;
			walkData1 %= 12;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (to->_y > pos.y) ? 14 : 17;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	} else {

		if (dist.x > dist.y) {
			walkData2 = (to->_x > pos.x) ? 0 : 9;
			walkData1 %= 16;
			v16 = walkData1 / 2;
		} else {
			walkData2 = (to->_y > pos.y) ? 18 : 21;
			walkData1 %= 8;
			v16 = walkData1 / 4;
		}

	}

	return v16;
}

void Parallaction::checkDoor(Character &character) {
	if (_currentLocationIndex != _doorData1) {
		_doorData1 = _currentLocationIndex;
		_zoneTrap = nullZonePtr;
	}

	_engineFlags &= ~kEngineWalking;

	Common::Point foot;

	character.getFoot(foot);
	ZonePtr z = hitZone(kZoneDoor, foot.x, foot.y);

	if (z) {

		if ((z->_flags & kFlagsClosed) == 0) {
			_location._startPosition = z->u.door->_startPos;
			_location._startFrame = z->u.door->_startFrame;

			scheduleLocationSwitch(z->u.door->_location);
			_zoneTrap = nullZonePtr;

		} else {
			_cmdExec->run(z->_commands, z);
		}
	}

	character.getFoot(foot);
	z = hitZone(kZoneTrap, foot.x, foot.y);

	if (z) {
		setLocationFlags(kFlagsEnter);
		_cmdExec->run(z->_commands, z);
		clearLocationFlags(kFlagsEnter);
		_zoneTrap = z;
	} else
	if (_zoneTrap) {
		setLocationFlags(kFlagsExit);
		_cmdExec->run(_zoneTrap->_commands, _zoneTrap);
		clearLocationFlags(kFlagsExit);
		_zoneTrap = nullZonePtr;
	}

}


void Parallaction::finalizeWalk(Character &character) {
	checkDoor(character);
	delete character._walkPath;
	character._walkPath = 0;

	character._ani->_frame = walkData2;
}

void Parallaction_ns::walk(Character &character) {
	if ((_engineFlags & kEngineWalking) == 0) {
		return;
	}

	Common::Point curPos;
	character.getFoot(curPos);

	WalkNodeList::iterator it = character._walkPath->begin();

	if (it != character._walkPath->end()) {
		if ((*it)->_x == curPos.x && (*it)->_y == curPos.y) {
			debugC(1, kDebugWalk, "walk reached node (%i, %i)", (*it)->_x, (*it)->_y);
			it = character._walkPath->erase(it);
		}
	}
	if (it == character._walkPath->end()) {
		debugC(1, kDebugWalk, "walk reached last node");
		finalizeWalk(character);
		return;
	}

	// selectWalkFrame must be performed before position is changed by clipMove
	int16 walkFrame = selectWalkFrame(character, curPos, *it);

	Common::Point newPos(curPos);
	clipMove(newPos, *it);
	character.setFoot(newPos);

	if (newPos == curPos) {
		debugC(1, kDebugWalk, "walk was blocked by an unforeseen obstacle");
		finalizeWalk(character);
	} else {
		character._ani->_frame = walkFrame + walkData2 + 1;
	}

	return;
}


WalkNode::WalkNode() : _x(0), _y(0) {
}

WalkNode::WalkNode(int16 x, int16 y) : _x(x), _y(y) {
}

WalkNode::WalkNode(const WalkNode& w) : _x(w._x), _y(w._y) {
}

void WalkNode::getPoint(Common::Point &p) const {
	p.x = _x;
	p.y = _y;
}

PathBuilder::PathBuilder(AnimationPtr anim) : _anim(anim), _list(0) {
}


} // namespace Parallaction
