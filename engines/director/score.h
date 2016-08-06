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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef DIRECTOR_SCORE_H
#define DIRECTOR_SCORE_H

#include "common/rect.h"
#include "common/stream.h"
#include "common/array.h"
#include "director/resource.h"
#include "graphics/managed_surface.h"
#include "common/str.h"
#include "image/image_decoder.h"
#include "graphics/font.h"

namespace Director {

class Lingo;
class DirectorSound;
class Score;
class DirectorEngine;

#define CHANNEL_COUNT 24

enum CastType {
	kCastBitmap = 1,
	kCastFilmLoop,
	kCastText,
	kCastPalette,
	kCastPicture,
	kCastSound,
	kCastButton,
	kCastShape,
	kCastMovie,
	kCastDigitalVideo,
	kCastScript
};

//Director v4
enum SpriteType {
	kInactiveSprite, //turns the sprite off
	kBitmapSprite,
	kRectangleSprite,
	kRoundedRectangleSprite,
	kOvalSprite,
	kLineTopBottomSprite, //line from top left to bottom right
	kLineBottomTopSprite, //line from bottom left to top right
	kTextSprite,
	kButtonSprite,
	kCheckboxSprite,
	kRadioButtonSprite,
	kUndeterminedSprite = 16 //use castType property to examine the type of cast member associated with sprite
};

enum SpritePosition {
	kSpritePositionUnk1 = 0,
	kSpritePositionEnabled,
	kSpritePositionUnk2,
	kSpritePositionFlags = 4,
	kSpritePositionCastId = 6,
	kSpritePositionY = 8,
	kSpritePositionX = 10,
	kSpritePositionHeight = 12,
	kSpritePositionWidth = 14
};

enum MainChannelsPosition {
	kScriptIdPosition = 0,
	kSoundType1Position,
	kTransFlagsPosition,
	kTransChunkSizePosition,
	kTempoPosition,
	kTransTypePosition,
	kSound1Position,
	kSkipFrameFlagsPosition = 8,
	kBlendPosition,
	kSound2Position,
	kSound2TypePosition = 11,
	kPaletePosition = 15
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

enum ScriptType {
	kMovieScript = 0,
	kSpriteScript = 1,
	kFrameScript = 2,
	kMaxScriptType = 2
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

struct Cast {
	CastType type;
	Common::Rect initialRect;
	byte modified;
};

struct BitmapCast : Cast {
	BitmapCast(Common::SeekableSubReadStreamEndian &stream);

	Common::Rect boundingRect;
	uint16 regX;
	uint16 regY;
	uint8 flags;
};

enum ShapeType {
	kShapeRectangle,
	kShapeRoundRect,
	kShapeOval,
	kShapeLine
};

struct ShapeCast : Cast {
	ShapeCast(Common::SeekableSubReadStreamEndian &stream);

	ShapeType shapeType;
	uint16 pattern;
	byte fgCol;
	byte bgCol;
	byte fillType;
	byte lineThickness;
	byte lineDirection;
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
	kTextFlagEditable,
	kTextFlagAutoTab,
	kTextFlagDoNotWrap
};

enum SizeType {
	kSizeNone,
	kSizeSmallest,
	kSizeSmall,
	kSizeMedium,
	kSizeLarge,
	kSizeLargest
};

struct TextCast : Cast {
	TextCast(Common::SeekableSubReadStreamEndian &stream);

	SizeType borderSize;
	SizeType gutterSize;
	SizeType boxShadow;

	uint32 fontId;
	uint16 fontSize;
	TextType textType;
	TextAlignType textAlign;
	SizeType textShadow;
	Common::Array<TextFlag> textFlags;
};

enum ButtonType {
	kTypeButton,
	kTypeCheckBox,
	kTypeRadio
};

struct ButtonCast : TextCast {
	ButtonCast(Common::SeekableSubReadStreamEndian &stream) : TextCast(stream) {
		buttonType = static_cast<ButtonType>(stream.readUint16BE());
	}

	ButtonType buttonType;
};

struct CastInfo {
	Common::String script;
	Common::String name;
	Common::String directory;
	Common::String fileName;
	Common::String type;
};

struct PaletteInfo {
	uint8 firstColor;
	uint8 lastColor;
	uint8 flags;
	uint8 speed;
	uint16 frameCount;
};

class Sprite {
public:
	Sprite();
	Sprite(const Sprite &sprite);
	~Sprite();
	bool _enabled;
	byte _castId;
	InkType _ink;
	uint16 _trails;
	Cast *_cast;
	uint16 _flags;
	Common::Point _startPoint;
	uint16 _width;
	uint16 _height;
	//TODO: default constraint = 0, if turned on, sprite is constrainted to the bounding rect
	//As i know, constrainted != 0 only if sprite moveable
	byte _constraint;
	byte _moveable;
	byte _backColor;
	byte _foreColor;
	uint16 _left;
	uint16 _right;
	uint16 _top;
	uint16 _bottom;
	byte _blend;
	bool _visible;
	SpriteType _type;
	//Using in digital movie sprites
	byte _movieRate;
	uint16 _movieTime;
	uint16 _startTime;
	uint16 _stopTime;
	byte _volume;
	byte _stretch;
	//Using in shape sprites
	byte _lineSize;
	//Using in text sprites
	Common::String _editableText;
};

class Frame {
public:
	Frame(DirectorEngine *vm);
	Frame(const Frame &frame);
	~Frame();
	void readChannel(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	void prepareFrame(Score *score);
	uint16 getSpriteIDFromPos(Common::Point pos);

private:
	void playTransition(Score *score);
	void playSoundChannel();
	void renderSprites(Graphics::ManagedSurface &surface, bool renderTrail);
	void renderText(Graphics::ManagedSurface &surface, uint16 spriteId);
	void renderButton(Graphics::ManagedSurface &surface, uint16 spriteId);
	void readPaletteInfo(Common::SeekableSubReadStreamEndian &stream);
	void readSprite(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	void readMainChannels(Common::SeekableSubReadStreamEndian &stream, uint16 offset, uint16 size);
	Image::ImageDecoder *getImageFrom(uint16 spriteID);
	void drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawMatteSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawGhostSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
	void drawReverseSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
public:
	uint8 _actionId;
	uint8 _transDuration;
	uint8 _transArea; //1 - Whole Stage, 0 - Changing Area
	uint8 _transChunkSize;
	TransitionType _transType;
	PaletteInfo *_palette;
	uint8 _tempo;

	uint16 _sound1;
	uint8 _soundType1;
	uint16 _sound2;
	uint8 _soundType2;

	uint8 _skipFrameFlag;
	uint8 _blend;
	Common::Array<Sprite *> _sprites;
	Common::Array<Common::Rect > _drawRects;
	DirectorEngine *_vm;
};

struct Label {
	Common::String name;
	uint16 number;
	Label(Common::String name1, uint16 number1) { name = name1; number = number1; }
};

class Score {
public:
	Score(DirectorEngine *vm);
	~Score();

	static Common::Rect readRect(Common::SeekableSubReadStreamEndian &stream);
	static int compareLabels(const void *a, const void *b);
	void loadArchive();
	void setStartToLabel(Common::String label);
	void gotoloop();
	void gotonext();
	void gotoprevious();
	void startLoop();
	void processEvents();
	Archive *getArchive() const { return _movieArchive; };
	void loadCastData(Common::SeekableSubReadStreamEndian &stream);
	void setCurrentFrame(uint16 frameId) { _currentFrame = frameId; }
	Common::String getMacName() const { return _macName; }
	Sprite *getSpriteById(uint16 id);
private:
	void update();
	void readVersion(uint32 rid);
	void loadConfig(Common::SeekableSubReadStreamEndian &stream);
	void loadPalette(Common::SeekableSubReadStreamEndian &stream);
	void loadFrames(Common::SeekableSubReadStreamEndian &stream);
	void loadLabels(Common::SeekableSubReadStreamEndian &stream);
	void loadActions(Common::SeekableSubReadStreamEndian &stream);
	void loadCastInfo(Common::SeekableSubReadStreamEndian &stream, uint16 id);
	void loadScriptText(Common::SeekableSubReadStreamEndian &stream);
	void loadFileInfo(Common::SeekableSubReadStreamEndian &stream);
	void loadFontMap(Common::SeekableSubReadStreamEndian &stream);
	void dumpScript(const char *script, ScriptType type, uint16 id);
	Common::String getString(Common::String str);
	Common::Array<Common::String> loadStrings(Common::SeekableSubReadStreamEndian &stream, uint32 &entryType, bool hasHeader = true);

public:
	Common::Array<Frame *> _frames;
	Common::HashMap<int, Cast *> _casts;
	Common::HashMap<uint16, CastInfo *> _castsInfo;
	Common::SortedArray<Label *> *_labels;
	Common::HashMap<uint16, Common::String> _actions;
	Common::HashMap<uint16, Common::String> _fontMap;
	Graphics::ManagedSurface *_surface;
	Graphics::ManagedSurface *_trailSurface;
	Graphics::Font *_font;
	Archive *_movieArchive;
	Common::Rect _movieRect;

private:
	uint16 _versionMinor;
	uint16 _versionMajor;
	Common::String _macName;
	Common::String _createdBy;
	Common::String _changedBy;
	Common::String _script;
	Common::String _directory;
	byte _currentFrameRate;
	uint16 _castArrayStart;
	uint16 _currentFrame;
	Common::String _currentLabel;
	uint32 _nextFrameTime;
	uint32 _flags;
	bool _stopPlay;
	uint16 _castArrayEnd;
	uint16 _movieScriptCount;
	uint16 _stageColor;
	Lingo *_lingo;
	DirectorSound *_soundManager;
	DirectorEngine *_vm;
};

} //End of namespace Director

#endif
