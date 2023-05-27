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

#include "common/random.h"

#include "vcruise/audio_player.h"
#include "vcruise/circuitpuzzle.h"
#include "vcruise/runtime.h"
#include "vcruise/script.h"


namespace VCruise {

#ifdef PEEK_STACK
#error "PEEK_STACK is already defined"
#endif

#ifdef TAKE_STACK
#error "TAKE_STACK is already defined"
#endif

#ifdef OPCODE_STUB
#error "OPCODE_STUB is already defined"
#endif

#define PEEK_STACK(n)                                                                         \
	if (!requireAvailableStack(n))                                                            \
		return;                                                                               \
	const StackValue *stackArgs = &this->_scriptStack[this->_scriptStack.size() - (n)]


#define TAKE_STACK_INT_NAMED(n, arrayName)                                                    \
	StackInt_t arrayName[n];                                                                  \
	do {                                                                                      \
		if (!requireAvailableStack(n))                                                        \
			return;                                                                           \
		const uint stackSize = _scriptStack.size();                                           \
		const StackValue *stackArgsPtr = &this->_scriptStack[stackSize - (n)];                \
		for (uint i = 0; i < (n); i++) {                                                      \
			if (stackArgsPtr[i].type != StackValue::kNumber)                                  \
				error("Expected op argument %u to be a number", i);                           \
			arrayName[i] = stackArgsPtr[i].value.i;                                           \
		}                                                                                     \
		this->_scriptStack.resize(stackSize - (n));                                           \
	} while (false)

#define TAKE_STACK_INT(n) TAKE_STACK_INT_NAMED(n, stackArgs)

#define TAKE_STACK_STR_NAMED(n, arrayName)                                     \
	Common::String arrayName[n];                                               \
	do {                                                                       \
		if (!requireAvailableStack(n))                                         \
			return;                                                            \
		const uint stackSize = _scriptStack.size();                            \
		const StackValue *stackArgsPtr = &this->_scriptStack[stackSize - (n)]; \
		for (uint i = 0; i < (n); i++) {                                       \
			if (stackArgsPtr[i].type != StackValue::kString)                   \
				error("Expected op argument %u to be a string", i);            \
			arrayName[i] = Common::move(stackArgsPtr[i].value.s);              \
		}                                                                      \
		this->_scriptStack.resize(stackSize - (n));                            \
	} while (false)

#define TAKE_STACK_STR(n) TAKE_STACK_STR_NAMED(n, stackArgs)

#define TAKE_STACK_VAR_NAMED(n, arrayName)                                     \
	StackValue arrayName[n];                                                   \
	do {                                                                       \
		if (!requireAvailableStack(n))                                         \
			return;                                                            \
		const uint stackSize = _scriptStack.size();                            \
		const StackValue *stackArgsPtr = &this->_scriptStack[stackSize - (n)]; \
		for (uint i = 0; i < (n); i++)                                         \
			arrayName[i] = Common::move(stackArgsPtr[i]);                      \
		this->_scriptStack.resize(stackSize - (n));                            \
	} while (false)

#define TAKE_STACK_VAR(n) TAKE_STACK_VAR_NAMED(n, stackArgs)

#define OPCODE_STUB(op)                           \
	void Runtime::scriptOp##op(ScriptArg_t arg) { \
		error("Unimplemented opcode '" #op "'");  \
	}

void Runtime::scriptOpNumber(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(arg));
}

void Runtime::scriptOpRotate(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs + kAnimDefStackArgs);

	_panLeftAnimationDef = stackArgsToAnimDef(stackArgs + 0);
	_panRightAnimationDef = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);
	_haveHorizPanAnimations = true;
}

void Runtime::scriptOpAngle(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_scriptStack.push_back(StackValue((stackArgs[0] == static_cast<StackInt_t>(_direction)) ? 1 : 0));
}

void Runtime::scriptOpAngleGGet(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] < 0 || stackArgs[0] >= static_cast<StackInt_t>(GyroState::kNumGyros))
		error("Invalid gyro index in angleGGet op");

	_scriptStack.push_back(StackValue(_gyros.gyros[stackArgs[0]].currentState));
}

void Runtime::scriptOpSpeed(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_scriptEnv.fpsOverride = stackArgs[0];
}

void Runtime::scriptOpSAnimL(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs + 2);

	if (stackArgs[kAnimDefStackArgs] != 0)
		warning("sanimL second operand wasn't zero (what does that do?)");

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);
	uint direction = stackArgs[kAnimDefStackArgs + 1];

	if (direction >= kNumDirections)
		error("sanimL invalid direction");

	_haveIdleAnimations[direction] = true;

	StaticAnimation &outAnim = _idleAnimations[direction];

	outAnim = StaticAnimation();
	outAnim.animDefs[0] = animDef;
	outAnim.animDefs[1] = animDef;
}

void Runtime::scriptOpChangeL(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	// ChangeL changes the screen number.
	//
	// If this isn't an entry script, then this must also re-trigger the entry script.
	//
	// In Reah, it also forces screen entry scripts to replay, which is needed for things like the fountain.
	//
	// In Schizm, it's needed for the preset buttons in the airship navigation coordinates to work correctly
	// (Room 41 screen 0c2h)
	//
	// The check is required because otherwise, this causes an infinite loop in the temple when approaching the
	// bells puzzle (Room 65 screen 0b2h) due to fnMlynekZerowanie -> 1 fnMlynkiLokacja -> changeL to MLYNKIZLEWEJ1
	_screenNumber = stackArgs[0];
	_havePendingScreenChange = true;

	if (!_scriptEnv.isEntryScript)
		_forceScreenChange = true;
}

void Runtime::scriptOpAnimR(ScriptArg_t arg) {
	bool isRight = false;

	if (_scriptEnv.panInteractionID == kPanLeftInteraction) {
		debug(1, "Pan-left interaction from direction %u", _direction);

		uint reverseDirectionSlice = (kNumDirections - _direction);
		if (reverseDirectionSlice == kNumDirections)
			reverseDirectionSlice = 0;

		uint initialFrame = reverseDirectionSlice * (_panLeftAnimationDef.lastFrame - _panLeftAnimationDef.firstFrame) / kNumDirections + _panLeftAnimationDef.firstFrame;

		AnimationDef trimmedAnimation = _panLeftAnimationDef;
		trimmedAnimation.lastFrame--;

		debug(1, "Running frame loop of %u - %u from frame %u", trimmedAnimation.firstFrame, trimmedAnimation.lastFrame, initialFrame);

		changeAnimation(trimmedAnimation, initialFrame, false, _animSpeedRotation);
		_gameState = kGameStatePanLeft;
	} else if (_scriptEnv.panInteractionID == kPanRightInteraction) {
		debug(1, "Pan-right interaction from direction %u", _direction);

		uint initialFrame = _direction * (_panRightAnimationDef.lastFrame - _panRightAnimationDef.firstFrame) / kNumDirections + _panRightAnimationDef.firstFrame;

		AnimationDef trimmedAnimation = _panRightAnimationDef;
		trimmedAnimation.lastFrame--;

		debug(1, "Running frame loop of %u - %u from frame %u", trimmedAnimation.firstFrame, trimmedAnimation.lastFrame, initialFrame);

		changeAnimation(trimmedAnimation, initialFrame, false, _animSpeedRotation);
		_gameState = kGameStatePanRight;

		isRight = true;
	}

	uint cursorID = 0;
	if (_haveHorizPanAnimations) {
		uint panCursor = kPanCursorDraggableHoriz;

		if (isRight)
			panCursor |= kPanCursorDirectionRight;
		else
			panCursor |= kPanCursorDirectionLeft;

		cursorID = _panCursors[panCursor];
	}

	changeToCursor(_cursors[cursorID]);
	drawCompass();
}

void Runtime::scriptOpAnimF(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs + 3);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);

	const AnimationDef *faceDirectionAnimDef = nullptr;
	uint initialFrame = 0;
	uint stopFrame = 0;
	if (computeFaceDirectionAnimation(stackArgs[kAnimDefStackArgs + 2], faceDirectionAnimDef, initialFrame, stopFrame)) {
		_postFacingAnimDef = animDef;
		_animStopFrame = stopFrame;
		changeAnimation(*faceDirectionAnimDef, initialFrame, false, _animSpeedRotation);
		_gameState = kGameStateWaitingForFacingToAnim;
	} else {
		consumeAnimChangeAndAdjustAnim(animDef);	// Needed for Schizm when entering the statue after finishing the temple.

		changeAnimation(animDef, animDef.firstFrame, true, _animSpeedDefault);
		_gameState = kGameStateWaitingForAnimation;
	}
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;
	clearIdleAnimations();

	uint cursorID = kCursorArrow;
	if (_scriptEnv.panInteractionID == kPanUpInteraction)
		cursorID = _panCursors[kPanCursorDraggableUp | kPanCursorDirectionUp];
	else if (_scriptEnv.panInteractionID == kPanDownInteraction)
		cursorID = _panCursors[kPanCursorDraggableDown | kPanCursorDirectionDown];

	changeToCursor(_cursors[cursorID]);
}

void Runtime::scriptOpAnimN(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	const AnimationDef *faceDirectionAnimDef = nullptr;
	uint initialFrame = 0;
	uint stopFrame = 0;
	if (computeFaceDirectionAnimation(stackArgs[0], faceDirectionAnimDef, initialFrame, stopFrame)) {
		_animStopFrame = stopFrame;
		changeAnimation(*faceDirectionAnimDef, initialFrame, false);
		_gameState = kGameStateWaitingForFacing;
	}

	_direction = stackArgs[0];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpAnimG(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs * 2 + 1);

	_gyros.posAnim = stackArgsToAnimDef(stackArgs + 0);
	_gyros.negAnim = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);
	_gyros.isVertical = (stackArgs[kAnimDefStackArgs * 2 + 0] != 0);

	if (_gyros.isVertical)
		changeToCursor(_cursors[_panCursors[kPanCursorDraggableUp | kPanCursorDraggableDown]]);
	else
		changeToCursor(_cursors[_panCursors[kPanCursorDraggableHoriz]]);

	_gyros.dragBasePoint = _mousePos;
	_gyros.dragBaseState = _gyros.gyros[_gyros.activeGyro].currentState;
	_gyros.dragCurrentState = _gyros.dragBaseState;

	_gameState = kGameStateGyroIdle;
}

void Runtime::scriptOpAnimS(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);

	consumeAnimChangeAndAdjustAnim(animDef);

	// Static animations start on the last frame
	changeAnimation(animDef, animDef.lastFrame, false);

	// We have a choice of when to terminate animations: At the start of the final frame, or at the end of the final frame.
	// Terminating at the start of the final frame means many frames can play in a single gameplay frame.
	//
	// In Reah, we terminate at the start because it doesn't really cause problems anywhere and helps some things like
	// the basket weight puzzle in the bathhouse.
	//
	// In Schizm, several things like the mechanical computer and balloon gas puzzle pressure meter don't behave
	// well when doing this, so we terminate at the end of the frame instead there.
	_animTerminateAtStartOfFrame = (_gameID == GID_SCHIZM);

	_gameState = kGameStateWaitingForAnimation;
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpAnim(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs + 2);

	AnimationDef animDef = stackArgsToAnimDef(stackArgs + 0);

	consumeAnimChangeAndAdjustAnim(animDef);

	changeAnimation(animDef, animDef.firstFrame, true, _animSpeedDefault);

	_gameState = kGameStateWaitingForAnimation;
	_screenNumber = stackArgs[kAnimDefStackArgs + 0];
	_direction = stackArgs[kAnimDefStackArgs + 1];
	_havePendingScreenChange = true;

	clearIdleAnimations();

	if (_loadedAnimationHasSound)
		changeToCursor(nullptr);
	else {
		uint cursorID = kCursorArrow;
		if (_scriptEnv.panInteractionID == kPanUpInteraction)
			cursorID = _panCursors[kPanCursorDraggableUp | kPanCursorDirectionUp];
		else if (_scriptEnv.panInteractionID == kPanDownInteraction)
			cursorID = _panCursors[kPanCursorDraggableDown | kPanCursorDirectionDown];

		changeToCursor(_cursors[cursorID]);
	}
}

void Runtime::scriptOpStatic(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs);

	debug(10, "scriptOpStatic() kAnimDefStackArgs: %d", kAnimDefStackArgs);
	for (uint i = 0; i < kAnimDefStackArgs; i++) {
		debug(10, "\tstackArgs[%d]: %d", i, stackArgs[i]);
	}

	// FIXME: What does this actually do?
	// It looks like this sets the last frame of an animation as the current scene graphic, but
	// in some cases that's wrong.  For instance, after solving the temple puzzle in Reah, viewing
	// the rock on the left (screen 0c4 in room 20) runs ":PLANAS_SKALA static" after the rock
	// symbol displays.  However, :PLANAS_SKALA shows the rock with no symbol.
	//
	// Another problem occurs when viewing the rotor puzzle in the citadel, described below for now.
#if 0
	// QUIRK/BUG WORKAROUND: Static animations don't override other static animations!
	//
	// In Reah Room05, the script for 0b8 (NGONG) sets the static animation to :NNAWA_NGONG and then
	// to :NSWIT_SGONG, but NNAWA_NGONG is the correct one, so we must ignore the second static animation
	if (_haveIdleStaticAnimation)
		return;

	AnimationDef animDef = stackArgsToAnimDef(stackArgs);

	// QUIRK: In the Reah citadel rotor puzzle, all of the "BKOLO" screens execute :DKOLO1_BKOLO1 static but
	// doing that would replace the transition animation's last frame with the new static animation frame,
	// blanking out the puzzle, so we must detect if the new static animation is the same as the existing
	// one and if so, ignore it.
	if (animDef.animName == _idleCurrentStaticAnimation)
		return;

	// FIXME: _idleCurrentStaticAnimation must be cleared sometime!  Maybe on loading a save.

	changeAnimation(animDef, animDef.lastFrame, false, _animSpeedStaticAnim);

	_havePendingPreIdleActions = true;
	_haveHorizPanAnimations = false;
	_haveIdleStaticAnimation = true;
	_idleCurrentStaticAnimation = animDef.animName;

	_gameState = kGameStateWaitingForAnimation;
#endif
}

void Runtime::scriptOpVarLoad(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	uint32 varID = (static_cast<uint32>(_roomNumber) << 16) | static_cast<uint32>(stackArgs[0]);

	Common::HashMap<uint32, int32>::const_iterator it = _variables.find(varID);
	if (it == _variables.end())
		_scriptStack.push_back(StackValue(0));
	else
		_scriptStack.push_back(StackValue(it->_value));
}

void Runtime::scriptOpVarStore(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	uint32 varID = (static_cast<uint32>(_roomNumber) << 16) | static_cast<uint32>(stackArgs[1]);

	_variables[varID] = stackArgs[0];
}

void Runtime::scriptOpVarAddAndStore(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	uint32 varID = (static_cast<uint32>(_roomNumber) << 16) | static_cast<uint32>(stackArgs[0]);

	Common::HashMap<uint32, int32>::iterator it = _variables.find(varID);
	if (it == _variables.end())
		_variables[varID] = stackArgs[1];
	else
		it->_value += stackArgs[1];
}

void Runtime::scriptOpVarGlobalLoad(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	uint32 varID = static_cast<uint32>(stackArgs[0]);

	Common::HashMap<uint32, int32>::const_iterator it = _variables.find(varID);
	if (it == _variables.end())
		_scriptStack.push_back(StackValue(0));
	else
		_scriptStack.push_back(StackValue(it->_value));
}

void Runtime::scriptOpVarGlobalStore(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	uint32 varID = static_cast<uint32>(stackArgs[1]);

	_variables[varID] = stackArgs[0];
}

void Runtime::scriptOpItemCheck(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	for (const InventoryItem &item : _inventory) {
		if (item.itemID == static_cast<uint>(stackArgs[0])) {
			_scriptEnv.lastHighlightedItem = item.itemID;
			_scriptStack.push_back(StackValue(1));
			return;
		}
	}

	_scriptStack.push_back(StackValue(0));
}

void Runtime::scriptOpItemRemove(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	inventoryRemoveItem(stackArgs[0]);
}

void Runtime::scriptOpItemHighlightSet(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	bool isHighlighted = (stackArgs[1] != 0);

	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID == static_cast<uint>(stackArgs[0])) {
			item.highlighted = isHighlighted;
			drawInventory(slot);
			break;
		}
	}
}

void Runtime::scriptOpItemHighlightSetTrue(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID == static_cast<uint>(stackArgs[0])) {
			item.highlighted = true;
			drawInventory(slot);
			break;
		}
	}
}

void Runtime::scriptOpItemAdd(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] == 0) {
		// Weird special case, happens in Reah when breaking the glass barrier, this is called with 0 as the parameter.
		// This can't be an inventory clear because it will not clear the crutch, but it does take away the gong beater,
		// so the only explanation I can think of is that it clears the previously-checked inventory item.
		inventoryRemoveItem(_scriptEnv.lastHighlightedItem);
	} else
		inventoryAddItem(stackArgs[0]);
}

void Runtime::scriptOpItemClear(ScriptArg_t arg) {
	for (uint slot = 0; slot < kNumInventorySlots; slot++) {
		InventoryItem &item = _inventory[slot];

		if (item.itemID != 0) {
			item.highlighted = false;
			item.itemID = 0;
			item.graphic.reset();
			drawInventory(slot);
		}
	}
}

void Runtime::scriptOpItemHaveSpace(ScriptArg_t arg) {
	for (const InventoryItem &item : _inventory) {
		if (item.itemID == 0) {
			_scriptStack.push_back(StackValue(1));
			return;
		}
	}

	_scriptStack.push_back(StackValue(0));
}

void Runtime::scriptOpSetCursor(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= _cursors.size())
		error("Invalid cursor ID");

	uint resolvedCursorID = stackArgs[0];

	Common::HashMap<StackInt_t, uint>::const_iterator overrideIt = _scriptCursorIDToResourceIDOverride.find(resolvedCursorID);
	if (overrideIt != _scriptCursorIDToResourceIDOverride.end())
		resolvedCursorID = overrideIt->_value;

	changeToCursor(_cursors[resolvedCursorID]);
}

void Runtime::scriptOpSetRoom(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_roomNumber = stackArgs[0];
}

void Runtime::scriptOpLMB(ScriptArg_t arg) {
	if (!_scriptEnv.lmb) {
		_idleHaveClickInteraction = true;
		terminateScript();
	}
}

void Runtime::scriptOpLMB1(ScriptArg_t arg) {
	if (!_scriptEnv.lmbDrag) {
		_idleHaveDragInteraction = true;
		terminateScript();
	}
}

void Runtime::scriptOpSoundS1(ScriptArg_t arg) {
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorNo, *cachedSound, 100, 0, false, false);
}

void Runtime::scriptOpSoundS2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(1, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorNo, *cachedSound, sndParamArgs[0], 0, false, false);
}

void Runtime::scriptOpSoundS3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorNo, *cachedSound, sndParamArgs[0], sndParamArgs[1], false, false);
}

void Runtime::scriptOpSoundL1(ScriptArg_t arg) {
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorYes, *cachedSound, getDefaultSoundVolume(), 0, false, false);
}

void Runtime::scriptOpSoundL2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(1, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorYes, *cachedSound, sndParamArgs[0], 0, false, false);
}

void Runtime::scriptOpSoundL3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorYes, *cachedSound, sndParamArgs[0], sndParamArgs[1], false, false);
}

void Runtime::scriptOp3DSoundL2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(3, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		setSound3DParameters(*cachedSound, sndParamArgs[1], sndParamArgs[2], _pendingSoundParams3D);
		triggerSound(kSoundLoopBehaviorYes, *cachedSound, sndParamArgs[0], 0, true, false);
	}
}

void Runtime::scriptOp3DSoundL3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(4, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		setSound3DParameters(*cachedSound, sndParamArgs[2], sndParamArgs[3], _pendingSoundParams3D);
		triggerSound(kSoundLoopBehaviorYes, *cachedSound, sndParamArgs[0], sndParamArgs[1], true, false);
	}
}

void Runtime::scriptOp3DSoundS2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(3, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		setSound3DParameters(*cachedSound, sndParamArgs[1], sndParamArgs[2], _pendingSoundParams3D);
		triggerSound(kSoundLoopBehaviorNo, *cachedSound, sndParamArgs[0], 0, true, false);
	}
}

void Runtime::scriptOpStopAL(ScriptArg_t arg) {
	warning("stopaL not implemented yet");
}

void Runtime::scriptOpAddXSound(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(3, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	RandomAmbientSound sound;
	sound.name = sndNameArgs[0];
	sound.volume = sndParamArgs[0];
	sound.balance = sndParamArgs[1];
	sound.frequency = sndParamArgs[2];

	_randomAmbientSounds.push_back(sound);
}

void Runtime::scriptOpClrXSound(ScriptArg_t arg) {
	_randomAmbientSounds.clear();
}

void Runtime::scriptOpStopSndLA(ScriptArg_t arg) {
	warning("StopSndLA not implemented yet");
}

void Runtime::scriptOpStopSndLO(ScriptArg_t arg) {
	TAKE_STACK_VAR(1);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByNameOrID(stackArgs[0], false, soundID, cachedSound);

	if (cachedSound)
		stopSound(*cachedSound);
}

void Runtime::scriptOpRange(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	_pendingSoundParams3D.minRange = stackArgs[0];
	_pendingSoundParams3D.maxRange = stackArgs[1];
	_pendingSoundParams3D.unknownRange = stackArgs[2];
}

void Runtime::scriptOpMusic(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	changeMusicTrack(stackArgs[0]);
}

void Runtime::scriptOpMusicVolRamp(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	uint32 duration = static_cast<uint32>(stackArgs[0]) * 100u;
	int32 newVolume = stackArgs[1];

	_musicVolumeRampRatePerMSec = 0;

	if (duration == 0) {
		_musicVolume = newVolume;
		if (_musicPlayer)
			_musicPlayer->setVolume(newVolume);
	} else {
		if (newVolume != _musicVolume) {
			uint32 timestamp = g_system->getMillis();

			_musicVolumeRampRatePerMSec = (newVolume - _musicVolume) * 65536 / static_cast<int32>(duration);
			_musicVolumeRampStartTime = timestamp;
			_musicVolumeRampStartVolume = _musicVolume;
			_musicVolumeRampEnd = newVolume;
		}
	}
}

void Runtime::scriptOpParm0(ScriptArg_t arg) {
	TAKE_STACK_INT(4);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm0");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.numPreviousStatesRequired = 3;
	for (uint i = 0; i < 3; i++)
		gyro.requiredPreviousStates[i] = stackArgs[i + 1];
}

void Runtime::scriptOpParm1(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm1");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.currentState = stackArgs[1];
	gyro.requiredState = stackArgs[2];

	gyro.requireState = true;
}

void Runtime::scriptOpParm2(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	_gyros.completeInteraction = stackArgs[0];
	_gyros.failureInteraction = stackArgs[1];
	_gyros.frameSeparation = stackArgs[2];

	if (_gyros.frameSeparation <= 0)
		error("Invalid gyro frame separation");
}

void Runtime::scriptOpParm3(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] < 0 || static_cast<uint>(stackArgs[0]) >= GyroState::kNumGyros)
		error("Invalid gyro index for Parm3");

	uint gyroIndex = stackArgs[0];

	Gyro &gyro = _gyros.gyros[gyroIndex];
	gyro.wrapAround = true;
}

void Runtime::scriptOpParmG(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	int32 gyroSlot = stackArgs[0];
	int32 dragMargin = stackArgs[1];
	int32 maxValue = stackArgs[2];

	if (gyroSlot < 0 || static_cast<uint>(gyroSlot) >= GyroState::kNumGyros)
		error("Invalid gyro slot from ParmG op");

	_gyros.activeGyro = gyroSlot;
	_gyros.dragMargin = dragMargin;
	_gyros.maxValue = maxValue;
}

void Runtime::scriptOpSParmX(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	_pendingStaticAnimParams.initialDelay = stackArgs[0];
	_pendingStaticAnimParams.repeatDelay = stackArgs[1];
	_pendingStaticAnimParams.lockInteractions = (stackArgs[2] != 0);
}

void Runtime::scriptOpSAnimX(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs * 2 + 1);

	AnimationDef animDef1 = stackArgsToAnimDef(stackArgs + 0);
	AnimationDef animDef2 = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);

	uint direction = stackArgs[kAnimDefStackArgs * 2 + 0];

	if (direction >= kNumDirections)
		error("sanimX invalid direction");

	_haveIdleAnimations[direction] = true;

	StaticAnimation &outAnim = _idleAnimations[direction];

	outAnim = StaticAnimation();
	outAnim.animDefs[0] = animDef1;
	outAnim.animDefs[1] = animDef2;
	outAnim.params = _pendingStaticAnimParams;
}

void Runtime::scriptOpVolumeUp3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_VAR_NAMED(1, sndIDArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByNameOrID(sndIDArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSoundRamp(*cachedSound, sndParamArgs[0] * 100, sndParamArgs[1], false);
}

void Runtime::scriptOpVolumeDn2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(1, sndParamArgs);
	TAKE_STACK_VAR_NAMED(1, sndIDArgs);

	uint32 durationMSec = static_cast<uint>(sndParamArgs[0]) * 100u;

	if (sndIDArgs[0].type == StackValue::kNumber && sndIDArgs[0].value.i == 0) {
		// Apply to all sounds
		for (const Common::SharedPtr<SoundInstance> &sndPtr : _activeSounds)
			triggerSoundRamp(*sndPtr, durationMSec, 0, true);
	} else {
		StackInt_t soundID = 0;
		SoundInstance *cachedSound = nullptr;
		resolveSoundByNameOrID(sndIDArgs[0], true, soundID, cachedSound);

		if (cachedSound)
			triggerSoundRamp(*cachedSound, durationMSec, getSilentSoundVolume(), true);
	}
}

void Runtime::scriptOpVolumeDn3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_VAR_NAMED(1, sndIDArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByNameOrID(sndIDArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSoundRamp(*cachedSound, sndParamArgs[0] * 100, sndParamArgs[1], false);
}

void Runtime::scriptOpVolumeDn4(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(3, sndParamArgs);
	TAKE_STACK_VAR_NAMED(1, sndIDArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByNameOrID(sndIDArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSoundRamp(*cachedSound, sndParamArgs[0] * 100, sndParamArgs[1], sndParamArgs[2] != 0);
}

void Runtime::scriptOpRandom(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] == 0)
		_scriptStack.push_back(StackValue(0));
	else
		_scriptStack.push_back(StackValue(_rng->getRandomNumber(stackArgs[0] - 1)));
}

void Runtime::scriptOpDrop(ScriptArg_t arg) {
	TAKE_STACK_VAR(1);
	(void)stackArgs;
}

void Runtime::scriptOpDup(ScriptArg_t arg) {
	TAKE_STACK_VAR(1);

	_scriptStack.push_back(stackArgs[0]);
	_scriptStack.push_back(stackArgs[0]);
}

void Runtime::scriptOpSwap(ScriptArg_t arg) {
	TAKE_STACK_VAR(2);

	_scriptStack.push_back(Common::move(stackArgs[1]));
	_scriptStack.push_back(Common::move(stackArgs[0]));
}

void Runtime::scriptOpSay1(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	// uint unk = sndParamArgs[0];
	uint cycleLength = sndParamArgs[1];
	debug(5, "Say1 cycle length: %u", cycleLength);

	Common::String soundIDStr = sndNameArgs[0];

	if (soundIDStr.size() < 4)
		error("Say1 sound name was invalid");

	uint32 cycleID = 0;
	
	for (uint i = 0; i < 4; i++) {
		char d = soundIDStr[i];
		if (d < '0' || d > '9')
			error("Invalid sound ID for say1");

		cycleID = cycleID * 10 + (d - '0');
	}

	uint &cyclePosRef = _sayCycles[static_cast<uint32>(cycleID)];

	uint32 cycledSoundID = (cyclePosRef + cycleID);
	cyclePosRef++;

	if (cyclePosRef == cycleLength)
		cyclePosRef = 0;

	soundIDStr = soundIDStr.substr(4);
	for (uint i = 0; i < 4; i++) {
		soundIDStr.insertChar(static_cast<char>((cycledSoundID % 10) + '0'), 0);
		cycledSoundID /= 10;
	}

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(soundIDStr, true, soundID, cachedSound);

	if (cachedSound) {
		triggerSound(kSoundLoopBehaviorNo, *cachedSound, 100, 0, false, true);
		triggerWaveSubtitles(*cachedSound, soundIDStr);
	}
}

void Runtime::scriptOpSay2(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		// The third param seems to control sound interruption, but say3 is a Reah-only op and it's only ever 1.
		if (sndParamArgs[1] != 1)
			error("Invalid interrupt arg for say2, only 1 is supported.");

		triggerSound(kSoundLoopBehaviorNo, *cachedSound, 100, 0, false, true);
		triggerWaveSubtitles(*cachedSound, sndNameArgs[0]);
	}
}

void Runtime::scriptOpSay3(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		TriggeredOneShot oneShot;
		oneShot.soundID = soundID;
		oneShot.uniqueSlot = sndParamArgs[0];

		// The third param seems to control sound interruption, but say3 is a Reah-only op and it's only ever 1.
		if (sndParamArgs[1] != 1)
			error("Invalid interrupt arg for say3, only 1 is supported.");

		if (Common::find(_triggeredOneShots.begin(), _triggeredOneShots.end(), oneShot) == _triggeredOneShots.end()) {
			triggerSound(kSoundLoopBehaviorNo, *cachedSound, 100, 0, false, true);
			_triggeredOneShots.push_back(oneShot);

			triggerWaveSubtitles(*cachedSound, sndNameArgs[0]);
		}
	}
}

void Runtime::scriptOpSay3Get(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		TriggeredOneShot oneShot;
		oneShot.soundID = soundID;
		oneShot.uniqueSlot = sndParamArgs[0];

		// The third param seems to control sound interruption, but say3 is a Reah-only op and it's only ever 1.
		if (sndParamArgs[1] != 1)
			error("Invalid interrupt arg for say3, only 1 is supported.");

		if (Common::find(_triggeredOneShots.begin(), _triggeredOneShots.end(), oneShot) == _triggeredOneShots.end()) {
			triggerSound(kSoundLoopBehaviorNo, *cachedSound, 100, 0, false, true);
			_triggeredOneShots.push_back(oneShot);
			_scriptStack.push_back(StackValue(soundID));
		} else
			_scriptStack.push_back(StackValue(0));
	} else
		_scriptStack.push_back(StackValue(0));
}

void Runtime::scriptOpSetTimer(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_timers[static_cast<uint>(stackArgs[0])] = g_system->getMillis() + static_cast<uint32>(stackArgs[1]) * 1000u;
}

void Runtime::scriptOpGetTimer(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	bool isCompleted = true;

	Common::HashMap<uint, uint32>::const_iterator timerIt = _timers.find(stackArgs[0]);
	if (timerIt != _timers.end())
		isCompleted = (g_system->getMillis() >= timerIt->_value);

	_scriptStack.push_back(StackValue(isCompleted ? 1 : 0));
}

void Runtime::scriptOpDelay(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_gameState = kGameStateDelay;
	_delayCompletionTime = g_system->getMillis() + stackArgs[0];
}

void Runtime::scriptOpLoSet(ScriptArg_t arg) {
	scriptOpVerticalPanSet(_havePanDownFromDirection);
}

void Runtime::scriptOpLoGet(ScriptArg_t arg) {
	scriptOpVerticalPanGet();
}

void Runtime::scriptOpHiSet(ScriptArg_t arg) {
	scriptOpVerticalPanSet(_havePanUpFromDirection);
}

void Runtime::scriptOpHiGet(ScriptArg_t arg) {
	scriptOpVerticalPanGet();
}

void Runtime::scriptOpVerticalPanSet(bool *flags) {
	TAKE_STACK_INT(2);

	uint baseDirection = static_cast<uint>(stackArgs[0]) % kNumDirections;
	uint radius = stackArgs[1];

	flags[baseDirection] = true;

	uint rDir = baseDirection;
	uint lDir = baseDirection;
	for (uint i = 1; i <= radius; i++) {
		rDir++;
		if (rDir == kNumDirections)
			rDir = 0;

		if (lDir == 0)
			lDir = kNumDirections;
		lDir--;

		flags[lDir] = true;
		flags[rDir] = true;
	}
}

void Runtime::scriptOpVerticalPanGet() {
	TAKE_STACK_INT(2);

	// In any scenario where this is used, there is a corresponding hi/lo set and this only ever triggers off of interactions,
	// so don't really even need to check anything other than the facing direction?
	uint baseDirection = static_cast<uint>(stackArgs[0]) % kNumDirections;
	uint radius = stackArgs[1];

	uint rtDirection = (baseDirection + kNumDirections - _direction) % kNumDirections;
	uint lfDirection = (_direction + kNumDirections - baseDirection) % kNumDirections;

	bool isInRadius = (rtDirection <= radius || lfDirection <= radius);

	_scriptStack.push_back(StackValue(isInRadius ? 1 : 0));
}

void Runtime::scriptOpCallFunction(ScriptArg_t arg) {
	Common::SharedPtr<Script> function = _scriptSet->functions[arg];
	if (function) {
		CallStackFrame newFrame;
		newFrame._script = function;
		newFrame._nextInstruction = 0;

		_scriptCallStack.push_back(newFrame);

		_gameState = kGameStateScriptReset;
	} else {
		error("Unknown function '%s'", _scriptSet->functionNames[arg].c_str());
	}
}

void Runtime::scriptOpReturn(ScriptArg_t arg) {
	_scriptCallStack.pop_back();
	_gameState = kGameStateScriptReset;
}

void Runtime::scriptOpSaveAs(ScriptArg_t arg) {
	TAKE_STACK_INT(4);

	// Just ignore this op, it looks like it's for save room remapping of some sort but we allow
	// saves at any idle screen.
	(void)stackArgs;
}

void Runtime::scriptOpSave0(ScriptArg_t arg) {
	warning("save0 op not implemented");
}

void Runtime::scriptOpExit(ScriptArg_t arg) {
	_isInGame = false;
	_mostRecentlyRecordedSaveState.reset();
	_mostRecentValidSaveState.reset();

	if (_gameID == GID_REAH) {
		_havePendingScreenChange = true;
		_forceScreenChange = true;

		_roomNumber = 40;
		_screenNumber = 0xa1;

		terminateScript();

		changeMusicTrack(0);
		if (_musicPlayer)
			_musicPlayer->setVolumeAndBalance(applyVolumeScale(getDefaultSoundVolume()), 0);
	} else {
		error("Don't know what screen to go to on exit");
	}
}

void Runtime::scriptOpNot(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_scriptStack.push_back(StackValue((stackArgs[0] == 0) ? 1 : 0));
}

void Runtime::scriptOpAnd(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] != 0 && stackArgs[1] != 0) ? 1 : 0));
}

void Runtime::scriptOpOr(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] != 0 || stackArgs[1] != 0) ? 1 : 0));
}

void Runtime::scriptOpAdd(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(stackArgs[0] + stackArgs[1]));
}

void Runtime::scriptOpSub(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(stackArgs[0] - stackArgs[1]));
}

void Runtime::scriptOpNegate(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_scriptStack.push_back(StackValue(-stackArgs[0]));
}

void Runtime::scriptOpCmpEq(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] == stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpCmpNE(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] != stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpCmpLt(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] < stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpCmpLE(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] <= stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpCmpGt(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] > stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpCmpGE(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue((stackArgs[0] >= stackArgs[1]) ? 1 : 0));
}

void Runtime::scriptOpBitLoad(ScriptArg_t arg) {
	TAKE_STACK_INT(2);


	_scriptStack.push_back(StackValue((stackArgs[0] >> stackArgs[1]) & 1));
}

void Runtime::scriptOpBitSet0(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	ScriptArg_t bitMask = static_cast<ScriptArg_t>(1) << stackArgs[1];
	_scriptStack.push_back(StackValue(stackArgs[0] & ~bitMask));
}

void Runtime::scriptOpBitSet1(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	ScriptArg_t bitMask = static_cast<ScriptArg_t>(1) << stackArgs[1];
	_scriptStack.push_back(StackValue(stackArgs[0] | bitMask));
}

void Runtime::scriptOpDisc1(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK_INT(1);
	(void)stackArgs;
	_scriptStack.push_back(StackValue(1));
}

void Runtime::scriptOpDisc2(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK_INT(2);
	(void)stackArgs;
	_scriptStack.push_back(StackValue(1));
}

void Runtime::scriptOpDisc3(ScriptArg_t arg) {
	// Disc check, always pass
	TAKE_STACK_INT(3);
	(void)stackArgs;
	_scriptStack.push_back(StackValue(1));
}

void Runtime::scriptOpGoto(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	uint newInteraction = static_cast<uint>(stackArgs[0]);

	Common::SharedPtr<Script> newScript = nullptr;

	if (_scriptSet) {
		RoomScriptSet *roomScriptSet = getRoomScriptSetForCurrentRoom();

		if (roomScriptSet) {
			const ScreenScriptSetMap_t &screenScriptsMap = roomScriptSet->screenScripts;
			ScreenScriptSetMap_t::const_iterator screenScriptIt = screenScriptsMap.find(_screenNumber);
			if (screenScriptIt != screenScriptsMap.end()) {
				const ScreenScriptSet &screenScriptSet = *screenScriptIt->_value;

				ScriptMap_t::const_iterator interactionScriptIt = screenScriptSet.interactionScripts.find(newInteraction);
				if (interactionScriptIt != screenScriptSet.interactionScripts.end())
					newScript = interactionScriptIt->_value;
			}
		}
	}

	if (newScript) {
		// This only happens in Reah so we don't have to worry about what to do about frames on the callstack in Schizm
		_gameState = kGameStateScriptReset;

		CallStackFrame frame;
		frame._script = newScript;
		frame._nextInstruction = 0;

		_scriptCallStack.resize(1);
		_scriptCallStack[0] = frame;
	} else {
		error("Goto target %u couldn't be resolved", newInteraction);
	}
}

void Runtime::scriptOpEscOn(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_escOn = (stackArgs[0] != 0);
}

void Runtime::scriptOpEscOff(ScriptArg_t arg) {
	_escOn = false;
}

void Runtime::scriptOpEscGet(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_scriptEnv.esc ? 1 : 0));
	_scriptEnv.esc = false;
}

void Runtime::scriptOpBackStart(ScriptArg_t arg) {
	_scriptEnv.exitToMenu = true;
}

void Runtime::scriptOpBlockSaves(ScriptArg_t arg) {
	warning("SAVES SHOULD BE BLOCKED ON THIS SCREEN");
}

void Runtime::scriptOpAnimName(ScriptArg_t arg) {
	if (_roomNumber >= _roomDefs.size())
		error("Can't resolve animation for room, room number was invalid");

	Common::String &animName = _scriptSet->strings[arg];

	// In Reah, animations are mapped to rooms.
	// 
	// In Schizm this can get very complicated: It supports overlapping room logics which in some cases
	// have animation ranges mapped to a different animation.
	//
	// For example, in Schizm, rooms 25-28 all share one logic file and their corresponding animations are
	// largely duplicates of each other with different skies.
	//
	// It appears that the animation to select is based on the remapped room if the animation can't be
	// found in its primary room.
	//
	// For example, PRZYCUMIE_KRZESELKO is mapped to an animation range in room 25 and another range in
	// room 26, and there is no mapping for room 28.  In this case, the animation frame range from room 25
	// is used, but it is remapped to animation 28.
	Common::SharedPtr<RoomDef> roomDef = _roomDefs[_roomNumber];
	if (roomDef) {
		Common::HashMap<Common::String, AnimationDef>::const_iterator it = roomDef->animations.find(animName);
		if (it != roomDef->animations.end()) {
			pushAnimDef(it->_value);
			return;
		}
	}

	if (_roomNumber < _roomDuplicationOffsets.size() && _roomDuplicationOffsets[_roomNumber] != 0) {
		int roomToUse = _roomNumber - _roomDuplicationOffsets[_roomNumber];

		roomDef = _roomDefs[roomToUse];

		Common::HashMap<Common::String, AnimationDef>::const_iterator it = roomDef->animations.find(animName);
		if (it != roomDef->animations.end()) {
			AnimationDef animDef = it->_value;

			if (animDef.animNum == roomToUse)
				animDef.animNum = _roomNumber;
			else if (animDef.animNum == -roomToUse)
				animDef.animNum = -static_cast<int>(_roomNumber);

			pushAnimDef(animDef);
			return;
		}
	}


	error("Can't resolve animation for room, couldn't find animation '%s'", animName.c_str());
}

void Runtime::scriptOpValueName(ScriptArg_t arg) {
	if (_roomNumber >= _roomDefs.size())
		error("Invalid room number for var name op");

	const RoomDef *roomDef = _roomDefs[_roomNumber].get();
	if (!roomDef)
		error("Room def doesn't exist");

	const Common::String &varName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, int>::const_iterator it = roomDef->values.find(varName);
	if (it == roomDef->values.end())
		error("Value '%s' doesn't exist in room %i", varName.c_str(), static_cast<int>(_roomNumber));

	_scriptStack.push_back(StackValue(it->_value));
}

void Runtime::scriptOpVarName(ScriptArg_t arg) {
	if (_roomNumber >= _roomDefs.size())
		error("Invalid room number for var name op");

	const RoomDef *roomDef = _roomDefs[_roomNumber].get();
	if (!roomDef)
		error("Room def doesn't exist");

	const Common::String &varName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, uint>::const_iterator it = roomDef->vars.find(varName);
	if (it == roomDef->vars.end())
		error("Var '%s' doesn't exist in room %i", varName.c_str(), static_cast<int>(_roomNumber));

	_scriptStack.push_back(StackValue(it->_value));
}

void Runtime::scriptOpSoundName(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_scriptSet->strings[arg]));
}

void Runtime::scriptOpCursorName(ScriptArg_t arg) {
	const Common::String &cursorName = _scriptSet->strings[arg];

	Common::HashMap<Common::String, StackInt_t>::const_iterator namedCursorIt = _namedCursors.find(cursorName);
	if (namedCursorIt == _namedCursors.end()) {
		error("Unimplemented cursor name '%s'", cursorName.c_str());
		return;
	}

	_scriptStack.push_back(StackValue(namedCursorIt->_value));
}

void Runtime::scriptOpDubbing(ScriptArg_t arg) {
	warning("Dubbing op not implemented");
}

void Runtime::scriptOpCheckValue(ScriptArg_t arg) {
	PEEK_STACK(1);

	if (stackArgs[0].type == StackValue::kNumber && stackArgs[0].value.i == arg)
		_scriptStack.pop_back();
	else
		_scriptCallStack.back()._nextInstruction++;
}

void Runtime::scriptOpJump(ScriptArg_t arg) {
	_scriptCallStack.back()._nextInstruction = arg;
}

void Runtime::scriptOpMusicStop(ScriptArg_t arg) {
	_musicPlayer.reset();
	_musicActive = false;
}

void Runtime::scriptOpMusicPlayScore(ScriptArg_t arg) {
	TAKE_STACK_STR(2);

	_scoreTrack = stackArgs[0];
	_scoreSection = stackArgs[1];
	_musicActive = true;

	startScoreSection();
}

void Runtime::scriptOpScoreAlways(ScriptArg_t arg) {
	assert(_gameID == GID_SCHIZM);

	_musicMuteDisabled = true;

	// We don't call startScoreSection here because ScoreAlways is always followed by a PlayScore
	// that triggers the actual music, and we don't want to play any amount of the score that's about
	// to be disabled.  PlayScore will call startScoreSection after changing to the correct section.
}

void Runtime::scriptOpScoreNormal(ScriptArg_t arg) {
	_musicMuteDisabled = false;

	if (_musicMute) {
		_musicPlayer.reset();
		_scoreSectionEndTime = 0;
	}
}

void Runtime::scriptOpSndPlay(ScriptArg_t arg) {
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorAuto, *cachedSound, getSilentSoundVolume(), 0, false, false);
}

void Runtime::scriptOpSndPlayEx(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_VAR_NAMED(1, sndNameArgs);

	Common::String soundName;
	if (sndNameArgs[0].type == StackValue::kString)
		soundName = sndNameArgs[0].value.s;
	else if (sndNameArgs[0].type == StackValue::kNumber) {
		// Sometimes the name is a string, such as the bell puzzle in the temple.
		// In this case the number is the name, with no suffix.
		soundName = Common::String::format("%i", static_cast<int>(sndNameArgs[0].value.i));
	} else
		error("Invalid sound name type for SndPlayEx");

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(soundName, true, soundID, cachedSound);

	if (cachedSound)
		triggerSound(kSoundLoopBehaviorAuto, *cachedSound, sndParamArgs[0], sndParamArgs[1], false, false);
}

void Runtime::scriptOpSndPlay3D(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(5, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	SoundParams3D sndParams;
	sndParams.minRange = sndParamArgs[2];
	sndParams.maxRange = sndParamArgs[3];
	sndParams.unknownRange = sndParamArgs[4]; // Doesn't appear to be the same thing as Reah.  Usually 1000, sometimes 2000 or 3000.

	if (cachedSound) {
		setSound3DParameters(*cachedSound, sndParamArgs[0], sndParamArgs[1], sndParams);
		triggerSound(kSoundLoopBehaviorAuto, *cachedSound, getSilentSoundVolume(), 0, true, false);
	}
}

void Runtime::scriptOpSndPlaying(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	SoundInstance *snd = resolveSoundByID(stackArgs[0]);
	if (!snd || !snd->cache) {
		_scriptStack.push_back(StackValue(0));
		return;
	}

	if (snd->cache->isLoopActive) {
		_scriptStack.push_back(StackValue(1));
		return;
	}

	bool hasEnded = (snd->endTime < g_system->getMillis());

	_scriptStack.push_back(StackValue(hasEnded ? 1 : 0));
}

void Runtime::scriptOpSndWait(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	SoundInstance *snd = resolveSoundByID(stackArgs[0]);
	if (snd) {
		_delayCompletionTime = snd->endTime;
		_gameState = kGameStateDelay;
	}
}

void Runtime::scriptOpSndHalt(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	SoundInstance *snd = resolveSoundByID(stackArgs[0]);
	if (snd) {
		convertLoopingSoundToNonLooping(*snd);

		_delayCompletionTime = snd->endTime;
		_gameState = kGameStateDelay;
	}
}

void Runtime::scriptOpSndToBack(ScriptArg_t arg) {
	recordSounds(*_altState);
}

void Runtime::scriptOpSndStop(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	SoundInstance *cachedSound = resolveSoundByID(stackArgs[0]);

	if (cachedSound)
		stopSound(*cachedSound);
}

void Runtime::scriptOpSndStopAll(ScriptArg_t arg) {
	for (const Common::SharedPtr<SoundInstance> &snd : _activeSounds)
		stopSound(*snd);
}

void Runtime::scriptOpVolumeAdd(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	SoundInstance *cachedSound = resolveSoundByID(static_cast<uint>(stackArgs[0]));

	if (cachedSound)
		triggerSoundRamp(*cachedSound, stackArgs[1] * 100, cachedSound->volume + stackArgs[2], false);
}

void Runtime::scriptOpVolumeChange(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	SoundInstance *cachedSound = resolveSoundByID(static_cast<uint>(stackArgs[0]));

	if (cachedSound)
		triggerSoundRamp(*cachedSound, stackArgs[1] * 100, stackArgs[2], false);
}

void Runtime::scriptOpAnimVolume(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	_animVolume = stackArgs[0];

	applyAnimationVolume();
}

void Runtime::scriptOpAnimChange(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	if (stackArgs[1] == 0)
		error("animChange frame count shouldn't be zero");

	_scriptEnv.animChangeSet = true;
	_scriptEnv.animChangeFrameOffset = stackArgs[0];
	_scriptEnv.animChangeNumFrames = stackArgs[1] - 1;
}

void Runtime::scriptOpScreenName(ScriptArg_t arg) {
	const Common::String &scrName = _scriptSet->strings[arg];

	uint roomNumber = _roomNumber;
	if (roomNumber < _roomDuplicationOffsets.size())
		roomNumber -= _roomDuplicationOffsets[roomNumber];

	RoomToScreenNameToRoomMap_t::const_iterator roomIt = _globalRoomScreenNameToScreenIDs.find(roomNumber);
	if (roomIt != _globalRoomScreenNameToScreenIDs.end()) {
		ScreenNameToRoomMap_t::const_iterator screenIt = roomIt->_value.find(scrName);

		if (screenIt != roomIt->_value.end()) {
			_scriptStack.push_back(StackValue(static_cast<StackInt_t>(screenIt->_value)));
			return;
		}
	}

	error("Couldn't resolve screen name '%s'", scrName.c_str());
}

void Runtime::scriptOpExtractByte(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(static_cast<StackInt_t>((stackArgs[0] >> (stackArgs[1] * 8) & 0xff))));
}

void Runtime::scriptOpInsertByte(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	StackInt_t value = stackArgs[0];
	StackInt_t valueToInsert = (stackArgs[1] & 0xff);
	int bytePos = stackArgs[2];

	StackInt_t mask = static_cast<StackInt_t>(0xff) << (bytePos * 8);

	value -= (value & mask);
	value += (valueToInsert << (bytePos * 8));

	_scriptStack.push_back(StackValue(value));
}

void Runtime::scriptOpString(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_scriptSet->strings[arg]));
}

void Runtime::scriptOpSpeechEx(ScriptArg_t arg) {
	TAKE_STACK_INT_NAMED(2, sndParamArgs);
	TAKE_STACK_STR_NAMED(1, sndNameArgs);

	StackInt_t soundID = 0;
	SoundInstance *cachedSound = nullptr;
	resolveSoundByName(sndNameArgs[0], true, soundID, cachedSound);

	if (cachedSound) {
		TriggeredOneShot oneShot;
		oneShot.soundID = soundID;
		oneShot.uniqueSlot = sndParamArgs[0];

		if (Common::find(_triggeredOneShots.begin(), _triggeredOneShots.end(), oneShot) == _triggeredOneShots.end()) {
			triggerSound(kSoundLoopBehaviorNo, *cachedSound, sndParamArgs[1], 0, false, true);
			_triggeredOneShots.push_back(oneShot);

			triggerWaveSubtitles(*cachedSound, sndNameArgs[0]);
		}
	}
}

void Runtime::scriptOpSpeechTest(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	bool found = false;

	for (const TriggeredOneShot &oneShot : _triggeredOneShots) {
		if (oneShot.soundID == static_cast<uint>(stackArgs[0])) {
			found = true;
			break;
		}
	}

	_scriptStack.push_back(StackValue(found ? 1 : 0));
}

void Runtime::scriptOpRandomInclusive(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	if (stackArgs[0] == 0)
		_scriptStack.push_back(StackValue(0));
	else
		_scriptStack.push_back(StackValue(_rng->getRandomNumber(stackArgs[0])));
}

void Runtime::scriptOpHeroOut(ScriptArg_t arg) {
	TAKE_STACK_INT(3);

	_swapOutRoom = stackArgs[0];
	_swapOutScreen = stackArgs[1];
	_swapOutDirection = stackArgs[2];
}

void Runtime::scriptOpHeroGetPos(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	bool thisHero = false;
	switch (stackArgs[0]) {
	case 0:
		thisHero = (_hero == 0);
		break;
	case 1:
		thisHero = (_hero == 1);
		break;
	case 2:
		thisHero = false;
		break;
	default:
		error("Unhandled heroGetPos argument %i", static_cast<int>(stackArgs[0]));
		return;
	}

	uint roomNumber = thisHero ? _roomNumber : _altState->roomNumber;
	uint screenNumber = thisHero ? _screenNumber : _altState->screenNumber;
	uint direction = thisHero ? _direction : _altState->direction;

	uint combined = (roomNumber << 16) | (screenNumber << 8) | direction;

	_scriptStack.push_back(StackValue(static_cast<StackInt_t>(combined)));
}

void Runtime::scriptOpHeroSetPos(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	bool thisHero = false;
	switch (stackArgs[0]) {
	case 0:
		thisHero = (_hero == 0);
		break;
	case 1:
		thisHero = (_hero == 1);
		break;
	case 2:
		thisHero = false;
		break;
	default:
		error("Unhandled heroSetPos argument %i", static_cast<int>(stackArgs[0]));
		return;
	}

	if (thisHero) {
		error("heroSetPos for the current hero isn't supported (and Schizm's game scripts shouldn't be doing it).");
		return;
	}

	_altState->roomNumber = (stackArgs[1] >> 16) & 0xff;
	_altState->screenNumber = (stackArgs[1] >> 8) & 0xff;
	_altState->direction = stackArgs[1] & 0xff;
	_altState->havePendingPostSwapScreenReset = true;
}

void Runtime::scriptOpHeroGet(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_hero));
}

void Runtime::scriptOpGetRoom(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_roomNumber));
}

void Runtime::scriptOpBitAnd(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(stackArgs[0] & stackArgs[1]));
}

void Runtime::scriptOpBitOr(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(stackArgs[0] | stackArgs[1]));
}

void Runtime::scriptOpAngleGet(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_direction));
}

void Runtime::scriptOpIsDVDVersion(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_isCDVariant ? 0 : 1));
}

void Runtime::scriptOpIsCDVersion(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_isCDVariant ? 1 : 0));
}

void Runtime::scriptOpDisc(ScriptArg_t arg) {
	TAKE_STACK_INT(1);

	(void)stackArgs;

	// Always pass correct disc checks
	_scriptStack.push_back(StackValue(1));
}

void Runtime::scriptOpHidePanel(ScriptArg_t arg) {
	_isInGame = false;

	clearTray();
}

void Runtime::scriptOpRotateUpdate(ScriptArg_t arg) {
	warning("RotateUpdate op not implemented yet");
}

void Runtime::scriptOpMul(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	_scriptStack.push_back(StackValue(stackArgs[0] * stackArgs[1]));
}

void Runtime::scriptOpDiv(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	if (stackArgs[1] == 0) {
		error("Division by zero");
		return;
	}

	_scriptStack.push_back(StackValue(stackArgs[0] / stackArgs[1]));
}

void Runtime::scriptOpMod(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	if (stackArgs[1] == 0) {
		error("Division by zero");
		return;
	}

	_scriptStack.push_back(StackValue(stackArgs[0] % stackArgs[1]));
}

void Runtime::scriptOpGetDigit(ScriptArg_t arg) {
	TAKE_STACK_INT(2);

	StackInt_t digit = (stackArgs[0] >> (stackArgs[1] * 4)) & 0xf;

	_scriptStack.push_back(StackValue(digit));
}

void Runtime::scriptOpPuzzleInit(ScriptArg_t arg) {
	TAKE_STACK_INT(kAnimDefStackArgs * 2 + 3);

	AnimationDef animDef1 = stackArgsToAnimDef(stackArgs + 0);
	AnimationDef animDef2 = stackArgsToAnimDef(stackArgs + kAnimDefStackArgs);

	int firstMover = stackArgs[kAnimDefStackArgs * 2 + 0];
	int firstMover2 = stackArgs[kAnimDefStackArgs * 2 + 1];
	int unknownParam = stackArgs[kAnimDefStackArgs * 2 + 2];

	if (firstMover != firstMover2 || unknownParam != 0)
		error("PuzzleInit had a weird parameter");

	clearCircuitPuzzle();
	_circuitPuzzle.reset(new CircuitPuzzle(firstMover));
	_circuitPuzzleConnectAnimation = animDef1;
	_circuitPuzzleBlockAnimation = animDef2;

	_scriptEnv.puzzleWasSet = true;

	if (firstMover == 2)
		scriptOpPuzzleDoMove2(0);
}

void Runtime::scriptOpPuzzleWhoWon(ScriptArg_t arg) {
	StackInt_t winner = 0;
	if (_circuitPuzzle) {
		switch (_circuitPuzzle->checkConclusion()) {
		case CircuitPuzzle::kConclusionNone:
			winner = 0;
			break;
		case CircuitPuzzle::kConclusionPlayerWon:
			winner = 1;
			break;
		case CircuitPuzzle::kConclusionPlayerLost:
			winner = 2;
			break;
		default:
			error("Unhandled puzzle conclusion");
			break;
		}
	}

	_scriptStack.push_back(StackValue(winner));
}

void Runtime::scriptOpPuzzleCanPress(ScriptArg_t arg) {
	_scriptStack.push_back(StackValue(_idleIsOnOpenCircuitPuzzleLink ? 1 : 0));
}

void Runtime::scriptOpPuzzleDoMove1(ScriptArg_t arg) {
	if (!_idleIsOnOpenCircuitPuzzleLink)
		error("Attempted puzzleDoMove1 but don't have a circuit point");

	if (!_circuitPuzzle)
		error("Attempted puzzleDoMove1 but the circuit puzzle is gone");

	_circuitPuzzle->addLink(_idleCircuitPuzzleCoord, _idleIsCircuitPuzzleLinkDown ? CircuitPuzzle::kCellDirectionDown : CircuitPuzzle::kCellDirectionRight);

	SoundInstance *snd = nullptr;
	StackInt_t soundID = 0;
	resolveSoundByName("85_connect", true, soundID, snd);

	if (snd)
		triggerSound(kSoundLoopBehaviorNo, *snd, 0, 0, false, false);

	const CircuitPuzzle::CellRectSpec *rectSpec = _circuitPuzzle->getCellRectSpec(_idleCircuitPuzzleCoord);

	if (rectSpec) {
		AnimationDef animDef = _circuitPuzzleConnectAnimation;
		animDef.constraintRect = _idleIsCircuitPuzzleLinkDown ? rectSpec->_downLinkRect : rectSpec->_rightLinkRect;

		changeAnimation(animDef, false);

		_gameState = kGameStateWaitingForAnimation;
	}

	clearCircuitHighlightRect(_idleCircuitPuzzleLinkHighlightRect);
	_idleIsOnOpenCircuitPuzzleLink = false;

	changeToCursor(_cursors[kCursorArrow]);
}

void Runtime::scriptOpPuzzleDoMove2(ScriptArg_t arg) {
	if (!_circuitPuzzle)
		error("Attempted puzzleDoMove2 but the circuit puzzle is gone");

	CircuitPuzzle::CellDirection actionDirection = CircuitPuzzle::kCellDirectionDown;
	Common::Point actionCoord;

	if (_circuitPuzzle->executeAIAction(*_rng, actionCoord, actionDirection)) {
		SoundInstance *snd = nullptr;
		StackInt_t soundID = 0;
		resolveSoundByName("85_block", true, soundID, snd);

		if (snd)
			triggerSound(kSoundLoopBehaviorNo, *snd, 0, 0, false, false);

		const CircuitPuzzle::CellRectSpec *rectSpec = _circuitPuzzle->getCellRectSpec(actionCoord);

		if (rectSpec) {
			AnimationDef animDef = _circuitPuzzleBlockAnimation;
			animDef.constraintRect = (actionDirection == CircuitPuzzle::kCellDirectionDown) ? rectSpec->_downBarrierRect : rectSpec->_rightBarrierRect;

			changeAnimation(animDef, false);

			_gameState = kGameStateWaitingForAnimation;
		}
	}
}

void Runtime::scriptOpPuzzleDone(ScriptArg_t arg) {
	_circuitPuzzle.reset();
}

// Only used in fnRandomBirds and fnRandomMachines in Room 60, both of which are unused
OPCODE_STUB(SndAddRandom)
OPCODE_STUB(SndClearRandom)

// Only used in Room 02 (cheat room, which isn't supported)
OPCODE_STUB(Speech)
OPCODE_STUB(Say)
OPCODE_STUB(Garbage)

// Referenced in Room 30 screen 0a4 interaction 0a0, however there is no interaction with that ID,
// so this is unreachable.
OPCODE_STUB(Fn)

#undef TAKE_STACK_STR
#undef TAKE_STACK_STR_NAMED
#undef TAKE_STACK_INT
#undef TAKE_STACK_INT_NAMED
#undef TAKE_STACK_VAR
#undef TAKE_STACK_VAR_NAMED
#undef PEEK_STACK
#undef OPCODE_STUB

} // End of namespace VCruise
