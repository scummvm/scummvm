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

#ifndef SCI_SCI_H
#define SCI_SCI_H

#include "engines/engine.h"
#include "common/macresman.h"
#include "common/util.h"
#include "common/random.h"
#include "sci/engine/vm_types.h"	// for Selector
#include "sci/debug.h"	// for DebugState

struct ADGameDescription;

/**
 * This is the namespace of the SCI engine.
 *
 * Status of this engine: Awesome
 *
 * Games using this engine:
 * - Newer Sierra adventure games (based on FreeSCI)
 *
 * @todo give a concrete list of supported games. Could also
 * list future games, with status for each.
 */
namespace Sci {

// GUI-options, primarily used by detection_tables.h
#define GAMEOPTION_PREFER_DIGITAL_SFX       GUIO_GAMEOPTIONS1
#define GAMEOPTION_ORIGINAL_SAVELOAD        GUIO_GAMEOPTIONS2
#define GAMEOPTION_FB01_MIDI                GUIO_GAMEOPTIONS3
#define GAMEOPTION_JONES_CDAUDIO            GUIO_GAMEOPTIONS4
#define GAMEOPTION_KQ6_WINDOWS_CURSORS      GUIO_GAMEOPTIONS5
#define GAMEOPTION_SQ4_SILVER_CURSORS       GUIO_GAMEOPTIONS6
#define GAMEOPTION_EGA_UNDITHER             GUIO_GAMEOPTIONS7
// HIGH_RESOLUTION_GRAPHICS availability is checked for in SciEngine::run()
#define GAMEOPTION_HIGH_RESOLUTION_GRAPHICS GUIO_GAMEOPTIONS8
#define GAMEOPTION_ENABLE_BLACK_LINED_VIDEO GUIO_GAMEOPTIONS9
#define GAMEOPTION_HQ_VIDEO                 GUIO_GAMEOPTIONS10
#define GAMEOPTION_ENABLE_CENSORING         GUIO_GAMEOPTIONS11
#define GAMEOPTION_LARRYSCALE               GUIO_GAMEOPTIONS12
#define GAMEOPTION_UPSCALE_VIDEOS           GUIO_GAMEOPTIONS13

struct EngineState;
class Vocabulary;
class ResourceManager;
class Kernel;
class GameFeatures;
class GuestAdditions;
class Console;
class AudioPlayer;
class SoundCommandParser;
class EventManager;
class SegManager;
class ScriptPatcher;
class Sync;

class GfxAnimate;
class GfxCache;
class GfxCompare;
class GfxControls16;
class GfxControls32;
class GfxCoordAdjuster16;
class GfxCursor;
class GfxMacIconBar;
class GfxMenu;
class GfxPaint16;
class GfxPaint32;
class GfxPalette;
class GfxPalette32;
class GfxRemap;
class GfxRemap32;
class GfxPorts;
class GfxScreen;
class GfxText16;
class GfxText32;
class GfxTransitions;

#ifdef ENABLE_SCI32
class GfxFrameout;
class Audio32;
class Video32;
class GfxTransitions32;
class GfxCursor32;
#endif

// our engine debug levels
enum kDebugLevels {
	kDebugLevelError         = 1 << 0,
	kDebugLevelNodes         = 1 << 1,
	kDebugLevelGraphics      = 1 << 2,
	kDebugLevelStrings       = 1 << 3,
	kDebugLevelMemory        = 1 << 4,
	kDebugLevelFuncCheck     = 1 << 5,
	kDebugLevelBresen        = 1 << 6,
	kDebugLevelSound         = 1 << 7,
	kDebugLevelBaseSetter    = 1 << 8,
	kDebugLevelParser        = 1 << 9,
	kDebugLevelSaid          = 1 << 10,
	kDebugLevelFile          = 1 << 11,
	kDebugLevelTime          = 1 << 12,
	kDebugLevelRoom          = 1 << 13,
	kDebugLevelAvoidPath     = 1 << 14,
	kDebugLevelDclInflate    = 1 << 15,
	kDebugLevelVM            = 1 << 16,
	kDebugLevelScripts       = 1 << 17,
	kDebugLevelGC            = 1 << 18,
	kDebugLevelResMan        = 1 << 19,
	kDebugLevelOnStartup     = 1 << 20,
	kDebugLevelDebugMode     = 1 << 21,
	kDebugLevelScriptPatcher = 1 << 22,
	kDebugLevelWorkarounds   = 1 << 23,
	kDebugLevelVideo         = 1 << 24,
	kDebugLevelGame          = 1 << 25
};

enum SciGameId {
	GID_ASTROCHICKEN,
	GID_CAMELOT,
	GID_CASTLEBRAIN,
	GID_CHEST,
	GID_CHRISTMAS1988,
	GID_CHRISTMAS1990,
	GID_CHRISTMAS1992,
	GID_CNICK_KQ,
	GID_CNICK_LAURABOW,
	GID_CNICK_LONGBOW,
	GID_CNICK_LSL,
	GID_CNICK_SQ,
	GID_ECOQUEST,
	GID_ECOQUEST2,
	GID_FAIRYTALES,
	GID_FREDDYPHARKAS,
	GID_FUNSEEKER,
	GID_GK1DEMO,	// We have a separate ID for GK1 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_GK1,
	GID_GK2,
	GID_HOYLE1,
	GID_HOYLE2,
	GID_HOYLE3,
	GID_HOYLE4,
	GID_HOYLE5,
	GID_ICEMAN,
	GID_INNDEMO,
	GID_ISLANDBRAIN,
	GID_JONES,
	GID_KQ1,
	GID_KQ4,
	GID_KQ5,
	GID_KQ6,
	GID_KQ7,
	GID_KQUESTIONS,
	GID_LAURABOW,
	GID_LAURABOW2,
	GID_LIGHTHOUSE,
	GID_LONGBOW,
	GID_LSL1,
	GID_LSL2,
	GID_LSL3,
	GID_LSL5,
	GID_LSL6,
	GID_LSL6HIRES, // We have a separate ID for LSL6 SCI32, because it's actually a completely different game
	GID_LSL7,
	GID_MOTHERGOOSE, // this one is the SCI0 version
	GID_MOTHERGOOSE256, // this one handles SCI1 and SCI1.1 variants, at least those 2 share a bit in common
	GID_MOTHERGOOSEHIRES, // this one is the SCI2.1 hires version, completely different from the other ones
	GID_MSASTROCHICKEN,
	GID_PEPPER,
	GID_PHANTASMAGORIA,
	GID_PHANTASMAGORIA2,
	GID_PQ1,
	GID_PQ2,
	GID_PQ3,
	GID_PQ4,
	GID_PQ4DEMO,	// We have a separate ID for PQ4 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_PQSWAT,
	GID_QFG1,
	GID_QFG1VGA,
	GID_QFG2,
	GID_QFG3,
	GID_QFG4,
	GID_QFG4DEMO,	// We have a separate ID for QFG4 demo, because it's actually a completely different game (SCI1.1 vs SCI2/SCI2.1)
	GID_RAMA,
	GID_SHIVERS,
	//GID_SHIVERS2,	// Not SCI
	GID_SLATER,
	GID_SQ1,
	GID_SQ3,
	GID_SQ4,
	GID_SQ5,
	GID_SQ6,
	GID_TORIN,
	GID_FANMADE
};

/**
 * SCI versions
 * For more information, check here:
 * https://wiki.scummvm.org/index.php/Sierra_Game_Versions#SCI_Games
 */
enum SciVersion {
	SCI_VERSION_NONE,
	SCI_VERSION_0_EARLY, // KQ4 early, LSL2 early, XMAS card 1988
	SCI_VERSION_0_LATE, // KQ4, LSL2, LSL3, SQ3 etc
	SCI_VERSION_01, // KQ1 and multilingual games (S.old.*)
	SCI_VERSION_1_EGA_ONLY, // SCI 1 EGA with parser (i.e. QFG2 only)
	SCI_VERSION_1_EARLY, // KQ5 floppy, SQ4 floppy, XMAS card 1990, Fairy tales, Jones floppy
	SCI_VERSION_1_MIDDLE, // LSL1, Jones CD, LSL3 & SQ3 multilingual Amiga
	SCI_VERSION_1_LATE, // Dr. Brain 1, EcoQuest 1, Longbow, PQ3, SQ1, LSL5, KQ5 CD
	SCI_VERSION_1_1, // Dr. Brain 2, EcoQuest 1 CD, EcoQuest 2, KQ6, QFG3, SQ4CD, XMAS 1992 and many more
	SCI_VERSION_2, // GK1, PQ4 floppy, QFG4 floppy
	SCI_VERSION_2_1_EARLY, // GK2 demo, KQ7 1.4/1.51, LSL6 hires, PQ4CD, QFG4CD
	SCI_VERSION_2_1_MIDDLE, // GK2, Hoyle 5, KQ7 2.00b, MUMG Deluxe, Phantasmagoria 1, PQ:SWAT, Shivers 1, SQ6, Torin
	SCI_VERSION_2_1_LATE, // demos of LSL7, Lighthouse, RAMA
	SCI_VERSION_3 // LSL7, Lighthouse, RAMA, Phantasmagoria 2
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
	SciEngine(OSystem *syst, const ADGameDescription *desc, SciGameId gameId);
	~SciEngine();

	// Engine APIs
	virtual Common::Error run();
	bool hasFeature(EngineFeature f) const;
	void pauseEngineIntern(bool pause);
	virtual GUI::Debugger *getDebugger();
	Console *getSciDebugger();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	void syncSoundSettings(); ///< from ScummVM to the game
	void updateSoundMixerVolumes();
	uint32 getTickCount();
	void setTickCount(const uint32 ticks);

	const SciGameId &getGameId() const { return _gameId; }
	const char *getGameIdStr() const;
	int getResourceVersion() const;
	Common::Language getLanguage() const;
	Common::Platform getPlatform() const;
	bool isDemo() const;
	bool isCD() const;
	bool forceHiresGraphics() const;

	/** Returns true if the game's original platform is big-endian. */
	bool isBE() const;

	bool hasParser() const;
	bool hasMacIconBar() const;

	inline ResourceManager *getResMan() const { return _resMan; }
	inline ScriptPatcher *getScriptPatcher() const { return _scriptPatcher; }
	inline Kernel *getKernel() const { return _kernel; }
	inline EngineState *getEngineState() const { return _gamestate; }
	inline Vocabulary *getVocabulary() const { return _vocabulary; }
	inline EventManager *getEventManager() const { return _eventMan; }
	inline reg_t getGameObject() const { return _gameObjectAddress; } // Gets the game object VM address

	Common::RandomSource &getRNG() { return _rng; }

	Common::String getSavegameName(int nr) const;
	Common::String getSavegamePattern() const;

	Common::String getFilePrefix() const;

	/** Prepend 'TARGET-' to the given filename. */
	Common::String wrapFilename(const Common::String &name) const;

	/** Remove the 'TARGET-' prefix of the given filename, if present. */
	Common::String unwrapFilename(const Common::String &name) const;

	const char *getGameObjectName(); // Gets the name of the game object (should only be used for identifying fanmade games)

	/**
	 * Checks if we are in a QfG import screen, where special handling
	 * of file-listings is performed.
	 */
	int inQfGImportRoom() const;

	/* Shows a ScummVM message box explaining how to import Qfg saved character files */
	void showQfgImportMessageBox() const;

	void sleep(uint32 msecs);

	void scriptDebug();
	bool checkExportBreakpoint(uint16 script, uint16 pubfunct);
	bool checkSelectorBreakpoint(BreakpointType breakpointType, reg_t send_obj, int selector);
	bool checkAddressBreakpoint(const reg_t &address);

public:
	bool checkKernelBreakpoint(const Common::String &name);

	/**
	 * Processes a multilanguage string based on the current language settings and
	 * returns a string that is ready to be displayed.
	 * @param str		the multilanguage string
	 * @param sep		optional seperator between main language and subtitle language,
	 *					if NULL is passed no subtitle will be added to the returned string
	 * @return processed string
	 */
	Common::String strSplitLanguage(const char *str, uint16 *splitLanguage, const char *sep = "\r----------\r");
	Common::String strSplit(const char *str, const char *sep = "\r----------\r") {
		return strSplitLanguage(str, NULL, sep);
	}

	kLanguage getSciLanguage();
	void setSciLanguage(kLanguage lang);
	void setSciLanguage();

	Common::String getSciLanguageString(const Common::String &str, kLanguage lang, kLanguage *lang2 = NULL, uint16 *languageSplitter = NULL) const;

	// Check if vocabulary needs to get switched (in multilingual parser games)
	void checkVocabularySwitch();

	// Initializes ports and paint16 for non-sci32 games, also sets default palette
	void initGraphics();

public:
	GfxAnimate *_gfxAnimate; // Animate for 16-bit gfx
	GfxCache *_gfxCache;
	GfxCompare *_gfxCompare;
	GfxControls16 *_gfxControls16; // Controls for 16-bit gfx
	GfxControls32 *_gfxControls32; // Controls for 32-bit gfx
	GfxCoordAdjuster16 *_gfxCoordAdjuster;
	GfxCursor *_gfxCursor;
	GfxMenu *_gfxMenu; // Menu for 16-bit gfx
	GfxPalette *_gfxPalette16;
	GfxPalette32 *_gfxPalette32; // Palette for 32-bit gfx
	GfxRemap *_gfxRemap16;	// Remapping for the QFG4 demo
	GfxRemap32 *_gfxRemap32; // Remapping for 32-bit gfx
	GfxPaint16 *_gfxPaint16; // Painting in 16-bit gfx
	GfxPaint32 *_gfxPaint32; // Painting in 32-bit gfx
	GfxPorts *_gfxPorts; // Port managment for 16-bit gfx
	GfxScreen *_gfxScreen;
	GfxText16 *_gfxText16;
	GfxText32 *_gfxText32;
	GfxTransitions *_gfxTransitions; // transitions between screens for 16-bit gfx
	GfxMacIconBar *_gfxMacIconBar; // Mac Icon Bar manager

#ifdef ENABLE_SCI32
	Audio32 *_audio32;
	Video32 *_video32;
	GfxFrameout *_gfxFrameout; // kFrameout and the like for 32-bit gfx
	GfxTransitions32 *_gfxTransitions32;
	GfxCursor32 *_gfxCursor32;
#endif

	AudioPlayer *_audio;
	Sync *_sync;
	SoundCommandParser *_soundCmd;
	GameFeatures *_features;
	GuestAdditions *_guestAdditions;

	opcode_format (*_opcode_formats)[4];

	DebugState _debugState;

	Common::MacResManager *getMacExecutable() { return &_macExecutable; }

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

	/**
	 * Loads the Mac executable for SCI1 games
	 */
	void loadMacExecutable();

	void initStackBaseWithSelector(Selector selector);

	bool gameHasFanMadePatch();
	void setLauncherLanguage();

	const ADGameDescription *_gameDescription;
	const SciGameId _gameId;
	ResourceManager *_resMan; /**< The resource manager */
	ScriptPatcher *_scriptPatcher; /**< The script patcher */
	EngineState *_gamestate;
	Kernel *_kernel;
	Vocabulary *_vocabulary;
	int16 _vocabularyLanguage;
	EventManager *_eventMan;
	reg_t _gameObjectAddress; /**< Pointer to the game object */
	Console *_console;
	Common::RandomSource _rng;
	Common::MacResManager _macExecutable;
	bool _forceHiresGraphics; // user-option for GK1, KQ6, PQ4
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
 * Same as above, but this version doesn't assert on unknown SCI versions.
 * Only used by the fallback detector
 */
SciVersion getSciVersionForDetection();

/**
 * Convenience function converting an SCI version into a human-readable string.
 */
const char *getSciVersionDesc(SciVersion version);

} // End of namespace Sci

#endif // SCI_SCI_H
