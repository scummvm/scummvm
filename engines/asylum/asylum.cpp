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
 */

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "common/file.h"

#include "common/stream.h"
#include "sound/audiostream.h"
#include "sound/wave.h"

#include "asylum/asylum.h"
#include "asylum/screen.h"
#include "asylum/video.h"

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
	delete _video;
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
	_resMgr = new ResourceManager;
	_video = new Video(this);

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
	_video->playVideo(0);

	showMainMenu();

	// DEBUG
    // Control loop test. Basically just keep the
    // ScummVM window alive until ESC is pressed.
    // This will facilitate drawing tests ;)

    uint32 lastRefresh = 0;

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


	// eyes animation index table
	//const uint32 eyesTable[8] = {3, 5, 1, 7, 4, 8, 2, 6};
	byte pal[256 * 3];
	_resMgr->getPalette(1, 17, pal);
	GraphicResource *bg  = _resMgr->getGraphic(1, 0)->getEntry(0);
	GraphicResource *cur = _resMgr->getGraphic(1, 2)->getEntry(0);

	_system->setMouseCursor(cur->data, cur->width, cur->height, 1, 1, 0);
	// FIXME: is there any reason why the cursor palette is set here, 
	// when it's the same as the rest of the game's palette?
	//_system->setCursorPalette(pal, 0, 1024);
	_system->showMouse(true);

	_screen->setFrontBuffer(
			0, 0,
			bg->width, bg->height,
			bg->data);
	_screen->setPalette(pal);
	_screen->updateScreen();
}

} // namespace Asylum
