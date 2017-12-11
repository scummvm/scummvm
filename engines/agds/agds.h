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

#ifndef AGDS_AGDS_H
#define AGDS_AGDS_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "engines/advancedDetector.h"
#include "agds/database.h"
#include "agds/processExitCode.h"
#include "agds/resourceManager.h"
#include "graphics/pixelformat.h"

/**
 * This is the namespace of the AGDS engine.
 *
 * Status of this engine: In Progress
 *
 * Games using this engine:
 * - Black Mirror (Windows)
 */
namespace AGDS {

class Object;
class Process;
class Region;
class MJPGPlayer;
class Screen;

class AGDSEngine : public Engine {
	friend class Process;

public:
	AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~AGDSEngine();

	Common::Error run();

private:
	bool initGraphics();
	bool load();
	void runProcess();

	Object * loadObject(const Common::String & name);
	Screen * loadScreen(const Common::String & name);

	Region * loadRegion(const Common::String &name);
	Common::String loadFilename(const Common::String &name);

	int appendToSharedStorage(const Common::String &value);
	const Common::String & getSharedStorage(int id) const;

	void setGlobal(const Common::String &name, int value) {
		_globals.setVal(name, value);
	}
	bool hasGlobal(const Common::String &name) const {
		return _globals.find(name) != _globals.end();
	}
	int getGlobal(const Common::String &name) const;

	void setTimer(int timer) {
		_timer = timer;
	}

	bool active() const { return _timer <= 0 && !_mjpgPlayer; }
	void playFilm(const Common::String &video, const Common::String &audio);

	ResourceManager & resourceManager() {
		return _resourceManager;
	}

	Screen *currentScreen() {
		return _currentScreen;
	}

	const Graphics::Surface * loadPicture(const Common::String &name)
	{ return _resourceManager.loadPicture(name, _pixelFormat); }

private:
	typedef Common::HashMap<Common::String, Object *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ObjectsType;
	typedef Common::HashMap<Common::String, Screen *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ScreensType;
	typedef Common::List<Process> ProcessListType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> GlobalsType;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	Database					_data, _patch; //data and patch databases
	ObjectsType					_objects;
	ScreensType					_screens;
	ProcessListType				_processes;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
	GlobalsType					_globals;
	int							_timer;
	Graphics::PixelFormat		_pixelFormat;
	MJPGPlayer *				_mjpgPlayer;
	Screen *					_currentScreen;
	Common::Point				_mouse;
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
