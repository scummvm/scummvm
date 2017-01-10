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
	kTheDate = 1,
	kTheFrame,
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
	kTheTime,

	kTheField,

	kTheChars,
	kTheItems,
	kTheLines,
	kTheWords,

	kTheSprite,
	kTheCast,
	kTheCastMembers,
	kThePerFrameHook,
	kTheTicks,
	kTheTimer,
	kTheTimeoutKeydown,
	kTheTimeoutLapsed,
	kTheTimeoutLength,
	kTheTimeoutMouse,
	kTheTimeoutPlay,
	kTheTimeoutScript,
	kTheWindow,

	kTheBeepOn,
	kTheButtonStyle,
	kTheCenterStage,
	kTheCheckBoxAccess,
	kTheCheckBoxType,
	kTheClickOn,
	kTheControlDown,
	kTheCommandDown,
	kTheDoubleClick,
	kTheFixStageSize,
	kTheFullColorPermit,
	kTheImageDirect,
	kTheKey,
	kTheKeyDownScript,
	kTheKeyCode,
	kTheLabelList,
	kTheLastClick,
	kTheLastEvent,
	kTheLastFrame,
	kTheLastKey,
	kTheLastRoll,
	kTheMachineType,
	kTheMemorySize,
	kTheMouseCast,
	kTheMouseChar,
	kTheMouseDown,
	kTheMouseItem,
	kTheMouseLine,
	kTheMouseUp,
	kTheMouseWord,
	kTheOptionDown,
	kThePauseState,
	kTheRightMouseUp,
	kTheRightMouseDown,
	kTheSoundEnabled,
	kTheSoundLevel,
	kTheStillDown,
	kTheSwitchColorDepth,
	kTheResult,
	kTheSelection,
	kTheSelEnd,
	kTheSelStart,
	kTheShiftDown,
	kTheSqrt,

	kTheColorDepth,
	kTheColorQD,
	kTheExitLock,
	kTheFloatPrecision,
	kTheItemDelimiter,
	kTheMultiSound,
	kThePreloadEventAbort,
	kTheRomanLingo,
	kTheStage,
	kTheStageBottom,
	kTheStageColor,
	kTheStageLeft,
	kTheStageRight,
	kTheStageTop
};

enum TheFieldType {
	kTheNOField = 0,
	kTheAbbr    = 1,
	kTheCastNum,
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
	kTheImmediate,
	kTheInk,
	kTheLeft,
	kTheLineSize,
	kTheLong,
	kTheLoop,
	kTheLoaded,
	kTheModified,
	kTheMoveable,
	kTheMovieRate,
	kTheMovieTime,
	kTheNumber,
	kTheName,
	kThePalette,
	kThePattern,
	kThePausedAtStart,
	kThePicture,
	kThePreload,
	kThePuppet,
	kThePurgePriority,
	kTheRect,
	kTheRegPoint,
	kTheRight,
	kTheShort,
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
	kTheTextAlign,
	kTheTextFont,
	kTheTextheight,
	kTheTextSize,
	kTheTextStyle,
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
