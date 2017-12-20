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
#include "common/random.h"
#include "common/rect.h"
#include "engines/advancedDetector.h"
#include "agds/soundManager.h"
#include "agds/database.h"
#include "agds/inventory.h"
#include "agds/processExitCode.h"
#include "agds/resourceManager.h"
#include "agds/screen.h"
#include "graphics/pixelformat.h"
#include "video/flic_decoder.h"

/**
 * This is the namespace of the AGDS engine.
 *
 * Status of this engine: In Progress
 *
 * Games using this engine:
 * - Black Mirror (Windows)
 */

namespace Graphics { struct TransparentSurface; }

namespace AGDS {

class Animation;
class Object;
class Process;
struct Region;
struct MouseRegion;
class MJPGPlayer;
class Screen;
class SystemVariable;

class AGDSEngine : public Engine {
	friend class Process;
	typedef Common::List<Process> ProcessListType;

public:
	AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~AGDSEngine();

	Common::Error run();

	void setGlobal(const Common::String &name, int value) {
		_globals.setVal(name, value);
	}
	bool hasGlobal(const Common::String &name) const {
		return _globals.find(name) != _globals.end();
	}
	int getGlobal(const Common::String &name) const;

private:
	bool initGraphics();
	bool load();
	void runProcess(ProcessListType::iterator &it);
	void runProcess();

public:
	Object * loadObject(const Common::String & name, const Common::String & prototype = Common::String());
	void runObject(Object *object);
	void runObject(const Common::String & name, const Common::String &prototype = Common::String());
	void runProcess(Object *object, uint ip = 0);

	void resetCurrentScreen();
	void loadScreen(const Common::String & name);
	void setCurrentScreen(Screen *screen);

	Region * loadRegion(const Common::String &name);
	Common::String loadText(const Common::String &name);

	int appendToSharedStorage(const Common::String &value);
	const Common::String & getSharedStorage(int id) const;

	void setTimer(int timer) {
		_timer = timer;
	}

	bool active() const { return _timer <= 0 && !_mjpgPlayer; }
	void playFilm(const Common::String &video, const Common::String &audio);

	ResourceManager & resourceManager() {
		return _resourceManager;
	}

	Inventory & inventory() {
		return _inventory;
	}

	Screen * getCurrentScreen() {
		return _currentScreen;
	}

	Common::String & getCurrentScreenName() {
		return _currentScreenName;
	}

	const Graphics::PixelFormat & pixelFormat() const {
		return _pixelFormat;
	}

	Graphics::TransparentSurface *loadPicture(const Common::String &name);
	Graphics::TransparentSurface *convertToTransparent(const Graphics::Surface *surface); //destroys surface!

	Animation * loadAnimation(const Common::String &name);
	void loadDefaultMouseCursor(const Common::String &name) {
		_defaultMouseCursor = loadAnimation(name);
	}
	void changeMouseArea(int id, int enabled);
	void enableUser(bool enabled) {
		_userEnabled = enabled;
	}

	void initSystemVariables();
	SystemVariable *getSystemVariable(const Common::String &name);

private:
	typedef Common::HashMap<Common::String, Object *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ObjectsType;
	typedef Common::HashMap<Common::String, Region *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> RegionsType;
	typedef Common::HashMap<Common::String, SystemVariable *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SystemVariablesType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> GlobalsType;
	typedef Common::HashMap<Common::String, Animation *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> AnimationsType;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	SoundManager				_soundManager;
	Database					_data, _patch; //data and patch databases
	ObjectsType					_objects;
	RegionsType					_regions;
	AnimationsType				_animations;
	ProcessListType				_processes;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
	GlobalsType					_globals;
	SystemVariablesType			_systemVars;
	int							_timer;
	Graphics::PixelFormat		_pixelFormat;
	MJPGPlayer *				_mjpgPlayer;
	Screen *					_currentScreen;
	Common::String				_currentScreenName;
	Screen *					_previousScreen;
	Animation *					_defaultMouseCursor;
	Common::Point				_mouse;
	MouseRegion *				_currentRegion;
	bool						_userEnabled;
	MouseMap					_mouseMap;
	Common::RandomSource		_random;
	Inventory					_inventory;
	bool						_fastMode;
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
