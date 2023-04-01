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

#ifndef TETRAEDGE_GAME_SYBERIA_GAME_H
#define TETRAEDGE_GAME_SYBERIA_GAME_H

#include "common/types.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/random.h"

#include "tetraedge/game/documents_browser.h"
#include "tetraedge/game/inventory.h"
#include "tetraedge/game/inventory_menu.h"
#include "tetraedge/game/in_game_scene.h"
#include "tetraedge/game/game.h"
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

class SyberiaGame : public Tetraedge::Game {
public:
	SyberiaGame();
	~SyberiaGame();

	struct HitObject {
		bool onChangeWarp();
		bool onDown();
		bool onUp();
		bool onValidated();
		//byte OnVisible(); empty never used?

		Common::String _name;
		SyberiaGame *_game;
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

	void addArtworkUnlocked(const Common::String &name, bool notify);
	void addRandomSound(const Common::String &s1, const Common::String &s2, float f1, float f2);
	void addToBag(const Common::String &objname) override;
	void addToHand(const Common::String &objname);
	void addToScore(int score);

	bool changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) override;

	void draw() override;
	void enter() override; // will load game if _loadName is set.

	void finishFreemium();
	void finishGame() override;
	void initLoadedBackupData() override;
	void leave(bool flag) override;
	void loadBackup(const Common::String &path);
	bool loadCharacter(const Common::String &name);
	bool loadPlayerCharacter(const Common::String &name);
	bool loadScene(const Common::String &name);

	// Not in original. Load unlocked artwork from ScummVM config.
	void loadUnlockedArtwork() override;

	void playRandomSound(const Common::String &name);
	void resetPreviousMousePos();
	bool unloadCharacter(const Common::String &character);
	bool unloadCharacters();
	bool unloadPlayerCharacter(const Common::String &character);
	void update() override;

	bool _movePlayerCharacterDisabled;
	bool _sceneCharacterVisibleFromLoad;
	bool _isCharacterWalking;
	bool _isCharacterIdle;

	const Common::Path &sceneZonePath() const { return _sceneZonePath; }
	Objectif &objectif() { return _objectif; }
	Common::Array<YieldedCallback> &yieldedCallbacks() { return _yieldedCallbacks; }
	void setSaveRequested() { _saveRequested = true; }
	bool markersVisible() const { return _markersVisible; }
	const TeVector3f32 &posPlayer() const { return _posPlayer; }
	void setPosPlayer(const TeVector3f32 &pos) { _posPlayer = pos; }
	TeTimer &walkTimer() { return _walkTimer; }
	void setExitZone(const Common::String &zone) { _exitZone = zone; }
	bool isArtworkUnlocked(const Common::String &name) const;
	static Common::String artworkConfName(const Common::String &name);

	void setRunModeEnabled(bool val) { _runModeEnabled = val; }
	bool runModeEnabled() const { return _runModeEnabled; }

private:
	bool addAnimToSet(const Common::String &path);
	void addNoScale2Children();
	void addNoScaleChildren();

	void attachButtonsLayoutGoto() {}; // does nothing?
	void createButtonsLayoutGoto() {}; // does nothing?
	void deleteButtonsLayoutGoto() {}; // does nothing?

	bool changeWarp2(const Common::String &zone, const Common::String &scene, bool fadeFlag);

	void deleteNoScale();

	void initNoScale();
	void initScene(bool param_1, const Common::String &scenePath);
	bool initWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag);

	bool onCallNumber(Common::String val);
	bool onCharacterAnimationFinished(const Common::String &val);
	bool onCharacterAnimationPlayerFinished(const Common::String &val);
	bool onDialogFinished(const Common::String &val) override;
	bool onDisplacementFinished();
	bool onDisplacementPlayerFinished();
	bool onFinishedCheckBackup(bool result);
	bool onFinishedLoadingBackup(const Common::String &val) override;
	bool onFinishedSavingBackup(int something);
	bool onMarkersVisible(TeCheckboxLayout::State state);
	bool onMouseClick(const Common::Point &pt);
	bool onVideoFinished() override;

	void removeNoScale2Children();
	void removeNoScaleChildren();

	bool _enteredFlag2;

	int _score;

	int _frameCounter;

	TeVector2s32 _previousMousePos;
	TeVector2s32 _lastCharMoveMousePos;

	Common::String _warpZone;
	Common::String _warpScene;
	bool _warpFadeFlag;
	bool _warped;

	Common::String _exitZone;
	Common::String _someSceneName;
	Common::Path _sceneZonePath;

	Common::Array<HitObject *> _gameHitObjects;
	// These are static in original, but cleaner to keep here.
	Common::Array<YieldedCallback> _yieldedCallbacks;

	Common::HashMap<Common::String, Common::Array<RandomSound *>> _randomSounds;

	Common::HashMap<Common::String, bool> _unlockedArtwork;

	int _gameLoadState;

	Objectif _objectif;

	bool _markersVisible;
	bool _saveRequested;
	bool _randomSoundFinished;

	RandomSound *_randomSound;
	TeTimer _randomSoundTimer;

	TeLayout *_noScaleLayout;

	TeVector3f32 _posPlayer;

	Common::Point _lastUpdateMousePos;

	// Syberia 2 specific data
	bool _runModeEnabled;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_GAME_SYBERIA_GAME_H
