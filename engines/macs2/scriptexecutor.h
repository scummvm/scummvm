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
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str-array.h"

namespace Common {
class MemoryReadStream;
}

namespace Macs2 {
class Macs2Engine;
class Character;
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

/** Result of a script opcode handler / executeOpcodes(). */
enum class OpcodeResult {
	/** Continue with the next opcode in the current script. */
	Continue,
	/** Break the opcode loop, then finish (clears running flag). */
	FinishScript,
	/** Pause execution and wait for an external callback (leaves running flag set). */
	WaitForCallback,
	/**
	 * Finish immediately (leaves running flag set).
	 * Used by changeScene / waitForWalk / showDialogueChoice / dismissPanel.
	 */
	ReturnFinished
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
public:
	typedef OpcodeResult (ScriptExecutor::*OpcodeHandler)();

	struct OpcodeEntry {
		const char *name;
		OpcodeHandler handler;
	};

	/** Install the active opcode table (use when a platform/version remaps opcodes). */
	void setOpcodeTable(const OpcodeEntry *table, uint size);
	/** Name for opcode, or "?" if unset/out of range. */
	const char *opcodeName(uint8 opcode) const;

	/** Script dialect v1 opcode table (MCS / Amiga demo bytecode). */
	static const OpcodeEntry kV1OpcodeTable[];
	static const uint kV1OpcodeTableSize;
	/**
	 * Script dialect v2 opcode table (extends v1 through 0x6D).
	 * Remaps a few audio slots and adds 0x4F..0x6D
	 */
	static const OpcodeEntry kV2OpcodeTable[];
	static const uint kV2OpcodeTableSize;

private:
#ifdef DEMACS2
public:
#endif
	OpcodeResult scriptSetVar();
	OpcodeResult scriptSetVarOr();
	OpcodeResult scriptIfTrue();
	OpcodeResult scriptIfFalse();
	OpcodeResult scriptCompare();
	OpcodeResult scriptIfInteraction();
	OpcodeResult scriptEndIf();
	OpcodeResult scriptElse();
	OpcodeResult scriptNop09();
	OpcodeResult scriptPrintStringLeft();
	OpcodeResult scriptMoveObject();
	OpcodeResult scriptChangeScene();
	OpcodeResult scriptShowDialogue();
	OpcodeResult scriptWalkToPosition();
	OpcodeResult scriptWaitForWalk();
	OpcodeResult scriptSkipWord();
	OpcodeResult scriptClearDialogueChoices();
	OpcodeResult scriptAddDialogueChoice();
	OpcodeResult scriptShowDialogueChoice();
	OpcodeResult scriptDismissPanel();
	OpcodeResult scriptWalkToAndPickup();
	OpcodeResult scriptSetPickupFrames();
	OpcodeResult scriptSetupObject();
	OpcodeResult scriptSetSkippable();
	OpcodeResult scriptClearSkippable();
	OpcodeResult scriptPlayAnimation();
	OpcodeResult scriptTestPathfinding();
	OpcodeResult scriptSetYOffset();
	OpcodeResult scriptSetMotion();
	OpcodeResult scriptSetOrientation();
	OpcodeResult scriptMoveToPosition();
	OpcodeResult scriptAddValues();
	OpcodeResult scriptSubValues();
	OpcodeResult scriptLoadSpecialAnim();
	OpcodeResult scriptSetDirection();
	OpcodeResult scriptStopAnimation();
	OpcodeResult scriptOpenInventory();
	OpcodeResult scriptLoadObjectAnim();
	OpcodeResult scriptCheckObjectData();
	OpcodeResult scriptCheckInventory();
	OpcodeResult scriptSetSnapToTarget();
	OpcodeResult scriptTestObjectAnimFrame();
	OpcodeResult scriptPrintStringRight();
	OpcodeResult scriptSetPaletteDarkness();
	OpcodeResult scriptSetObjectShading();
	OpcodeResult scriptSetObjectScaling();
	OpcodeResult scriptSetHotspotOverride();
	OpcodeResult scriptSetObjectBounds();
	OpcodeResult scriptDismissAllPanels();
	OpcodeResult scriptResetToSceneScript();
	OpcodeResult scriptLoadOverlayFont();
	OpcodeResult scriptAddOverlayTextEntry();
	OpcodeResult scriptClearOverlayText();
	OpcodeResult scriptFadeToBlack();
	OpcodeResult scriptLoadPcmSound();
	OpcodeResult scriptPlayPcmSound();
	OpcodeResult scriptWaitForSound();
	OpcodeResult scriptStopPcmSound();
	OpcodeResult scriptLoadMusicSlot();
	OpcodeResult scriptPlayMusicSlot();
	OpcodeResult scriptStopMusicSlot();
	OpcodeResult scriptWaitForMusic();
	OpcodeResult scriptFreeMusicSlot();
	OpcodeResult scriptGetObjectX();
	OpcodeResult scriptGetObjectY();
	OpcodeResult scriptGetObjectField8();
	OpcodeResult scriptGetObjectOrientation();
	OpcodeResult scriptClearActorInventory();
	OpcodeResult scriptSetPathfindingRemap();
	OpcodeResult scriptWaitForAdlib();
	OpcodeResult scriptSkipUntil14();
	OpcodeResult scriptChangeAnimation();
	OpcodeResult scriptFrameWait();
	OpcodeResult scriptSetPathfinding();
	OpcodeResult scriptTestSceneAnimFrame();
	OpcodeResult scriptEndOverlayText();
	OpcodeResult scriptFadeFromBlack();
	OpcodeResult scriptFreePcmSound();

	/** Seek to the length-prefixed end of the current opcode payload. */
	void scriptSkipOpcodeRemainder(uint8 opcode);

	// Dialect v2: remapped audio slots (v1 PCM/music-slot opcodes are NOPs here).
	OpcodeResult scriptNopSkipRemainder();
	OpcodeResult scriptPlaySfx();
	OpcodeResult scriptPlaySong();
	OpcodeResult scriptStopSong();
	/** Fixed 13-byte Pascal-style filename field from the script stream. */
	Common::String scriptReadFixedFileName();
	Common::String scriptParsePascalFileName(const Common::String &raw);
	/** Resolve SPEECH/SOUNDFX basename (no extension) for openStreamFile; empty if none. */
	Common::Path resolveAudioFilePath(const Common::String &fileName, bool preferSpeech) const;
	/** Resolve MUSICGS then MUSICOPL; empty if neither exists. */
	Common::Path resolveMidiFilePath(const Common::String &fileName) const;
	/**
	 * Optional generated dialogue audio (kEnhAudioChanges):
	 * scene -> SPEECH/sSS_OOOO.*, object -> SPEECH/oOOO_OOOO.*.
	 * Missing files are ignored.
	 */
	void tryPlayGeneratedDialogueSpeech(uint16 stringOffset);

	// Dialect v2: extended opcodes 0x4F..0x6D (stubs until backends exist).
	OpcodeResult scriptSetMainActor();
	OpcodeResult scriptLoadDeltaAnim();
	OpcodeResult scriptPlayDeltaAnim();
	OpcodeResult scriptRemoveDeltaAnim();
	OpcodeResult scriptSetButtonStep();
	OpcodeResult scriptTestButtonAnimFrame();
	OpcodeResult scriptScreenShot();
	OpcodeResult scriptWaitObjectAnimStep();
	OpcodeResult scriptWaitSpecialAnimStep();
	OpcodeResult scriptSetObjectAdjust();
	OpcodeResult scriptReloadSpecialAnim();
	OpcodeResult scriptPlayDiskDelta();
	OpcodeResult scriptSetDiskCache();
	OpcodeResult scriptSetMidiVolume();
	OpcodeResult scriptSetWaveVolume();
	OpcodeResult scriptLoadSpecialAnimSlot();
	OpcodeResult scriptSetSpecialAnimSlot();
	OpcodeResult scriptClearSpecialAnimSlot();
	OpcodeResult scriptSetDeltaRange();
	OpcodeResult scriptClearDeltaRange();
	OpcodeResult scriptAddDeltaSfx();
	OpcodeResult scriptClearDeltaSfxList();
	OpcodeResult scriptShowActionBar();
	OpcodeResult scriptHideActionBar();
	OpcodeResult scriptSetCursorType();
	OpcodeResult scriptCheckDeltaSpeed();
	OpcodeResult scriptLoadDistanceMask();
	OpcodeResult scriptLoadAreaMask();
	OpcodeResult scriptLoadWalkMask();
	OpcodeResult scriptLoadShadowMask();
	OpcodeResult scriptTalkTo();

	inline void scriptUnimplementedOpcode(const char *source, uint16 opcode) {
		debug("Unimplemented opcode (%s): %.2x.", source, opcode);
	}
#ifdef DEMACS2
private:
#endif

	// Active opcode table
	const OpcodeEntry *_opcodeTable = nullptr;
	uint _opcodeTableSize = 0;

	// State variables from here

	// Overall state
	ExecutorState _state = ExecutorState::Idle;

	// Currently executed script
	Common::MemoryReadStream *_stream = nullptr;

	// [1014h] global - current assumption is that this is set when we run
	// the script for the scene initialization and reset when we run when the
	// scene is active
	bool _isSceneInitRun = false;

	// [1012h] global - current assumption is that this guards script runs that
	// [1012h] g_wRepeatRunFlag - set during the repeat script pass
	// that runs after scene init to process object scripts
	bool _repeatRunFlag = false;

	// Binary runs init and repeat as separate runScriptExecutor calls inside
	// scriptChangeScene. Init pass = scene script (isSceneInit) + object scripts.
	// Repeat pass = second call with RepeatRunFlag=1 only.
	bool _initPassComplete = false;
	bool _deferredRepeatAfterInit = false;
	bool _terminateOuterScriptBeforeRepeat = false;

	uint16 _executingObjectIndex = 0;

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
	uint32 _timerEndMillis = 0;
	bool _isFrameWaitActive = false;
	uint16 _frameWaitTicksRemaining = 0;

	// We use this array to gather the dialogue choices as they come in

	Common::String identifyScriptOpcode(uint8 opcode, uint8 opcode2);
	Common::String identifyHelperOpcode(uint8 opcode, uint16 value);

	byte readByte();
	uint16 readUint16();

	// We use this to keep track of whether we have read all bytes we should have read
	uint32 _expectedEndLocation = 0;
	uint8 _lastOpcode = 0;
	uint32 _lastOpcodeStreamPos = 0;

	// scriptSkipBlock: skips nested opcode blocks
	void scriptSkipBlock();
	// scriptSkipAlternate: alternate skip (for opcode 8)
	void scriptSkipAlternate();

	bool loadIndexedResource(Common::Array<uint8> &outData, uint8 resourceIndex, uint16 objectTableOffset = 0x189);
	/**
	 * Load PCM for opcode 0x3E.
	 * DOS: indexed MCS resource (2-byte header). Amiga: OS_/MXOS from DataA (raw PCM).
	 * On success, rateHz/headerSkip describe how playSample should consume the buffer.
	 */
	bool loadSoundResource(Common::Array<uint8> &outData, uint8 resourceIndex,
						   int &rateHz, int &headerSkip);
	bool loadMusicResource(Common::Array<uint8> &outData, uint8 resourceIndex);

	// scriptReadValue: reads a typed value from the script stream
	void scriptReadValuePair(uint16 &out1, uint16 &out2);

	// Combines both 16 bit values into a 32 bit value
	uint32 scriptReadValue32();

	// Returns only the first of the two 16 bit values
	uint16 scriptReadValue16();

	/** Read a script value and convert to screen/runtime coordinates. */
	int16 scriptReadCoord16();

	/** Skip optional padding word after a script variable index when required. */
	void skipOptionalVarIndexPadding();

	// Saves the given value in a script variable
	void scriptSaveVariableHelper(uint32 value);

	void scriptPrintString(bool alignRight = false);

	Common::StringArray _debugBuffer;
	bool _lastOpcodeTriggeredSkip = false;
	void beginBuffering();
	void endBuffering(bool shouldMark = false);

	// g_wExecutingScriptObjectId [0F92h]: 0 when executing the scene script,
	// 1..0x200 the object index whose script is running, > 0x200 when the
	// object iteration in run() is exhausted (or a script terminated the run,
	// e.g. opcode 0x29). The binary drives the whole script-selection flow off
	// this single value; there is no separate scene-vs-object state flag.
	uint16 _executingScriptObjectId = 0;
	// Binary g_wScriptEndPosition [0xf90] and g_wScriptIsExecuting [0xf88].
	// End is the active script byte limit (scene+0x520b or object runtime+0x18b);
	// isExecuting is (position < end), persisted in savegames.
	uint32 _scriptEndPosition = 0;
	bool _scriptIsExecuting = false;

	uint32 effectiveScriptEnd() const;
	void clampScriptEndToStream();

public:
	ScriptExecutor(Macs2::Macs2Engine *engine);
	~ScriptExecutor();

	void syncScriptIsExecutingFlag();

	/** Strip a trailing audio extension (.wav/.ogg/...) if present. */
	static Common::String stripAudioExtension(const Common::String &fileName);

	void setIdle() { _state = ExecutorState::Idle; }

	/**
	 * Restore mid-script execution after loadGameFromFile (1008:747e).
	 * When isExecuting, mirrors binary: reattach scene/object script at saved
	 * position and leave the executor resumable (WaitingForCallback) so the next
	 * runScriptExecutor continues instead of rewinding to a fresh scene pass.
	 */
	void restoreScriptExecutionAfterLoad(bool isExecuting, uint16 executingObjectId,
										 uint16 scriptPosition, uint16 scriptEndPosition);

	/** Align executing object id / end position with the active script stream before save. */
	void prepareScriptStateForSave();

	Common::Array<uint16> _dialogueChoiceScriptIndices;
	Common::Array<Common::StringArray> _dialogueChoices;

	// Binary scriptOpenInventory (1008:c3e6) / handleInput panel-3 close (1008:e8bf):
	// saved script stream state restored when container inventory closes.
	uint32 _savedOpenInventoryScriptPos = 0;
	uint32 _savedOpenInventoryScriptEndPos = 0;
	uint16 _savedOpenInventoryExecutingObjectId = 0;
	// Legacy alias used by saveload; kept in sync with _savedOpenInventoryScriptPos.
	uint32 _secondaryInventoryLocation = 0;
	bool _hasPendingExternalInventoryResume = false;
	uint16 _externalInventorySourceObjectID = 0;
	MouseMode _savedExternalInventoryMouseMode = MouseMode::Use;

	// Determines if we actually look something up when looking up 9F4D FF 27
	// in the obstacles/pathfinding map
	// Should be 1 while we execute a "character stopped walking" script,
	// otherwise 0
	bool _pathWalkableResult = false;
	bool _isRepeatRun = false;

	// chosen dialogue script index
	int _chosenDialogueOption = 0;
	uint16 _dialogueSpeakerObjectID = 0;

	// 0x2000 bytes / 4 bytes per var = 2048 variables max (indices 1-0x800).
	// All zeroed on init by memsetBytes in loadResourceFile.
	Common::Array<ScriptVariable> _variables;

	// g_wCursorMode (1020:0fe6): the active cursor mode (0x13..0x1A).
	MouseMode _cursorMode = MouseMode::Walk;
	MouseMode _cursorModeBeforeWait = MouseMode::Walk;
	// Set by opcodes 0x0A/0x0D/0x17 while waiting for the player to dismiss text UI.
	// Binary keeps a clickable cursor for those waits; step() must not force hourglass.
	bool _waitingForUiClick = false;

	uint16 _interactedObjectID = 0;
	// g_wInteractedInventoryItemId [1026h]: inventory item involved in a
	// use-inventory-item-on-object interaction (0x400 + item object index).
	uint16 _interactedInventoryItemId = 0;
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
	/** Dialogue/subtitle text display toggle (native options HUD). */
	bool _textEnabled = true;
	bool _overlayTextStageActive = false;
	bool _inventoryActionFlag = false;
	bool _inventoryCombineFlag = false;
	Common::Array<uint8> _musicSlots[2];
	uint16 _activeMusicSlot = 0;
	uint16 _musicControlMode = 0;
	// g_wMusicControlStep: per-tick volume fade step for the music fade in gameTick.
	uint16 _musicControlStep = 0;
	uint16 _musicControlVolume = 0;
	// g_wWaitForPcmSound [100Ch] (opcode 0x41), g_wWaitForMusicControl [100Eh]
	// (opcode 0x47), g_wWaitForAdlibReady [1010h] (opcode 0x4E): wait flags
	// polled by the gameTick wait cascade (View1::tick).
	bool _waitForPcmSound = false;
	bool _waitForMusicControl = false;
	bool _waitForAdlibReady = false;
	// Dialect-v2: wait until an object/scene anim sequencePosition reaches a target.
	bool _waitForObjectAnimStep = false;
	uint16 _waitObjectAnimObjectId = 0;
	uint16 _waitObjectAnimSlot = 0;
	uint16 _waitObjectAnimTargetStep = 0;
	bool _waitForSpecialAnimStep = false;
	uint16 _waitSpecialAnimIndex = 0;
	uint16 _waitSpecialAnimTargetStep = 0;
	bool _waitForDeltaAnim = false;
	bool _waitForDeltaSpeed = false;
	bool _debugPaused = false;
	bool _pickupInProgress = false;
	uint16 _pickupActorObjectID = 0;
	uint16 _pickupTargetObjectID = 0;

	bool _isRunningScript = false;
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

	// executeOpcodes (1008:db56): the opcode dispatch loop for the current script.
	OpcodeResult executeOpcodes();

	// Execute the scene script and any object scripts until execution should stop.
	// Fresh runs bind Scenes::_currentSceneScript at offset 0; mid-script resumes continue in place.
	void run(bool firstRun = false);

	void setScript(Common::MemoryReadStream *stream);
	void releaseObjectStream();

	void setCurrentSceneScriptAt(uint32 offset);

	void tick();

	void startTimer(uint32 duration);
	void endTimer();
	void startFrameWait(uint16 duration);
	void endFrameWait();

	// True when script execution is paused on any wait opcode (frame, walk, dialogue, etc.).
	bool isScriptWaitDeferred() const {
		return _state == ExecutorState::WaitingForCallback ||
			   _frameWaitTicksRemaining != 0 || _walkTargetObjectIndex != 0 ||
			   _waitForPcmSound || _waitForMusicControl || _waitForAdlibReady ||
			   _waitForObjectAnimStep || _waitForSpecialAnimStep ||
			   _waitForDeltaAnim || _waitForDeltaSpeed;
	}

	bool isExecuting() const {
		return _state != ExecutorState::Idle;
	}

	/** Binary g_wScriptIsExecuting: script VM paused mid-bytecode (incl. UI waits). */
	bool isScriptMidExecution() const {
		return _scriptIsExecuting;
	}

	uint32 getScriptPosition() const;
	// Returns the position of the last executed/executing opcode (for debugger highlight)
	uint32 getDebugOpcodePosition() const;
	bool isWaitingForCallback() const {
		return _state == ExecutorState::WaitingForCallback;
	}
	uint32 getScriptEndPosition() const;
	// Binary g_wExecutingScriptObjectId [0xf92]: 0 = scene script, 1..0x200 = object.
	// Not _executingObjectIndex (iteration cursor / last scene index after Idle).
	uint16 getExecutingObjectId() const {
		return _executingScriptObjectId;
	}
	void setExecutingObjectId(uint16 id) {
		_executingScriptObjectId = id;
	}
	uint16 getFrameWaitCounter() const {
		return _frameWaitTicksRemaining;
	}
	void setFrameWaitCounter(uint16 val) {
		_frameWaitTicksRemaining = val;
	}
	bool isFrameWaitActive() const {
		return _isFrameWaitActive;
	}
	bool getRepeatRunFlag() const {
		return _repeatRunFlag;
	}
	void setRepeatRunFlag(bool val) {
		_repeatRunFlag = val;
	}
	uint32 getVariableValue(int index) const;

	// Plate / walk debugging: log actor position, area, walkability, script waits.
	void debugLogActorWalkState(const char *context);

	// Computes the read-only runtime value for a type 0xFF special
	// (FF:value), inlined in scriptReadValue in the original binary
	uint32 getSpecialValue(uint16 value);

	// Returns true if the save/load menu can be opened (no blocking state)
	bool canOpenSaveMenu() const {
		return !_isSceneInitRun && !_isFrameWaitActive && !_isTimerActive;
	}

	// g_wScriptErrorCode (1020:0f86): non-zero halts opcode dispatch (1008:db56).
	uint16 _scriptErrorCode = 0;
	Character *getOrCreateCharacter(uint16 objectID);
	void saveWalkRuntime(const Character *c, GameObject *o);
	void restoreWalkRuntime(Character *c, const GameObject *o);
	void clearStoredWalkRuntime(GameObject *o);
	void seedMoveToPositionState(GameObject *object, Character *c, const Common::Point &target, uint16 targetVerticalOffset);
	void seedMotionState(GameObject *object, Character *c, uint16 targetVerticalOffset, uint16 verticalOffsetDelta, uint16 motionDistance);
	void saveOpenInventoryScriptContext();
	void restoreOpenInventoryScriptContext();
	void setScriptError(uint16 code);
	bool hasScriptError() const {
		return _scriptErrorCode != 0;
	}
	void clearScriptError() {
		_scriptErrorCode = 0;
	}
	uint16 getScriptErrorCode() const {
		return _scriptErrorCode;
	}

	// Debug globals PTR_LOOP_1020_06c2 / PTR_LOOP_1020_06c4 (saved on script halt).
	uint32 _errorScriptPosition = 0;
	uint16 _errorScriptContext = 0;
	void recordScriptErrorPosition();

	// Binary scriptChangeScene (1008:ad6e) synchronous init/repeat script passes.
	void beginSceneEntryInitPass();
	void finishSceneEntryRepeatPass(bool terminateOuterScript);
	void runSceneEntryScriptPasses();
	void runSceneScriptPass(bool initRun, bool repeatRun);

	// Binary executeOpcodes (1008:db56): blocking waits save cursor then set 0x1A.
	void enterBlockingWaitCursor();
	void clearScriptUiWaitState();

	// Resets the script to the beginning.
	// Confirmed: runScriptExecutor (1008:e3e7) sets position=0 on fresh runs
	// (g_wScriptIsExecuting==0). Mid-execution pauses resume from current position.
	void rewind();
};

} // namespace Script
} // namespace Macs2

#endif
