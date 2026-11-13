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
#include "tesoro/core/cursor.h"
#include "tesoro/core/jim.h"
#include "tesoro/core/level.h"
#include "tesoro/core/menuitem.h"
#include "tesoro/core/namekey.h"
#include "tesoro/core/spriteobject.h"

namespace Tesoro {

TesoroEngine *g_engine;

/** The levels the engine shows: their second step draws and composes the frame. */
class MenuLevel : public Level {
public:
	explicit MenuLevel(TesoroEngine &engine) : _engine(engine) {}

	int update2() override { return _engine.levelSecondStep(*this); }

private:
	TesoroEngine &_engine;
};

TesoroEngine::TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
																			   _gameDescription(gameDesc), _clock(syst->getMillis()), _randomSource("Tesoro"), _world(*this) {
	g_engine = this;
}

TesoroEngine::~TesoroEngine() {
	_world.mainList().clear();
	delete _interface;
	delete _mainMenu;
	delete _newGameMenu;
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

	_mainMenu = loadLevel("PRINCIPAL.LVL");
	_newGameMenu = loadLevel("NUEVO.LVL");
	if (!_mainMenu || !_newGameMenu)
		return Common::kReadingFailed;
	hookMenuItems(_mainMenu);
	hookMenuItems(_newGameMenu);
	_level = _mainMenu;
	_level->camera().moveTo(0, 0, 0, gameTime());

	// The interface level holds the pointer (its other kinds are not ported yet and are skipped with warnings).
	_interface = loadLevel("INTERFACE.LVL");
	if (_interface) {
		ObjectList &objects = _interface->objects();
		for (uint i = 0; i < objects.size(); ++i) {
			if (objects[i]->kindId() == 10)
				_cursor = static_cast<Cursor *>(objects[i]);
		}
	}
	if (_cursor) {
		_cursor->setSurroundings(&_pointer, _level);
		_cursor->setHost(&_world);
	}

	// The pointer, the interface level and the level the player is in take their turns in this order.
	if (_cursor)
		_world.mainList().insertByPriority(_cursor, 0xf);
	if (_interface)
		_world.mainList().insertByPriority(_interface, 0x13);
	_world.mainList().insertByPriority(_level, 0x14);

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

	// The clock is frozen for the rest of the frame, so every object sees the same time.
	_clock.freeze(_system->getMillis());

	_display.startFrame();
	const int32 now = gameTime();
	if (_interface)
		_interface->setFrameTime(now);
	if (_level)
		_level->setFrameTime(now);

	// The first pass moves things; the second, unless the first asked to stop, draws the picture.
	if (_world.mainList().runUpdate() != 3)
		_world.mainList().runUpdate2();

	// Without a level nothing composes the frame, so the pointer is shown on its own.
	if (!_level)
		presentFrame();

	if (_showOutlines) {
		if (_interface)
			drawOutlines(*_interface);
		if (_level)
			drawOutlines(*_level);
		_screen->addDirtyRect(Common::Rect(Display::kWidth, Display::kHeight));
	}
	_clock.unfreeze();
	return !shouldQuit();
}

void TesoroEngine::hookMenuItems(Level *level) {
	ObjectList &objects = level->objects();
	for (uint i = 0; i < objects.size(); ++i) {
		if (objects[i]->kindId() == 25)
			static_cast<MenuItem *>(objects[i])->setHost(this);
	}
}

void TesoroEngine::fadeOut(int steps, int delayMs) {
	_display.fadeOut(steps, delayMs, _screen);
}

void TesoroEngine::enterMainMenu() {
	showMenu(_mainMenu);
}

void TesoroEngine::enterNewGameMenu() {
	showMenu(_newGameMenu);
}

void TesoroEngine::showMenu(Level *menu) {
	_world.mainList().remove(_level);
	_level = menu;
	_world.mainList().insertByPriority(_level, 0x14);
	_level->camera().moveTo(0, 0, 0, gameTime());
	if (_cursor)
		_cursor->setSurroundings(&_pointer, _level);
}

void TesoroEngine::startScript(const char *block, const char *label) {
	notPorted("starting a script");
}

void TesoroEngine::startGame() {
	notPorted("starting the game");
}

void TesoroEngine::openSavedGames() {
	notPorted("the saved games screen");
}

void TesoroEngine::clearHoverText() {
	setHoverText(nullptr);
}

void TesoroEngine::notPorted(const char *what) {
	warning("Tesoro: %s is not ported yet", what);
}

uint16 TesoroEngine::screenColour(byte r, byte g, byte b) const {
	return (uint16)_screen->format.RGBToColor(r, g, b);
}

static void screenLine(Graphics::Screen &screen, int x0, int y0, int x1, int y1, uint16 colour) {
	// Bresenham, clipped to the screen.
	const int dx = ABS(x1 - x0), dy = -ABS(y1 - y0);
	const int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;
	for (;;) {
		if (x0 >= 0 && x0 < screen.w && y0 >= 0 && y0 < screen.h)
			*(uint16 *)screen.getBasePtr(x0, y0) = colour;
		if (x0 == x1 && y0 == y1)
			break;
		const int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;
		}
	}
}

void TesoroEngine::drawOutlines(Level &level) {
	const Camera &camera = level.camera();
	const uint16 hitter = screenColour(0xff, 0x40, 0x40), collidable = screenColour(0xff, 0xe0, 0x40);
	const uint16 plain = screenColour(0x90, 0x90, 0x90), outline = screenColour(0x40, 0xff, 0xff);

	ObjectList &objects = level.objects();
	for (uint i = 0; i < objects.size(); ++i) {
		const Object *object = objects[i];
		if (!(object->flags() & Object::kActive))
			continue;

		// World coordinates point up and are relative to the object; the screen is centred on the camera.
		const int32 ox = object->x() - camera.scaledX() + 0x140;
		const int32 oy = camera.scaledY() - object->y() + 0xf0;

		const ObjectRect &r = object->hitRect();
		const uint16 colour = (object->flags() & Object::kHitter) ? hitter : (object->flags() & Object::kCollidable) ? collidable : plain;
		if (r.x1 != r.x2 || r.y1 != r.y2) {
			screenLine(*_screen, ox + r.x1, oy - r.y1, ox + r.x2, oy - r.y1, colour);
			screenLine(*_screen, ox + r.x2, oy - r.y1, ox + r.x2, oy - r.y2, colour);
			screenLine(*_screen, ox + r.x2, oy - r.y2, ox + r.x1, oy - r.y2, colour);
			screenLine(*_screen, ox + r.x1, oy - r.y2, ox + r.x1, oy - r.y1, colour);
		}

		// The outline of the current step: points, with the last joined back to the first.
		const AnimPlayer *player = const_cast<Object *>(object)->currentPlayer();
		if (!player || !player->bank())
			continue;
		const AnimStep &step = player->bank()->step(player->current());
		if (step.type != 4 || step.entries.size() < 2)
			continue;
		for (uint p = 0; p < step.entries.size(); ++p) {
			const uint32 a = step.entries[p], b = step.entries[(p + 1) % step.entries.size()];
			screenLine(*_screen, ox + (int16)(a & 0xffff), oy - (int16)(a >> 16), ox + (int16)(b & 0xffff), oy - (int16)(b >> 16), outline);
		}
	}
}

void TesoroEngine::presentFrame() {
	_display.compose();
	_screen->copyRectToSurface(_display.frame(), Display::kWidth * 2, 0, 0, Display::kWidth, Display::kHeight);
	_screen->addDirtyRect(Common::Rect(Display::kWidth, Display::kHeight));
}

int TesoroEngine::levelSecondStep(Level &level) {
	level.drawRooms(gameTime(), _display);
	// The interface level only draws: the level the player is in composes the frame.
	if (&level != _interface)
		presentFrame();
	return 1;
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

Level *TesoroEngine::loadLevel(const char *name) {
	Common::String upper(name);
	upper.toUppercase(); // archive names are upper case
	const uint32 offset = _archive.findEntry(upper.c_str());
	if (!offset) {
		warning("Tesoro: level %s is not in the archive", upper.c_str());
		return nullptr;
	}

	Level *level = new MenuLevel(*this);
	if (!level->load(*_archive.seekEntry(offset), offset, AnimBank::kLoadNow, gameTime())) {
		warning("Tesoro: level %s cannot be read", upper.c_str());
		delete level;
		return nullptr;
	}
	level->attachObjects();
	return level;
}

bool TesoroEngine::level(int index) {
	Level *level = _world.level(index);
	if (!level)
		return false;

	// The world keeps every level it has loaded, so the one being replaced is not deleted.
	_world.mainList().remove(_level);
	if (_world.jimLevel() == _level)
		_world.jim()->leaveLevel();
	_level = level;
	_world.mainList().insertByPriority(_level, 0x14);
	if (_cursor)
		_cursor->setSurroundings(&_pointer, _level);
	return true;
}

void TesoroEngine::setHoverText(const char *text) {
	if (_cursor)
		_cursor->label().set(text, text ? 0xffff : 0);	// names are shown in white
}

bool TesoroEngine::putJim(int32 x, int32 y) {
	Jim *jim = _world.jim();
	if (!jim || !_level)
		return false;
	// enterLevel puts the level in the main list itself.
	_world.mainList().remove(_level);
	jim->enterLevel(_level, &_level->room(0), x, y);
	_cursor->setSurroundings(&_pointer, _level);
	return true;
}

bool TesoroEngine::makeHitter(const char *name) {
	Object *object = _level ? _level->findObject(nameKey64(name)) : nullptr;
	if (!object)
		return false;
	object->setFlags(object->flags() | Object::kCollidable | Object::kHitter);
	return true;
}

} // End of namespace Tesoro
