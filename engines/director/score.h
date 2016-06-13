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

namespace Director {

class Lingo;
class DirectorSound;

#define CHANNEL_COUNT 24

enum castType {
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

enum spritePositions {
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

enum mainChannelsPosition {
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

enum inkType {
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

enum scriptType {
    kMovieScript,
    kSpriteScript,
    kFrameScript
};

enum transitionType {
    kTransWipeRight = 1,
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
    kTransDisolvePixelsFast,
    kTransDisolveBoxyRects,
    kTransDisolveBoxySquares,
    kTransDisolvePatterns,
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
    kTransDisolveBitsTrans,
    kTransDisolvePixels,
    kTransDisolveBits
};

struct Cast {
    castType type;
    Common::Rect initialRect;
};

struct BitmapCast : Cast {
    BitmapCast(Common::SeekableReadStream &stream);
    Common::Rect boundingRect;
    uint16 regX;
    uint16 regY;
    uint8 flags;
};

struct ShapeCast : Cast {
    ShapeCast(Common::SeekableReadStream &stream);
    byte shapeType;
    uint16 pattern;
    byte fgCol;
    byte bgCol;
    byte fillType;
    byte lineThickness;
    byte lineDirection;
};

struct TextCast : Cast {
    TextCast(Common::SeekableReadStream &stream);
    byte borderSize;
    byte gutterSize;
    byte boxShadow;

    byte textType;
    byte textAlign;
    byte textShadow;
    byte textFlags;
};

struct ButtonCast : TextCast {
    ButtonCast(Common::SeekableReadStream &stream) : TextCast(stream) {
        buttonType = stream.readUint16BE();
    }
    //TODO types?
    uint16 buttonType;
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
    bool _enabled;
    byte _castId;
    inkType _ink;
    uint16 _trails;
    Cast *_cast;
    uint16 _flags;
    Common::Point _startPoint;
    uint16 _width;
    uint16 _height;
};

class Frame {
public:
    Frame();
    ~Frame();
    Frame(const Frame &frame);
    void readChannel(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
    void prepareFrame(Archive &_movie, Graphics::ManagedSurface &surface, Graphics::ManagedSurface &trailSurface, Common::Rect movieRect);
    uint16 getSpriteIDFromPos(Common::Point pos);
private:
    void playTransition(Graphics::ManagedSurface &frameSurface, Common::Rect transRect);
    void playSoundChannel();
    void renderSprites(Archive &_movie, Graphics::ManagedSurface &surface, Common::Rect movieRect, bool renderTrail);
    void readPaletteInfo(Common::SeekableReadStream &stream);
    void readSprite(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
    void readMainChannels(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
    void drawBackgndTransSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
    void drawMatteSprite(Graphics::ManagedSurface &target, const Graphics::Surface &sprite, Common::Rect &drawRect);
public:
    uint8 _actionId;
    uint8 _transDuration;
    uint8 _transArea; //1 - Whole Stage, 0 - Changing Area
    uint8 _transChunkSize;
    transitionType _transType;
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
};

class Score {
public:
    Score(Archive &movie, Lingo &lingo, DirectorSound &soundManager);
    ~Score();
    static Common::Rect readRect(Common::SeekableReadStream &stream);
    void startLoop();

private:
    void processEvents();
    void update();
    void readVersion(uint32 rid);
    void loadConfig(Common::SeekableReadStream &stream);
    void loadCastData(Common::SeekableReadStream &stream);
    void loadFrames(Common::SeekableReadStream &stream);
    void loadLabels(Common::SeekableReadStream &stream);
    void loadActions(Common::SeekableReadStream &stream);
    void loadCastInfo(Common::SeekableReadStream &stream, uint16 id);
    void loadFileInfo(Common::SeekableReadStream &stream);
    void loadFontMap(Common::SeekableReadStream &stream);
    void dumpScript(uint16 id, scriptType type, Common::String script);
    Common::String getString(Common::String str);
    Common::Array<Common::String> loadStrings(Common::SeekableReadStream &stream, uint32 &entryType, bool hasHeader = true);
public:
    Common::Array<Frame *> _frames;
    Common::HashMap<int, Cast *> _casts;
    Common::HashMap<uint16, CastInfo *> _castsInfo;
    Common::HashMap<uint16, Common::String> _labels;
    Common::HashMap<uint16, Common::String> _actions;
    Common::HashMap<uint16, Common::String> _fontMap;
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
    uint32 _nextFrameTime;
    uint32 _flags;
    bool _stopPlay;
    uint16 _castArrayEnd;
    Common::Rect _movieRect;
    uint16 _stageColor;
    Archive *_movieArchive;
    Graphics::ManagedSurface *_surface;
    Graphics::ManagedSurface *_trailSurface;
    Lingo *_lingo;
    DirectorSound *_soundManager;
};

} //End of namespace Director

#endif
