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

#include "engines/nancy/state/mainmenu.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/button.h"

#include "common/config-manager.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::MainMenu);
}

namespace Nancy {
namespace State {

MainMenu::~MainMenu() {
	for (auto *button : _buttons) {
		delete button;
	}
}

void MainMenu::process() {
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
}

void MainMenu::onStateEnter(const NancyState::NancyState prevState) {
	registerGraphics();
}

bool MainMenu::onStateExit(const NancyState::NancyState nextState) {
	return true;
}

void MainMenu::registerGraphics() {
	_background.registerGraphics();

	for (auto *button : _buttons) {
		button->registerGraphics();
	}
}

void MainMenu::clearButtonState() {
	for (auto *button : _buttons) {
		button->_isClicked = false;
	}
}

void MainMenu::init() {
	_menuData = g_nancy->_menuData;
	assert(_menuData);

	_background.init(_menuData->_imageName);
	_background.registerGraphics();

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
	g_nancy->setMouseEnabled(true);

	if (!g_nancy->_sound->isSoundPlaying("MSND")) {
		g_nancy->_sound->playSound("MSND");
	}

	for (uint i = 0; i < _menuData->_buttonDests.size(); ++i) {
		_buttons.push_back(new UI::Button(5, _background._drawSurface,
			_menuData->_buttonDownSrcs[i], _menuData->_buttonDests[i],
			_menuData->_buttonHighlightSrcs.size() ? _menuData->_buttonHighlightSrcs[i] : Common::Rect(),
			_menuData->_buttonDisabledSrcs.size() ? _menuData->_buttonDisabledSrcs[i] : Common::Rect()));
		
		_buttons.back()->init();
		_buttons.back()->setVisible(false);
	}

	registerGraphics();

	// Disable continue if game was just started
	// Perhaps could be enabled always, and just load the latest save?
	if (!Scene::hasInstance()) {
		_buttons[3]->setDisabled(true);
	}

	_state = kRun;
}

void MainMenu::run() {
	NancyInput input = g_nancy->_input->getInput();

	if (_selected != -1) {
		input.input &= ~NancyInput::kLeftMouseButtonUp;
	}

	for (uint i = 0; i < _buttons.size(); ++i) {
		auto *button = _buttons[i];
		button->handleInput(input);
		if (_selected == -1 && button->_isClicked) {
			if (button->_isDisabled) {
				g_nancy->_sound->playSound("BUDE");
			} else {
				g_nancy->_sound->playSound("BUOK");
			}

			_selected = i;
		}
	}

	if (_selected != -1) {
		if (!g_nancy->_sound->isSoundPlaying("BUOK") && !g_nancy->_sound->isSoundPlaying("BUDE")) {
			if (_buttons[_selected]->_isDisabled) {
				_selected = -1;
				clearButtonState();
			} else {
				_state = kStop;
			}
		}
	}

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
}

void MainMenu::stop() {
	switch (_selected) {
	case 0:
		// Credits
		g_nancy->setState(NancyState::kCredits);
		break;
	case 1:
		// New Game
		if (Scene::hasInstance()) {
			NancySceneState.destroy(); // Destroy the existing Scene and create a new one
		}

		g_nancy->setState(NancyState::kScene);
		break;
	case 2:
		// Load and Save Game, TODO
		g_nancy->setState(NancyState::kLoadSave);
		break;
	case 3:
		// Continue
		g_nancy->setState(NancyState::kScene);
		break;
	case 4:
		// Second Chance
		if (Scene::hasInstance()) {
			NancySceneState.destroy(); // Destroy the existing Scene and create a new one
		}

		ConfMan.setInt("save_slot", g_nancy->getMetaEngine()->getMaximumSaveSlot(), Common::ConfigManager::kTransientDomain);
		g_nancy->setState(NancyState::kScene);
		break;
	case 5:
		// Game Setup
		g_nancy->setState(NancyState::kSetup);
		break;
	case 6:
		// Exit Game
		g_nancy->quitGame();
		break;
	case 7:
		// Help
		g_nancy->setState(NancyState::kHelp);
		break;
	}
}

} // End of namespace State
} // End of namespace Nancy
