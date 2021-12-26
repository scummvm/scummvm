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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/graphics/palette.h"

namespace Ultima {
namespace Ultima8 {

void Palette::load(Common::ReadStream &rs, Common::ReadStream &xformrs) {
	load(rs);
	for (int i = 0; i < 256; i++)
		_xform_untransformed[i] = xformrs.readUint32LE();
}

void Palette::load(Common::ReadStream &rs) {
	int i;
	rs.read(_palette, 768);

	// convert from 0-63 to 0-255 _palette
	for (i = 0; i < 256; i++) {
		_palette[i * 3] = (_palette[i * 3] * 255) / 63;
		_palette[i * 3 + 1] = (_palette[i * 3 + 1] * 255) / 63;
		_palette[i * 3 + 2] = (_palette[i * 3 + 2] * 255) / 63;
	}

	for (i = 0; i < 256; i++)
		_xform_untransformed[i] = 0;

	// Setup the transformation _matrix
	_matrix[0] = 0x800;
	_matrix[1] = 0;
	_matrix[2] = 0;
	_matrix[3] = 0;
	_matrix[4] = 0;
	_matrix[5] = 0x800;
	_matrix[6] = 0;
	_matrix[7] = 0;
	_matrix[8] = 0;
	_matrix[9] = 0;
	_matrix[10] = 0x800;
	_matrix[11] = 0;

	_transform = Transform_None;
}

void Palette::transformRGB(int &r_, int &g_, int &b_) const {
	const int r = r_;
	const int g = g_;
	const int b = b_;

	r_ = (r * _matrix[0] + g * _matrix[1] + b * _matrix[2]  + 255 * _matrix[3]) / 2048;
	if (r_ < 0) r_ = 0;
	if (r_ > 0xFF) r_ = 0xFF;

	g_ = (r * _matrix[4] + g * _matrix[5] + b * _matrix[6]  + 255 * _matrix[7]) / 2048;
	if (g_ < 0) g_ = 0;
	if (g_ > 0xFF) g_ = 0xFF;

	b_ = (r * _matrix[8] + g * _matrix[9] + b * _matrix[10] + 255 * _matrix[11]) / 2048;
	if (b_ < 0) b_ = 0;
	if (b_ > 0xFF) b_ = 0xFF;
}

} // End of namespace Ultima8
} // End of namespace Ultima
