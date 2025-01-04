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

} // End of namespace MediaStation

#endif