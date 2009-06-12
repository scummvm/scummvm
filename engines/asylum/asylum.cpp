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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "common/stream.h"
#include "sound/audiostream.h"
#include "sound/wave.h"
#include "asylum/screen.h"
#include "asylum/asylum.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, Common::Language language)
    : Engine(system) {

    Common::File::addDefaultDirectory(_gameDataDir.getChild("Data"));
    Common::File::addDefaultDirectory(_gameDataDir.getChild("Vids"));
	Common::File::addDefaultDirectory(_gameDataDir.getChild("Music"));

    _eventMan->registerRandomSource(_rnd, "asylum");
}

AsylumEngine::~AsylumEngine() {
    //Common::clearAllDebugChannels();
    delete _screen;
	delete _resMgr;
}

Common::Error AsylumEngine::run() {
    Common::Error err;
    err = init();
    if (err != Common::kNoError)
            return err;
    return go();
}

// Will do the same as subroutine at address 0041A500
Common::Error AsylumEngine::init() {
	// initialize engine objects

	_screen = new Screen(_system);
	_resMgr = new ResourceManager(this);

	// initializing game
	// TODO: save dialogue key codes into sntrm_k.txt (need to figure out why they use such thing)
	// TODO: get hand icon resource before starting main menu
	// TODO: load startup configurations (address 0041A970)
	// TODO: setup cinematics (address 0041A880) (probably we won't need it)
	// TODO: init unknown game stuffs (address 0040F430)

	// TODO: if savegame exists on folder, than start NewGame()

    return Common::kNoError;
}

Common::Error AsylumEngine::go() {
	// Play intro movie

	_resMgr->loadVideo(0);

	showMainMenu();

	// DEBUG
    // Control loop test. Basically just keep the
    // ScummVM window alive until ESC is pressed.
    // This will facilitate drawing tests ;)

	Common::EventManager *em = _system->getEventManager();
	while (!shouldQuit()) {
		Common::Event ev;
		if (em->pollEvent(ev)) {
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					// Push a quit event
					Common::Event event;
					event.type = Common::EVENT_QUIT;
					g_system->getEventManager()->pushEvent(event);
				}
				//if (ev.kbd.keycode == Common::KEYCODE_RETURN)
			}
		}

		_screen->updateScreen();
		_system->delayMillis(10);
	}

    return Common::kNoError;
}

void AsylumEngine::showMainMenu() {
	// main menu background
	_resMgr->loadGraphic(1, 0, 0);
	_resMgr->loadPalette(1, 17);
	_resMgr->loadCursor(1, 2, 0);

	/*

	// Music - start

	// Just play some music for now
	// FIXME: this should be moved to the bundle manager, but currently the whole manager needs
	// an overhaul...
	Common::File musFile;
	musFile.open("mus.005");
	uint32 entryCount = musFile.readUint32LE();
	uint32 offset1 = 0;
	uint32 offset2 = 0;
	for (uint32 i = 0; i < entryCount; i++) {
		if (offset1 == 0)
			offset1 = musFile.readUint32LE();
		// HACK: This will ultimately read the last entry in the bundle lookup table
		// This will only work for file bunfles with 1 music file included (like mus.005)
		offset2 = musFile.readUint32LE();
	}

	byte *buffer = new byte[offset2 - offset1];
	musFile.read(buffer, offset2 - offset1);
	musFile.close();

	Common::MemoryReadStream *mem = new Common::MemoryReadStream(buffer, offset2 - offset1);

	// Now create the audio stream and play it (it's just a regular WAVE file)
	Audio::AudioStream *mus = Audio::makeWAVStream(mem, true);
	Audio::SoundHandle _musicHandle;
	_mixer->playInputStream(Audio::Mixer::kMusicSoundType, &_musicHandle, mus);

	// Music - end
	 */
}

} // namespace Asylum
