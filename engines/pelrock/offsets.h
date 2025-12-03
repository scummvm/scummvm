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
#ifndef PELROCK_OFFSETS_H
#define PELROCK_OFFSETS_H

#include "common/scummsys.h"

namespace Pelrock {

    static const uint32_t cursor_offsets[5] = {
        0x0FDDFD,
        0x0FDCDD,
        0x0FDF1D,
        0x0FE33D,
        0x367EF0
    };

    static const uint32_t kBalloonFramesOffset = 2176936;
    static const uint32_t kBalloonFramesSize = 24950;

    static const uint32_t ALFRED7_ALFRED_COMB_R = 67768;
    static const uint32_t ALFRED7_ALFRED_COMB_L = 88408;

    static const uint32_t kSettingsMenuOffset = 910097; // Placeholder offset
    static const uint32_t kSettingsPaletteOffset = 1038141; // 640 * 480



} // End of namespace Pelrock
#endif

