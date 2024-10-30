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

#include "common/intrinsics.h"

#include "dgds/minigames/shell_game.h"
#include "dgds/dgds.h"
#include "dgds/globals.h"
#include "dgds/font.h"
#include "dgds/includes.h"
#include "dgds/sound.h"

namespace Dgds {

ShellGame::ShellGame() : _swapStatus(0), _revealPeaStep(0),
_currentPeaPosition(0), _lastPass(0), _distractStep(0),
_distractDelay(0), _state13Counter(0), _swapPea1(0), _swapPea2(0),
_lastSwapPea1(0), _swapMoveDist(0), _swapMoveStep(0), _swapCount(0),
_reverseDirection(0), _clockwise(false)
{
}

void ShellGame::init() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());
	assert(globals);
	_shellGameImg.reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
	_shellGameImg->loadBitmap("SHELLGM2.BMP");
	globals->setShellPea(engine->getRandom().getRandomNumber(2));
	_distractStep = 14;
	_distractDelay = 0;
	_state13Counter = 0;
}

static int16 _getState() {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	return globals->getNativeGameState();
}

static void _setState(int16 val) {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	globals->setNativeGameState(val);
}

static int16 _getPeaPosition() {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	return globals->getShellPea();
}

static void _setPeaPosition(int16 val) {
	HocGlobals *globals = static_cast<HocGlobals *>(DgdsEngine::getInstance()->getGameGlobals());
	globals->setShellPea(val);
}


void ShellGame::drawShellGameStr(int16 count, int16 x, int16 y) const {
	const Common::String countStr = Common::String::format("%d", count);
	DgdsEngine *engine = DgdsEngine::getInstance();
	const DgdsFont *fnt = engine->getFontMan()->getFont(FontManager::k4x5Font);
	fnt->drawString(&engine->getStoredAreaBuffer(), countStr, x, y, 50, 13);
}

void ShellGame::drawShells() const {
	const Common::Rect screenRect(SCREEN_WIDTH, SCREEN_HEIGHT);
	for (uint i = 0; i < 3; i++)
		_shellGameImg->drawBitmap(0, 98 + i * 55, 153, screenRect, DgdsEngine::getInstance()->getStoredAreaBuffer());
}

void ShellGame::shellGameTick() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());
	assert(globals);

	if (!_shellGameImg)
		init();

	// copy the background to the saved buffer (hack.. but the original does it too)
	engine->getStoredAreaBuffer().blitFrom(engine->getBackgroundBuffer());

	// Draw the shekels.
	drawShellGameStr(globals->getSheckels(), 131, 123);

	// Draw the bet
	drawShellGameStr(globals->getShellBet(), 201, 123);

	update();

	if (_revealPeaStep) {
		revealPea(false);
	} else {
		if (_swapStatus)
			swapShells(false);
		else
			drawShells();
	}
}


void ShellGame::update() {
	int16 state = _getState();
	if (state == 1 || state == 7) {
		_currentPeaPosition = _getPeaPosition();
		if (state == 7)
			_currentPeaPosition = _currentPeaPosition / 16;
		_setPeaPosition(_getPeaPosition() & 0xf);
		_revealPeaStep = 1;
		state++;
	} else if (state == 4 || state == 10) {
		_swapStatus = 1;
		_swapCount = 0;
		state++;
	} else if (state == 13) {
		if (_state13Counter) {
			_state13Counter--;
			if (!_state13Counter)
				state = 10;
		} else {
			_state13Counter = 20;
		}
	}
	_setState(state);
}


void ShellGame::revealPea(bool flag) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	const Common::Rect screenRect(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (_revealPeaStep == 1)
		engine->_soundPlayer->playSFX(147);

	uint16 x_offset = 55 * _currentPeaPosition;
	uint16 y_offset;
	if (_revealPeaStep > 40) {
		y_offset = 60 - _revealPeaStep;
	} else if (_revealPeaStep > 20) {
		y_offset = 20;
	} else {
		y_offset = _revealPeaStep;
	}

	// Draw the pea
	if (_currentPeaPosition == _getPeaPosition())
		_shellGameImg->drawBitmap(1, 112 + x_offset, 166, screenRect, engine->getStoredAreaBuffer());

	// Draw the shell revealing the pea
	_shellGameImg->drawBitmap(0, 98 + x_offset, 153 - y_offset, screenRect, engine->getStoredAreaBuffer());

	// Draw the other shells
	for (int i = 0; i < 3; i++) {
		if (i != _currentPeaPosition)
			_shellGameImg->drawBitmap(0, 98 + i * 55, 153, screenRect, engine->getStoredAreaBuffer());
	}

	if (!flag) {
		if (_lastPass) {
			_lastPass = false;
			_revealPeaStep = 0;
			_setState(_getState() + 1);
		} else {
			if (y_offset == 0)
				_lastPass = true;
			else
				_revealPeaStep++;
		}
	}
}


bool ShellGame::checkDistract() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	HocGlobals *globals = static_cast<HocGlobals *>(engine->getGameGlobals());

	int16 sheckels = globals->getSheckels();
	int16 bet = globals->getShellBet();
	if ((sheckels + bet >= 300)
			|| (sheckels + bet >= 150 && (engine->getRandom().getRandomNumber(256) & 0xc0))
			|| bet > 95
			|| (bet > 45 && (engine->getRandom().getRandomNumber(3)))) {
		_distractStep++;
		if (_distractStep > 21)
			_distractStep = 14;
		return true;
	}

	return false;
}



void ShellGame::setupSwap() {
	DgdsEngine *engine = DgdsEngine::getInstance();
	do {
		_swapPea1 = engine->getRandom().getRandomNumber(2);
	} while (_swapPea1 == _lastSwapPea1);

	_lastSwapPea1 = _swapPea1;

	if (_swapPea1 == 0) {
		_swapPea2 = 1;
	} else if (_swapPea1 == 1) {
			_swapPea2 = 2;
	} else {
		_swapPea1 = 0;
		_swapPea2 = 2;
	}

	if (_getPeaPosition() == _swapPea1) {
		_setPeaPosition(_swapPea2);
	} else if (_getPeaPosition() == _swapPea2) {
		_setPeaPosition(_swapPea1);
	}

	_swapMoveDist = (_swapPea2 - _swapPea1) * 55;
	_lastPass = false;
	_swapMoveStep = 0;
	_swapStatus = 2;
}


void ShellGame::swapShells(bool flag) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	const Common::Rect screenRect(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (!flag) {
		if (_swapStatus == 1) {
			engine->_soundPlayer->playSFX(145);
			setupSwap();
			_clockwise = (bool)(engine->getRandom().getRandomNumber(255) & 0x40);
			_reverseDirection = 0;
		}

		uint16 move_speed = (2 << engine->getDifficulty()) + 2;
		if (_getState() >= 10 && _getState() <= 12)
			move_speed++;
		if (!_lastPass) {
			_swapMoveStep += move_speed;
			if (abs(_swapPea1 - _swapPea2) != 1)
				_swapMoveStep += move_speed;
			if (_swapMoveStep > _swapMoveDist)
				_swapMoveStep = _swapMoveDist;
		}
	}

	uint16 xbase1 = 98 + _swapPea1 * 55;
	uint16 xbase2 = 98 + _swapPea2 * 55;

	double move = ((double)_swapMoveStep / _swapMoveDist) * M_PI;
	uint16 y_offset;
	if (abs(_swapPea1 - _swapPea2) == 1)
		y_offset = 26;
	else
		y_offset = 28;

	y_offset = (int16)(y_offset * sin(move));

	for (int i = 0; i < 3; i++) {
		if (i != _swapPea1 && i != _swapPea2)
			_shellGameImg->drawBitmap(0, 98 + i * 55, 153, screenRect, engine->getStoredAreaBuffer());
	}

	if (!flag) {
		if ((_reverseDirection == 0) && (_swapMoveStep >= (_swapMoveDist / 2))) {
			if (engine->getRandom().getRandomNumber(7)) {
				_reverseDirection = -1;
			} else {
				_reverseDirection = 1;
				if (_getPeaPosition() == _swapPea1)
					_setPeaPosition(_swapPea2);
				else if (_getPeaPosition() == _swapPea2)
					_setPeaPosition(_swapPea1);
			}
		}
	}

	uint16 x1,x2,y1,y2;
	if (_reverseDirection <= 0) {
		x1 = xbase1 + _swapMoveStep;
		x2 = xbase2 - _swapMoveStep;
	} else {
		x1 = xbase2 - _swapMoveStep;
		x2 = xbase1 + _swapMoveStep;
	}

	if (_clockwise) {
		y1 = 153 - y_offset;
		y2 = 153 + y_offset;
	} else {
		y1 = 153 + y_offset;
		y2 = 153 - y_offset;
	}

	_shellGameImg->drawBitmap(0, x1, y1, screenRect, engine->getStoredAreaBuffer());
	_shellGameImg->drawBitmap(0, x2, y2, screenRect, engine->getStoredAreaBuffer());

	if (!flag) {
		int16 state = _getState();
		if (_lastPass) {
			_lastPass = false;
			if (state == 11)
				_setState(0);
			else
				_setState(6);
			_swapStatus = 0;
		} else  if (_distractDelay) {
			_distractDelay--;
		} else if (_swapMoveStep == _swapMoveDist) {
			_swapCount++;
			if (state == 11) {
				_lastPass = true;
			} else if (_swapCount == 8 && checkDistract()) {
				 _setState(_distractStep);
				 _distractDelay = 20;
			} else {
				 if (_swapCount < 12 || engine->getRandom().getRandomNumber(7) || state >= 14)
					_swapStatus = 1;
				 else
					_lastPass = true;
			}
		}
	}
}


void ShellGame::shellGameEnd() {
	_shellGameImg.reset();
}

} // end namespace Dgds
