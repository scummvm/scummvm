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

#include "engines/nancy/action/recordtypes.h"
#include "common/array.h"

namespace Nancy {

struct ConditionalResponseDesc {
    byte characterID; // 0: Daryl, 1: Connie, 2: Hal, 3: Hulk
    uint fileOffset;
    uint16 sceneID;
    FlagDesc conditions[7];
};

struct GoodbyeDesc {
    byte characterID;
    uint fileOffset;
    uint16 sceneIDs[4];
};

static const uint nancy1ResponseBaseFileOffset = 0xB1FE0; // TODO there could be more than one version of the exe

#define EMPTY_DESC {-1, PlayState::kFalse }

static const GoodbyeDesc nancy1Goodbyes[] = {
    // Daryl
    {
        0,
        0x11B0,
        { 0xC94, 0xC95, 0xC96, 0xC97}
    },

    // Connie
    {
        1,
        0x11D8,
        { 0xFC, 0x9D8, 0x9D9, 0x9DB }
    },

    // Hal
    {
        2,
        0x11FC,
        { 0x1C3, 0x1C4, 0x1C5, 0x1C6}
    },

    // Hulk
    {
        3,
        0x1228,
        { 0xCE2, 0xCE0, 0xCE2, 0xCE0 } // only two responses
    }
};

static const ConditionalResponseDesc nancy1ConditionalResponses[] = {
    // Daryl
    {
        0, 
        0x840,
        0x7C,
        {
            { 0x1D, PlayState::kTrue },
            { 0x39, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x804,
        0x7F,
        {
            { 0x13, PlayState::kTrue },
            { 0x37, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x7BC,
        0x81,
        {
            { 0xB, PlayState::kTrue },
            { 0x38, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x750,
        0x83,
        {
            { 0, PlayState::kTrue },
            { 1, PlayState::kFalse },
            { 0x6B, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x6F4,
        0x84,
        {
            { 0x64, PlayState::kTrue },
            { 0x1E, PlayState::kFalse },
            { 0x14, PlayState::kFalse },
            { 0xC, PlayState::kFalse },
            { 0x6C, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x5EC,
        0x86,
        {
            { 0x6D, PlayState::kFalse },
            { 0x6, PlayState::kTrue },
            { 0x8, PlayState::kTrue },
            { 0x5E, PlayState::kTrue },
            { 0x17, PlayState::kTrue },
            { 0x24, PlayState::kTrue },
            { 0x9, PlayState::kTrue }
        }
    },

    {
        0,
        0x554,
        0x8B,
        {
            { 0x6E, PlayState::kFalse },
            { 0x24, PlayState::kTrue },
            { 0x9, PlayState::kTrue },
            { 0x5E, PlayState::kFalse },
            { 0x8, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x4F0,
        0x8D,
        {
            { 0x6F, PlayState::kFalse },
            { 0x5E, PlayState::kTrue },
            { 0x24, PlayState::kTrue },
            { 0x9, PlayState::kTrue },
            { 0x8, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x458,
        0x8F,
        {
            { 0x70, PlayState::kFalse },
            { 0x24, PlayState::kTrue },
            { 0x9, PlayState::kTrue },
            { 0x6, PlayState::kTrue },
            { 0x8, PlayState::kTrue },
            { 0x5E, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x3BC,
        0x90,
        {
            { 0x71, PlayState::kFalse },
            { 0x5E, PlayState::kTrue },
            { 0x24, PlayState::kFalse },
            { 0x8, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x320,
        0x91,
        {
            { 0x72, PlayState::kFalse },
            { 0x5E, PlayState::kTrue },
            { 0x8, PlayState::kTrue },
            { 0x6, PlayState::kTrue },
            { 0x24, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x2AC,
        0x92,
        {
            { 0x73, PlayState::kFalse },
            { 0x8, PlayState::kTrue },
            { 0x6, PlayState::kTrue },
            { 0x5E, PlayState::kFalse },
            { 0x24, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x1F0,
        0x96,
        {
            { 0x74, PlayState::kFalse },
            { 0x1D, PlayState::kTrue },
            { 0x13, PlayState::kTrue },
            { 0xB, PlayState::kTrue },
            { 0x5E, PlayState::kFalse },
            { 0x24, PlayState::kFalse },
            { 0x8, PlayState::kFalse }
        }
    },

    {
        0,
        0x190,
        0x97,
        {
            { 0x27, PlayState::kFalse },
            { 0x5, PlayState::kTrue },
            EMPTY_DESC
        }
    },

    {
        0,
        0xF0,
        0x9C,
        {
            { 0x28, PlayState::kTrue },
            { 0x75, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x94,
        0x93,
        {
            { 0xC, PlayState::kFalse },
            { 0x6, PlayState::kTrue },
            { 0x76, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0x58,
        0x94,
        {
            { 0x14, PlayState::kFalse },
            { 0x4, PlayState::kTrue },
            { 0x77, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        0,
        0,
        0x95,
        {
            { 0x1E, PlayState::kFalse },
            { 0x63, PlayState::kTrue },
            { 0x78, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    // Connie
    {
        1,
        0xBE4,
        0xE9,
        {
            { 0x1D, PlayState::kTrue },
            { 0x18, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0xB8C,
        0xEA,
        {
            { 0x1F, PlayState::kTrue },
            { 0x19, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0xB54,
        0xEB,
        {
            { 0xB, PlayState::kTrue },
            { 0x1A, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0xB14,
        0xEC,
        {
            { 0x26, PlayState::kTrue },
            { 0x1C, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0xABC,
        0xED,
        {
            { 0, PlayState::kTrue },
            { 1, PlayState::kFalse },
            { 0x79, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0xA00,
        0xEE,
        {
            { 2, PlayState::kTrue },
            { 3, PlayState::kTrue },
            { 0x17, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0x6F4,
        0xEF,
        {
            { 0x64, PlayState::kTrue },
            { 0x16, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0x968,
        0xF0,
        {
            { 0x5, PlayState::kTrue },
            { 0x14, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        1,
        0x8C8,
        0xF5,
        {
            { 0x28, PlayState::kTrue },
            EMPTY_DESC
        }
    },

    {
        1,
        0x884,
        0xE7,
        {
            { 0xD, PlayState::kTrue },
            { 0x5E, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    // Hal
    {
        2,
        0xED0,
        0x1B3,
        {
            { 0x1D, PlayState::kTrue },
            { 0x11, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0x804,
        0x1B5,
        {
            { 0x13, PlayState::kTrue },
            { 0xE, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xE74,
        0x1B6,
        {
            { 0x1B, PlayState::kTrue },
            { 0xF, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xE2C,
        0x1B7,
        {
            { 0x26, PlayState::kTrue },
            { 0x10, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xDD4,
        0x1B9,
        {
            { 0, PlayState::kTrue },
            { 1, PlayState::kFalse },
            { 0x68, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xD48,
        0x1BA,
        {
            { 0, PlayState::kTrue },
            { 1, PlayState::kFalse },
            { 0x20, PlayState::kTrue },
            { 0x69, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0x6F4,
        0x1BB,
        {
            { 0x6A, PlayState::kFalse },
            { 0x64, PlayState::kTrue },
            { 0x5, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xCC8,
        0x1BC,
        {
            { 0x8, PlayState::kTrue },
            { 0x6, PlayState::kTrue },
            { 0xC, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        2,
        0xC2C,
        0x1BE,
        {
            { 0x28, PlayState::kTrue },
            EMPTY_DESC
        }
    },

    // Hulk
    {
        3,
        0x1164,
        0x14D,
        {
            { 0x13, PlayState::kTrue },
            { 0x3A, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0xB54,
        0x150,
        {
            { 0xB, PlayState::kTrue },
            { 0x25, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0x10D8,
        0x153,
        {
            { 0x12, PlayState::kTrue },
            { 0x21, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {

        3,
        0xE2C,
        0x154,
        {
            { 0x26, PlayState::kTrue },
            { 0x22, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0x108C,
        0x155,
        {
            { 0, PlayState::kTrue },
            { 1, PlayState::kFalse },
            { 0x66, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0x6F4,
        0x156,
        {
            { 0x67, PlayState::kFalse },
            { 0x64, PlayState::kTrue },
            EMPTY_DESC
        }
    },

    {
        3,
        0x1028,
        0x157,
        {
            { 0x63, PlayState::kTrue },
            { 0x24, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0xFB0,
        0x158,
        {
            { 0x5, PlayState::kTrue },
            { 0x1E, PlayState::kFalse },
            EMPTY_DESC
        }
    },

    {
        3,
        0xF10,
        0x159,
        {
            { 0x28, PlayState::kTrue },
            EMPTY_DESC
        }
    }
};

} // End of namespace Nancy