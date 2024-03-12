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
#include "common/queue.h"

#include "engines/nancy/commontypes.h"
#include "engines/nancy/puzzledata.h"

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
class ConversationSound;
class PlaySecondaryMovie;
}

namespace Misc {
class Lightning;
class SpecialEffect;
}

namespace UI {
class Button;
class ViewportOrnaments;
class TextboxOrnaments;
class InventoryBoxOrnaments;
class Clock;
}

namespace State {

// The game state that handles all of the gameplay
class Scene : public State, public Common::Singleton<Scene> {
	friend class Nancy::Action::ActionRecord;
	friend class Nancy::Action::ActionManager;
	friend class Nancy::NancyConsole;
	friend class Nancy::NancyEngine;

public:
	struct SceneSummary {
		// SSUM and TSUM
		// Default values set to match those applied when loading from a TSUM chunk
		Common::String description;
		Common::Path videoFile;

		uint16 videoFormat = kLargeVideoFormat;
		Common::Array<Common::Path> palettes;
		SoundDescription sound;

		byte panningType = kPan360;
		uint16 numberOfVideoFrames = 0;
		uint16 degreesPerRotation = 18;
		uint16 totalViewAngle = 0;
		uint16 horizontalScrollDelta = 1;
		uint16 verticalScrollDelta = 10;
		uint16 horizontalEdgeSize = 15;
		uint16 verticalEdgeSize = 15;
		Time slowMoveTimeDelta = 400;
		Time fastMoveTimeDelta = 66;

		// Sound start vectors, used in nancy3 and up
		Math::Vector3d listenerPosition;

		void read(Common::SeekableReadStream &stream);
		void readTerse(Common::SeekableReadStream &stream);
	};

	Scene();
	virtual ~Scene();

	// State API
	void process() override;
	void onStateEnter(const NancyState::NancyState prevState) override;
	bool onStateExit(const NancyState::NancyState nextState) override;

	// Used when winning/losing game
	void setDestroyOnExit() { _destroyOnExit = true; }

	bool isRunningAd() const { return _isRunningAd; }

	void changeScene(const SceneChangeDescription &sceneDescription);
	void pushScene(int16 itemID = -1);
	void popScene(bool inventory = false);

	void setPlayerTime(Time time, byte relative);
	Time getPlayerTime() const { return _timers.playerTime; }
	Time getTimerTime() const { return _timers.timerIsActive ? _timers.timerTime : 0; }
	byte getPlayerTOD() const;

	void addItemToInventory(int16 id);
	void removeItemFromInventory(int16 id, bool pickUp = true);
	int16 getHeldItem() const { return _flags.heldItem; }
	void setHeldItem(int16 id);
	void setNoHeldItem();
	byte hasItem(int16 id) const;
	byte getItemDisabledState(int16 id) const { return _flags.disabledItems[id]; }
	void setItemDisabledState(int16 id, byte state) { _flags.disabledItems[id] = state; }

	void installInventorySoundOverride(byte command, const SoundDescription &sound, const Common::String &caption, uint16 itemID);
	void playItemCantSound(int16 itemID = -1, bool notHoldingSound = false);

	void setEventFlag(int16 label, byte flag);
	void setEventFlag(FlagDescription eventFlag);
	bool getEventFlag(int16 label, byte flag) const;
	bool getEventFlag(FlagDescription eventFlag) const;

	void setLogicCondition(int16 label, byte flag);
	bool getLogicCondition(int16 label, byte flag) const;
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

	UI::FullScreenImage &getFrame() { return _frame; }
	UI::Viewport &getViewport() { return _viewport; }
	UI::Textbox &getTextbox() { return _textbox; }
	UI::InventoryBox &getInventoryBox() { return _inventoryBox; }
	UI::Clock *getClock();

	Action::ActionManager &getActionManager() { return _actionManager; }

	SceneChangeDescription &getSceneInfo() { return _sceneState.currentScene; }
	SceneChangeDescription &getNextSceneInfo() { return _sceneState.nextScene; }
	const SceneSummary &getSceneSummary() const { return _sceneState.summary; }

	void setActiveMovie(Action::PlaySecondaryMovie *activeMovie);
	Action::PlaySecondaryMovie *getActiveMovie();
	void setActiveConversation(Action::ConversationSound *activeConversation);
	Action::ConversationSound *getActiveConversation();

	Graphics::ManagedSurface &getLastScreenshot() { return _lastScreenshot; }

	// The Vampire Diaries only;
	void beginLightning(int16 distance, uint16 pulseTime, int16 rgbPercent);

	// Used from nancy2 onwards
	void specialEffect(byte type, uint16 fadeToBlackTime, uint16 frameTime);
	void specialEffect(byte type, uint16 totalTime, uint16 fadeToBlackTime, Common::Rect rect);

	// Get the persistent data for a given puzzle type
	PuzzleData *getPuzzleData(const uint32 tag);

private:
	void init();
	void load(bool fromSaveFile = false);
	void run();
	void handleInput();

	void initStaticData();

	void clearSceneData();
	void clearPuzzleData();

	enum State {
		kInit,
		kLoad,
		kStartSound,
		kRun
	};

	struct SceneState {
		SceneSummary summary;
		SceneChangeDescription currentScene;
		SceneChangeDescription nextScene;
		SceneChangeDescription pushedScene;
		bool isScenePushed = false;
		SceneChangeDescription pushedInvScene;
		int16 pushedInvItemID = -1;
		bool isInvScenePushed = false;
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
			LogicCondition();
			byte flag;
			Time timestamp;
		};

		LogicCondition logicConditions[30];
		Common::Array<byte> eventFlags;
		Common::HashMap<uint16, uint16> sceneCounts;
		Common::Array<byte> items;
		Common::Array<byte> disabledItems;
		int16 heldItem = -1;
		int16 primaryVideoResponsePicked = -1;
	};

	struct InventorySoundOverride {
		bool isDefault = false; // When true, other fields are ignored
		SoundDescription sound;
		Common::String caption;
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
	RenderObject *_clock;

	Common::Rect _mapHotspot;

	// General data
	SceneState _sceneState;
	PlayFlags _flags;
	Timers _timers;
	uint16 _difficulty;
	Common::Array<uint16> _hintsRemaining;
	int16 _lastHintCharacter;
	int16 _lastHintID;
	NancyState::NancyState _gameStateRequested;
	Common::HashMap<uint16, InventorySoundOverride> _inventorySoundOverrides;

	Misc::Lightning *_lightning;
	Common::Queue<Misc::SpecialEffect> _specialEffects;

	Common::HashMap<uint32, PuzzleData *> _puzzleData;

	Action::ActionManager _actionManager;
	Action::PlaySecondaryMovie *_activeMovie;
	Action::ConversationSound *_activeConversation;

	// Contains a screenshot of the Scene state from the last time it was exited
	Graphics::ManagedSurface _lastScreenshot;

	RenderObject _hotspotDebug;

	bool _destroyOnExit;
	bool _isRunningAd;

	State _state;
};

#define NancySceneState Nancy::State::Scene::instance()

} // End of namespace State
} // End of namespace Nancy

#endif // NANCY_STATE_SCENE_H
