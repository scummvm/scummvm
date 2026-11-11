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
#include "tesoro/core/level.h"
#include "tesoro/core/spriteobject.h"

namespace Tesoro {

TesoroEngine *g_engine;

TesoroEngine::TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			   _gameDescription(gameDesc), _clock(syst->getMillis()), _randomSource("Tesoro") {
	g_engine = this;
}

TesoroEngine::~TesoroEngine() {
	delete _level;
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

	Graphics::FrameLimiter limiter(g_system, 60);

	const uint32 fontOffset = _archive.findEntry("FUENTES.ANM");
	if (!fontOffset || !_font.load(*_archive.seekEntry(fontOffset), fontOffset, AnimBank::kLoadNow)) {
		warning("Tesoro: the font FUENTES.ANM cannot be read");
	}
	_display.setFont(_font.bank());

	while (runFrame()) {
		limiter.delayBeforeSwap();
		_screen->update();
		limiter.startFrame();
	}

	return Common::kNoError;
}


void TesoroEngine::setButton(PointerState::Button button, bool down) {
	if (_buttonsDown[button] == down)
		return;
	_buttonsDown[button] = down;
	const uint32 now = _system->getMillis();
	if (down)
		_pointer.buttons[button] |= now - _pointer.releasedAt[button] < PointerState::kDoublePressDelay ? PointerState::kDoublePress : PointerState::kPressed;
	else {
		_pointer.buttons[button] |= PointerState::kReleased;
		_pointer.releasedAt[button] = now;
	}
}

void TesoroEngine::handleEvent(const Common::Event &e) {
	switch (e.type) {
	case Common::EVENT_MOUSEMOVE:
		// The original's mouse counts from the middle of the window.
		_pointer.x = e.mouse.x - 320;
		_pointer.y = e.mouse.y - 240;
		break;
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
		setButton(PointerState::kLeft, e.type == Common::EVENT_LBUTTONDOWN);
		break;
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		setButton(PointerState::kRight, e.type == Common::EVENT_RBUTTONDOWN);
		break;
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		_buttonsDown[2] = e.type == Common::EVENT_MBUTTONDOWN;
		break;
	case Common::EVENT_KEYDOWN:
		if (e.kbd.keycode == Common::KEYCODE_ESCAPE && !e.kbdRepeat)
			_escape |= PointerState::kPressed;
		break;
	case Common::EVENT_KEYUP:
		if (e.kbd.keycode == Common::KEYCODE_ESCAPE)
			_escape |= PointerState::kReleased;
		break;
	default:
		break;
	}
}

bool TesoroEngine::runFrame() {
	_pointer.buttons[0] = _pointer.buttons[1] = 0;
	_escape = 0;
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e))
		handleEvent(e);

	if (_pointer.buttons[0] || _pointer.buttons[1] || _escape)
		debug("Pointer (%d, %d) left %02x right %02x escape %02x", _pointer.x, _pointer.y, _pointer.buttons[0], _pointer.buttons[1], _escape);

	const int32 now = gameTime();
	_display.startFrame();
	_display.fillBackground(0x4208);	// dark grey
	if (_level)
		_level->drawRooms(now, _display);
	_display.queueText((const byte *)"Hello, world!", 0xFFFF, Display::kWidth / 2, 50, Display::kWidth, 0, true, 16);

	_display.compose();
	_screen->copyRectToSurface(_display.frame(), Display::kWidth * 2, 0, 0, Display::kWidth, Display::kHeight);
	_screen->addDirtyRect(Common::Rect(Display::kWidth, Display::kHeight));
	return !shouldQuit();
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

Camera &TesoroEngine::camera() {
	return _level ? _level->camera() : _camera;
}

bool TesoroEngine::level(const char *name) {
	Common::String upper(name);
	upper.toUppercase(); // archive names are upper case
	const uint32 offset = _archive.findEntry(upper.c_str());
	if (!offset) {
		warning("Tesoro: level %s is not in the archive", upper.c_str());
		return false;
	}

	Level *level = new Level();
	if (!level->load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow, gameTime())) {
		warning("Tesoro: level %s cannot be read", upper.c_str());
		delete level;
		return false;
	}
	level->attachObjects();

	// The level replaces whatever was shown before.
	delete _level;
	_level = level;
	return true;
}

} // End of namespace Tesoro
