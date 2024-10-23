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
	kTheCastLibs,
	kTheCastMembers,
	kTheCenterStage,
	kTheChars,
	kTheCheckBoxAccess,
	kTheCheckBoxType,
	kTheChunk,
	kTheClickLoc,
	kTheClickOn,
	kTheColorDepth,
	kTheColorQD,
	kTheCommandDown,
	kTheControlDown,
	kTheDate,
	kTheDeskTopRectList,
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
	kTheRollOver,
	kTheRomanLingo,
	kTheRunMode,
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
	kTheVideoForWindowsPresent,
	kTheWindow,
	kTheWindowList,
	kTheWords,
	kTheXtras,
	kTheMaxTheEntityType		// This must be always last
};

enum TheFieldType {
	kTheNOField = 0,
	kTheAbbr    = 1,
	kTheBackColor,
	kTheBlend,
	kTheBottom,
	kTheCastLibNum,
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
	kTheMember,
	kTheMemberNum,
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
	kTheScrollTop,
	kTheMaxTheFieldType		// This must be always last
};

struct TheEntity {
	TheEntityType entity;
	const char *name;
	bool hasId;
	int version;
	bool isFunction;
};

struct TheEntityField {
	TheEntityType entity;
	const char *name;
	TheFieldType field;
	int version;
};

} // End of namespace Director

#endif
