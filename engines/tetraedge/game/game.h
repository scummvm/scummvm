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

#ifndef TETRAEDGE_GAME_GAME_H
#define TETRAEDGE_GAME_GAME_H

#include "common/types.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/random.h"

#include "tetraedge/game/documents_browser.h"
#include "tetraedge/game/inventory.h"
#include "tetraedge/game/inventory_menu.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/notifier.h"
#include "tetraedge/game/game_sound.h"
#include "tetraedge/game/question2.h"
#include "tetraedge/game/dialog2.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeLuaThread;

class Game {
public:
	Game();
	virtual ~Game();

	struct YieldedCallback {
		TeLuaThread *_luaThread;
		Common::String _luaParam;
		Common::String _luaParam2;
		Common::String _luaFnName;
		// Note: original game long, and int fields.. unused?
	};

	void addNoScale2Child(TeLayout *layout);
	virtual void addToBag(const Common::String &objname) = 0;

	virtual bool changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) = 0;

	virtual void draw() = 0;
	virtual void enter() = 0; // will load game if _loadName is set.
	// Note: game uses ILayouts here..
	static TeI3DObject2 *findLayoutByName(TeLayout *parent, const Common::String &name);
	static TeSpriteLayout *findSpriteLayoutByName(TeLayout *parent, const Common::String &name);

	virtual void finishGame() = 0;
	virtual void initLoadedBackupData() = 0;
	bool isDocumentOpened();
	bool isMouse() { return false; }
	bool isMoviePlaying();
	bool launchDialog(const Common::String &param_1, uint param_2, const Common::String &charname,
					  const Common::String &animfile, float animblend);
	virtual void leave(bool flag) = 0;

	// Not in original. Load unlocked artwork from ScummVM config.
	virtual void loadUnlockedArtwork() {};

	//void pauseMovie(); // Unused
	//void pauseSounds() {}; // Unused, does nothing?
	bool playMovie(const Common::String &vidPath, const Common::String &musicPath, float volume = 1.0f);
	void playSound(const Common::String &name, int param_2, float volume);
	void removeNoScale2Child(TeLayout *layout);
	void resumeMovie();
	void resumeSounds() {}; // does nothing?
	void saveBackup(const Common::String &saveName);
	bool setBackground(const Common::String &name);
	void setCurrentObjectSprite(const Common::String &spritePath);
	bool showMarkers(bool val);
	bool startAnimation(const Common::String &animName, int loopcount, bool reversed);
	// void startAnimationPart(const Common::String &param_1, int param_2, int param_3, int param_4, bool param_5) {}; // Unused.
	void stopSound(const Common::String &name);
	Common::Error syncGame(Common::Serializer &s); // Basically replaces saveBackup from original..
	virtual void update() = 0;

	InventoryMenu &inventoryMenu() { return _inventoryMenu; }
	Inventory &inventory() { return _inventory; }
	DocumentsBrowser &documentsBrowser() { return _documentsBrowser; }
	bool entered() const { return _entered; }
	bool running() const { return _running; }
	bool luaShowOwnerError() const { return _luaShowOwnerError; }

	bool _returnToMainMenu;
	bool _firstInventory;

	const Common::String &currentZone() const { return _currentZone; }
	const Common::String &currentScene() const { return _currentScene; }
	TeLuaScript &luaScript() { return _luaScript; }
	TeLuaContext &luaContext() { return _luaContext; }
	InGameScene &scene() { return _scene; }
	Dialog2 &dialog2() { return _dialog2; }
	Question2 &question2() { return _question2; }
	TeLuaGUI &forGui() { return _forGui; }
	TeLuaGUI &inGameGui() { return _inGameGui; }
	bool hasLoadName() const { return !_loadName.empty(); }
	void setLoadName(const Common::String &loadName) { _loadName = loadName; }

	bool onAnswered(const Common::String &val);
	virtual bool onDialogFinished(const Common::String &val) = 0;
	virtual bool onFinishedLoadingBackup(const Common::String &val) { return false; }
	bool onInventoryButtonValidated();
	bool onLockVideoButtonValidated();
	bool onMouseMove();
	bool onSkipVideoButtonValidated();
	virtual bool onVideoFinished() = 0;

protected:
	bool _luaShowOwnerError;
	bool _running;
	bool _entered;

	//TeLuaGUI _gui1; // Never used.
	TeLuaGUI _setAnimGui;
	TeLuaGUI _forGui;
	TeLuaGUI _inGameGui;

	Inventory _inventory;
	InventoryMenu _inventoryMenu;

	InGameScene _scene;

	Common::String _loadName;

	Common::String _currentScene;
	Common::String _currentZone;
	Common::String _prevSceneName;

	Common::Array<GameSound *> _gameSounds;

	static const int NUM_OBJECTS_TAKEN_IDS = 5;
	static const char *OBJECTS_TAKEN_IDS[NUM_OBJECTS_TAKEN_IDS];
	bool _objectsTakenBits[NUM_OBJECTS_TAKEN_IDS];
	int _objectsTakenVal;

	TeTimer _playedTimer;
	TeTimer _walkTimer;
	TeLuaScript _luaScript;
	TeLuaContext _luaContext;
	TeLuaScript _gameEnterScript;
	TeMusic _music;
	Notifier _notifier;
	DocumentsBrowser _documentsBrowser;

	Question2 _question2;
	Dialog2 _dialog2;

	int _dialogsTold;

	TeLayout *_noScaleLayout2;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_GAME_H
