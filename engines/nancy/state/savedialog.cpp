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
#include "engines/nancy/cursor.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/savedialog.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/button.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::SaveDialog);
}

namespace Nancy {
namespace State {

SaveDialog::~SaveDialog() {
	delete _yesButton;
	delete _noButton;
	delete _cancelButton;
}

void SaveDialog::process() {
	if (g_nancy->_sound->isSoundPlaying("BUOK")) {
		return;
	}

	switch (_state) {
	case kInit:
		init();
		// fall through
	case kRun:
		run();
		break;
	case kStop:
		stop();
		break;
	}

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
}

void SaveDialog::onStateEnter(const NancyState::NancyState prevState) {
	registerGraphics();
}

bool SaveDialog::onStateExit(const NancyState::NancyState nextState) {
	return true;
}

void SaveDialog::registerGraphics() {
	_background.registerGraphics();

	if (_yesButton) {
		_yesButton->registerGraphics();
	}

	if (_noButton) {
		_noButton->registerGraphics();
	}

	if (_cancelButton) {
		_cancelButton->registerGraphics();
	}
}

void SaveDialog::init() {
	_dialogData = g_nancy->_saveDialogData;
	assert(_dialogData);

	_background.init(_dialogData->_imageName);

	_yesButton = new UI::Button(1, _background._drawSurface, _dialogData->_yesDownSrc, _dialogData->_yesDest, _dialogData->_yesHighlightSrc);
	_noButton = new UI::Button(1, _background._drawSurface, _dialogData->_noDownSrc, _dialogData->_noDest, _dialogData->_noHighlightSrc);
	_cancelButton = new UI::Button(1, _background._drawSurface, _dialogData->_cancelDownSrc, _dialogData->_cancelDest, _dialogData->_cancelHighlightSrc);

	registerGraphics();

	_state = kRun;
}

void SaveDialog::run() {
	NancyInput input = g_nancy->_input->getInput();

	_yesButton->handleInput(input);
	if (_yesButton->_isClicked) {
		_selected = 0;
		_state = kStop;
		g_nancy->_sound->playSound("BUOK");
	}

	_noButton->handleInput(input);
	if (_noButton->_isClicked) {
		_selected = 1;
		_state = kStop;
		g_nancy->_sound->playSound("BUOK");
	}

	_cancelButton->handleInput(input);
	if (_cancelButton->_isClicked) {
		_selected = 2;
		_state = kStop;
		g_nancy->_sound->playSound("BUOK");
	}
}

void SaveDialog::stop() {
	switch (_selected) {
	case 0 :
		g_nancy->setState(NancyState::kLoadSave);
		break;
	case 1 :
		g_nancy->quitGame();
		break;
	case 2 :
		g_nancy->setToPreviousState();
		break;
	default:
		break;
	}
}

} // End of namespace State
} // End of namespace Nancy

