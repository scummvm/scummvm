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

#include "common/scummsys.h"
#include "access/access.h"
#include "access/room.h"
#include "access/noctropolis/noctropolis_game.h"
#include "access/noctropolis/noctropolis_player.h"
#include "access/noctropolis/noctropolis_resources.h"

namespace Access {

namespace Noctropolis {

NoctropolisPlayer::NoctropolisPlayer(AccessEngine *vm, bool isStil) : Player(vm), _lastDirection(DOWN),
_animManager(nullptr), _isStil(isStil) {
	_game = (NoctropolisEngine *)vm;
}

NoctropolisPlayer::~NoctropolisPlayer() {
	delete _animManager;
}

void NoctropolisPlayer::load() {
	Player::load();

	// Overwrite game-specific values
	_playerOffset.x = _vm->_screen->_scaleTable1[20];
	_playerOffset.y = _vm->_screen->_scaleTable1[62];
	_leftDelta = -9;
	_rightDelta = 33;
	_upDelta = 5;
	_downDelta = -5;
	_scrollConst = 5;

	_sideWalkMin = 0;
	_sideWalkMax = 7;
	_upWalkMin = 8;
	_upWalkMax = 14;
	_downWalkMin = 15;
	_downWalkMax = 23;

	// playerPalette is configured in Player::load.
}

void NoctropolisPlayer::loadAnimation(int fileNum, int subFile) {
	if (!_animManager)
		_animManager = new AnimationManager(_vm);
	Resource *data = _vm->_files->loadFile(fileNum, subFile);
	_animManager->loadAnimations(data);
	delete data;
}

/*
 * Get the animation number for the given direction.
 * Noctropolis anim nums don't match the order of Direction in player.h.
 */
static int _animNumForDir(Direction dir) {
	switch (dir) {
	case UP:		return 0;
	case UPRIGHT: 	return 1;
	case RIGHT: 	return 2;
	case DOWNRIGHT: return 3;
	case DOWN: 		return 4;
	case DOWNLEFT: 	return 5;
	case LEFT: 		return 6;
	case UPLEFT:	return 7;
	default:		return -1;
	}
}

void NoctropolisPlayer::updatePlayerDirection() {
	if (_playerOff)
		return;

	int16 absDeltaX = abs(_moveTo.x - _rawPlayer.x);
	int16 absDeltaY = abs(_moveTo.y - _rawPlayer.y);

	int scaleEntry = (_vm->_room->_roomFlag & kRoomFlagTopView) ? 4 : 20;
	int16 minMove = _vm->_screen->_scaleTable1[scaleEntry];
	if (minMove < absDeltaX) {
		// Need to move in X
		if (minMove < absDeltaY) {
			if (_moveTo.y < _rawPlayer.y) {
				if (_moveTo.x < _rawPlayer.x) {
					_playerDirection = UPLEFT;
				} else {
					_playerDirection = UPRIGHT;
				}
			} else {
				if (_moveTo.x < _playerX) {
					_playerDirection = DOWNLEFT;
				} else {
					_playerDirection = DOWNRIGHT;
				}
			}
		} else {
			// Move in x only
			_rawPlayer.y = _moveTo.y;
			if (_moveTo.x < _rawPlayer.x) {
				_playerDirection = LEFT;
			} else {
				_playerDirection = RIGHT;
			}
		}
	} else if (minMove < absDeltaY) {
		// Move in y only
		_rawPlayer.x = _moveTo.x;
		if (_moveTo.y < _rawPlayer.y) {
			_playerDirection = UP;
		} else {
			_playerDirection = DOWN;
		}
	} else {
		// Delta < minMove, stop.
		_playerDirection = NONE;
		_rawPlayer.x = _moveTo.x;
		_rawPlayer.y = _moveTo.y;
	}
}

void NoctropolisPlayer::calcManScale() {
	// Note: original does _vm->_player->_playerOff check here, but that
	// makes no sense - should check for the current player's on/off value surely?

	// If we don't do this, Stil warps in at the wrong size/offset after we
	// defeat the succubus becuase her _playerOffset is never configured.
	if (!_playerOff) {
		calcManScale1();
	}
}

void NoctropolisPlayer::calcManScale1() {
	_vm->_scale = (_isStil ? _vm->_stilScaleOff : _vm->_manScaleOff);
	if (!(_vm->_room->_roomFlag & kRoomFlagTopView)) {
		if (_vm->_scale == 0) {
			_vm->_scale = ((((_rawPlayer.y - _vm->_scaleMaxY + _vm->_scaleN1) *
				_vm->_scaleT1 + (_vm->_scaleH2 << 8)) & 0xff00) / _vm->_scaleH1 * _vm->_scaleI) >> 8;
		}

		_vm->_screen->setScaleTable(_vm->_scale);
		_playerOffset.x = _vm->_screen->_scaleTable1[60];
		_playerOffset.y = _vm->_screen->_scaleTable1[_isStil ? 180 : 200];
	} else {
		_vm->_screen->setScaleTable(_vm->_scale);
		_playerOffset.x = _vm->_screen->_scaleTable1[6];
		_playerOffset.y = _vm->_screen->_scaleTable1[6];
	}
}

void NoctropolisPlayer::updateTimers() {
	if (_animManager)
		_animManager->updateTimers();
}

void NoctropolisPlayer::walk() {
	if (_playerOff)
		return;

	_vm->_curPlayer = this;

	updatePlayerDirection();

	calcManScale();

	if (_playerDirection == NONE) {
		// Idle animation (using last direction)
		_playerX = _rawPlayer.x;
		_playerY = _rawPlayer.y;
		_playerXLow = _playerYLow = 0;
		if (_animManager)
			_animManager->animate(_animNumForDir(_lastDirection) + 8);
		_playerMove = false;
	} else {
		// Move animation
		int dirAnimNum = _animNumForDir(_playerDirection);
		if (_playerDirection != _lastDirection || (_animManager && !_animManager->hasTimer())) {
			_lastDirection = _playerDirection;
			if (_animManager) {
				_animManager->popBackTimer();
				Animation *anim = _animManager->setAnimation(dirAnimNum);
				_animManager->setAnimTimer(anim);
			}
		}
		_playerX = _rawPlayer.x;
		_playerY = _rawPlayer.y;
		if (_animManager)
			_animManager->animate(dirAnimNum);
		_rawPlayer.x = _playerX;
		_rawPlayer.y = _playerY;

		calcManScale1();

		/*
		if (local_30 != (byte *)0x0) {
			local_48.animNum = dirAnimNum;
			local_48.animTable = _playerAnimation;
			local_48.somePtr = (byte *)&local_30;
			SetPlayerRelFrame((uchar **)local_48.somePtr,&local_48);
		}*/
	}
}

void NoctropolisPlayer::setDirFromScript(byte newDir) {
	// The opposite of _animNumForDir above.
	switch (newDir) {
	case 0: _playerDirection = UP;			break;
	case 1: _playerDirection = UPRIGHT;		break;
	case 2: _playerDirection = RIGHT; 		break;
	case 3: _playerDirection = DOWNRIGHT;	break;
	case 4: _playerDirection = DOWN;		break;
	case 5: _playerDirection = DOWNLEFT;	break;
	case 6: _playerDirection = LEFT;		break;
	case 7: _playerDirection = UPLEFT;		break;
	default: _playerDirection = NONE;		break;
	}
}


void NoctropolisPlayer::synchronize(Common::Serializer &s) {
	Player::synchronize(s);
	s.syncAsSint16LE(_moveTo.x);
	s.syncAsSint16LE(_moveTo.y);
	s.syncAsByte(_playerMove);
}


} // End of namespace Noctropolis

} // End of namespace Access
