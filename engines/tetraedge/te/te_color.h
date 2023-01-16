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

#ifndef TETRAEDGE_TE_TE_COLOR_H
#define TETRAEDGE_TE_TE_COLOR_H

#include "common/types.h"
#include "common/stream.h"

namespace Tetraedge {

class TeColor {
public:
	TeColor();
	TeColor(uint32 rgba);
	TeColor(uint16 shortcol);
	TeColor(byte r, byte g, byte b, byte a);

	byte &r() { return _c[0]; };
	byte &g() { return _c[1]; };
	byte &b() { return _c[2]; };
	byte &a() { return _c[3]; };

	const byte &r() const { return _c[0]; };
	const byte &g() const { return _c[1]; };
	const byte &b() const { return _c[2]; };
	const byte &a() const { return _c[3]; };

	uint32 getPacked() const;
	uint32 getPacked32() const;

	bool serialize(Common::WriteStream &stream) const;
	bool deserialize(Common::ReadStream &stream);

	bool operator==(const TeColor &c) const {
		return (_c[0] == c._c[0] && _c[1] == c._c[1] &&
				_c[2] == c._c[2] && _c[3] == c._c[3]);
	}
	bool operator!=(const TeColor &c) {
		return !operator==(c);
	}

	Common::String dump() const {
		return Common::String::format("TeColor(%d %d %d %d)",
									 _c[0], _c[1], _c[2], _c[3]);
	}

private:
	byte _c[4];
};

TeColor operator*(const TeColor &c1, const TeColor &c2);
TeColor operator*(const TeColor &c1, double amount);
TeColor operator+(const TeColor &c1, const TeColor &c2);

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_COLOR_H
