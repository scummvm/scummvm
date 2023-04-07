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

#include "engines/nancy/nancy.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/help.h"

#include "engines/nancy/ui/button.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Help);
}

namespace Nancy {
namespace State {

Help::Help() :
		_state(kInit),
		_image(),
		_button(nullptr) {}

Help::~Help() {
	delete _button;
}

void Help::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kBegin:
		begin();
		// fall through
	case kRun:
		run();
		break;
	case kWait:
		wait();
		break;
	}
}

void Help::onStateEnter(const NancyState::NancyState prevState) {
	if (prevState == NancyState::kPause) {
		g_nancy->_sound->pauseSound("MSND", false);
	}
}

bool Help::onStateExit(const NancyState::NancyState nextState) {
	// Handle the GMM being called
	if (nextState == NancyState::kPause) {
		g_nancy->_sound->pauseSound("MSND", true);
		
		return false;
	} else {
		return true;
	}
}

void Help::init() {
	HELP *helpData = g_nancy->_helpData;
	assert(helpData);
	_image.init(helpData->imageName);

	_button = new UI::Button(5, _image._drawSurface, helpData->buttonSrc, helpData->buttonDest);
	_button->init();

	_state = kBegin;
}

void Help::begin() {
	if (!g_nancy->_sound->isSoundPlaying("MSND")) {
		g_nancy->_sound->playSound("MSND");
	}

	_image.registerGraphics();
	_button->registerGraphics();
	_image.setVisible(true);

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);

	_state = kRun;
}

void Help::run() {
	NancyInput input = g_nancy->_input->getInput();
	_button->handleInput(input);

	if (_button->_isClicked) {
		_button->_isClicked = false;
		g_nancy->_sound->playSound("BUOK");
		_buttonPressActivationTime = g_system->getMillis() + g_nancy->_bootSummary->buttonPressTimeDelay;
		_state = kWait;
	}
}

void Help::wait() {
	if (g_system->getMillis() > _buttonPressActivationTime) {
		g_nancy->setToPreviousState();
	}
}

} // End of namespace State
} // End of namespace Nancy
