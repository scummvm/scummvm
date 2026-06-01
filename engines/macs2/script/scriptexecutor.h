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
	void scriptOpcode0x01();
	void scriptOpcode0x02();
	void scriptOpcode0x03();
	void scriptOpcode0x04();
	bool scriptOpcode0x05();
	void scriptOpcode0x06();
	void scriptOpcode0x07();
	void scriptOpcode0x08();
	void scriptOpcode0x09();
	void scriptOpcode0x0A();
	bool scriptOpcode0x0B();
	ExecutionResult scriptOpcode0x0C();
	ExecutionResult scriptOpcode0x0D();
	bool scriptOpcode0x10();
	ExecutionResult scriptOpcode0x11();
	void scriptOpcode0x14();
	void scriptOpcode0x15();
	void scriptOpcode0x16();
	ExecutionResult scriptOpcode0x17();
	ExecutionResult scriptOpcode0x18();
	bool scriptOpcode0x19();
	bool scriptOpcode0x1A();
	bool scriptOpcode0x1B();
	void scriptOpcode0x1C();
	void scriptOpcode0x1D();
	bool scriptOpcode0x1E();
	void scriptOpcode0x1F();
	bool scriptOpcode0x20();
	bool scriptOpcode0x21();
	bool scriptOpcode0x22();
	bool scriptOpcode0x23();
	void scriptOpcode0x24();
	void scriptOpcode0x25();
	void scriptOpcode0x26();
	bool scriptOpcode0x27();
	void scriptOpcode0x28();
	bool scriptOpcode0x29();
	void scriptOpcode0x2A();
	bool scriptOpcode0x2B();
	bool scriptOpcode0x2C();
	bool scriptOpcode0x2D();
	bool scriptOpcode0x2F();
	void scriptOpcode0x30();
	void scriptOpcode0x31();
	bool scriptOpcode0x32();
	bool scriptOpcode0x33();
	void scriptOpcode0x13();
	void scriptOpcode0x0E();
	void scriptOpcode0x0F();
	void scriptOpcode0x12();
	void scriptOpcode0x2E();
	void scriptOpcode0x39();
	void scriptOpcode0x3D();
	void scriptOpcode0x3F();

	inline void ScriptUnimplementedOpcode(const char *source, uint16 opcode) {
		debug("Unimplemented opcode (%s): %.2x.", source, opcode);
	}

	inline void ScriptUnimplementedOpcode_Helper(uint16 opcode) {
		// TODO: Could this also be done with a template?
		ScriptUnimplementedOpcode("Helper", opcode);
	}

	inline void ScriptUnimplementedOpcode_Main(uint16 opcode) {
		// TODO: Could this also be done with a template?
		ScriptUnimplementedOpcode("Main", opcode);
	}

	// State variables from here

	// Overall state
	ExecutorState _state = ExecutorState::Idle;

	// Currently executed script
	Common::MemoryReadStream *_stream;

	// [1014h] global - current assumption is that this is set when we run
	// the script for the scene initialization and reset when we run when the
	// scene is active
	bool IsSceneInitRun = false;

	// [1012h] global - current assumption is that this guards script runs that
	// [1012h] g_wRepeatRunFlag - set during the repeat script pass
	// that runs after scene init to process object scripts
	bool repeatRunFlag = false;

	uint16 executingObjectIndex;

	ScriptExecutionState scriptExecutionState = ScriptExecutionState::ExecutingSceneScript;

	// Returns true iff the object is relevant to be executing in the current scene
	bool IsRelevantObject(const GameObject *obj);

	// Handles the next step of execution based on the current state.
	// Can be run right after a previous step or be called after execution was paused
	// Needs to update the state to be valid again
	void Step();

	// Depending on the current state, chooses the next script to run
	// and adjusts the state
	// Should return true iff a new script was loaded
	bool LoadNextScript();

	bool isTimerActive = false;
	uint32 timerEndMillis;
	bool isFrameWaitActive = false;
	uint16 frameWaitTicksRemaining = 0;

	// We use this array to gather the dialogue choices as they come in
	Common::Array<Common::StringArray> DialogueChoices;

	// TODO: Put in a git module
	Common::String IdentifyScriptOpcode(uint8 opcode, uint8 opcode2);
	Common::String IdentifyHelperOpcode(uint8 opcode, uint16 value);

	// Does pretty much what 9F07 does
	byte ReadByte();
	uint16 ReadWord();

	// We use this to keep track of whether we have read all bytes we should have read
	uint32 expectedEndLocation;

	// fn0037_A3D2 - scriptSkipBlock: skips nested opcode blocks
	void scriptSkipBlock();
	// fn0037_A37A - scriptscriptSkipAlternate: alternate skip (for opcode 8)
	void scriptSkipAlternate();

	bool IsPathWalkable(const Common::Point &from, const Common::Point &to);
	bool loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 objectTableOffset = 0x189);
	bool loadSoundResource(Common::Array<uint8> &outData, uint8 resourceIndex);
	bool loadMusicResource(Common::Array<uint8> &outData, uint8 resourceIndex);

	// void getAreaAtPoint(uint16 x, uint16 y);

	// fn0037_9F4D - scriptReadValue: reads a typed value from the script stream
	void scriptReadValuePair(uint16 &out1, uint16 &out2);

	// Function to be used if we only want to have the script be advanced
	// due to a skipped implementation
	void scriptReadValue_Placeholder();

	// Combines both 16 bit values into a 32 bit value
	uint32 scriptReadValue32();

	// Returns only the first of the two 16 bit values
	uint16 scriptReadValue16();

	// Saves the given value in a script variable
	// fn0037_A334 - scriptSaveVariable: saves value to a script variable
	void scriptSaveVariable(uint32 value);

	// fn0037_C991 proc
	// Implements a walk to
	void scriptLoadSpecialAnimImpl();

	// Implements opcode 0x28 - stops the current animation on an object
	// fn0037_C8E4 - scriptStopAnimation
	void scriptStopAnimationImpl();

	// Implements opcode 0e - changing scene animations

	// 01E7:A903
	void ScriptPrintString(bool alignRight = false);

	Common::StringArray debugBuffer;
	bool lastOpcodeTriggeredSkip = false;
	void BeginBuffering();
	void EndBuffering(bool shouldMark = false);

	// Global [0F92h], seems to be 0 if we execute the script of the scene
	// Global [0F92h] g_wExecutingScriptObjectId: 0 when executing the
	// scene script, otherwise the object index whose script is running.
	uint16 _executingScriptObjectID;

public:
	int64 _streamDumpPosition;
	void DumpWholeScript();

	// Button 8 skip from handleInput (1008:e8bf)
	bool skipToEndOfSkippableSection();

	// This is where a secondary inventory was last opened,
	// when it is closed, we need to execute from here
	uint32 secondaryInventoryLocation;
	bool hasPendingExternalInventoryResume = false;
	uint16 externalInventorySourceObjectID = 0;
	MouseMode savedExternalInventoryMouseMode = MouseMode::Use;

	// Implements a lookup in the "areas" map
	uint16 getAreaAtPoint(uint16 x, uint16 y);

	// Determines if we actually look something up when looking up 9F4D FF 27
	// in the obstacles/pathfinding map
	// Should be 1 while we execute a "character stopped walking" script,
	// otherwise 0
	bool pathWalkableResult = false;
	bool isRepeatRun = false;

	// Scene data [di+53B7h] - TODO: Confirm that we use a script variable as well as this thing
	int chosenDialogueOption = 0;
	uint16 activeDialogueSpeakerObjectID = 0;

	// 0x2000 bytes / 4 bytes per var = 2048 variables max (indices 1-0x800).
	// All zeroed on init by memsetBytes in loadResourceFile.
	Common::Array<ScriptVariable> _variables;

	void SetVariableValue(uint16 index, uint16 a, uint16 b);

	void SetVariableValue(uint16 index, uint32 value);

	Common::Point GetCharPosition();

	MouseMode _mouseMode = MouseMode::Walk;
	MouseMode _cursorModeBeforeWait = MouseMode::Walk;

	uint16 _interactedObjectID = 0;
	uint16 _interactedOtherObjectID = 0;

	// Is set to true in opcode 2C if an object is inside another target object
	bool inventoryCheckResult = false;
	bool animBlobRangeTestResult = false;
	bool soundEnabled = true;
	// [102Ah] - separate from soundEnabled (sound). Controls whether the
	// player can fast-forward the current script section via button 8.
	// Set by opcode 0x1C, cleared by opcode 0x1D.
	bool scriptSkippable = false;
	bool musicEnabled = true;
	bool soundSystemActive = true;
	bool overlayTextStageActive = false;
	bool inventoryActionFlag = false;
	bool inventoryCombineFlag = false;
	Common::Array<uint8> musicSlots[2];
	uint16 activeMusicSlot = 0;
	uint16 musicControlMode = 0;
	uint16 musicControlParam = 0;
	uint16 musicControlVolume = 0;
	bool waitForSoundPlayback = false;
	bool waitForMusicControl = false;
	bool waitForAdlibReady = false;
	bool pickupInProgress = false;
	uint16 pickupActorObjectID = 0;
	uint16 pickupTargetObjectID = 0;
	MouseMode savedPickupMouseMode = MouseMode::Use;

	// Legacy flag for script callback scheduling. Marked for removal.
	bool requestCallback = false;

	Macs2::Macs2Engine *_engine;

	ScriptExecutor();

	ExecutionResult ExecuteScript();

	// Will execute the script and any object scripts until execution should be stopped
	// TODO: Consider if we should let the executor also figure out where to get the
	// first script from
	void Run(bool firstRun = false);

	void SetScript(Common::MemoryReadStream *stream);

	void SetCurrentSceneScriptAt(uint32 offset);

	void tick();

	void StartTimer(uint32 duration);
	void EndTimer();
	void StartFrameWait(uint16 duration);
	void EndFrameWait();

	bool isRunningScript = false;
	bool isAwaitingCallback = false;

	// TODO: Impplement mutexes correctly
	bool IsExecuting() const {
		return _state != ExecutorState::Idle;
		// return isRunningScript || isAwaitingCallback;
	}

	uint32 GetScriptPosition() const;
	uint32 GetScriptEndPosition() const;
	uint16 GetExecutingObjectId() const { return executingObjectIndex; }
	uint32 GetVariableValue(int index) const;

	// Computes the read-only runtime value for a type 0xFF special
	// (FF:value), matching scriptReadValuePair. For debugger display only.
	uint32 GetSpecialValue(uint16 value);

	// Returns true if the save/load menu can be opened (no blocking state)
	bool canOpenSaveMenu() const {
		return !IsSceneInitRun && !isFrameWaitActive && !isTimerActive;
	}

	// Mutex indicating if the A3D2 function is active
	bool isSkipping = false;

	// Resets the script to the beginning.
	// Confirmed: runScriptExecutor (1008:e3e7) sets position=0 on fresh runs
	// (g_wScriptIsExecuting==0). Mid-execution pauses resume from current position.
	void Rewind();
};
} // namespace Script

} // namespace Macs2

#endif
