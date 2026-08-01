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

// Original game viewport dimensions (all scene maps and buffers use these).
static constexpr int kScreenWidth     = 320;
static constexpr int kScreenWidthLast = kScreenWidth - 1;
static constexpr int kGameHeight      = 200;
static constexpr int kGameHeightLast  = kGameHeight - 1;

// SCUMM-style verb/inventory strip (kEnhUIUX enhancement only).
static constexpr int kUIHeight       = 64;
static constexpr int kScreenHeight   = kGameHeight + kUIHeight;
static constexpr int kScreenHeightLast = kScreenHeight - 1;

/**
 * RESOURCE.MCS header / layout (loadResourceFile @ 1008:2e8d).
 * Magic is 12 ASCII bytes "AHFFMACS0100".
 * (Input-recording uses a different 12-byte tag; do not confuse with MCS.)
 */
static constexpr uint kMcsMagicSize = 12;
static constexpr const char *kMcsMagicV1 = "AHFFMACS0100";

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

} // namespace Macs2

#endif // MACS2_CONSTANTS_H
