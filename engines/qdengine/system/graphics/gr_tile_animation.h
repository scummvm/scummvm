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

#include "common/path.h"

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

	bool load(Common::SeekableReadStream *fh, int version);

	void drawFrame(const Vect2i &position, int32 frame_index, int32 mode, int closest_scale) const;
	void drawFrame(const Vect2i &position, int frame_index, float angle, int mode = 0) const;
	void drawFrame_scale(const Vect2i &position, int frame_index, float scale, int mode) const;

	static void setProgressHandler(CompressionProgressHandler handler, void *context) {
		_progressHandler = handler;
		_progressHandlerContext = context;
	}

	void addScale(int i, float scale);
	byte *decode_frame_data(int frame_index, int closest_scale) const;
	int find_closest_scale(float *scale) const;
	bool wasFrameSizeChanged(int frame_index, int scaleIdx, float scale) const;

	void dumpTiles(Common::Path baseName, int tilesPerRow);

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

	struct ScaleArray {
		float _scale;
		Vect2i _frameSize;
		Vect2i _frameTileSize;
		int _frameStart;
	};

	Std::vector<ScaleArray> _scaleArray;

	/// индекс кадров - номера тайлов, из которых состоят кадры
	/// _frameTileSize.x * _frameTileSize.y на кадр
	FrameIndex _frameIndex;

	typedef Std::vector<uint32> TileOffsets;
	/// смещения до данных каждого тайла
	TileOffsets _tileOffsets;
	typedef Std::vector<uint32> TileData;
	/// данные тайлов
	TileData _tileData;

	Std::vector<Vect2i> _frameSizeArray;

	static CompressionProgressHandler _progressHandler;
	static void *_progressHandlerContext;
};

} // namespace QDEngine

#endif // QDENGINE_SYSTEM_GRAPHICS_GR_TILE_ANIMATION_H
