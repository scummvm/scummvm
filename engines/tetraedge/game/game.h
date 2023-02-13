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
#include "tetraedge/game/cellphone.h"
#include "tetraedge/game/game_sound.h"
#include "tetraedge/game/objectif.h"
#include "tetraedge/game/question2.h"
#include "tetraedge/game/dialog2.h"
#include "tetraedge/te/te_lua_gui.h"
#include "tetraedge/te/te_music.h"
#include "tetraedge/te/te_checkbox_layout.h"
#include "tetraedge/te/te_vector2s32.h"

namespace Tetraedge {

class TeLuaThread;

class Game {
public:
	Game();
	~Game();

	struct HitObject {
		bool onChangeWarp();
		bool onDown();
		bool onUp();
		bool onValidated();
		//byte OnVisible(); empty never used?

		Common::String _name;
		Game *_game;
		TeButtonLayout *_button;
	};

	class RandomSound {
	public:
		Common::Path _path;
		Common::String _name;
		TeMusic _music;
		float _f1;
		float _volume;
		bool onSoundFinished();
	};

	struct YieldedCallback {
		TeLuaThread *_luaThread;
		Common::String _luaParam;
		Common::String _luaParam2;
		Common::String _luaFnName;
		// Note: original game long, and int fields.. unused?
	};

	//enum EGameScoreID {}; // Not needed?

	bool addAnimToSet(const Common::String &path);
	void addArtworkUnlocked(const Common::String &name, bool notify);
	void addNoScale2Child(TeLayout *layout);
	void addNoScale2Children();
	void addNoScaleChildren();
	void addRandomSound(const Common::String &s1, const Common::String &s2, float f1, float f2);
	void addToBag(const Common::String &objname);
	void addToHand(const Common::String &objname);
	void addToScore(int score);
	void attachButtonsLayoutGoto() {}; // does nothing?
	void createButtonsLayoutGoto() {}; // does nothing?
	void deleteButtonsLayoutGoto() {}; // does nothing?

	bool changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag);
	bool changeWarp2(const Common::String &zone, const Common::String &scene, bool fadeFlag);

	void deleteNoScale();
	void draw();
	void enter(); // will load game if _loadName is set.
	// Note: game uses ILayouts here..
	static TeI3DObject2 *findLayoutByName(TeLayout *parent, const Common::String &name);
	static TeSpriteLayout *findSpriteLayoutByName(TeLayout *parent, const Common::String &name);

	void finishFreemium();
	void finishGame();
	void initLoadedBackupData();
	void initNoScale();
	void initScene(bool param_1, const Common::String &scenePath);
	bool initWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag);
	bool isDocumentOpened();
	bool isMouse() { return false; }
	bool isMoviePlaying();
	bool launchDialog(const Common::String &param_1, uint param_2, const Common::String &param_3,
					  const Common::String &param_4, float param_5);
	void leave(bool flag);
	void loadBackup(const Common::String &path);
	bool loadCharacter(const Common::String &name);
	bool loadPlayerCharacter(const Common::String &name);
	bool loadScene(const Common::String &name);

	// Not in original. Load unlocked artwork from ScummVM config.
	void loadUnlockedArtwork();

	bool onAnswered(const Common::String &val);
	bool onCallNumber(Common::String val);
	bool onCharacterAnimationFinished(const Common::String &val);
	bool onCharacterAnimationPlayerFinished(const Common::String &val);
	bool onDialogFinished(const Common::String &val);
	bool onDisplacementFinished();
	bool onDisplacementPlayerFinished();
	bool onFinishedCheckBackup(bool result);
	bool onFinishedLoadingBackup(const Common::String &val);
	bool onFinishedSavingBackup(int something);
	bool onInventoryButtonValidated();
	bool onLockVideoButtonValidated();
	bool onMarkersVisible(TeCheckboxLayout::State state);
	bool onMouseClick(const Common::Point &pt);
	bool onMouseMove();
	bool onSkipVideoButtonValidated();
	bool onVideoFinished();

	void pauseMovie();
	void pauseSounds() {}; // does nothing?
	bool playMovie(const Common::String &vidPath, const Common::String &musicPath, float volume = 1.0f);
	void playRandomSound(const Common::String &name);
	void playSound(const Common::String &name, int param_2, float volume);
	void removeNoScale2Child(TeLayout *layout);
	void removeNoScale2Children();
	void removeNoScaleChildren();
	void resetPreviousMousePos();
	void resumeMovie();
	void resumeSounds() {}; // does nothing?
	void saveBackup(const Common::String &saveName);
	bool setBackground(const Common::String &name);
	void setCurrentObjectSprite(const Common::String &spritePath);
	bool showMarkers(bool val);
	bool startAnimation(const Common::String &animName, int loopcount, bool reversed);
	void startAnimationPart(const Common::String &param_1, int param_2, int param_3, int param_4, bool param_5) {};
	void stopSound(const Common::String &name);
	Common::Error syncGame(Common::Serializer &s); // Basically replaces saveBackup from original..
	bool unloadCharacter(const Common::String &character);
	bool unloadCharacters();
	bool unloadPlayerCharacter(const Common::String &character);
	void update();

	InventoryMenu &inventoryMenu() { return _inventoryMenu; }
	Inventory &inventory() { return _inventory; }
	DocumentsBrowser &documentsBrowser() { return _documentsBrowser; }
	bool entered() const { return _entered; }
	bool running() const { return _running; }
	bool luaShowOwnerError() const { return _luaShowOwnerError; }

	bool _returnToMainMenu;
	bool _firstInventory;
	bool _movePlayerCharacterDisabled;
	bool _sceneCharacterVisibleFromLoad;
	bool _isCharacterWalking;
	bool _isCharacterIdle;

	const Common::String &currentZone() const { return _currentZone; }
	const Common::String &currentScene() const { return _currentScene; }
	const Common::Path &sceneZonePath() const { return _sceneZonePath; }
	TeLuaScript &luaScript() { return _luaScript; }
	TeLuaContext &luaContext() { return _luaContext; }
	InGameScene &scene() { return _scene; }
	Dialog2 &dialog2() { return _dialog2; }
	Question2 &question2() { return _question2; }
	TeLuaGUI &forGui() { return _forGui; }
	TeLuaGUI &inGameGui() { return _inGameGui; }
	Objectif &objectif() { return _objectif; }
	Common::Array<YieldedCallback> &yieldedCallbacks() { return _yieldedCallbacks; }
	void setSaveRequested() { _saveRequested = true; }
	bool markersVisible() const { return _markersVisible; }
	const TeVector3f32 &posPlayer() const { return _posPlayer; }
	void setPosPlayer(const TeVector3f32 &pos) { _posPlayer = pos; }
	TeTimer &walkTimer() { return _walkTimer; }
	void setExitZone(const Common::String &zone) { _exitZone = zone; }
	Common::RandomSource &randomSource() { return _randomSource; }
	void setLoadName(const Common::String &loadName) { _loadName = loadName; }
	bool hasLoadName() const { return !_loadName.empty(); }
	bool isArtworkUnlocked(const Common::String &name) const;
	static Common::String artworkConfName(const Common::String &name);

	void setRunModeEnabled(bool val) { _runModeEnabled = val; }
	bool runModeEnabled() const { return _runModeEnabled; }

private:
	bool _luaShowOwnerError;
	bool _running;
	bool _entered;
	bool _enteredFlag2;

	TeLuaGUI _gui1; // TODO: Is this ever used?
	TeLuaGUI _setAnimGui;
	TeLuaGUI _forGui;
	TeLuaGUI _inGameGui;

	Inventory _inventory;
	InventoryMenu _inventoryMenu;
	int _score;

	int _frameCounter;

	InGameScene _scene;

	static char **_objectsTakenIDs;

	TeVector2s32 _previousMousePos;
	TeVector2s32 _lastCharMoveMousePos;

	Common::String _warpZone;
	Common::String _warpScene;
	bool _warpFadeFlag;
	bool _warped;

	Common::String _currentZone;
	Common::String _currentScene;
	Common::String _exitZone;
	Common::String _prevSceneName;
	Common::String _someSceneName;
	Common::Path _sceneZonePath;

	Common::String _loadName;

	Common::Array<GameSound *> _gameSounds;
	Common::Array<HitObject *> _gameHitObjects;
	// These are static in original, but cleaner to keep here.
	Common::Array<YieldedCallback> _yieldedCallbacks;

	Common::HashMap<Common::String, bool> _unlockedArtwork;
	Common::HashMap<Common::String, Common::Array<RandomSound *>> _randomSounds;

	int _gameLoadState;

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
	Objectif _objectif;

	static const int NUM_OBJECTS_TAKEN_IDS = 5;
	static const char *OBJECTS_TAKEN_IDS[NUM_OBJECTS_TAKEN_IDS];
	bool _objectsTakenBits[NUM_OBJECTS_TAKEN_IDS];
	int _objectsTakenVal;
	int _dialogsTold;

	bool _markersVisible;
	bool _saveRequested;
	bool _randomSoundFinished;

	Common::RandomSource _randomSource;
	RandomSound *_randomSound;
	TeTimer _randomSoundTimer;

	TeLayout *_noScaleLayout;
	TeLayout *_noScaleLayout2;

	TeVector3f32 _posPlayer;

	Common::Point _lastUpdateMousePos;

	// Syberia 2 specific data
	bool _runModeEnabled;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_GAME_H
