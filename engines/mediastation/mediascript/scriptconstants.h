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
	kOpcodeIf = 201,
	kOpcodeIfElse = 202,
	kOpcodeAssignVariable = 203,
	kOpcodeOr = 204,
	kOpcodeXor = 205,
	kOpcodeAnd = 206,
	kOpcodeEquals = 207,
	kOpcodeNotEquals = 208,
	kOpcodeLessThan = 209,
	kOpcodeGreaterThan = 210,
	kOpcodeLessThanOrEqualTo = 211,
	kOpcodeGreaterThanOrEqualTo = 212,
	kOpcodeAdd = 213,
	kOpcodeSubtract = 214,
	kOpcodeMultiply = 215,
	kOpcodeDivide = 216,
	kOpcodeModulo = 217,
	kOpcodeNegate = 218,
	kOpcodeCallFunction = 219,
	kOpcodeCallMethod = 220,
	kOpcodeDeclareLocals = 221,
	kOpcodeReturn = 222,
	kOpcodeReturnNoValue = 223,
	kOpcodeWhile = 224,
	kOpcodeCallFunctionInVariable = 225, // IndirectCall
	kOpcodeCallMethodInVariable = 226 // IndirectMsg
};
const char *opcodeToStr(Opcode opcode);

enum VariableScope {
	kVariableScopeLocal = 1,
	kVariableScopeParameter = 2,
	kVariableScopeIndirectParameter = 3,
	kVariableScopeGlobal = 4
};
const char *variableScopeToStr(VariableScope scope);

enum BuiltInFunction {
	kUnk1Function = 10,
	// TODO: Figure out if effectTransitionOnSync = 13 is consistent across titles?
	kEffectTransitionFunction = 12, // PARAMS: 1
	kEffectTransitionOnSyncFunction = 13,
	kDrawingFunction = 37, // PARAMS: 5
	// TODO: Figure out if TimeOfDay = 101 is consistent across titles.
	kDebugPrintFunction = 180, // PARAMS: 1+
	// TODO: Figure out code for DebugPrint.
	// TODO: Figure out code for Quit.
};
const char *builtInFunctionToStr(BuiltInFunction function);

enum BuiltInMethod {
	kInvalidMethod = 0,
	// TODO: What object types does CursorSet apply to?
	// Currently it's only in var_7be1_cursor_currentTool in
	// IBM/Crayola.
	kCursorSetMethod = 200, // PARAMS: 0

	// SPATIAL ENTITY METHODS.
	kSpatialHideMethod = 203, // PARAMS: 1
	kSpatialMoveToMethod = 204, // PARAMS: 2
	kSpatialMoveToByOffsetMethod = 205, // PARAMS: 2
	kSpatialZMoveToMethod = 216, // PARAMS: 1
	kSpatialShowMethod = 202, // PARAMS: 1
	kTimePlayMethod = 206, // PARAMS: 1
	kTimeStopMethod = 207, // PARAMS: 0
	kIsPlayingMethod = 372, // PARAMS: 0
	kSetDissolveFactorMethod = 241, // PARAMS: 1
	kSpatialCenterMoveToMethod = 230,
	kGetLeftXMethod = 233,
	kGetTopYMethod = 234,
	kGetWidthMethod = 235, // PARAMS: 0
	kGetHeightMethod = 236, // PARAMS: 0
	kGetCenterXMethod = 237,
	kGetCenterYMethod = 238,
	kGetZCoordinateMethod = 239,
	kIsPointInsideMethod = 246,
	kGetMouseXOffsetMethod = 264,
	kGetMouseYOffsetMethod = 265,
	kIsVisibleMethod = 269,

	// HOTSPOT METHODS.
	kMouseActivateMethod = 210, // PARAMS: 1
	kMouseDeactivateMethod = 211, // PARAMS: 0
	kTriggerAbsXPositionMethod = 321, // PARAMS: 0
	kTriggerAbsYPositionMethod = 322, // PARAMS: 0
	kIsActiveMethod = 371, // PARAMS: 0

	// SPRITE METHODS.
	kMovieResetMethod = 219, // PARAMS: 0
	kSetCurrentClipMethod = 220, // PARAMS: 1
	kIncrementFrameMethod = 221, // PARAMS: 0-1
	kDecrementFrameMethod = 222, // PARAMS: 0-1
	kGetCurrentClipIdMethod = 240, // PARAMS: 0

	// STAGE METHODS.
	kSetWorldSpaceExtentMethod = 363, // PARAMS: 2
	kSetBoundsMethod = 287, // PARAMS: 4

	// CAMERA METHODS.
	kStopPanMethod = 350, // PARAMS: 0
	kViewportMoveToMethod = 352, // PARAMS: 2
	kYViewportPositionMethod = 357, // PARAMS: 0
	kPanToMethod = 370, // PARAMS: 4

	// CANVAS METHODS.
	kClearToPaletteMethod = 379, // PARAMS: 1

	// DOCUMENT METHODS.
	kLoadContextMethod = 374, // PARAMS: 1
	kReleaseContextMethod = 375, // PARAMS: 1
	kBranchToScreenMethod = 201, // PARAMS: 1
	kIsLoadedMethod = 376, // PARAMS: 1

	// PATH METHODS.
	kSetDurationMethod = 262, // PARAMS: 1
	kPercentCompleteMethod = 263,

	// TEXT METHODS.
	kTextMethod = 290,
	kSetTextMethod = 291,
	kSetMaximumTextLengthMethod = 293, // PARAM: 1

	// COLLECTION METHODS.
	// These are arrays used in Media Script.
	kAppendMethod = 247, // PARAMS: 1+
	kApplyMethod = 248, // PARAMS: 1+
	kCountMethod = 249, // PARAMS: 0
	kDeleteFirstMethod = 250, // PARAMS: 0
	kDeleteLastMethod = 251, // PARAMS: 0
	kEmptyMethod = 252, // PARAMS: 0
	kGetAtMethod = 253, // PARAMS: 1
	kIsEmptyMethod = 254, // PARAMS: 0
	kJumbleMethod = 255, // PARAMS: 0
	kSeekMethod = 256, // PARAMS: 1
	kSendMethod = 257, // PARAMS: 1+
	kDeleteAtMethod = 258, // PARAMS: 1
	kInsertAtMethod = 259, // PARAMS: 2
	kReplaceAtMethod = 260, // PARAMS: 2
	kPrependListMethod = 261, // PARAMS: 1+
	kSortMethod = 266, // PARAMS: 0

	// PRINTER METHODS.
	kOpenLensMethod = 346, // PARAMS: 0
	kCloseLensMethod = 347, // PARAMS: 0
};
const char *builtInMethodToStr(BuiltInMethod method);

enum EventType {
	// TIMER EVENTS.
	kTimerEvent = 5,

	// HOTSPOT EVENTS.
	kMouseDownEvent = 6,
	kMouseUpEvent = 7,
	kMouseMovedEvent = 8,
	kMouseEnteredEvent = 9,
	kMouseExitedEvent = 10,
	kKeyDownEvent = 13, // PARAMS: 1 - ASCII code.

	// SOUND EVENTS.
	kSoundEndEvent = 14,
	kSoundAbortEvent = 19,
	kSoundFailureEvent = 20,
	kSoundStoppedEvent = 29,
	kSoundBeginEvent = 30,

	// MOVIE EVENTS.
	kMovieEndEvent = 15,
	kMovieAbortEvent = 21,
	kMovieFailureEvent = 22,
	kMovieStoppedEvent = 31,
	kMovieBeginEvent = 32,

	// SPRITE EVENTS.
	// Just "MovieEnd" in source.
	kSpriteMovieEndEvent = 23,

	// SCREEN EVENTS.
	kEntryEvent = 17,
	kExitEvent = 27,

	// CONTEXT EVENTS.
	kLoadCompleteEvent = 44, // PARAMS: 1 - Context ID

	// TEXT EVENTS.
	kInputEvent = 37,
	kErrorEvent = 38,

	// CAMERA EVENTS.
	kPanAbortEvent = 43,
	kPanEndEvent = 42,

	// PATH EVENTS.
	kStepEvent = 28,
	kPathStoppedEvent = 33,
	kPathEndEvent = 16
};
const char *eventTypeToStr(EventType type);

enum OperandType {
	kOperandTypeEmpty = 0,
	kOperandTypeBool = 151,
	kOperandTypeFloat = 152,
	kOperandTypeInt = 153,
	kOperandTypeString = 154,
	kOperandTypeParamToken = 155,
	kOperandTypeAssetId = 156,
	kOperandTypeTime = 157,
	kOperandTypeVariable = 158,
	kOperandTypeFunctionId = 159,
	kOperandTypeMethodId = 160,
	kOperandTypeCollection = 161
};
const char *operandTypeToStr(OperandType type);

enum ScriptValueType {
	kScriptValueTypeEmpty = 0,
	kScriptValueTypeFloat = 1,
	kScriptValueTypeBool = 2,
	kScriptValueTypeTime = 3,
	kScriptValueTypeParamToken = 4,
	kScriptValueTypeAssetId = 5,
	kScriptValueTypeString = 6,
	kScriptValueTypeCollection = 7,
	kScriptValueTypeFunctionId = 8,
	kScriptValueTypeMethodId = 9
};
const char *scriptValueTypeToStr(ScriptValueType type);

} // End of namespace MediaStation

#endif
