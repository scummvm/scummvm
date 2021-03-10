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

void Logo::init() {
	_surf = new Graphics::Surface;

	Common::SeekableReadStream *lg = NanEngine.getBootChunkStream("LG0");
	lg->seek(0);

	if (!NanEngine.resource->loadImage(lg->readString(), *_surf))
		error("Failed to load logo image");

	_state = kStartSound;
}

void Logo::startSound() {
	SoundDescription desc;
	desc.read(*NanEngine.getBootChunkStream("MSND"), SoundDescription::kMenu);
	NanEngine.sound->loadSound(desc);
	MSNDchannelID = desc.channelID;

	NanEngine.sound->playSound(MSNDchannelID);

	_startTicks = NanEngine._system->getMillis();
	_state = kRun;
}

void Logo::run() {
	switch (_runState) {
	case kBlit:
		NanEngine._system->copyRectToScreen(_surf->getPixels(), _surf->pitch, 0, 0, _surf->w, _surf->h);
		_runState = kWait;
		break;
	case kWait:
		if (NanEngine._system->getMillis() - _startTicks >= 7000 || (NanEngine.input->getInput().input & NancyInput::kLeftMouseButtonDown))
			_state = kStop;
	}
}

void Logo::stop() {
	_surf->free();
	delete _surf;

	// The original engine checks for N+D and N+C key combos here.
	// For the N+C key combo it looks for some kind of cheat file
	// to initialize the game state with.

	NanEngine.sound->stopSound(MSNDchannelID);

	NanEngine.setState(NancyEngine::kScene);
	NanEngine._system->fillScreen(0);
}

} // End of namespace State
} // End of namespace Nancy
