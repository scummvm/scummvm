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
#include "avalanche/gyro.h"
#include "avalanche/animation.h"
#include "avalanche/gyro.h"
#include "avalanche/lucerna.h"
#include "avalanche/scrolls.h"
#include "avalanche/menu.h"
#include "avalanche/pingo.h"
#include "avalanche/timer.h"
#include "avalanche/celer.h"
#include "avalanche/closing.h"

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
		if (_vm->_gyro->_alive && _vm->_gyro->_avvyIsAwake) {
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
	_vm->_gyro->init();

	_vm->_scrolls->resetScroll();
	_vm->_lucerna->dusk();
	_vm->_lucerna->loadDigits();

	_vm->_parser->_inputTextPos = 0;
	_vm->_parser->_quote = true;

	// TSkellern = 0; Replace with a more local variable sometime
	_vm->_lucerna->drawToolbar();
	_vm->_scrolls->setReadyLight(2);

	_vm->_animation->_direction = Animation::kDirStopped;
	_vm->_animation->loadAnims();

	_vm->_lucerna->dawn();
	_vm->_parser->_cursorState = false;
	_vm->_parser->cursorOn();
	_vm->_animation->_sprites[0]._speedX = _vm->_gyro->kWalk;
	_vm->_animation->updateSpeed();

	_vm->_menu->init();

	int16 loadSlot = Common::ConfigManager::instance().getInt("save_slot");
	if (loadSlot >= 0) {
		_vm->_gyro->_thinks = 2; // You always have money.
		_vm->_lucerna->thinkAbout(Gyro::kObjectMoney, Gyro::kThing);

		_vm->loadGame(loadSlot);
	} else {
		_vm->_gyro->_isLoaded = false; // Set to true in _vm->loadGame().
		_vm->_gyro->newGame(); // No game was requested- load the default.

		_vm->_gyro->_soundFx = !_vm->_gyro->_soundFx;
		_vm->_lucerna->fxToggle();
		_vm->_lucerna->thinkAbout(Gyro::kObjectMoney, Gyro::kThing);

		_vm->_scrolls->displayScrollChain('q', 83); // Info on the game, etc.
	}
}

void Avalot::run(Common::String arg) {
	setup();

	do {
		uint32 beginLoop = _vm->_system->getMillis();

		_vm->updateEvents(); // The event handler.

		_vm->_lucerna->_clock.update();
		_vm->_menu->update();
		_vm->_celer->updateBackgroundSprites();
		_vm->_animation->animLink();
		_vm->_lucerna->checkClick();
		_vm->_timer->updateTimer();

#ifdef DEBUG
		// ONLY FOR TESTING!!!
		for (int i = 0; i < _vm->_gyro->_lineNum; i++) {
			LineType *curLine = &_vm->_gyro->_lines[i];
			_vm->_graphics->_surface.drawLine(curLine->_x1, curLine->_y1, curLine->_x2, curLine->_y2, curLine->col);
		}

		for (int i = 0; i < _vm->_gyro->_fieldNum; i++) {
			FieldType *curField = &_vm->_gyro->_fields[i];
			if (curField->_x1 < 640)
				_vm->_graphics->_surface.frameRect(Common::Rect(curField->_x1, curField->_y1, curField->_x2, curField->_y2), kColorLightmagenta);
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
