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
#include "engines/nancy/graphics.h"

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
	return _destroyOnExit;
}

void MainMenu::registerGraphics() {
	_background.registerGraphics();

	for (auto *button : _buttons) {
		button->registerGraphics();
	}

	g_nancy->_graphics->redrawAll();
}

void MainMenu::clearButtonState() {
	for (auto *button : _buttons) {
		button->_isClicked = false;
	}
}

void MainMenu::init() {
	_menuData = GetEngineData(MENU);
	assert(_menuData);

	_background.init(_menuData->_imageName);
	_background.registerGraphics();

	g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
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
	} else {
		if (NancySceneState.isRunningAd()) {
			// Always destroy current state to make sure music starts again
			NancySceneState.destroy();

			if (ConfMan.hasKey("restore_after_ad", Common::ConfigManager::kTransientDomain)) {
				// Returning to running game, restore second chance
				ConfMan.setInt("save_slot", g_nancy->getMetaEngine()->getMaximumSaveSlot(), Common::ConfigManager::kTransientDomain);
			} else {
				// Not returning to running game, disable Continue button
				_buttons[3]->setDisabled(true);
			}
		}
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

	g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
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
		if (g_nancy->getEngineData("SDLG") && Nancy::State::Scene::hasInstance() && !g_nancy->_hasJustSaved) {
			if (!ConfMan.hasKey("sdlg_return", Common::ConfigManager::kTransientDomain)) {
				// Request the "Do you want to save before quitting" dialog
				ConfMan.setInt("sdlg_id", 0, Common::ConfigManager::kTransientDomain);
				_destroyOnExit = false;
				g_nancy->setState(NancyState::kSaveDialog);
			} else {
				// Dialog has returned
				_destroyOnExit = true;
				g_nancy->_graphics->suppressNextDraw();
				uint ret = ConfMan.getInt("sdlg_return", Common::ConfigManager::kTransientDomain);
				ConfMan.removeKey("sdlg_return", Common::ConfigManager::kTransientDomain);
				switch (ret) {
				case 0 :
					// "Yes" switches to LoadSave
					g_nancy->setState(NancyState::kLoadSave);
					break;
				case 1 :
					// "No" quits the game
					g_nancy->quitGame();

					// fall through
				case 2 :
					// "Cancel" keeps us in the main menu
					_selected = -1;
					for (uint i = 0; i < _buttons.size(); ++i) {
						_buttons[i]->_isClicked = false;
					}
					_state = kRun;
					break;
				default:
					break;
				}
			}
		} else {
			// Earlier games had no "Do you want to save before quitting" dialog, directly quit
			g_nancy->quitGame();

			// Fallback for when the ScummVM "Ask for confirmation on exit" option is enabled, and
			// the player clicks cancel
			_selected = -1;
			for (uint i = 0; i < _buttons.size(); ++i) {
				_buttons[i]->_isClicked = false;
			}
			_state = kRun;
			break;
		}

		break;
	case 7:
		// Help
		g_nancy->setState(NancyState::kHelp);
		break;
	case 8:
		// More Nancy Drew!
		if (Scene::hasInstance()) {
			// The second chance slot is used as temporary save. We make sure not to
			// overwrite it when selecting the ad button multiple times in a row.
			if (!ConfMan.hasKey("restore_after_ad", Common::ConfigManager::kTransientDomain)) {
				g_nancy->secondChance();
			}

			ConfMan.setBool("restore_after_ad", true, Common::ConfigManager::kTransientDomain);
			NancySceneState.destroy();
		}

		ConfMan.setBool("load_ad", true, Common::ConfigManager::kTransientDomain);
		g_nancy->setState(NancyState::kScene);
		break;
	}
}

} // End of namespace State
} // End of namespace Nancy
