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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "graphics/screen.h"

#include "qdengine/qdengine.h"
#include "qdengine/console.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/qdcore/qd_game_dispatcher.h"
#include "qdengine/qdcore/util/plaympp_api.h"
#include "qdengine/system/sound/snd_dispatcher.h"

namespace QDEngine {

QDEngineEngine *g_engine;

QDEngineEngine::QDEngineEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("QDEngine") {
	g_engine = this;
	_pixelformat = Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0);

	_screenW = 640;
	_screenH = 480;

	ConfMan.registerDefault("game_speed", 1);
	ConfMan.registerDefault("enable_sound", true);
	ConfMan.registerDefault("enable_music", true);
	ConfMan.registerDefault("logic_period", 25);
	ConfMan.registerDefault("logic_synchro_by_clock", true);
	ConfMan.registerDefault("music_volume", 255);
	ConfMan.registerDefault("show_fps", false);
	ConfMan.registerDefault("sound_volume", 255);
	ConfMan.registerDefault("splash_enabled", true);
	ConfMan.registerDefault("splash_time", 3000);

	memset(_tagMap, 0, sizeof(_tagMap));
}

QDEngineEngine::~QDEngineEngine() {
	delete _screen;
	cleanup_XML_Parser();
}

uint32 QDEngineEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::Language QDEngineEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::String QDEngineEngine::getGameId() const {
	return _gameDescription->gameId;
}

const char *QDEngineEngine::getExeName() const {
	return _gameDescription->filesDescriptions[1].fileName;
}

Common::Error QDEngineEngine::run() {
	initGraphics(640, 480);
	_screen = new Graphics::Screen();

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	byte pal[256 * 3] = { 0 };
	Common::Event e;
	int offset = 0;

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		// Cycle through a simple palette
		++offset;
		for (int i = 0; i < 256; ++i)
			pal[i * 3 + 1] = (i + offset) % 256;
		g_system->getPaletteManager()->setPalette(pal, 0, 256);
		_screen->update();

		engineMain();

		// Delay for a bit. All events loops should have a delay
		// to prevent the system being unduly loaded
		g_system->delayMillis(10);
	}

	return Common::kNoError;
}

bool canSaveGameStateCurrently(Common::U32String *msg)  {
	return qdGameDispatcher::get_dispatcher()->get_active_scene() != nullptr;
}

Common::Error QDEngineEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (qdGameDispatcher::get_dispatcher()->save_save(stream))
		return Common::kNoError;

	return Common::kWritingFailed;
}

Common::Error QDEngineEngine::loadGameStream(Common::SeekableReadStream *stream) {
	if (qdGameDispatcher::get_dispatcher()->load_save(stream))
		return Common::kNoError;

	return Common::kReadingFailed;
}

void QDEngineEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	sndDispatcher::get_dispatcher()->syncSoundSettings();
	mpegPlayer::instance().syncMusicSettings();
}

} // namespace QDEngine

// Translates cp-1251..utf-8
byte *transCyrillic(const Common::String &str) {
	const byte *s = (const byte *)str.c_str();
	static byte tmp[1024];

#ifndef WIN32
	static int trans[] = { 0xa8, 0xd081, 0xb8, 0xd191, 0xc0, 0xd090,
		0xc1, 0xd091, 0xc2, 0xd092, 0xc3, 0xd093, 0xc4, 0xd094,
		0xc5, 0xd095, 0xc6, 0xd096, 0xc7, 0xd097, 0xc8, 0xd098,
		0xc9, 0xd099, 0xca, 0xd09a, 0xcb, 0xd09b, 0xcc, 0xd09c,
		0xcd, 0xd09d, 0xce, 0xd09e, 0xcf, 0xd09f, 0xd0, 0xd0a0,
		0xd1, 0xd0a1, 0xd2, 0xd0a2, 0xd3, 0xd0a3, 0xd4, 0xd0a4,
		0xd5, 0xd0a5, 0xd6, 0xd0a6, 0xd7, 0xd0a7, 0xd8, 0xd0a8,
		0xd9, 0xd0a9, 0xda, 0xd0aa, 0xdb, 0xd0ab, 0xdc, 0xd0ac,
		0xdd, 0xd0ad, 0xde, 0xd0ae, 0xdf, 0xd0af, 0xe0, 0xd0b0,
		0xe1, 0xd0b1, 0xe2, 0xd0b2, 0xe3, 0xd0b3, 0xe4, 0xd0b4,
		0xe5, 0xd0b5, 0xe6, 0xd0b6, 0xe7, 0xd0b7, 0xe8, 0xd0b8,
		0xe9, 0xd0b9, 0xea, 0xd0ba, 0xeb, 0xd0bb, 0xec, 0xd0bc,
		0xed, 0xd0bd, 0xee, 0xd0be, 0xef, 0xd0bf, 0xf0, 0xd180,
		0xf1, 0xd181, 0xf2, 0xd182, 0xf3, 0xd183, 0xf4, 0xd184,
		0xf5, 0xd185, 0xf6, 0xd186, 0xf7, 0xd187, 0xf8, 0xd188,
		0xf9, 0xd189, 0xfa, 0xd18a, 0xfb, 0xd18b, 0xfc, 0xd18c,
		0xfd, 0xd18d, 0xfe, 0xd18e, 0xff, 0xd18f, 0x00 };
#endif

	int i = 0;

	for (const byte *p = s; *p; p++) {
#ifdef WIN32
		// translate from cp1251 to cp866
		byte c = *p;
		if (c >= 0xC0 && c <= 0xEF)
			c = c - 0xC0 + 0x80;
		else if (c >= 0xF0)
			c = c - 0xF0 + 0xE0;
		else if (c == 0xA8)
			c = 0xF0;
		else if (c == 0xB8)
			c = 0xF1;
		tmp[i++] = c;
#else
		if (*p < 128) {
			tmp[i++] = *p;
		} else {
			int j;
			for (j = 0; trans[j]; j += 2) {
				if (trans[j] == *p) {
					tmp[i++] = (trans[j + 1] >> 8) & 0xff;
					tmp[i++] = trans[j + 1] & 0xff;
					break;
				}
			}

			if (!trans[j]) {
				warning("transCyrillic: no mapping for %d (0x%x)", *p, *p);
				tmp[i++] = '?';
				tmp[i++] = '?';
			}
		}
#endif
	}

	tmp[i] = 0;

	return tmp;
}
