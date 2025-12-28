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

#include "mediastation/mediascript/scriptconstants.h"

namespace MediaStation {

const char *expressionTypeToStr(ExpressionType type) {
	switch (type) {
	case kExpressionTypeEmpty:
		return "Empty";
	case kExpressionTypeVariable:
		return "Variable";
	case kExpressionTypeValue:
		return "Value";
	case kExpressionTypeOperation:
		return "Operation";
	default:
		return "UNKNOWN";
	}
}

const char *opcodeToStr(Opcode opcode) {
	switch (opcode) {
	case kOpcodeIf:
		return "If";
	case kOpcodeIfElse:
		return "IfElse";
	case kOpcodeAssignVariable:
		return "AssignVariable";
	case kOpcodeOr:
		return "Or";
	case kOpcodeXor:
		return "Xor";
	case kOpcodeAnd:
		return "And";
	case kOpcodeEquals:
		return "==";
	case kOpcodeNotEquals:
		return "!=";
	case kOpcodeLessThan:
		return "<";
	case kOpcodeGreaterThan:
		return ">";
	case kOpcodeLessThanOrEqualTo:
		return "<=";
	case kOpcodeGreaterThanOrEqualTo:
		return ">=";
	case kOpcodeAdd:
		return "+";
	case kOpcodeSubtract:
		return "-";
	case kOpcodeMultiply:
		return "*";
	case kOpcodeDivide:
		return "/";
	case kOpcodeModulo:
		return "%";
	case kOpcodeNegate:
		return "-";
	case kOpcodeCallFunction:
		return "CallFunction";
	case kOpcodeCallMethod:
		return "CallMethod";
	case kOpcodeDeclareLocals:
		return "DeclareLocals";
	case kOpcodeReturn:
		return "Return";
	case kOpcodeReturnNoValue:
		return "ReturnNoValue";
	case kOpcodeWhile:
		return "While";
	case kOpcodeCallFunctionInVariable:
		return "CallFunctionInVariable";
	case kOpcodeCallMethodInVariable:
		return "CallMethodInVariable";
	default:
		return "UNKNOWN";
	}
}

const char *variableScopeToStr(VariableScope scope) {
	switch (scope) {
	case kVariableScopeLocal:
		return "Local";
	case kVariableScopeParameter:
		return "Parameter";
	case kVariableScopeIndirectParameter:
		return "IndirectParameter";
	case kVariableScopeGlobal:
		return "Global";
	default:
		return "UNKNOWN";
	}
}

const char *builtInFunctionToStr(BuiltInFunction function) {
	switch (function) {
	case kRandomFunction:
		return "Random";
	case kTimeOfDayFunction:
		return "TimeOfDay";
	case kEffectTransitionFunction:
		return "EffectTransition";
	case kEffectTransitionOnSyncFunction:
		return "EffectTransitionOnSync";
	case kPlatformFunction:
		return "Platform";
	case kSquareRootFunction:
		return "SquareRoot";
	case kGetUniqueRandomFunction:
		return "GetUniqueRandom";
	case kCurrentRunTimeFunction:
		return "CurrentRunTime";
	case kSetGammaCorrectionFunction:
		return "SetGammaCorrection";
	case kGetDefaultGammaCorrectionFunction:
		return "GetDefaultGammaCorrection";
	case kGetCurrentGammaCorrectionFunction:
		return "GetCurrentGammaCorrection";
	case kSetAudioVolumeFunction:
		return "SetAudioVolume";
	case kGetAudioVolumeFunction:
		return "GetAudioVolume";
	case kSystemLanguagePreferenceFunction:
		return "SystemLanguagePreference";
	case kSetRegistryFunction:
		return "SetRegistry";
	case kGetRegistryFunction:
		return "GetRegistry";
	case kSetProfileFunction:
		return "SetProfile";
	case kMazeGenerateFunction:
		return "MazeGenerate";
	case kMazeApplyMoveMaskFunction:
		return "MazeApplyMoveMask";
	case kMazeSolveFunction:
		return "MazeSolve";
	case kBeginTimedIntervalFunction:
		return "BeginTimedInterval";
	case kEndTimedIntervalFunction:
		return "EndTimedInterval";
	case kDrawingFunction:
		return "Drawing";
	case kLegacy_RandomFunction:
		return "Legacy Random";
	case kLegacy_TimeOfDayFunction:
		return "Legacy TimeOfDay";
	case kLegacy_EffectTransitionFunction:
		return "Legacy EffectTransition";
	case kLegacy_EffectTransitionOnSyncFunction:
		return "Legacy EffectTransitionOnSync";
	case kLegacy_PlatformFunction:
		return "Legacy Platform";
	case kLegacy_SquareRootFunction:
		return "Legacy SquareRoot";
	case kLegacy_GetUniqueRandomFunction:
		return "Legacy GetUniqueRandom";
	case kLegacy_DebugPrintFunction:
		return "DebugPrint";
	case kLegacy_SystemLanguagePreferenceFunction:
		return "Legacy SystemLanguagePreference";
	default:
		return "UNKNOWN";
	}
}

const char *builtInMethodToStr(BuiltInMethod method) {
	switch (method) {
	case kCursorSetMethod:
		return "CursorSet";
	case kSpatialHideMethod:
		return "SpatialHide";
	case kSpatialMoveToMethod:
		return "SpatialMoveTo";
	case kSpatialMoveToByOffsetMethod:
		return "SpatialMoveToByOffset";
	case kSpatialZMoveToMethod:
		return "SpatialZMoveTo";
	case kSpatialCenterMoveToMethod:
		return "SpatialCenterMoveTo";
	case kSpatialShowMethod:
		return "SpatialShow";
	case kTimePlayMethod:
		return "TimePlay";
	case kTimeStopMethod:
		return "TimeStop";
	case kIsPlayingMethod:
		return "IsPlaying/SetMultipleStreams";
	case kSetDissolveFactorMethod:
		return "SetDissolveFactor";
	case kMouseActivateMethod:
		return "MouseActivate";
	case kMouseDeactivateMethod:
		return "MouseDeactivate";
	case kGetLeftXMethod:
		return "GetLeftX";
	case kGetTopYMethod:
		return "GetTopY";
	case kTriggerAbsXPositionMethod:
		return "TriggerAbsXPosition";
	case kTriggerAbsYPositionMethod:
		return "TriggerAbsYPosition";
	case kIsActiveMethod:
		return "IsActive";
	case kGetWidthMethod:
		return "GetWidth";
	case kGetHeightMethod:
		return "GetHeight";
	case kGetCenterXMethod:
		return "GetCenterX";
	case kGetCenterYMethod:
		return "GetCenterY";
	case kGetZCoordinateMethod:
		return "GetZCoordinate";
	case kIsPointInsideMethod:
		return "IsPointInside";
	case kGetMouseXOffsetMethod:
		return "GetMouseXOffset";
	case kGetMouseYOffsetMethod:
		return "GetMouseYOffset";
	case kIsVisibleMethod:
		return "IsVisible";
	case kSetMousePositionMethod:
		return "SetMousePosition";
	case kGetXScaleMethod1:
	case kGetXScaleMethod2:
		return "GetXScale";
	case kSetScaleMethod:
		return "SetScale";
	case kSetXScaleMethod:
		return "SetXScale";
	case kGetYScaleMethod:
		return "GetYScale";
	case kSetYScaleMethod:
		return "SetYScale";
	case kMovieResetMethod:
		return "MovieReset";
	case kSetCurrentClipMethod:
		return "SetCurrentClip";
	case kIncrementFrameMethod:
		return "IncrementFrame";
	case kDecrementFrameMethod:
		return "DecrementFrame";
	case kGetCurrentClipIdMethod:
		return "GetCurrentClipId";
	case kSetWorldSpaceExtentMethod:
		return "SetWorldSpaceExtent";
	case kSetBoundsMethod:
		return "SetBounds";
	case kStageGetWidthMethod:
		return "StageGetWidth";
	case kStageGetHeightMethod:
		return "StageGetHeight";
	case kAddToStageMethod:
		return "AddToStage\\OpenLens";
	case kRemoveFromStageMethod:
		return "RemoveFromStage\\CloseLens";
	case kAddedToStageMethod:
		return "AddedToStage";
	case kStartPanMethod:
		return "StartPan";
	case kStopPanMethod:
		return "StopPan";
	case kIsPanningMethod:
		return "IsPanning";
	case kViewportMoveToMethod:
		return "ViewportMoveTo";
	case kAdjustCameraViewportMethod:
		return "AdjustCameraViewport";
	case kAdjustCameraViewportSpatialCenterMethod:
		return "AdjustCameraViewportSpatialCenter";
	case kSetCameraBoundsMethod:
		return "SetCameraBounds";
	case kXViewportPositionMethod:
		return "XViewportPosition";
	case kYViewportPositionMethod:
		return "YViewportPosition";
	case kPanToMethod:
		return "PanTo";
	case kClearToPaletteMethod:
		return "ClearToPalette";
	case kDocumentLoadContextMethod:
		return "LoadContext";
	case kDocumentReleaseContextMethod:
		return "ReleaseContext";
	case kDocumentBranchToScreenMethod:
		return "BranchToScreen";
	case kDocumentQuitMethod:
		return "Quit";
	case kDocumentContextLoadInProgressMethod:
		return "ContextLoadInProgress";
	case kDocumentSetMultipleSoundsMethod:
		return "SetMultipleSounds";
	case kDocumentContextIsLoadedMethod:
		return "IsLoaded";
	case kSetDurationMethod:
		return "SetDuration";
	case kPercentCompleteMethod:
		return "PercentComplete";
	case kTextMethod:
		return "Text";
	case kSetTextMethod:
		return "SetText";
	case kSetMaximumTextLengthMethod:
		return "SetMaximumTextLength";
	case kAppendMethod:
		return "Append";
	case kApplyMethod:
		return "Apply";
	case kCountMethod:
		return "Count";
	case kDeleteFirstMethod:
		return "DeleteFirst";
	case kDeleteLastMethod:
		return "DeleteLast";
	case kEmptyMethod:
		return "Empty";
	case kGetAtMethod:
		return "GetAt";
	case kIsEmptyMethod:
		return "IsEmpty";
	case kJumbleMethod:
		return "Jumble";
	case kSeekMethod:
		return "Seek";
	case kSendMethod:
		return "Send";
	case kDeleteAtMethod:
		return "DeleteAt";
	case kInsertAtMethod:
		return "InsertAt";
	case kReplaceAtMethod:
		return "ReplaceAt";
	case kPrependListMethod:
		return "PrependList";
	case kSortMethod:
		return "Sort";
	default:
		return "UNKNOWN";
	}
}

const char *eventTypeToStr(EventType type) {
	switch (type) {
	case kTimerEvent:
		return "Timer";
	case kMouseDownEvent:
		return "MouseDown";
	case kMouseUpEvent:
		return "MouseUp";
	case kMouseMovedEvent:
		return "MouseMoved";
	case kMouseEnteredEvent:
		return "MouseEntered";
	case kMouseExitedEvent:
		return "MouseExited";
	case kKeyDownEvent:
		return "KeyDown";
	case kSoundEndEvent:
		return "SoundEnd";
	case kSoundAbortEvent:
		return "SoundAbort";
	case kSoundFailureEvent:
		return "SoundFailure";
	case kSoundStoppedEvent:
		return "SoundStopped";
	case kSoundBeginEvent:
		return "SoundBegin";
	case kMovieEndEvent:
		return "MovieEnd";
	case kMovieAbortEvent:
		return "MovieAbort";
	case kMovieFailureEvent:
		return "MovieFailure";
	case kMovieStoppedEvent:
		return "MovieStopped";
	case kMovieBeginEvent:
		return "MovieBegin";
	case kSpriteMovieEndEvent:
		return "SpriteMovieEnd";
	case kScreenEntryEvent:
		return "ScreenEntry";
	case kScreenExitEvent:
		return "ScreenExit";
	case kContextLoadCompleteEvent:
		return "ContextLoadComplete";
	case kContextLoadCompleteEvent2:
		return "ContextLoadComplete2";
	case kContextLoadAbortEvent:
		return "ContextLoadAbort";
	case kContextLoadFailureEvent:
		return "ContextLoadFailure";
	case kTextInputEvent:
		return "TextInput";
	case kTextErrorEvent:
		return "TextError";
	case kCameraPanStepEvent:
		return "CameraPanStep";
	case kCameraPanAbortEvent:
		return "CameraPanAbort";
	case kCameraPanEndEvent:
		return "CameraPanEnd";
	case kPathStepEvent:
		return "PathStep";
	case kPathStoppedEvent:
		return "PathStopped";
	case kPathEndEvent:
		return "PathEnd";
	default:
		return "UNKNOWN";
	}
}

const char *operandTypeToStr(OperandType type) {
	switch (type) {
	case kOperandTypeEmpty:
		return "Empty";
	case kOperandTypeBool:
		return "Bool";
	case kOperandTypeFloat:
		return "Float";
	case kOperandTypeInt:
		return "Int";
	case kOperandTypeString:
		return "String";
	case kOperandTypeParamToken:
		return "DollarSignVariable";
	case kOperandTypeActorId:
		return "ActorId";
	case kOperandTypeTime:
		return "Time";
	case kOperandTypeVariable:
		return "Variable";
	case kOperandTypeFunctionId:
		return "FunctionId";
	case kOperandTypeMethodId:
		return "MethodId";
	case kOperandTypeCollection:
		return "Collection";
	default:
		return "UNKNOWN";
	}
}

const char *scriptValueTypeToStr(ScriptValueType type) {
	switch (type) {
	case kScriptValueTypeEmpty:
		return "Empty";
	case kScriptValueTypeFloat:
		return "Float";
	case kScriptValueTypeBool:
		return "Bool";
	case kScriptValueTypeTime:
		return "Time";
	case kScriptValueTypeParamToken:
		return "Int";
	case kScriptValueTypeActorId:
		return "ActorId";
	case kScriptValueTypeString:
		return "String";
	case kScriptValueTypeCollection:
		return "Collection";
	case kScriptValueTypeFunctionId:
		return "FunctionId";
	case kScriptValueTypeMethodId:
		return "MethodId";
	default:
		return "UNKNOWN";
	}
}

} // End of namespace MediaStation
