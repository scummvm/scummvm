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

#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"
#include "gui/message.h"

#include "supernova2/screen.h"
#include "supernova2/supernova2.h"
#include "supernova2/state.h"

namespace Supernova2 {


//StringId GameManager::guiCommands[] = {
//	kStringCommandGo, kStringCommandLook, kStringCommandTake, kStringCommandOpen, kStringCommandClose,
//	kStringCommandPress, kStringCommandPull, kStringCommandUse, kStringCommandTalk, kStringCommandGive
//};

//StringId GameManager::guiStatusCommands[] = {
//	kStringStatusCommandGo, kStringStatusCommandLook, kStringStatusCommandTake, kStringStatusCommandOpen, kStringStatusCommandClose,
//	kStringStatusCommandPress, kStringStatusCommandPull, kStringStatusCommandUse, kStringStatusCommandTalk, kStringStatusCommandGive
//};

GameManager::GameManager(Supernova2Engine *vm)
	: _vm(vm)
    , _mouseClickType(Common::EVENT_INVALID) {
	initRooms();
	changeRoom(INTRO);
	initState();
}

GameManager::~GameManager() {
}

void GameManager::initState() {
	_processInput = false;
	_guiEnabled = true;
	_animationEnabled = true;
	_mouseClicked = false;
	_keyPressed = false;
	_mouseX = -1;
	_mouseY = -1;
	_inventoryScroll = 0;
	_timerPaused = 0;
	_timePaused = false;
	_messageDuration = 0;
	_animationTimer = 0;

	_currentSentence = -1;
	for (int i = 0 ; i < 6 ; ++i) {
		_sentenceNumber[i] = -1;
//		_texts[i] = kNoString;
		_rows[i] = 0;
		_rowsStart[i] = 0;
	}

	_prevImgId = 0;
}

void GameManager::initRooms() {
	_rooms[INTRO] = new Intro(_vm, this);
	_rooms[AIRPORT] = new Airport(_vm, this);
}

void GameManager::updateEvents() {
	if (_animationEnabled && !_vm->_screen->isMessageShown() && _animationTimer == 0)
		_currentRoom->animation();

	_mouseClicked = false;
	_keyPressed = false;
	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyPressed = true;
			processInput(event.kbd);
			break;
		case Common::EVENT_LBUTTONUP:
			// fallthrough
		case Common::EVENT_RBUTTONUP:
			if (_currentRoom->getId() != INTRO)
				return;
			_mouseClicked = true;
			// fallthrough
		case Common::EVENT_MOUSEMOVE:
			_mouseClickType = event.type;
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			if (_guiEnabled)
				//processInput();
			break;
		default:
			break;
		}
	}
}
void GameManager::processInput(Common::KeyState &state) {
	_key = state;

	switch (state.keycode) {
	case Common::KEYCODE_F1:
		// help
		break;
	case Common::KEYCODE_F2:
		// show game doc
		break;
	case Common::KEYCODE_F3:
		// show game info
		break;
	case Common::KEYCODE_F4:
		//_vm->setTextSpeed();
		break;
	case Common::KEYCODE_F5:
		// load/save
		break;
	case Common::KEYCODE_x:
		if (state.flags & Common::KBD_ALT) {
			//if (_vm->quitGameDialog())
				_vm->quitGame();
		}
		break;
	case Common::KEYCODE_d:
		if (state.flags & Common::KBD_CTRL)
			_vm->_console->attach();
		break;
	default:
		break;
	}
}

void GameManager::getInput() {
	while (!_vm->shouldQuit()) {
		updateEvents();
		if (_mouseClicked || _keyPressed)
			break;
		g_system->updateScreen();
		g_system->delayMillis(_vm->_delay);
	}
}

void GameManager::wait(int ticks) {
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
	} while (g_system->getMillis() < end && !_vm->shouldQuit());
}

void GameManager::waitOnInput(int ticks) {
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
	} while (g_system->getMillis() < end && !_vm->shouldQuit() && !_keyPressed && !_mouseClicked);
}

bool GameManager::waitOnInput(int ticks, Common::KeyCode &keycode) {
	keycode = Common::KEYCODE_INVALID;
	uint32 end = g_system->getMillis() + ticksToMsec(ticks);
	do {
		g_system->delayMillis(_vm->_delay);
		updateEvents();
		g_system->updateScreen();
		if (_keyPressed) {
			keycode = _key.keycode;
			_key.reset();
			return true;
		} else if (_mouseClicked)
			return true;
	} while (g_system->getMillis() < end  && !_vm->shouldQuit());
	return false;
}

void GameManager::changeRoom(RoomId id) {
	_currentRoom = _rooms[id];
	_newRoom = true;
}

void GameManager::resetInputState() {
//	setObjectNull(_inputObject[0]);
//	setObjectNull(_inputObject[1]);
//	_inputVerb = ACTION_WALK;
	_processInput = false;
	_mouseClicked = false;
	_keyPressed = false;
	_key.reset();
	_mouseClickType = Common::EVENT_MOUSEMOVE;

	//processInput();
}

void GameManager::executeRoom() {
	if (_processInput && !_vm->_screen->isMessageShown() && _guiEnabled) {
//		handleInput();
		if (_mouseClicked) {
			Common::Event event;
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(0, 0);
			_vm->getEventManager()->pushEvent(event);
			event.type = Common::EVENT_MOUSEMOVE;
			event.mouse = Common::Point(_mouseX, _mouseY);
			_vm->getEventManager()->pushEvent(event);
		}

		resetInputState();
	}

	if (_guiEnabled) {
		if (!_vm->_screen->isMessageShown()) {
			g_system->fillScreen(kColorBlack);
			_vm->renderRoom(*_currentRoom);
		}
//		drawMapExits();
//		drawInventory();
//		drawStatus();
//		drawCommandBox();
	}

	//if (_vm->_screen->getViewportBrightness() == 0)
	//	_vm->paletteFadeIn();

	if (!_currentRoom->hasSeen() && _newRoom) {
		_newRoom = false;
		_currentRoom->onEntrance();
	}
}
}

