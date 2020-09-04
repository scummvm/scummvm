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
	kTheObject = 1,
	kTheActorList = 2,
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
	kTheFrameLabel,
	kTheFramePalette,
	kTheFrameScript,
	kTheFrameTempo,
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
	kTheMaxInteger,
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
	kThePi,
	kThePreloadEventAbort,
	kThePreLoadRAM,
	kTheQuickTimePresent,
	kTheRandomSeed,
	kTheResult,
	kTheRightMouseDown,
	kTheRightMouseUp,
	kTheRomanLingo,
	kTheScummvmVersion,			// set the Director version via lingo in tests
	kTheSearchCurrentFolder,
	kTheSearchPath,
	kTheSelection,
	kTheSelEnd,
	kTheSelStart,
	kTheShiftDown,
	kTheSoundEntity,
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
	kTheTimeoutKeyDown,
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
	kTheWindowList,
	kTheWords,
	kTheMaxTheEntityType		// This must be always last
};

enum TheFieldType {
	kTheNOField = 0,
	kTheAbbr    = 1,
	kTheBackColor,
	kTheBlend,
	kTheBottom,
	kTheCastNum,
	kTheCastType,
	kTheCenter,
	kTheCheckMark,
	kTheConstraint,
	kTheController,
	kTheCrop,
	kTheCursor,
	kTheDepth,
	kTheDirectToStage,
	kTheDrawRect,
	kTheDuration,
	kTheEditableText,
	kTheEnabled,
	kTheFileName,
	kTheForeColor,
	kTheFrameRate,
	kTheHeight,
	kTheHilite,
	kTheImmediate,
	kTheInk,
	kTheLast,
	kTheLeft,
	kTheLineSize,
	kTheLoaded,
	kTheLoc,
	kTheLocH,
	kTheLocV,
	kTheLong,
	kTheLoop,
	kTheModal,
	kTheModified,
	kTheMoveableSprite,
	kTheMovieRate,
	kTheMovieTime,
	kTheName,
	kTheNumber,
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
	kTheScoreColor,
	kTheScript,
	kTheScriptNum,
	kTheScriptText,
	kTheShort,
	kTheSize,
	kTheSound,
	kTheSourceRect,
	kTheStartTime,
	kTheStopTime,
	kTheStrech,
	kTheStretch,
	kTheText,
	kTheTextAlign,
	kTheTextFont,
	kTheTextHeight,
	kTheTextSize,
	kTheTextStyle,
	kTheTitle,
	kTheTitleVisible,
	kTheTop,
	kTheTrails,
	kTheType,
	kTheVideo,
	kTheVisibility,
	kTheVisible,
	kTheVolume,
	kTheWidth,
	kTheWindowType,
	kTheMaxTheFieldType		// This must be always last
};

struct TheEntity {
	TheEntityType entity;
	const char *name;
	bool hasId;
	int version;
};

struct TheEntityField {
	TheEntityType entity;
	const char *name;
	TheFieldType field;
	int version;
};

} // End of namespace Director

#endif
