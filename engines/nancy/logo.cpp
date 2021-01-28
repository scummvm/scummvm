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

#include "engines/nancy/logo.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/audio.h"
#include "engines/nancy/input.h"

#include "common/error.h"
#include "common/system.h"
#include "common/events.h"

#include "audio/audiostream.h"
#include "audio/mixer.h"

#include "graphics/surface.h"

namespace Nancy {

void LogoSequence::process() {
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

void LogoSequence::init() {
	_surf = new Graphics::Surface;

	if (!_engine->_res->loadImage("ciftree", _engine->_logos[0].name, *_surf))
		error("Failed to load %s", _engine->_logos[0].name.c_str());

	_state = kStartSound;
	_engine->_gameFlow.previousGameState = NancyEngine::kLogo;
}

void LogoSequence::startSound() {
	Common::SeekableReadStream *msnd = _engine->getBootChunkStream("MSND");
	char name[10];
	msnd->seek(0);
	msnd->read(name, 10);
	Common::String sname(name);
	_engine->sound->loadSound(sname, 0);
	_engine->sound->pauseSound(0, false);

	_startTicks = _engine->_system->getMillis();
	_state = kRun;
}

void LogoSequence::run() {
	switch (_runState) {
	case kBlit:
		_engine->_system->copyRectToScreen(_surf->getPixels(), _surf->pitch, 0, 0, _surf->w, _surf->h);
		_runState = kWait;
		break;
	case kWait:
		if (_engine->_system->getMillis() - _startTicks >= 7000 || (_engine->input->getInput(InputManager::kLeftMouseButtonDown)))
			_state = kStop;
	}
}

void LogoSequence::stop() {
	_surf->free();
	delete _surf;

	// The original engine checks for N+D and N+C key combos here.
	// For the N+C key combo it looks for some kind of cheat file
	// to initialize the game state with.

	_engine->_gameFlow.minGameState = NancyEngine::kScene;
	_engine->_system->fillScreen(0);
}

} // End of namespace Nancy