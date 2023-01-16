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

#include "tetraedge/te/te_color.h"

namespace Tetraedge {

TeColor::TeColor() : _c{0, 0, 0, 0xff} {
}

TeColor::TeColor(uint32 rgba) {
	_c[0] = (byte)(rgba >> 24);
	_c[1] = (byte)(rgba >> 16);
	_c[2] = (byte)(rgba >> 8);
	_c[3] = (byte)rgba;
}

TeColor::TeColor(uint16 shortcol) {
	_c[0] = (byte)(shortcol >> 7) & 0xf8;
	_c[1] = (byte)(shortcol >> 2) & 0xf8;
	_c[2] = (byte)shortcol << 3;
	_c[3] = 0;
}

TeColor::TeColor(byte r, byte g, byte b, byte a) : _c{r, g, b, a} {
}

uint32 TeColor::getPacked() const {
	return (g() & 0xf8) << 2 | (r() & 0xf8) << 7 | (b() >> 3);
}

uint32 TeColor::getPacked32() const {
	return (r() << 24) | (g() << 16) | (b() << 8) | a();
}

bool TeColor::serialize(Common::WriteStream &stream) const {
	for (int i = 0; i < 4; i++)
		stream.writeByte(_c[i]);
	return true;
}
bool TeColor::deserialize(Common::ReadStream &stream) {
	for (int i = 0; i < 4; i++)
		_c[i] = stream.readByte();
	return true;
}

TeColor operator*(const TeColor &c1, const TeColor &c2) {
	return TeColor(
		((c2.r() * (c1.r() / 255.0)) / 255.0) * 255.0,
		((c2.g() * (c1.g() / 255.0)) / 255.0) * 255.0,
		((c2.b() * (c1.b() / 255.0)) / 255.0) * 255.0,
		((c2.a() * (c1.a() / 255.0)) / 255.0) * 255.0);
}

TeColor operator*(const TeColor &c, double amount) {
	return TeColor(c.r() * amount, c.g() * amount, c.b() * amount, c.a() * amount);
}

TeColor operator+(const TeColor &c1, const TeColor &c2) {
	return TeColor(c1.r() + c2.r(), c1.g() + c2.g(), c1.b() + c2.b(), c1.a() + c2.a());
}

} // end namespace Tetraedge
