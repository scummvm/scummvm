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

#ifndef TWINE_TWINE_H
#define TWINE_TWINE_H

#include "backends/keymapper/keymap.h"
#include "common/random.h"
#include "common/rect.h"
#include "engines/engine.h"

#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "metaengine.h"
#include "twine/scene/actor.h"
#include "twine/input.h"
#include "twine/detection.h"

namespace TwinE {

/** Definition for European version */
#define EUROPE_VERSION 0
/** Definition for American version */
#define USA_VERSION 1
/** Definition for Modification version */
#define MODIFICATION_VERSION 2

/** Original screen width */
#define SCREEN_WIDTH 640
/** Original screen height */
#define SCREEN_HEIGHT 480
/** Default frames per second */
#define DEFAULT_FRAMES_PER_SECOND 20

/** Number of colors used in the game */
#define NUMOFCOLORS 256

static const struct TwinELanguage {
	const char *name;
	const char *id;
} LanguageTypes[] = {
    {"English", "EN_"},
    {"French", "FR_"},
    {"German", "DE_"},
    {"Spanish", "SP_"},
    {"Italian", "IT_"},
    {"Portuguese", ""}};

enum MidiFileType {
	MIDIFILE_NONE,
	MIDIFILE_DOS,
	MIDIFILE_WIN
};

/** Config movie types */
enum MovieType {
	CONF_MOVIE_NONE = 0,
	CONF_MOVIE_FLA = 1,
	CONF_MOVIE_FLAWIDE = 2,
	CONF_MOVIE_FLAGIF = 3
};

/** Configuration file structure
	Used in the engine to load/use certain parts of code according with
	this settings. Check \a lba.cfg file for valid values for each settings.\n
	All the settings with (*) means they are new and only exist in this engine. */
struct ConfigFile {
	/** Index into the LanguageTypes array. */
	int32 LanguageId = 0;
	bool Voice = true;
	/** Enable/Disable game dialogues */
	bool FlagDisplayText = false;
	/** Type of music file to be used */
	MidiFileType MidiType = MIDIFILE_NONE;
	/** *Game version */
	int32 Version = EUROPE_VERSION;
	/** If you want to use the LBA CD or not */
	int32 UseCD = 0;
	/** Allow various sound types */
	int32 Sound = 0;
	/** Allow various movie types */
	int32 Movie = CONF_MOVIE_FLA;
	/** Flag used to keep the game frames per second */
	int32 Fps = 0;
	/** Flag to display game debug */
	bool Debug = false;

	// these settings are not available in the original version
	/** Use cross fade effect while changing images, or be as the original */
	bool CrossFade = false;
	/** Flag to toggle Wall Collision */
	bool WallCollision = false;
	/** Use original autosaving system or save when you want */
	bool UseAutoSaving = false;
	bool Mouse = false;

	// these settings can be changed in-game - and must be persisted
	/** Shadow mode type, value: all, character only, none */
	int32 ShadowMode = 0;
	// TODO: currently unused
	int32 PolygonDetails = 2;
	/** Scenery Zoom */
	bool SceZoom = false;
};

class Actor;
class Animations;
class Collision;
class Extra;
class GameState;
class Grid;
class Movements;
class Interface;
class Menu;
class FlaMovies;
class MenuOptions;
class Music;
class Redraw;
class Renderer;
class Resources;
class Scene;
class Screens;
class ScriptLife;
class ScriptMove;
class Holomap;
class Sound;
class Text;
class DebugGrid;
struct Keyboard;
class Debug;
class DebugScene;

enum class EngineState {
	Menu,
	GameLoop,
	LoadedGame,
	QuitGame
};

struct ScopedEngineFreeze {
	TwinEEngine* _engine;
	ScopedEngineFreeze(TwinEEngine* engine);
	~ScopedEngineFreeze();
};

struct ScopedCursor {
	TwinEEngine* _engine;
	ScopedCursor(TwinEEngine* engine);
	~ScopedCursor();
};

class ScopedFPS {
private:
	uint32 _fps;
	uint32 _start;
public:
	ScopedFPS(uint32 fps = DEFAULT_FRAMES_PER_SECOND);
	~ScopedFPS();
};

class TwinEEngine : public Engine {
private:
	int32 isTimeFreezed = 0;
	int32 saveFreezedTime = 0;
	int32 _mouseCursorState = 0;
	ActorMoveStruct loopMovePtr; // mainLoopVar1
	PauseToken _pauseToken;
	TwineGameType _gameType;
	EngineState _state = EngineState::Menu;

	void processBookOfBu();
	void processBonusList();
	void processInventoryAction();
	void processOptionsMenu();
	/** recenter screen on followed actor automatically */
	void centerScreenOnActor();

public:
	TwinEEngine(OSystem *system, Common::Language language, uint32 flagsTwineGameType, TwineGameType gameType);
	~TwinEEngine() override;

	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

	bool canLoadGameStateCurrently() override { return true; }
	bool canSaveGameStateCurrently() override;

	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	void wipeSaveSlot(int slot);
	SaveStateList getSaveSlots() const;
	void autoSave();

	void pushMouseCursorVisible();
	void popMouseCursorVisible();

	bool isLBA1() const { return _gameType == TwineGameType::GType_LBA; }
	bool isLBA2() const { return _gameType == TwineGameType::GType_LBA2; }

	Actor *_actor;
	Animations *_animations;
	Collision *_collision;
	Extra *_extra;
	GameState *_gameState;
	Grid *_grid;
	Movements *_movements;
	Interface *_interface;
	Menu *_menu;
	FlaMovies *_flaMovies;
	MenuOptions *_menuOptions;
	Music *_music;
	Redraw *_redraw;
	Renderer *_renderer;
	Resources *_resources;
	Scene *_scene;
	Screens *_screens;
	ScriptLife *_scriptLife;
	ScriptMove *_scriptMove;
	Holomap *_holomap;
	Sound *_sound;
	Text *_text;
	DebugGrid *_debugGrid;
	Input *_input;
	Debug *_debug;
	DebugScene *_debugScene;

	/** Configuration file structure
	 * Contains all the data used in the engine to configurated the game in particulary ways. */
	ConfigFile cfgfile;

	/** Initialize LBA engine */
	void initEngine();
	void initMCGA();
	void initSVGA();

	void initConfigurations();
	/** Initialize all needed stuffs at first time running engine */
	void initAll();
	void processActorSamplePosition(int32 actorIdx);
	/**
	 * Game engine main loop
	 * @return true if we want to show credit sequence
	 */
	int32 runGameEngine();
	/** Allocate video memory, both front and back buffers */
	void allocVideoMemory();
	/**
	 * @return A random value between [0-max)
	 */
	int getRandomNumber(uint max = 0x7FFF);
	int32 quitGame = 0;
	int32 lbaTime = 0;

	Graphics::ManagedSurface imageBuffer;
	/** Work video buffer */
	Graphics::ManagedSurface workVideoBuffer;
	/** Main game video buffer */
	Graphics::ManagedSurface frontVideoBuffer;

	int32 loopInventoryItem = 0;
	int32 loopActorStep = 0;

	/** Disable screen recenter */
	bool disableScreenRecenter = false;

	int32 zoomScreen = 0;

	void freezeTime();
	void unfreezeTime();

	/**
	 * Game engine main loop
	 * @return true if we want to show credit sequence
	 */
	bool gameEngineLoop();

	Common::RandomSource _rnd;
	Common::Language _gameLang;
	uint32 _gameFlags;

	/**
	 * Deplay certain seconds till proceed - Can also Skip this delay
	 * @param time time in seconds to delay
	 */
	bool delaySkip(uint32 time);

	/**
	 * Set a new palette in the SDL screen buffer
	 * @param palette palette to set
	 */
	void setPalette(const uint32 *palette);

	/** Blit surface in the screen */
	void flip();

	/**
	 * Blit surface in the screen in a determinate area
	 * @param left left position to start copy
	 * @param top top position to start copy
	 * @param right right position to start copy
	 * @param bottom bottom position to start copy
	 */
	void copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom);
	void copyBlockPhys(const Common::Rect &rect);

	/** Cross fade feature
	 * @param buffer screen buffer
	 * @param palette new palette to cross fade
	 */
	void crossFade(const Graphics::ManagedSurface &buffer, const uint32 *palette);

	/** Handle keyboard pressed keys */
	void readKeys();

	/**
	 * Display text in screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param string text to display
	 * @param center if the text should be centered accoding with the giving positions
	 */
	void drawText(int32 x, int32 y, const char *string, int32 center);
};

} // namespace TwinE

#endif
