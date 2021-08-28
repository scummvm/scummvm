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
#include "engines/advancedDetector.h"
#include "engines/engine.h"

#include "engines/metaengine.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "twine/detection.h"
#include "twine/input.h"
#include "twine/scene/actor.h"

namespace TwinE {

/** Definition for European version */
#define EUROPE_VERSION 0
/** Definition for American version */
#define USA_VERSION 1
/** Definition for Modification version */
#define MODIFICATION_VERSION 2

/** Default frames per second */
#define DEFAULT_FRAMES_PER_SECOND 20

#define ORIGINAL_WIDTH 640
#define ORIGINAL_HEIGHT 480

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
	/** Flag to display game debug */
	bool Debug = false;
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
	TwinEEngine *_engine;
	ScopedEngineFreeze(TwinEEngine *engine);
	~ScopedEngineFreeze();
};

struct ScopedCursor {
	TwinEEngine *_engine;
	ScopedCursor(TwinEEngine *engine);
	~ScopedCursor();
};

class FrameMarker {
private:
	TwinEEngine *_engine;
	uint32 _fps;
	uint32 _start;
public:
	FrameMarker(TwinEEngine *engine, uint32 fps = DEFAULT_FRAMES_PER_SECOND);
	~FrameMarker();
};

class TwineScreen : public Graphics::Screen {
private:
	using Super = Graphics::Screen;
	TwinEEngine *_engine;

public:
	TwineScreen(TwinEEngine *engine);

	void update() override;
};

class TwinEEngine : public Engine {
private:
	int32 _isTimeFreezed = 0;
	int32 _saveFreezedTime = 0;
	int32 _mouseCursorState = 0;
	ActorMoveStruct _loopMovePtr; // mainLoopVar1
	PauseToken _pauseToken;
	TwineGameType _gameType;
	EngineState _state = EngineState::Menu;
	Common::String _queuedFlaMovie;

	ScriptLife *_scriptLife;
	ScriptMove *_scriptMove;

	Common::RandomSource _rnd;
	Common::Language _gameLang;

	void processBookOfBu();
	void processBonusList();
	void processInventoryAction();
	void processOptionsMenu();

	void initConfigurations();
	/** Initialize all needed stuffs at first time running engine */
	void initAll();
	void initEngine();
	void processActorSamplePosition(int32 actorIdx);
	/** Allocate video memory, both front and back buffers */
	void allocVideoMemory(int32 w, int32 h);

	/**
	 * Game engine main loop
	 * @return true if we want to show credit sequence
	 */
	int32 runGameEngine();
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
	bool isMod() const { return (_gameFlags & TwinE::TF_MOD) != 0; }
	bool isDemo() const { return (_gameFlags & ADGF_DEMO) != 0; };
	const char *getGameId() const;

	bool unlockAchievement(const Common::String &id);

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
	Holomap *_holomap;
	Sound *_sound;
	Text *_text;
	DebugGrid *_debugGrid;
	Input *_input;
	Debug *_debug;
	DebugScene *_debugScene;

	/** Configuration file structure
	 * Contains all the data used in the engine to configurated the game in particulary ways. */
	ConfigFile _cfgfile;

	int32 _frameCounter = 0;
	int32 _quitGame = 0;
	int32 _lbaTime = 0;

	int32 _loopInventoryItem = 0;
	int32 _loopActorStep = 0;
	uint32 _gameFlags;

	/** Disable screen recenter */
	bool _disableScreenRecenter = false;

	Graphics::ManagedSurface _imageBuffer;
	/** Work video buffer */
	Graphics::ManagedSurface _workVideoBuffer;
	/** Main game video buffer */
	TwineScreen _frontVideoBuffer;

	int width() const;
	int height() const;
	Common::Rect rect() const;
	Common::Rect centerOnScreen(int32 w, int32 h) const;
	Common::Rect centerOnScreenX(int32 w, int32 y, int32 h) const;

	void initSceneryView();
	void exitSceneryView();

	void queueMovie(const char *filename);

	/**
	 * @return A random value between [0-max)
	 */
	int getRandomNumber(uint max = 0x7FFF);

	void blitWorkToFront(const Common::Rect &rect);
	void blitFrontToWork(const Common::Rect &rect);
	void restoreFrontBuffer();
	void saveFrontBuffer();

	void freezeTime();
	void unfreezeTime();

	/**
	 * Game engine main loop
	 * @return true if we want to show credit sequence
	 */
	bool gameEngineLoop();

	/**
	 * Deplay certain seconds till proceed - Can also Skip this delay
	 * @param time time in milliseconds to delay
	 */
	bool delaySkip(uint32 time);

	/**
	 * Set a new palette in the SDL screen buffer
	 * @param palette palette to set in RGBA
	 */
	void setPalette(const uint32 *palette);
	/**
	 * @brief Set the Palette object
	 *
	 * @param startColor the first palette entry to be updated
	 * @param numColors the number of palette entries to be updated
	 * @param palette palette to set in RGB
	 */
	void setPalette(uint startColor, uint numColors, const byte *palette);

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
	 * @param palette new palette to cross fade
	 */
	void crossFade(const uint32 *palette);

	/** Handle keyboard pressed keys */
	void readKeys();

	/**
	 * Display text in screen
	 * @param x X coordinate in screen
	 * @param y Y coordinate in screen
	 * @param text text to display
	 * @param center if the text should be centered accoding with the giving positions
	 */
	void drawText(int32 x, int32 y, const Common::String &text, bool center = false, bool bigFont = false, int width = 100);
};

inline int TwinEEngine::width() const {
	return _frontVideoBuffer.w;
}

inline int TwinEEngine::height() const {
	return _frontVideoBuffer.h;
}

inline Common::Rect TwinEEngine::rect() const {
	return Common::Rect(0, 0, _frontVideoBuffer.w - 1, _frontVideoBuffer.h - 1);
}

} // namespace TwinE

#endif
