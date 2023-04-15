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
struct SceneChangeDescription;

namespace Action {
class SliderPuzzle;
class ConversationSound;
}

namespace Misc {
class Lightning;
}

namespace UI {
class Button;
class ViewportOrnaments;
class TextboxOrnaments;
class InventoryBoxOrnaments;
class Clock;
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
	friend class Nancy::NancyConsole;
	friend class Nancy::NancyEngine;

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
		byte panningType;
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
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

	void changeScene(uint16 id, uint16 frame, uint16 verticalOffset, byte continueSceneSound, int8 paletteID = -1);
	void changeScene(const SceneChangeDescription &sceneDescription);
	void pushScene();
	void popScene();

	void pauseSceneSpecificSounds();
	void unpauseSceneSpecificSounds();

	void setPlayerTime(Time time, byte relative);
	Time getPlayerTime() const { return _timers.playerTime; }
	byte getPlayerTOD() const;

	void addItemToInventory(uint16 id);
	void removeItemFromInventory(uint16 id, bool pickUp = true);
	int16 getHeldItem() const { return _flags.heldItem; }
	void setHeldItem(int16 id);
	byte hasItem(int16 id) const { return _flags.items[id]; }

	void setEventFlag(int16 label, byte flag = kEvOccurred);
	void setEventFlag(FlagDescription eventFlag);
	bool getEventFlag(int16 label, byte flag = kEvOccurred) const;
	bool getEventFlag(FlagDescription eventFlag) const;

	void setLogicCondition(int16 label, byte flag = kLogUsed);
	bool getLogicCondition(int16 label, byte flag = kLogUsed) const;
	void clearLogicConditions();

	void setDifficulty(uint difficulty) { _difficulty = difficulty; }
	uint16 getDifficulty() const { return _difficulty; }

	byte getHintsRemaining() const { return _hintsRemaining[_difficulty]; }
	void useHint(uint16 characterID, uint16 hintID);

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

	void setActiveConversation(Action::ConversationSound *activeConversation);
	Action::ConversationSound *getActiveConversation();

	// The Vampire Diaries only;
	void beginLightning(int16 distance, uint16 pulseTime, int16 rgbPercent);

	// Game-specific data that needs to be saved/loaded
	SliderPuzzleState *_sliderPuzzleState;
	RippedLetterPuzzleState *_rippedLetterPuzzleState;

private:
	void init();
	void load();
	void run();
	void handleInput();

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

		uint16 continueSceneSound = kLoadSceneSound;
	};

	struct Timers {
		Time pushedPlayTime;
		Time lastTotalTime;
		Time sceneTime;
		Time timerTime;
		bool timerIsActive = false;
		Time playerTime; // In-game time of day, adds a minute every 5 seconds
		Time playerTimeNextMinute; // Stores the next tick count until we add a minute to playerTime
	};

	struct PlayFlags {
		struct LogicCondition {
			byte flag = kLogNotUsed;
			Time timestamp;
		};

		LogicCondition logicConditions[30];
		Common::Array<byte> eventFlags;
		uint16 sceneHitCount[2001];
		Common::Array<byte> items;
		int16 heldItem = -1;
		int16 primaryVideoResponsePicked = -1;
	};

	// UI
	UI::FullScreenImage _frame;
	UI::Viewport _viewport;
	UI::Textbox _textbox;
	UI::InventoryBox _inventoryBox;

	UI::Button *_menuButton;
	UI::Button *_helpButton;
	Time _buttonPressActivationTime;

	UI::ViewportOrnaments *_viewportOrnaments;
	UI::TextboxOrnaments *_textboxOrnaments;
	UI::InventoryBoxOrnaments *_inventoryBoxOrnaments;
	UI::Clock *_clock;

	// General data
	SceneState _sceneState;
	PlayFlags _flags;
	Timers _timers;
	uint16 _difficulty;
	Common::Array<uint16> _hintsRemaining;
	int16 _lastHintCharacter;
	int16 _lastHintID;
	NancyState::NancyState _gameStateRequested;

	Misc::Lightning *_lightning;

	Common::Rect _mapHotspot;

	Action::ActionManager _actionManager;
	Action::ConversationSound *_activeConversation;

	State _state;

	bool _shouldClearTextbox = true;
};

#define NancySceneState Nancy::State::Scene::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_SCENE_H
