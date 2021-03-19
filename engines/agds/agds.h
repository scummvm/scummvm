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
#include "common/array.h"
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
struct ObjectPatch;
typedef Common::SharedPtr<ObjectPatch> ObjectPatchPtr;
struct Patch;
typedef Common::SharedPtr<Patch> PatchPtr;
class Process;
typedef Common::SharedPtr<Process> ProcessPtr;
struct Region;
typedef Common::SharedPtr<Region> RegionPtr;
struct MouseRegion;
class MJPGPlayer;
class Screen;
class SystemVariable;
class Console;

class AGDSEngine : public Engine {
	friend class Process;
	typedef Common::Array<ProcessPtr> ProcessListType;
	static constexpr uint MaxProcesses = 100;

public:

	struct Color {
		uint8 r = 0;
		uint8 g = 0;
		uint8 b = 0;

		void FromString(const Common::String &rgb);
		Common::String ToString() const;

		uint32 map(const Graphics::PixelFormat &format) const;
	};

	AGDSEngine(OSystem *syst, const ADGameDescription *gameDesc);
	AGDSEngine(const AGDSEngine &) = delete;
	~AGDSEngine();

	Common::Error run() override;

	void setGlobal(const Common::String &name, int value);
	int getGlobal(const Common::String &name) const;
	bool hasGlobal(const Common::String &name) const {
		return _globals.contains(name);
	}

private:
	void addSystemVar(const Common::String &name, SystemVariable *var);
	bool initGraphics(int w, int h);
	bool load();
	void runProcesses();
	void tick();

public:

	bool hasFeature(EngineFeature f) const;
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	bool canLoadGameStateCurrently() { return true; }
	bool canSaveGameStateCurrently() { return _userEnabled; }

	ObjectPtr loadObject(const Common::String & name, const Common::String & prototype = Common::String(), bool allowInitialise = true);
	ObjectPtr runObject(const Common::String & name, const Common::String &prototype = Common::String(), bool allowInitialise = true);
	void runObject(const ObjectPtr &object);
	void runProcess(const ObjectPtr &object, uint ip = 0);
	void stopProcess(const Common::String & name, bool stopNow = false);
	void reactivate(const Common::String &name, bool runNow = false);

	void resetCurrentScreen();
	void loadScreen(const Common::String & name, ScreenLoadingType type, bool savePatch = true);
	void loadNextScreen();
	void saveScreenPatch();

	RegionPtr loadRegion(const Common::String &name);
	Common::String loadText(const Common::String &name);

	int appendToSharedStorage(const Common::String &value);
	const Common::String & getSharedStorage(int id) const;

	bool active() const { return !_mjpgPlayer && !(_soundManager.playing(_syncSoundId) || _tellTextTimer > 0); }
	void playFilm(Process &process, const Common::String &video, const Common::String &audio, const Common::String &subtitles);
	void skipFilm();

	ResourceManager & resourceManager() {
		return _resourceManager;
	}

	SoundManager & soundManager() {
		return _soundManager;
	}

	Inventory & inventory() {
		return _inventory;
	}

	Dialog & dialog() {
		return _dialog;
	}

	const ProcessListType & processes() const {
		return _processes;
	}

	TextLayout & textLayout() {
		return _textLayout;
	}

	Screen * getCurrentScreen() {
		return _currentScreen;
	}
	Console * getConsole();

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
	void loadCharacter(const Common::String &id, const Common::String &name, const Common::String &object);
	Character * getCharacter(const Common::String &name) {
		return _currentCharacterName == name? _currentCharacter: nullptr;
	}
	Character * currentCharacter() const {
		return _currentCharacter;
	}
	Character * jokes() const {
		return _jokes;
	}

	void loadDefaultMouseCursor(const Common::String &name) {
		_defaultMouseCursorName = name;
		_defaultMouseCursor = loadMouseCursor(name);
	}

	void changeMouseArea(int id, int enabled);

	void enableUser(bool enabled) {
		_userEnabled = enabled;
	}
	void enableSystemUser(bool enabled) {
		_systemUserEnabled = enabled;
	}
	bool userEnabled() const {
		return _userEnabled && _systemUserEnabled;
	}

	void newGame();
	void initSystemVariables();
	SystemVariable *getSystemVariable(const Common::String &name);

	void setNextScreenName(const Common::String &nextScreenName, ScreenLoadingType type);
	void returnToPreviousScreen();

	void runDialog(const Common::String &dialogParentProcess, const Common::String &dialogProcess) {
		_dialog.run(dialogParentProcess, dialogProcess);
	}

	void tickInventory();

	int playSound(const Common::String &process, const Common::String &resource, const Common::String &filename, const Common::String &phaseVar, bool playing = true) {
		return _soundManager.play(process, resource, filename, phaseVar, playing);
	}

	void playSoundSync(const Common::String &filename) {
		_syncSoundId = playSound(Common::String(), Common::String(), filename, Common::String());
	}
	void setAmbientSoundId(int id) { _ambientSoundId = id; }

	void tell(Process &process, const Common::String &region, Common::String text, Common::String sound, bool npc);

	bool fastMode() const {
		return _fastMode;
	}

	Common::Point mousePosition() const {
		return _mouse;
	}

	void currentInventoryObject(const ObjectPtr & object);
	void resetCurrentInventoryObject();
	void returnCurrentInventoryObject();

	const ObjectPtr & currentInventoryObject() const {
		return _currentInventoryObject;
	}

	bool showHints() const {
		return _hintMode;
	}

	void reAddInventory();
	PatchPtr getPatch(const Common::String &screenName) const;
	PatchPtr createPatch(const Common::String &screenName);
	ObjectPatchPtr getObjectPatch(const Common::String &screenName) const;
	ObjectPatchPtr createObjectPatch(const Common::String &screenName);

	void shadowIntensity(int intensity) {
		_shadowIntensity = intensity;
	}
	int getRandomNumber(int max);

private:
	void loadPatches(Common::SeekableReadStream *file, Database & db);

	typedef Common::HashMap<int, Graphics::TransparentSurface *> PictureCacheType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> PictureCacheLookup;
	typedef Common::Array<Common::String> SystemVariablesListType;
	typedef Common::HashMap<Common::String, SystemVariable *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> SystemVariablesType;
	typedef Common::HashMap<Common::String, int, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> GlobalsType;
	typedef Common::HashMap<int, Font *> FontsType;
	typedef Common::HashMap<Common::String, PatchPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> PatchesType;
	typedef Common::HashMap<Common::String, ObjectPatchPtr, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> ObjectPatchesType;
	typedef Common::HashMap<Common::String, Common::Array<uint8>> PatchDatabase;

	const ADGameDescription *	_gameDescription;
	ResourceManager				_resourceManager;
	SoundManager				_soundManager;
	Database					_data;
	PictureCacheType			_pictureCache;
	PictureCacheLookup			_pictureCacheLookup;
	int							_pictureCacheId;
	FontsType					_fonts;
	ProcessListType				_processes;
	PatchesType					_patches;
	ObjectPatchesType			_objectPatches;
	int							_sharedStorageIndex;
	Common::String				_sharedStorage[10];
	GlobalsType					_globals;
	SystemVariablesListType		_systemVarList;
	SystemVariablesType			_systemVars;
	Graphics::PixelFormat		_pixelFormat;
	Color						_colorKey;
	Color						_minShadowColor;
	Color						_maxShadowColor;
	int							_shadowIntensity;
	MJPGPlayer *				_mjpgPlayer;
	uint32						_filmStarted;
	Common::String				_filmProcess;
	Screen *					_currentScreen;
	Common::String				_currentScreenName;
	bool						_loadingScreen;
	Character *					_currentCharacter;
	Character *					_jokes;
	Common::String				_currentCharacterName, _currentCharacterFilename, _currentCharacterObject;
	Common::String				_nextScreenName;
	ScreenLoadingType			_nextScreenType;
	Common::String 				_defaultMouseCursorName;
	Animation *					_defaultMouseCursor;
	Common::Point				_mouse;
	MouseRegion *				_currentRegion;
	bool						_userEnabled;
	bool						_systemUserEnabled;
	MouseMap					_mouseMap;
	Common::RandomSource		_random;

	Inventory					_inventory;
	Common::String				_inventoryRegionName;
	RegionPtr					_inventoryRegion;
	ObjectPtr					_currentInventoryObject;

	Dialog						_dialog;

	// Original engine use weird names for the vars, I keep them.
	int							_tellTextTimer;
	TextLayout					_textLayout;

	int							_syncSoundId;
	int							_ambientSoundId;

	bool						_fastMode;
	bool						_hintMode;
};


} // End of namespace AGDS

#endif /* AGDS_AGDS_H */
