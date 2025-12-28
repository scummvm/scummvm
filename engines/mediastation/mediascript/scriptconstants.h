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
	kRandomFunction = 0x0A,
	kTimeOfDayFunction = 0x0B,
	kEffectTransitionFunction = 0x0C,
	kEffectTransitionOnSyncFunction = 0x0D,
	kPlatformFunction = 0x0E,
	kSquareRootFunction = 0x0F,
	kGetUniqueRandomFunction = 0x10,
	kCurrentRunTimeFunction = 0x11,
	kSetGammaCorrectionFunction = 0x12,
	kGetDefaultGammaCorrectionFunction = 0x13,
	kGetCurrentGammaCorrectionFunction = 0x14,
	kSetAudioVolumeFunction = 0x17,
	kGetAudioVolumeFunction = 0x18,
	kSystemLanguagePreferenceFunction = 0x19,
	kSetRegistryFunction = 0x1A,
	kGetRegistryFunction = 0x1B,
	kSetProfileFunction = 0x1C,
	kMazeGenerateFunction = 0x1F,
	kMazeApplyMoveMaskFunction = 0x20,
	kMazeSolveFunction = 0x21,
	kBeginTimedIntervalFunction = 0x22,
	kEndTimedIntervalFunction = 0x23,
	kDrawingFunction = 0x25,

	// Early engine versions (like for Lion King and such), had different opcodes
	// for some functions, even though the functions were the same. So those are
	// defined here.
	kLegacy_RandomFunction = 0x64,
	kLegacy_TimeOfDayFunction = 0x65,
	kLegacy_EffectTransitionFunction = 0x66,
	kLegacy_EffectTransitionOnSyncFunction = 0x67,
	kLegacy_PlatformFunction = 0x68,
	kLegacy_SquareRootFunction = 0x69,
	kLegacy_GetUniqueRandomFunction = 0x6A,
	kLegacy_DebugPrintFunction = 0xB4,
	kLegacy_SystemLanguagePreferenceFunction = 0xC8,
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
	// NOTE: IDs 0x15A and 0x15B seem to be double-assigned
	// between two camera methods and two printer methods.
	kAddToStageMethod = 0x15A,
	kRemoveFromStageMethod = 0x15B,
	kAddedToStageMethod = 0x15C,
	kStartPanMethod = 0x15D,
	kStopPanMethod = 0x15E,
	kIsPanningMethod = 0x15F,
	kViewportMoveToMethod = 0x160,
	kAdjustCameraViewportMethod = 0x161,
	kAdjustCameraViewportSpatialCenterMethod = 0x162,
	kSetCameraBoundsMethod = 0x163,
	kXViewportPositionMethod = 0x164,
	kYViewportPositionMethod = 0x165,
	kPanToMethod = 0x172,

	// CANVAS METHODS.
	kClearToPaletteMethod = 0x17B,

	// DOCUMENT METHODS.
	kDocumentBranchToScreenMethod = 0xC9,
	kDocumentQuitMethod = 0xD9,
	kDocumentContextLoadInProgressMethod = 0x169,
	kDocumentSetMultipleStreamsMethod = 0x174,
	kDocumentSetMultipleSoundsMethod = 0x175,
	kDocumentLoadContextMethod = 0x176,
	kDocumentReleaseContextMethod = 0x177,
	kDocumentContextIsLoadedMethod = 0x178,

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
	// NOTE: IDs 0x15A and 0x15B seem to be double-assigned
	// between two camera methods and two printer methods.
	kOpenLensMethod = 0x15A,
	kCloseLensMethod = 0x15B,
};
const char *builtInMethodToStr(BuiltInMethod method);

enum EventType {
	kTimerEvent = 0x05,
	kMouseDownEvent = 0x06,
	kMouseUpEvent = 0x07,
	kMouseMovedEvent = 0x08,
	kMouseEnteredEvent = 0x09,
	kMouseExitedEvent = 0x0A,
	kKeyDownEvent = 0x0D,
	kSoundEndEvent = 0x0E,
	kMovieEndEvent = 0x0F,
	kPathEndEvent = 0x10,
	kScreenEntryEvent = 0x11,
	kSoundAbortEvent = 0x13,
	kSoundFailureEvent = 0x14,
	kMovieAbortEvent = 0x15,
	kMovieFailureEvent = 0x16,
	kSpriteMovieEndEvent = 0x17,
	kScreenExitEvent = 0x1B,
	kPathStepEvent = 0x1C,
	kSoundStoppedEvent = 0x1D,
	kSoundBeginEvent = 0x1E,
	kMovieStoppedEvent = 0x1F,
	kMovieBeginEvent = 0x20,
	kPathStoppedEvent = 0x21,
	kTextInputEvent = 0x25,
	kTextErrorEvent = 0x26,
	kCameraPanStepEvent = 0x29,
	kCameraPanEndEvent = 0x2A,
	kCameraPanAbortEvent = 0x2B,
	kContextLoadCompleteEvent = 0x2C,
	// TODO: These last 3 events appear as valid event types, but I haven't found
	// scripts that actually use them. So the names might be wrong.
	kContextLoadCompleteEvent2 = 0x2D,
	kContextLoadAbortEvent = 0x2E,
	kContextLoadFailureEvent = 0x2F,
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
