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
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/events.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"

namespace Agi {

// Input

void PreAgiEngine::waitAnyKeyAnim() {
	waitAnyKey(true);
}

int PreAgiEngine::getSelection(int type) {
	Common::Event event;

	// Selection types:
	// 0: Y/N
	// 1: 1-9
	for (;;) {
		while (_eventMan->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_system->quit();
			case Common::EVENT_LBUTTONUP:
				if (type == 0)
					return 1;
			case Common::EVENT_RBUTTONUP:
				return 0;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_y:
					if (type == 0)
						return 1;
				case Common::KEYCODE_n:
					if (type == 0)
						return 0;
				case Common::KEYCODE_ESCAPE:
					if (type == 1)
						return 0;
				case Common::KEYCODE_1:
				case Common::KEYCODE_2:
				case Common::KEYCODE_3:
				case Common::KEYCODE_4:
				case Common::KEYCODE_5:
				case Common::KEYCODE_6:
				case Common::KEYCODE_7:
				case Common::KEYCODE_8:
				case Common::KEYCODE_9:
					if (type == 1)
						return event.kbd.keycode - Common::KEYCODE_1 + 1;
				default:
					if (type == 0) {
						return 2;
					} else {
						return 10;
					}
				}
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

bool PreAgiEngine::waitAnyKeyChoice() {
	Common::Event event;

	for (;;) {
		while (_eventMan->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_system->quit();
			case Common::EVENT_LBUTTONUP:
				return true;
			case Common::EVENT_RBUTTONUP:
				return false;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE: //Escape
					return false;
				default:
					return true;
				}
				break;
			default:
				break;
			}
		}
	}
}

void PreAgiEngine::waitAnyKey(bool anim) {
	Common::Event event;
	
	for (;;) {
		while (_eventMan->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_QUIT:
				_system->quit();
			case Common::EVENT_KEYDOWN:
			case Common::EVENT_LBUTTONUP:
			case Common::EVENT_RBUTTONUP:
				return;
			default:
				break;
			}
		}
		// TODO
		/*if (anim) {
			_game->Animate();
			UpdateScreen();
		}*/
	}
}

}
