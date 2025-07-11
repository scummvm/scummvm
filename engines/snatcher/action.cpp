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


#include "snatcher/action.h"
#include "snatcher/graphics.h"
#include "snatcher/mem_mapping.h"
#include "snatcher/resource.h"
#include "snatcher/snatcher.h"
#include "snatcher/sound.h"
#include "snatcher/util.h"
#include "common/endian.h"


namespace Snatcher {

ActionSequenceHandler::ActionSequenceHandler(SnatcherEngine *vm, ResourcePointer *scd, bool palTiming) : _vm(vm), _scd(scd), _palTiming(palTiming), _stage(0), _counter1(0), _counter2(0), _progressMSB(0), _progressLSB(0),
	_weaponState(0), _enemySpeed(0), _active(false), _handleShotPhase(0), _updateEnemiesPhase(0), _enemyAttackStatus(0), _targetField(0), _hscrollB(0), _vscrollB(0),
		_enemyAppearanceCount(0), _civilianAppearanceCount(0), _abortFight(0), _nextEnemyAppearanceTimer(0), _enemyAttackTimer(0), _shoot_78EE(0), _activeEnemies(0), _result(),
			_attackerId(0), _animNo(0), _shoot_7912(0), _projectileCoords(nullptr) {
	_projectileCoords = new int16[18]();
}

ActionSequenceHandler::~ActionSequenceHandler() {
	delete[] _projectileCoords;
}

#define animSet(x, y, z) \
	_vm->gfx()->setAnimParameter(x, GraphicsEngine::kAnimPara##y, z)
#define animGet(x, y) \
	_vm->gfx()->getAnimParameter(x, GraphicsEngine::kAnimPara##y)

uint16 fromPAL(uint16 value) {
	// Convert the PAL value to NTSC equivalent. The original game's VINT interrupt obviously runs at a different speed for PAL versions, so the
	// timer tables have been adjusted to produce the same result. We just convert the values from the tables so that they match the NTSC version.
	uint32 clc = value * 0x3C0000 / 0x32;
	return (clc >> 16) + ((clc & 0xFFFF) ? 1 : 0);
}

bool ActionSequenceHandler::run(bool useLightGun) {
	_useLightGun = useLightGun;
	switch (_progressMSB) {
	case 0:
		switch (_progressLSB) {
		case 0:
			++_progressLSB;
			_counter1 = 0;
			_vm->gfx()->enqueuePaletteEvent(_scd->makePtr(MemMapping::MEM_PALDATA_02));
			break;
		case 1:
			if (++_counter1 < 2)
				break;
			_counter1 = 0;
			_vm->gfx()->enqueuePaletteEvent(_scd->makePtr(MemMapping::MEM_PALDATA_00));
			if (_stage == 0 || _stage == 11) {
				_progressLSB += 2;
			} else {
				_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_GFXDATA_02));
				if (_stage != 5) {
					_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_GFXDATA_01));
					_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_GFXDATA_03));
				}
				++_progressLSB;
			}
			break;
		case 2:
			//hscrollB
			//vscrollB
			++_progressLSB;
			break;
		case 3:
			animSet(48, Enable, 0);
			animSet(48, ControlFlags, GraphicsEngine::kAnimHide | GraphicsEngine::kAnimPause);
			animSet(48, ActionTimer, 6);
			resetVars();
			animSet(21, Enable, 0);
			animSet(21, ControlFlags, GraphicsEngine::kAnimNone);
			animSet(21, Target, 0xFF);
			animSet(24, Enable, 0);
			animSet(24, ControlFlags, GraphicsEngine::kAnimNone);
			animSet(22, Enable, 0);
			animSet(22, ControlFlags, GraphicsEngine::kAnimNone);
			animSet(23, Enable, 0);
			animSet(23, ControlFlags, GraphicsEngine::kAnimNone);
			_counter2 = 0;
			_abortFight = 0;
			initStage();

			_nextEnemyAppearanceTimer = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_05)[_stage];
			if (_palTiming)
				_nextEnemyAppearanceTimer = fromPAL(_nextEnemyAppearanceTimer);
			_progressMSB = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_04)[_stage];
			_progressLSB = 0;

			if (_enemySpeed == 0) {
				_enemySpeed = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_06)[_stage];
				if (_palTiming)
					_enemySpeed = fromPAL(_enemySpeed);
			}
			break;

		default:
			break;
		}
		break;

	case 1:
		switch (_handleShotPhase) {
		case 0:
			animSet(56, ControlFlags, GraphicsEngine::kAnimPause);
			animSet(57, ControlFlags, GraphicsEngine::kAnimPause);
			animSet(56, Frame, 1);
			animSet(57, Frame, 1);
			_enemyAttackTimer = 128;
			++_handleShotPhase;
			break;
		case 1:
			if (_enemyAttackTimer == 0) {
				++_handleShotPhase;
			} else {
				if (!(_enemyAttackTimer & 0x3F))
					_vm->sound()->fmSendCommand(17, 0);
				--_enemyAttackTimer;
			}
			break;
		case 2:
			animSet(56, Enable, 0);
			animSet(57, Enable, 0);
			_vm->sound()->fmSendCommand(18, 0);
			_handleShotPhase = 0;
			++_progressMSB;
			break;
		default:
			break;
		}
		break;

	case 2:
		handleWeaponDrawAndTargetSelection();
		handleShot();
		if (_stage == 14) {
			if (_shoot_78EE) {
				animSet(22, PosX, 112);
				animSet(22, PosY, 56);
			}
			if (++_counter2 >= 256) {
				++_progressMSB;
				_progressLSB = 0;
			}
		} else {
			updateEnemies();
			updateStage();
			enemyAttack();
			checkFinished();
		}
		break;

	case 3:
		if (_civilianAppearanceCount && _civilianAppearanceCount == _result.civiliansShot)
			_result.allCiviliansShot = 1;
		if (_stage != 0 && _stage != 11)
			_vm->gfx()->enqueuePaletteEvent(_scd->makePtr(_result.hitsTaken < 6 ? MemMapping::MEM_PALDATA_04 : MemMapping::MEM_PALDATA_03));
		animSet(22, Enable, 0);
		animSet(23, Enable, 0);
		_vm->allowLightGunInput(false);
		_counter1 = 0;
		_vm->sound()->fmSendCommand(0xFF, 0);
		++_progressMSB;
		break;

	case 4:
		switch (_progressLSB) {
		case 0:
			if (_vm->sound()->pcmGetStatus().statusBits & 8)
				break;
			if ((_scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_11 : MemMapping::MEM_ACTIONSEQDATA_10)[_stage] == 1) || (++_counter1 >= 64)) {
					++_progressLSB;
			} else if (_result.hitsTaken < 6) {
				handleWeaponDrawAndTargetSelection();
				handleShot();
			}
			break;
		case 1:
			++_progressLSB;
			animSet(21, Enable, 0);
			_vm->gfx()->reset(GraphicsEngine::kResetPalEvents);
			break;
		case 2:
			++_progressLSB;
			_counter1 = 0;
			_vm->gfx()->enqueuePaletteEvent(_scd->makePtr(MemMapping::MEM_PALDATA_02));
			break;
		case 3:
			if (_stage != 0 && _stage != 11) {
				if (++_counter1 < 2)
					break;
				_counter1 = 0;
				_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_GFXDATA_04));
			}
			++_progressLSB;
			break;
		case 4:
			if (++_counter1 < 2)
				break;
			_counter1 = 0;
			++_progressMSB;
			_progressLSB = 0;
			_vm->gfx()->enqueuePaletteEvent(_scd->makePtr(MemMapping::MEM_PALDATA_01));
			break;
		default:
			break;
		}
		break;

	case 5:
		if (++_counter1 < 2)
			break;
		_weaponState = _enemySpeed = _progressMSB = 0;
		_vm->allowLightGunInput(false);
		_active = false;
		break;

	default:
		break;
	}

	return _active;
}

bool ActionSequenceHandler::resetScore() {
	if (_vm->gfx()->frameCount() & 1)
		return true;

	int16 a = 0;
	int16 b = 0;
	bool res = true;

	switch(_progressMSB) {
	case 0:
		a = animGet(58, Frame);
		if (a > 0) {
			animSet(58, Frame, --a);
			b = animGet(59, Frame);
			if (b > a)
				animSet(59, Frame, --b);
		} else {
			++_progressMSB;
		}
		break;
	case 1:
		animSet(58, ControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		animSet(59, ControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
		animSet(58, Frame, 0);
		animSet(59, Frame, 0);
		++_progressMSB;
		break;
	default:
		_progressMSB = 0;
		res = false;
		break;
	}

	return res;
}

void ActionSequenceHandler::setup(uint16 stage) {
	_stage = stage;
	_counter1 = 0;
	_counter2 = 0;
	_progressMSB = 0;
	_progressLSB = 0;
	_active = true;
}

void ActionSequenceHandler::setDifficulty(uint16 setting) {
	_enemySpeed = setting;
}

void ActionSequenceHandler::resetVars() {
	_weaponState = 0;
	_handleShotPhase = 0;
	_updateEnemiesPhase = _enemyAttackStatus = _targetField = 0;
	_hscrollB = _vscrollB = 0;
	_result = Result();
	_enemyAppearanceCount = _civilianAppearanceCount = 0;
	_shoot_7912 = _attackerId = 0;
	_animNo = 0;
}

void ActionSequenceHandler::initStage() {
	uint8 e = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage];
	for (int i = 0; i < 9; ++i) {
		animSet(32 + i, Frame, 2);
		animSet(32 + i, FrameDelay, 0);
		animSet(32 + i, Phase, 0);
		animSet(32 + i, Enable, e);
	}
	const int16 *in = reinterpret_cast<const int16*>(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_15)());
	for (int i = 0; i < 18; ++i)
		_projectileCoords[i] = FROM_BE_16(*in++);

	switch (_stage) {
	case 2:
		_projectileCoords[1] = _projectileCoords[4] = 72;
		_projectileCoords[10] = 8;
		_projectileCoords[13] = 35;
		break;
	case 5:
		_projectileCoords[1] = 93;
		_projectileCoords[10] = 54;
		break;
	case 6:
		_projectileCoords[6] = 38;
		_projectileCoords[15] = 72;
		break;
	case 7:
		_projectileCoords[5] = 150;
		_projectileCoords[14] = 48;
		break;
	case 9:
		_projectileCoords[1] = 96;
		_projectileCoords[10] = 0;
		break;
	case 10:
		_projectileCoords[9] = 0;
		_projectileCoords[11] = 0;
		_projectileCoords[15] = 88;
		_projectileCoords[17] = 88;
		break;
	case 13:
		_projectileCoords[9] = 24;
		_projectileCoords[11] = 24;
		_projectileCoords[12] = 40;
		_projectileCoords[4] = 110;
		_projectileCoords[13] = 39;
		_projectileCoords[14] = 40;
		break;
	default:
		break;
	}
}

void ActionSequenceHandler::handleWeaponDrawAndTargetSelection() {
	if (_weaponState < 2) {
		animSet(23, PosX, 0);
		animSet(23, PosY, 0);
		if (_abortFight || _progressMSB >= 4)
			return;

		uint16 in = _vm->input().singleFrameControllerFlagsRemapped;

		if ((!_useLightGun && (in & 0x40)) || (_useLightGun && (in & 0x200))) {
			_vm->allowLightGunInput(false);
			_weaponState ^= 1;
			int16 enableAnim = 0;
			if (_weaponState) {
				_vm->sound()->pcmSendCommand(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_01)[_stage], -1);
				_result.weaponDrawn = enableAnim = 1;
				if (_useLightGun) {
					_vm->allowLightGunInput(true);
					animSet(22, Enable, 0);
				}
			}
			if (!_weaponState || !_useLightGun) {
				animSet(22, Enable, enableAnim);
				animSet(22, PosX, 112);
				animSet(22, PosY, 56);
				_targetField = 4;
			}
			animSet(23, Enable, enableAnim);
		}

		if (!_weaponState || _useLightGun || _shoot_78EE)
			return;

		in = _vm->input().sustainedControllerFlagsRemapped;
		uint8 horz = 0;
		uint8 vert = 0;
		_targetField = 4;

		if (in & 8) {
			horz += 2;
			_targetField += 1;
		}
		if (in & 4)
			_targetField -= 1;
		else
			horz += 2;
		if (in & 2) {
			vert += 2;
			_targetField += 3;
		}
		if (in & 1)
			_targetField -= 3;
		else
			vert += 2;

		animSet(22, PosX, (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_13) + horz).readSINT16());
		animSet(22, PosY, (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_14) + vert).readSINT16());

	} else {
		if (_weaponState & 2) {
			int16 r = Util::rngMakeNumber() & 0x0F;
			animSet(23, PosX, r - 8);
			animSet(22, PosY, animGet(22, PosY) + r - 8);
			r = Util::rngMakeNumber() & 0x0F;
			animSet(23, PosY, r - 8);
			animSet(22, PosX, animGet(22, PosX) + r - 8);
			_enemyAttackTimer = (_enemySpeed >> 2) + 24;
		}
		_weaponState -= 2;
	}
}

void ActionSequenceHandler::handleShot() {
	if (_useLightGun) {
		// Weirdly, during the calibration, the original substracts an y-offset, but not here. When testing with an
		// emulator, the aiming is not satisfactory either, so I guess this is either an original bug or it really
		// does behave differently with a real light gun (y-coords aren't that easy to get there, depending on the
		// TV, the distance from the TV etc.).
		const int16 yOffs = -24;

		int16 v = animGet(21, ActionTimer);
		if (v && ++v >= 2) {
			v = 0;
			_targetField = 0;
			if (_vm->input().lightGunPos.x >= 160)
				_targetField = 2;
			else if (_vm->input().lightGunPos.x >= 96)
				_targetField = 1;
			if (_vm->input().lightGunPos.y + yOffs >= 88)
				_targetField += 6;
			else if (_vm->input().lightGunPos.y + yOffs>= 48)
				_targetField += 3;
		}
		animSet(21, ActionTimer, v);
	}

	uint16 in = _vm->input().singleFrameControllerFlagsRemapped;
	int16 tf = (_stage == 14) ? 4 : _targetField;
	int16 vl = 0;

	for (bool loop = true; loop;) {
		loop = false;
		switch (_handleShotPhase) {
		case 0:
			if (animGet(21, Enable) || (animGet(21, Target) != 0xFF) || _progressMSB == 4 || _weaponState != 1)
				break;
			if ((!_useLightGun && ((in & 0x40) || !(in & 0x20))) || (_useLightGun && ((in & 0x200) || !(in & 0x100))))
				break;
			if (_stage == 14)
				_shoot_78EE = 1;
			++_result.shotsFired;
			_handleShotPhase = 4;
			if (_useLightGun)
				animSet(21, ActionTimer, 1);
			break;

		case 1:
			animSet(21, Enable, 1);
			animSet(21, FrameDelay, 2);
			animSet(21, Frame, 0);
			animSet(21, Target, tf);
			animSet(21, PosX, (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_15) + (tf << 1)).readSINT16());
			animSet(21, PosY, (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_16) + (tf << 1)).readSINT16());
			_vm->sound()->pcmSendCommand(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_02)[_stage], -1);
			++_handleShotPhase;
			break;

		case 2:
			if (animGet(21, Frame) >= 4) {
				if (_stage == 14) {
					_result.civiliansShot = 1;
					_progressMSB = 3;
					_progressLSB = 0;
					_vm->sound()->pcmSendCommand(71, -1);
				} else {
					++_handleShotPhase;
				}
			}
			break;

		case 3:
			tf = animGet(21, Target);
			vl = animGet(32 + tf, Phase) & 0x0F;
			if (vl < 2 || vl > 4) {
				if (_stage != 10) {
					if (_useLightGun || _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] == 0) {
						if (_enemySpeed < ((_stage == 0 || _stage == 11 || !_useLightGun) ? 32 : 48))
								++_enemySpeed;
					}

					if (++_result.shotsMissed == 0)
						_result.shotsMissed = 0xFF;
				}

			} else {
				vl = animGet(32 + tf, Phase) & 0xF0;
				animSet(32 + tf, Phase, vl | 6);
				if ((vl >> 4) & 1) {
					if ((_result.enemiesShot & 7) == 7 && _enemySpeed >= 2)
						_enemySpeed -= 2;
					++_result.enemiesShot;

					if (_stage == 0 || _stage == 11) {
						animSet(59, Enable, 1);
						animSet(59, ControlFlags, GraphicsEngine::kAnimPause);
						animSet(59, Frame, animGet(59, Frame) + 1);
					}
					_vm->sound()->pcmSendCommand(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_00)[_stage], -1);

				} else {
					++_result.civiliansShot;
					if (_enemySpeed >= 2)
						_enemySpeed -= 2;
					if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] == 0)
						_vm->sound()->pcmSendCommand(14, -1);
				}

				if (_stage == 9) {
					_vm->allowLightGunInput(false);
					_abortFight = 1;
				}
			}
			if (_stage == 10) {
				if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_19)[tf] & 2) {
					_vm->sound()->pcmSendCommand(50, -1);
					_vm->sound()->fmSendCommand(19, 0);
					if (_result.hitsTaken < 6) {
						vl = animGet(48, ActionTimer) - 1;
						animSet(48, ActionTimer, vl);
						(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_20) + 6).writeUINT16((_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_21) + (vl << 1)).readUINT16());
						_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_20));
						++_result.hitsTaken;
					}

				} else {
					if (++_result.shotsMissed == 0)
						_result.shotsMissed = 0xFF;
				}
			}

			animSet(21, Target, 0xFF);
			animSet(21, Enable, 0);
			_handleShotPhase = 0;

			break;

		case 4:
			++_handleShotPhase;
			if (_enemySpeed < 16)
				loop = true;
			break;

		case 5:
			_handleShotPhase = 1;
			if (_enemySpeed < 4)
				loop = true;
			break;

		default:
			break;

		}
	}
}

void ActionSequenceHandler::updateEnemies() {
	_activeEnemies = 0;
	for (int i = 32; i < 41; ++i) {
		int16 a = animGet(i, Phase);
		int16 b = 0;
		if (a != 0xFF)
			b = a & 0x0F;
		if (b)
			++_activeEnemies;
		switch (b) {
		case 0:
			b = _vm->gfx()->getAnimScriptByte(i, 0);
			if (b == 2) {
				animSet(i, ControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			} else if (b < 128 && a != 0xFF) {
				animSet(i, Enable, 1);
			} else {
				animSet(i, Phase, 0xFF);
				animSet(i, ControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
				animSet(i, Enable, 0);
			}
			break;

		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if (b == 1) {
				animSet(i, ControlFlags, GraphicsEngine::kAnimNone);
			} else if (b < 5) {
				a = animGet(i, ActionTimer) - 1;
				animSet(i, ActionTimer, a);
			}
			if (b > 1 && b < 5 && a >= 0)
				break;

			a = _vm->gfx()->getAnimScriptByte(i, b);
			if (a == animGet(i, Frame)) {
				animSet(i, Frame, a + 2);
				animSet(i, Phase, animGet(i, Phase) + 1);
				a = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] << 1;
				animSet(i, ActionTimer, a + _enemySpeed);
			}

			if (b == 1)
				break;

			if ((animGet(i, Phase) & 0x0F) == b) {
				if (b < 5)
					animSet(i, ActionTimer, 0);
				break;
			}

			if (b == 5) {
				animSet(i, Frame, animGet(i, Frame) - 2);
				if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] == 0 && animGet(i, Phase) >= 48)
					++_result.hitsTaken;
				animSet(i, Phase, 0);
				break;
			}

			if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] == 0)
				break;

			if (b == 2 && (_result.enemiesShot <= _scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_08 : MemMapping::MEM_ACTIONSEQDATA_07)[_stage]))
				break;

			if (b == 4 && !(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] & 0x80) && ((_result.enemiesShot <= _scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_08 : MemMapping::MEM_ACTIONSEQDATA_07)[_stage]) || (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] >= 6)))
				break;

			if (b == 3) {
				if ((animGet(i, Phase) & 0xF0) != 0x20 && !_enemyAttackStatus && !_enemyAttackTimer && _weaponState < 2) {
					_attackerId = i - 32;
					_enemyAttackStatus = 1;
				}
			} else {
				animSet(i, Phase, (animGet(i, Phase) & 0xF0) + 7 - b);
				animSet(i, Frame, _vm->gfx()->getAnimScriptByte(i, 6 - b) + 2);
				a = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] << 1;
				animSet(i, ActionTimer, a + _enemySpeed);
			}
			break;

		case 6:
			animSet(i, Phase, (animGet(i, Phase) & 0xF0) + 7);
			animSet(i, Frame, _vm->gfx()->getAnimScriptByte(i, 6));
			a = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] << 1;
			animSet(i, ActionTimer, a + _enemySpeed);
			break;

		case 7:
			a = _vm->gfx()->getAnimScriptByte(i, b);
			if (a == animGet(i, Frame)) {
				animSet(i, Frame, a + 2);
				animSet(i, Phase, animGet(i, Phase) + 1);
				a = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] << 1;
				animSet(i, ActionTimer, a + _enemySpeed);
			}

			if ((animGet(i, Phase) & 0x0F) != 8)
				break;

			animSet(i, Frame, animGet(i, Frame) - 2);
			animSet(i, Phase, 0);

			if (_stage != 0 && _stage != 11 && _stage != 7)
				animSet(i, ControlFlags, GraphicsEngine::kAnimPause | GraphicsEngine::kAnimHide);
			break;

		default:
			break;
		}
	}
}
void ActionSequenceHandler::updateStage() {
	if (_updateEnemiesPhase == 0) {
		if (--_nextEnemyAppearanceTimer >= 0)
			return;

		uint8 v =_scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_08 : MemMapping::MEM_ACTIONSEQDATA_07)[_stage];
		if (v && _result.enemiesShot >= v)
			return;

		v = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_09)[_stage];
		if (v && _enemyAppearanceCount == v)
			return;

		if (_activeEnemies == _scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_11 : MemMapping::MEM_ACTIONSEQDATA_10)[_stage])
			return;

		_nextEnemyAppearanceTimer = 16 + _enemySpeed + (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] << 1);
		if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] >= 7)
			_nextEnemyAppearanceTimer = 5;
		_updateEnemiesPhase = 1;

	} else if (_updateEnemiesPhase == 1) {
		uint8 num = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F];
		_animNo = num + 32;
		int16 v = animGet(_animNo, Phase);
		if (v == 0xFF || ((v & 0x0F) != 0))
			return;

		_updateEnemiesPhase = 2;
		++_enemyAppearanceCount;

		switch (_stage) {
		case 0:
			v = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_18)[_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F] & 3];
			_vm->gfx()->animCopySpec(26 + v - 1, _animNo);
			animSet(_animNo, Frame, 2);
			break;
		case 2:
			v = _shoot_7912 | (num < 3 ? 1 : 0);
			_vm->gfx()->animCopySpec(26 + v, _animNo);
			_shoot_7912 = (v & 2) ^ 2;
			v = (v << 1) | 1;
			break;
		case 4:
			_vm->gfx()->animCopySpec(26, _animNo);
			v = 1;
			break;
		case 10:
			v = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_19)[num];
			break;
		case 11:
			v = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_18)[_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_17)[Util::rngMakeNumber() & 0x1F]];
			_vm->gfx()->animCopySpec(26 + v + 1, _animNo);
			animSet(_animNo, Frame, 2);
			break;
		case 14:
			v = 2;
			break;
		default:
			v = 1;
			break;
		}

		v = v << 4;
		animSet(_animNo, Phase, v);

		if (v == 0x20) {
			++_civilianAppearanceCount;
			--_enemyAppearanceCount;
		}

	} else {
		uint8 v = _vm->gfx()->getAnimScriptByte(_animNo, 0);
		animSet(_animNo, Frame, v != 2 ? v + 2 : 2);
		animSet(_animNo, Enable, 1);
		if (animGet(_animNo, Phase) != 0x20 && (_stage == 0 || _stage == 11)) {
			animSet(58, Enable, 1);
			animSet(58, ControlFlags, GraphicsEngine::kAnimPause);
			animSet(58, Frame, animGet(58, Frame) + 1);
		}

		animSet(_animNo, Phase, (animGet(_animNo, Phase) & 0xF0) | 1);
		_updateEnemiesPhase = 0;

		if (_stage == 0 || _stage == 11)
			_vm->sound()->fmSendCommand(34, 0);
	}
}

void ActionSequenceHandler::enemyAttack() {
	if (!_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage])
		return;

	if (_enemyAttackStatus == 0) {
		animSet(24, Enable, 0);
		animSet(24, Frame, 0);
		if (_enemyAttackTimer)
			--_enemyAttackTimer;
	} else if (_enemyAttackStatus == 1) {
		animSet(24, PosX, _projectileCoords[_attackerId]);
		animSet(24, PosY, _projectileCoords[_attackerId + 9]);
		animSet(24, Enable, 1);
		animSet(24, Frame, 2);
		_enemyAttackStatus = 2;
		uint8 snd = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_03)[_stage];
		if (snd)
			_vm->sound()->pcmSendCommand(snd, -1);
	} else if (_enemyAttackStatus == 2) {
		if (_vm->gfx()->getAnimScriptByte(24, 0) == animGet(24, Frame))
			_enemyAttackStatus = 3;
	} else if (_enemyAttackStatus == 3) {
		_weaponState += 56;
		_enemyAttackStatus = 0;
		animSet(24, Enable, 0);
		if (_stage == 4)
			_vm->sound()->pcmSendCommand(52, -1);
		else
			_vm->sound()->fmSendCommand(19, 0);
		if (_result.hitsTaken < 6) {
			int16 vl = animGet(48, ActionTimer) - 1;
			animSet(48, ActionTimer, vl);
			(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_20) + 6).writeUINT16((_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_21) + (vl << 1)).readUINT16());
			_vm->gfx()->enqueueDrawCommands(_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_20));
			++_result.hitsTaken;
		}
	}
}

void ActionSequenceHandler::checkFinished() {
	if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] && _result.hitsTaken >= 6) {
		for (int i = 32; i < 41; ++i)
			animSet(i, ControlFlags, animGet(i, ControlFlags) | GraphicsEngine::kAnimPause);
		_vm->gfx()->reset(GraphicsEngine::kResetPalEvents);
		_progressMSB = 3;
		_result.hitsTaken = 6;
	} else {
		uint8 v = _scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_08 : MemMapping::MEM_ACTIONSEQDATA_07)[_stage];
		if (v && _result.enemiesShot < v)
			return;

		v = _scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_09)[_stage];
		if (v && (_updateEnemiesPhase || (_enemyAppearanceCount < v)))
			return;

		if (_scd->makePtr(_useLightGun ? MemMapping::MEM_ACTIONSEQDATA_11 : MemMapping::MEM_ACTIONSEQDATA_10)[_stage] != 1 && _enemyAttackStatus != 0)
			return;

		if (_handleShotPhase != 0)
			return;

		for (int i = 32; i < 41; ++i) {
			int16 a = animGet(i, Phase);
			if (a == 0xFF)
				continue;
			if ((a & 0x0F) != 0)
				return;
			if (_scd->makePtr(MemMapping::MEM_ACTIONSEQDATA_12)[_stage] == 0 || _stage == 7)
				continue;
			animSet(i, Enable, 0);
		}

		if (_stage == 0 || _stage == 11) {
			animSet(56, Enable, 1);
			animSet(57, Enable, 1);
			animSet(56, Frame, 2);
			animSet(57, Frame, 2);
		}
		_progressMSB = 3;
	}
}

#undef animSet
#undef animGet

} // End of namespace Snatcher
