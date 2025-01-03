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

#include "common/algorithm.h"
#include "common/memstream.h"
#include "got/data/defines.h"
#include "got/vars.h"

namespace Got {

void LEVEL::sync(Common::Serializer &s) {
	for (int i = 0; i < 12; ++i)
		s.syncBytes(icon[i], 20);

	s.syncAsByte(bg_color);
	s.syncAsByte(type);

	s.syncBytes(actor_type, 16);
	s.syncBytes(actor_loc, 16);
	s.syncBytes(actor_value, 16);
	s.syncBytes(pal_colors, 3);
	s.syncBytes(actor_invis, 16);
	s.syncBytes(extra, 13);
	s.syncBytes(static_obj, 30);

	for (int i = 0; i < 30; ++i)
		s.syncAsSint16LE(static_x[i]);
	for (int i = 0; i < 30; ++i)
		s.syncAsSint16LE(static_y[i]);

	s.syncBytes(new_level, 10);
	s.syncBytes(new_level_loc, 10);

	s.syncAsByte(area);
	s.syncBytes(actor_dir, 16);
	s.syncBytes(future, 3);
}

void LEVEL::load(int level) {
	Common::MemoryReadStream src(_G(sd_data)[level], 512);
	Common::Serializer s(&src, nullptr);
	sync(s);
}

void LEVEL::save(int level) {
	Common::MemoryWriteStream dest(_G(sd_data)[level], 512);
	Common::Serializer s(nullptr, &dest);
	sync(s);
}

} // namespace Got
