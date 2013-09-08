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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

/* AVALOT		The kernel of the program. */

#include "avalanche/avalanche.h"

#include "avalanche/graphics.h"
#include "avalanche/avalot.h"
#include "avalanche/gyro2.h"
#include "avalanche/animation.h"
#include "avalanche/gyro2.h"
#include "avalanche/lucerna2.h"
#include "avalanche/scrolls2.h"
#include "avalanche/dropdown2.h"
#include "avalanche/pingo2.h"
#include "avalanche/timeout2.h"
#include "avalanche/celer2.h"
#include "avalanche/enid2.h"
#include "avalanche/visa2.h"
#include "avalanche/closing2.h"

#include "common/str.h"
#include "common/textconsole.h"
#include "common/config-manager.h"



namespace Avalanche {



Avalot::Avalot(AvalancheEngine *vm) {
	_vm = vm;
}

void Avalot::handleKeyDown(Common::Event &event) {
	//if (keyboardclick)
	//	click();
	//
	// To be implemented later with the sounds, I assume.


	if ((Common::KEYCODE_F1 <= event.kbd.keycode) && (event.kbd.keycode <= Common::KEYCODE_F15))
		_vm->_parser->handleFunctionKey(event);
	else if ((32 <= event.kbd.ascii) && (event.kbd.ascii <= 128) && (event.kbd.ascii != 47))
		_vm->_parser->handleInputText(event);
	else
		switch (event.kbd.keycode) { // We can control Avvy with the numpad as well.
		case Common::KEYCODE_KP8:
			event.kbd.keycode = Common::KEYCODE_UP;
			break;
		case Common::KEYCODE_KP2:
			event.kbd.keycode = Common::KEYCODE_DOWN;
			break;
		case Common::KEYCODE_KP6:
			event.kbd.keycode = Common::KEYCODE_RIGHT;
			break;
		case Common::KEYCODE_KP4:
			event.kbd.keycode = Common::KEYCODE_LEFT;
			break;
		case Common::KEYCODE_KP9:
			event.kbd.keycode = Common::KEYCODE_PAGEUP;
			break;
		case Common::KEYCODE_KP3:
			event.kbd.keycode = Common::KEYCODE_PAGEDOWN;
			break;
		case Common::KEYCODE_KP7:
			event.kbd.keycode = Common::KEYCODE_HOME;
			break;
		case Common::KEYCODE_KP1:
			event.kbd.keycode = Common::KEYCODE_END;
			break;
		default:
			break;
	}

	switch (event.kbd.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP5:
		if (_vm->_gyro->_alive && _vm->_gyro->_dna._avvyIsAwake) {
			_vm->_animation->handleMoveKey(event); // Fallthroughs are intended.
			_vm->_lucerna->drawDirection();
			return;
		}
	case Common::KEYCODE_BACKSPACE:
		_vm->_parser->handleBackspace();
		break;
	case Common::KEYCODE_RETURN:
		_vm->_parser->handleReturn();
		break;
	default:
		break;
	}

	_vm->_lucerna->drawDirection();
}



void Avalot::setup() {
	_vm->_gyro->_mouse = _vm->_gyro->kMouseStateNo;
	_vm->_gyro->_letMeOut = false;
	_vm->_scrolls->resetScroll();
	CursorMan.showMouse(true);
	_vm->_gyro->_holdTheDawn = true;
	_vm->_lucerna->dusk();
	_vm->_gyro->_currentMouse = 177;
	_vm->_gyro->setMousePointerWait();
	_vm->_gyro->_dropsOk = true;
	_vm->_gyro->_mouseText = "";
	_vm->_gyro->_dropdownActive = false;
	_vm->_lucerna->loadDigits();
	_vm->_gyro->_cheat = false;
	_vm->_gyro->_cp = 0;
	_vm->_parser->_inputTextPos = 0;
	_vm->_parser->_quote = true;
	_vm->_gyro->_ledStatus = 177;
	_vm->_gyro->_defaultLed = 2;
	// TSkellern = 0; Replace with a more local variable sometime
	_vm->_gyro->_dna._direction = _vm->_gyro->kDirectionStopped;
	_vm->_gyro->_enidFilename = ""; // Undefined.
	_vm->_lucerna->drawToolbar();
	_vm->_scrolls->setReadyLight(2);
	for (byte i = 0; i < 3; i++)
		_vm->_gyro->_scoreToDisplay[i] = -1; // Impossible digits.

	_vm->_animation->loadtrip();

	_vm->_animation->get_back_loretta();
	_vm->_gyro->_holdTheDawn = false;
	_vm->_lucerna->dawn();
	_vm->_parser->_cursorState = false;
	_vm->_parser->cursorOn();
	_vm->_animation->tr[0]._speedX = _vm->_gyro->kWalk;
	_vm->_animation->newspeed();



	int16 loadSlot = Common::ConfigManager::instance().getInt("save_slot");
	if (loadSlot >= 0) {
		_vm->_gyro->_thinks = 2; // You always have money.
		_vm->_lucerna->thinkAbout(_vm->_gyro->kObjectMoney, Gyro::kThing);

		_vm->loadGame(loadSlot);
	} else {
		_vm->_gyro->isLoaded = false; // Set to true in _vm->loadGame().
		_vm->_gyro->newGame(); // No game was requested- load the default.

		_vm->_gyro->_soundFx = ! _vm->_gyro->_soundFx;
		_vm->_lucerna->fxToggle();
		_vm->_lucerna->thinkAbout(_vm->_gyro->kObjectMoney, Gyro::kThing);

		_vm->_visa->dixi('q', 83); // Info on the game, etc.
	}
}



void Avalot::run(Common::String arg) {
	setup();

	do {
		uint32 beginLoop = _vm->_system->getMillis();

		_vm->updateEvents(); // The event handler.



		_vm->_lucerna->_clock.update();
		_vm->_dropdown->updateMenu();
		_vm->_gyro->forceNumlock();
		_vm->_animation->get_back_loretta();
		_vm->_celer->updateBackgroundSprites();
		_vm->_animation->animLink();
		_vm->_lucerna->checkClick();
		_vm->_timeout->updateTimer();



#ifdef DEBUG
		// ONLY FOR TESTING!!!
		for (byte i = 0; i < _vm->_gyro->_lineNum; i++)
			_vm->_graphics->_surface.drawLine(_vm->_gyro->_lines[i]._x1, _vm->_gyro->_lines[i]._y1, _vm->_gyro->_lines[i]._x2, _vm->_gyro->_lines[i]._y2, _vm->_gyro->_lines[i].col);

		for (byte i = 0; i < _vm->_gyro->_fieldNum; i++) {
			if (_vm->_gyro->_fields[i]._x1 < 640)
				_vm->_graphics->_surface.frameRect(Common::Rect(_vm->_gyro->_fields[i]._x1, _vm->_gyro->_fields[i]._y1, _vm->_gyro->_fields[i]._x2, _vm->_gyro->_fields[i]._y2), kColorLightmagenta);
		}
		// ONLY FOR TESTING!!!
#endif


		_vm->_graphics->refreshScreen();  // TODO: Maybe it'll have a better place later. Move it there when it's needed.

		uint32 delay = _vm->_system->getMillis() - beginLoop;
		if (delay <= 55)
			_vm->_system->delayMillis(55 - delay); // Replaces _vm->_gyro->slowdown(); 55 comes from 18.2 Hz (B Flight).
	} while (!_vm->_gyro->_letMeOut && !_vm->shouldQuit());

	//if (logging)
	//	close(logfile);
	warning("STUB: Avalot::run()");

	_vm->_closing->exitGame();
}

} // End of namespace Avalanche
