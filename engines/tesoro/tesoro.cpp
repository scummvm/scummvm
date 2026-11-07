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

#include "tesoro/tesoro.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/framelimiter.h"
#include "graphics/paletteman.h"
#include "tesoro/console.h"
#include "tesoro/detection.h"

#include "graphics/managed_surface.h"

namespace Tesoro {

TesoroEngine *g_engine;

TesoroEngine::TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			   _gameDescription(gameDesc), _clock(syst->getMillis()), _randomSource("Tesoro") {
	g_engine = this;
}

TesoroEngine::~TesoroEngine() {
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

	playAnim("JIMDERECHA.ANM", _animPlayer);
	playAnim("JIMABAJO.ANM", _animPlayer2);

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(e)) {
		}

		if (_animPlayer.bank() && _animPlayer2.bank()) {
			_animPlayer.advance(gameTime());
			_animPlayer2.advance(gameTime());
			const AnimBank &bank = *_animPlayer.bank();
			const AnimStep &step = bank.step(_animPlayer.current());
			const AnimBank &bank2 = *_animPlayer2.bank();
			const AnimStep &step2 = bank2.step(_animPlayer2.current());

			_display.startFrame();
			_display.fillBackground(0x4208);	// dark grey, the same colour as before

			if (step.hasImage()) {
				debug("Displaying image %d at (%d, %d)", step.imageIndex, 300 + step.imageX, 300 - step.imageY);
				_display.queueImage(300 + step.imageX, 300 - step.imageY, 3, bank.image(step.imageIndex), 0);
			}
			if (step2.hasImage()) {
				debug("Displaying image %d at (%d, %d)", step2.imageIndex, 300 + step2.imageX, 300 - step2.imageY);
				_display.queueImage(300 + step2.imageX, 300 - step2.imageY, 5, bank2.image(step2.imageIndex), 0);
			}

			_display.compose();
			_screen->copyRectToSurface(_display.frame(), Display::kWidth * 2, 0, 0, Display::kWidth, Display::kHeight);
			_screen->addDirtyRect(Common::Rect(Display::kWidth, Display::kHeight));
		}

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
	if (!player.load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow, false)) {
		debug("Could not load %s", upper.c_str());
		return false;
	}
	// All steps up to the first one without an image, looping, with the cycle length totalled.
	player.setRange(0, -1, true, true, gameTime());
	return true;
}

} // End of namespace Tesoro
