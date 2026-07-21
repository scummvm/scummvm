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

#ifndef MADS_NEBULAR_ROOMS_TELEPORTER_H
#define MADS_NEBULAR_ROOMS_TELEPORTER_H

#include "common/rect.h"

namespace MADS {
namespace RexNebular {
namespace Rooms {

extern void teleporter_init();
extern int teleporter_address(int code, bool working);
extern void teleporter_handle_key();
extern Common::Point teleporter_compute_location();
extern bool teleporter_parser();
extern void teleporter_daemon();

} // namespace Rooms
} // namespace RexNebular
} // namespace MADS

#endif
