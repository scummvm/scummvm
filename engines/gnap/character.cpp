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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "gnap/gnap.h"
#include "gnap/character.h"
#include "gnap/gamesys.h"

namespace Gnap {

Character::Character(GnapEngine *vm) : _vm(vm) {
	_pos = Common::Point(0, 0);
	_idleFacing = kDirIdleLeft;
	_actionStatus = 0;
	_sequenceId = 0;
	_sequenceDatNum = 0;
	_id = 0;
	_gridX = 0;
	_gridY = 0;

	_walkNodesCount = 0;
	_walkDestX = _walkDestY = 0;
	_walkDeltaX = _walkDeltaY = 0;
	_walkDirX = _walkDirY = 0;
	_walkDirXIncr = _walkDirYIncr = 0;

	for(int i = 0; i < kMaxGridStructs; i++) {
		_walkNodes[i]._id = 0;
		_walkNodes[i]._sequenceId = 0;
		_walkNodes[i]._deltaX = 0;
		_walkNodes[i]._deltaY = 0;
		_walkNodes[i]._gridX1 = 0;
		_walkNodes[i]._gridY1 = 0;
	}
}

Character::~Character() {}

void Character::walkStep() {
	for (int i = 1; i < _vm->_gridMaxX; ++i) {
		Common::Point checkPt = Common::Point(_pos.x + i, _pos.y);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x - i, _pos.y);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x, _pos.y + 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x, _pos.y - 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x + 1, _pos.y + 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x - 1, _pos.y + 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x + 1, _pos.y - 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}

		checkPt = Common::Point(_pos.x - 1, _pos.y - 1);
		if (!_vm->isPointBlocked(checkPt)) {
			walkTo(checkPt, -1, -1, 1);
			break;
		}
	}
}
/************************************************************************************************/

PlayerGnap::PlayerGnap(GnapEngine * vm) : Character(vm) {
	_brainPulseNum = 0;
	_brainPulseRndValue = 0;
}

int PlayerGnap::getSequenceId(int kind, Common::Point gridPos) {
	int sequenceId = 0;

	switch (kind) {
	case kGSPullOutDevice:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x83F;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x83D;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x83B;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x839;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = 0x839;
				break;
			case kDirBottomLeft:
				sequenceId = 0x83B;
				break;
			case kDirUpRight:
				sequenceId = 0x83D;
				break;
			default:
				sequenceId = 0x83F;
				break;
			}
		}
		break;

	case kGSPullOutDeviceNonWorking:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x829;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x828;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x827;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x826;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = 0x826;
				break;
			case kDirBottomLeft:
				sequenceId = 0x827;
				break;
			case kDirUpRight:
				sequenceId = 0x828;
				break;
			default:
				sequenceId = 0x829;
				break;
			}
		}
		break;

	case kGSScratchingHead:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x834;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x885;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x834;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x833;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = 0x833;
				_idleFacing = kDirBottomRight;
				break;
			case kDirBottomLeft:
				sequenceId = 0x834;
				_idleFacing = kDirBottomLeft;
				break;
			case kDirUpRight:
				sequenceId = 0x885;
				_idleFacing = kDirUpRight;
				break;
			default:
				sequenceId = 0x834;
				_idleFacing = kDirBottomLeft;
				break;
			}
		}
		break;

	case kGSIdle:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x7BC;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x7BB;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x7BA;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7B9;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = 0x7B9;
				break;
			case kDirBottomLeft:
				sequenceId = 0x7BA;
				break;
			case kDirUpRight:
				sequenceId = 0x7BB;
				break;
			default:
				sequenceId = 0x7BC;
				break;
			}
		}
		break;

	case kGSBrainPulsating:
		_brainPulseNum = (_brainPulseNum + 1) & 1;
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x812;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7FE;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7D6;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7EA;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7EA;
				break;
			case kDirBottomLeft:
				sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7D6;
				break;
			case kDirUpRight:
				sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7FE;
				break;
			default:
				sequenceId = _brainPulseRndValue + _brainPulseNum + 0x812;
				break;
			}
		}
		break;

	case kGSImpossible:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x831;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7A8;
					_idleFacing = kDirBottomRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x831;
					_idleFacing = kDirBottomLeft;
				} else {
					if (_pos.x % 2)
						sequenceId = 0x7A8;
					else
						sequenceId = 0x89A;
					_idleFacing = kDirBottomRight;
				}
			}
		} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
			sequenceId = 0x831;
			_idleFacing = kDirBottomLeft;
		} else {
			if (_vm->_currentSceneNum % 2)
				sequenceId = 0x7A8;
			else
				sequenceId = 0x89A;
			_idleFacing = kDirBottomRight;
		}
		break;

	case kGSDeflect:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.y > gridPos.y) {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x830;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x82F;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridPos.x) {
					sequenceId = 0x82E;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7A7;
					_idleFacing = kDirBottomRight;
				}
			}
		} else {
			switch (_idleFacing) {
			case kDirBottomRight:
				sequenceId = 0x7A7;
				break;
			case kDirBottomLeft:
				sequenceId = 0x82E;
				break;
			case kDirUpLeft:
				sequenceId = 0x830;
				break;
			case kDirUpRight:
				sequenceId = 0x82F;
				break;
			case kDirIdleLeft:
			case kDirIdleRight:
			default:
				break;
			}
		}
		break;

	case kGSUseDevice:
		switch (_idleFacing) {
		case kDirBottomRight:
			sequenceId = 0x83A;
			break;
		case kDirBottomLeft:
			sequenceId = 0x83C;
			break;
		case kDirUpLeft:
			sequenceId = 0x840;
			break;
		case kDirUpRight:
			sequenceId = 0x83E;
			break;
		case kDirIdleLeft:
		case kDirIdleRight:
		default:
			break;
		}
		break;

	case kGSMoan1:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.x > gridPos.x) {
				sequenceId = 0x832;
				_idleFacing = kDirBottomLeft;
			} else {
				sequenceId = 0x7AA;
				_idleFacing = kDirBottomRight;
			}
		} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
			sequenceId = 0x832;
			_idleFacing = kDirBottomLeft;
		} else {
			sequenceId = 0x7AA;
			_idleFacing = kDirBottomRight;
		}
		break;

	case kGSMoan2:
		if (gridPos.x > 0 && gridPos.y > 0) {
			if (_pos.x > gridPos.x) {
				sequenceId = 0x832;
				_idleFacing = kDirBottomLeft;
			} else {
				sequenceId = 0x7AA;
				_idleFacing = kDirBottomRight;
			}
		} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
			sequenceId = 0x832;
			_idleFacing = kDirBottomLeft;
		} else {
			sequenceId = 0x7AA;
			_idleFacing = kDirBottomRight;
		}
		break;

	default:
		break;
	}

	return sequenceId | 0x10000;
}

void PlayerGnap::useJointOnPlatypus() {
	PlayerPlat& plat = *_vm->_plat;

	_vm->setGrabCursorSprite(-1);
	if (doPlatypusAction(1, 0, 0x107C1, 0)) {
		_actionStatus = 100;
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_gameSys->setAnimation(0x10876, plat._id, 0);
		_vm->_gameSys->insertSequence(0x10875, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncWait, 0, 15 * (5 * _pos.x - 30), 48 * (_pos.y - 7));
		_sequenceDatNum = 1;
		_sequenceId = 0x875;
		_vm->_gameSys->insertSequence(0x10876, plat._id,
			plat._sequenceId | (plat._sequenceDatNum << 16), plat._id,
			kSeqSyncWait, 0, 15 * (5 * plat._pos.x - 25), 48 * (plat._pos.y - 7));
		plat._sequenceDatNum = 1;
		plat._sequenceId = 0x876;
		plat._idleFacing = kDirIdleLeft;
		playSequence(0x107B5);
		walkStep();
		while (_vm->_gameSys->getAnimationStatus(0) != 2 && !_vm->_gameDone) {
			_vm->updateMouseCursor();
			_vm->gameUpdateTick();
		}
		_vm->_gameSys->setAnimation(0, 0, 0);
		_actionStatus = -1;
	} else {
		playSequence(getSequenceId(kGSScratchingHead, plat._pos) | 0x10000);
	}
}

void PlayerGnap::kissPlatypus(int callback) {
	PlayerPlat& plat = *_vm->_plat;

	if (doPlatypusAction(-1, 0, 0x107D1, callback)) {
		_actionStatus = 100;
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_gameSys->setAnimation(0x10847, _id, 0);
		_vm->_gameSys->insertSequence(0x10847, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncWait, 0, 15 * (5 * _pos.x - 20) - (21 - _vm->_gridMinX), 48 * (_pos.y - 6) - (146 - _vm->_gridMinY));
		_sequenceDatNum = 1;
		_sequenceId = 0x847;
		_vm->_gameSys->insertSequence(0x107CB, plat._id,
			makeRid(plat._sequenceDatNum, plat._sequenceId), plat._id,
			kSeqSyncWait, _vm->getSequenceTotalDuration(0x10847), 75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
		plat._sequenceDatNum = 1;
		plat._sequenceId = 0x7CB;
		plat._idleFacing = kDirIdleLeft;
		playSequence(0x107B5);
		while (_vm->_gameSys->getAnimationStatus(0) != 2 && !_vm->_gameDone) {
			_vm->updateMouseCursor();
			_vm->doCallback(callback);
			_vm->gameUpdateTick();
		}
		_vm->_gameSys->setAnimation(0, 0, 0);
		_actionStatus = -1;
	} else {
		playSequence(getSequenceId(kGSScratchingHead, plat._pos) | 0x10000);
	}
}

void PlayerGnap::useDeviceOnPlatypus() {
	PlayerPlat& plat = *_vm->_plat;

	playSequence(makeRid(1, getSequenceId(kGSPullOutDevice, plat._pos)));

	if (plat._idleFacing != kDirIdleLeft) {
		_vm->_gameSys->insertSequence(makeRid(1, 0x7D5), plat._id,
			makeRid(plat._sequenceDatNum, plat._sequenceId), plat._id,
			kSeqSyncWait, 0, 75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
		plat._sequenceId = 0x7D5;
		plat._sequenceDatNum = 1;
	} else {
		_vm->_gameSys->insertSequence(makeRid(1, 0x7D4), plat._id,
			makeRid(plat._sequenceDatNum, plat._sequenceId), plat._id,
			kSeqSyncWait, 0, 75 * plat._pos.x - plat._gridX, 48 * plat._pos.y - plat._gridY);
		plat._sequenceId = 0x7D4;
		plat._sequenceDatNum = 1;
	}

	int newSequenceId = getSequenceId(kGSUseDevice, Common::Point(0, 0));
	_vm->_gameSys->insertSequence(makeRid(1, newSequenceId), _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
	_sequenceId = newSequenceId;
	_sequenceDatNum = 1;
}

void PlayerGnap::initBrainPulseRndValue() {
	_brainPulseRndValue = 2 * _vm->getRandom(10);
}

void PlayerGnap::playSequence(int sequenceId) {
	_vm->_timers[2] = _vm->getRandom(30) + 20;
	_vm->_timers[3] = 300;
	idle();
	_vm->_gameSys->insertSequence(sequenceId, _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqScale | kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
	_sequenceId = ridToEntryIndex(sequenceId);
	_sequenceDatNum = ridToDatIndex(sequenceId);
}

void PlayerGnap::updateIdleSequence() {
	if (_actionStatus < 0) {
		if (_vm->_timers[2] > 0) {
			if (_vm->_timers[3] == 0) {
				_vm->_timers[2] = 60;
				_vm->_timers[3] = 300;
				if (_idleFacing == kDirBottomRight) {
					switch (_vm->getRandom(5)) {
					case 0:
						playSequence(0x107A6);
						break;
					case 1:
						playSequence(0x107AA);
						break;
					case 2:
						playSequence(0x10841);
						break;
					default:
						playSequence(0x108A2);
						break;
					}
				} else if (_idleFacing == kDirBottomLeft) {
					if (_vm->getRandom(5) > 2)
						playSequence(0x10832);
					else
						playSequence(0x10842);
				}
			}
		} else {
			_vm->_timers[2] = _vm->getRandom(30) + 20;
			if (_idleFacing == kDirBottomRight) {
				_vm->_gameSys->insertSequence(0x107BD, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
				_sequenceId = 0x7BD;
				_sequenceDatNum = 1;
			} else if (_idleFacing == kDirBottomLeft) {
				_vm->_gameSys->insertSequence(0x107BE, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
				_sequenceId = 0x7BE;
				_sequenceDatNum = 1;
			}
		}
	} else {
		_vm->_timers[2] = _vm->getRandom(30) + 20;
		_vm->_timers[3] = 300;
	}
}

void PlayerGnap::updateIdleSequence2() {
	if (_actionStatus < 0) {
		if (_vm->_timers[2] > 0) {
			if (_vm->_timers[3] == 0) {
				_vm->_timers[2] = 60;
				_vm->_timers[3] = 300;
				if (_idleFacing == kDirBottomRight) {
					playSequence(0x107AA);
				} else if (_idleFacing == kDirBottomLeft) {
					playSequence(0x10832);
				}
			}
		} else {
			_vm->_timers[2] = _vm->getRandom(30) + 20;
			if (_idleFacing == kDirBottomRight) {
				_vm->_gameSys->insertSequence(0x107BD, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
				_sequenceId = 0x7BD;
				_sequenceDatNum = 1;
			} else if (_idleFacing == kDirBottomLeft) {
				_vm->_gameSys->insertSequence(0x107BE, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
				_sequenceId = 0x7BE;
				_sequenceDatNum = 1;
			}
		}
	} else {
		_vm->_timers[2] = _vm->getRandom(30) + 20;
		_vm->_timers[3] = 300;
	}
}

void PlayerGnap::initPos(int gridX, int gridY, Facing facing) {
	_vm->_timers[2] = 30;
	_vm->_timers[3] = 300;
	_pos = Common::Point(gridX, gridY);
	if (facing == kDirIdleLeft)
		_idleFacing = kDirBottomRight;
	else
		_idleFacing = facing;

	if (_idleFacing == kDirBottomLeft) {
		_sequenceId = 0x7B8;
	} else {
		_sequenceId = 0x7B5;
		_idleFacing = kDirBottomRight;
	}
	_id = 20 * _pos.y;
	_sequenceDatNum = 1;
	_vm->_gameSys->insertSequence(makeRid(1, _sequenceId), 20 * _pos.y,
		0, 0,
		kSeqScale, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
}

int PlayerGnap::getWalkSequenceId(int deltaX, int deltaY) {
	static const int walkSequenceIds[9] = {
		0x7B2, 0x000, 0x7B4,
		0x7AD, 0x000, 0x7AE,
		0x7B1, 0x000, 0x7B3
	};

	int id = 3 * (deltaX + 1) + deltaY + 1;
	assert(id >= 0 && id < 9);

	return walkSequenceIds[id];
}

bool PlayerGnap::walkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) {
	PlayerPlat& plat = *_vm->_plat;
	int datNum = flags & 3;

	_vm->_timers[2] = 200;
	_vm->_timers[3] = 300;

	int gridX = gridPos.x;
	if (gridX < 0)
		gridX = (_vm->_leftClickMouseX - _vm->_gridMinX + 37) / 75;

	int gridY = gridPos.y;
	if (gridY < 0)
		gridY = (_vm->_leftClickMouseY - _vm->_gridMinY + 24) / 48;

	_walkDestX = CLIP(gridX, 0, _vm->_gridMaxX - 1);
	_walkDestY = CLIP(gridY, 0, _vm->_gridMaxY - 1);

	if (animationIndex >= 0 && _walkDestX == plat._pos.x && _walkDestY == plat._pos.y)
		plat.makeRoom();

	bool done = findPath1(_pos.x, _pos.y, 0);

	if (!done)
		done = findPath2(_pos.x, _pos.y, 0);

	if (!done)
		done = findPath3(_pos.x, _pos.y);

	if (!done)
		done = findPath4(_pos.x, _pos.y);

	idle();

	int gnapSequenceId = _sequenceId;
	int gnapId = _id;
	int gnapSequenceDatNum = _sequenceDatNum;

	debugC(kDebugBasic, "_gnap->_walkNodesCount: %d", _walkNodesCount);

	for (int index = 0; index < _walkNodesCount; ++index) {
		_walkNodes[index]._id = index + 20 * _walkNodes[index]._gridY1;
		if (_walkNodes[index]._deltaX == 1 && _walkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7AB), _walkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7AB;
				gnapSequenceId = 0x7AB;
			} else {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7AC), _walkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7AC;
				gnapSequenceId = 0x7AC;
			}
		} else if (_walkNodes[index]._deltaX == -1 && _walkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7AF), _walkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7AF;
				gnapSequenceId = 0x7AF;
			} else {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7B0), _walkNodes[index]._id,
					makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7B0;
				gnapSequenceId = 0x7B0;
			}
		} else {
			if (_walkNodes[index]._deltaY == -1)
				_walkNodes[index]._id -= 10;
			else
				_walkNodes[index]._id += 10;
			int newSequenceId = getWalkSequenceId(_walkNodes[index]._deltaX, _walkNodes[index]._deltaY);
			_vm->_gameSys->insertSequence(makeRid(datNum, newSequenceId), _walkNodes[index]._id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
			_walkNodes[index]._sequenceId = newSequenceId;
			gnapSequenceId = newSequenceId;
		}
		gnapId = _walkNodes[index]._id;
		gnapSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_walkNodesCount > 0) {
			_sequenceId = gnapSequenceId;
			_id = gnapId;
			_idleFacing = getWalkFacing(_walkNodes[_walkNodesCount - 1]._deltaX, _walkNodes[_walkNodesCount - 1]._deltaY);
			_sequenceDatNum = datNum;
			if (animationIndex >= 0)
				_vm->_gameSys->setAnimation(makeRid(_sequenceDatNum, _sequenceId), _id, animationIndex);
		} else if (animationIndex >= 0) {
			_vm->_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_vm->_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_sequenceId = ridToEntryIndex(sequenceId);
			_sequenceDatNum = ridToDatIndex(sequenceId);
			if (_sequenceId == 0x7B9) {
				_idleFacing = kDirBottomRight;
			} else {
				switch (_sequenceId) {
				case 0x7BA:
					_idleFacing = kDirBottomLeft;
					break;
				case 0x7BB:
					_idleFacing = kDirUpRight;
					break;
				case 0x7BC:
					_idleFacing = kDirUpLeft;
					break;
				default:
					break;
				}
			}
		} else {
			if (_walkNodesCount > 0) {
				_sequenceId = getWalkStopSequenceId(_walkNodes[_walkNodesCount - 1]._deltaX, _walkNodes[_walkNodesCount - 1]._deltaY);
				_idleFacing = getWalkFacing(_walkNodes[_walkNodesCount - 1]._deltaX, _walkNodes[_walkNodesCount - 1]._deltaY);
			} else if (gridX >= 0 || gridY >= 0) {
				switch (_idleFacing) {
				case kDirBottomRight:
					_sequenceId = 0x7B9;
					break;
				case kDirBottomLeft:
					_sequenceId = 0x7BA;
					break;
				case kDirUpRight:
					_sequenceId = 0x7BB;
					break;
				default:
					_sequenceId = 0x7BC;
					break;
				}
			} else {
				int dirX = _vm->_leftClickMouseX - (_vm->_gridMinX + 75 * _pos.x);
				int dirY = _vm->_leftClickMouseY - (_vm->_gridMinY + 48 * _pos.y);
				if (dirX == 0)
					dirX = 1;
				if (dirY == 0)
					dirY = 1;
				_sequenceId = getWalkStopSequenceId(dirX / abs(dirX), dirY / abs(dirY));
				_idleFacing = getWalkFacing(dirX / abs(dirX), dirY / abs(dirY));
			}
			_sequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_id = 20 * _walkDestY + 1;
		} else {
			_id = _walkNodesCount + animationIndex + 20 * _walkDestY;
			_vm->_gameSys->setAnimation(makeRid(_sequenceDatNum, _sequenceId), _walkNodesCount + animationIndex + 20 * _walkDestY, animationIndex);
		}

		if (flags & 4) {
			_vm->_gameSys->insertSequence(makeRid(_sequenceDatNum, _sequenceId), _id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 0, 0);
		} else {
			_vm->_gameSys->insertSequence(makeRid(_sequenceDatNum, _sequenceId), _id,
				makeRid(gnapSequenceDatNum, gnapSequenceId), gnapId,
				kSeqScale | kSeqSyncWait, 0, 75 * _walkDestX - _gridX, 48 * _walkDestY - _gridY);
		}
	}

	_pos = Common::Point(_walkDestX, _walkDestY);

	return done;
}

int PlayerGnap::getShowSequenceId(int index, int gridX, int gridY) {
	int sequenceId;
	Facing facing = _idleFacing;

	if (gridY > 0 && gridX > 0) {
		if (_pos.x > gridX)
			_idleFacing = kDirUpLeft;
		else
			_idleFacing = kDirUpRight;
	} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
		_idleFacing = kDirUpLeft;
	} else {
		_idleFacing = kDirUpRight;
	}

	switch (index) {
	case 0:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x8A0;
		else
			sequenceId = 0x8A1;
		break;
	case 1:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x880;
		else
			sequenceId = 0x895;
		break;
	case 2:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x884;
		else
			sequenceId = 0x899;
		break;
		//Skip 3
	case 4:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x881;
		else
			sequenceId = 0x896;
		break;
	case 5:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x883;
		else
			sequenceId = 0x898;
		break;
	case 6:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87E;
		else
			sequenceId = 0x893;
		break;
	case 7:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x848;
		else
			sequenceId = 0x890;
		break;
	case 8:
	case 12:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87D;
		else
			sequenceId = 0x892;
		break;
	case 9:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x882;
		else
			sequenceId = 0x897;
		break;
	case 10:
	case 11:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87C;
		else
			sequenceId = 0x891;
		break;
	case 13:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x888;
		else
			sequenceId = 0x89D;
		break;
	case 14:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87F;
		else
			sequenceId = 0x894;
		break;
	case 15:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87B;
		else
			sequenceId = 0x8A3;
		break;
	case 16:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x877;
		else
			sequenceId = 0x88C;
		break;
		//Skip 17
	case 18:
		sequenceId = 0x887;
		break;
	case 19:
	case 25:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x87A;
		else
			sequenceId = 0x88F;
		break;
	case 20:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x878;
		else
			sequenceId = 0x88D;
		break;
	case 21:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x879;
		else
			sequenceId = 0x88E;
		break;
	case 22:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x88A;
		else
			sequenceId = 0x89F;
		break;
	case 23:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x889;
		else
			sequenceId = 0x89E;
		break;
	case 24:
		if (_idleFacing == kDirUpRight)
			sequenceId = 0x886;
		else
			sequenceId = 0x89B;
		break;
		//Skip 26
		//Skip 27
		//Skip 28
		//Skip 29
	default:
		_idleFacing = facing;
		sequenceId = getSequenceId(kGSImpossible, Common::Point(0, 0));
		break;
	}

	return sequenceId;
}

void PlayerGnap::idle() {
	if (_sequenceDatNum == 1 &&
			(_sequenceId == 0x7A6 || _sequenceId == 0x7AA ||
			_sequenceId == 0x832 || _sequenceId == 0x841 ||
			_sequenceId == 0x842 || _sequenceId == 0x8A2 ||
			_sequenceId == 0x833 || _sequenceId == 0x834 ||
			_sequenceId == 0x885 || _sequenceId == 0x7A8 ||
			_sequenceId == 0x831 || _sequenceId == 0x89A)) {
		_vm->_gameSys->insertSequence(getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncExists, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
		_sequenceId = getSequenceId(kGSIdle, Common::Point(0, 0));
		_sequenceDatNum = 1;
	}
}

void PlayerGnap::actionIdle(int sequenceId) {
	if (_sequenceId != -1 && ridToDatIndex(sequenceId) == _sequenceDatNum && ridToEntryIndex(sequenceId) == _sequenceId) {
		_vm->_gameSys->insertSequence(getSequenceId(kGSIdle, Common::Point(0, 0)) | 0x10000, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncExists, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
		_sequenceId = getSequenceId(kGSIdle, Common::Point(0, 0));
		_sequenceDatNum = 1;
	}
}

void PlayerGnap::playImpossible(Common::Point gridPos) {
	playSequence(getSequenceId(kGSImpossible, gridPos) | 0x10000);
}

void PlayerGnap::playScratchingHead(Common::Point gridPos) {
	playSequence(getSequenceId(kGSScratchingHead, gridPos) | 0x10000);
}

void PlayerGnap::playMoan1(Common::Point gridPos) {
	playSequence(getSequenceId(kGSMoan1, gridPos) | 0x10000);
}

void PlayerGnap::playMoan2(Common::Point gridPos) {
	playSequence(getSequenceId(kGSMoan2, gridPos) | 0x10000);
}

void PlayerGnap::playBrainPulsating(Common::Point gridPos) {
	playSequence(getSequenceId(kGSBrainPulsating, gridPos) | 0x10000);
}

void PlayerGnap::playPullOutDevice(Common::Point gridPos) {
	playSequence(getSequenceId(kGSPullOutDevice, gridPos) | 0x10000);
}

void PlayerGnap::playPullOutDeviceNonWorking(Common::Point gridPos) {
	playSequence(getSequenceId(kGSPullOutDeviceNonWorking, gridPos) | 0x10000);
}

void PlayerGnap::playUseDevice(Common::Point gridPos) {
	playSequence(getSequenceId(kGSUseDevice, gridPos) | 0x10000);
}

void PlayerGnap::playIdle(Common::Point gridPos) {
	playSequence(getSequenceId(kGSIdle, gridPos) | 0x10000);
}

void PlayerGnap::playShowItem(int itemIndex, int gridLookX, int gridLookY) {
	playSequence(getShowSequenceId(itemIndex, gridLookX, gridLookY) | 0x10000);
}

void PlayerGnap::playShowCurrItem(Common::Point destPos, int gridLookX, int gridLookY) {
	PlayerPlat& plat = *_vm->_plat;

	if (plat._pos == destPos)
		plat.makeRoom();
	walkTo(destPos, -1, -1, 1);
	playShowItem(_vm->_grabCursorSpriteIndex, gridLookX, gridLookY);
}

bool PlayerGnap::doPlatypusAction(int gridX, int gridY, int platSequenceId, int callback) {
	PlayerPlat& plat = *_vm->_plat;
	bool result = false;

	if (_actionStatus <= -1 && plat._actionStatus <= -1) {
		_actionStatus = 100;
		Common::Point checkPt = plat._pos + Common::Point(gridX, gridY);
		if (_vm->isPointBlocked(checkPt) && (_pos != checkPt)) {
			plat.walkStep();
			checkPt = plat._pos + Common::Point(gridX, gridY);
		}

		if (!_vm->isPointBlocked(checkPt) && (_pos != checkPt)) {
			walkTo(checkPt, 0, 0x107B9, 1);
			while (_vm->_gameSys->getAnimationStatus(0) != 2 && !_vm->_gameDone) {
				_vm->updateMouseCursor();
				_vm->doCallback(callback);
				_vm->gameUpdateTick();
			}
			_vm->_gameSys->setAnimation(0, 0, 0);
			if (_pos == plat._pos + Common::Point(gridX, gridY)) {
				_vm->_gameSys->setAnimation(platSequenceId, plat._id, 1);
				plat.playSequence(platSequenceId);
				while (_vm->_gameSys->getAnimationStatus(1) != 2 && !_vm->_gameDone) {
					_vm->updateMouseCursor();
					_vm->doCallback(callback);
					_vm->gameUpdateTick();
				}
				result = true;
			}
		}
		_actionStatus = -1;
	}
	return result;
}

void PlayerGnap::useDisguiseOnPlatypus() {
	_vm->_gameSys->setAnimation(0x10846, _id, 0);
	playSequence(0x10846);
	while (_vm->_gameSys->getAnimationStatus(0) != 2 && !_vm->_gameDone)
		_vm->gameUpdateTick();

	_vm->_newSceneNum = 47;
	_vm->_isLeavingScene = true;
	_vm->_sceneDone = true;
	_vm->setFlag(kGFPlatypusDisguised);
}

/************************************************************************************************/

PlayerPlat::PlayerPlat(GnapEngine * vm) : Character(vm) {}

int PlayerPlat::getSequenceId(int kind, Common::Point gridPos) {
	// The original had 3 parameters, all always set to 0.
	// The code to handle the other values has been removed.

	int sequenceId = 0x7CB;

	if (_idleFacing != kDirIdleLeft) {
		sequenceId = 0x7CC;
		_idleFacing = kDirIdleRight;
	}

	return sequenceId | 0x10000;
}

void PlayerPlat::playSequence(int sequenceId) {
	_vm->_gameSys->insertSequence(sequenceId, _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqScale | kSeqSyncWait, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
	_sequenceId = ridToEntryIndex(sequenceId);
	_sequenceDatNum = ridToDatIndex(sequenceId);
}

void PlayerPlat::updateIdleSequence() {
	if (_actionStatus < 0 && _vm->_gnap->_actionStatus < 0) {
		if (_vm->_timers[0] > 0) {
			if (_vm->_timers[1] == 0) {
				_vm->_timers[1] = _vm->getRandom(20) + 30;
				int rnd = _vm->getRandom(10);
				if (_idleFacing != kDirIdleLeft) {
					if (rnd != 0 || _sequenceId != 0x7CA) {
						if (rnd != 1 || _sequenceId != 0x7CA)
							playSequence(0x107CA);
						else
							playSequence(0x10845);
					} else {
						playSequence(0x107CC);
					}
				} else if (rnd != 0 || _sequenceId != 0x7C9) {
					if (rnd != 1 || _sequenceId != 0x7C9) {
						if (rnd != 2 || _sequenceId != 0x7C9)
							playSequence(0x107C9);
						else
							playSequence(0x108A4);
					} else {
						playSequence(0x10844);
					}
				} else {
					playSequence(0x107CB);
				}
			}
		} else {
			_vm->_timers[0] = _vm->getRandom(75) + 75;
			makeRoom();
		}
	} else {
		_vm->_timers[0] = 100;
		_vm->_timers[1] = 35;
	}
}

void PlayerPlat::updateIdleSequence2() {
	PlayerGnap& gnap = *_vm->_gnap;

	if (_actionStatus < 0 && gnap._actionStatus < 0) {
		if (_vm->_timers[0]) {
			if (!_vm->_timers[1]) {
				_vm->_timers[1] = _vm->getRandom(20) + 30;
				if (_idleFacing != kDirIdleLeft) {
					if (_vm->getRandom(10) >= 2 || _sequenceId != 0x7CA)
						playSequence(0x107CA);
					else
						playSequence(0x107CC);
				} else {
					if (_vm->getRandom(10) >= 2 || _sequenceId != 0x7C9) {
						playSequence(0x107C9);
					} else {
						playSequence(0x107CB);
					}
				}
			}
		} else {
			_vm->_timers[0] = _vm->getRandom(75) + 75;
			makeRoom();
		}
	} else {
		_vm->_timers[0] = 100;
		_vm->_timers[1] = 35;
	}
}

void PlayerPlat::initPos(int gridX, int gridY, Facing facing) {
	_vm->_timers[0] = 50;
	_vm->_timers[1] = 20;
	_pos = Common::Point(gridX, gridY);
	if (facing == kDirIdleLeft)
		_idleFacing = kDirIdleLeft;
	else
		_idleFacing = facing;
	if (_idleFacing == kDirIdleRight) {
		_sequenceId = 0x7D1;
	} else {
		_sequenceId = 0x7C1;
		_idleFacing = kDirIdleLeft;
	}
	_id = 20 * _pos.y;
	_sequenceDatNum = 1;
	_vm->_gameSys->insertSequence(makeRid(1, _sequenceId), 20 * _pos.y,
		0, 0,
		kSeqScale, 0, 75 * _pos.x - _gridX, 48 * _pos.y - _gridY);
}

int PlayerPlat::getWalkSequenceId(int deltaX, int deltaY) {
	static const int walkSequenceIds[9] = {
		0x7C5, 0x000, 0x7C8,
		0x7C4, 0x000, 0x7C7,
		0x7C3, 0x000, 0x7C6
	};

	int id = 3 * (deltaX + 1) + deltaY + 1;
	assert(id >= 0 && id < 9);

	return walkSequenceIds[id];
}

bool PlayerPlat::walkTo(Common::Point gridPos, int animationIndex, int sequenceId, int flags) {
	// Note: flags is always 1. The code could be simplified.

	int datNum = flags & 3;
	PlayerGnap& gnap = *_vm->_gnap;

	_vm->_timers[1] = 60;

	int gridX = gridPos.x;
	if (gridX < 0)
		gridX = (_vm->_leftClickMouseX - _vm->_gridMinX + 37) / 75;

	int gridY = gridPos.y;
	if (gridY < 0)
		gridY = (_vm->_leftClickMouseY - _vm->_gridMinY + 24) / 48;

	_walkDestX = CLIP(gridX, 0, _vm->_gridMaxX - 1);
	_walkDestY = CLIP(gridY, 0, _vm->_gridMaxY - 1);

	if (animationIndex >= 0 && gnap._pos == Common::Point(_walkDestX, _walkDestY))
		gnap.walkStep();

	bool done = findPath1(_pos.x, _pos.y, 0);

	if (!done)
		done = findPath2(_pos.x, _pos.y, 0);

	if (!done)
		done = findPath3(_pos.x, _pos.y);

	if (!done)
		done = findPath4(_pos.x, _pos.y);

	int platSequenceId = _sequenceId;
	int platId = _id;
	int platSequenceDatNum = _sequenceDatNum;

	for (int index = 0; index < _walkNodesCount; ++index) {
		_walkNodes[index]._id = index + 20 * _walkNodes[index]._gridY1;
		if (_walkNodes[index]._deltaX == 1 && _walkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7CD), _walkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7CD;
				platSequenceId = 0x7CD;
			} else {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7CE), _walkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7CE;
				platSequenceId = 0x7CE;
			}
		} else if (_walkNodes[index]._deltaX == -1 && _walkNodes[index]._deltaY == 0) {
			if (index % 2) {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7CF), _walkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7CF;
				platSequenceId = 0x7CF;
			} else {
				_vm->_gameSys->insertSequence(makeRid(datNum, 0x7D0), _walkNodes[index]._id,
					makeRid(platSequenceDatNum, platSequenceId), platId,
					kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
				_walkNodes[index]._sequenceId = 0x7D0;
				platSequenceId = 0x7D0;
			}
		} else {
			if (_walkNodes[index]._deltaY == -1)
				_walkNodes[index]._id -= 10;
			else
				_walkNodes[index]._id += 10;
			int newSequenceId = getWalkSequenceId(_walkNodes[index]._deltaX, _walkNodes[index]._deltaY);
			_vm->_gameSys->insertSequence(makeRid(datNum, newSequenceId), _walkNodes[index]._id,
				makeRid(platSequenceDatNum, platSequenceId), platId,
				kSeqScale | kSeqSyncWait, 0, 75 * _walkNodes[index]._gridX1 - _gridX, 48 * _walkNodes[index]._gridY1 - _gridY);
			_walkNodes[index]._sequenceId = newSequenceId;
			platSequenceId = newSequenceId;
		}
		platId = _walkNodes[index]._id;
		platSequenceDatNum = datNum;
	}

	if (flags & 8) {
		if (_walkNodesCount > 0) {
			_sequenceId = platSequenceId;
			_id = platId;
			_sequenceDatNum = datNum;
			if (_walkNodes[_walkNodesCount - 1]._deltaX > 0)
				_idleFacing = kDirIdleLeft;
			else if (_walkNodes[_walkNodesCount - 1]._deltaX < 0)
				_idleFacing = kDirIdleRight;
			else if (_walkNodes[_walkNodesCount - 1]._gridX1 % 2)
				_idleFacing = kDirIdleRight;
			else
				_idleFacing = kDirIdleLeft;
			if (animationIndex >= 0)
				_vm->_gameSys->setAnimation(makeRid(_sequenceDatNum, _sequenceId), _id, animationIndex);
		} else if (animationIndex >= 0) {
			_vm->_gameSys->setAnimation(0x107D3, 1, animationIndex);
			_vm->_gameSys->insertSequence(0x107D3, 1, 0, 0, kSeqNone, 0, 0, 0);
		}
	} else {
		if (sequenceId >= 0 && sequenceId != -1) {
			_sequenceId = ridToEntryIndex(sequenceId);
			_sequenceDatNum = ridToDatIndex(sequenceId);
			if (_sequenceId == 0x7C2) {
				_idleFacing = kDirIdleLeft;
			} else if (_sequenceId == 0x7D2) {
				_idleFacing = kDirIdleRight;
			}
		} else {
			if (_walkNodesCount > 0) {
				if (_walkNodes[_walkNodesCount - 1]._deltaX > 0) {
					_sequenceId = 0x7C2;
					_idleFacing = kDirIdleLeft;
				} else if (_walkNodes[_walkNodesCount - 1]._deltaX < 0) {
					_sequenceId = 0x7D2;
					_idleFacing = kDirIdleRight;
				} else if (_walkNodes[0]._deltaX > 0) {
					_sequenceId = 0x7C2;
					_idleFacing = kDirIdleLeft;
				} else if (_walkNodes[0]._deltaX < 0) {
					_sequenceId = 0x7D2;
					_idleFacing = kDirIdleRight;
				} else {
					_sequenceId = 0x7D2;
					_idleFacing = kDirIdleRight;
				}
			} else if (_idleFacing != kDirIdleLeft) {
				_sequenceId = 0x7D2;
			} else {
				_sequenceId = 0x7C2;
			}
			_sequenceDatNum = datNum;
		}

		if (animationIndex < 0) {
			_id = 20 * _walkDestY;
		} else {
			_id = animationIndex + 20 * _walkDestY;
			_vm->_gameSys->setAnimation(makeRid(_sequenceDatNum, _sequenceId), animationIndex + 20 * _walkDestY, animationIndex);
		}

		if (flags & 4)
			_vm->_gameSys->insertSequence(makeRid(_sequenceDatNum, _sequenceId), _id,
			makeRid(platSequenceDatNum, platSequenceId), platId,
			9, 0, 0, 0);
		else
			_vm->_gameSys->insertSequence(makeRid(_sequenceDatNum, _sequenceId), _id,
			makeRid(platSequenceDatNum, platSequenceId), platId,
			9, 0, 75 * _walkDestX - _gridX, 48 * _walkDestY - _gridY);
	}

	_pos = Common::Point(_walkDestX, _walkDestY);

	return done;
}
} // End of namespace Gnap
