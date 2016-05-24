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

struct Sprite {
public:
    bool enabled;
    uint8 castId;
    //castType type;
    uint16 flags;
    Common::Point startPoint;
    uint16 width;
    uint16 height;
    Sprite();
    Sprite(const Sprite& sprite);
};

struct Frame {
public:
    uint8 actionId;
    
    uint8 transFlags;
    uint8 transChunkSize;
    uint8 transType;
    uint8 tempo;
    
    uint16 sound1;
    uint8 soundType1;
    uint16 sound2;
    uint8 soundType2;

    uint8 skipFrameFlag;
    uint8 blend;
    Common::Array<Sprite*> sprites;

    Frame();
    ~Frame();
    Frame(const Frame& frame);
    void readChannel(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
private:
    void readSprite(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
    void readMainChannels(Common::SeekableReadStream &stream, uint16 offset, uint16 size);
};

struct Score {
public:
	Score(Common::SeekableReadStream &stream);
    Common::Array<Frame*> frames;
};

} //End of namespace Director