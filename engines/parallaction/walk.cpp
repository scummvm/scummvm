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



inline byte PathBuffer::getValue(uint16 x, uint16 y) {
	byte m = data[(x >> 3) + y * internalWidth];
	uint bit = 0;
	switch (_vm->getGameType()) {
	case GType_Nippon:
		bit = (_vm->getPlatform() == Common::kPlatformPC) ? (x & 7) : (7 - (x & 7));
		break;

	case GType_BRA:
		// Amiga and PC versions pack the path bits the same way in BRA
		bit = 7 - (x & 7);
		break;

	default:
		error("path mask not yet implemented for this game type");
	}
	return ((1 << bit) & m) >> bit;
}

// adjusts position towards nearest walkable point
//
void PathBuilder_NS::correctPathPoint(Common::Point &to) {

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

uint32 PathBuilder_NS::buildSubPath(const Common::Point& pos, const Common::Point& stop) {

	uint32 v28 = 0;
	uint32 v2C = 0;
	uint32 v34 = pos.sqrDist(stop);				// square distance from current position and target
	uint32 v30 = v34;

	_subPath.clear();

	Common::Point v20(pos);

	while (true) {

		PointList::iterator nearest = _vm->_location._walkPoints.end();
		PointList::iterator locNode = _vm->_location._walkPoints.begin();

		// scans location path nodes searching for the nearest Node
		// which can't be farther than the target position
		// otherwise no _closest_node is selected
		while (locNode != _vm->_location._walkPoints.end()) {

			Common::Point v8 = *locNode;
			v2C = v8.sqrDist(stop);
			v28 = v8.sqrDist(v20);

			if (v2C < v34 && v28 < v30) {
				v30 = v28;
				nearest = locNode;
			}

			locNode++;
		}

		if (nearest == _vm->_location._walkPoints.end()) break;

		v20 = *nearest;
		v34 = v30 = v20.sqrDist(stop);

		_subPath.push_back(*nearest);
	}

	return v34;

}

//
//	x, y: mouse click (foot) coordinates
//
PointList *PathBuilder_NS::buildPath(uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "PathBuilder::buildPath to (%i, %i)", x, y);

	Common::Point to(x, y);
	correctPathPoint(to);
	debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to.x, to.y);

	Common::Point v48(to);
	Common::Point v44(to);

	uint16 v38 = walkFunc1(to.x, to.y, v44);
	if (v38 == 1) {
		// destination directly reachable
		debugC(1, kDebugWalk, "direct move to (%i, %i)", to.x, to.y);

		_list = new PointList;
		_list->push_back(v48);
		return _list;
	}

	// path is obstructed: look for alternative
	_list = new PointList;
	_list->push_back(v48);
#if 0
	printNodes(_list, "start");
#endif

	Common::Point stop(v48.x, v48.y);
	Common::Point pos;
	_ch->getFoot(pos);

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

	stop = *_list->begin();
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
uint16 PathBuilder_NS::walkFunc1(int16 x, int16 y, Common::Point& node) {

	Common::Point arg(x, y);

	Common::Point v4(0, 0);

	Common::Point foot;
	_ch->getFoot(foot);

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

			node = v4;
			return (x - v4.x) * (x - v4.x) + (y - v4.y) * (y - v4.y);
		}

		v8 = foot;

	}

	// there exists an unobstructed path
	return 1;
}

void Parallaction::clipMove(Common::Point& pos, const Common::Point& to) {

	if ((pos.x < to.x) && (pos.x < _pathBuffer->w) && (_pathBuffer->getValue(pos.x + 2, pos.y) != 0)) {
		pos.x = (pos.x + 2 < to.x) ? pos.x + 2 : to.x;
	}

	if ((pos.x > to.x) && (pos.x > 0) && (_pathBuffer->getValue(pos.x - 2, pos.y) != 0)) {
		pos.x = (pos.x - 2 > to.x) ? pos.x - 2 : to.x;
	}

	if ((pos.y < to.y) && (pos.y < _pathBuffer->h) && (_pathBuffer->getValue(pos.x, pos.y + 2) != 0)) {
		pos.y = (pos.y + 2 <= to.y) ? pos.y + 2 : to.y;
	}

	if ((pos.y > to.y) && (pos.y > 0) && (_pathBuffer->getValue(pos.x, pos.y - 2) != 0)) {
		pos.y = (pos.y - 2 >= to.y) ? pos.y - 2 : to.y;
	}

	return;
}


void Parallaction::checkDoor(const Common::Point &foot) {

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
	_engineFlags &= ~kEngineWalking;

	Common::Point foot;
	character.getFoot(foot);
	checkDoor(foot);

	delete character._walkPath;
	character._walkPath = 0;
}

void Parallaction_ns::walk(Character &character) {
	if ((_engineFlags & kEngineWalking) == 0) {
		return;
	}

	Common::Point curPos;
	character.getFoot(curPos);

	// update target, if previous was reached
	PointList::iterator it = character._walkPath->begin();
	if (it != character._walkPath->end()) {
		if ((*it).x == curPos.x && (*it).y == curPos.y) {
			debugC(1, kDebugWalk, "walk reached node (%i, %i)", (*it).x, (*it).y);
			it = character._walkPath->erase(it);
		}
	}

	// advance character towards the target
	Common::Point targetPos;
	if (it == character._walkPath->end()) {
		debugC(1, kDebugWalk, "walk reached last node");
		finalizeWalk(character);
		targetPos = curPos;
	} else {
		// targetPos is saved to help setting character direction
		targetPos.x = (*it).x;
		targetPos.y = (*it).y;

		Common::Point newPos(curPos);
		clipMove(newPos, targetPos);
		character.setFoot(newPos);

		if (newPos == curPos) {
			debugC(1, kDebugWalk, "walk was blocked by an unforeseen obstacle");
			finalizeWalk(character);
			targetPos = newPos;	// when walking is interrupted, targetPos must be hacked so that a still frame can be selected
		}
	}

	// targetPos is used to select the direction (and the walkFrame) of a character,
	// since it doesn't cause the sudden changes in orientation that newPos would.
	// Since newPos is 'adjusted' according to walkable areas, an imaginary line drawn
	// from curPos to newPos is prone to abrutply change in direction, thus making the
	// code select 'too different' frames when walking diagonally against obstacles,
	// and yielding an annoying shaking effect in the character.
	character.updateDirection(curPos, targetPos);
}



PathBuilder_NS::PathBuilder_NS(Character *ch) : PathBuilder(ch), _list(0) {
}

#define isPositionOnPath(x,y) _vm->_pathBuffer->getValue((x), (y))


bool PathBuilder_BR::directPathExists(const Common::Point &from, const Common::Point &to) {

	Common::Point copy(from);
	Common::Point p(copy);

	while (p != to) {

		if (p.x < to.x && isPositionOnPath(p.x + 1, p.y)) p.x++;
		if (p.x > to.x && isPositionOnPath(p.x - 1, p.y)) p.x--;
		if (p.y < to.y && isPositionOnPath(p.x, p.y + 1)) p.y++;
		if (p.y > to.y && isPositionOnPath(p.x, p.y - 1)) p.y--;

		if (p == copy && p != to) {
			return false;
		}

		copy = p;
	}

	return true;
}

PointList* PathBuilder_BR::buildPath(uint16 x, uint16 y) {
	Common::Point foot;
	_ch->getFoot(foot);

	debugC(1, kDebugWalk, "buildPath: from (%i, %i) to (%i, %i)", foot.x, foot.y, x, y);

	PointList *list = new PointList;

	// look for easy path first
	Common::Point dest(x, y);
	if (directPathExists(foot, dest)) {
		list->push_back(dest);
		debugC(3, kDebugWalk, "buildPath: direct path found");
		return list;
	}

	// look for short circuit cases
	ZonePtr z0 = _vm->hitZone(kZonePath, x, y);
	if (z0 == nullZonePtr) {
		list->push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 0");
		return list;
	}
	ZonePtr z1 = _vm->hitZone(kZonePath, foot.x, foot.y);
	if (z1 == nullZonePtr || z1 == z0) {
		list->push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 1");
		return list;
	}

	// build complex path
	int id = atoi(z0->_name);

	if (z1->u.path->_lists[id].empty()) {
		list->clear();
		debugC(3, kDebugWalk, "buildPath: no path");
		return list;
	}

	PointList::iterator b = z1->u.path->_lists[id].begin();
	PointList::iterator e = z1->u.path->_lists[id].end();
	for ( ; b != e; b++) {
		list->push_front(*b);
	}
	list->push_back(dest);
	debugC(3, kDebugWalk, "buildPath: complex path");

	return list;
}

PathBuilder_BR::PathBuilder_BR(Character *ch) : PathBuilder(ch) {
}

} // namespace Parallaction
