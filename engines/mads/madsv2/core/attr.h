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

#ifndef MADS_CORE_ATTR_H
#define MADS_CORE_ATTR_H

#include "mads/madsv2/core/buffer.h"
#include "mads/madsv2/core/room.h"
#include "mads/madsv2/core/tile.h"

namespace MADS {
namespace MADSV2 {

#define ATTR_WALK_MASK         0x80
#define ATTR_SPECIAL_MASK      0x70
#define ATTR_DEPTH_MASK        0x0f

#define ATTR_PACKED_ATTR_MASK  0x03
#define ATTR_PACKED_WALK_MASK  0x02
#define ATTR_PACKED_DEPTH_MASK 0x01

extern int attr_walk(Buffer *attr, int x, int y);

extern int attr_depth(TileMapHeader *depth_map, int x, int y);

extern int attr_special(Buffer *attr, int x, int y);

extern int attr_load(char *base_name, int item_type, int variant,
	Buffer *target, int size_x, int size_y);

extern int attr_load_error;

} // namespace MADSV2
} // namespace MADS

#endif
