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

#ifndef TESORO_H
#define TESORO_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "tesoro/detection.h"
#include "tesoro/core/archive.h"
#include "tesoro/core/animbank.h"
#include "tesoro/core/camera.h"
#include "tesoro/core/clock.h"
#include "tesoro/core/pointerstate.h"
#include "tesoro/core/display.h"
#include "tesoro/core/object.h"
#include "tesoro/world.h"

namespace Tesoro {

struct TesoroGameDescription;

class Level;
class Cursor;
class MenuItem;
class Console;

class TesoroEngine : public Engine {
private:
	friend class World;
	friend class MenuItem;
	friend class Console;

	const ADGameDescription *_gameDescription;
	GameClock _clock;
	Common::RandomSource _randomSource;
	Archive _archive;
	Camera _camera;
	Common::Array<AnimPlayer> _testPlayers;	// the "level" of the test object
	Level *_level = nullptr;		// the level shown
	Level *_mainMenu = nullptr;	// PRINCIPAL.LVL and NUEVO.LVL stay loaded while the other is shown
	Level *_newGameMenu = nullptr;
	Level *_interface = nullptr;	// INTERFACE.LVL, which holds the pointer
	Cursor *_cursor = nullptr;
	Display _display;
	PointerState _pointer;
	bool _buttonsDown[3] = { false, false, false };
	AnimPlayer _font;
	World _world;
	byte _escape = 0;
	bool _showOutlines = false;

	// What clicks on the menu items ask for.
	void fadeOut(int steps, int delayMs);
	void enterMainMenu();
	void enterNewGameMenu();
	void startScript(const char *block, const char *label);
	void startGame();
	void openSavedGames();
	int32 now() { return gameTime(); }
	void clearHoverText();
	void notPorted(const char *what);

	/** Makes a menu level the one shown, with its camera on the origin. */
	void showMenu(Level *menu);

	/** Hands the menu items of a level this engine as their host. */
	void hookMenuItems(Level *level);

	void handleEvent(const Common::Event &e);
	void setButton(PointerState::Button button, bool down);

	/** One frame: events, then drawing. False when the game should end. */
	bool runFrame();

	void resetKeys() { _escape = 0; }

protected:
	// Engine APIs
	Common::Error run() override;
public:
	Graphics::Screen *_screen = nullptr;
public:
	TesoroEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~TesoroEngine() override;

	int32 gameTime() { return _clock.update(_system->getMillis()); }

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	/** The camera of the level being shown, else the test object's. */
	Camera &camera();
	/** Loads a level from the archive and shows it. */
	bool level(int index);
	/** Second step of a level: draws it and, unless it is the interface level, composes the frame onto the screen. */
	int levelSecondStep(Level &level);
	void presentFrame();
	/** Draws the hit rectangle and outline of every object of a level over the finished frame. */
	void drawOutlines(Level &level);
	uint16 screenColour(byte r, byte g, byte b) const;
	/** Loads a level file from the archive; null if it is missing or cannot be read. */
	Level *loadLevel(const char *name);
	/** Sets the label next to the pointer (null clears it). */
	void setHoverText(const char *text);
	/** Flags an object of the level as collidable and a hitter, so the pointer can touch it. False if there is no such object. */
	bool makeHitter(const char *name);
	/** Puts Jim into the shown level at a place. False if he cannot be built. */
	bool putJim(int32 x, int32 y);
	Cursor *cursor() { return _cursor; }
	Level *shownLevel() { return _level; }
	World &world() { return _world; }
	/** Shows or hides the hit rectangles and outlines of the objects over the picture. Returns the new state. */
	bool toggleOutlines() { return _showOutlines = !_showOutlines; }
};

extern TesoroEngine *g_engine;
#define SHOULD_QUIT ::Tesoro::g_engine->shouldQuit()

} // End of namespace Tesoro

#endif // TESORO_H
