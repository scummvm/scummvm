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

#include "engines/nancy/state/setupmenu.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/ui/button.h"
#include "engines/nancy/ui/scrollbar.h"

#include "common/config-manager.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::SetupMenu);
}

namespace Nancy {
namespace State {

SetupMenu::~SetupMenu() {
	for (auto *tog : _toggles) {
		delete tog;
	}

	for (auto *scroll : _scrollbars) {
		delete scroll;
	}

	delete _exitButton;
}

void SetupMenu::process() {
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

void SetupMenu::onStateEnter(const NancyState::NancyState prevState) {
	registerGraphics();
}

bool SetupMenu::onStateExit(const NancyState::NancyState nextState) {
	return true;
}

void SetupMenu::registerGraphics() {
	_background.registerGraphics();

	for (auto *tog : _toggles) {
		tog->registerGraphics();
	}

	for (auto *scroll : _scrollbars) {
		scroll->registerGraphics();
	}

	if (_exitButton) {
		_exitButton->registerGraphics();
	}
}

const Common::String SetupMenu::getToggleConfManKey(uint id) {
	GameType gameType = g_nancy->getGameType();

	if (gameType == kGameTypeVampire) {
		// Note that toggle id 1 (interlaced video) is ignored since we don't support that option
		switch (id) {
		case 0 :
			return "subtitles";
		case 2 :
			return "player_speech";
		case 3 :
			return "character_speech";
		default:
			return "";
		}
	} else if (gameType <= kGameTypeNancy5) {
		switch (id) {
		case 0 :
			return "subtitles";
		case 1 :
			return "player_speech";
		case 2 :
			return "character_speech";
		default:
			return "";
		}
	} else {
		switch (id) {
		case 0 :
			return "subtitles";
		case 1 :
			return "auto_move";
		default:
			return "";
		}
	}
}

void SetupMenu::init() {
	_setupData = GetEngineData(SET);
	assert(_setupData);

	if (g_nancy->getGameType() == kGameTypeVampire) {
		// There is a setup.bmp an the top directory of the first disk,
		// which we need to avoid
		_background.init(Common::Path("ART/").joinInPlace(_setupData->_imageName));
	} else {
		_background.init(_setupData->_imageName);
	}

	_background.registerGraphics();

	g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
	g_nancy->setMouseEnabled(true);

	g_nancy->_sound->stopSound("MSND");

	for (uint i = 0; i < _setupData->_sounds.size(); ++i) {
		if (!g_nancy->_sound->isSoundPlaying(_setupData->_sounds[i])) {
			g_nancy->_sound->loadSound(_setupData->_sounds[i]);
			g_nancy->_sound->playSound(_setupData->_sounds[i]);
		}
	}

	for (uint i = 0; i < _setupData->_buttonDests.size() - 1; ++i) {
		_toggles.push_back(new UI::Toggle(5, _background._drawSurface,
			_setupData->_buttonDownSrcs[i], _setupData->_buttonDests[i]));

		_toggles.back()->init();
	}

	// Set toggle visibility
	for (uint i = 0; i < _toggles.size(); ++i) {
		_toggles[i]->setState(ConfMan.getBool(getToggleConfManKey(i), ConfMan.getActiveDomainName()));
	}

	for (uint i = 0; i < _setupData->_scrollbarSrcs.size(); ++i) {
		_scrollbars.push_back(new UI::Scrollbar(7, _setupData->_scrollbarSrcs[i],
			_background._drawSurface, Common::Point(_setupData->_scrollbarsCenterXPosL[i] + 1, _setupData->_scrollbarsCenterYPos[i]),
			_setupData->_scrollbarsCenterXPosR[i] + 1 - _setupData->_scrollbarsCenterXPosL[i] - 1, false));
		_scrollbars.back()->init();
		_scrollbars.back()->setVisible(true);
	}

	// Set scrollbar positions
	_scrollbars[0]->setPosition(ConfMan.getInt("speech_volume") / 255.0);
	_scrollbars[1]->setPosition(ConfMan.getInt("music_volume") / 255.0);
	_scrollbars[2]->setPosition(ConfMan.getInt("sfx_volume") / 255.0);

	_exitButton = new UI::Button(5, _background._drawSurface,
		_setupData->_buttonDownSrcs.back(), _setupData->_buttonDests.back());
	_exitButton->init();
	_exitButton->setVisible(false);

	registerGraphics();

	_state = kRun;
}

void SetupMenu::run() {
	NancyInput input = g_nancy->_input->getInput();

	for (uint i = 0; i < _scrollbars.size(); ++i) {
		auto *scroll = _scrollbars[i];

		float startPos = scroll->getPos();
		scroll->handleInput(input);
		float endPos = scroll->getPos();

		if (endPos != startPos) {
			Audio::Mixer::SoundType type = Audio::Mixer::SoundType::kSpeechSoundType;
			switch (i) {
			case 0 :
				type = Audio::Mixer::SoundType::kSpeechSoundType;
				ConfMan.setInt("speech_volume", endPos * 255, ConfMan.getActiveDomainName());
				break;
			case 1 :
				type = Audio::Mixer::SoundType::kMusicSoundType;
				ConfMan.setInt("music_volume", endPos * 255, ConfMan.getActiveDomainName());
				break;
			case 2 :
				type = Audio::Mixer::SoundType::kSFXSoundType;
				ConfMan.setInt("sfx_volume", endPos * 255, ConfMan.getActiveDomainName());
				break;
			default:
				break;
			}

			g_system->getMixer()->setVolumeForSoundType(type, endPos * 255);
		}
	}

	for (uint i = 0; i < _toggles.size(); ++i) {
		auto *tog = _toggles[i];
		tog->handleInput(input);
		if (tog->_stateChanged) {
			Common::String key = getToggleConfManKey(i);
			if (key.size()) {
				// Make sure we don't write an empty string as a key in ConfMan
				ConfMan.setBool(key, tog->_toggleState, ConfMan.getActiveDomainName());
			}
		}
	}

	if (_exitButton) {
		_exitButton->handleInput(input);

		if (_exitButton->_isClicked) {
			g_nancy->_sound->playSound("BUOK");
			_state = kStop;
		}
	}

	g_nancy->_cursor->setCursorType(CursorManager::kNormalArrow);
}

void SetupMenu::stop() {
	if (g_nancy->_sound->isSoundPlaying("BUOK")) {
		return;
	}

	for (auto &sound : _setupData->_sounds) {
		g_nancy->_sound->stopSound(sound);
	}

	ConfMan.flushToDisk();

	g_nancy->setToPreviousState();
}

} // End of namespace State
} // End of namespace Nancy
