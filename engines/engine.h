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
 * $URL$
 * $Id$
 */

#ifndef ENGINES_ENGINE_H
#define ENGINES_ENGINE_H

#include "common/scummsys.h"
#include "common/fs.h"
#include "common/str.h"

class OSystem;

namespace Audio {
	class Mixer;
}
namespace Common {
	class EventManager;
	class SaveFileManager;
	class TimerManager;
}
namespace GUI {
	class Debugger;
	class Dialog;
}

/**
 * Setup the backend's graphics mode.
 */
void initCommonGFX(bool defaultTo1XScaler);

/**
 * Initialized graphics and shows error message.
 */
void GUIErrorMessage(const Common::String msg);


class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::TimerManager * _timer;

protected:
	Common::EventManager *_eventMan;
	Common::SaveFileManager *_saveFileMan;
	
	GUI::Dialog *_mainMenuDialog;
	virtual int runDialog(GUI::Dialog &dialog);

	const Common::String _targetName; // target name for saves
	
	const Common::FSNode _gameDataDir;	// FIXME: Get rid of this

private:
	/**
	 * The pause level, 0 means 'running', a positive value indicates
	 * how often the engine has been paused (and hence how often it has
	 * to be un-paused before it resumes running). This makes it possible
	 * to nest code which pauses the engine.
	 */
	int _pauseLevel;

public:

	/** @name Overloadable methods
	 *
	 *  All Engine subclasses should consider overloading some or all of the following methods.
	 */
	//@{

	Engine(OSystem *syst);
	virtual ~Engine();

	/**
	 * Init the engine.
	 * @return 0 for success, else an error code.
	 */
	virtual int init() = 0;

	/**
	 * Start the main engine loop.
	 * The return value is not yet used, but could indicate whether the user
	 * wants to return to the launch or to fully quit ScummVM.
	 * @return 0 for success, else an error code.
	 */
	virtual int go() = 0;

	/**
	 * Prepare an error string, which is printed by the error() function.
	 */
	virtual void errorString(const char *buf_input, char *buf_output);

	/**
	 * Return the engine's debugger instance, if any. Used by error() to
	 * invoke the debugger when a severe error is reported.
	 */
	virtual GUI::Debugger *getDebugger() { return 0; }

	/**
	 * Notify the engine that the sound settings in the config manager may have
	 * changed and that it hence should adjust any internal volume etc. values
	 * accordingly.
	 * @todo find a better name for this
	 */
	virtual void syncSoundSettings();

	/** 
	 * Load a game state.
	 * @return returns 0 on success, anything else indicates failure
	 * @todo define proper error values
	 */
	virtual int loadGameState(int slot);

	/**
	 * Indicates whether a game state can be loaded.
	 */
	virtual bool canLoadGameStateCurrently();

	/**
	 * Save a game state.
	 * @return returns 0 on success, anything else indicates failure
	 *
	 * @todo define proper error values
	 * @todo actually we need to pass the user entered name to the engine
	 */
	virtual int saveGameState(int slot);

	/**
	 * Indicates whether a game state can be saved.
	 */
	virtual bool canSaveGameStateCurrently();

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
	void quitGame();

	/**
	 * Return whether the ENGINE should quit respectively should return to the
	 * launcher.
	 */
	bool shouldQuit() const;

	/**
	 * Pause or resume the engine. This should stop/resume any audio playback
	 * and other stuff. Called right before the system runs a global dialog
	 * (like a global pause, main menu, options or 'confirm exit' dialog).
	 *
	 * This is a convenience tracker which automatically keeps track on how
	 * often the engine has been paused, ensuring that after pausing an engine
	 * e.g. twice, it has to be unpaused twice before actuallying resuming.
	 *
	 * @param pause		true to pause the engine, false to resume it
	 */
	void pauseEngine(bool pause);

	/**
	 * Return whether the engine is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Run the Global Main Menu Dialog
	 */
	void openMainMenuDialog();


	/**
	 * A feature in this context means an ability of the engine which can be
	 * either available or not.
	 */
	enum EngineFeature {
		/**
		 * 'Return to launcher' feature is supported, i.e., EVENT_RTL is handled.
		 */
		kSupportsRTL = 0,
		
		/**
		 * Listing all Save States for a given target is supported, i.e.,
		 * the listSaves() method is implemented.
		 * Used for --list-saves support, as well as the GMM load dialog.
		 */
		kSupportsListSaves = 1,

		/** 
		 * Loading from the in-game common ScummVM options dialog is supported
		 */
		kSupportsLoadingDuringRuntime = 8,

		/** 
		 * Saving from the in-game common ScummVM options dialog is supported
		 */
		kSupportsSavingDuringRuntime = 9
	};

	/**
	 * Determine whether the engine supports the specified feature.
	 *
	 * @todo  Let this return false by default, or even turn it into a pure virtual method.
	 */
	bool hasFeature(EngineFeature f);

public:

	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();

protected:

	/**
	 * Indicate whether an autosave should be performed.
	 */
	bool shouldPerformAutoSave(int lastSaveTime);

};

extern Engine *g_engine;

#endif
