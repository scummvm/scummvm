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

enum class BuiltInFunction {
	// TODO: Figure out if effectTransitionOnSync = 13 is consistent across titles?
	effectTransition = 12, // PARAMS: 1
	drawing = 37, // PARAMS: 5
	// TODO: Figure out if TimeOfDay = 101 is consistent across titles.
	DebugPrint = 180, // PARAMS: 1+
	// TODO: Figure out code for DebugPrint.
	// TODO: Figure out code for Quit.
};

enum class BuiltInMethod {
	// TODO: What object types does CursorSet apply to?
	// Currently it's only in var_7be1_cursor_currentTool in
	// IBM/Crayola.
	cursorSet = 200, // PARAMS: 0
	spatialHide = 203, // PARAMS: 1
	spatialMoveTo = 204, // PARAMS: 2
	spatialZMoveTo = 216, // PARAMS: 1
	spatialShow = 202, // PARAMS: 1
	timePlay = 206, // PARAMS: 1
	timeStop = 207, // PARAMS: 0
	isPlaying = 372, // PARAMS: 0
	setDissolveFactor = 241, // PARAMS: 1

	// HOTSPOT METHODS.
	mouseActivate = 210, // PARAMS: 1
	mouseDeactivate = 211, // PARAMS: 0
	xPosition = 233, // PARAMS: 0
	yPosiion = 234, // PARAMS: 0
	TriggerAbsXPosition = 321, // PARAMS: 0
	TriggerAbsYPosition = 322, // PARAMS: 0
	isActive = 371, // PARAMS: 0

	// IMAGE METHODS.
	Width = 235, // PARAMS: 0
	Height = 236, // PARAMS: 0
	isVisible = 269,

	// SPRITE METHODS.
	movieReset = 219, // PARAMS: 0

	// STAGE METHODS.
	setWorldSpaceExtent = 363, // PARAMS: 2
	setBounds = 287, // PARAMS: 4

	// CAMERA METHODS.
	stopPan = 350, // PARAMS: 0
	viewportMoveTo = 352, // PARAMS: 2
	yViewportPosition = 357, // PARAMS: 0
	panTo = 370, // PARAMS: 4

	// CANVAS METHODS.
	clearToPalette = 379, // PARAMS: 1

	// DOCUMENT METHODS.
	loadContext = 374, // PARAMS: 1
	releaseContext = 375, // PARAMS: 1
	branchToScreen = 201, // PARAMS: 1
	isLoaded = 376, // PARAMS: 1

	// PATH METHODS.
	setDuration = 262, // PARAMS: 1
	percentComplete = 263,

	// TEXT METHODS.
	text = 290,
	setText = 291,
	setMaximumTextLength = 293, // PARAM: 1

	// COLLECTION METHODS.
	// These aren't assets but arrays used in Media Script.
	isEmpty = 254, // PARAMS: 0
	empty = 252, // PARAMS: 0
	append = 247, // PARAMS: 1+
	getAt = 253, // PARAMS: 1
	count = 249, // PARAMS: 0
	// Looks like this lets you call a method on all the items in a collection.
	// Examples look like : var_7be1_collect_shapes.send(spatialHide);
	send = 257, // PARAMS: 1+. Looks like the first param is the function,
	// and the next params are any arguments you want to send.
	// Seeking seems to be finding the index where a certain item is.
	seek = 256, // PARAMS: 1
	sort = 266, // PARAMS: 0
	deleteAt = 258, // PARAMS: 1

	// PRINTER METHODS.
	openLens = 346, // PARAMS: 0
	closeLens = 347, // PARAMS: 0
};

} // End of namespace MediaStation

#endif