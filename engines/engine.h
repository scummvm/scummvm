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

#ifndef ENGINES_ENGINE_H
#define ENGINES_ENGINE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"
#include "common/queue.h"
#include "common/singleton.h"
#include "engines/enhancements.h"

class OSystem;
class MetaEngineDetection;
class MetaEngine;

namespace Audio {
class Mixer;
}
namespace Common {
class Error;
class EventManager;
class SaveFileManager;
class TimerManager;
class FSNode;
class SeekableReadStream;
class WriteStream;
}
namespace GUI {
class Debugger;
class Dialog;
}

/**
 * @defgroup engines_engine Engine
 * @ingroup engines
 *
 * @brief API for managing various functions of the engine.
 *
 * @{
 */

/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessage(const Common::U32String &msg, const char *url = nullptr);
/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessage(const Common::String &msg, const char *url = nullptr);
/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessageWithURL(const Common::U32String &msg, const char *url);
/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessageWithURL(const Common::String &msg, const char *url);
/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessageFormatU32StringPtr(const Common::U32String *fmt, ...);
/**
 * Initialize graphics and show an error message.
 */
template<class... TParam>
inline void GUIErrorMessageFormat(const Common::U32String &fmt, TParam... param) {
	GUIErrorMessageFormatU32StringPtr(&fmt, Common::forward<TParam>(param)...);
}
/**
 * Initialize graphics and show an error message.
 */
void GUIErrorMessageFormat(MSVC_PRINTF const char *fmt, ...) GCC_PRINTF(1, 2);


class Engine;

/**
* Class for managing pausing by Engine::pauseEngine that hands out pause tokens.
*
* Each token represents one requested level of pause.
*/
class PauseToken {
public:
	PauseToken();
	/**
	 * Construct a pause token.
	 */
	PauseToken(const PauseToken &);
#if __cplusplus >= 201103L
	PauseToken(PauseToken &&);
#endif
	~PauseToken();
	/**
	 * Assign the pause token.
	 */
	void operator=(const PauseToken &);
#if __cplusplus >= 201103L
	void operator=(PauseToken &&);
#endif
	/**
	* Manually release the PauseToken.
	*
	* Only allowed if the token currently represents a pause request.
	*/
	void clear();

	/**
	 * Return true if the PauseToken represents a pause level,
	 * false if it is empty.
	 */
	bool isActive() const { return _engine != nullptr; }

private:
	PauseToken(Engine *);

	Engine *_engine;
	/**
	 * @see Engine
	 */
	friend class Engine;
};

class Engine {
public:
	/**
	 * The OSystem instance used by the engine.
	 */
	OSystem *_system;
	/**
	 * The Mixer instance used by the engine.
	 */
	Audio::Mixer *_mixer;

protected:
	/**
	 * The TimerManager instance used by the engine.
	 */
	Common::TimerManager *_timer;
	/**
	 * The EventManager instance used by the engine.
	 */
	Common::EventManager *_eventMan;
	/**
	 * The SaveFileManager used by the engine.
	 */
	Common::SaveFileManager *_saveFileMan;

	/**
	 * The Dialog instance used by the engine.
	 */
	GUI::Dialog *_mainMenuDialog;
	/**
	 * Run a GUI dialog.
	 */
	virtual int runDialog(GUI::Dialog &dialog);

	/**
	 * Target name for saves.
	 */
	const Common::String _targetName;

	int32 _activeEnhancements = kEnhGameBreakingBugFixes;

private:
	/**
	 * The associated metaengine
	 */
	MetaEngine *_metaEngine;

	/**
	 * The pause level.
	 *
	 * 0 means 'running'. A positive value indicates how often the engine
	 * has been paused and thus how many times it must be unpaused before
	 * it resumes running. This makes it possible to nest the code that pauses the engine.
	 */
	int _pauseLevel;

	/**
	 * The time when the pause was started.
	 */
	uint32 _pauseStartTime;

	/**
	 * The time when the engine was started.
	 *
	 * This value is used to calculate the current play time of the game running.
	 */
	int32 _engineStartTime;

	/**
	 * Autosave interval.
	 */
	int _autosaveInterval;

	/**
	 * The last time an autosave was done.
	 */
	int _lastAutosaveTime;

	/**
	 * Save slot selected via the global main menu.
	 *
	 * This slot will be loaded after the main menu execution (not from inside
	 * the menu loop, to avoid bugs like #4420).
	 */
	int _saveSlotToLoad;

	/**
	 * Used for preventing recursion during autosave.
	 */
	bool _autoSaving;

	/**
	 * Optional debugger for the engine.
	 */
	GUI::Debugger *_debugger;

	/**
	 * Flag for whether the quitGame method has been called
	 */
	static bool _quitRequested;

public:
	/**
	 * Engine features.
	 *
	 * A feature in this context means an ability of the engine
	 * that can be either available or not.
	 * @see Engine::hasFeature()
	 */
	enum EngineFeature {
		/**
		 * Enable the subtitle speed and toggle items in the Options section
		 * of the global main menu.
		 */
		kSupportsSubtitleOptions,

		/**
		 * The 'Return to launcher' feature is supported.
		 *
		 * This means that EVENT_RETURN_TO_LAUNCHER is handled
		 * either directly, or indirectly (the engine calls and honors
		 * the result of the Engine::shouldQuit() method appropriately).
		 */
		kSupportsReturnToLauncher,

		/**
		 * Loading save states during runtime is supported.
		 *
		 * This means that the engine implements loadGameState() and canLoadGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsLoadingDuringRuntime,

		/**
		 * Loading save states during runtime is supported.
		 *
		 * This means that the engine implements saveGameState() and canSaveGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsSavingDuringRuntime,

		/**
		 * Changing the game settings during runtime is supported.
		 *
		 * This enables showing the Engine Options tab in the config dialog
		 * accessed through the global main menu.
		 */
		kSupportsChangingOptionsDuringRuntime,

		/**
		 * Arbitrary resolutions are supported.
		 *
		 * This means that the engine allows the backend to override the resolution
		 * passed to OSystem::setupScreen.
		 * The engine will need to read the actual resolution used by the
		 * backend using OSystem::getWidth and OSystem::getHeight.
		 */
		kSupportsArbitraryResolutions,

		/**
		 * The game provides custom help.
		 *
		 * This enables the help button in the main menu.
		 */
		 kSupportsHelp,

		/**
		 * The engine provides overrides to the quit and exit to launcher dialogs.
		 */
		kSupportsQuitDialogOverride,
	};



	/** @name Overloadable methods
	 *
	 *  All Engine subclasses should consider overloading some or all of the following methods.
	 *  @{
	 */

	/**
	 * Create an instance of the engine.
	 */
	Engine(OSystem *syst);
	/**
	 * Destroy the engine instance.
	 */
	virtual ~Engine();

	/**
	 * Initialize SearchMan according to the game path.
	 *
	 * By default, this adds the directory in non-flat mode with a depth of 4 as
	 * priority 0 to SearchMan.
	 *
	 * @param gamePath The base directory of the game data.
	 */
	virtual void initializePath(const Common::FSNode &gamePath);

	/**
	 * Initialize the engine and start its main loop.
	 *
	 * @return kNoError on success, otherwise an error code.
	 */
	virtual Common::Error run() = 0;

	/**
	 * Prepare an error string that is printed by the error() function.
	 */
	virtual void errorString(const char *buf_input, char *buf_output, int buf_output_size);

	/**
	 * Return the engine's debugger instance, if any.
	 */
	virtual GUI::Debugger *getDebugger() final { return _debugger; }

	/**
	 * Set the engine's debugger.
	 *
	 * Once set, the Engine class is responsible for managing
	 * the debugger, and freeing it on exit.
	 */
	void setDebugger(GUI::Debugger *debugger) {
		assert(!_debugger);
		_debugger = debugger;
	}

	/**
	 * Return the engine's debugger instance, or create one if none is present.
	 *
	 * Used by error() to invoke the debugger when a severe error is reported.
	 */
	GUI::Debugger *getOrCreateDebugger();

	/**
	 * Determine whether the engine supports the specified feature.
	 */
	virtual bool hasFeature(EngineFeature f) const { return false; }

	bool enhancementEnabled(int32 cls);

	/**
	 * Notify the engine that the sound settings in the config manager might have
	 * changed and that it should adjust any internal volume (and other) values
	 * accordingly.
	 *
	 * The default implementation sets the volume levels of all mixer sound
	 * types according to the config entries of the active domain.
	 * When overwriting, call the default implementation first, then adjust the
	 * volumes further (if required).
	 *
	 * @note When setting volume levels, respect the "mute" config entry.
	 *
	 * @note The volume for the plain sound type is reset to the maximum
	 *       volume. If the engine can associate its own value for this
	 *       type, it needs to overwrite this member and set it accordingly.
	 * @todo find a better name for this
	 */
	virtual void syncSoundSettings();

	/**
	 * Notify the engine that the settings editable from the Game tab in the
	 * in-game options dialog might have changed and that they need to be applied
	 * if necessary.
	 */
	virtual void applyGameSettings() {}

	/**
	 * Flip mute all sound options.
	 */
	virtual void flipMute();

	/**
	 * Generate the savegame filename.
	 */
	virtual Common::String getSaveStateName(int slot) const {
		return Common::String::format("%s.%03d", _targetName.c_str(), slot);
	}

	/**
	 * Load a game state.
	 *
	 * @param slot	The slot from which a save state should be loaded.
	 *
	 * @return kNoError on success, otherwise an error code.
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Load a game state.
	 *
	 * @param stream	The stream to load the save state from.
	 *
	 * @return kNoError on success, otherwise an error code.
	 */
	virtual Common::Error loadGameStream(Common::SeekableReadStream *stream);

	/**
	 * Set the game slot for a savegame to be loaded after the global
	 * main menu execution.
	 *
	 * This is to avoid loading a savegame from
	 * inside the menu loop which causes bugs like #4420.
	 *
	 * @param slot	The slot from which a save state should be loaded.
	 */
	void setGameToLoadSlot(int slot);

	/**
	 * Indicate whether a game state can be loaded.
	 *
	 * @param msg        Optional pointer to message explaining why it is disabled
	 */
	virtual bool canLoadGameStateCurrently(Common::U32String *msg = nullptr);

	/**
	 * Save a game state.
	 *
	 * @param slot        The slot into which the save state should be stored.
	 * @param desc        Description for the save state, entered by the user.
	 * @param isAutosave  Expected to be true if an autosave is being created.
	 *
	 * @return kNoError on success, otherwise an error code.
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);

	/**
	 * Save a game state.
	 *
	 * @param stream      The write stream to save the savegame data to.
	 * @param isAutosave  Expected to be true if an autosave is being created.
	 *
	 * @return kNoError on success, otherwise an error code.
	 */
	virtual Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false);

	/**
	 * Indicate whether a game state can be saved.
	 *
	 * @param msg        Optional pointer to message explaining why it is disabled
	 */
	virtual bool canSaveGameStateCurrently(Common::U32String *msg = nullptr);

	/**
	 * Show the ScummVM save dialog, allowing users to save their game.
	 */
	bool saveGameDialog();

	/**
	 * Show the ScummVM Restore dialog, allowing users to load a game.
	 */
	bool loadGameDialog();

protected:
	/**
	 * Actual implementation of pauseEngine by subclasses.
	 *
	 * @see Engine::pauseEngine
	 */
	virtual void pauseEngineIntern(bool pause);

	 /** @} */


public:

	/**
	 * Request the engine to quit.
	 *
	 * Sends an EVENT_QUIT event to the Event Manager.
	 */
	static void quitGame();

	/**
	 * Return whether the engine should quit.
	 *
	 * This can mean either quitting ScummVM altogether, or returning to the launcher.
	 */
	static bool shouldQuit();

	/**
	 * Return the MetaEngine instance used by this engine.
	 */
	inline MetaEngine *getMetaEngine() const { return _metaEngine; }

	/**
	 * Set the MetaEngine instance used by this engine.
	 */
	inline void setMetaEngine(MetaEngine *metaEngine) { _metaEngine = metaEngine; }

	/**
	 * Pause the engine.
	 *
	 * This should stop any audio playback and other operations.
	 * Called right before the system runs a global dialog
	 * (like a global pause, main menu, options, or 'confirm exit' dialog).
	 *
	 * Returns a PauseToken. Multiple pause tokens can exist. The engine will
	 * be resumed when all associated pause tokens reach the end of their lives.
	 */
	PauseToken pauseEngine();
private:
	/** Resume the engine.
	 *
	 * This should resume any audio playback and other operations.
	 * Only PauseToken is allowed to call this member function. Use the PauseToken
	 * that you got from pauseEngine to resume the engine.
	 */
	void resumeEngine();
	/**
	 * @see Engine::PauseToken
	 */
	friend class PauseToken;

	/**
	 * Warns before overwriting autosave.
	 *
	 * @return true if it is safe to save, false to avoid saving.
	 */
	bool warnBeforeOverwritingAutosave();

public:

	/**
	 * Return whether the engine is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Run the global main menu dialog.
	 */
	void openMainMenuDialog();

	/**
	 * Display a warning to the user that the game is not fully supported.
	 *
	 * @param msg  A message that will be presented to user. If empty, then
	 *             generic message regarding unsupported game is presented
	 *
	 * @return True if the user chooses to start anyway, false otherwise.
	 */
	static bool warnUserAboutUnsupportedGame(Common::String msg = Common::String());

	/**
	 * Display an error message to the user that the game is not supported.
	 *
	 * @param extraMsg  An extra message that will be appended to the default message.
	 */
	static void errorUnsupportedGame(Common::String extraMsg);

	/**
	 * Get the total play time.
	 *
	 * @return How long the player has been playing in ms.
	 */
	uint32 getTotalPlayTime() const;

	/**
	 * Set the game time counter to the specified time.
	 *
	 * This can be used to set the play time counter after loading a savegame,
	 * for example. Another use case is when the engine wants to exclude from
	 * the counter the time that the user spent in original engine dialogs.
	 *
	 * @param time Play time to set up in ms.
	 */
	void setTotalPlayTime(uint32 time = 0);

	/**
	 * Return the TimerManager instance used by this engine.
	 */
	inline Common::TimerManager *getTimerManager() { return _timer; }
	/**
	 * Return the EventManager instance used by this engine.
	 */
	inline Common::EventManager *getEventManager() { return _eventMan; }
	/**
	 * Return the SaveFileManager instance used by this engine.
	 */
	inline Common::SaveFileManager *getSaveFileManager() { return _saveFileMan; }

public:
	/**
	 * Check if extracted CD Audio files are found.
	 */
	bool existExtractedCDAudioFiles(uint track = 1);
	/**
	 * On some systems, check whether the game appears to be run
	 * from the same CD drive, which also should play CD audio.
	 */
	bool isDataAndCDAudioReadFromSameCD();
	/**
	 *Display a warning for no extracted CD Audio files found.
	 */
	void warnMissingExtractedCDAudio();

	/**
	 * Check whether it is time to autosave, and if so, do it.
	 */
	void handleAutoSave();

	/**
	 * Autosave immediately if autosaves are enabled.
	 */
	void saveAutosaveIfEnabled();

	/**
	 * Indicate whether an autosave can currently be done.
	 */
	virtual bool canSaveAutosaveCurrently() {
		return canSaveGameStateCurrently();
	}

	/**
	 * Return the slot that should be used for autosaves, or -1 for engines that
	 * don't support autosave.
	 *
	 * @note	This should match the meta engine getAutosaveSlot() method.
	 */
	virtual int getAutosaveSlot() const {
		return 0;
	}

protected:
	/**
	 * Syncs the engine's mixer using the default volume syncing behavior.
	 */
	void defaultSyncSoundSettings();
};


/** @name Chained games
*
*  @{
*/

/**
 * Singleton class which manages chained games.
 *
 * A chained game is one that starts automatically, optionally loading
 * a saved game, instead of returning to the launcher.
 */
class ChainedGamesManager : public Common::Singleton<ChainedGamesManager> {
private:
	struct Game {
		Common::String target;
		int slot;
	};

	Common::Queue<Game> _chainedGames;

public:
	ChainedGamesManager();
	/** Clear the chained games manager of any games. */
	void clear();
	/** Load a game into a slot in the chained games manager. */
	void push(const Common::String target, const int slot = -1);
	/** Pop the last game loaded into the chained games manager. */
	bool pop(Common::String &target, int &slot);
	/** Returns true if the chained games manager has no elements in the queue. */
	bool empty() { return _chainedGames.empty(); }
};

/** Convenience shortcut for accessing the chained games manager. */
#define ChainedGamesMan ChainedGamesManager::instance()
/** @} */
/** Current workaround for MidiEmu & error(). */ //FIXME
extern Engine *g_engine;
/** @} */
#endif
