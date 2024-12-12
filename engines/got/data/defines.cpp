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

#include "got/data/defines.h"

namespace Got {

void LEVEL::load(Common::SeekableReadStream *src) {
	for (int i = 0; i < 12; ++i)
		src->read(icon[i], 20);

	bg_color = src->readByte();
	type = src->readByte();

	src->read(actor_type, 16);
	src->read(actor_loc, 16);
	src->read(actor_value, 16);
	src->read(pal_colors, 3);
	src->read(actor_invis, 16);
	src->read(extra, 13);
	src->read(static_obj, 30);

	for (int i = 0; i < 30; ++i)
		static_x[i] = src->readSint16LE();
	for (int i = 0; i < 30; ++i)
		static_y[i] = src->readSint16LE();

	src->read(new_level, 10);
	src->read(new_level_loc, 10);

	area = src->readByte();
	src->read(actor_dir, 16);
	src->read(future, 3);
}

} // namespace Got
