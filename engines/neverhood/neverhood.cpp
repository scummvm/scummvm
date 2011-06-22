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
 */

#include "common/algorithm.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "graphics/cursorman.h"

#include "engines/util.h"

#include "audio/mididrv.h"
#include "audio/mixer.h"

#include "neverhood/neverhood.h"

namespace Neverhood {

NeverhoodEngine::NeverhoodEngine(OSystem *syst, const NeverhoodGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	if (!_mixer->isReady()) {
		warning("Sound initialization failed.");
	}

	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));

	_rnd = new Common::RandomSource("neverhood");

}

NeverhoodEngine::~NeverhoodEngine() {
	delete _rnd;
}

Common::Error NeverhoodEngine::run() {
	// Initialize backend
	_system->beginGFXTransaction();
		initCommonGFX(false);
	_system->initSize(640, 480);
	_system->endGFXTransaction();

	_isSaveAllowed = false;

	return Common::kNoError;
}

void NeverhoodEngine::updateEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();

	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyState = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			_keyState = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
			_mouseX = event.mouse.x;
			_mouseY = event.mouse.y;
			break;
		/*			
		case Common::EVENT_LBUTTONDOWN:
			_buttonState |= kLeftButton;
			break;
		case Common::EVENT_LBUTTONUP:
			_buttonState &= ~kLeftButton;
			break;
		case Common::EVENT_RBUTTONDOWN:
			_buttonState |= kRightButton;
			break;
		case Common::EVENT_RBUTTONUP:
			_buttonState &= ~kRightButton;
			break;
		case Common::EVENT_QUIT:
			_system->quit();
			break;
		*/			
		default:
			break;
		}
	}
}

} // End of namespace Neverhood
