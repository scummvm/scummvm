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

#include "engines/nancy/state/logo.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"

#include "common/error.h"
#include "common/system.h"
#include "common/events.h"
#include "common/str.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

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

void Logo::init() {
	_surf = new Graphics::Surface;

	if (!_engine->_res->loadImage("ciftree", _engine->_logos[0].name, *_surf))
		error("Failed to load %s", _engine->_logos[0].name.c_str());

	_state = kStartSound;
}

void Logo::startSound() {
	SoundDescription desc;
	desc.read(*_engine->getBootChunkStream("MSND"), SoundDescription::kMenu);
	_engine->sound->loadSound(desc);
	MSNDchannelID = desc.channelID;
	desc.read(*_engine->getBootChunkStream("BUOK"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);
	desc.read(*_engine->getBootChunkStream("BUDE"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);
	desc.read(*_engine->getBootChunkStream("BULS"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);
	desc.read(*_engine->getBootChunkStream("GLOB"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);
	desc.read(*_engine->getBootChunkStream("CURT"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);
	desc.read(*_engine->getBootChunkStream("CANT"), SoundDescription::kNormal);
	_engine->sound->loadSound(desc);

	_engine->sound->playSound(MSNDchannelID);

	_startTicks = _engine->_system->getMillis();
	_state = kRun;
}

void Logo::run() {
	switch (_runState) {
	case kBlit:
		_engine->_system->copyRectToScreen(_surf->getPixels(), _surf->pitch, 0, 0, _surf->w, _surf->h);
		_runState = kWait;
		break;
	case kWait:
		if (_engine->_system->getMillis() - _startTicks >= 7000 || (_engine->input->getInput().input & NancyInput::kLeftMouseButtonDown))
			_state = kStop;
	}
}

void Logo::stop() {
	_surf->free();
	delete _surf;

	// The original engine checks for N+D and N+C key combos here.
	// For the N+C key combo it looks for some kind of cheat file
	// to initialize the game state with.

	_engine->setGameState(NancyEngine::kScene);
	_engine->_system->fillScreen(0);
}

} // End of namespace State
} // End of namespace Nancy
