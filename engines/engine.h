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

#include "common/events.h"
#include "common/scummsys.h"
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

using GUI::Dialog;

class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::TimerManager * _timer;

protected:
	Common::EventManager *_eventMan;
	Common::SaveFileManager *_saveFileMan;
	
	Dialog *_mainMenuDialog;
	virtual int runDialog(Dialog &dialog);

	const Common::String _targetName; // target name for saves
	const Common::String _gameDataPath;

private:
	/**
	 * The autosave interval, given in second. Used by shouldPerformAutoSave.
	 */
	int _autosavePeriod;

	/**
	 * The pause level, 0 means 'running', a positive value indicates
	 * how often the engine has been paused (and hence how often it has
	 * to be un-paused before it resumes running). This makes it possible
	 * to nest code which pauses the engine.
	 */
	int _pauseLevel;

public:
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
	 * @return a result code
	 */
	virtual int go() = 0;

	/** Specific for each engine: prepare error string. */
	virtual void errorString(const char *buf_input, char *buf_output);

	/**
	 * Return the engine's debugger instance, if any. Used by error() to
	 * invoke the debugger when a severe error is reported.
	 */
	virtual GUI::Debugger *getDebugger() { return 0; }

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
	 * Quit the engine, sends a Quit event to the Event Manager
	 */
	void quitGame();

	/**
	 * Return whether the engine is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

	/**
	 * Return whether or not the ENGINE should quit
	 */
	bool quit() const { return (_eventMan->shouldQuit() || _eventMan->shouldRTL()); }

	/** Run the Global Main Menu Dialog
	 */
	virtual void mainMenuDialog();

	/** Sync the engine's sound settings with the config manager
	 */
	virtual void syncSoundSettings();

	/** Determine whether the engine supports the specified MetaEngine feature
	 */
	virtual bool hasFeature(int f);

public:

	/** Setup the backend's graphics mode. */
	void initCommonGFX(bool defaultTo1XScaler);

	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();

	/** Indicate whether an autosave should be performed. */
	bool shouldPerformAutoSave(int lastSaveTime);

	/** Initialized graphics and shows error message. */
	void GUIErrorMessage(const Common::String msg);

	/**
	 * Actual implementation of pauseEngine by subclasses. See there
	 * for details.
	 */
	virtual void pauseEngineIntern(bool pause);
};

extern Engine *g_engine;

#endif
