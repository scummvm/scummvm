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

#ifndef NANCY_DATATYPES_H
#define NANCY_DATATYPES_H

#include "common/str.h"
#include "common/rect.h"
#include "common/array.h"

#include "graphics/surface.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {

// Various data types we need to extract from chunks go here

// Describes a scene
struct SceneSummary {
    void read(Common::SeekableReadStream &stream);

    Common::String description; // 0x00
    Common::String videoFile;   // 0x32
    //
    uint16 videoFormat;         // 0x3E, value is 1 or 2
    Common::String audioFile;   // 0x40
    int16 audioID;              // 0x4A
    uint16 audioVolume;         // 0x5A
    //
    uint16 verticalScrollDelta; // 0x72
    uint16 horizontalEdgeSize;  // 0x74
    uint16 verticalEdgeSize;    // 0x76
    uint16 slowMoveTimeDelta;   // 0x78
    uint16 fastMoveTimeDelta;   // 0x7A
    byte unknown7C;             // 0x7C, enum with 4 values
    //
};

// Describes the viewport
struct View {
    void read(Common::SeekableReadStream &stream);
    // The bounds of the destination rectangle on screen
    Common::Rect destination;
    // The bounds of the source rectangle (Background -> screen)
    Common::Rect source;
    // VideoFileFormat 1 rectangle bounds (video -> Background)
    Common::Rect f1Dest;
    // VideoFileFormat 2 rectangle bounds (video -> Background)
    Common::Rect f2Dest;
};

// Holds the coordinates for the bitmaps of all cursors
struct Cursors {
    void read(Common::SeekableReadStream &stream);

    Common::Rect rects[84];
    Common::Rect primaryVideoInactiveZone;
    Common::Point primaryVideoInitialPos;
};

struct Inventory {
    struct ItemDesc {
        Common::String name; // 0x00
        byte oneTimeUse = 0; // 0x14
        Common::Rect sourceRect; // 0x16
    };

    void read(Common::SeekableReadStream &stream);

    Common::Rect sliderSource; // 0x00
    Common::Point sliderDefaultDest; // 0x10
    //...
    Common::Rect shadesSrc[14]; // 0xD6
    Common::Rect shadesDst; // 0x1B6
    uint16 shadesFrameTime; // 0x1C6
    Common::String inventoryBoxIconsImageName; // 0x1C8
    Common::String inventoryCursorsImageName; // 0x1D2

    Common::Rect emptySpaceSource; // 0x1E4
    ItemDesc items[11]; // 0x1F4
};

struct Font {
    void read(Common::SeekableReadStream &stream);

    Common::String imageName; // 0x00
    Common::String description; // 0xA
    //
    Common::Point colorCoordsOffset; // y is def at 0x34, x is just a guess

    uint16 spaceWidth = 0;              // 0x38
    
    uint16 uppercaseOffset = 0;         // 0x3C
    uint16 lowercaseOffset = 0;         // 0x3E
    uint16 digitOffset = 0;             // 0x40
    uint16 periodOffset = 0;            // 0x42
    uint16 commaOffset = 0;             // 0x44
    uint16 equalitySignOffset = 0;      // 0x46
    uint16 colonOffset = 0;             // 0x48
    uint16 dashOffset = 0;              // 0x4A
    uint16 questionMarkOffset = 0;      // 0x4C
    uint16 exclamationMarkOffset = 0;   // 0x4E
    uint16 percentOffset = 0;           // 0x50
    uint16 ampersandOffset = 0;         // 0x52
    uint16 asteriskOffset = 0;          // 0x54
    uint16 leftBracketOffset = 0;       // 0x56
    uint16 rightBracketOffset = 0;      // 0x58
    uint16 plusOffset = 0;              // 0x5A
    uint16 apostropheOffset = 0;        // 0x5C
    uint16 semicolonOffset = 0;         // 0x5E
    uint16 slashOffset = 0;             // 0x60

    Common::Array<Common::Rect> symbolRects; // 0x62
    
    Graphics::Surface image;
};

} // End of namespace Nancy

#endif // NANCY_DATATYPES_H