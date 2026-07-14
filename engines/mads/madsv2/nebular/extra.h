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

extern RoomPtr room_load_rex(int id, int variant, const char *base_path, Buffer *picture,
	Buffer *depth, Buffer *walk, Buffer *special, TileMapHeader *picMap,
	TileMapHeader *depthMap, TileResource *picResource, TileResource *depthResource,
	int picture_ems_handle, int depth_ems_handle, int load_flags);
extern int room_load_depth(Load *load_handle, Buffer *depth, Buffer *walk, Room *room_info,
	int variant, bool packedFormat);

} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS

#endif
