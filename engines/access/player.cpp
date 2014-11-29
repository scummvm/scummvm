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

#include "common/algorithm.h"
#include "common/textconsole.h"
#include "access/player.h"
#include "access/access.h"
#include "access/resources.h"
#include "access/amazon/amazon_player.h"

namespace Access {

Player *Player::init(AccessEngine *vm) {
	switch (vm->getGameID()) {
	case GType_Amazon:
		return new Amazon::AmazonPlayer(vm);
	default:
		return new Player(vm);
	}
}

Player::Player(AccessEngine *vm): Manager(vm), ImageEntry() {
	Common::fill(&_walkOffRight[0], &_walkOffRight[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffLeft[0], &_walkOffLeft[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffUp[0], &_walkOffUp[PLAYER_DATA_COUNT], 0);
	Common::fill(&_walkOffDown[0], &_walkOffDown[PLAYER_DATA_COUNT], 0);

	_playerSprites = nullptr;
	_playerSprites1 = nullptr;
	_manPal1 = nullptr;
	_frameNumber = 0;
	_monData = nullptr;
	_rawTempL = 0;
	_rawXTemp = 0;
	_rawYTempL = 0;
	_rawYTemp = 0;
	_playerXLow = 0;
	_playerX = 0;
	_playerYLow = 0;
	_playerY = 0;
	_frame = 0;
	_playerOff = false;
	_playerMove = false;
	_leftDelta = _rightDelta = 0;
	_upDelta = _downDelta = 0;
	_scrollConst = 0;
	_scrollFlag = false;
	_scrollThreshold = 0;
	_scrollAmount = 0;
	_scrollEnd = 0;
	_roomNumber = 0;
	_collideFlag = false;
	_move = NONE;
	_playerDirection = NONE;
	_xFlag = _yFlag = 0;
	_inactiveYOff = 0;
}

Player::~Player() {
	delete _playerSprites;
	delete[] _manPal1;
}

void Player::load() {
	_playerOffset.x = _vm->_screen->_scaleTable1[25];
	_playerOffset.y = _vm->_screen->_scaleTable1[67];
	_leftDelta = -3;
	_rightDelta = 33;
	_upDelta = 5;
	_downDelta = -10;
	_scrollConst = 5;

	for (int i = 0; i < PLAYER_DATA_COUNT; ++i) {
		_walkOffRight[i] = SIDEOFFR[i];
		_walkOffLeft[i] = SIDEOFFL[i];
		_walkOffUp[i] = SIDEOFFU[i];
		_walkOffDown[i] = SIDEOFFD[i];
		_walkOffUR[i].x = DIAGOFFURX[i];
		_walkOffUR[i].y = DIAGOFFURY[i];
		_walkOffDR[i].x = DIAGOFFDRX[i];
		_walkOffDR[i].y = DIAGOFFDRY[i];
		_walkOffUL[i].x = DIAGOFFULX[i];
		_walkOffUL[i].y = DIAGOFFULY[i];
		_walkOffDL[i].x = DIAGOFFDLX[i];
		_walkOffDL[i].y = DIAGOFFDLY[i];
	}

	_sideWalkMin = 0;
	_sideWalkMax = 7;
	_upWalkMin = 16;
	_upWalkMax = 23;
	_downWalkMin = 8;
	_downWalkMax = 15;
	_diagUpWalkMin = 0;
	_diagUpWalkMax = 7;
	_diagDownWalkMin = 0;
	_diagDownWalkMax = 7;

	_playerSprites = _playerSprites1;
	if (_manPal1) {
		Common::copy(_manPal1 + 0x270, _manPal1 + 0x270 + 0x60, _vm->_screen->_manPal);
	} else {
		Common::fill(_vm->_screen->_manPal, _vm->_screen->_manPal + 0x60, 0);
	}
}

void Player::loadSprites(const Common::String &name) {
	freeSprites();

	Resource *data = _vm->_files->loadFile(name);
	_playerSprites1 = new SpriteResource(_vm, data);
	delete data;
}

void Player::freeSprites() {
	delete _playerSprites;
	_playerSprites1 = nullptr;
	_playerSprites = nullptr;
}

void Player::removeSprite1() {
	if (_playerSprites1) {
		delete _playerSprites1;
		_playerSprites1 = nullptr;
	}
}

void Player::calcManScale() {
	if (!_vm->_manScaleOff) {
		_vm->_scale = ((((_rawPlayer.y - _vm->_scaleMaxY + _vm->_scaleN1) * 
			_vm->_scaleT1 + (_vm->_scaleH2 << 8)) & 0xff00) / _vm->_scaleH1 * _vm->_scaleI) >> 8;
		_vm->_screen->setScaleTable(_vm->_scale);

		_playerOffset.x = _vm->_screen->_scaleTable1[20];
		_playerOffset.y = _vm->_screen->_scaleTable1[67];
		_inactiveYOff = _playerOffset.y;
	}
}

void Player::walk() {
	_collideFlag = false;
	_playerDirection = NONE;
	
	if (_playerOff)
		return;
	else if (_vm->_timers[0]._flag) {
		plotCom3();
		return;
	}

	++_vm->_timers[0]._flag;
	switch (_move) {
	case UP:
		_playerMove = false;
		walkUp();
		break;
	case DOWN:
		_playerMove = false;
		walkDown();
		break;
	case LEFT:
		_playerMove = false;
		walkLeft();
		break;
	case RIGHT:
		_playerMove = false;
		walkRight();
		break;
	case UPLEFT:
		_playerMove = false;
		walkUpLeft();
		break;
	case DOWNLEFT:
		_playerMove = false;
		walkDownLeft();
		break;
	case UPRIGHT:
		_playerMove = false;
		walkUpRight();
		break;
	case DOWNRIGHT:
		_playerMove = false;
		walkDownRight();
		break;
	default:
		checkMove();
		break;
	}
}

void Player::calcPlayer() {
	Screen &scr = *_vm->_screen;
	scr._bufferStart.x = (scr._scrollCol << 4) + scr._scrollX;
	scr._bufferStart.y = (scr._scrollRow << 4) + scr._scrollY;
	_playerX = _rawPlayer.x - scr._bufferStart.x;
	_playerY = _rawPlayer.y - scr._bufferStart.y;
}

void Player::walkUp() {
	if (_frame > _upWalkMax || _frame < _upWalkMin)
		_frame = _upWalkMin;

	_playerDirection = UP;
	int walkOff = _walkOffUp[_frame - _upWalkMin];
	int tempL = _rawPlayerLow.y - _vm->_screen->_scaleTable2[walkOff];
	_rawYTempL = (byte)tempL;
	int yTemp = _rawPlayer.y - _vm->_screen->_scaleTable1[walkOff] - 
		(tempL < 0 ? 1 : 0);
	_rawYTemp = yTemp;
	_rawXTemp = _rawPlayer.x;

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.y = _rawYTempL;
		
		calcManScale();

		// This code looks totally useless as 'si' is unconditionally set in plotCom
		//if (_vm->_currentMan != 3 && (_frame == 17 || _frame == 21))
		//	warning("TODO: walkUp - si = 0?");

		if (++_frame > _upWalkMax)
			_frame = _upWalkMin;

		plotCom(0);
	}
}

void Player::walkDown() {
	if (_frame > _downWalkMax || _frame < _downWalkMin)
		_frame = _downWalkMin;

	_playerDirection = DOWN;
	int walkOff = _walkOffDown[_frame - _downWalkMin];
	int tempL = _vm->_screen->_scaleTable2[walkOff] + _rawPlayerLow.y;
	_rawYTempL = (byte)tempL;
	_rawYTemp = _vm->_screen->_scaleTable1[walkOff] + _rawPlayer.y + (tempL >= 0x100 ? 1 : 0);
	_rawXTemp = _rawPlayer.x;

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.y = _rawYTempL;

		calcManScale();

		// This code looks totally useless as 'si' is unconditionally set in plotCom
		//if (_vm->_currentMan != 3 && (_frame == 10 || _frame == 14))
		//	warning("TODO: walkDown - si = 0?");

		if (++_frame > _downWalkMax)
			_frame = _downWalkMin;

		plotCom(0);
	}
}

void Player::walkLeft() {
	if (_frame > _sideWalkMax || _frame < _sideWalkMin)
		_frame = _sideWalkMin;

	_playerDirection = LEFT;

	bool flag = _scrollEnd == 1;
	if (!flag) {
		calcPlayer();
		flag = (_playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		int walkOffset = _walkOffLeft[_frame - _sideWalkMin];
		int tempL = _rawPlayerLow.x - _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[walkOffset] -
			(tempL < 0 ? 1 : 0);
	} else {
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[_scrollConst];
	}	
	_rawYTemp = _rawPlayer.y;

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayerLow.x = _rawTempL;
		++_frame;

		// This code looks totally useless as 'si' is unconditionally set in plotCom1
		//if (_vm->_currentMan != 3 && (_frame == 1 || _frame == 5))
		//	warning("TODO: walkLeft - si = 0?");

		if (_frame > _sideWalkMax)
			_frame = _sideWalkMin;

		plotCom1();
	}
}

void Player::walkRight() {
	if (_frame > _sideWalkMax || _frame < _sideWalkMin)
		_frame = _sideWalkMin;

	_playerDirection = RIGHT;

	bool flag = _scrollEnd == 2;
	if (!flag) {
		calcPlayer();
		flag = (_vm->_screen->_clipWidth - _playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		int walkOffset = _walkOffRight[_frame - _sideWalkMin];
		int tempL = _rawPlayerLow.x + _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[walkOffset] +
			(tempL >= 0x100 ? 1 : 0);
	} else {
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[_scrollConst];
	}
	_rawYTemp = _rawPlayer.y;

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayerLow.x = _rawTempL;
		++_frame;

		// Useless check removed
		if (_frame > _sideWalkMax)
			_frame = _sideWalkMin;

		plotCom(0);
	}
}

void Player::walkUpLeft() {
	if (_frame > _diagUpWalkMax || _frame < _diagUpWalkMin)
		_frame = _diagUpWalkMin;

	_playerDirection = UPLEFT;

	int walkOffset, tempL;
	bool flag = _scrollEnd == 1;
	if (!flag) {
		calcPlayer();
		flag = (_playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		walkOffset = _walkOffUL[_frame - _diagUpWalkMin].x;
		tempL = _rawPlayerLow.x - _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[walkOffset] -
			(tempL < 0 ? 1 : 0);
	} else {
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[_scrollConst];
	}
	
	walkOffset = _walkOffUL[_frame - _diagUpWalkMin].y;
	tempL = _rawPlayerLow.y - _vm->_screen->_scaleTable2[walkOffset];
	_rawYTempL = (byte)tempL;
	_rawYTemp = _rawPlayer.y - _vm->_screen->_scaleTable1[walkOffset] -
		(tempL < 0 ? 1 : 0);;

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.x = _rawTempL;
		_rawPlayerLow.y = _rawYTempL;

		++_frame;
		calcManScale();

		// This code looks totally useless as 'si' is unconditionally set in plotCom1
		//if (_vm->_currentMan != 3 && (_frame == 1 || _frame == 5))
		//	warning("TODO: walkUpLeft - si = 0?");

		if (_frame > _diagUpWalkMax)
			_frame = _diagUpWalkMin;

		plotCom1();
	}
}

void Player::walkDownLeft() {
	if (_frame > _diagDownWalkMax || _frame < _diagDownWalkMin)
		_frame = _diagDownWalkMin;

	_playerDirection = DOWNLEFT;

	int walkOffset, tempL;
	bool flag = _scrollEnd == 1;
	if (!flag) {
		calcPlayer();
		flag = (_playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		walkOffset = _walkOffDL[_frame - _sideWalkMin].x;
		tempL = _rawPlayerLow.x - _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[walkOffset] -
			(tempL < 0 ? 1 : 0);
	} else {
		_rawXTemp = _rawPlayer.x - _vm->_screen->_scaleTable1[_scrollConst];
	}

	walkOffset = _walkOffDL[_frame - _diagDownWalkMin].y;
	tempL = _rawPlayerLow.y + _vm->_screen->_scaleTable2[walkOffset];
	_rawYTempL = (byte)tempL;
	_rawYTemp = _rawPlayer.y + _vm->_screen->_scaleTable1[walkOffset] +
		(tempL >= 0x100 ? 1 : 0);

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.x = _rawTempL;
		_rawPlayerLow.y = _rawYTempL;

		++_frame;
		calcManScale();
		
		// This code looks totally useless as 'si' is unconditionally set in plotCom1
		//if (_vm->_currentMan != 3 && (_frame == 1 || _frame == 5))
		//	warning("TODO: walkDownLeft - si = 0?");

		if (_frame > _diagDownWalkMax)
			_frame = _diagDownWalkMin;

		plotCom1();
	}
}

void Player::walkUpRight() {
	if (_frame > _diagUpWalkMax || _frame < _diagUpWalkMin)
		_frame = _diagUpWalkMin;

	_playerDirection = UPRIGHT;

	int walkOffset, tempL;
	bool flag = _scrollEnd == 1;
	if (!flag) {
		calcPlayer();
		flag = (_vm->_screen->_clipWidth - _playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		walkOffset = _walkOffUR[_frame - _diagUpWalkMin].x;
		tempL = _rawPlayerLow.x + _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[walkOffset] +
			(tempL >= 0x100 ? 1 : 0);
	} else {
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[_scrollConst];
	}

	walkOffset = _walkOffUL[_frame - _diagUpWalkMin].y;
	tempL = _rawPlayerLow.y - _vm->_screen->_scaleTable2[walkOffset];
	_rawYTempL = (byte)tempL;
	_rawYTemp = _rawPlayer.y - _vm->_screen->_scaleTable1[walkOffset] -
		(tempL < 0 ? 1 : 0);

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.x = _rawTempL;
		_rawPlayerLow.y = _rawYTempL;

		++_frame;
		calcManScale();

		// This code looks totally useless as 'si' is unconditionally set in plotCom
		//if (_vm->_currentMan != 3 && (_frame == 1 || _frame == 5))
		//	warning("TODO: walkUpRight - si = 0?");

		if (_frame > _diagUpWalkMax)
			_frame = _diagUpWalkMin;

		plotCom(0);
	}
}

void Player::walkDownRight() {
	if (_frame > _diagDownWalkMax || _frame < _diagDownWalkMin)
		_frame = _diagDownWalkMin;

	_playerDirection = DOWNRIGHT;

	int walkOffset, tempL;
	bool flag = _scrollEnd == 1;
	if (!flag) {
		calcPlayer();
		flag = (_playerX - _vm->_screen->_scaleTable1[_scrollConst] -
			_vm->_player->_scrollThreshold) > 0;
	}
	if (flag) {
		walkOffset = _walkOffUR[_frame - _sideWalkMin].x;
		tempL = _rawPlayerLow.x + _vm->_screen->_scaleTable2[walkOffset];
		_rawTempL = (byte)tempL;
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[walkOffset] +
			(tempL >= 0x100 ? 1 : 0);
	}
	else {
		_rawXTemp = _rawPlayer.x + _vm->_screen->_scaleTable1[_scrollConst];
	}

	walkOffset = _walkOffDR[_frame - _diagDownWalkMin].y;
	tempL = _rawPlayerLow.y - _vm->_screen->_scaleTable2[walkOffset];
	_rawYTempL = (byte)tempL;
	_rawYTemp = _rawPlayer.y + _vm->_screen->_scaleTable1[walkOffset] +
		(tempL >= 0x100 ? 1 : 0);

	if (_vm->_room->codeWalls()) {
		plotCom2();
	} else {
		_rawPlayer.x = _rawXTemp;
		_rawPlayer.y = _rawYTemp;
		_rawPlayerLow.x = _rawTempL;
		_rawPlayerLow.y = _rawYTempL;

		++_frame;
		calcManScale();

		// This code looks totally useless as 'si' is unconditionally set in plotCom1
		//if (_vm->_currentMan != 3 && (_frame == 1 || _frame == 5))
		//	warning("TODO: walkDownRight - si = 0?");

		if (_frame > _diagDownWalkMax)
			_frame = _diagDownWalkMin;

		plotCom1();
	}
}

void Player::checkMove() {
	if (_playerMove) {
		if (_xFlag == 0 && _yFlag == 0) {
			int xp = (_playerOffset.x / 2) + _rawPlayer.x - _moveTo.x;
			if (xp < 0)
				xp = -xp;
			int yp = _rawPlayer.y - _moveTo.y;
			if (yp < 0)
				yp = -yp;

			if (xp >= yp)
				_xFlag = 1;
			else
				_yFlag = 1;
		}

		if (_yFlag == 1) {
			int yd = _rawPlayer.y - _moveTo.y;
			if ((yd >= 0 && yd <= _upDelta) || (yd < 0 && -yd <= _upDelta)) {
				++_yFlag;
				if (_xFlag) {
					_playerMove = false;
					_xFlag = _yFlag = 0;
				} else {
					++_xFlag;
				}
			} else {
				if (yd >= 0)
					walkUp();
				else
					walkDown();

				if (_collideFlag) {
					_playerMove = false;
					_xFlag = _yFlag = 0;
				}
			}
		} else if (_xFlag == 1) {
			int xd = (_playerOffset.x / 2) + _rawPlayer.x - _moveTo.x;
			if ((xd >= 0 && xd <= -_leftDelta) || (xd < 0 && -xd <= -_leftDelta)) {
				++_xFlag;

				if (_yFlag) {
					_playerMove = false;
					_xFlag = _yFlag = 0;
				}
			} else {
				if (xd >= 0)
					walkLeft();
				else
					walkRight();

				if (_collideFlag) {
					_playerMove = false;
					_xFlag = _yFlag = 0;
				}
			}
		} else if (!_yFlag) {
			++_yFlag;
		} else {
			_playerMove = false;
			_xFlag = _yFlag = 0;
		}
	}

	plotCom3();
}

void Player::plotCom(int flags) {
	_flags &= ~2;
	_flags &= ~8;
	_flags |= flags;

	plotCom3();
}

void Player::plotCom1() {
	plotCom(2);
}

void Player::plotCom2() {
	// WORKAROUND: Amazon has at least one cutscene with the player not properly turned off
	if (!_playerOff && _spritesPtr != nullptr)
		_vm->_images.addToList(*this);
}

void Player::plotCom3() {
	// Update the base ImageEntry fields for the player
	_position.x = _rawPlayer.x;
	_position.y = _rawPlayer.y - _playerOffset.y;
	_offsetY = _playerOffset.y;
	_spritesPtr = _playerSprites;
	_frameNumber = _frame;

	plotCom2();
}

void Player::checkScroll() {
	_scrollFlag = false;
	if (_playerDirection == NONE)
		return;

	if ((_playerDirection == UPLEFT || _playerDirection == DOWNLEFT ||
			_playerDirection == LEFT) && _playerX <= _scrollThreshold) {
		// Scroll right
		if (!scrollRight()) {
			if (_playerDirection == DOWNLEFT)
				goto scrollUp;
			
			return;
		}
	} else if ((_playerDirection == UPRIGHT || _playerDirection == DOWNRIGHT ||
			_playerDirection == RIGHT) && (_vm->_screen->_clipWidth -
			_playerX - _scrollThreshold) <= 0) {
		// Scroll left
		if (!scrollLeft()) {
			if (_playerDirection == DOWNRIGHT)
				goto scrollUp;
	
			return;
		}
	}

	if ((_playerDirection == UPRIGHT || _playerDirection == UPLEFT ||
			_playerDirection == UP) && _playerY <= _scrollThreshold) {
		scrollDown();
	} else {
scrollUp:
		if ((_playerDirection == DOWNRIGHT || _playerDirection == DOWNLEFT ||
			_playerDirection == DOWN) && (_vm->_screen->_clipHeight -
			_playerY - _scrollThreshold) <= 0) {
			// Scroll up
			if (scrollUp()) {
				_scrollEnd = 4;
				_vm->_screen->_scrollY &= TILE_HEIGHT;
				_scrollFlag = true;
			}
		}
	}
}

bool Player::scrollUp() {
	_scrollAmount = -(_vm->_screen->_clipHeight - _playerY - _scrollThreshold);
	if ((_vm->_screen->_scrollRow + _vm->_screen->_vWindowHeight) >=
			_vm->_room->_playFieldHeight)
		return true;

	_scrollFlag = true;
	_vm->_screen->_scrollY = _vm->_screen->_scrollY + _scrollAmount;

	while (_vm->_screen->_scrollY >= TILE_HEIGHT && !_vm->shouldQuit()) {
			_vm->_screen->_scrollY -= TILE_HEIGHT;
		++_vm->_screen->_scrollRow;
		_vm->_buffer1.moveBufferUp();

		_vm->_room->buildRow(_vm->_screen->_scrollRow + _vm->_screen->_vWindowHeight,
			_vm->_screen->_vWindowLinesTall);

		if ((_vm->_screen->_scrollRow + _vm->_screen->_vWindowHeight) >=
			_vm->_room->_playFieldHeight)
			return true;

		if (_vm->_screen->_scrollY <= TILE_HEIGHT)
			return false;
	}

	return false;
}

bool Player::scrollDown() {
	_scrollAmount = -(_playerY - _scrollThreshold);
	_scrollFlag = true;
	_vm->_screen->_scrollY -= _scrollAmount;
	if (_vm->_screen->_scrollY >= 0)
		return true;

	do {
		_vm->_screen->_scrollY += TILE_HEIGHT;
		if (--_vm->_screen->_scrollRow < 0)
			break;

		_vm->_buffer1.moveBufferDown();
		_vm->_room->buildRow(_vm->_screen->_scrollRow, 0);

		if (_vm->_screen->_scrollY >= 0)
			return false;
	} while (!_vm->shouldQuit());

	_scrollEnd = 3;
	_vm->_screen->_scrollY = 0;
	_vm->_screen->_scrollRow = 0;
	return true;
}

bool Player::scrollLeft() {
	Screen &screen = *_vm->_screen;
	_scrollAmount = -(screen._clipWidth - _playerX - _scrollThreshold);
	if ((screen._scrollCol + screen._vWindowWidth) == _vm->_room->_playFieldWidth) {
		_scrollEnd = 2;
		screen._scrollX = 0;
		_scrollFlag = true;
		return true;
	} else {
		_scrollFlag = true;
		screen._scrollX = screen._scrollX + _scrollAmount;

		do {
			if (screen._scrollX < TILE_WIDTH)
				return true;

			screen._scrollX -= TILE_WIDTH;
			++screen._scrollCol;
			_vm->_buffer1.moveBufferLeft();
			_vm->_room->buildColumn(screen._scrollCol + screen._vWindowWidth,
				screen._vWindowBytesWide);
		} while (!_vm->shouldQuit() && (screen._scrollX >= TILE_WIDTH));

		return (_playerDirection == UPRIGHT);
	}
}

bool Player::scrollRight() {
	_scrollAmount = -(_playerX - _scrollThreshold);
	_scrollFlag = true;
	_vm->_screen->_scrollX -= _scrollAmount;

	if (_vm->_screen->_scrollX < 0) {
		do {
			_vm->_screen->_scrollX += TILE_WIDTH;
			if (--_vm->_screen->_scrollCol < 0) {
				_scrollEnd = true;
				_vm->_screen->_scrollX = 0;
				_vm->_screen->_scrollCol = 0;
				return true;
			}

			_vm->_buffer1.moveBufferRight();
			_vm->_room->buildColumn(_vm->_screen->_scrollCol, 0);
		} while (!_vm->shouldQuit() && (_vm->_screen->_scrollX < 0));

		return false;
	}

	return true;
}

void Player::synchronize(Common::Serializer &s) {
	s.syncAsUint16LE(_roomNumber);
	s.syncAsSint16LE(_rawPlayerLow.x);
	s.syncAsSint16LE(_rawPlayer.x);
	s.syncAsSint16LE(_rawPlayerLow.y);
	s.syncAsSint16LE(_rawPlayer.y);
}

} // End of namespace Access
