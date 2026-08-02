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

#ifndef MACS2_CONSTANTS_H
#define MACS2_CONSTANTS_H

namespace Macs2 {

// V1 (AHFFMACS0100) viewport - default engine dimensions.
static constexpr int kScreenWidth     = 320;
static constexpr int kScreenWidthLast = kScreenWidth - 1;
static constexpr int kGameHeight      = 200;
static constexpr int kGameHeightLast  = kGameHeight - 1;

// V2 dimensions.
static constexpr int kWinScreenWidth     = 640;
static constexpr int kWinScreenWidthLast = kWinScreenWidth - 1;
static constexpr int kWinGameHeight      = 400;
static constexpr int kWinGameHeightLast  = kWinGameHeight - 1;

// SCUMM-style verb/inventory strip (kEnhUIUX enhancement only).
static constexpr int kUIHeight         = 64;
static constexpr int kScreenHeight     = kGameHeight + kUIHeight;
static constexpr int kScreenHeightLast = kScreenHeight - 1;
static constexpr int kWinScreenHeight  = kWinGameHeight + kUIHeight;

/**
 * MCS file layout.
 * Magic is 12 ASCII bytes "AHFFMACS0100" (v1) or "AHFFMACS0200" (v2).
 */
static constexpr uint kMcsMagicSize = 12;
static constexpr const char *kMcsMagicV1 = "AHFFMACS0100";
static constexpr const char *kMcsMagicV2 = "AHFFMACS0200";

/** MCS v1 absolute file offsets (after validating AHFFMACS0100). */
static constexpr uint32 kMcsV1ActorIndexOffset = 0x0C;
static constexpr uint32 kMcsV1DirectoryOffset = 0x10;
static constexpr uint32 kMcsV1DirectorySize = 0x3000; // 512 entries * 12 bytes
/** Object DATA file-offset dword, relative to directory base (entry i at +rel + i*12). */
static constexpr uint32 kMcsV1ObjectDataPtrRel = 0x17F4;
/** Object SCRIPT/resource-table file-offset dword, relative to directory base. */
static constexpr uint32 kMcsV1ObjectScriptPtrRel = 0x17F8;
static constexpr uint32 kMcsV1VanillaPaletteOffset = kMcsV1DirectoryOffset + kMcsV1DirectorySize; // 0x3010
static constexpr uint32 kMcsV1VanillaPaletteSize = 0x300;
static constexpr uint32 kMcsV1ShadingTableOffset = kMcsV1VanillaPaletteOffset + kMcsV1VanillaPaletteSize; // 0x3310
static constexpr uint32 kMcsV1ShadingTableSize = 0x800;
static constexpr uint kMcsV1CursorImageCount = 0x21;
static constexpr uint kMcsV1MapSceneOffsetCount = 256;
static constexpr uint32 kMcsV1MapSceneOffsetsSize = kMcsV1MapSceneOffsetCount * 4; // 0x400

static constexpr uint32 kMcsV2ActorIndexOffset = 0x20E;
static constexpr uint32 kMcsV2DirectoryOffset = 0x212;

} // namespace Macs2

#endif // MACS2_CONSTANTS_H
