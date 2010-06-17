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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_H
#define SCI_H

#include "engines/engine.h"
#include "common/util.h"
#include "engine/vm_types.h"	// for Selector

struct ADGameDescription;

/**
 * This is the namespace of the SCI engine.
 *
 * Status of this engine: ???
 *
 * Supported games:
 * - ???
 */
namespace Sci {

// Uncomment this to use old music functions
//#define USE_OLD_MUSIC_FUNCTIONS

struct EngineState;
class Vocabulary;
class ResourceManager;
class Kernel;
class GameFeatures;
class Console;
class AudioPlayer;
class EventManager;

class GfxAnimate;
class GfxCache;
class GfxCompare;
class GfxControls;
class GfxCoordAdjuster;
class GfxCursor;
class GfxMacIconBar;
class GfxMenu;
class GfxPaint;
class GfxPaint16;
class GfxPaint32;
class GfxPalette;
class GfxPorts;
class GfxScreen;
class GfxText16;
class GfxTransitions;

#ifdef ENABLE_SCI32
class SciGui32;
class GfxFrameout;
#endif

// our engine debug levels
enum kDebugLevels {
	kDebugLevelError      = 1 << 0,
	kDebugLevelNodes      = 1 << 1,
	kDebugLevelGraphics   = 1 << 2,
	kDebugLevelStrings    = 1 << 3,
	kDebugLevelMemory     = 1 << 4,
	kDebugLevelFuncCheck  = 1 << 5,
	kDebugLevelBresen     = 1 << 6,
	kDebugLevelSound      = 1 << 7,
	kDebugLevelGfxDriver  = 1 << 8,
	kDebugLevelBaseSetter = 1 << 9,
	kDebugLevelParser     = 1 << 10,
	kDebugLevelMenu       = 1 << 11,
	kDebugLevelSaid       = 1 << 12,
	kDebugLevelFile       = 1 << 13,
	kDebugLevelTime       = 1 << 14,
	kDebugLevelRoom       = 1 << 15,
	kDebugLevelAvoidPath  = 1 << 16,
	kDebugLevelDclInflate = 1 << 17,
	kDebugLevelVM         = 1 << 18,
	kDebugLevelScripts    = 1 << 19,
	kDebugLevelGC         = 1 << 20,
	kDebugLevelSci0Pic    = 1 << 21,
	kDebugLevelResMan     = 1 << 22,
	kDebugLevelOnStartup  = 1 << 23
};

/** SCI versions */
enum SciVersion {
	SCI_VERSION_NONE,
	SCI_VERSION_0_EARLY, // Early KQ4, 1988 xmas card
	SCI_VERSION_0_LATE, // KQ4, LSL2, LSL3, SQ3 etc
	SCI_VERSION_01, // KQ1 and multilingual games (S.old.*)
	SCI_VERSION_1_EGA, // EGA with parser, QFG2
	SCI_VERSION_1_EARLY, // KQ5. (EGA/VGA)
	SCI_VERSION_1_MIDDLE, // LSL1, JONESCD. (EGA?/VGA)
	SCI_VERSION_1_LATE, // ECO1, LSL5. (EGA/VGA)
	SCI_VERSION_1_1, // KQ6, ECO2
	SCI_VERSION_2, // GK1, PQ4 (Floppy), QFG4 (Floppy)
	SCI_VERSION_2_1, // GK2, KQ7, SQ6, Torin
	SCI_VERSION_3 // LSL7, RAMA, Lighthouse
};

/** Supported languages */
enum kLanguage {
	K_LANG_NONE = 0,
	K_LANG_ENGLISH = 1,
	K_LANG_FRENCH = 33,
	K_LANG_SPANISH = 34,
	K_LANG_ITALIAN = 39,
	K_LANG_GERMAN = 49,
	K_LANG_JAPANESE = 81,
	K_LANG_PORTUGUESE = 351
};


class SciEngine : public Engine {
	friend class Console;
public:
	SciEngine(OSystem *syst, const ADGameDescription *desc);
	~SciEngine();

	// Engine APIs
	virtual Common::Error run();
	bool hasFeature(EngineFeature f) const;
	void pauseEngineIntern(bool pause);
	virtual GUI::Debugger *getDebugger();
	Console *getSciDebugger();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const char *desc);
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	void syncSoundSettings();

	const char* getGameID() const;
	int getResourceVersion() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	uint32 getFlags() const;
	bool isDemo() const;

	inline ResourceManager *getResMan() const { return _resMan; }
	inline Kernel *getKernel() const { return _kernel; }
	inline EngineState *getEngineState() const { return _gamestate; }
	inline Vocabulary *getVocabulary() const { return _vocabulary; }
	inline EventManager *getEventManager() const { return _eventMan; }
	inline reg_t getGameObject() const { return _gameObj; }

	Common::String getSavegameName(int nr) const;
	Common::String getSavegamePattern() const;

	Common::String getFilePrefix() const;

	/** Prepend 'TARGET-' to the given filename. */
	Common::String wrapFilename(const Common::String &name) const;

	/** Remove the 'TARGET-' prefix of the given filename, if present. */
	Common::String unwrapFilename(const Common::String &name) const;

	void sleep(uint32 msecs);

public:

	/**
	 * Processes a multilanguage string based on the current language settings and
	 * returns a string that is ready to be displayed.
	 * @param str		the multilanguage string
	 * @param sep		optional seperator between main language and subtitle language,
	 *					if NULL is passed no subtitle will be added to the returned string
	 * @return processed string
	 */
	Common::String strSplit(const char *str, const char *sep = "\r----------\r");

	kLanguage getSciLanguage();
	void setSciLanguage(kLanguage lang);
	void setSciLanguage();

	Common::String getSciLanguageString(const char *str, kLanguage lang, kLanguage *lang2 = NULL) const;

	// Initializes ports and paint16 for non-sci32 games, also sets default palette
	void initGraphics();

public:
	GfxAnimate *_gfxAnimate; // Animate for 16-bit gfx
	GfxCache *_gfxCache;
	GfxCompare *_gfxCompare;
	GfxControls *_gfxControls; // Controls for 16-bit gfx
	GfxCoordAdjuster *_gfxCoordAdjuster;
	GfxCursor *_gfxCursor;
	GfxMenu *_gfxMenu; // Menu for 16-bit gfx
	GfxPalette *_gfxPalette;
	GfxPaint *_gfxPaint;
	GfxPaint16 *_gfxPaint16; // Painting in 16-bit gfx
	GfxPaint32 *_gfxPaint32; // Painting in 32-bit gfx
	GfxPorts *_gfxPorts; // Port managment for 16-bit gfx
	GfxScreen *_gfxScreen;
	GfxText16 *_gfxText16;
	GfxTransitions *_gfxTransitions; // transitions between screens for 16-bit gfx
	GfxMacIconBar *_gfxMacIconBar; // Mac Icon Bar manager

#ifdef ENABLE_SCI32
	GfxFrameout *_gfxFrameout; // kFrameout and the like for 32-bit gfx
#endif

	AudioPlayer *_audio;
	GameFeatures *_features;

private:
	/**
	 * Initializes a SCI game
	 * This function must be run before script_run() is executed. Graphics data
	 * is initialized iff s->gfx_state != NULL.
	 * @param[in] s	The state to operate on
	 * @return		true on success, false if an error occurred.
	 */
	bool initGame();

	/**
	 * Runs a SCI game
	 * This is the main function for SCI games. It takes a valid state, loads
	 * script 0 to it, finds the game object, allocates a stack, and runs the
	 * init method of the game object. In layman's terms, this runs a SCI game.
	 * @param[in] s	Pointer to the pointer of the state to operate on
	  */
	void runGame();

	/**
	 * Uninitializes an initialized SCI game
	 * This function should be run after each script_run() call.
	 * @param[in] s	The state to operate on
	 */
	void exitGame();

#ifdef USE_OLD_MUSIC_FUNCTIONS
	/**
	 * Initializes the sound part of a SCI game
	 * This function may only be called if game_init() did not initialize
	 * the sound data.
	 * @param[in] s				The state to initialize the sound in
	 * @param[in] sound_flags	Flags to pass to the sound subsystem
	 * @param[in] soundVersion	sound-version that got detected during game init
	 */
	void initGameSound(int sound_flags, SciVersion soundVersion);
#endif

	void initStackBaseWithSelector(Selector selector);

	const ADGameDescription *_gameDescription;
	ResourceManager *_resMan; /**< The resource manager */
	EngineState *_gamestate;
	Kernel *_kernel;
	Vocabulary *_vocabulary;
	EventManager *_eventMan;
	reg_t _gameObj; /**< Pointer to the game object */
	Console *_console;
};


/**
 * Global instance of the SciEngine class, similar to g_engine.
 * This is a hackish way to make all central components available
 * everywhere. Ideally, we would get rid of this again in the future,
 * but for now it's a pragmatic and simple way to achieve the goal.
 */
extern SciEngine *g_sci;

/**
 * Convenience function to obtain the active SCI version.
 */
SciVersion getSciVersion();

/**
 * Convenience function converting an SCI version into a human-readable string.
 */
const char *getSciVersionDesc(SciVersion version);

} // End of namespace Sci

#endif // SCI_H
