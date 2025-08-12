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

#include "ultima/ultima8/gfx/palette.h"
#include "ultima/ultima8/gfx/texture.h"
#include "ultima/ultima8/misc/debugger.h"

namespace Ultima {
namespace Ultima8 {

void Palette::load(Common::ReadStream &rs, Common::ReadStream &xformrs) {
	load(rs);
	for (int i = 0; i < 256; i++)
		_xform_untransformed[i] = xformrs.readUint32LE();
}

void Palette::load(Common::ReadStream &rs) {
	int i;
	byte raw[768];
	rs.read(raw, 768);

	// convert from 0-63 to 0-255 _palette
	for (i = 0; i < 256; i++) {
		set(i, PALETTE_6BIT_TO_8BIT(raw[i * 3]), PALETTE_6BIT_TO_8BIT(raw[i * 3 + 1]), PALETTE_6BIT_TO_8BIT(raw[i * 3 + 2]));
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

void Palette::updateNativeMap(const Graphics::PixelFormat &format, int maxindex) {
	if (maxindex == 0)
		maxindex = size();

	for (int i = 0; i < maxindex; i++) {
		int32 r, g, b;
		byte sr, sg, sb;

		// Normal palette
		get(i, sr, sg, sb);
		if (format.isCLUT8()) {
			_native_untransformed[i] = i;
		} else {
			_native_untransformed[i] = format.RGBToColor(sr, sg, sb);
		}

		r = _matrix[0] * sr +
			_matrix[1] * sg +
			_matrix[2] * sb +
			_matrix[3] * 255;
		if (r < 0)
			r = 0;
		if (r > 0x7F800)
			r = 0x7F800;
		r = r >> 11;

		g = _matrix[4] * sr +
			_matrix[5] * sg +
			_matrix[6] * sb +
			_matrix[7] * 255;
		if (g < 0)
			g = 0;
		if (g > 0x7F800)
			g = 0x7F800;
		g = g >> 11;

		b = _matrix[8] * sr +
			_matrix[9] * sg +
			_matrix[10] * sb +
			_matrix[11] * 255;
		if (b < 0)
			b = 0;
		if (b > 0x7F800)
			b = 0x7F800;
		b = b >> 11;

		// Transformed normal palette
		if (format.isCLUT8()) {
			_native[i] = findBestColor(static_cast<uint8>(r),
									   static_cast<uint8>(g),
									   static_cast<uint8>(b));
		} else {
			_native[i] = format.RGBToColor(static_cast<uint8>(r),
										   static_cast<uint8>(g),
										   static_cast<uint8>(b));
		}

		// Transformed XFORM palette (Uses the TEX32 format)
		if (TEX32_A(_xform_untransformed[i])) {
			r = _matrix[0] * TEX32_R(_xform_untransformed[i]) +
				_matrix[1] * TEX32_G(_xform_untransformed[i]) +
				_matrix[2] * TEX32_B(_xform_untransformed[i]) +
				_matrix[3] * 255;
			if (r < 0)
				r = 0;
			if (r > 0x7F800)
				r = 0x7F800;

			g = _matrix[4] * TEX32_R(_xform_untransformed[i]) +
				_matrix[5] * TEX32_G(_xform_untransformed[i]) +
				_matrix[6] * TEX32_B(_xform_untransformed[i]) +
				_matrix[7] * 255;
			if (g < 0)
				g = 0;
			if (g > 0x7F800)
				g = 0x7F800;

			b = _matrix[8] * TEX32_R(_xform_untransformed[i]) +
				_matrix[9] * TEX32_G(_xform_untransformed[i]) +
				_matrix[10] * TEX32_B(_xform_untransformed[i]) +
				_matrix[11] * 255;
			if (b < 0)
				b = 0;
			if (b > 0x7F800)
				b = 0x7F800;

			_xform[i] = TEX32_PACK_RGBA(static_cast<uint8>(r >> 11),
										static_cast<uint8>(g >> 11),
										static_cast<uint8>(b >> 11),
										TEX32_A(_xform_untransformed[i]));
		} else
			_xform[i] = 0;
	}
}

} // End of namespace Ultima8
} // End of namespace Ultima
