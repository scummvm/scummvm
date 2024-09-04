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

#ifndef QDENGINE_SYSTEM_GRAPHICS_GR_TILE_SPRITE_H
#define QDENGINE_SYSTEM_GRAPHICS_GR_TILE_SPRITE_H

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
	grTileSprite(const uint32 *data_ptr = 0);

	bool operator == (const grTileSprite &sprite) const;

	bool isEmpty() const {
		return !_data;
	}

	const uint32 *data() const {
		return _data;
	}

	static uint32 comprasionTolerance() {
		return _comprasionTolerance;
	}
	static void setComprasionTolerance(uint32 value) {
		_comprasionTolerance = value;
	}

	static uint32 compress(const uint32 *in_data, uint32 *out_data, grTileCompressionMethod compress_method);
	static bool uncompress(const uint32 *in_data, uint32 in_data_length, uint32 *out_data, grTileCompressionMethod compress_method);

private:

	const uint32 *_data;

	/// толерантность побайтового сравнения данных, [0, 255]
	static uint32 _comprasionTolerance;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_TILE_SPRITE_H
