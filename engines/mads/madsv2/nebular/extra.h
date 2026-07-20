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

#ifndef MADS_NEBULAR_EXTRA_H
#define MADS_NEBULAR_EXTRA_H

#include "mads/madsv2/core/room.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {

/**
 * Rex Nebular's game-specific room loader, called from room_load() (see core/room.h) in
 * place of the generic loader used by every other game. Reads the room's .DAT header,
 * loads its background art into picture, its sprite series, and stamps the room's images
 * onto picture, then defers to room_load_variant() to populate depth, walk, and special
 * for the room's default variant.
 */
extern RoomPtr room_load(int id, int variant, const char *base_path, Buffer *picture,
	Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *picMap,
	TileMapHeader *depthMap, TileResource *picResource, TileResource *depthResource,
	int picture_ems_handle, int depth_ems_handle, int load_flags);

/**
 * Rex Nebular's version of the default room_load_variant() (see core/room.h), called
 * from it whenever the active room is Rex Nebular - both when populating a freshly
 * loaded room's default variant (from room_load()) and when switching to a different
 * attribute variant thereafter. Reads the variant's run-length encoded depth/walk/special
 * data from the room's .DAT file and unpacks it into depth (nibble-packed, 2 pixels/byte),
 * walk (bit-packed, 8 pixels/byte), and special (nibble-packed, 2 pixels/byte) surfaces.
 * Any of depth, walk, or special may be null to skip populating that surface. Packed-format
 * rooms (packedFormat == true) have no spare bits to encode a special zone code, so
 * special is left untouched in that case.
 */
extern int room_load_variant(Load *load_handle, Buffer *depth, Buffer *walk, Buffer *special,
	Room *room_info, int variant, bool packedFormat);

/**
 * Rex Nebular's version of buffer_legal() (see core/buffer.h), called from
 * player_start_walking() to scan each rail segment of a walk path for a special
 * trigger zone. Unlike the generic engine's boolean, bit-packed special surface,
 * Rex's special surface is nibble-packed (2 pixels/byte, matching the depth buffer's
 * layout) and holds a 0-7 special zone code per pixel (see ATTR_SPECIAL_MASK). Traces
 * a Bresenham line from (x1, y1) to (x2, y2) using the same algorithm as buffer_legal(),
 * and returns the first non-zero code the line crosses, or 0 if the path is clear.
 */
extern int buffer_legal(const Buffer &special, int orig_wrap, int x1, int y1, int x2, int y2);

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
