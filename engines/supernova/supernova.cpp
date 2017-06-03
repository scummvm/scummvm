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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/str.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/screen.h"
#include "graphics/palette.h"
//#include "graphics/font.h"
//#include "graphics/fontman.h"

#include "supernova/supernova.h"


namespace Supernova {

SupernovaEngine::SupernovaEngine(OSystem *syst)
	: Engine(syst)
	, _console(NULL)
{
//	const Common::FSNode gameDataDir(ConfMan.get("path"));
//	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	// setup engine specific debug channels
	DebugMan.addDebugChannel(kDebugGeneral, "general", "Supernova general debug channel");

	_rnd = new Common::RandomSource("supernova");
}

SupernovaEngine::~SupernovaEngine() {
	DebugMan.clearAllDebugChannels();
	
	delete _rnd;
	delete _console;
}

Common::Error SupernovaEngine::run() {
	initGraphics(kScreenWidth, kScreenHeight);
	_console = new Console(this);

	initData();

	bool running = true;
	while (running) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				running = false;
				break;
			
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_d && event.kbd.hasFlags(Common::KBD_CTRL)) {
					_console->attach();
				}
				if (event.kbd.keycode == Common::KEYCODE_q) {
					playSound(48, 13530);
				}
				
				break;
			default:
				break;
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
	}
	
	//deinit timer/sound/..
	stopSound();

	return Common::kNoError;
}

void SupernovaEngine::initData() {
	Common::File f;
	if (!f.open("msn_data.047")) {
		error("File s could not be read!");
	}
	
	debug("%s\t%u", f.getName(), f.size());
}

void SupernovaEngine::playSound(int filenumber, int offset) {
	Common::File *file = new Common::File();
	Common::String filename(Common::String::format("msn_data.0%2d", filenumber));
	if (!file->open(filename)) {
		error("File %s could not be read!", filename.c_str());
	}
	
	file->seek(offset);
	Audio::SeekableAudioStream *audioStream = Audio::makeRawStream(file, 11931, Audio::FLAG_UNSIGNED | Audio::FLAG_LITTLE_ENDIAN);
	stopSound();
	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_soundHandle, audioStream);
}

void SupernovaEngine::stopSound() {
	if (_mixer->isSoundHandleActive(_soundHandle))
		_mixer->stopHandle(_soundHandle);
}

void SupernovaEngine::renderImage(int file, int section) {
	// STUB
}

}
