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

#ifndef MACS2_SCRIPTEXECUTOR_H
#define MACS2_SCRIPTEXECUTOR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str-array.h"

namespace Common {
class MemoryReadStream;
}

namespace Macs2 {
class Macs2Engine;
class GameObject;

namespace Script {

struct ScriptVariable {
	uint16 a;
	uint16 b;
};
// Order of cursors when loading from the file is
//  { Talk = 0, Look = 1, Touch = 2, Walk = 3};
// Cursor mode values from setCursorMode (1008:3ea5) and handleInput.
// Each mode indexes into the cursor image array at _PTR_LOOP_1020_075a
// (16 bytes per entry: data ptr, size, width, height, hotspot x/y).
enum class MouseMode {
	Talk = 0x13,
	Look = 0x14,
	Use = 0x15,
	Walk = 0x16,
	UseInventory = 0x17,
	PanelUse = 0x18,    // Used in map mode
	PanelCursor = 0x19, // Used when action bar is open
	Disabled = 0x1A     // Hidden/disabled cursor
};

enum class ExecutorState {
	// We are not executing anything at the moment
	Idle,
	// We are in the middle of executing
	Executing,
	// Executing but paused until a callback happens
	WaitingForCallback
};

enum class ExecutionResult {
	// We have finished executing the script
	ScriptFinished,

	// We are now waiting for a callback
	WaitingForCallback
};

enum class ScriptExecutionState {
	// We are executing the scene script
	ExecutingSceneScript,

	// We are executing the other scripts
	ExecutingOtherScripts
};

/**
 * Here is how the state of the script executor works
 * We can have a scheduled run (which will execute the next tick) or we can run it right
 * away
 * A run can have the two global variables set on it, and therefore might require being
 * run a second time
 * A run can remain within one function call or it can have time-dependent phases during
 * which execution is paused and we wait (e.g. a timed wait event)
 * After the scene script, we iterate executingObjectId from 1 to 0x200,
 * executing the script of each object that has runtime data allocated
 * (confirmed from runScriptExecutor at 1008:e3e7).
 *
 */
class ScriptExecutor {
private:
	enum class OpcodeControlFlow : uint8 {
		Fallthrough,
		Continue,
		ScriptFinished,
		WaitingForCallback
	};

	void scriptSetVar();
	void scriptSetVarOr();
	void scriptIfTrue();
	void scriptIfFalse();
	bool scriptCompare();
	void scriptIfInteraction();
	void scriptEndIf();
	void scriptElse();
	void scriptNop09();
	void scriptPrintStringLeft();
	bool scriptMoveObject();
	ExecutionResult scriptChangeScene();
	ExecutionResult scriptShowDialogue();
	bool scriptWalkToPosition();
	ExecutionResult scriptWaitForWalk();
	void scriptSkipWord();
	void scriptClearDialogueChoices();
	void scriptAddDialogueChoice();
	ExecutionResult scriptShowDialogueChoice();
	ExecutionResult scriptDismissPanel();
	bool scriptWalkToAndPickup();
	bool scriptSetPickupFrames();
	bool scriptSetupObject();
	void scriptSetSkippable();
	void scriptClearSkippable();
	bool scriptPlayAnimation();
	void scriptTestPathfinding();
	bool scriptSetYOffset();
	bool scriptSetMotion();
	bool scriptSetOrientation();
	bool scriptMoveToPosition();
	void scriptAddValues();
	void scriptSubValues();
	void scriptLoadSpecialAnim();
	bool scriptSetDirection();
	void scriptStopAnimation();
	bool scriptOpenInventory();
	void scriptLoadObjectAnim();
	bool scriptCheckObjectData();
	bool scriptCheckInventory();
	bool scriptSetSnapToTarget();
	bool scriptTestObjectAnimFrame();
	void scriptPrintStringRight();
	void scriptSetVolume();
	bool scriptSetObjectClickable();
	bool scriptSetObjectVisible();
	bool scriptSetHotspotOverride();
	bool scriptSetObjectBounds();
	void scriptDismissAllPanels();
	void scriptResetToSceneScript();
	void scriptLoadOverlayFont();
	OpcodeControlFlow scriptAddOverlayTextEntry();
	void scriptClearOverlayText();
	void scriptFadeToBlack();
	bool scriptLoadPcmSound();
	bool scriptPlayPcmSound();
	bool scriptWaitForSound();
	void scriptStopPcmSound();
	bool scriptLoadMusicSlot();
	bool scriptPlayMusicSlot();
	bool scriptStopMusicSlot();
	bool scriptWaitForMusic();
	bool scriptFreeMusicSlot();
	bool scriptGetObjectX();
	bool scriptGetObjectY();
	bool scriptGetObjectField8();
	bool scriptGetObjectOrientation();
	void scriptClearActorInventory();
	bool scriptSetPathfindingRemap();
	bool scriptWaitForAdlib();
	void scriptSkipUntil14();
	void scriptChangeAnimation();
	void scriptFrameWait();
	void scriptSetPathfinding();
	void scriptTestSceneAnimFrame();
	void scriptEndOverlayText();
	void scriptFadeFromBlack();
	void scriptFreePcmSound();

	inline void scriptUnimplementedOpcode(const char *source, uint16 opcode) {
		debug("Unimplemented opcode (%s): %.2x.", source, opcode);
	}

	// State variables from here

	// Overall state
	ExecutorState _state = ExecutorState::Idle;

	// Currently executed script
	Common::MemoryReadStream *_stream;

	// [1014h] global - current assumption is that this is set when we run
	// the script for the scene initialization and reset when we run when the
	// scene is active
	bool _isSceneInitRun = false;

	// [1012h] global - current assumption is that this guards script runs that
	// [1012h] g_wRepeatRunFlag - set during the repeat script pass
	// that runs after scene init to process object scripts
	bool _repeatRunFlag = false;

	uint16 _executingObjectIndex;

	ScriptExecutionState _scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;

	// Returns true if the object is relevant to be executing in the current scene
	bool isRelevantObject(const GameObject *obj) const;

	// Handles the next step of execution based on the current state.
	// Can be run right after a previous step or be called after execution was paused
	// Needs to update the state to be valid again
	void step();

	// Depending on the current state, chooses the next script to run
	// and adjusts the state
	// Should return true if a new script was loaded
	bool loadNextScript();

	bool _isTimerActive = false;
	uint32 _timerEndMillis;
	bool _isFrameWaitActive = false;
	uint16 _frameWaitTicksRemaining = 0;

	// We use this array to gather the dialogue choices as they come in
	Common::Array<Common::StringArray> _dialogueChoices;

	Common::String identifyScriptOpcode(uint8 opcode, uint8 opcode2);
	Common::String identifyHelperOpcode(uint8 opcode, uint16 value);

	byte readByte();
	uint16 readUint16();

	// We use this to keep track of whether we have read all bytes we should have read
	uint32 _expectedEndLocation;

	// scriptSkipBlock: skips nested opcode blocks
	void scriptSkipBlock();
	// scriptSkipAlternate: alternate skip (for opcode 8)
	void scriptSkipAlternate();

	bool loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 objectTableOffset = 0x189);
	bool loadSoundResource(Common::Array<uint8> &outData, uint8 resourceIndex);
	bool loadMusicResource(Common::Array<uint8> &outData, uint8 resourceIndex);

	// scriptReadValue: reads a typed value from the script stream
	void scriptReadValuePair(uint16 &out1, uint16 &out2);

	// Combines both 16 bit values into a 32 bit value
	uint32 scriptReadValue32();

	// Returns only the first of the two 16 bit values
	uint16 scriptReadValue16();

	// Saves the given value in a script variable
	void scriptSaveVariableHelper(uint32 value);

	void scriptPrintString(bool alignRight = false);

	Common::StringArray _debugBuffer;
	bool _lastOpcodeTriggeredSkip = false;
	void beginBuffering();
	void endBuffering(bool shouldMark = false);

	// Global [0F92h], seems to be 0 if we execute the script of the scene
	// Global [0F92h] g_wExecutingScriptObjectId: 0 when executing the
	// scene script, otherwise the object index whose script is running.
	uint16 _executingScriptObjectID;

public:
	ScriptExecutor();

	void setWaitingForCallback() { _state = ExecutorState::WaitingForCallback; }

	// This is where a secondary inventory was last opened,
	// when it is closed, we need to execute from here
	uint32 _secondaryInventoryLocation;
	bool _hasPendingExternalInventoryResume = false;
	uint16 _externalInventorySourceObjectID = 0;
	MouseMode _savedExternalInventoryMouseMode = MouseMode::Use;

	// Determines if we actually look something up when looking up 9F4D FF 27
	// in the obstacles/pathfinding map
	// Should be 1 while we execute a "character stopped walking" script,
	// otherwise 0
	bool _pathWalkableResult = false;
	bool _isRepeatRun = false;

	// Scene data [di+53B7h] - TODO: Confirm that we use a script variable as well as this thing
	int _chosenDialogueOption = 0;
	uint16 _activeDialogueSpeakerObjectID = 0;

	// 0x2000 bytes / 4 bytes per var = 2048 variables max (indices 1-0x800).
	// All zeroed on init by memsetBytes in loadResourceFile.
	Common::Array<ScriptVariable> _variables;

	MouseMode _mouseMode = MouseMode::Walk;
	MouseMode _cursorModeBeforeWait = MouseMode::Walk;

	uint16 _interactedObjectID = 0;
	uint16 _interactedOtherObjectID = 0;
	uint16 _walkTargetObjectIndex = 0;
	// Script click state: set by handleInput when user clicks during script execution.
	// Saved/restored by scriptOpenInventory across UI interactions.
	uint16 _scriptClickFlag = 0;
	uint16 _scriptClickX = 0;
	uint16 _scriptClickY = 0;
	uint16 _scriptClickResult = 0;
	uint16 _savedScriptClickFlag = 0;
	uint16 _savedScriptClickX = 0;
	uint16 _savedScriptClickY = 0;
	uint16 _savedScriptClickResult = 0;

	// Is set to true in opcode 2C if an object is inside another target object
	bool _inventoryCheckResult = false;
	bool _animBlobRangeTestResult = false;
	bool _soundEnabled = true;
	// [102Ah] - separate from soundEnabled (sound). Controls whether the
	// player can fast-forward the current script section via button 8.
	// Set by opcode 0x1C, cleared by opcode 0x1D.
	bool _scriptSkippable = false;
	bool _musicEnabled = true;
	bool _soundSystemActive = true;
	bool _overlayTextStageActive = false;
	bool _inventoryActionFlag = false;
	bool _inventoryCombineFlag = false;
	Common::Array<uint8> _musicSlots[2];
	uint16 _activeMusicSlot = 0;
	uint16 _musicControlMode = 0;
	uint16 _musicControlParam = 0;
	uint16 _musicControlVolume = 0;
	bool _waitForSoundPlayback = false;
	bool _waitForMusicControl = false;
	bool _waitForAdlibReady = false;
	bool _debugPaused = false;
	bool _pickupInProgress = false;
	uint16 _pickupActorObjectID = 0;
	uint16 _pickupTargetObjectID = 0;
	MouseMode _savedPickupMouseMode = MouseMode::Use;

	// Legacy flag for script callback scheduling. Marked for removal.
	bool _requestCallback = false;
	bool _isRunningScript = false;
	bool _isAwaitingCallback = false;
	// Mutex indicating if the A3D2 function is active
	bool _isSkipping = false;

	Macs2::Macs2Engine *_engine;

	// Button 8 skip from handleInput (1008:e8bf)
	bool skipToEndOfSkippableSection();

	// Implements a lookup in the "areas" map
	uint16 getAreaAtPoint(uint16 x, uint16 y);

	void setVariableValue(uint16 index, uint16 a, uint16 b);

	void setVariableValue(uint16 index, uint32 value);

	Common::Point getCharPosition();

	ExecutionResult executeScript();

	// Will execute the script and any object scripts until execution should be stopped
	// TODO: Consider if we should let the executor also figure out where to get the
	// first script from
	void run(bool firstRun = false);

	void setScript(Common::MemoryReadStream *stream);

	void setCurrentSceneScriptAt(uint32 offset);

	void tick();

	void startTimer(uint32 duration);
	void endTimer();
	void startFrameWait(uint16 duration);
	void endFrameWait();

	bool isExecuting() const {
		// TODO: Implement mutexes correctly
		// return _isRunningScript || _isAwaitingCallback;
		return _state != ExecutorState::Idle;
	}

	uint32 getScriptPosition() const;
	uint32 getScriptEndPosition() const;
	uint16 getExecutingObjectId() const { return _executingObjectIndex; }
	void setExecutingObjectId(uint16 id) { _executingObjectIndex = id; }
	uint16 getFrameWaitCounter() const { return _frameWaitTicksRemaining; }
	void setFrameWaitCounter(uint16 val) { _frameWaitTicksRemaining = val; }
	uint32 getVariableValue(int index) const;

	// Computes the read-only runtime value for a type 0xFF special
	// (FF:value), matching scriptReadValuePair. For debugger display only.
	uint32 getSpecialValue(uint16 value);

	// Returns true if the save/load menu can be opened (no blocking state)
	bool canOpenSaveMenu() const {
		return !_isSceneInitRun && !_isFrameWaitActive && !_isTimerActive;
	}

	// Resets the script to the beginning.
	// Confirmed: runScriptExecutor (1008:e3e7) sets position=0 on fresh runs
	// (g_wScriptIsExecuting==0). Mid-execution pauses resume from current position.
	void rewind();
};

} // namespace Script
} // namespace Macs2

#endif
