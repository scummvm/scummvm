/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_WORLD_WORLDPOINT_H
#define ULTIMA8_WORLD_WORLDPOINT_H

/**
 * A 3D point in the format needed for passing back and forth to usecode.
 * Not intended for general engine use, as it has a sort of clunky interface.
 */
struct WorldPoint {
	uint8 _buf[5];

	uint16 getX() const {
		return _buf[0] + (_buf[1] << 8);
	}
	uint16 getY() const {
		return _buf[2] + (_buf[3] << 8);
	}
	uint16 getZ() const {
		return _buf[4];
	}

	void setX(int32 x) {
		_buf[0] = static_cast<uint8>(x);
		_buf[1] = static_cast<uint8>(x >> 8);
	}
	void setY(int32 y) {
		_buf[2] = static_cast<uint8>(y);
		_buf[3] = static_cast<uint8>(y >> 8);
	}
	void setZ(int32 z) {
		_buf[4] = static_cast<uint8>(z);
	}
};

#endif
