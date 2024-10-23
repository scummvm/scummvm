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

#ifndef DIRECTOR_TYPES_H
#define DIRECTOR_TYPES_H

namespace Director {

#define CONTINUATION (0xAC)

enum {
	kFewFamesMaxCounter = 19,
};

enum {
	kShotColorDiffThreshold = 2,
	kShotPercentPixelThreshold = 1
};

#define kQuirksCacheArchive "quirks"

enum MovieFlag {
	kMovieFlagRemapPalettesWhenNeeded =  (1 << 6),
	kMovieFlagAllowOutdatedLingo	= (1 << 8)
};

enum CastType {
	kCastTypeAny = -1,
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
	kCastRTE = 12,
	kCastTransition = 14,
};

enum ScriptType {
	kNoneScript = -1,
	kScoreScript = 0,
	kCastScript = 1,
	kMovieScript = 2,
	kEventScript = 3,
	kTestScript = 4,
	kParentScript = 7,
	kMaxScriptType = 7	// Sync with types.cpp:28, array scriptTypes[]
};

enum EventHandlerSourceType {
	kNoneHandler = 0,
	kPrimaryHandler = 1,
	kSpriteHandler = 2,
	kCastHandler = 3,
	kFrameHandler = 4,
	kMovieHandler = 5
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
	kFilmLoopSprite					= 17,
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

enum RenderMode {
	kRenderModeNormal,
	kRenderForceUpdate
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
	kClutSystemWin = -101,
	kClutSystemWinD5 = -102
};

enum {
	kNumBuiltinTiles = 8
};

enum DirectorCursor {
	kCursorMouseDown,
	kCursorMouseUp
};

enum PlayState {
	kPlayNotStarted,
	kPlayLoaded,
	kPlayStarted,
	kPlayStopped,
	kPlayPaused,
	kPlayPausedAfterLoading,
};

enum SymbolType {
	VOIDSYM,
	OPCODE,
	CBLTIN,	// builtin command
	FBLTIN,	// builtin function
	HBLTIN,	// builtin handler (can be called as either command or func)
	KBLTIN,	// builtin constant
	FBLTIN_LIST, // builtin function w/list override check
	HBLTIN_LIST, // builtin handler w/list override check
	HANDLER	// user-defined handler
};

enum ChunkType {
	kChunkChar,
	kChunkWord,
	kChunkItem,
	kChunkLine
};

enum FileVer {
	kFileVer300 = 0x404,
	kFileVer310 = 0x405,
	kFileVer400 = 0x45B,
	kFileVer404 = 0x45D,
	kFileVer500 = 0x4B1,
	kFileVer600 = 0x4C2,
	kFileVer700 = 0x4C8,
	kFileVer800 = 0x582,
	kFileVer850 = 0x6A4,
	kFileVer1000 = 0x73B,
	kFileVer1100 = 0x781,
	kFileVer1150 = 0x782,
	kFileVer1200 = 0x79F
};

enum DatumType {
	ARGC,
	ARGCNORET,
	ARRAY,
	CASTREF,
	CASTLIBREF,
	CHUNKREF,
	FIELDREF,
	FLOAT,
	GLOBALREF,
	INT,
	LOCALREF,
	MENUREF,
	OBJECT,
	PARRAY,
	PICTUREREF,
	POINT,
	PROPREF,
	RECT,
	STRING,
	SYMBOL,
	VARREF,
	VOID,
};

enum VarType {
	kVarGeneric,
	kVarArgument,
	kVarProperty,
	kVarInstance,
	kVarGlobal,
	kVarLocal
};

enum LPPFlag {
	kLPPNone = 0,
	kLPPSimple = 1 << 0,
	kLPPForceD2 = 1 << 1,
	kLPPTrimGarbage = 1 << 2,
};

struct CastMemberID {
	int member;
	int castLib;

	CastMemberID() : member(0), castLib(0) {}
	CastMemberID(int memberID, int castLibID)
		: member(memberID), castLib(castLibID) {}

	bool operator==(const CastMemberID &c) const {
		return member == c.member && castLib == c.castLib;
	}
	bool operator!=(const CastMemberID &c) const {
		return member != c.member || castLib != c.castLib;
	}

	bool isNull() const { return member == 0 && castLib == 0; }

	Common::String asString() const;

	uint hash() const { return ((castLib & 0xffff) << 16) + (member & 0xffff); }
};

enum CompareResult {
	kCompareLess	= 1 << 0,
	kCompareEqual	= 1 << 1,
	kCompareGreater = 1 << 2,
	kCompareError	= 1 << 3,
};

enum DebugDrawModes {
	kDebugDrawCast  = 1 << 0,
	kDebugDrawFrame = 1 << 1,
};

struct Datum;
struct PCell;
typedef Common::Array<Datum> DatumArray;
typedef Common::Array<PCell> PropertyArray;

const char *scriptType2str(ScriptType scr);
const char *castType2str(CastType type);
const char *spriteType2str(SpriteType type);
const char *inkType2str(InkType type);

} // End of namespace Director

namespace Common {

template<>
struct Hash<Director::CastMemberID> {
	uint operator()(const Director::CastMemberID &id) const {
		return id.hash();
	}
};

} // End of namespace Common

#endif
