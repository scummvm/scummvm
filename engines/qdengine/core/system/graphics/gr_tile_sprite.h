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

#ifndef __GR_TILE_SPRITE_H__
#define __GR_TILE_SPRITE_H__


namespace QDEngine {

const int GR_TILE_SPRITE_SIZE_SHIFT = 4;
const int GR_TILE_SPRITE_SIZE_X     = 1 << GR_TILE_SPRITE_SIZE_SHIFT;
const int GR_TILE_SPRITE_SIZE_Y     = 1 << GR_TILE_SPRITE_SIZE_SHIFT;

const int GR_TILE_SPRITE_SIZE       = GR_TILE_SPRITE_SIZE_X * GR_TILE_SPRITE_SIZE_Y;
const int GR_TILE_SPRITE_SIZE_BYTES = GR_TILE_SPRITE_SIZE * 4;

enum grTileCompressionMethod {
	TILE_UNCOMPRESSED,
	TILE_COMPRESS_RLE,
	TILE_COMPRESS_LZ77
};

/// Тайл-спрайт

/// Квадратный 32х битный спрайт фиксированного размера.
/// Данные внешние.
class grTileSprite {
public:
	grTileSprite(const unsigned *data_ptr = 0);

	bool operator == (const grTileSprite &sprite) const;

	bool isEmpty() const {
		return !data_;
	}

	const unsigned *data() const {
		return data_;
	}

	static unsigned comprasionTolerance() {
		return comprasionTolerance_;
	}
	static void setComprasionTolerance(unsigned value) {
		comprasionTolerance_ = value;
	}

	static unsigned compress(const unsigned *in_data, unsigned *out_data, grTileCompressionMethod compress_method);
	static bool uncompress(const unsigned *in_data, unsigned in_data_length, unsigned *out_data, grTileCompressionMethod compress_method);

private:

	const unsigned *data_;

	/// толерантность побайтового сравнения данных, [0, 255]
	static unsigned comprasionTolerance_;
};

} // namespace QDEngine

#endif /*__GR_TILE_SPRITE_H__ */
