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
#include "colony/sound.h"
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

// shoot.c SetPower(): adjust player's 3 power levels and update display.
// p0=weapons delta, p1=life delta, p2=armor delta.
void ColonyEngine::setPower(int p0, int p1, int p2) {
	_me.power[0] = MAX<int32>(_me.power[0] + p0, 0);
	_me.power[1] = MAX<int32>(_me.power[1] + p1, 0);
	_me.power[2] = MAX<int32>(_me.power[2] + p2, 0);

	if (_me.power[1] <= 0) {
		debugC(1, kColonyDebugUI, "Player died! power=[%d,%d,%d]",
			(int)_me.power[0], (int)_me.power[1], (int)_me.power[2]);
		terminateGame(false);
	}
}

// shoot.c CShoot(): player fires weapon at screen center.
// Traces a ray in the facing direction to find the first robot hit.
void ColonyEngine::cShoot() {
	if (_gameMode == kModeBattle) {
		battleShoot();
		return;
	}

	if (_me.power[0] <= 0 || _weapons <= 0)
		return;

	_sound->play(Sound::kBang);

	// Drain weapons power: -(1 << level) per shot
	setPower(-(1 << _level), 0, 0);

	// Draw crosshair flash via XOR lines on the 3D viewport
	int cx = _screenR.left + _screenR.width() / 2;
	int cy = _screenR.top + _screenR.height() / 2;
	_gfx->setXorMode(true);
	for (int r = 4; r <= 20; r += 4) {
		_gfx->drawLine(cx - r, cy - r, cx + r, cy - r, 0xFFFFFF);
		_gfx->drawLine(cx + r, cy - r, cx + r, cy + r, 0xFFFFFF);
		_gfx->drawLine(cx + r, cy + r, cx - r, cy + r, 0xFFFFFF);
		_gfx->drawLine(cx - r, cy + r, cx - r, cy - r, 0xFFFFFF);
	}
	_gfx->copyToScreen();
	_system->updateScreen();
	_system->delayMillis(30);
	// XOR again to erase
	for (int r = 4; r <= 20; r += 4) {
		_gfx->drawLine(cx - r, cy - r, cx + r, cy - r, 0xFFFFFF);
		_gfx->drawLine(cx + r, cy - r, cx + r, cy + r, 0xFFFFFF);
		_gfx->drawLine(cx + r, cy + r, cx - r, cy + r, 0xFFFFFF);
		_gfx->drawLine(cx - r, cy + r, cx - r, cy - r, 0xFFFFFF);
	}
	_gfx->setXorMode(false);

	// Hit detection: find the closest visible robot in the player's aim direction.
	// For each visible robot, compute the angle from the player to the robot and
	// compare with the player's look direction. Pick the nearest matching robot.
	int bestIdx = -1;
	int bestDist = INT_MAX;

	for (uint i = 0; i < _objects.size(); i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive)
			continue;
		int t = obj.type;
		// Skip non-shootable types: only robots and boss types are valid targets
		bool isRobot = (t >= kRobEye && t <= kRobUPyramid) ||
			t == kRobQueen || t == kRobDrone || t == kRobSoldier;
		if (!isRobot)
			continue;

		int ox = obj.where.xindex;
		int oy = obj.where.yindex;
		if (ox < 0 || ox >= 31 || oy < 0 || oy >= 31 || !_visibleCell[ox][oy])
			continue;

		// Compute angle from player to robot (256-unit circle matching look direction)
		int dx = obj.where.xloc - _me.xloc;
		int dy = obj.where.yloc - _me.yloc;
		int dist = (int)sqrtf((float)(dx * dx + dy * dy));
		if (dist < 64)
			continue; // too close (same cell)

		// atan2 → 256-unit angle
		float rad = atan2f((float)dy, (float)dx);
		int angleToRobot = (int)(rad * 128.0f / (float)M_PI) & 0xFF;
		int angleDiff = (int8)((uint8)angleToRobot - _me.look);

		// Angular tolerance scales with distance: closer = wider cone
		int threshold = CLIP(3000 / MAX(dist, 1), 2, 16);
		if (abs(angleDiff) > threshold)
			continue;

		if (dist < bestDist) {
			bestDist = dist;
			bestIdx = (int)i + 1; // 1-based robot index
		}
	}

	if (bestIdx > 0) {
		// Check that the shot isn't blocked by a large object closer than the target
		const Thing &target = _objects[bestIdx - 1];
		bool blocked = false;
		for (uint i = 0; i < _objects.size(); i++) {
			const Thing &obj = _objects[i];
			if (!obj.alive || (int)i + 1 == bestIdx)
				continue;
			int t = obj.type;
			// These objects block shots
			if (t == kObjForkLift || t == kObjTeleport || t == kObjPToilet ||
				t == kObjBox2 || t == kObjReactor || t == kObjScreen) {
				int dx = obj.where.xloc - _me.xloc;
				int dy = obj.where.yloc - _me.yloc;
				int objDist = (int)sqrtf((float)(dx * dx + dy * dy));
				if (objDist < bestDist) {
					float rad = atan2f((float)dy, (float)dx);
					int angleToObj = (int)(rad * 128.0f / (float)M_PI) & 0xFF;
					int angleDiff = (int8)((uint8)angleToObj - _me.look);
					int threshold = CLIP(3000 / MAX(objDist, 1), 2, 16);
					if (abs(angleDiff) <= threshold) {
						blocked = true;
						break;
					}
				}
			}
		}

		if (!blocked) {
			debugC(1, kColonyDebugAnimation, "CShoot: hit robot %d (type=%d, dist=%d)",
				bestIdx, target.type, bestDist);
			destroyRobot(bestIdx);
		}
	}
}

// shoot.c DestroyRobot(): player damages a robot.
// Damage = (epower[0] * weapons^2) << 1. Robot dies when power[1] <= 0.
void ColonyEngine::destroyRobot(int num) {
	if (num <= 0 || num > (int)_objects.size())
		return;

	Thing &obj = _objects[num - 1];
	if (!obj.alive)
		return;

	auto qlog = [](int32 x) -> int {
		int i = 0;
		while (x > 0) { x >>= 1; i++; }
		return i;
	};

	int epower0 = qlog(_me.power[0]);
	int weapons2 = _weapons * _weapons;
	int damage = (epower0 * weapons2) << 1;

	// Face robot towards player
	obj.where.look = obj.where.ang = (uint8)(_me.ang + 128);

	obj.where.power[1] -= damage;
	debugC(1, kColonyDebugAnimation, "DestroyRobot(%d): type=%d damage=%d remaining_hp=%d",
		num, obj.type, damage, (int)obj.where.power[1]);

	if (obj.where.power[1] <= 0) {
		if (obj.type == kRobQueen) {
			obj.alive = 0;
			const int gx = obj.where.xindex;
			const int gy = obj.where.yindex;
			if (gx >= 0 && gx < 32 && gy >= 0 && gy < 32)
				_robotArray[gx][gy] = 0;
			if (_level >= 1 && _level <= 7) {
				_levelData[_level - 1].visit = 1;
				_levelData[_level - 1].queen = 0;
			}
			_sound->play(Sound::kExplode);
			debugC(1, kColonyDebugAnimation, "Queen destroyed on level %d", _level);
		} else if (obj.count != 0) {
			// Robot fully destroyed: remove and drop egg
			obj.alive = 0;
			int gx = obj.where.xindex;
			int gy = obj.where.yindex;
			if (gx >= 0 && gx < 32 && gy >= 0 && gy < 32)
				_robotArray[gx][gy] = 0;
			_sound->play(Sound::kExplode);
			debugC(1, kColonyDebugAnimation, "Robot %d destroyed!", num);
		} else {
			// Robot regresses to egg form
			obj.where.power[1] = 10 + ((_randomSource.getRandomNumber(15)) << _level);
			obj.grow = -1;
			obj.count = 0;
			debugC(1, kColonyDebugAnimation, "Robot %d regressed to egg", num);
		}
	}
}

} // End of namespace Colony
