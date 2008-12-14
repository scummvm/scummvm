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



#define IS_PATH_CLEAR(x,y) _vm->_gfx->_backgroundInfo->path.getValue((x), (y))

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

	if (IS_PATH_CLEAR(to.x, to.y)) return;

	int maxX = _vm->_gfx->_backgroundInfo->path.w;
	int maxY = _vm->_gfx->_backgroundInfo->path.h;

	int16 right = to.x;
	int16 left = to.x;
	do {
		right++;
	} while (!IS_PATH_CLEAR(right, to.y) && (right < maxX));
	do {
		left--;
	} while (!IS_PATH_CLEAR(left, to.y) && (left > 0));
	right = (right == maxX) ? 1000 : right - to.x;
	left = (left == 0) ? 1000 : to.x - left;


	int16 top = to.y;
	int16 bottom = to.y;
	do {
		top--;
	} while (!IS_PATH_CLEAR(to.x, top) && (top > 0));
	do {
		bottom++;
	} while (!IS_PATH_CLEAR(to.x, bottom) && (bottom < maxY));
	top = (top == 0) ? 1000 : to.y - top;
	bottom = (bottom == maxY) ? 1000 : bottom - to.y;


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
void PathBuilder_NS::buildPath(uint16 x, uint16 y) {
	debugC(1, kDebugWalk, "PathBuilder::buildPath to (%i, %i)", x, y);

	_ch->_walkPath.clear();

	Common::Point to(x, y);
	correctPathPoint(to);
	debugC(1, kDebugWalk, "found closest path point at (%i, %i)", to.x, to.y);

	Common::Point v48(to);
	Common::Point v44(to);

	uint16 v38 = walkFunc1(to, v44);
	if (v38 == 1) {
		// destination directly reachable
		debugC(1, kDebugWalk, "direct move to (%i, %i)", to.x, to.y);
		_ch->_walkPath.push_back(v48);
		return;
	}

	// path is obstructed: look for alternative
	_ch->_walkPath.push_back(v48);
	Common::Point pos;
	_ch->getFoot(pos);

	uint32 v34 = buildSubPath(pos, v48);
	if (v38 != 0 && v34 > v38) {
		// no alternative path (gap?)
		_ch->_walkPath.clear();
		_ch->_walkPath.push_back(v44);
		return;
	}
	_ch->_walkPath.insert(_ch->_walkPath.begin(), _subPath.begin(), _subPath.end());

	buildSubPath(pos, *_ch->_walkPath.begin());
	_ch->_walkPath.insert(_ch->_walkPath.begin(), _subPath.begin(), _subPath.end());

	return;
}


//
//	x,y : top left coordinates
//
//	0 : Point not reachable
//	1 : Point reachable in a straight line
//	other values: square distance to target (point not reachable in a straight line)
//
uint16 PathBuilder_NS::walkFunc1(const Common::Point &to, Common::Point& node) {

	Common::Point arg(to);

	Common::Point v4;

	Common::Point foot;
	_ch->getFoot(foot);

	Common::Point v8(foot);

	while (foot != arg) {

		if (foot.x < to.x && IS_PATH_CLEAR(foot.x + 1, foot.y)) foot.x++;
		if (foot.x > to.x && IS_PATH_CLEAR(foot.x - 1, foot.y)) foot.x--;
		if (foot.y < to.y && IS_PATH_CLEAR(foot.x, foot.y + 1)) foot.y++;
		if (foot.y > to.y && IS_PATH_CLEAR(foot.x, foot.y - 1)) foot.y--;


		if (foot == v8 && foot != arg) {
			// foot couldn't move and still away from target

			v4 = foot;

			while (foot != arg) {

				if (foot.x < to.x && !IS_PATH_CLEAR(foot.x + 1, foot.y)) foot.x++;
				if (foot.x > to.x && !IS_PATH_CLEAR(foot.x - 1, foot.y)) foot.x--;
				if (foot.y < to.y && !IS_PATH_CLEAR(foot.x, foot.y + 1)) foot.y++;
				if (foot.y > to.y && !IS_PATH_CLEAR(foot.x, foot.y - 1)) foot.y--;

				if (foot == v8 && foot != arg)
					return 0;

				v8 = foot;
			}

			node = v4;
			return v4.sqrDist(to);
		}

		v8 = foot;

	}

	// there exists an unobstructed path
	return 1;
}

void PathWalker_NS::clipMove(Common::Point& pos, const Common::Point& to) {

	if ((pos.x < to.x) && (pos.x < _vm->_gfx->_backgroundInfo->path.w) && IS_PATH_CLEAR(pos.x + 2, pos.y)) {
		pos.x = (pos.x + 2 < to.x) ? pos.x + 2 : to.x;
	}

	if ((pos.x > to.x) && (pos.x > 0) && IS_PATH_CLEAR(pos.x - 2, pos.y)) {
		pos.x = (pos.x - 2 > to.x) ? pos.x - 2 : to.x;
	}

	if ((pos.y < to.y) && (pos.y < _vm->_gfx->_backgroundInfo->path.h) && IS_PATH_CLEAR(pos.x, pos.y + 2)) {
		pos.y = (pos.y + 2 <= to.y) ? pos.y + 2 : to.y;
	}

	if ((pos.y > to.y) && (pos.y > 0) && IS_PATH_CLEAR(pos.x, pos.y - 2)) {
		pos.y = (pos.y - 2 >= to.y) ? pos.y - 2 : to.y;
	}

	return;
}


void PathWalker_NS::checkDoor(const Common::Point &foot) {

	ZonePtr z = _vm->hitZone(kZoneDoor, foot.x, foot.y);
	if (z) {
		if ((z->_flags & kFlagsClosed) == 0) {
			_vm->_location._startPosition = z->u.door->_startPos;
			_vm->_location._startFrame = z->u.door->_startFrame;
			_vm->scheduleLocationSwitch(z->u.door->_location);
			_vm->_zoneTrap = nullZonePtr;
		} else {
			_vm->_cmdExec->run(z->_commands, z);
		}
	}

	z = _vm->hitZone(kZoneTrap, foot.x, foot.y);
	if (z) {
		_vm->setLocationFlags(kFlagsEnter);
		_vm->_cmdExec->run(z->_commands, z);
		_vm->clearLocationFlags(kFlagsEnter);
		_vm->_zoneTrap = z;
	} else
	if (_vm->_zoneTrap) {
		_vm->setLocationFlags(kFlagsExit);
		_vm->_cmdExec->run(_vm->_zoneTrap->_commands, _vm->_zoneTrap);
		_vm->clearLocationFlags(kFlagsExit);
		_vm->_zoneTrap = nullZonePtr;
	}

}


void PathWalker_NS::finalizeWalk() {
	_engineFlags &= ~kEngineWalking;

	Common::Point foot;
	_ch->getFoot(foot);
	checkDoor(foot);

	_ch->_walkPath.clear();
}

void PathWalker_NS::walk() {
	if ((_engineFlags & kEngineWalking) == 0) {
		return;
	}

	Common::Point curPos;
	_ch->getFoot(curPos);

	// update target, if previous was reached
	PointList::iterator it = _ch->_walkPath.begin();
	if (it != _ch->_walkPath.end()) {
		if (*it == curPos) {
			debugC(1, kDebugWalk, "walk reached node (%i, %i)", (*it).x, (*it).y);
			it = _ch->_walkPath.erase(it);
		}
	}

	// advance character towards the target
	Common::Point targetPos;
	if (it == _ch->_walkPath.end()) {
		debugC(1, kDebugWalk, "walk reached last node");
		finalizeWalk();
		targetPos = curPos;
	} else {
		// targetPos is saved to help setting character direction
		targetPos = *it;

		Common::Point newPos(curPos);
		clipMove(newPos, targetPos);
		_ch->setFoot(newPos);

		if (newPos == curPos) {
			debugC(1, kDebugWalk, "walk was blocked by an unforeseen obstacle");
			finalizeWalk();
			targetPos = newPos;	// when walking is interrupted, targetPos must be hacked so that a still frame can be selected
		}
	}

	// targetPos is used to select the direction (and the walkFrame) of a character,
	// since it doesn't cause the sudden changes in orientation that newPos would.
	// Since newPos is 'adjusted' according to walkable areas, an imaginary line drawn
	// from curPos to newPos is prone to abrutply change in direction, thus making the
	// code select 'too different' frames when walking diagonally against obstacles,
	// and yielding an annoying shaking effect in the character.
	_ch->updateDirection(curPos, targetPos);
}



PathBuilder_NS::PathBuilder_NS(Character *ch) : PathBuilder(ch), _list(0) {
}


bool PathBuilder_BR::directPathExists(const Common::Point &from, const Common::Point &to) {

	Common::Point copy(from);
	Common::Point p(copy);

	while (p != to) {

		if (p.x < to.x && IS_PATH_CLEAR(p.x + 1, p.y)) p.x++;
		if (p.x > to.x && IS_PATH_CLEAR(p.x - 1, p.y)) p.x--;
		if (p.y < to.y && IS_PATH_CLEAR(p.x, p.y + 1)) p.y++;
		if (p.y > to.y && IS_PATH_CLEAR(p.x, p.y - 1)) p.y--;

		if (p == copy && p != to) {
			return false;
		}

		copy = p;
	}

	return true;
}

void PathBuilder_BR::buildPath(uint16 x, uint16 y) {
	Common::Point foot;
	_ch->getFoot(foot);

	debugC(1, kDebugWalk, "buildPath: from (%i, %i) to (%i, %i)", foot.x, foot.y, x, y);
	_ch->_walkPath.clear();

	// look for easy path first
	Common::Point dest(x, y);
	if (directPathExists(foot, dest)) {
		_ch->_walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: direct path found");
		return;
	}

	// look for short circuit cases
	ZonePtr z0 = _vm->hitZone(kZonePath, x, y);
	if (!z0) {
		_ch->_walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 0");
		return;
	}
	ZonePtr z1 = _vm->hitZone(kZonePath, foot.x, foot.y);
	if (!z1 || z1 == z0) {
		_ch->_walkPath.push_back(dest);
		debugC(3, kDebugWalk, "buildPath: corner case 1");
		return;
	}

	// build complex path
	int id = atoi(z0->_name);

	if (z1->u.path->_lists[id].empty()) {
		_ch->_walkPath.clear();
		debugC(3, kDebugWalk, "buildPath: no path");
		return;
	}

	PointList::iterator b = z1->u.path->_lists[id].begin();
	PointList::iterator e = z1->u.path->_lists[id].end();
	for ( ; b != e; b++) {
		_ch->_walkPath.push_front(*b);
	}
	_ch->_walkPath.push_back(dest);
	debugC(3, kDebugWalk, "buildPath: complex path");

	return;
}

PathBuilder_BR::PathBuilder_BR(Character *ch) : PathBuilder(ch) {
}

void PathWalker_BR::finalizeWalk() {
	_engineFlags &= ~kEngineWalking;
	_first = true;
	_fieldC = 1;

	Common::Point foot;
	_ch->getFoot(foot);

	ZonePtr z = _vm->hitZone(kZoneDoor, foot.x, foot.y);
	if (z && ((z->_flags & kFlagsClosed) == 0)) {
		_vm->_location._startPosition = z->u.door->_startPos; // foot pos
		_vm->_location._startFrame = z->u.door->_startFrame;

#if 0
		// TODO: implement working follower. Must find out a location in which the code is
		// used and which is stable enough.
		_followerFootInit.x = -1;
		if (_follower && z->u.door->startPos2.x != -1) {
			_followerFootInit.x = z->u.door->startPos2.x;	// foot pos
			_followerFootInit.y = z->u.door->startPos2.y;	// foot pos
		}
		_followerFootInit.z = -1;
		if (_follower && z->u.door->startPos2.z != -1) {
			_followerFootInit.z = z->u.door->startPos2.z;	// foot pos
		}
#endif

		_vm->scheduleLocationSwitch(z->u.door->_location);
		_vm->_cmdExec->run(z->_commands, z);
	}

#if 0
	// TODO: Input::walkTo must be extended to support destination frame in addition to coordinates
	// TODO: the frame argument must be passed to PathWalker through PathBuilder, so probably
	// a merge between the two Path managers is the right solution
	if (_engineFlags & FINAL_WALK_FRAME) {	// this flag is set in readInput()
		_engineFlags &= ~FINAL_WALK_FRAME;
		_char.ani->_frame = _moveToF; 	// from readInput()...
	} else {
		_char.ani->_frame = _dirFrame;	// from walk()
	}
	_char.setFoot(foot);
#endif

	_ch->_ani->setF(_dirFrame);	// temporary solution

#if 0
	// TODO: support scrolling ;)
	if (foot.x > _gfx->hscroll + 600) _gfx->scrollRight(78);
	if (foot.x < _gfx->hscroll + 40) _gfx->scrollLeft(78);
	if (foot.y > 350) _gfx->scrollDown(100);
	if (foot.y < 80) _gfx->scrollUp(100);
#endif

	return;
}


void PathWalker_BR::walk() {
	if ((_engineFlags & kEngineWalking) == 0) {
		return;
	}

#if 0
	// TODO: support delays in walking. This requires extending Input::walkIo().
	if (ch._walkDelay > 0) {
		ch._walkDelay--;
		if (ch._walkDelay == 0 && _ch._ani->_scriptName) {
			// stop script and reset
			_ch._ani->_flags &= ~kFlagsActing;
			Script *script = findScript(_ch._ani->_scriptName);
			script->_nextCommand = script->firstCommand;
		}
		return;
	}
#endif

	GfxObj *obj = _ch->_ani->gfxobj;

	Common::Rect rect;
	obj->getRect(_ch->_ani->getF(), rect);

	uint scale;
	if (rect.bottom > _vm->_location._zeta0) {
		scale = 100;
	} else
	if (rect.bottom < _vm->_location._zeta1) {
		scale = _vm->_location._zeta2;
	} else {
		scale = _vm->_location._zeta2 + ((rect.bottom - _vm->_location._zeta1) * (100 - _vm->_location._zeta2)) / (_vm->_location._zeta0 - _vm->_location._zeta1);
	}
	int xStep = (scale * 16) / 100 + 1;
	int yStep = (scale * 10) / 100 + 1;

	debugC(9, kDebugWalk, "calculated step: (%i, %i)\n", xStep, yStep);

	if (_fieldC == 0) {
		_ch->_walkPath.erase(_ch->_walkPath.begin());

		if (_ch->_walkPath.empty()) {
			finalizeWalk();
			debugC(3, kDebugWalk, "PathWalker_BR::walk, case 0\n");
			return;
		} else {
			debugC(3, kDebugWalk, "PathWalker_BR::walk, moving to next node\n");
		}
	}

	_ch->getFoot(_startFoot);

	_fieldC = 0;
	_step++;
	_step %= 8;


	int maxX = _vm->_gfx->_backgroundInfo->width;
	int minX = 0;
	int maxY = _vm->_gfx->_backgroundInfo->height;
	int minY = 0;

	int walkFrame = _step;
	_dirFrame = 0;
	Common::Point newpos(_startFoot), delta;

	Common::Point p(*_ch->_walkPath.begin());

	if (_startFoot.y < p.y && _startFoot.y < maxY && IS_PATH_CLEAR(_startFoot.x, yStep + _startFoot.y)) {
		if (yStep + _startFoot.y <= p.y) {
			_fieldC = 1;
			delta.y = yStep;
			newpos.y = yStep + _startFoot.y;
		} else {
			delta.y = p.y - _startFoot.y;
			newpos.y = p.y;
		}
		_dirFrame = 9;
	} else
	if (_startFoot.y > p.y && _startFoot.y > minY && IS_PATH_CLEAR(_startFoot.x, _startFoot.y - yStep)) {
		if (_startFoot.y - yStep >= p.y) {
			_fieldC = 1;
			delta.y = yStep;
			newpos.y = _startFoot.y - yStep;
		} else {
			delta.y = _startFoot.y - p.y;
			newpos.y = p.y;
		}
		_dirFrame = 0;
	}

	if (_startFoot.x < p.x && _startFoot.x < maxX && IS_PATH_CLEAR(_startFoot.x + xStep, _startFoot.y)) {
		if (_startFoot.x + xStep <= p.x) {
			_fieldC = 1;
			delta.x = xStep;
			newpos.x = xStep + _startFoot.x;
		} else {
			delta.x = p.x - _startFoot.x;
			newpos.x = p.x;
		}
		if (delta.y < delta.x) {
			_dirFrame = 18;	// right
		}
	} else
	if (_startFoot.x > p.x && _startFoot.x > minX && IS_PATH_CLEAR(_startFoot.x - xStep, _startFoot.y)) {
		if (_startFoot.x - xStep >= p.x) {
			_fieldC = 1;
			delta.x = xStep;
			newpos.x = _startFoot.x - xStep;
		} else {
			delta.x = _startFoot.x - p.x;
			newpos.x = p.x;
		}
		if (delta.y < delta.x) {
			_dirFrame = 27;	// left
		}
	}

	debugC(9, kDebugWalk, "foot (%i, %i) dest (%i, %i) deltas = %i/%i \n", _startFoot.x, _startFoot.y, p.x, p.y, delta.x, delta.y);

	if (_fieldC) {
		debugC(9, kDebugWalk, "PathWalker_BR::walk, foot moved from (%i, %i) to (%i, %i)\n", _startFoot.x, _startFoot.y, newpos.x, newpos.y);
		_ch->_ani->setF(walkFrame + _dirFrame + 1);
		_startFoot.x = newpos.x;
		_startFoot.y = newpos.y;
		_ch->setFoot(_startFoot);
		_ch->_ani->setZ(newpos.y);
	}

	if (_fieldC || !_ch->_walkPath.empty()) {
//		checkTrap();
		debugC(3, kDebugWalk, "PathWalker_BR::walk, case 1\n");
		return;
	}

	debugC(3, kDebugWalk, "PathWalker_BR::walk, case 2\n");
	finalizeWalk();
	return;
}

PathWalker_BR::PathWalker_BR(Character *ch) : PathWalker(ch), _fieldC(1), _first(true) {

}


} // namespace Parallaction
