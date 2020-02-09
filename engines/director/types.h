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

#ifndef DIRECTOR_TYPES_H
#define DIRECTOR_TYPES_H

namespace Director {

enum CastType {
	kCastTypeNull = 0,
	kCastBitmap = 1,
	kCastFilmLoop = 2,
	kCastText = 3,
	kCastPalette = 4,
	kCastPicture = 5,
	kCastSound = 6,
	kCastButton = 7,
	kCastShape = 8,
	kCastMovie = 9,
	kCastDigitalVideo = 10,
	kCastLingoScript = 11,
	kCastRTE = 12
};

enum ScriptType {
	kNoneScript = -1,
	kMovieScript = 0,
	kSpriteScript = 1,
	kFrameScript = 2,
	kCastScript = 3,
	kGlobalScript = 4,
	kScoreScript = 5,
	kMaxScriptType = 5	// Sync with score.cpp:45, array scriptTypes[]
};

enum ShapeType {
	kShapeRectangle = 1,
	kShapeRoundRect = 2,
	kShapeOval = 3,
	kShapeLine = 4
};

enum TextType {
	kTextTypeAdjustToFit,
	kTextTypeScrolling,
	kTextTypeFixed
};

enum TextAlignType {
	kTextAlignRight = -1,
	kTextAlignLeft,
	kTextAlignCenter
};

enum TextFlag {
	kTextFlagEditable	= (1 << 0),
	kTextFlagAutoTab	= (1 << 1),
	kTextFlagDoNotWrap	= (1 << 2)
};

enum SizeType {
	kSizeNone,
	kSizeSmallest,
	kSizeSmall,
	kSizeMedium,
	kSizeLarge,
	kSizeLargest
};

enum ButtonType {
	kTypeButton,
	kTypeCheckBox,
	kTypeRadio
};

enum SpriteType {
	kInactiveSprite					= 0,	// turns the sprite off
	kBitmapSprite					= 1,
	kRectangleSprite				= 2,	// QuickDraw
	kRoundedRectangleSprite			= 3,	// QuickDraw
	kOvalSprite						= 4,	// QuickDraw
	kLineTopBottomSprite			= 5,	// line from top left to bottom right
	kLineBottomTopSprite			= 6,	// line from bottom left to top right
	kTextSprite						= 7,
	kButtonSprite					= 8,
	kCheckboxSprite					= 9,
	kRadioButtonSprite				= 10,
	kPictSprite						= 11,	// Cast picture
	kOutlinedRectangleSprite		= 12,	// QuickDraw
	kOutlinedRoundedRectangleSprite	= 13,	// QuickDraw
	kOutlinedOvalSprite				= 14,	// QuickDraw
	kThinkLineSprite				= 15,	// 2pt width line
	kCastMemberSprite				= 16,	// Specified by cast member
	kFilmLoopSpite					= 17,
	kDirMovieSprite					= 18
};

enum InkType {
	kInkTypeCopy,
	kInkTypeTransparent,
	kInkTypeReverse,
	kInkTypeGhost,
	kInkTypeNotCopy,
	kInkTypeNotTrans,
	kInkTypeNotReverse,
	kInkTypeNotGhost,
	kInkTypeMatte,
	kInkTypeMask,
	//10-31 Not used (Lingo in a Nutshell)
	kInkTypeBlend = 32,
	kInkTypeAddPin,
	kInkTypeAdd,
	kInkTypeSubPin,
	kInkTypeBackgndTrans,
	kInkTypeLight,
	kInkTypeSub,
	kInkTypeDark
};

enum LEvent {
	kEventPrepareMovie,
	kEventStartMovie,
	kEventStepMovie,
	kEventStopMovie,

	kEventNew,
	kEventBeginSprite,
	kEventEndSprite,

	kEventNone,
	kEventEnterFrame,
	kEventPrepareFrame,
	kEventIdle,
	kEventStepFrame,
	kEventExitFrame,
	kEventTimeout,

	kEventActivateWindow,
	kEventDeactivateWindow,
	kEventMoveWindow,
	kEventResizeWindow,
	kEventOpenWindow,
	kEventCloseWindow,

	kEventKeyUp,
	kEventKeyDown,
	kEventMouseUp,
	kEventMouseDown,
	kEventRightMouseUp,
	kEventRightMouseDown,
	kEventMouseEnter,
	kEventMouseLeave,
	kEventMouseUpOutSide,
	kEventMouseWithin,

	kEventStart
};

enum TransitionType {
	kTransNone,
	kTransWipeRight,
	kTransWipeLeft,
	kTransWipeDown,
	kTransWipeUp,
	kTransCenterOutHorizontal,
	kTransEdgesInHorizontal,
	kTransCenterOutVertical,
	kTransEdgesInVertical,
	kTransCenterOutSquare,
	kTransEdgesInSquare,
	kTransPushLeft,
	kTransPushRight,
	kTransPushDown,
	kTransPushUp,
	kTransRevealUp,
	kTransRevealUpRight,
	kTransRevealRight,
	kTransRevealDown,
	kTransRevealDownRight,
	kTransRevealDownLeft,
	kTransRevealLeft,
	kTransRevealUpLeft,
	kTransDissolvePixelsFast,
	kTransDissolveBoxyRects,
	kTransDissolveBoxySquares,
	kTransDissolvePatterns,
	kTransRandomRows,
	kTransRandomColumns,
	kTransCoverDown,
	kTransCoverDownLeft,
	kTransCoverDownRight,
	kTransCoverLeft,
	kTransCoverRight,
	kTransCoverUp,
	kTransCoverUpLeft,
	kTransCoverUpRight,
	kTransTypeVenitianBlind,
	kTransTypeCheckerboard,
	kTransTypeStripsBottomBuildLeft,
	kTransTypeStripsBottomBuildRight,
	kTransTypeStripsLeftBuildDown,
	kTransTypeStripsLeftBuildUp,
	kTransTypeStripsRightBuildDown,
	kTransTypeStripsRightBuildUp,
	kTransTypeStripsTopBuildLeft,
	kTransTypeStripsTopBuildRight,
	kTransZoomOpen,
	kTransZoomClose,
	kTransVerticalBinds,
	kTransDissolveBitsTrans,
	kTransDissolvePixels,
	kTransDissolveBits
};

struct Datum;
typedef Common::Array<Datum> DatumArray;

const char *scriptType2str(ScriptType scr);

} // End of namespace Director

#endif
