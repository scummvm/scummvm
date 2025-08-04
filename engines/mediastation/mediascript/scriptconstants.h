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

#ifndef MEDIASTATION_MEDIASCRIPT_BUILTINS_H
#define MEDIASTATION_MEDIASCRIPT_BUILTINS_H

namespace MediaStation {

enum ExpressionType {
	kExpressionTypeEmpty = 0x0000,
	kExpressionTypeVariable = 0x0065,
	kExpressionTypeValue = 0x0066,
	kExpressionTypeOperation = 0x0067,
};
const char *expressionTypeToStr(ExpressionType type);

enum Opcode {
	kOpcodeIf = 0xC9,
	kOpcodeIfElse = 0xCA,
	kOpcodeAssignVariable = 0xCB,
	kOpcodeOr = 0xCC,
	kOpcodeXor = 0xCD,
	kOpcodeAnd = 0xCE,
	kOpcodeEquals = 0xCF,
	kOpcodeNotEquals = 0xD0,
	kOpcodeLessThan = 0xD1,
	kOpcodeGreaterThan = 0xD2,
	kOpcodeLessThanOrEqualTo = 0xD3,
	kOpcodeGreaterThanOrEqualTo = 0xD4,
	kOpcodeAdd = 0xD5,
	kOpcodeSubtract = 0xD6,
	kOpcodeMultiply = 0xD7,
	kOpcodeDivide = 0xD8,
	kOpcodeModulo = 0xD9,
	kOpcodeNegate = 0xDA,
	kOpcodeCallFunction = 0xDB,
	kOpcodeCallMethod = 0xDC,
	kOpcodeDeclareLocals = 0xDD,
	kOpcodeReturn = 0xDE,
	kOpcodeReturnNoValue = 0xDF,
	kOpcodeWhile = 0xE0,
	kOpcodeCallFunctionInVariable = 0xE1, // IndirectCall
	kOpcodeCallMethodInVariable = 0xE2 // IndirectMsg
};
const char *opcodeToStr(Opcode opcode);

enum VariableScope {
	kVariableScopeLocal = 0x1,
	kVariableScopeParameter = 0x2,
	kVariableScopeIndirectParameter = 0x3,
	kVariableScopeGlobal = 0x4
};
const char *variableScopeToStr(VariableScope scope);

enum BuiltInFunction {
	kUnk1Function = 0xA,
	// TODO: Figure out if effectTransitionOnSync = 13 is consistent across titles?
	kEffectTransitionFunction = 0xC,
	kEffectTransitionOnSyncFunction = 0xD,
	kDrawingFunction = 0x25,
	// TODO: Figure out if TimeOfDay = 101 is consistent across titles.
	kDebugPrintFunction = 0xB4,
	// TODO: Figure out code for DebugPrint.
	// TODO: Figure out code for Quit.
};
const char *builtInFunctionToStr(BuiltInFunction function);

enum BuiltInMethod {
	kInvalidMethod = 0,
	// TODO: What object types does CursorSet apply to?
	// Currently it's only in var_7be1_cursor_currentTool in
	// IBM/Crayola.
	kCursorSetMethod = 0xC8,

	// SPATIAL ENTITY METHODS.
	kSpatialHideMethod = 0xCB,
	kSpatialMoveToMethod = 0xCC,
	kSpatialMoveToByOffsetMethod = 0xCD,
	kSpatialZMoveToMethod = 0xD8,
	kSpatialShowMethod = 0xCA,
	kTimePlayMethod = 0xCE,
	kTimeStopMethod = 0xCF,
	kIsPlayingMethod = 0x174,
	kSetDissolveFactorMethod = 0xF1,
	kSpatialCenterMoveToMethod = 0xE6,
	kGetLeftXMethod = 0xE9,
	kGetTopYMethod = 0xEA,
	kGetWidthMethod = 0xEB,
	kGetHeightMethod = 0xEC,
	kGetCenterXMethod = 0xED,
	kGetCenterYMethod = 0xEE,
	kGetZCoordinateMethod = 0xEF,
	kIsPointInsideMethod = 0xF6,
	kGetMouseXOffsetMethod = 0x108,
	kGetMouseYOffsetMethod = 0x109,
	kIsVisibleMethod = 0x10D,
	kSetMousePositionMethod = 0x129,
	// It isn't clear what the difference is meant to be
	// between these two, as the code looks the same for both.
	kGetXScaleMethod1 = 0x16E,
	kGetXScaleMethod2 = 0x17E,
	kSetScaleMethod = 0x16F,
	kSetXScaleMethod = 0x17F,
	kGetYScaleMethod = 0x180,
	kSetYScaleMethod = 0x181,

	// HOTSPOT METHODS.
	// NOTE: IDs 0xD2 and 0xD3 seem to be double-assigned
	// between two hotspot methods and two stage methods.
	kMouseActivateMethod = 0xD2,
	kMouseDeactivateMethod = 0xD3,
	kTriggerAbsXPositionMethod = 0x141,
	kTriggerAbsYPositionMethod = 0x142,
	kIsActiveMethod = 0x173,

	// SPRITE METHODS.
	kMovieResetMethod = 0xDB,
	kSetCurrentClipMethod = 0xDC,
	kIncrementFrameMethod = 0xDD,
	kDecrementFrameMethod = 0xDE,
	kGetCurrentClipIdMethod = 0xF0,

	// STAGE METHODS.
	// NOTE: IDs 0xD2 and 0xD3 seem to be double-assigned
	// between two hotspot methods and two stage methods.
	kAddActorToStageMethod = 0xD2,
	kRemoveActorFromStageMethod = 0xD3,
	kSetWorldSpaceExtentMethod = 0x16B,
	kSetBoundsMethod = 0x11F,
	kStageSetSizeMethod = 0x16B,
	kStageGetWidthMethod = 0x16C,
	kStageGetHeightMethod = 0x16D,

	// CAMERA METHODS.
	kStopPanMethod = 0x15E,
	kViewportMoveToMethod = 0x160,
	kYViewportPositionMethod = 0x165,
	kPanToMethod = 0x172,

	// CANVAS METHODS.
	kClearToPaletteMethod = 0x17B,

	// DOCUMENT METHODS.
	kLoadContextMethod = 0x176,
	kReleaseContextMethod = 0x177,
	kBranchToScreenMethod = 0xC9,
	kIsLoadedMethod = 0x178,

	// PATH METHODS.
	kSetDurationMethod = 0x106,
	kPercentCompleteMethod = 0x107,

	// TEXT METHODS.
	kTextMethod = 0x122,
	kSetTextMethod = 0x123,
	kSetMaximumTextLengthMethod = 0x125,

	// COLLECTION METHODS.
	// These are arrays used in Media Script.
	kAppendMethod = 0xF7,
	kApplyMethod = 0xF8,
	kCountMethod = 0xF9,
	kDeleteFirstMethod = 0xFA,
	kDeleteLastMethod = 0xFB,
	kEmptyMethod = 0xFC,
	kGetAtMethod = 0xFD,
	kIsEmptyMethod = 0xFE,
	kJumbleMethod = 0xFF,
	kSeekMethod = 0x100,
	kSendMethod = 0x101,
	kDeleteAtMethod = 0x102,
	kInsertAtMethod = 0x103,
	kReplaceAtMethod = 0x104,
	kPrependListMethod = 0x105,
	kSortMethod = 0x10A,

	// PRINTER METHODS.
	kOpenLensMethod = 0x15A,
	kCloseLensMethod = 0x15B,
};
const char *builtInMethodToStr(BuiltInMethod method);

enum EventType {
	// TIMER EVENTS.
	kTimerEvent = 0x5,

	// HOTSPOT EVENTS.
	kMouseDownEvent = 0x6,
	kMouseUpEvent = 0x7,
	kMouseMovedEvent = 0x8,
	kMouseEnteredEvent = 0x9,
	kMouseExitedEvent = 0xA,
	kKeyDownEvent = 0xD,

	// SOUND EVENTS.
	kSoundEndEvent = 0xE,
	kSoundAbortEvent = 0x13,
	kSoundFailureEvent = 0x14,
	kSoundStoppedEvent = 0x1D,
	kSoundBeginEvent = 0x1E,

	// MOVIE EVENTS.
	kMovieEndEvent = 0xF,
	kMovieAbortEvent = 0x15,
	kMovieFailureEvent = 0x16,
	kMovieStoppedEvent = 0x1F,
	kMovieBeginEvent = 0x20,

	// SPRITE EVENTS.
	// Just "MovieEnd" in source.
	kSpriteMovieEndEvent = 0x17,

	// SCREEN EVENTS.
	kEntryEvent = 0x11,
	kExitEvent = 0x1B,

	// CONTEXT EVENTS.
	kLoadCompleteEvent = 0x2C,

	// TEXT EVENTS.
	kInputEvent = 0x25,
	kErrorEvent = 0x26,

	// CAMERA EVENTS.
	kPanAbortEvent = 0x2B,
	kPanEndEvent = 0x2A,

	// PATH EVENTS.
	kStepEvent = 0x1C,
	kPathStoppedEvent = 0x21,
	kPathEndEvent = 0x10
};
const char *eventTypeToStr(EventType type);

enum OperandType {
	kOperandTypeEmpty = 0x0,
	kOperandTypeBool = 0x97,
	kOperandTypeFloat = 0x98,
	kOperandTypeInt = 0x99,
	kOperandTypeString = 0x9A,
	kOperandTypeParamToken = 0x9B,
	kOperandTypeActorId = 0x9C,
	kOperandTypeTime = 0x9D,
	kOperandTypeVariable = 0x9E,
	kOperandTypeFunctionId = 0x9F,
	kOperandTypeMethodId = 0xA0,
	kOperandTypeCollection = 0xA1
};
const char *operandTypeToStr(OperandType type);

enum ScriptValueType {
	kScriptValueTypeEmpty = 0x0,
	kScriptValueTypeFloat = 0x1,
	kScriptValueTypeBool = 0x2,
	kScriptValueTypeTime = 0x3,
	kScriptValueTypeParamToken = 0x4,
	kScriptValueTypeActorId = 0x5,
	kScriptValueTypeString = 0x6,
	kScriptValueTypeCollection = 0x7,
	kScriptValueTypeFunctionId = 0x8,
	kScriptValueTypeMethodId = 0x9
};
const char *scriptValueTypeToStr(ScriptValueType type);

} // End of namespace MediaStation

#endif
