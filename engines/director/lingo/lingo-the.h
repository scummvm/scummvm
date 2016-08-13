/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

 #ifndef DIRECTOR_LINGO_LINGO_THE_H
 #define DIRECTOR_LINGO_LINGO_THE_H

namespace Director {

enum TheEntityType {
	kTheNOEntity = 0,
	kTheFrame = 1,
	kTheFreeBlock,
	kTheFreeBytes,
	kThePathName,
	kTheMenu,
	kTheMenuItem,
	kTheMenuItems,
	kTheMenus,
	kTheMovie,
	kTheMouseH,
	kTheMouseV,
	kTheMouseDownScript,
	kTheMouseUpScript,

	kTheSprite,
	kTheCast,
	kThePerFrameHook,
	kTheTicks,
	kTheTimer,
	kTheTimeoutLength,
	kTheWindow,

	kTheClickOn,
	kTheDoubleClick,
	kTheLastClick,
	kTheLastFrame,
	kTheLastEvent,
	kTheMouseDown,
	kTheMouseUp,
	kTheRightMouseUp,
	kTheRightMouseDown,
	kTheStillDown,
	kTheKey,
	kTheKeyCode,
	kTheControlDown,
	kTheCommandDown,
	kTheShiftDown,
	kTheOptionDown,

	kTheColorDepth,
	kTheColorQD,
	kTheExitLock,
	kTheFloatPrecision,
	kTheItemDelimiter,
	kTheMultiSound,
	kThePreloadEventAbort,
	kTheRomanLingo,
	kTheStage
};

enum TheFieldType {
	kTheNOField = 0,
	kTheCastNum = 1,
	kTheCastType,
	kTheCheckMark,
	kTheController,
	kTheCursor,
	kTheDepth,
	kTheDirectToStage,
	kTheDrawRect,
	kTheLocH,
	kTheLocV,
	kTheBackColor,
	kTheBlend,
	kTheBottom,
	kTheConstraint,
	kTheEditableText,
	kTheEnabled,
	kTheForeColor,
	kTheFrameRate,
	kTheFilename,
	kTheHeight,
	kTheHilite,
	kTheInk,
	kTheLeft,
	kTheLineSize,
	kTheLoop,
	kTheLoaded,
	kTheModified,
	kTheMoveable,
	kTheMovieRate,
	kTheMovieTime,
	kTheNumber,
	kTheName,
	kThePalette,
	kThePausedAtStart,
	kThePicture,
	kThePreload,
	kThePurgePriority,
	kTheRect,
	kTheRegPoint,
	kTheRight,
	kTheStopTime,
	kTheStretch,
	kTheStartTime,
	kTheScript,
	kTheScriptNum,
	kTheScriptText,
	kTheSize,
	kTheStrech,
	kTheSound,
	kTheSourceRect,
	kTheText,
	kTheTop,
	kTheTrails,
	kTheType,
	kTheVisible,
	kTheVolume,
	kTheWidth
};

struct TheEntity {
	TheEntityType entity;
	const char *name;
	bool hasId;
};

struct TheEntityField {
	TheEntityType entity;
	const char *name;
	TheFieldType field;
};

} // End of namespace Director

#endif
