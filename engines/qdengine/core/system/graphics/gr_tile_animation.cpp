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

#define _NO_ZIP_
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/file.h"
#include "qdengine/qdengine.h"
#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/system/graphics/gr_dispatcher.h"
#include "qdengine/core/system/graphics/gr_tile_animation.h"


namespace QDEngine {

CompressionProgressHandler grTileAnimation::progressHandler_;
void *grTileAnimation::progressHandlerContext_;

grTileAnimation::grTileAnimation() {
	clear();
}

void grTileAnimation::clear() {
	hasAlpha_ = false;

	compression_ = TILE_UNCOMPRESSED;

	frameCount_ = 0;
	frameSize_ = Vect2i(0, 0);
	frameTileSize_ = Vect2i(0, 0);

	frameIndex_.clear();
	FrameIndex(frameIndex_).swap(frameIndex_);

	tileOffsets_.clear();
	TileOffsets(tileOffsets_).swap(tileOffsets_);

	tileData_.clear();
	TileData(tileData_).swap(tileData_);
}

void grTileAnimation::init(int frame_count, const Vect2i &frame_size, bool alpha_flag) {
	clear();

	hasAlpha_ = alpha_flag;

	frameSize_ = frame_size;

	frameTileSize_.x = (frame_size.x + GR_TILE_SPRITE_SIZE_X / 2) / GR_TILE_SPRITE_SIZE_X;
	frameTileSize_.y = (frame_size.y + GR_TILE_SPRITE_SIZE_Y / 2) / GR_TILE_SPRITE_SIZE_Y;

	frameIndex_.reserve(frame_count * frameTileSize_.x * frameTileSize_.y);

	tileOffsets_.reserve(frame_count * frameTileSize_.x * frameTileSize_.y + 1);
	tileOffsets_.push_back(0);

	tileData_.reserve(frame_count * frameTileSize_.x * frameTileSize_.y * GR_TILE_SPRITE_SIZE);

	frameCount_ = frame_count;
}

void grTileAnimation::compact() {
	TileOffsets(tileOffsets_).swap(tileOffsets_);
	TileData(tileData_).swap(tileData_);
	debugC(3, kDebugLog, "Tile animation: %lu Kbytes", (frameIndex_.size() + tileData_.size() + tileOffsets_.size()) * 4 / 1024);
}

bool grTileAnimation::compress(grTileCompressionMethod method) {
	if (compression_ != TILE_UNCOMPRESSED)
		return false;

	compression_ = method;

	TileData tile_data;
	tile_data.reserve(tileData_.size());

	TileOffsets tile_offsets;
	tile_offsets.reserve(tileOffsets_.size());
	tile_offsets.push_back(0);

	TileData tile_vector = TileData(GR_TILE_SPRITE_SIZE * 4, 0);

	int count = tileCount();
	for (int i = 0; i < count; i++) {
		if (progressHandler_) {
			int percent_done = 100 * (i + 1) / count;
			(*progressHandler_)(percent_done, progressHandlerContext_);
		}

		unsigned *data = &*tileData_.begin() + i * GR_TILE_SPRITE_SIZE;

		unsigned offs = tile_offsets.back();
		unsigned sz = grTileSprite::compress(data, &*tile_vector.begin(), method);
		tile_data.insert(tile_data.end(), tile_vector.begin(), tile_vector.begin() + sz);
		tile_offsets.push_back(offs + sz);
	}

	tileData_.swap(tile_data);
	tileOffsets_.swap(tile_offsets);

	return true;
}


grTileSprite grTileAnimation::getTile(int tile_index) const {
	debugC(3, kDebugTemp, "The tile index is given by %d", tile_index);
	static unsigned tile_buf[GR_TILE_SPRITE_SIZE];

	switch (compression_) {
	case TILE_UNCOMPRESSED:
		return grTileSprite(&*tileData_.begin() + tileOffsets_[tile_index]);
	default:
		if (!grTileSprite::uncompress(&*tileData_.begin() + tileOffsets_[tile_index], GR_TILE_SPRITE_SIZE, tile_buf, compression_)) {
			xassert(0 && "Неизвестный алгоритм сжатия");
		}
		return grTileSprite(tile_buf);
	}
}

void grTileAnimation::addFrame(const unsigned *frame_data) {
	TileData tile_vector = TileData(GR_TILE_SPRITE_SIZE, 0);
	TileData tile_vector2 = TileData(GR_TILE_SPRITE_SIZE * 4, 0);

	if (progressHandler_) {
		int percent_done = 100 * (frameIndex_.size() / (frameTileSize_.x * frameTileSize_.y) + 1) / (frameCount_ ? frameCount_ : 1);
		(*progressHandler_)(percent_done, progressHandlerContext_);
	}

	for (int i = 0; i < frameTileSize_.y; i ++) {
		for (int j = 0; j < frameTileSize_.x; j ++) {
			std::fill(tile_vector.begin(), tile_vector.end(), 0);

			const unsigned *data_ptr = frame_data + j * GR_TILE_SPRITE_SIZE_X
			                           + i * GR_TILE_SPRITE_SIZE_Y * frameSize_.x;

			unsigned *tile_ptr = &tile_vector[0];
			for (int y = 0; y < GR_TILE_SPRITE_SIZE_Y; y++) {
				if (y + i * GR_TILE_SPRITE_SIZE_Y >= frameSize_.y) break;

				for (int x = 0; x < GR_TILE_SPRITE_SIZE_X; x++) {
					if (x + j * GR_TILE_SPRITE_SIZE_X >= frameSize_.x) break;
					tile_ptr[x] = data_ptr[x];
				}

				data_ptr += frameSize_.x;
				tile_ptr += GR_TILE_SPRITE_SIZE_X;
			}

			int tile_id = -1;
			int tile_count = tileCount();
			for (int tile_idx = 0; tile_idx < tile_count; tile_idx++) {
				grTileSprite tile = getTile(tile_idx);
				if (tile == grTileSprite(&tile_vector[0])) {
					tile_id = tile_idx;
					break;
				}
			}

			if (tile_id == -1) {
				unsigned sz = GR_TILE_SPRITE_SIZE;
				unsigned offs = tileOffsets_.back();

				tileData_.insert(tileData_.end(), tile_vector.begin(), tile_vector.end());
				tileOffsets_.push_back(offs + sz);
				frameIndex_.push_back(tile_count);
			} else
				frameIndex_.push_back(tile_id);
		}
	}
}

bool grTileAnimation::save(XStream &fh) const {
	fh < frameCount_ < frameSize_.x < frameSize_.y < frameTileSize_.x < frameTileSize_.y < (int)compression_;

	fh < (int)frameIndex_.size();
	fh.write(&frameIndex_[0], frameIndex_.size() * sizeof(unsigned));

	fh < (int)tileOffsets_.size();
	fh.write(&tileOffsets_[0], tileOffsets_.size() * sizeof(unsigned));

	fh < (int)tileData_.size();
	fh.write(&tileData_[0], tileData_.size() * sizeof(unsigned));

	return true;
}

bool grTileAnimation::load(XStream &fh) {
	warning("STUB: grTileAnimation::load(XStream &fh)");
	return true;
}

bool grTileAnimation::load(Common::SeekableReadStream *fh) {

	debugC(7, kDebugLoad, "grTileAnimation::load(): pos start: %lu", fh->pos());

	frameCount_ = fh->readSint32LE();
	frameSize_.x = fh->readSint32LE();
	frameSize_.y = fh->readSint32LE();
	frameTileSize_.x = fh->readSint32LE();
	frameTileSize_.y = fh->readSint32LE();
	uint32 size = fh->readUint32LE();

	debugC(7, kDebugLoad, "grTileAnimation::load(): frameCount: %d  frame: %d x %d tile: %d x %d compsize: %d", frameCount_, frameSize_.x, frameSize_.y,
		frameTileSize_.x, frameTileSize_.y, size);

	compression_ = grTileCompressionMethod(size);

	size = fh->readUint32LE();
	frameIndex_.resize(size);
	debugC(7, kDebugLoad, "grTileAnimation::load(): pos: %d frameIndex_ size: %u", fh->pos() - 4, size);
	for (int i = 0; i < size; i++) {
		frameIndex_[i] = fh->readUint32LE();
		debugCN(8, kDebugLoad, " %d ", frameIndex_[i]);
	}
	debugCN(8, kDebugLoad, "\n");

	size = fh->readUint32LE();
	tileOffsets_.resize(size);
	for (int i = 0; i < size; i++) {
		tileOffsets_[i] = fh->readUint32LE();
	}

	size = fh->readUint32LE();
	tileData_.resize(size);
	for (int i = 0; i < size; i++) {
		tileData_[i] = fh->readUint32LE();
	}

	return true;
}

bool grTileAnimation::load(XZipStream &fh) {
	warning("STUB: grTileAnimation::load(XZipStream fh)");
	return true;
}

void grTileAnimation::drawFrame(const Vect2i &position, int32 frame_index, int32 mode) const {
	Vect2i pos0 = position - frameSize_ / 2;

	int32 dx = GR_TILE_SPRITE_SIZE_X;
	int32 dy = GR_TILE_SPRITE_SIZE_Y;

	if (mode & GR_FLIP_HORIZONTAL) {
		pos0.x += frameSize_.x - GR_TILE_SPRITE_SIZE_X;
		dx = -dx;
	}
	if (mode & GR_FLIP_VERTICAL) {
		pos0.y += frameSize_.y - GR_TILE_SPRITE_SIZE_Y;
		dy = -dy;
	}

//	grDispatcher::instance()->Rectangle(position.x - frameSize_.x/2, position.y - frameSize_.y/2, frameSize_.x, frameSize_.y, 0xFFFFF, 0, GR_OUTLINED);

	const uint32 *index_ptr = &frameIndex_[0] + frameTileSize_.x * frameTileSize_.y * frame_index;

	debugC(3, kDebugTemp, "The length of frameIndex is given by %u", frameIndex_.size());
	debugC(3, kDebugTemp, "The value of increment is given by %d", frameTileSize_.x * frameTileSize_.y * frame_index);
	debugC(3, kDebugTemp, "grTileAnimation::drawFrame %u", index_ptr);
	debugC(3, kDebugTemp, "grTileAnimation::drawFrame *index_ptr: %d", *index_ptr);

	Vect2i pos = pos0;
	for (int32 i = 0; i < frameTileSize_.y; i++) {
		pos.x = pos0.x;

		for (int32 j = 0; j < frameTileSize_.x; j++) {
			grDispatcher::instance()->PutTileSpr(pos.x, pos.y, getTile(*index_ptr++), hasAlpha_, mode);
			pos.x += dx;
		}

		pos.y += dy;
	}
}

void grTileAnimation::drawFrame(const Vect2i &position, int frame_index, float angle, int mode) const {
	unsigned char *buf = (unsigned char *)grDispatcher::instance()->temp_buffer(frameSize_.x * frameSize_.y * 4);

	const unsigned *index_ptr = &frameIndex_[0] + frameTileSize_.x * frameTileSize_.y * frame_index;

	for (int i = 0; i < frameTileSize_.y; i++) {
		for (int j = 0; j < frameTileSize_.x; j++) {
			unsigned char *buf_ptr = buf + (i * frameSize_.x + j) * 4;
			const unsigned char *data_ptr = (const unsigned char *)getTile(*index_ptr++).data();
			int dx = min(frameSize_.x - j * GR_TILE_SPRITE_SIZE_X, GR_TILE_SPRITE_SIZE_X) * 4;
			int dy = min(frameSize_.y - i * GR_TILE_SPRITE_SIZE_Y, GR_TILE_SPRITE_SIZE_Y);
			for (int k = 0; k < GR_TILE_SPRITE_SIZE_Y; k++) {
				memcpy(buf_ptr, data_ptr, dx);
				data_ptr += GR_TILE_SPRITE_SIZE_X * 4;
				buf_ptr += frameSize_.x * 4;
			}
		}
	}

	grDispatcher::instance()->PutSpr_rot(position, frameSize_, buf, hasAlpha_, mode, angle);
}

} // namespace QDEngine
