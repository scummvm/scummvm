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
#include "common/file.h"
#include "common/fs.h"
#include "common/system.h"
#include "graphics/pixelformat.h"

#include "engines/util.h"

#include "griffon/griffon.h"
#include "griffon/console.h"

namespace Griffon {

GriffonEngine::GriffonEngine(OSystem *syst) : Engine(syst) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "sound");

	_rnd = new Common::RandomSource("griffon");

	_console = nullptr;
	_mixer = nullptr;

	_shouldQuit = false;
	_gameEnd = false;

	_musicChannel = -1;
	_menuChannel = -1;

	_cloudAngle = 0.0f;

	for (int i = 0; i < 33; ++i) {
		for (int j = 0; j < 6; ++j) {
			_objectInfo[i].nFrames = 0;
			_objectInfo[i].xTiles = 0;
			_objectInfo[i].yTiles = 0;
			_objectInfo[i].speed = 0;
			_objectInfo[i].type = 0;
			_objectInfo[i].script = 0;
		}
	}

	for (int i = 0; i < 256; ++i) {
		_objectFrame[i][0] = 0;
		_objectFrame[i][1] = 0;
	}

	// Synchronize the sound settings from ScummVM
	syncSoundSettings();
}

GriffonEngine::~GriffonEngine() {
	delete _rnd;
}

void GriffonEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	bool mute = false;
	config.music = config.effects = false;

	if (ConfMan.hasKey("mute"))
		mute = ConfMan.getBool("mute");

	if (!mute) {
		config.music = !ConfMan.getBool("music_mute");
		config.effects = !ConfMan.getBool("sfx_mute");
	}

	config.musicVol = ConfMan.getInt("music_volume");
	config.effectsVol = ConfMan.getInt("sfx_volume");
}

void GriffonEngine::saveConfig() {
	ConfMan.setBool("mute", !(config.music || config.effectsVol));
	ConfMan.setBool("music_mute", !config.music);
	ConfMan.setBool("sfx_mute", !config.effects);
	ConfMan.setInt("music_volume", config.musicVol);
	ConfMan.setInt("sfx_volume", config.effectsVol);

	ConfMan.flushToDisk();
}

Common::Error GriffonEngine::run() {
	initGraphics(320, 240, new Graphics::PixelFormat(4, 8, 8, 8, 8, 24, 16, 8, 0));

	_mixer = g_system->getMixer();

	_console = new Console();

	initialize();
	showLogos();

	if (_shouldQuit)
		return Common::kNoError;

	while (!_shouldQuit) {
		_gameEnd = false;

		title(0);

		if (!_shouldQuit && !_gameEnd)
			saveLoadNew();
	}

	return Common::kNoError;
}

}
