/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
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

#ifndef BASE_ENGINE_H
#define BASE_ENGINE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"

class OSystem;
namespace Audio {
	class Mixer;
}
namespace Common {
	class SaveFileManager;
	class Timer;
}

class Engine {
public:
	OSystem *_system;
	Audio::Mixer *_mixer;
	Common::Timer * _timer;

protected:
	const Common::String _targetName; // target name for saves
	const Common::String _gameDataPath;
	Common::SaveFileManager *_saveFileMan;

private:
	int _autosavePeriod;

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

	void initCommonGFX(bool defaultTo1XScaler);

	/** On some systems, check if the game appears to be run from CD. */
	void checkCD();

	/* Indicate if an autosave should be performed */
	bool shouldPerformAutoSave(int lastSaveTime);

	/** Initialized graphics and shows error message */
	void GUIErrorMessage(const Common::String msg);
};

// Used when no valid game was found in the directory
// Just pops up an error message and returns to launcher
class Engine_Empty : public Engine {
public:
	Engine_Empty(OSystem *syst, const Common::String msg = "No valid games were found in specified directory.");
	virtual ~Engine_Empty() {}

	// Displays error message and do not run go() method
	int init() { GUIErrorMessage(_message); return 1; }

	// Just indicate that we want return to launcher
	int go() { return 1; }

	void errorString(char *buf_input, char *buf_output) {}

private:
	Common::String _message;
};



extern Engine *g_engine;

#endif
