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

enum MovieFlag {
	kMovieFlagAllowOutdatedLingo	= (1 << 8)
};

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
	kScoreScript = 0,
	kCastScript = 1,
	kMovieScript = 2,
	kEventScript = 3,
	kMaxScriptType = 3	// Sync with score-loading.cpp:45, array scriptTypes[]
};

enum ScriptFlag {
	kScriptFlagUnused		= (1 << 0x0),
	kScriptFlagFuncsGlobal	= (1 << 0x1),
	kScriptFlagVarsGlobal	= (1 << 0x2),	// Occurs in event scripts (which have no local vars). Correlated with use of alternate global var opcodes.
	kScriptFlagUnk3			= (1 << 0x3),
	kScriptFlagFactoryDef	= (1 << 0x4),
	kScriptFlagUnk5			= (1 << 0x5),
	kScriptFlagUnk6			= (1 << 0x6),
	kScriptFlagUnk7			= (1 << 0x7),
	kScriptFlagHasFactory	= (1 << 0x8),
	kScriptFlagEventScript	= (1 << 0x9),
	kScriptFlagEventScript2	= (1 << 0xa),
	kScriptFlagUnkB			= (1 << 0xb),
	kScriptFlagUnkC			= (1 << 0xc),
	kScriptFlagUnkD			= (1 << 0xd),
	kScriptFlagUnkE			= (1 << 0xe),
	kScriptFlagUnkF			= (1 << 0xf)
};

enum ObjectType {
	kNoneObj = 0,
	kFactoryObj = 1 << 0,
	kXObj = 1 << 1,
	kScriptObj = 1 << 2,
	kXtraObj = 1 << 3,
	kAllObj = kFactoryObj | kXObj | kScriptObj | kXtraObj,
	kWindowObj = 1 << 4,
	kCastMemberObj = 1 << 5
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

enum FrameRateType {
	kFrameRateDefault = -1,
	kFrameRateNormal = 0,
	kFrameRateFastest = 1,
	kFrameRateFixed = 2
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
	kThickLineSprite				= 15,	// 2pt width line
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
	kEventGeneric,
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

	kEventStartUp,

	kEventMenuCallback
};

enum TransitionType {
	kTransNone,
	kTransWipeRight,
	kTransWipeLeft,
	kTransWipeDown,
	kTransWipeUp,
	kTransCenterOutHorizontal,	// 5
	kTransEdgesInHorizontal,
	kTransCenterOutVertical,
	kTransEdgesInVertical,
	kTransCenterOutSquare,
	kTransEdgesInSquare,		// 10
	kTransPushLeft,
	kTransPushRight,
	kTransPushDown,
	kTransPushUp,
	kTransRevealUp,				// 15
	kTransRevealUpRight,
	kTransRevealRight,
	kTransRevealDownRight,
	kTransRevealDown,
	kTransRevealDownLeft,		// 20
	kTransRevealLeft,
	kTransRevealUpLeft,
	kTransDissolvePixelsFast,
	kTransDissolveBoxyRects,
	kTransDissolveBoxySquares,	// 25
	kTransDissolvePatterns,
	kTransRandomRows,
	kTransRandomColumns,
	kTransCoverDown,
	kTransCoverDownLeft,		// 30
	kTransCoverDownRight,
	kTransCoverLeft,
	kTransCoverRight,
	kTransCoverUp,
	kTransCoverUpLeft,			// 35
	kTransCoverUpRight,
	kTransVenetianBlind,
	kTransCheckerboard,
	kTransStripsBottomBuildLeft,
	kTransStripsBottomBuildRight,	// 40
	kTransStripsLeftBuildDown,
	kTransStripsLeftBuildUp,
	kTransStripsRightBuildDown,
	kTransStripsRightBuildUp,
	kTransStripsTopBuildLeft,		// 45
	kTransStripsTopBuildRight,
	kTransZoomOpen,
	kTransZoomClose,
	kTransVerticalBinds,
	kTransDissolveBitsFast,	// 50
	kTransDissolvePixels,
	kTransDissolveBits
};

// TODO: Can there be any more built-in palette types?
enum PaletteType {
	kClutSystemMac = -1,
	kClutRainbow = -2,
	kClutGrayscale = -3,
	kClutPastels = -4,
	kClutVivid = -5,
	kClutNTSC = -6,
	kClutMetallic = -7,
	kClutSystemWin = -101
};

enum {
	kCursorDefault,
	kCursorMouseDown,
	kCursorMouseUp
};

enum PlayState {
	kPlayNotStarted,
	kPlayStarted,
	kPlayStopped
};

enum SymbolType {
	VOIDSYM,
	OPCODE,
	CBLTIN,	// builtin command
	FBLTIN,	// builtin function
	HBLTIN,	// builtin handler (can be called as either command or func)
	KBLTIN,	// builtin constant
	HANDLER	// user-defined handler
};

enum ChunkType {
	kChunkChar,
	kChunkWord,
	kChunkItem,
	kChunkLine
};

struct Datum;
struct PCell;
typedef Common::Array<Datum> DatumArray;
typedef Common::Array<PCell> PropertyArray;

const char *scriptType2str(ScriptType scr);

} // End of namespace Director

#endif
