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

const char *instructionTypeToStr(InstructionType type) {
    switch (type) {
    case kInstructionTypeEmpty:
        return "Empty";
    case kInstructionTypeFunctionCall:
        return "FunctionCall";
    case kInstructionTypeOperand:
        return "Operand";
    case kInstructionTypeVariableRef:
        return "VariableReference";
    default:
        return "UNKNOWN";
    }
}

const char *opcodeToStr(Opcode opcode) {
    switch (opcode) {
    case kOpcodeIfElse:
        return "IfElse";
    case kOpcodeAssignVariable:
        return "AssignVariable";
    case kOpcodeOr:
        return "Or";
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
    case kOpcodeCallRoutine:
        return "CallRoutine";
    case kOpcodeCallMethod:
        return "CallMethod";
    case kOpcodeDeclareVariables:
        return "DeclareVariables";
    case kOpcodeReturn:
        return "Return";
    case kOpcodeUnk1:
        return "UNKNOWN (Unk1)";
    case kOpcodeWhile:
        return "While";
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
    case kVariableScopeGlobal:
        return "Global";
    default:
        return "UNKNOWN";
    }
}

const char *builtInFunctionToStr(BuiltInFunction function) {
    switch (function) {
    case kEffectTransitionFunction:
        return "EffectTransition";
    case kDrawingFunction:
        return "Drawing";
    case kDebugPrintFunction:
        return "DebugPrint";
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
    case kSpatialZMoveToMethod:
        return "SpatialZMoveTo";
    case kSpatialShowMethod:
        return "SpatialShow";
    case kTimePlayMethod:
        return "TimePlay";
    case kTimeStopMethod:
        return "TimeStop";
    case kIsPlayingMethod:
        return "IsPlaying";
    case kSetDissolveFactorMethod:
        return "SetDissolveFactor";
    case kMouseActivateMethod:
        return "MouseActivate";
    case kMouseDeactivateMethod:
        return "MouseDeactivate";
    case kXPositionMethod:
        return "XPosition";
    case kYPositionMethod:
        return "YPosition";
    case kTriggerAbsXPositionMethod:
        return "TriggerAbsXPosition";
    case kTriggerAbsYPositionMethod:
        return "TriggerAbsYPosition";
    case kIsActiveMethod:
        return "IsActive";
    case kWidthMethod:
        return "Width";
    case kHeightMethod:
        return "Height";
    case kIsVisibleMethod:
        return "IsVisible";
    case kMovieResetMethod:
        return "MovieReset";
    case kSetWorldSpaceExtentMethod:
        return "SetWorldSpaceExtent";
    case kSetBoundsMethod:
        return "SetBounds";
    case kStopPanMethod:
        return "StopPan";
    case kViewportMoveToMethod:
        return "ViewportMoveTo";
    case kYViewportPositionMethod:
        return "YViewportPosition";
    case kPanToMethod:
        return "PanTo";
    case kClearToPaletteMethod:
        return "ClearToPalette";
    case kLoadContextMethod:
        return "LoadContext";
    case kReleaseContextMethod:
        return "ReleaseContext";
    case kBranchToScreenMethod:
        return "BranchToScreen";
    case kIsLoadedMethod:
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
    case kIsEmptyMethod:
        return "IsEmpty";
    case kEmptyMethod:
        return "Empty";
    case kAppendMethod:
        return "Append";
    case kGetAtMethod:
        return "GetAt";
    case kCountMethod:
        return "Count";
    case kSendMethod:
        return "Send";
    case kSeekMethod:
        return "Seek";
    case kSortMethod:
        return "Sort";
    case kDeleteAtMethod:
        return "DeleteAt";
    case kOpenLensMethod:
        return "OpenLens";
    case kCloseLensMethod:
        return "CloseLens";
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
    case kEntryEvent:
        return "EntryEvent";
    case kExitEvent:
        return "ExitEvent";
    case kLoadCompleteEvent:
        return "LoadComplete";
    case kInputEvent:
        return "Input";
    case kErrorEvent:
        return "Error";
    case kPanAbortEvent:
        return "PanAbort";
    case kPanEndEvent:
        return "PanEnd";
    case kStepEvent:
        return "StepEvent";
    case kPathStoppedEvent:
        return "PathStopped";
    case kPathEndEvent:
        return "PathEnd";
    default:
        return "UNKNOWN";
    }
}

const char *eventHandlerArgumentTypeToStr(EventHandlerArgumentType type) {
    switch (type) {
    case kNullEventHandlerArgument:
        return "Null";
    case kAsciiCodeEventHandlerArgument:
        return "AsciiCode";
    case kTimeEventHandlerArgument:
        return "Time";
    case kUnk1EventHandlerArgument:
        return "Unk1";
    case kContextEventHandlerArgument:
        return "Context";
    default: 
        return "UNKNOWN";
    }
}

const char *operandTypeToStr(OperandType type) {
    switch (type) {
    case kOperandTypeEmpty:
        return "Empty";
    case kOperandTypeLiteral1:
        return "Literal1";
    case kOperandTypeLiteral2:
        return "Literal2";
    case kOperandTypeFloat1:
        return "Float1";
    case kOperandTypeFloat2:
        return "Float2";
    case kOperandTypeString:
        return "String";
    case kOperandTypeDollarSignVariable:
        return "DollarSignVariable";
    case kOperandTypeAssetId:
        return "AssetId";
    case kOperandTypeVariableDeclaration:
        return "VariableDeclaration";
    case kOperandTypeFunction:
        return "Function";
    default:
        return "UNKNOWN";
    }
}

const char *variableTypeToStr(VariableType type) {
    switch (type) {
    case kVariableTypeEmpty:
        return "Empty";
    case kVariableTypeCollection:
        return "Collection";
    case kVariableTypeString:
        return "String";
    case kVariableTypeAssetId:
        return "AssetId";
    case kVariableTypeInt:
        return "Unknown1";
    case kVariableTypeUnk2:
        return "Unknown2";
    case kVariableTypeBoolean:
        return "Boolean";
    case kVariableTypeFloat:
        return "Literal";
    default:
        return "UNKNOWN";
    }
}

} // End of namespace MediaStation
