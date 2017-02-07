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
	kTheActorList = 1,
	kTheBeepOn,
	kTheButtonStyle,
	kTheCast,
	kTheCastMembers,
	kTheCenterStage,
	kTheChars,
	kTheCheckBoxAccess,
	kTheCheckBoxType,
	kTheClickLoc,
	kTheClickOn,
	kTheColorDepth,
	kTheColorQD,
	kTheCommandDown,
	kTheControlDown,
	kTheDate,
	kTheDoubleClick,
	kTheExitLock,
	kTheField,
	kTheFixStageSize,
	kTheFloatPrecision,
	kTheFrame,
	kTheFrameScript,
	kTheFreeBlock,
	kTheFreeBytes,
	kTheFullColorPermit,
	kTheImageDirect,
	kTheItemDelimiter,
	kTheItems,
	kTheKey,
	kTheKeyCode,
	kTheKeyDownScript,
	kTheKeyUpScript,
	kTheLabelList,
	kTheLastClick,
	kTheLastEvent,
	kTheLastFrame,
	kTheLastKey,
	kTheLastRoll,
	kTheLines,
	kTheMachineType,
	kTheMemorySize,
	kTheMenu,
	kTheMenuItem,
	kTheMenuItems,
	kTheMenus,
	kTheMouseCast,
	kTheMouseChar,
	kTheMouseDown,
	kTheMouseDownScript,
	kTheMouseH,
	kTheMouseItem,
	kTheMouseLine,
	kTheMouseUp,
	kTheMouseUpScript,
	kTheMouseV,
	kTheMouseWord,
	kTheMovie,
	kTheMovieFileFreeSize,
	kTheMovieFileSize,
	kTheMovieName,
	kTheMoviePath,
	kTheMultiSound,
	kTheOptionDown,
	kTheParamCount,
	kThePathName,
	kThePauseState,
	kThePerFrameHook,
	kThePreloadEventAbort,
	kThePreLoadRAM,
	kTheQuickTimePresent,
	kTheResult,
	kTheRightMouseDown,
	kTheRightMouseUp,
	kTheRomanLingo,
	kTheSearchCurrentFolder,
	kTheSearchPath,
	kTheSelection,
	kTheSelEnd,
	kTheSelStart,
	kTheShiftDown,
	kTheSoundEnabled,
	kTheSoundLevel,
	kTheSprite,
	kTheSqrt,
	kTheStage,
	kTheStageBottom,
	kTheStageColor,
	kTheStageLeft,
	kTheStageRight,
	kTheStageTop,
	kTheStillDown,
	kTheSwitchColorDepth,
	kTheTicks,
	kTheTime,
	kTheTimeoutKeydown,
	kTheTimeoutLapsed,
	kTheTimeoutLength,
	kTheTimeoutMouse,
	kTheTimeoutPlay,
	kTheTimeoutScript,
	kTheTimer,
	kTheTrace,
	kTheTraceLoad,
	kTheTraceLogFile,
	kTheUpdateMovieEnabled,
	kTheWindow,
	kTheWords
};

enum TheFieldType {
	kTheNOField = 0,
	kTheAbbr    = 1,
	kTheCastNum,
	kTheCastType,
	kTheCenter,
	kTheCheckMark,
	kTheController,
	kTheCrop,
	kTheCursor,
	kTheDepth,
	kTheDirectToStage,
	kTheDrawRect,
	kTheDuration,
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
	kThePreLoad,
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
	kTheVideo,
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
