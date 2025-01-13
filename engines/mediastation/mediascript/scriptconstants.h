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

enum InstructionType {
	kInstructionTypeEmpty = 0x0000,
	kInstructionTypeFunctionCall = 0x0067,
	kInstructionTypeOperand = 0x0066,
	kInstructionTypeVariableRef = 0x0065
};

enum Opcode {
	kOpcodeIfElse = 202,
	kOpcodeAssignVariable = 203,
	kOpcodeOr = 204,
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
	kOpcodeUnk2 = 218, // TODO: Likely something with ## constants like ##DOWN?
	kOpcodeCallRoutine = 219,
	// Method calls are like routine calls, but they have an implicit "self"
	// parameter that is always the first. For example:
	//  @self . mouseActivate ( TRUE ) ;
	kOpcodeCallMethod = 220,
	// This seems to appear at the start of a function to declare the number of
	// local variables used in the function. It seems to be the `Declare`
	// keyword. In the observed examples, the number of variables to create is
	// given, then the next instructions are variable assignments for that number
	// of variables.
	kOpcodeDeclareVariables = 221,
	kOpcodeWhile = 224,
	kOpcodeReturn = 222,
	kOpcodeUnk1 = 223
};

enum VariableScope {
	kVariableScopeLocal = 1,
	kVariableScopeParameter = 2,
	kVariableScopeGlobal = 4
};

enum BuiltInFunction {
	// TODO: Figure out if effectTransitionOnSync = 13 is consistent across titles?
	kEffectTransitionFunction = 12, // PARAMS: 1
	kDrawingFunction = 37, // PARAMS: 5
	// TODO: Figure out if TimeOfDay = 101 is consistent across titles.
	kDebugPrintFunction = 180, // PARAMS: 1+
	// TODO: Figure out code for DebugPrint.
	// TODO: Figure out code for Quit.
};

enum BuiltInMethod {
	// TODO: What object types does CursorSet apply to?
	// Currently it's only in var_7be1_cursor_currentTool in
	// IBM/Crayola.
	kCursorSetMethod = 200, // PARAMS: 0
	kSpatialHideMethod = 203, // PARAMS: 1
	kSpatialMoveToMethod = 204, // PARAMS: 2
	kSpatialZMoveToMethod = 216, // PARAMS: 1
	kSpatialShowMethod = 202, // PARAMS: 1
	kTimePlayMethod = 206, // PARAMS: 1
	kTimeStopMethod = 207, // PARAMS: 0
	kIsPlayingMethod = 372, // PARAMS: 0
	kSetDissolveFactorMethod = 241, // PARAMS: 1

	// HOTSPOT METHODS.
	kMouseActivateMethod = 210, // PARAMS: 1
	kMouseDeactivateMethod = 211, // PARAMS: 0
	kXPositionMethod = 233, // PARAMS: 0
	kYPositionMethod = 234, // PARAMS: 0
	kTriggerAbsXPositionMethod = 321, // PARAMS: 0
	kTriggerAbsYPositionMethod = 322, // PARAMS: 0
	kIsActiveMethod = 371, // PARAMS: 0

	// IMAGE METHODS.
	kWidthMethod = 235, // PARAMS: 0
	kHeightMethod = 236, // PARAMS: 0
	kIsVisibleMethod = 269,

	// SPRITE METHODS.
	kMovieResetMethod = 219, // PARAMS: 0

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
	// These aren't assets but arrays used in Media Script.
	kIsEmptyMethod = 254, // PARAMS: 0
	kEmptyMethod = 252, // PARAMS: 0
	kAppendMethod = 247, // PARAMS: 1+
	kGetAtMethod = 253, // PARAMS: 1
	kCountMethod = 249, // PARAMS: 0
	// Looks like this lets you call a method on all the items in a collection.
	// Examples look like : var_7be1_collect_shapes.send(spatialHide);
	kSendMethod = 257, // PARAMS: 1+. Looks like the first param is the function,
	// Seeking seems to be finding the index where a certain item is.
	// and the next params are any arguments you want to send.
	kSeekMethod = 256, // PARAMS: 1
	kSortMethod = 266, // PARAMS: 0
	kDeleteAtMethod = 258, // PARAMS: 1

	// PRINTER METHODS.
	kOpenLensMethod = 346, // PARAMS: 0
	kCloseLensMethod = 347, // PARAMS: 0
};

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

    //SPRITE EVENTS.
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

enum EventHandlerArgumentType {
    kNullEventHandlerArgument = 0,
    kAsciiCodeEventHandlerArgument = 1, // TODO: Why is this datum type a float?
    kTimeEventHandlerArgument = 3,
    kUnk1EventHandlerArgument = 4, // Appars to happen with MovieStart?
    kContextEventHandlerArgument = 5
};

enum OperandType {
	kOperandTypeEmpty = 0, // a flag for C++ code, not real operand type.
	// TODO: Figure out the difference between these two.
	kOperandTypeLiteral1 = 151,
	kOperandTypeLiteral2 = 153,
	// TODO: Figure out the difference between these two.
	kOperandTypeFloat1 = 152,
	kOperandTypeFloat2 = 157,
	kOperandTypeString = 154,
	// TODO: This only seems to be used in effectTransition:
	//  effectTransition ( $FadeToPalette )
	// compiles to:
	//  [219, 102, 1]
	//  [155, 301]
	kOperandTypeDollarSignVariable = 155,
	kOperandTypeAssetId = 156,
	kOperandTypeVariableDeclaration = 158,
	kOperandTypeFunction = 160
};

enum VariableType {
	// This is an invalid type used for initialization only.
	kVariableTypeEmpty = 0x0000,

	// This is an "array", but the IMT sources
	// use the term "collection".
	kVariableTypeCollection = 0x0007,
	kVariableTypeString = 0x0006,
	kVariableTypeAssetId = 0x0005,
	// These seem to be used in Dalmatians, but I don't know what they are
	// used for.
	kVariableTypeUnk1 = 0x0004,
	// These seem to be constants of some sort? This is what some of these
	// IDs look like in PROFILE._ST:
	//  - $downEar 10026
	//  - $sitDown 10027
	// Seems like these can also reference variables:
	//  - var_6c14_bool_FirstThingLev3 315
	//  - var_6c14_NextEncouragementSound 316
	kVariableTypeUnk2 = 0x0003,
	kVariableTypeBoolean = 0x0002,
	kVariableTypeLiteral = 0x0001
};

} // End of namespace MediaStation

#endif