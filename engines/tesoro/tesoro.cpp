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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "graphics/managed_surface.h"

#include "tesoro/console.h"
#include "tesoro/tesoro.h"
#include "tesoro/detection.h"
#include "tesoro/core/spriteobject.h"

namespace Tesoro {

TesoroEngine *g_engine;

TesoroEngine::TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			   _gameDescription(gameDesc), _clock(syst->getMillis()), _randomSource("Tesoro") {
	g_engine = this;
}

TesoroEngine::~TesoroEngine() {
	delete _testObject;
	delete _screen;
}

uint32 TesoroEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String TesoroEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error TesoroEngine::run() {
	// Initialize 320x200 paletted graphics mode
	const Graphics::PixelFormat format(2, 5, 6, 5, 0, 11, 5, 0, 0);
	initGraphics(640, 480, &format);
	_screen = new Graphics::Screen(640, 480, format);

	if (!_archive.open("TESORO.DAT", "TESORO.TRE")) {
		GUIErrorMessage("Could not open TESORO.DAT and TESORO.TRE");
		return Common::kReadingFailed;
	}

	// Set the engine's debugger console
	setDebugger(new Console());

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	// Simple event handling loop
	Common::Event e;

	Graphics::FrameLimiter limiter(g_system, 60);

	const uint32 fontOffset = _archive.findEntry("FUENTES.ANM");
	if (!fontOffset || !_font.load(*_archive.seekEntry(fontOffset), fontOffset, AnimBank::kLoadNow)) {
		warning("Tesoro: the font FUENTES.ANM cannot be read");
	}
	_display.setFont(_font.bank());


	int32 kScrollSpeed = 4;	// camera units per frame while an arrow key is held
	int32 kCameraDivisor = 300;	// scaledX = x * 300 / divisor, so 300 draws at world scale

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type != Common::EVENT_KEYDOWN && e.type != Common::EVENT_KEYUP)
				continue;
			const int down = e.type == Common::EVENT_KEYDOWN ? 1 : 0;
			switch (e.kbd.keycode) {
			case Common::KEYCODE_LEFT: _scrollX = -down; break;
			case Common::KEYCODE_RIGHT: _scrollX = down; break;
			case Common::KEYCODE_DOWN: _scrollY = -down; break;
			case Common::KEYCODE_UP: _scrollY = down; break;
			default: break;
			}
		}

		const int32 now = gameTime();
		if (_scrollX || _scrollY)
			_camera.moveTo(_camera.x() + _scrollX * kScrollSpeed, _camera.y() + _scrollY * kScrollSpeed, 0, now);

		_display.startFrame();
		_display.fillBackground(0x4208);	// dark grey

		if (_testObject) {
			_camera.updateToward(kCameraDivisor, now);
			_testObject->updateBounds(now);
			_testObject->draw(_camera, _display);
		}
		_display.queueText((const byte *)"Hello, world!", 0xFFFF, Display::kWidth / 2, 50, Display::kWidth, 0, true, 16);

		_display.compose();
		_screen->copyRectToSurface(_display.frame(), Display::kWidth * 2, 0, 0, Display::kWidth, Display::kHeight);
		_screen->addDirtyRect(Common::Rect(Display::kWidth, Display::kHeight));

		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}

Common::Error TesoroEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

bool TesoroEngine::playAnim(const char *name, AnimPlayer &player) {
	Common::String upper(name);
	upper.toUppercase(); // archive names are upper case
	const uint32 offset = _archive.findEntry(upper.c_str());
	if (!offset) {
		debug("No entry called %s", upper.c_str());
		return false;
	}
	if (!player.load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow)) {
		debug("Could not load %s", upper.c_str());
		return false;
	}
	// All steps up to the first one without an image, looping, with the cycle length totalled.
	player.setRange(0, -1, true, true, gameTime());
	return true;
}

bool TesoroEngine::showObject(const char *name) {
	delete _testObject;
	_testObject = nullptr;

	_testPlayers.clear();
	_testPlayers.resize(1);
	if (!playAnim(name, _testPlayers[0]))
		return false;

	ObjectRecord record;
	Common::strlcpy(record.name, "TEST", sizeof(record.name));
	record.banks.push_back(0);

	const int32 now = gameTime();
	SpriteObject *object = new SpriteObject();

	object->init(record, 0, _testPlayers, 0);
	object->animation().selectBank(0, 0, -1, true, true, now);
	_testObject = object;

	// The centre may move 320 across and 240 up or down from the object at the origin.
	_camera.init();
	_camera.setBounds(-640, 480, 640, -480);
	_camera.moveTo(0, 0, 0, now);
	return true;
}

} // End of namespace Tesoro
