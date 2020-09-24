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
#include "agds/dialog.h"
#include "agds/inventory.h"
#include "agds/mouseMap.h"
#include "agds/processExitCode.h"
#include "agds/resourceManager.h"
#include "agds/screen.h"
#include "agds/textLayout.h"
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
struct Patch;
typedef Common::SharedPtr<Patch> PatchPtr;
class Process;
struct Region;
typedef Common::SharedPtr<Region> RegionPtr;
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

	Common::Error run() override;

	void setGlobal(const Common::String &name, int value) {
		_globals.setVal(name, value);
	}
	bool hasGlobal(const Common::String &name) const {
		return _globals.find(name) != _globals.end();
	}
	int getGlobal(const Common::String &name) const;

private:
	void addSystemVar(const Common::String &name, SystemVariable *var);
	bool initGraphics(int w, int h);
	bool load();
	void runProcesses();
	void tick();

public:

	bool hasFeature(EngineFeature f) const;
	Common::Error loadGameStream(Common::SeekableReadStream *file);
	Common::Error saveGameStream(Common::WriteStream *file, bool isAutosave);
	bool canLoadGameStateCurrently() { return true; }
	bool canSaveGameStateCurrently() { return _userEnabled; }

	ObjectPtr loadObject(const Common::String & name, const Common::String & prototype = Common::String());
	void runObject(const ObjectPtr &object);
	void runObject(const Common::String & name, const Common::String &prototype = Common::String());
	void runProcess(const ObjectPtr &object, uint ip = 0);

	void resetCurrentScreen();
	void loadScreen(const Common::String & name);

	RegionPtr loadRegion(const Common::String &name);
	Common::String loadText(const Common::String &name);

	int appendToSharedStorage(const Common::String &value);
	const Common::String & getSharedStorage(int id) const;

	bool active() const { return !_mjpgPlayer && !(_soundManager.playing(_syncSoundId) || _tellTextTimer > 0); }
	void playFilm(Process &process, const Common::String &video, const Common::String &audio);
	void skipFilm();

	ResourceManager & resourceManager() {
		return _resourceManager;
	}

	Inventory & inventory() {
		return _inventory;
	}

	Dialog & dialog() {
		return _dialog;
	}

	TextLayout & textLayout() {
		return _textLayout;
	}

	Screen * getCurrentScreen() {
		return _currentScreen;
	}

	Common::String & getCurrentScreenName() {
		return _currentScreenName;
	}
	ObjectPtr getCurrentScreenObject(const Common::String &name);

	const Graphics::PixelFormat & pixelFormat() const {
		return _pixelFormat;
	}

	Graphics::TransparentSurface *loadPicture(const Common::String &name);
	Graphics::Surface *createSurface(int w, int h);
	Graphics::TransparentSurface *convertToTransparent(Graphics::Surface *surface); //destroys surface!

	int loadFromCache(const Common::String & name) const;
	Graphics::TransparentSurface *loadFromCache(int id) const;
	int saveToCache(const Common::String &name, Graphics::TransparentSurface *surface);

	void loadFont(int id, const Common::String &name, int gw, int gh);
	Font *getFont(int id) const;

	Animation * loadAnimation(const Common::String &name);
	Animation * loadMouseCursor(const Common::String &name);
	Animation * findAnimationByPhaseVar(const Common::String &phaseVar);
	Character * loadCharacter(const Common::String &id, const Common::String &name, const Common::String &object);
	Character * getCharacter(const Common::String &name) const;

	void loadDefaultMouseCursor(const Common::String &name) {
		_defaultMouseCursorName = name;
		_defaultMouseCursor = loadMouseCursor(name);
	}

	void changeMouseArea(int id, int enabled);

	void enableUser(bool enabled) {
		_userEnabled = enabled;
	}
	bool userEnabled() const {
		return _userEnabled;
	}

	void newGame();
	void initSystemVariables();
	SystemVariable *getSystemVariable(const Common::String &name);

	void setNextScreenName(const Common::String &nextScreenName, bool savePrev) {
		if (_currentScreen && savePrev) {
			_previousScreenName = _currentScreenName;
		}
		_nextScreenName = nextScreenName;
	}

	void returnToPreviousScreen() {
		if (!_previousScreenName.empty()) {
			_nextScreenName = _previousScreenName;
			_previousScreenName.clear();
		}
	}

	void runDialog(const Common::String &dialogProcess) {
		_dialog.run(dialogProcess);
	}

	void tickInventory();

	int playSound(const Common::String &process, const Common::String &resource, const Common::String &phaseVar) {
		return _soundManager.play(process, resource, phaseVar);
	}

	void playSoundSync(const Common::String &resource, const Common::String &phaseVar) {
		_syncSoundId = playSound(Common::String(), resource, phaseVar);
	}

	void tell(const Common::String &region, const Common::String &text, const Common::String &sound, const Common::String &soundPhaseVar, bool npc);

	bool fastMode() const {
		return _fastMode;
	}

	Common::Point mousePosition() const {
		return _mouse;
	}

	void reactivate(const Common::String &name);

private:
	void loadPatches(Common::SeekableReadStream *file, Database & db);

	typedef Common::HashMap<int, Graphics::TransparentSurface *> PictureCacheType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> PictureCacheLookup;
	typedef Common::Array<Common::String> SystemVariablesListType;
	typedef Common::HashMap<Common::String, SystemVariable *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SystemVariablesType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> GlobalsType;
	typedef Common::HashMap<Common::String, Animation *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> AnimationsType;
	typedef Common::HashMap<Common::String, Character *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> CharactersType;
	typedef Common::HashMap<int, Font *> FontsType;
	typedef Common::HashMap<Common::String, PatchPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> PatchesType;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	SoundManager				_soundManager;
	Database					_data;
	PictureCacheType			_pictureCache;
	PictureCacheLookup			_pictureCacheLookup;
	int							_pictureCacheId;
	FontsType					_fonts;
	AnimationsType				_animations;
	CharactersType				_characters;
	ProcessListType				_processes;
	PatchesType					_patches;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
	GlobalsType					_globals;
	SystemVariablesListType		_systemVarList;
	SystemVariablesType			_systemVars;
	Graphics::PixelFormat		_pixelFormat;
	MJPGPlayer *				_mjpgPlayer;
	Common::String				_filmProcess;
	Screen *					_currentScreen;
	Common::String				_currentScreenName;
	Common::String				_nextScreenName;
	Common::String				_previousScreenName;
	Common::String 				_defaultMouseCursorName;
	Animation *					_defaultMouseCursor;
	Common::Point				_mouse;
	MouseRegion *				_currentRegion;
	bool						_userEnabled;
	MouseMap					_mouseMap;
	Common::RandomSource		_random;
	Inventory					_inventory;
	Common::String				_inventoryRegionName;
	RegionPtr					_inventoryRegion;
	Dialog						_dialog;

	// Original engine use weird names for the vars, I keep them.
	int							_tellTextTimer;
	TextLayout					_textLayout;

	bool						_resetTextLayoutIfSyncSoundStops;
	int							_syncSoundId;

	bool						_fastMode;
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
