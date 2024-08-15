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


#ifndef QDENGINE_SYSTEM_GRAPHICS_GR_TILE_ANIMATION_H
#define QDENGINE_SYSTEM_GRAPHICS_GR_TILE_ANIMATION_H

#include "qdengine/xmath.h"
#include "qdengine/system/graphics/gr_tile_sprite.h"

namespace Common {
class SeekableReadStream;
}

namespace QDEngine {

typedef void (*CompressionProgressHandler)(int percents_loaded, void *context);

class grTileAnimation {
public:
	grTileAnimation();

	bool isEmpty() const {
		return !_frameCount;
	}

	void clear();

	int frameCount() const {
		return _frameCount;
	}
	const Vect2i &frameSize() const {
		return _frameSize;
	}
	const Vect2i &frameTileSize() const {
		return _frameTileSize;
	}
	int tileCount() const {
		return _tileOffsets.size() - 1;
	}

	void init(int frame_count, const Vect2i &frame_size, bool alpha_flag);

	void compact();

	bool compress(grTileCompressionMethod method);

	grTileSprite getTile(int tile_index) const;

	void addFrame(const uint32 *frame_data);

	bool load(Common::SeekableReadStream *fh);

	void drawFrame(const Vect2i &position, int32 frame_index, int32 mode = 0) const;
	void drawFrame(const Vect2i &position, int frame_index, float angle, int mode = 0) const;

	static void setProgressHandler(CompressionProgressHandler handler, void *context) {
		_progressHandler = handler;
		_progressHandlerContext = context;
	}

private:

	grTileCompressionMethod _compression;

	/// true если есть альфа-канал
	bool _hasAlpha;

	/// размеры кадра в пикселах
	/// могут быть невыровненными по рамерам тайла
	Vect2i _frameSize;

	/// размеры кадра в тайлах
	Vect2i _frameTileSize;

	/// количество кадров
	int _frameCount;

	typedef Std::vector<uint32> FrameIndex;
	/// индекс кадров - номера тайлов, из которых состоят кадры
	/// _frameTileSize.x * _frameTileSize.y на кадр
	FrameIndex _frameIndex;

	typedef Std::vector<uint32> TileOffsets;
	/// смещения до данных каждого тайла
	TileOffsets _tileOffsets;
	typedef Std::vector<uint32> TileData;
	/// данные тайлов
	TileData _tileData;

	static CompressionProgressHandler _progressHandler;
	static void *_progressHandlerContext;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_TILE_ANIMATION_H

