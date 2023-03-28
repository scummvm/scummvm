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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/config-manager.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"

#include "engines/nancy/state/logo.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Logo);
}

namespace Nancy {
namespace State {

void Logo::process() {
	switch (_state) {
	case kInit:
		init();
		break;
	case kStartSound:
		startSound();
		break;
	case kRun:
		run();
		break;
	case kStop:
		stop();
	}
}

void Logo::onStateExit() {
	destroy();
}

void Logo::init() {

	_logoImage.init(g_nancy->_imageChunks["LG0"].imageName);
	_logoImage.registerGraphics();

	_state = kStartSound;
}

void Logo::startSound() {
	g_nancy->_sound->playSound("MSND");

	_startTicks = g_nancy->getTotalPlayTime();
	_state = kRun;
}

void Logo::run() {
	if ((g_nancy->getTotalPlayTime() - _startTicks >= g_nancy->getStaticData().logoEndAfter) ||
		(g_nancy->_input->getInput().input & NancyInput::kLeftMouseButtonDown)) {
		_state = kStop;
	}
}

void Logo::stop() {
	// The original engine checks for N+D and N+C key combos here.
	// For the N+C key combo it looks for some kind of cheat file
	// to initialize the game state with.

	if (ConfMan.getBool("original_menus")) {
		g_nancy->setState(NancyState::kMainMenu);
	} else {
		g_nancy->setState(NancyState::kScene);
	}
}

} // End of namespace State
} // End of namespace Nancy
