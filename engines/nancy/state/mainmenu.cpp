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

namespace Common {
DECLARE_SINGLETON(Nancy::State::MainMenu);
}

namespace Nancy {
namespace State {

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

void MainMenu::onStateExit() {
	destroy();
}

void MainMenu::init() {
	Common::SeekableReadStream *chunk = g_nancy->getBootChunkStream("MENU");
	chunk->seek(0);

	Common::String imageName;
	readFilename(*chunk, imageName);

	_background.init(imageName);
	_background.registerGraphics();

	g_nancy->_cursorManager->setCursorType(CursorManager::kNormalArrow);
	g_nancy->setMouseEnabled(true);

	if (!g_nancy->_sound->isSoundPlaying("MSND")) {
		g_nancy->_sound->playSound("MSND");
	}

	chunk->seek(0x20);

	// Unlike every other rect in the engine, these use int16 instead of int32
	for (uint i = 0; i < 8; ++i) {
		_destRects.push_back(Common::Rect());
		Common::Rect &rect = _destRects.back();
		rect.left = chunk->readSint16LE();
		rect.top = chunk->readSint16LE();
		rect.right = chunk->readSint16LE();
		rect.bottom = chunk->readSint16LE();
	}

	for (uint i = 0; i < 8; ++i) {
		_srcRects.push_back(Common::Rect());
		Common::Rect &rect = _srcRects.back();
		rect.left = chunk->readSint16LE();
		rect.top = chunk->readSint16LE();
		rect.right = chunk->readSint16LE();
		rect.bottom = chunk->readSint16LE();
	}

	_buttonDown.registerGraphics();

	_state = kRun;
}

void MainMenu::run() {
	NancyInput input = g_nancy->_input->getInput();

	_buttonDown.setVisible(false);

	if (input.input & NancyInput::kLeftMouseButtonUp) {
		for (uint i = 0; i < 8; ++i) {
			if (_destRects[i].contains(input.mousePos)) {
				if (i == 3 && !Scene::hasInstance()) {
					g_nancy->_sound->playSound("BUDE");
					_playedOKSound = false;
				} else {
					g_nancy->_sound->playSound("BUOK");
					_playedOKSound = true;
				}

				_selected = i;
				_state = kStop;

				_buttonDown._drawSurface.create(_background._drawSurface, _srcRects[i]);
				_buttonDown.moveTo(_destRects[i]);
				_buttonDown.setVisible(true);

				return;
			}
		}
	}
}

void MainMenu::stop() {
	if (!g_nancy->_sound->isSoundPlaying(_playedOKSound ? "BUOK" : "BUDE")) {
		switch (_selected) {
		case 0:
			// Credits
			g_nancy->setState(NancyState::kCredits);
			break;
		case 1:
			// New Game
			if (Scene::hasInstance()) {
				NancySceneState.destroy(); // Simply destroy the existing Scene and create a new one
			}

			g_nancy->setState(NancyState::kScene);
			break;
		case 2:
			// Load and Save Game, TODO
			_state = kRun;
			break;
		case 3:
			// Continue
			if (Scene::hasInstance()) {
				g_nancy->setState(NancyState::kScene);
			} else {
				_state = kRun;
			}
			break;
		case 4:
			// Second Chance
			if (!Scene::hasInstance()) {
				NancySceneState.process(); // run once to init the state
			}

			g_nancy->loadGameState(g_nancy->getAutosaveSlot());
			g_nancy->setState(NancyState::kScene);
			break;
		case 5:
			// Game Setup, TODO
			_state = kRun;
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
}

} // End of namespace State
} // End of namespace Nancy
