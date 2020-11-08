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

#ifndef ENGINES_ENGINE_H
#define ENGINES_ENGINE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/language.h"
#include "common/platform.h"
#include "common/queue.h"
#include "common/singleton.h"

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
 * Initializes graphics and shows error message.
 */
void GUIErrorMessage(const Common::U32String &msg, const char *url = nullptr);
void GUIErrorMessage(const Common::String &msg, const char *url = nullptr); // Redirect to GUIErrorMessage with U32Strings
void GUIErrorMessageWithURL(const Common::U32String &msg, const char *url);
void GUIErrorMessageWithURL(const Common::String &msg, const char *url); // Redirect to GUIErrorMessageWithURL with U32Strings
void GUIErrorMessageFormat(Common::U32String fmt, ...);
void GUIErrorMessageFormat(const char *fmt, ...) GCC_PRINTF(1, 2);

class Engine;


/**
* Manages pausing by Engine::pauseEngine handing out tokens that
* each represent one requested level of pause.
*/
class PauseToken {
public:
	PauseToken();
	PauseToken(const PauseToken &);
#if __cplusplus >= 201103L
	PauseToken(PauseToken &&);
#endif
	~PauseToken();

	void operator=(const PauseToken &);
#if __cplusplus >= 201103L
	void operator=(PauseToken &&);
#endif
	/** Manually releases the PauseToken. Only allowed if the token
	* currently represents a pause request.
	*/
	void clear();

	/**
	 * Returns true if the PauseToken represents a pause level,
	 * false if it is empty.
	 */
	bool isActive() const { return _engine != nullptr; }

private:
	PauseToken(Engine *);

	Engine *_engine;

	friend class Engine;
};

class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;

protected:
	Common::TimerManager *_timer;
	Common::EventManager *_eventMan;
	Common::SaveFileManager *_saveFileMan;

	GUI::Dialog *_mainMenuDialog;
	virtual int runDialog(GUI::Dialog &dialog);

	const Common::String _targetName; // target name for saves

private:
	/**
	 * The pause level, 0 means 'running', a positive value indicates
	 * how often the engine has been paused (and hence how often it has
	 * to be un-paused before it resumes running). This makes it possible
	 * to nest code which pauses the engine.
	 */
	int _pauseLevel;

	/**
	 * The time when the pause was started.
	 */
	uint32 _pauseStartTime;

	/**
	 * The time when the engine was started. This value is used to calculate
	 * the current play time of the game running.
	 */
	int32 _engineStartTime;

	/**
	 * Autosave interval
	 */
	const int _autosaveInterval;

	/**
	 * The last time an autosave was done
	 */
	int _lastAutosaveTime;

	/**
	 * Save slot selected via global main menu.
	 * This slot will be loaded after main menu execution (not from inside
	 * the menu loop, to avoid bugs like #2822778).
	 */
	int _saveSlotToLoad;

	/**
	 * Optional debugger for the engine
	 */
	GUI::Debugger *_debugger;
public:


	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 * @see Engine::hasFeature()
	 */
	enum EngineFeature {
		/**
		 * Enables the subtitle speed and toggle items in the Options section
		 * of the global main menu.
		 */
		kSupportsSubtitleOptions,

		/**
		 * 'Return to launcher' feature is supported, i.e., EVENT_RETURN_TO_LAUNCHER is handled
		 * either directly, or indirectly (that is, the engine calls and honors
		 * the result of the Engine::shouldQuit() method appropriately).
		 */
		kSupportsReturnToLauncher,

		/**
		 * Loading savestates during runtime is supported, that is, this engine
		 * implements loadGameState() and canLoadGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsLoadingDuringRuntime,

		/**
		 * Loading savestates during runtime is supported, that is, this engine
		 * implements saveGameState() and canSaveGameStateCurrently().
		 * If this feature is supported, then the corresponding MetaEngine *must*
		 * support the kSupportsListSaves feature.
		 */
		kSupportsSavingDuringRuntime,

		/**
		 * Changing the game settings during runtime is supported. This enables
		 * showing the engine options tab in the config dialog accessed through
		 * the Global Main Menu.
		 */
		kSupportsChangingOptionsDuringRuntime,

		/**
		 * Arbitrary resolutions are supported, that is, this engine allows
		 * the backend to override the resolution passed to OSystem::setupScreen.
		 * The engine will need to read the actual resolution used by the
		 * backend using OSystem::getWidth and OSystem::getHeight.
		 */
		kSupportsArbitraryResolutions
	};



	/** @name Overloadable methods
	 *
	 *  All Engine subclasses should consider overloading some or all of the following methods.
	 */
	//@{

	Engine(OSystem *syst);
	virtual ~Engine();

	/**
	 * Init SearchMan according to the game path.
	 *
	 * By default it adds the directory in non-flat mode with a depth of 4 as
	 * priority 0 to SearchMan.
	 *
	 * @param gamePath The base directory of the game data.
	 */
	virtual void initializePath(const Common::FSNode &gamePath);

	/**
	 * Init the engine and start its main loop.
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error run() = 0;

	/**
	 * Prepare an error string, which is printed by the error() function.
	 */
	virtual void errorString(const char *buf_input, char *buf_output, int buf_output_size);

	/**
	 * Return the engine's debugger instance, if any.
	 */
	virtual GUI::Debugger *getDebugger() { return _debugger; }

	/**
	 * Sets the engine's debugger. Once set, the Engine class is responsible for managing
	 * the debugger, and freeing it on exit
	 */
	void setDebugger(GUI::Debugger *debugger) {
		assert(!_debugger);
		_debugger = debugger;
	}

	/**
	 * Return the engine's debugger instance, or create one if none is present.
	 * Used by error() to invoke the debugger when a severe error is reported.
	 */
	GUI::Debugger *getOrCreateDebugger();

	/**
	 * Determine whether the engine supports the specified feature.
	 */
	virtual bool hasFeature(EngineFeature f) const { return false; }

	/**
	 * Notify the engine that the sound settings in the config manager may have
	 * changed and that it hence should adjust any internal volume etc. values
	 * accordingly.
	 * The default implementation sets the volume levels of all mixer sound
	 * types according to the config entries of the active domain.
	 * When overwriting, call the default implementation first, then adjust the
	 * volumes further (if required).
	 *
	 * @note When setting volume levels, respect the "mute" config entry.
	 * @note The volume for the plain sound type is reset to the maximum
	 *       volume. If the engine can associate its own value for this
	 *       type, it needs to overwrite this member and set it accordingly.
	 * @todo find a better name for this
	 */
	virtual void syncSoundSettings();

	/**
	 * Notify the engine that the settings editable from the game tab in the
	 * in-game options dialog may have changed and that they need to be applied
	 * if necessary.
	 */
	virtual void applyGameSettings() {}

	/**
	 * Flip mute all sound option.
	 */
	virtual void flipMute();

	/**
	 * Generates the savegame filename
	 */
	virtual Common::String getSaveStateName(int slot) const {
		return Common::String::format("%s.%03d", _targetName.c_str(), slot);
	}

	/**
	 * Load a game state.
	 * @param slot	the slot from which a savestate should be loaded
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Load a game state.
	 * @param stream	the stream to load the savestate from
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error loadGameStream(Common::SeekableReadStream *stream);

	/**
	 * Sets the game slot for a savegame to be loaded after global
	 * main menu execution. This is to avoid loading a savegame from
	 * inside the menu loop which causes bugs like #2822778.
	 *
	 * @param slot	the slot from which a savestate should be loaded.
	 */
	void setGameToLoadSlot(int slot);

	/**
	 * Indicates whether a game state can be loaded.
	 */
	virtual bool canLoadGameStateCurrently();

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @param isAutosave	Expected to be true if an autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false);

	/**
	 * Save a game state.
	 * @param stream	The write stream to save the savegame data to
	 * @param isAutosave	Expected to be true if an autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false);

	/**
	 * Indicates whether a game state can be saved.
	 */
	virtual bool canSaveGameStateCurrently();

	/**
	 * Shows the ScummVM save dialog, allowing users to save their game
	 */
	bool saveGameDialog();

	/**
	 * Shows the ScummVM Restore dialog, allowing users to load a game
	 */
	bool loadGameDialog();

protected:
	/**
	 * Actual implementation of pauseEngine by subclasses. See there
	 * for details.
	 */
	virtual void pauseEngineIntern(bool pause);

	//@}


public:

	/**
	 * Request the engine to quit. Sends a EVENT_QUIT event to the Event
	 * Manager.
	 */
	static void quitGame();

	/**
	 * Return whether the ENGINE should quit respectively should return to the
	 * launcher.
	 */
	static bool shouldQuit();

	static MetaEngineDetection &getMetaEngineDetection();
	static MetaEngine &getMetaEngine();

	/**
	 * Pause the engine. This should stop any audio playback
	 * and other stuff. Called right before the system runs a global dialog
	 * (like a global pause, main menu, options or 'confirm exit' dialog).
	 *
	 * Returns a PauseToken. Multiple pause tokens may exist. The engine will
	 * be resumed when all associated pause tokens reach the end of their lives.
	 */
	PauseToken pauseEngine();
private:
	/** Resume the engine. This should resume any audio playback and other stuff.
	*
	* Only PauseToken is allowed to call this member function. Use the PauseToken
	* that you got from pauseEngine to resume the engine.
	*/
	void resumeEngine();

	friend class PauseToken;

public:

	/**
	 * Return whether the engine is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Run the Global Main Menu Dialog
	 */
	void openMainMenuDialog();

	/**
	 * Display a warning to the user that the game is not fully supported.
	 *
	 * @return true if the user chose to start anyway, false otherwise
	 */
	static bool warnUserAboutUnsupportedGame();

	/**
	 * Display an error message to the user that the game is not supported.
	 *
	 * @param extraMsg    This will be appended to the default message.
	 *						
	 */
	static void errorUnsupportedGame(Common::String &extraMsg);

	/**
	 * Get the total play time.
	 *
	 * @return How long the player has been playing in ms.
	 */
	uint32 getTotalPlayTime() const;

	/**
	 * Set the game time counter to the specified time.
	 *
	 * This can be used to set the play time counter after loading a savegame
	 * for example. Another use case is in case the engine wants to exclude
	 * time from the counter the user spent in original engine dialogs.
	 *
	 * @param time Play time to set up in ms.
	 */
	void setTotalPlayTime(uint32 time = 0);

	inline Common::TimerManager *getTimerManager() { return _timer; }
	inline Common::EventManager *getEventManager() { return _eventMan; }
	inline Common::SaveFileManager *getSaveFileManager() { return _saveFileMan; }

public:
	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();


	/**
	 * Checks for whether it's time to do an autosave, and if so, does it.
	 */
	void handleAutoSave();

	/**
	 * Does an autosave immediately if autosaves are turned on
	 */
	void saveAutosaveIfEnabled();

	/**
	 * Indicates whether an autosave can currently be saved.
	 */
	virtual bool canSaveAutosaveCurrently() {
		return canSaveGameStateCurrently();
	}

	/**
	 * Returns the slot that should be used for autosaves
	 * @note	This should match the meta engine getAutosaveSlot() method
	 */
	virtual int getAutosaveSlot() const {
		return 0;
	}

	bool shouldPerformAutoSave(int lastSaveTime) {
		// TODO: Remove deprecated method once all engines are refactored
		// to no longer do autosaves directly themselves
		return false;
	}
};

// Chained games

/**
 * Singleton class which manages chained games. A chained game is one that
 * starts automatically, optionally loading a saved game, instead of returning
 * to the launcher.
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
	void clear();
	void push(const Common::String target, const int slot = -1);
	bool pop(Common::String &target, int &slot);
};

/** Convenience shortcut for accessing the chained games manager. */
#define ChainedGamesMan ChainedGamesManager::instance()

// FIXME: HACK for MidiEmu & error()
extern Engine *g_engine;

#endif
