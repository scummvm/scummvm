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

#ifndef __GR_TILE_ANIMATION_H__
#define __GR_TILE_ANIMATION_H__

#include "qdengine/core/system/graphics/gr_tile_sprite.h"


namespace QDEngine {

typedef void (*CompressionProgressHandler)(int percents_loaded, void *context);

class grTileAnimation {
public:
	grTileAnimation();

	bool isEmpty() const {
		return !frameCount_;
	}

	void clear();

	int frameCount() const {
		return frameCount_;
	}
	const Vect2i &frameSize() const {
		return frameSize_;
	}
	const Vect2i &frameTileSize() const {
		return frameTileSize_;
	}
	int tileCount() const {
		return tileOffsets_.size() - 1;
	}

	void init(int frame_count, const Vect2i &frame_size, bool alpha_flag);

	void compact();

	bool compress(grTileCompressionMethod method);

	grTileSprite getTile(int tile_index) const;

	void addFrame(const unsigned *frame_data);

	bool save(XStream &fh) const;
	bool load(XStream &fh);
	bool load(XZipStream &fh);

	void drawFrame(const Vect2i &position, int frame_index, int mode = 0) const;
	void drawFrame(const Vect2i &position, int frame_index, float angle, int mode = 0) const;

	static void setProgressHandler(CompressionProgressHandler handler, void *context) {
		progressHandler_ = handler;
		progressHandlerContext_ = context;
	}

private:

	grTileCompressionMethod compression_;

	/// true если есть альфа-канал
	bool hasAlpha_;

	/// размеры кадра в пикселах
	/// могут быть невыровненными по рамерам тайла
	Vect2i frameSize_;

	/// размеры кадра в тайлах
	Vect2i frameTileSize_;

	/// количество кадров
	int frameCount_;

	typedef std::vector<unsigned> FrameIndex;
	/// индекс кадров - номера тайлов, из которых состоят кадры
	/// frameTileSize_.x * frameTileSize_.y на кадр
	FrameIndex frameIndex_;

	typedef std::vector<unsigned> TileOffsets;
	/// смещения до данных каждого тайла
	TileOffsets tileOffsets_;
	typedef std::vector<unsigned> TileData;
	/// данные тайлов
	TileData tileData_;

	static CompressionProgressHandler progressHandler_;
	static void *progressHandlerContext_;
};

} // namespace QDEngine

#endif /* __GR_TILE_ANIMATION_H__ */
