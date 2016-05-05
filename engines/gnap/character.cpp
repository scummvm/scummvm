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
 */

#include "gnap/gnap.h"
#include "gnap/character.h"
#include "gnap/gamesys.h"

namespace Gnap {

Character::Character(GnapEngine *vm) : _vm(vm) {
	_pos = Common::Point(0, 0);
	_idleFacing = kDirNone;
	_actionStatus = 0;
	_sequenceId = 0;
	_sequenceDatNum = 0;
	_id = 0;
}

Character::~Character() {}

PlayerGnap::PlayerGnap(GnapEngine * vm) : Character(vm) {
	_brainPulseNum = 0;
	_brainPulseRndValue = 0;
}

int PlayerGnap::getSequenceId(int kind, int gridX, int gridY) {
	int sequenceId = 0;

	switch (kind) {
	case gskPullOutDevice:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x83F;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x83D;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskPullOutDeviceNonWorking:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x829;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x828;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskScratchingHead:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x834;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x885;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskIdle:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x7BC;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x7BB;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskBrainPulsating:
		_brainPulseNum = (_brainPulseNum + 1) & 1;
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x812;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = _brainPulseRndValue + _brainPulseNum + 0x7FE;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskImpossible:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x831;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7A8;
					_idleFacing = kDirBottomRight;
				}
			} else {
				if (_pos.x > gridX) {
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

	case gskDeflect:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x830;
					_idleFacing = kDirUpLeft;
				} else {
					sequenceId = 0x82F;
					_idleFacing = kDirUpRight;
				}
			} else {
				if (_pos.x > gridX) {
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
			case kDirNone:
			case kDirUnk4:
				break;
			}
		}
		break;

	case gskUseDevice:
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
		case kDirNone:
		case kDirUnk4:
			break;
		}
		break;

	case gskMoan1:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x832;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7AA;
					_idleFacing = kDirBottomRight;
				}
			} else {
				if (_pos.x > gridX) {
					sequenceId = 0x832;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7AA;
					_idleFacing = kDirBottomRight;
				}
			}
		} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
			sequenceId = 0x832;
			_idleFacing = kDirBottomLeft;
		} else {
			sequenceId = 0x7AA;
			_idleFacing = kDirBottomRight;
		}
		break;

	case gskMoan2:
		if (gridX > 0 && gridY > 0) {
			if (_pos.y > gridY) {
				if (_pos.x > gridX) {
					sequenceId = 0x832;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7AA;
					_idleFacing = kDirBottomRight;
				}
			} else {
				if (_pos.x > gridX) {
					sequenceId = 0x832;
					_idleFacing = kDirBottomLeft;
				} else {
					sequenceId = 0x7AA;
					_idleFacing = kDirBottomRight;
				}
			}
		} else if (_idleFacing != kDirBottomRight && _idleFacing != kDirUpRight) {
			sequenceId = 0x832;
			_idleFacing = kDirBottomLeft;
		} else {
			sequenceId = 0x7AA;
			_idleFacing = kDirBottomRight;
		}
		break;
	}

	return sequenceId | 0x10000;
}

void PlayerGnap::useJointOnPlatypus() {
	_vm->setGrabCursorSprite(-1);
	if (_vm->gnapPlatypusAction(1, 0, 0x107C1, 0)) {
		_actionStatus = 100;
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_gameSys->setAnimation(0x10876, _vm->_plat->_id, 0);
		_vm->_gameSys->insertSequence(0x10875, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncWait, 0, 15 * (5 * _pos.x - 30), 48 * (_pos.y - 7));
		_sequenceDatNum = 1;
		_sequenceId = 0x875;
		_vm->_gameSys->insertSequence(0x10876, _vm->_plat->_id,
			_vm->_plat->_sequenceId | (_vm->_plat->_sequenceDatNum << 16), _vm->_plat->_id,
			kSeqSyncWait, 0, 15 * (5 * _vm->_plat->_pos.x - 25), 48 * (_vm->_plat->_pos.y - 7));
		_vm->_plat->_sequenceDatNum = 1;
		_vm->_plat->_sequenceId = 0x876;
		_vm->_plat->_idleFacing = kDirNone;
		_vm->_gnap->playSequence(0x107B5);
		_vm->gnapWalkStep();
		while (_vm->_gameSys->getAnimationStatus(0) != 2) {
			_vm->updateMouseCursor();
			_vm->gameUpdateTick();
		}
		_vm->_gameSys->setAnimation(0, 0, 0);
		_actionStatus = -1;
	} else {
		_vm->_gnap->playSequence(getSequenceId(gskScratchingHead, _vm->_plat->_pos.x, _vm->_plat->_pos.y) | 0x10000);
	}
}

void PlayerGnap::kissPlatypus(int callback) {
	if (_vm->gnapPlatypusAction(-1, 0, 0x107D1, callback)) {
		_actionStatus = 100;
		_vm->_gameSys->setAnimation(0, 0, 1);
		_vm->_gameSys->setAnimation(0x10847, _id, 0);
		_vm->_gameSys->insertSequence(0x10847, _id,
			makeRid(_sequenceDatNum, _sequenceId), _id,
			kSeqSyncWait, 0, 15 * (5 * _pos.x - 20) - (21 - _vm->_gridMinX), 48 * (_pos.y - 6) - (146 - _vm->_gridMinY));
		_sequenceDatNum = 1;
		_sequenceId = 0x847;
		_vm->_gameSys->insertSequence(0x107CB, _vm->_plat->_id,
			makeRid(_vm->_plat->_sequenceDatNum, _vm->_plat->_sequenceId), _vm->_plat->_id,
			kSeqSyncWait, _vm->getSequenceTotalDuration(0x10847), 75 * _vm->_plat->_pos.x - _vm->_platGridX, 48 * _vm->_plat->_pos.y - _vm->_platGridY);
		_vm->_plat->_sequenceDatNum = 1;
		_vm->_plat->_sequenceId = 0x7CB;
		_vm->_plat->_idleFacing = kDirNone;
		_vm->_gnap->playSequence(0x107B5);
		while (_vm->_gameSys->getAnimationStatus(0) != 2) {
			_vm->updateMouseCursor();
			_vm->doCallback(callback);
			_vm->gameUpdateTick();
		}
		_vm->_gameSys->setAnimation(0, 0, 0);
		_actionStatus = -1;
	} else {
		_vm->_gnap->playSequence(getSequenceId(gskScratchingHead, _vm->_plat->_pos.x, _vm->_plat->_pos.y) | 0x10000);
	}
}

void PlayerGnap::useDeviceOnPlatypus() {
	_vm->_gnap->playSequence(makeRid(1, getSequenceId(gskPullOutDevice, _vm->_plat->_pos.x, _vm->_plat->_pos.y)));

	if (_vm->_plat->_idleFacing != kDirNone) {
		_vm->_gameSys->insertSequence(makeRid(1, 0x7D5), _vm->_plat->_id,
			makeRid(_vm->_plat->_sequenceDatNum, _vm->_plat->_sequenceId), _vm->_plat->_id,
			kSeqSyncWait, 0, 75 * _vm->_plat->_pos.x - _vm->_platGridX, 48 * _vm->_plat->_pos.y - _vm->_platGridY);
		_vm->_plat->_sequenceId = 0x7D5;
		_vm->_plat->_sequenceDatNum = 1;
	} else {
		_vm->_gameSys->insertSequence(makeRid(1, 0x7D4), _vm->_plat->_id,
			makeRid(_vm->_plat->_sequenceDatNum, _vm->_plat->_sequenceId), _vm->_plat->_id,
			kSeqSyncWait, 0, 75 * _vm->_plat->_pos.x - _vm->_platGridX, 48 * _vm->_plat->_pos.y - _vm->_platGridY);
		_vm->_plat->_sequenceId = 0x7D4;
		_vm->_plat->_sequenceDatNum = 1;
	}

	int newSequenceId = getSequenceId(gskUseDevice, 0, 0);
	_vm->_gameSys->insertSequence(makeRid(1, newSequenceId), _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
	_sequenceId = newSequenceId;
	_sequenceDatNum = 1;
}

void PlayerGnap::initBrainPulseRndValue() {
	_brainPulseRndValue = 2 * _vm->getRandom(10);
}

void PlayerGnap::playSequence(int sequenceId) {
	_vm->_timers[2] = _vm->getRandom(30) + 20;
	_vm->_timers[3] = 300;
	_vm->gnapIdle();
	_vm->_gameSys->insertSequence(sequenceId, _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqScale | kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
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
					kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
				_sequenceId = 0x7BD;
				_sequenceDatNum = 1;
			} else if (_idleFacing == kDirBottomLeft) {
				_vm->_gameSys->insertSequence(0x107BE, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
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
					kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
				_sequenceId = 0x7BD;
				_sequenceDatNum = 1;
			} else if (_idleFacing == kDirBottomLeft) {
				_vm->_gameSys->insertSequence(0x107BE, _id,
					makeRid(_sequenceDatNum, _sequenceId), _id,
					kSeqSyncWait, 0, 75 * _pos.x - _vm->_gnapGridX, 48 * _pos.y - _vm->_gnapGridY);
				_sequenceId = 0x7BE;
				_sequenceDatNum = 1;
			}
		}
	} else {
		_vm->_timers[2] = _vm->getRandom(30) + 20;
		_vm->_timers[3] = 300;
	}
}

/************************************************************************************************/

PlayerPlat::PlayerPlat(GnapEngine * vm) : Character(vm) {}

int PlayerPlat::getSequenceId(int kind, int gridX, int gridY) {
	// The original had 3 parameters, all always set to 0.
	// The code to handle the other values has been removed.

	int sequenceId = 0x7CB;

	if (_idleFacing != kDirNone) {
		sequenceId = 0x7CC;
		_idleFacing = kDirUnk4;
	}

	return sequenceId | 0x10000;
}

void PlayerPlat::playSequence(int sequenceId) {
	_vm->_gameSys->insertSequence(sequenceId, _id,
		makeRid(_sequenceDatNum, _sequenceId), _id,
		kSeqScale | kSeqSyncWait, 0, 75 * _pos.x - _vm->_platGridX, 48 * _pos.y - _vm->_platGridY);
	_sequenceId = ridToEntryIndex(sequenceId);
	_sequenceDatNum = ridToDatIndex(sequenceId);
}

void PlayerPlat::updateIdleSequence() {
	if (_actionStatus < 0 && _vm->_gnap->_actionStatus < 0) {
		if (_vm->_timers[0] > 0) {
			if (_vm->_timers[1] == 0) {
				_vm->_timers[1] = _vm->getRandom(20) + 30;
				int rnd = _vm->getRandom(10);
				if (_idleFacing != kDirNone) {
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
			_vm->platypusMakeRoom();
		}
	} else {
		_vm->_timers[0] = 100;
		_vm->_timers[1] = 35;
	}
}

void PlayerPlat::updateIdleSequence2() {
	if (_actionStatus < 0 && _vm->_gnap->_actionStatus < 0) {
		if (_vm->_timers[0]) {
			if (!_vm->_timers[1]) {
				_vm->_timers[1] = _vm->getRandom(20) + 30;
				if (_idleFacing != kDirNone) {
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
			_vm->platypusMakeRoom();
		}
	} else {
		_vm->_timers[0] = 100;
		_vm->_timers[1] = 35;
	}
}

} // End of namespace Gnap
