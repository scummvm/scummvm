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
#include "common/ptr.h"
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
class Character;
class Font;
class Object;
typedef Common::SharedPtr<Object> ObjectPtr;
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
	void tick();

public:
	ObjectPtr loadObject(const Common::String & name, const Common::String & prototype = Common::String());
	void runObject(ObjectPtr object);
	void runObject(const Common::String & name, const Common::String &prototype = Common::String());
	void runProcess(ObjectPtr object, uint ip = 0);

	void resetCurrentScreen();
	void loadScreen(const Common::String & name);
	void setCurrentScreen(Screen *screen);

	Region * loadRegion(const Common::String &name);
	Common::String loadText(const Common::String &name);

	int appendToSharedStorage(const Common::String &value);
	const Common::String & getSharedStorage(int id) const;

	bool active() const { return !_mjpgPlayer; }
	void playFilm(const Common::String &video, const Common::String &audio);
	void skipFilm();

	ResourceManager & resourceManager() {
		return _resourceManager;
	}

	Inventory & inventory() {
		return _inventory;
	}

	Screen * getCurrentScreen() {
		return _currentScreen;
	}

	ObjectPtr getCurrentScreenObject(const Common::String &name) {
		return _currentScreen? _currentScreen->find(name): ObjectPtr();
	}

	Common::String & getCurrentScreenName() {
		return _currentScreenName;
	}

	const Graphics::PixelFormat & pixelFormat() const {
		return _pixelFormat;
	}

	Graphics::TransparentSurface *loadPicture(const Common::String &name);
	Graphics::TransparentSurface *convertToTransparent(Graphics::Surface *surface); //destroys surface!
	Graphics::TransparentSurface *loadFromCache(int id) const;
	int saveToCache(Graphics::TransparentSurface *surface) {
		if (!surface)
			return -1;
		int id = _pictureCacheId++;
		_pictureCache[id] = surface;
		return id;
	}

	void loadFont(int id, const Common::String &name, int gw, int gh);
	Font *getFont(int id) const;

	Animation * loadAnimation(const Common::String &name);
	Animation * loadMouseCursor(const Common::String &name);
	Animation * findAnimationByPhaseVar(const Common::String &phaseVar);
	Character * loadCharacter(const Common::String &id, const Common::String &name, const Common::String &object);
	Character * getCharacter(const Common::String &name) const;

	void loadDefaultMouseCursor(const Common::String &name) {
		_defaultMouseCursor = loadMouseCursor(name);
	}

	void changeMouseArea(int id, int enabled);

	void enableUser(bool enabled) {
		_userEnabled = enabled;
	}
	bool userEnabled() const {
		return _userEnabled;
	}

	void initSystemVariables();
	SystemVariable *getSystemVariable(const Common::String &name);

	void runDialog(const Common::String &dialogScript, const Common::String & defs);
	bool tickDialog();
	void tickInventory();

	void playSound(const Common::String &resource, const Common::String &phaseVar) {
		_soundManager.play(resource, phaseVar);
	}

private:
	void parseDialogDefs(const Common::String &defs);

	typedef Common::HashMap<int, Graphics::TransparentSurface *> PictureCacheType;
	typedef Common::HashMap<Common::String, Object *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ObjectsType;
	typedef Common::HashMap<Common::String, Region *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> RegionsType;
	typedef Common::HashMap<Common::String, SystemVariable *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SystemVariablesType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> GlobalsType;
	typedef Common::HashMap<Common::String, Animation *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> AnimationsType;
	typedef Common::HashMap<Common::String, Character *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CharactersType;
	typedef Common::HashMap<int, Font *> FontsType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> DialogDefsType;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	SoundManager				_soundManager;
	Database					_data, _patch; //data and patch databases
	PictureCacheType			_pictureCache;
	int							_pictureCacheId;
	FontsType					_fonts;
	RegionsType					_regions;
	AnimationsType				_animations;
	CharactersType				_characters;
	ProcessListType				_processes;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
	GlobalsType					_globals;
	SystemVariablesType			_systemVars;
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
	DialogDefsType				_dialogDefs;
	Common::String				_dialogScript;
	uint32						_dialogScriptPos;
	Common::String				_dialogProcessName;
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
