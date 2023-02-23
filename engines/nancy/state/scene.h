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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_STATE_SCENE_H
#define NANCY_STATE_SCENE_H

#include "common/singleton.h"

#include "engines/nancy/commontypes.h"

#include "engines/nancy/action/actionmanager.h"

#include "engines/nancy/state/state.h"

#include "engines/nancy/ui/fullscreenimage.h"
#include "engines/nancy/ui/viewport.h"
#include "engines/nancy/ui/textbox.h"
#include "engines/nancy/ui/inventorybox.h"

namespace Common {
class SeekableReadStream;
class Serializer;
}

namespace Nancy {

class NancyEngine;
class NancyConsole;
class CheatDialog;
struct SceneChangeDescription;

namespace Action {
class SliderPuzzle;
class PlayPrimaryVideoChan0;
}

namespace UI {
class Button;
}

namespace State {

struct SceneInfo {
	uint16 sceneID = 0;
	uint16 frameID = 0;
	uint16 verticalOffset = 0;
	uint16 paletteID = 0;
};

// The game state that handles all of the gameplay
class Scene : public State, public Common::Singleton<Scene> {
	friend class Nancy::Action::ActionRecord;
	friend class Nancy::Action::ActionManager;
	friend class Nancy::Action::SliderPuzzle;
	friend class Nancy::NancyConsole;
	friend class Nancy::NancyEngine;
	friend class Nancy::CheatDialog;

public:
	enum GameStateChange : byte {
		kHelpMenu = 1 << 0,
		kMainMenu = 1 << 1,
		kSaveLoad = 1 << 2,
		kReloadSave = 1 << 3,
		kSetupMenu = 1 << 4,
		kCredits = 1 << 5,
		kMap = 1 << 6
	};

	struct SceneSummary { // SSUM
		Common::String description;
		Common::String videoFile;
		//
		uint16 videoFormat;
		Common::Array<Common::String> palettes;
		Common::String audioFile;
		SoundDescription sound;
		//
		NancyFlag dontWrap;
		uint16 numberOfVideoFrames;
		uint16 soundPanPerFrame;
		uint16 totalViewAngle;
		uint16 horizontalScrollDelta;
		uint16 verticalScrollDelta;
		uint16 horizontalEdgeSize;
		uint16 verticalEdgeSize;
		Time slowMoveTimeDelta;
		Time fastMoveTimeDelta;
		//

		void read(Common::SeekableReadStream &stream);
	};

	Scene();
	virtual ~Scene();

	// State API
	void process() override;
	void onStateEnter() override;
	void onStateExit() override;

	void changeScene(uint16 id, uint16 frame, uint16 verticalOffset, bool noSound);
	void changeScene(const SceneChangeDescription &sceneDescription);
	void pushScene();
	void popScene();

	void pauseSceneSpecificSounds();
	void unpauseSceneSpecificSounds();

	void addItemToInventory(uint16 id);
	void removeItemFromInventory(uint16 id, bool pickUp = true);
	int16 getHeldItem() const { return _flags.heldItem; }
	void setHeldItem(int16 id);
	NancyFlag hasItem(int16 id) const { return _flags.items[id]; }

	void setEventFlag(int16 label, NancyFlag flag = kTrue);
	void setEventFlag(EventFlagDescription eventFlag);
	bool getEventFlag(int16 label, NancyFlag flag = kTrue) const;
	bool getEventFlag(EventFlagDescription eventFlag) const;

	void setLogicCondition(int16 label, NancyFlag flag = kTrue);
	bool getLogicCondition(int16 label, NancyFlag flag = kTrue) const;
	void clearLogicConditions();

	void setDifficulty(uint difficulty) { _difficulty = difficulty; }
	uint16 getDifficulty() const { return _difficulty; }

	byte getHintsRemaining() const { return _hintsRemaining[_difficulty]; }
	void useHint(int hintID, int hintWeight);

	void requestStateChange(NancyState::NancyState state) { _gameStateRequested = state; }
	void resetStateToInit() { _state = kInit; }

	void resetAndStartTimer() { _timers.timerIsActive = true; _timers.timerTime = 0; }
	void stopTimer() { _timers.timerIsActive = false; _timers.timerTime = 0; }

	Time getMovementTimeDelta(bool fast) const { return fast ? _sceneState.summary.fastMoveTimeDelta : _sceneState.summary.slowMoveTimeDelta; }

	void registerGraphics();

	void synchronize(Common::Serializer &serializer);

	void setShouldClearTextbox(bool shouldClear) { _shouldClearTextbox = shouldClear; }

	UI::FullScreenImage &getFrame() { return _frame; }
	UI::Viewport &getViewport() { return _viewport; }
	UI::Textbox &getTextbox() { return _textbox; }
	UI::InventoryBox &getInventoryBox() { return _inventoryBox; }

	Action::ActionManager &getActionManager() { return _actionManager; }

	SceneInfo &getSceneInfo() { return _sceneState.currentScene; }
	SceneInfo &getNextSceneInfo() { return _sceneState.nextScene; }
	const SceneSummary &getSceneSummary() const { return _sceneState.summary; }

	void setActivePrimaryVideo(Action::PlayPrimaryVideoChan0 *activeVideo);
	Action::PlayPrimaryVideoChan0 *getActivePrimaryVideo();

private:
	void init();
	void load();
	void run();

	void initStaticData();

	void clearSceneData();

	enum State {
		kInit,
		kLoad,
		kStartSound,
		kRun
	};

	struct SceneState {
		SceneSummary summary;
		SceneInfo currentScene;
		SceneInfo nextScene;
		SceneInfo pushedScene;
		bool isScenePushed;

		bool doNotStartSound = false;
	};

	struct Timers {
		enum TimeOfDay { kDay = 0, kNight = 1, kDuskDawn = 2 };
		Time pushedPlayTime;
		Time lastTotalTime;
		Time sceneTime;
		Time timerTime;
		bool timerIsActive = false;
		Time playerTime; // In-game time of day, adds a minute every 5 seconds
		Time playerTimeNextMinute; // Stores the next tick count until we add a minute to playerTime
		TimeOfDay timeOfDay = kDay;
	};

	struct PlayFlags {
		struct LogicCondition {
			NancyFlag flag = NancyFlag::kFalse;
			Time timestamp;
		};

		LogicCondition logicConditions[30];
		Common::Array<NancyFlag> eventFlags;
		uint16 sceneHitCount[2001];
		Common::Array<NancyFlag> items;
		int16 heldItem = -1;
		int16 primaryVideoResponsePicked = -1;
	};

	struct SliderPuzzleState {
		Common::Array<Common::Array<int16>> playerTileOrder;
		bool playerHasTriedPuzzle;
	};

	// UI
	UI::FullScreenImage _frame;
	UI::Viewport _viewport;
	UI::Textbox _textbox;
	UI::InventoryBox _inventoryBox;

	UI::Button *_menuButton;
	UI::Button *_helpButton;

	// Data
	SceneState _sceneState;
	PlayFlags _flags;
	Timers _timers;
	SliderPuzzleState _sliderPuzzleState;
	uint16 _difficulty;
	Common::Array<uint16> _hintsRemaining;
	int16 _lastHint;
	NancyState::NancyState _gameStateRequested;

	Common::Rect _mapHotspot;

	Action::ActionManager _actionManager;
	Action::PlayPrimaryVideoChan0 *_activePrimaryVideo;

	State _state;

	bool _isComingFromMenu = true;
	bool _shouldClearTextbox = true;
};

#define NancySceneState Nancy::State::Scene::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_SCENE_H
