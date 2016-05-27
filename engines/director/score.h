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

#include "common/rect.h"
#include "common/stream.h"
#include "common/array.h"
#include "director/resource.h"

namespace Director {

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
    kSpritePositionWidth = 12,
    kSpritePositionHeight = 14
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

struct Cast {
    castType type;
    Common::Rect initialRect;
};

struct BitmapCast : Cast {
    Common::Rect boundingRect;
    uint16 regX;
    uint16 regY;
    uint8 flags;
};

struct ShapeCast : Cast {
    byte shapeType;
    uint16 pattern;
    byte fgCol;
    byte bgCol;
    byte fillType;
    byte lineThickness;
    byte lineDirection;
};

struct TextCast : Cast {
    byte borderSize;
    byte gutterSize;
    byte boxShadow;

    byte textType;
    byte textAlign;
    byte textShadow;
    byte textFlags;
};

struct ButtonCast : TextCast {
    //TODO types?
    uint16 buttonType;
};


class Sprite {
public:
    Sprite();
    Sprite(const Sprite &sprite);
    bool _enabled;
    byte _castId;
    //castType type;
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
    void display();

private:
    void readSprite(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
    void readMainChannels(Common::SeekableReadStream &stream, uint16 offset, uint16 size);

public:
    uint8 _actionId;
    uint8 _transFlags;
    uint8 _transChunkSize;
    uint8 _transType;
    uint8 _tempo;

    uint16 _sound1;
    uint8 _soundType1;
    uint16 _sound2;
    uint8 _soundType2;

    uint8 _skipFrameFlag;
    uint8 _blend;
    Common::Array<Sprite *> _sprites;

};

class Score {
public:
    Score(Common::SeekableReadStream &stream);
    void readVersion(uint32 rid);
    void loadConfig(Common::SeekableReadStream &stream);
    void loadCastData(Common::SeekableReadStream &stream);
    void play();

private:
    BitmapCast *getBitmapCast(Common::SeekableReadStream &stream);
    TextCast *getTextCast(Common::SeekableReadStream &stream);
    ButtonCast *getButtonCast(Common::SeekableReadStream &stream);
    ShapeCast *getShapeCast(Common::SeekableReadStream &stream);
    Common::Rect readRect(Common::SeekableReadStream &stream);
    void processEvents();
    void display();

public:
    Common::Array<Frame *> _frames;
    Common::HashMap<int, Cast *> _casts;

private:
    uint16 _versionMinor;
    uint16 _versionMajor;
    byte _currentFrameRate;
    uint16 _castArrayStart;
    uint16 _currentFrame;
    uint32 _nextFrameTime;
    bool _stopPlay;
    uint16 _castArrayEnd;
    Common::Rect _movieRect;
};

} //End of namespace Director
