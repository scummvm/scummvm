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
#include "colony/renderer.h"
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

	// Original command.c: interaction is filtered by forklift state.
	// fl==1 (empty forklift): only CRYO/BOX1/BOX2/TELEPORT (pick up), REACTOR, SCREEN
	// fl==2 (carrying): only REACTOR (drop), TELEPORT (use), SCREEN; others → bonk
	if (_fl == 1) {
		if (obj.type != kObjBox1 && obj.type != kObjBox2 && obj.type != kObjCryo &&
		    obj.type != kObjTeleport && obj.type != kObjReactor && obj.type != kObjScreen)
			return; // silently ignore — original has no default for fl==1
	} else if (_fl == 2) {
		if (obj.type != kObjReactor && obj.type != kObjTeleport && obj.type != kObjScreen) {
			_sound->play(Sound::kBonk);
			return;
		}
	}

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
		// GANIMATE.C DoPowerSuit(): if(!corepower[coreindex])return;
		if (!_corePower[_coreIndex])
			break;
		if (loadAnimation("suit")) {
			// DOS DoPowerSuit: initialize switch positions from current armor/weapons
			setObjectState(1, _armor * 2 + 1);
			setObjectState(3, _armor + 1);
			setObjectState(2, _weapons * 2 + 1);
			setObjectState(4, _weapons + 1);
			playAnimation();
		}
		break;
	case kObjTeleport:
	{
		if (_fl == 1) {
			// In empty forklift  pick up the teleporter itself
			if (loadAnimation("lift")) {
				_sound->play(Sound::kLift); // GANIMATE.C DoLift: DoLiftSound()
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
					_me.lookY = 0;
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
				_sound->play(Sound::kLift); // GANIMATE.C DoLift: DoLiftSound()
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
				_sound->play(Sound::kLift); // GANIMATE.C DoLift: DoLiftSound()
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
				_sound->play(Sound::kDrop); // GANIMATE.C DoLift: DoDropSound()
				_animationResult = 0;
				playAnimation();
				if (_animationResult) {
					_carryType = 0;
					_coreState[_coreIndex] = 1;
					_corePower[_coreIndex] = _corePower[2];
					_fl = 1;
				}
			}
		}
		break;
	default:
		// IBM_COMM.C: DoBonkSound() for unhandled object interactions
		_sound->play(Sound::kBonk);
		break;
	}
}

// shoot.c SetPower(): adjust player's 3 power levels and update display.
// p0=weapons delta, p1=life delta, p2=armor delta.
void ColonyEngine::setPower(int p0, int p1, int p2) {
	const int32 oldPower0 = _me.power[0];
	const int32 oldPower1 = _me.power[1];
	const int32 oldPower2 = _me.power[2];

	_me.power[0] = MAX<int32>(_me.power[0] + p0, 0);
	_me.power[1] = MAX<int32>(_me.power[1] + p1, 0);
	_me.power[2] = MAX<int32>(_me.power[2] + p2, 0);

	if (p0 != 0 || p1 != 0 || p2 != 0) {
		debugC(1, kColonyDebugCombat,
			"setPower: delta=[%d,%d,%d] from=[%d,%d,%d] to=[%d,%d,%d] mode=%s level=%d pos=(%d,%d)",
			p0, p1, p2,
			(int)oldPower0, (int)oldPower1, (int)oldPower2,
			(int)_me.power[0], (int)_me.power[1], (int)_me.power[2],
			_gameMode == kModeBattle ? "battle" : "colony",
			_level, _me.xindex, _me.yindex);
	}

	updateDOSPowerBars();

	if (_me.power[1] <= 0) {
		debugC(1, kColonyDebugUI, "Player died! power=[%d,%d,%d]",
			(int)_me.power[0], (int)_me.power[1], (int)_me.power[2]);
		terminateGame(false);
	}
}

// shoot.c CShoot(): player fires weapon at the cursor or screen center.
// Original hit detection uses the rendered object bounds on screen.
void ColonyEngine::cShoot() {
	if (_gameMode == kModeBattle) {
		battleShoot();
		return;
	}

	// Original command.c: space bar disabled when in forklift
	if (_fl)
		return;

	if (_me.power[0] <= 0 || _weapons <= 0)
		return;

	_sound->play(Sound::kBang);

	// Draw shoot effect then drain power (matching original order)
	const Common::Point aim = getAimPoint();
	const int cx = aim.x;
	const int cy = aim.y;
	doShootCircles(cx, cy);

	// Drain weapons power: -(1 << level) per shot
	setPower(-(1 << _level), 0, 0);

	int bestIdx = -1;
	int bestDist = INT_MAX;
	bool bestIsBlocker = false;

	for (uint i = 0; i < _objects.size(); i++) {
		const Thing &obj = _objects[i];
		if (!obj.alive)
			continue;

		if (obj.where.xmn > obj.where.xmx || obj.where.zmn > obj.where.zmx)
			continue;
		if (obj.where.xmn > cx || obj.where.xmx < cx ||
		    obj.where.zmn > cy || obj.where.zmx < cy)
			continue;

		int t = obj.type;
		bool isRobot = (t >= kRobEye && t <= kRobUPyramid) ||
			t == kRobQueen || t == kRobDrone || t == kRobSoldier;
		bool blocksShot = (t == kObjForkLift || t == kObjTeleport || t == kObjPToilet ||
			t == kObjBox2 || t == kObjReactor || t == kObjScreen);
		if (!isRobot && !blocksShot)
			continue;

		int dx = obj.where.xloc - _me.xloc;
		int dy = obj.where.yloc - _me.yloc;
		int dist = (int)sqrtf((float)(dx * dx + dy * dy));

		if (dist < bestDist) {
			bestDist = dist;
			bestIdx = (int)i + 1; // 1-based robot index
			bestIsBlocker = blocksShot;
		}
	}

	if (bestIdx > 0 && !bestIsBlocker) {
		const Thing &target = _objects[bestIdx - 1];
		debugC(1, kColonyDebugAnimation, "CShoot: hit robot %d (type=%d, dist=%d)",
			bestIdx, target.type, bestDist);
		// shoot.c: s0 = rtable[dist]; InvertOval (Mac) / doBurnHole (DOS)
		int hitRadius = (bestDist > 0) ? CLIP<int>(160 * 128 / bestDist, 1, 100) : 50;
		doBurnHole(cx, cy, hitRadius);
		destroyRobot(bestIdx);
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

	// shoot.c: if not already morphing, force the robot into its firing pose.
	if (!obj.grow)
		obj.opcode = 4; // FSHOOT

	// Face robot towards player
	obj.where.look = obj.where.ang = (uint8)(_me.ang + 128);

	obj.where.power[1] -= damage;
	debugC(1, kColonyDebugAnimation, "DestroyRobot(%d): type=%d damage=%d remaining_hp=%d",
		num, obj.type, damage, (int)obj.where.power[1]);

	if (obj.where.power[1] <= 0) {
		if (obj.count != 0) {
			// Robot fully destroyed: remove from grid and recycle slot
			obj.alive = 0;
			int gx = obj.where.xindex;
			int gy = obj.where.yindex;
			if (gx >= 0 && gx < 32 && gy >= 0 && gy < 32) {
				if (obj.type > kRobUPyramid && obj.type < kRobQueen) {
					if (_foodArray[gx][gy] == num)
						_foodArray[gx][gy] = 0;
				} else if (_robotArray[gx][gy] == num) {
					_robotArray[gx][gy] = 0;
				}
			}
			_sound->play(Sound::kExplode);
			copyOverflowObjectToSlot(num);
			debugC(1, kColonyDebugCombat, "Robot %d destroyed!", num);
		} else {
			// Robot regresses to egg form
			obj.where.power[1] = 10 + ((_randomSource.getRandomNumber(15)) << _level);
			obj.grow = -1;
			obj.count = 0;
			debugC(1, kColonyDebugCombat, "Robot %d regressed to egg", num);
		}
	}
}

// SHOOT.C doShootCircles(): three V-shaped lines (red/white/red) from the
// bottom-left and bottom-right corners of the viewport converging at the aim
// point, plus a small filled oval at center. Simulates a rifle-barrel perspective.
void ColonyEngine::doShootCircles(int cx, int cy) {
	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	if (isMac) {
		// Mac shoot.c CShoot(): patXor diagonal lines radiating from aim point
		// using rtable[] perspective scaling with PenSize decreasing from 10 to 7.
		// Simulate thick lines by drawing multiple parallel offsets.
		auto thickXorLine = [this](int x1, int y1, int x2, int y2, int size) {
			int half = size / 2;
			for (int dy = -half; dy <= half; dy++) {
				for (int dx = -half; dx <= half; dx++) {
					_gfx->drawLine(x1 + dx, y1 + dy, x2 + dx, y2 + dy, 0xFFFFFF);
				}
			}
		};

		_gfx->setXorMode(true);
		int psize = 10;
		for (int i = 100; i < 900; i += 200) {
			const int s0 = CLIP<int>(160 * 128 / i, 0, 1000);
			const int s1 = CLIP<int>(160 * 128 / (i + 100), 0, 1000);
			// Four diagonal ray segments from outer to inner
			thickXorLine(cx - s0, cy - s0, cx - s1, cy - s1, psize);
			thickXorLine(cx - s0, cy + s0, cx - s1, cy + s1, psize);
			thickXorLine(cx + s0, cy + s0, cx + s1, cy + s1, psize);
			thickXorLine(cx + s0, cy - s0, cx + s1, cy - s1, psize);
			psize--;
		}
		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(30);
		// XOR again to erase
		psize = 10;
		for (int i = 100; i < 900; i += 200) {
			const int s0 = CLIP<int>(160 * 128 / i, 0, 1000);
			const int s1 = CLIP<int>(160 * 128 / (i + 100), 0, 1000);
			thickXorLine(cx - s0, cy - s0, cx - s1, cy - s1, psize);
			thickXorLine(cx - s0, cy + s0, cx - s1, cy + s1, psize);
			thickXorLine(cx + s0, cy + s0, cx + s1, cy + s1, psize);
			thickXorLine(cx + s0, cy - s0, cx + s1, cy - s1, psize);
			psize--;
		}
		_gfx->setXorMode(false);
	} else {
		// DOS SHOOT.C doShootCircles(): three converging V-lines from viewport
		// bottom corners to aim point (red/white/red) + center oval.
		_gfx->drawLine(_screenR.left + 1, _screenR.bottom - 1, cx, cy - 1, 4); // vRED
		_gfx->drawLine(cx, cy - 1, _screenR.right - 2, _screenR.bottom - 1, 4);

		_gfx->drawLine(_screenR.left, _screenR.bottom - 1, cx, cy, 15); // vINTWHITE
		_gfx->drawLine(cx, cy, _screenR.right - 1, _screenR.bottom - 1, 15);

		_gfx->drawLine(_screenR.left - 1, _screenR.bottom - 1, cx, cy + 1, 4); // vRED
		_gfx->drawLine(cx, cy + 1, _screenR.right, _screenR.bottom - 1, 4);

		// Small filled oval at aim point
		_gfx->fillEllipse(cx, cy, 2, 2, 4); // vRED

		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(30);
	}
}

// SHOOT.C doBurnHole(): expanding concentric random rays in blue/yellow/white
// when a robot is hit. Creates an "explosion" effect at the hit location.
void ColonyEngine::doBurnHole(int cx, int cy, int radius) {
	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	if (isMac) {
		// Mac: InvertOval at robot bounds
		_gfx->setXorMode(true);
		_gfx->fillEllipse(cx, cy, radius, radius, 0xFFFFFF);
		_gfx->setXorMode(false);
		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(50);
	} else {
		// DOS: expanding random rays in light blue → yellow → white
		const int ra = MIN(radius * 2, _pQx * 6);
		const int d = ra * 2;
		const int dd = d * 2;

		for (int i = 2; i < dd; i += 1 + (i >> 1)) {
			const int i2 = i >> 1;
			int i4 = i2 >> 1;
			const int i8 = i4 >> 1;
			const int i16 = i8 >> 1;
			if (i8 == 0)
				i4 = 1;

			// Light blue rays
			for (int k = 0; k < i16; k++) {
				int tx = (_randomSource.getRandomNumber(i2) - i4);
				int ty = (_randomSource.getRandomNumber(i2) - i4);
				if (ABS(tx) + ABS(ty) < i8 * 3)
					_gfx->drawLine(cx + tx, cy + ty, cx - tx, cy - ty, 9); // vLTBLUE
			}
			// Yellow rays
			for (int k = 0; k < i16; k++) {
				int tx = (_randomSource.getRandomNumber(i2) - i4);
				int ty = (_randomSource.getRandomNumber(i2) - i4);
				if (ABS(tx) + ABS(ty) < (i * 3) >> 3)
					_gfx->drawLine(cx + tx, cy + ty, cx - tx, cy - ty, 14); // vYELLOW
			}
		}
		// White center core
		const int i8 = dd >> 4;
		const int i2 = dd >> 1;
		for (int k = 0; k < i8; k++) {
			int tx = (_randomSource.getRandomNumber(i2) / 2 - (dd >> 4));
			int ty = (_randomSource.getRandomNumber(i2) / 2 - (dd >> 4));
			if (ABS(tx) + ABS(ty) < (dd * 3) >> 4)
				_gfx->drawLine(cx + tx, cy + ty, cx - tx, cy - ty, 15); // vINTWHITE
		}
		// Center dot
		_gfx->fillEllipse(cx, cy, _pQx / 8, _pQy / 8, 15);
		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(50);
	}
}

// SHOOT.C MeGetShot(): XOR'd X-shaped crosshairs scattered across the viewport
// when a robot shoots the player. Four passes of 40 X-marks each.
void ColonyEngine::meGetShot() {
	const int vw = _screenR.width();
	const int vh = _screenR.height();
	if (vw <= 0 || vh <= 0)
		return;

	const bool isMac = (_renderMode == Common::kRenderMacintosh);

	if (isMac) {
		// Mac shoot.c: InvertRect(&Clip) — full viewport flash
		_gfx->setXorMode(true);
		_gfx->fillRect(_screenR, 0xFFFFFF);
		_gfx->setXorMode(false);
		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(30);
		// Erase flash
		_gfx->setXorMode(true);
		_gfx->fillRect(_screenR, 0xFFFFFF);
		_gfx->setXorMode(false);
	} else {
		// DOS SHOOT.C MeGetShot(): 4 passes × 40 XOR'd X-marks at random positions
		const int qx = _pQx;
		const int qy = _pQy >> 1;
		const int qx5 = (_pQx >> 2) * 5;

		_gfx->setXorMode(true);
		for (int pass = 0; pass < 4; pass++) {
			for (int i = 0; i < 40; i++) {
				int x, y;
				if (pass & 1) {
					// Passes 1,3: centered half-region
					x = _randomSource.getRandomNumber(vw / 2) + (vw >> 2);
					y = _randomSource.getRandomNumber(vh / 2) + (vh >> 2);
				} else {
					// Passes 0,2: full region
					x = _randomSource.getRandomNumber(vw);
					y = _randomSource.getRandomNumber(vh);
				}
				x += _screenR.left;
				y += _screenR.top;
				// Draw X-shaped crosshair (3 lines: two diagonals + horizontal)
				_gfx->drawLine(x - qx, y - qy, x + qx, y + qy, 15);
				_gfx->drawLine(x - qx5, y, x + qx5, y, 15);
				_gfx->drawLine(x - qx, y + qy, x + qx, y - qy, 15);
			}
		}
		_gfx->setXorMode(false);
		_gfx->copyToScreen();
		_system->updateScreen();
		_system->delayMillis(50);
	}
}

} // End of namespace Colony
