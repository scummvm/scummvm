/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "colony/colony.h"
#include "colony/gfx.h"
#include "common/system.h"
#include "common/debug.h"

namespace Colony {

void ColonyEngine::interactWithObject(int objNum) {
	if (objNum <= 0 || objNum > (int)_objects.size())
		return;

	const Thing &obj = _objects[objNum - 1];
	if (!obj.alive)
		return;

	const int x = CLIP<int>(obj.where.xindex, 0, 30);
	const int y = CLIP<int>(obj.where.yindex, 0, 30);
	_action0 = _mapData[x][y][4][3];
	_action1 = _mapData[x][y][4][4];
	_creature = 1;

	switch (obj.type) {
	case kObjDesk:
		if (loadAnimation("desk"))
			playAnimation();
		break;
	case kObjConsole:
		switch (_action0) {
		case 1: // Reactor console
			if (loadAnimation("reactor"))
				playAnimation();
			break;
		case 2: // Main ship controls
			if (loadAnimation("controls"))
				playAnimation();
			break;
		case 3: // Security console
			if (loadAnimation("security"))
				playAnimation();
			break;
		default:
			inform("IT DOES NOT SEEM TO BE WORKING.", true);
			break;
		}
		break;
	case kObjProjector:
		switch (_action0) {
		case 1:
			if (loadAnimation("slides"))
				playAnimation();
			break;
		case 2:
			if (loadAnimation("teleshow")) // "teleshow" matches original interaction
				playAnimation();
			break;
		default:
			inform("PROJECTOR OFFLINE", true);
			break;
		}
		break;
	case kObjPowerSuit:
		if (loadAnimation("suit"))
			playAnimation();
		break;
	case kObjTeleport:
	{
		if (_fl == 1) {
			// In empty forklift  pick up the teleporter itself
			if (loadAnimation("lift")) {
				_animationResult = 0;
				playAnimation();
				if (_animationResult) {
					PassPatch from;
					from.level = _level;
					from.xindex = obj.where.xindex;
					from.yindex = obj.where.yindex;
					from.xloc = obj.where.xloc;
					from.yloc = obj.where.yloc;
					from.ang = obj.where.ang;
					_carryPatch[1].level = 100;
					_carryPatch[1].xindex = 1;
					_carryPatch[1].yindex = 1;
					_carryPatch[1].xloc = 1;
					_carryPatch[1].yloc = 1;
					_carryPatch[1].ang = 1;
					newPatch(kObjTeleport, from, _carryPatch[1], _mapData[from.xindex][from.yindex][4]);
					_carryType = kObjTeleport;
					_robotArray[obj.where.xindex][obj.where.yindex] = 0;
					_objects[objNum - 1].alive = 0;
					_fl = 2;
				}
			}
			break;
		}
		// fl==0 or fl==2: use the teleporter
		_sound->play(Sound::kTeleport);
		int targetLevelRaw = _mapData[x][y][4][2];
		int targetX = _action0;
		int targetY = _action1;
		// Check if this teleporter was relocated via patch
		PassPatch tp;
		tp.level = _level;
		tp.xindex = x;
		tp.yindex = y;
		uint8 patchData[5];
		if (patchMapTo(tp, patchData)) {
			targetLevelRaw = patchData[2];
			targetX = patchData[3];
			targetY = patchData[4];
		}
		const int targetLevel = (targetLevelRaw == 0) ? _level : targetLevelRaw;
		if (targetLevel >= 100 || targetX <= 0 || targetX >= 31 || targetY <= 0 || targetY >= 31) {
			inform("TELEPORTER INITIALIZATION FAILED", true);
			break;
		}
		if (targetLevel != _level)
			loadMap(targetLevel);
		const int oldX = _me.xindex;
		const int oldY = _me.yindex;
		_me.xindex = targetX;
		_me.yindex = targetY;
		_me.xloc = (targetX << 8) + 128;
		_me.yloc = (targetY << 8) + 128;
		_me.ang = _me.look;
		if (oldX >= 0 && oldX < 32 && oldY >= 0 && oldY < 32)
			_robotArray[oldX][oldY] = 0;
		if (_me.xindex >= 0 && _me.xindex < 32 && _me.yindex >= 0 && _me.yindex < 32)
			_robotArray[_me.xindex][_me.yindex] = kMeNum;
		break;
	}
	case kObjDrawer:
		if (loadAnimation("vanity"))
			playAnimation();
		break;
	case kObjScreen:
		// original game shows "Full of stars" effect/text
		_sound->play(Sound::kStars1);
		inform("I CAN SEE THROUGH IT...", true);
		break;

	case kObjToilet:
	case kObjPToilet:
		_sound->play(Sound::kToilet);
		inform("IT'S A TOILET.", true);
		break;
	case kObjTub:
		_sound->play(Sound::kBath);
		inform("A BATHTUB. NO TIME FOR A SOAK.", true);
		break;

	case kObjSink:
		_sound->play(Sound::kSink);
		inform("A SINK. IT'S DRY.", true);
		break;
	case kObjTV:
		if (_level == 1)
			doText(56, 0);
		else
			doText(16, 0);
		break;
	case kObjForkLift:
		if (_fl == 0) {
			// Enter forklift: play animation, if confirmed, patch it away
			if (loadAnimation("forklift")) {
				playAnimation();
				if (_animationResult) {
					PassPatch from;
					from.level = _level;
					from.xindex = obj.where.xindex;
					from.yindex = obj.where.yindex;
					from.xloc = obj.where.xloc;
					from.yloc = obj.where.yloc;
					from.ang = obj.where.ang;
					_carryPatch[0].level = 100;
					_carryPatch[0].xindex = 0;
					_carryPatch[0].yindex = 0;
					_carryPatch[0].xloc = 0;
					_carryPatch[0].yloc = 0;
					_carryPatch[0].ang = 0;
					newPatch(kObjForkLift, from, _carryPatch[0], _mapData[from.xindex][from.yindex][4]);
					_robotArray[obj.where.xindex][obj.where.yindex] = 0;
					_objects[objNum - 1].alive = 0;
					_me.look = _me.ang = obj.where.ang;
					_fl = 1;
				}
			}
		}
		break;
	case kObjBox1:
	case kObjBox2:
	case kObjCryo:
		if (_fl == 1) {
			// In empty forklift  pick up object
			if (loadAnimation("lift")) {
				_animationResult = 0;
				playAnimation();
				if (_animationResult) {
					PassPatch from;
					from.level = _level;
					from.xindex = obj.where.xindex;
					from.yindex = obj.where.yindex;
					from.xloc = obj.where.xloc;
					from.yloc = obj.where.yloc;
					from.ang = obj.where.ang;
					_carryPatch[1].level = 100;
					_carryPatch[1].xindex = 1;
					_carryPatch[1].yindex = 1;
					_carryPatch[1].xloc = 1;
					_carryPatch[1].yloc = 1;
					_carryPatch[1].ang = 1;
					newPatch(obj.type, from, _carryPatch[1], _mapData[from.xindex][from.yindex][4]);
					_carryType = obj.type;
					_robotArray[obj.where.xindex][obj.where.yindex] = 0;
					_objects[objNum - 1].alive = 0;
					_fl = 2;
				}
			}
		} else if (_fl == 0 && obj.type == kObjCryo) {
			// Not in forklift  read cryo text
			doText(_action0, 0);
		}
		break;
	case kObjReactor:
		if (_fl == 1 && _coreState[_coreIndex] == 1) {
			// Empty forklift at open reactor  pick up reactor core
			if (loadAnimation("lift")) {
				_animationResult = 0;
				playAnimation();
				if (_animationResult) {
					_carryType = kObjReactor;
					_corePower[2] = _corePower[_coreIndex];
					_corePower[_coreIndex] = 0;
					_coreState[_coreIndex] = 2;
					_fl = 2;
				}
			}
		} else if (_fl == 2 && _carryType == kObjReactor && _coreState[_coreIndex] == 2) {
			// Carrying reactor core  drop it into reactor
			if (loadAnimation("lift")) {
				_animationResult = 0;
				playAnimation();
				if (!_animationResult) {
					_carryType = 0;
					_coreState[_coreIndex] = 1;
					_corePower[_coreIndex] = _corePower[2];
					_fl = 1;
				}
			}
		}
		break;
	default:
		break;
	}
}

} // End of namespace Colony
