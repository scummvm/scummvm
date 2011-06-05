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
 * $URL: https://svn.scummvm.org:4444/svn/dreamweb/dreamweb.cpp $
 * $Id: dreamweb.cpp 79 2011-06-05 08:26:54Z eriktorbjorn $
 *
 */

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/EventRecorder.h"
#include "common/file.h"
#include "common/func.h"
#include "common/iff_container.h"
#include "common/system.h"
#include "common/timer.h"

#include "engines/util.h"

#include "audio/mixer.h"

#include "graphics/palette.h"

#include "dreamweb/dreamweb.h"

namespace DreamWeb {

DreamWebEngine::DreamWebEngine(OSystem *syst, const DreamWebGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	// Setup mixer
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, ConfMan.getInt("sfx_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, ConfMan.getInt("music_volume"));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, ConfMan.getInt("speech_volume"));

	_rnd = new Common::RandomSource("dreamweb");

	_vSyncInterrupt = false;

	_console = 0;
	DebugMan.addDebugChannel(kDebugAnimation, "Animation", "Animation Debug Flag");
	DebugMan.addDebugChannel(kDebugSaveLoad, "SaveLoad", "Track Save/Load Function");
}

DreamWebEngine::~DreamWebEngine() {
	DebugMan.clearAllDebugChannels();
	delete _console;
	delete _rnd;
}

// Let's see if it's a good idea to emulate VSYNC interrupts with a timer like
// this. There's a chance we'll miss interrupts, which could be countered by
// counting them instead of just flagging them, but we'll see...

static void vSyncInterrupt(void *refCon) {
	DreamWebEngine *vm = (DreamWebEngine *)refCon;

	if (!vm->isPaused()) {
		vm->setVSyncInterrupt(true);
	}
}

void DreamWebEngine::setVSyncInterrupt(bool flag) {
	_vSyncInterrupt = flag;
}

void DreamWebEngine::waitForVSync() {
	while (!_vSyncInterrupt) {
		_system->delayMillis(10);
	}
	setVSyncInterrupt(false);
	// doshake
	// dofade
}

Common::Error DreamWebEngine::run() {
	_console = new DreamWebConsole(this);

	Common::EventManager *event_manager = _system->getEventManager();
	getTimerManager()->installTimerProc(vSyncInterrupt, 1000000 / 60, this);

	do {
		uint32 frame_time = _system->getMillis();
		Common::Event event;
		while (event_manager->pollEvent(event)) {
			switch(event.type) {
			case Common::EVENT_RTL:
				return Common::kNoError;
			case Common::EVENT_MOUSEMOVE:
				_mouse = event.mouse;
				break;
			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					if (event.kbd.flags & Common::KBD_CTRL) {
						_console->attach();
						_console->onFrame();
					}
					break;
				default:
					break;
				}
				break;
			default:
				debug(0, "skipped event type %d", event.type);
			}
		}
	} while (!shouldQuit());

	getTimerManager()->removeTimerProc(vSyncInterrupt);

	return Common::kNoError;
}

} // End of namespace DreamWeb
