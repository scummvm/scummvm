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

#ifndef MADS_CORE_VIDEO_H
#define MADS_CORE_VIDEO_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

extern byte video_mode;

void video_init(int mode, int set_mode);

void video_update(Buffer *from, int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y);

void video_flush_ega(int start_y, int size_y);



/* Mode-specific versions */

void video_update_vga(Buffer *from, int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y);

void video_update_ega(Buffer *from, int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y);

void video_update_tandy(Buffer *from, int from_x, int from_y,
	int unto_x, int unto_y,
	int size_x, int size_y);

} // namespace MADSV2
} // namespace MADS

#endif
