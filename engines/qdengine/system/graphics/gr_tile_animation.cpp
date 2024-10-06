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

#include "common/debug.h"
#include "common/file.h"

#include "graphics/managed_surface.h"
#include "image/png.h"

#include "qdengine/qdengine.h"
#include "qdengine/system/graphics/gr_dispatcher.h"
#include "qdengine/system/graphics/gr_tile_animation.h"


namespace QDEngine {

CompressionProgressHandler grTileAnimation::_progressHandler;
void *grTileAnimation::_progressHandlerContext;

grTileAnimation::grTileAnimation() {
	clear();
}

void grTileAnimation::clear() {
	_hasAlpha = false;

	_compression = TILE_UNCOMPRESSED;

	_frameCount = 0;
	_frameSize = Vect2i(0, 0);
	_frameTileSize = Vect2i(0, 0);

	_frameIndex.clear();
	FrameIndex(_frameIndex).swap(_frameIndex);

	_tileOffsets.clear();
	TileOffsets(_tileOffsets).swap(_tileOffsets);

	_tileData.clear();
	TileData(_tileData).swap(_tileData);
}

void grTileAnimation::init(int frame_count, const Vect2i &frame_size, bool alpha_flag) {
	clear();

	_hasAlpha = alpha_flag;

	_frameSize = frame_size;

	_frameTileSize.x = (frame_size.x + GR_TILE_SPRITE_SIZE_X / 2) / GR_TILE_SPRITE_SIZE_X;
	_frameTileSize.y = (frame_size.y + GR_TILE_SPRITE_SIZE_Y / 2) / GR_TILE_SPRITE_SIZE_Y;

	_frameIndex.reserve(frame_count * _frameTileSize.x * _frameTileSize.y);

	_tileOffsets.reserve(frame_count * _frameTileSize.x * _frameTileSize.y + 1);
	_tileOffsets.push_back(0);

	_tileData.reserve(frame_count * _frameTileSize.x * _frameTileSize.y * GR_TILE_SPRITE_SIZE);

	_frameCount = frame_count;
}

void grTileAnimation::compact() {
	TileOffsets(_tileOffsets).swap(_tileOffsets);
	TileData(_tileData).swap(_tileData);
	debugC(3, kDebugLog, "Tile animation: %u Kbytes", (_frameIndex.size() + _tileData.size() + _tileOffsets.size()) * 4 / 1024);
}

bool grTileAnimation::compress(grTileCompressionMethod method) {
	if (_compression != TILE_UNCOMPRESSED)
		return false;

	_compression = method;

	TileData tile_data;
	tile_data.reserve(_tileData.size());

	TileOffsets tile_offsets;
	tile_offsets.reserve(_tileOffsets.size());
	tile_offsets.push_back(0);

	TileData tile_vector = TileData(GR_TILE_SPRITE_SIZE * 4, 0);

	int count = tileCount();
	for (int i = 0; i < count; i++) {
		if (_progressHandler) {
			int percent_done = 100 * (i + 1) / count;
			(*_progressHandler)(percent_done, _progressHandlerContext);
		}

		uint32 *data = &*_tileData.begin() + i * GR_TILE_SPRITE_SIZE;

		uint32 offs = tile_offsets.back();
		uint32 sz = grTileSprite::compress(data, &*tile_vector.begin(), method);
		tile_data.insert(tile_data.end(), tile_vector.begin(), tile_vector.begin() + sz);
		tile_offsets.push_back(offs + sz);
	}

	_tileData.swap(tile_data);
	_tileOffsets.swap(tile_offsets);

	return true;
}


grTileSprite grTileAnimation::getTile(int tile_index) const {
	debugC(3, kDebugTemp, "The tile index is given by %d", tile_index);
	static uint32 tile_buf[GR_TILE_SPRITE_SIZE];

	switch (_compression) {
	case TILE_UNCOMPRESSED:
		return grTileSprite(&*_tileData.begin() + _tileOffsets[tile_index]);
	default:
		if (tile_index >= (int)_tileOffsets.size()) {
			warning("grTileAnimation::getTile(): Too big tile index %d >= %d", tile_index, _tileOffsets.size());
			break;
		}
		if (_tileOffsets[tile_index] >= _tileData.size()) {
			warning("grTileAnimation::getTile(): Too big tile offset %d (%d >= %d)", tile_index, _tileOffsets[tile_index], _tileData.size());
			break;
		}
		if (!grTileSprite::uncompress(&*_tileData.begin() + _tileOffsets[tile_index], GR_TILE_SPRITE_SIZE, tile_buf, _compression)) {
			warning("Unknown compression algorithm");
		}
	}

	return grTileSprite(tile_buf);
}

void grTileAnimation::addFrame(const uint32 *frame_data) {
	TileData tile_vector = TileData(GR_TILE_SPRITE_SIZE, 0);
	TileData tile_vector2 = TileData(GR_TILE_SPRITE_SIZE * 4, 0);

	if (_progressHandler) {
		int percent_done = 100 * (_frameIndex.size() / (_frameTileSize.x * _frameTileSize.y) + 1) / (_frameCount ? _frameCount : 1);
		(*_progressHandler)(percent_done, _progressHandlerContext);
	}

	for (int i = 0; i < _frameTileSize.y; i++) {
		for (int j = 0; j < _frameTileSize.x; j++) {
			Common::fill(tile_vector.begin(), tile_vector.end(), 0);

			const uint32 *data_ptr = frame_data + j * GR_TILE_SPRITE_SIZE_X
			                           + i * GR_TILE_SPRITE_SIZE_Y * _frameSize.x;

			uint32 *tile_ptr = &tile_vector[0];
			for (int y = 0; y < GR_TILE_SPRITE_SIZE_Y; y++) {
				if (y + i * GR_TILE_SPRITE_SIZE_Y >= _frameSize.y) break;

				for (int x = 0; x < GR_TILE_SPRITE_SIZE_X; x++) {
					if (x + j * GR_TILE_SPRITE_SIZE_X >= _frameSize.x) break;
					tile_ptr[x] = data_ptr[x];
				}

				data_ptr += _frameSize.x;
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
				uint32 sz = GR_TILE_SPRITE_SIZE;
				uint32 offs = _tileOffsets.back();

				_tileData.insert(_tileData.end(), tile_vector.begin(), tile_vector.end());
				_tileOffsets.push_back(offs + sz);
				_frameIndex.push_back(tile_count);
			} else
				_frameIndex.push_back(tile_id);
		}
	}
}

bool grTileAnimation::load(Common::SeekableReadStream *fh, int version) {
	int dL = (version >= 105) ? 2 : 7;

	debugC(dL, kDebugLoad, "grTileAnimation::load(): pos start: %d", (int)fh->pos());

	_frameCount = fh->readSint32LE();
	_frameSize.x = fh->readSint32LE();
	_frameSize.y = fh->readSint32LE();
	_frameTileSize.x = fh->readSint32LE();
	_frameTileSize.y = fh->readSint32LE();
	uint32 size = fh->readUint32LE();

	debugC(dL, kDebugLoad, "grTileAnimation::load(): frameCount: %d, frame: %d x %d, tile: %d x %d, comp: %d", _frameCount, _frameSize.x, _frameSize.y,
		_frameTileSize.x, _frameTileSize.y, size);

	_compression = grTileCompressionMethod(size);

	if (version >= 105) {
		size = fh->readUint32LE();
		_scaleArray.resize(size);

		debugC(dL, kDebugLoad, "grTileAnimation::load(): pos: %d _scaleArray size: %u", (int)fh->pos() - 4, size);

		debugCN(dL + 1, kDebugLoad, "   ");

		for (uint i = 0; i < size; i++) {
			float scale = fh->readFloatLE();

			addScale(i, scale);

			debugCN(dL + 1, kDebugLoad, " %f, { %d x %d, [%d x %d], tiles: %d } ", _scaleArray[i]._scale,
					_scaleArray[i]._frameSize.x, _scaleArray[i]._frameSize.y, _scaleArray[i]._frameTileSize.x,
					_scaleArray[i]._frameTileSize.y, _scaleArray[i]._frameStart);
		}
		debugCN(dL + 1, kDebugLoad, "\n");
	}

	_frameSizeArray.resize(_frameCount);

	if (version < 106) {
		for (int i = 0; i < _frameCount; i++)
			_frameSizeArray[i] = _frameSize;
	} else {
		debugC(dL, kDebugLoad, "grTileAnimation::load(): pos: %d _frameSizeArray size: %u", (int)fh->pos() - 4, _frameCount);

		debugCN(dL + 1, kDebugLoad, "   ");

		for (int i = 0; i < _frameCount; i++) {
			_frameSizeArray[i].x = fh->readUint32LE();
			_frameSizeArray[i].y = fh->readUint32LE();

			debugCN(dL + 1, kDebugLoad, " %d x %d, ", _frameSizeArray[i].x, _frameSizeArray[i].y);
		}
		debugCN(dL + 1, kDebugLoad, "\n");
	}

	size = fh->readUint32LE();
	_frameIndex.resize(size);
	debugC(dL, kDebugLoad, "grTileAnimation::load(): pos: %d _frameIndex size: %u", (int)fh->pos() - 4, size);

	debugCN(dL + 2, kDebugLoad, "   ");
	for (uint i = 0; i < size; i++) {
		_frameIndex[i] = fh->readUint32LE();
		debugCN(dL + 2, kDebugLoad, " %5d ", _frameIndex[i]);

		if ((i + 1) % 20 == 0)
			debugCN(dL + 2, kDebugLoad, "\n   ");
	}
	debugCN(dL + 2, kDebugLoad, "\n");

	size = fh->readUint32LE();

	debugC(dL, kDebugLoad, "grTileAnimation::load(): pos: %d _tileOffsets size: %u", (int)fh->pos() - 4, size);

	_tileOffsets.resize(size);

	debugCN(dL + 2, kDebugLoad, "   ");
	for (uint i = 0; i < size; i++) {
		_tileOffsets[i] = fh->readUint32LE();
		debugCN(dL + 2, kDebugLoad, " %6d ", _tileOffsets[i]);

		if ((i + 1) % 20 == 0)
			debugCN(dL + 2, kDebugLoad, "\n   ");
	}
	debugCN(dL + 2, kDebugLoad, "\n");

	size = fh->readUint32LE();

	debugC(dL, kDebugLoad, "grTileAnimation::load(): pos: %d _tileData size: %u", (int)fh->pos() - 4, size);

	_tileData.resize(size);

	for (uint i = 0; i < size; i++)
		_tileData[i] = fh->readUint32LE();

	debugC(dL + 1, kDebugLoad, "  --> grTileAnimation::load(): pos: %d remaining: %d", (int)fh->pos(), (int)(fh->size() - fh->pos()));

	return true;
}

void grTileAnimation::drawFrame(const Vect2i &position, int32 frame_index, int32 mode, int closest_scale) const {
	debugC(3, kDebugGraphics, "grTileAnimation::drawFrame([%d, %d], frame: %d, mode: %d, scale_idx: %d)", position.x, position.y, frame_index, mode, closest_scale);

	Vect2i frameSize = _frameSize;
	Vect2i frameTileSize = _frameTileSize;
	int frameStart = 0;

	if (closest_scale != -1) {
		frameSize = _scaleArray[closest_scale]._frameSize;
		frameTileSize = _scaleArray[closest_scale]._frameTileSize;
		frameStart = _scaleArray[closest_scale]._frameStart;
	}

	int xx = position.x - frameSize.x / 2;
	int yy = position.y - frameSize.y / 2;

	int32 dx = GR_TILE_SPRITE_SIZE_X;
	int32 dy = GR_TILE_SPRITE_SIZE_Y;

	if (mode & GR_FLIP_HORIZONTAL) {
		xx += frameSize.x - GR_TILE_SPRITE_SIZE_X;
		dx = -dx;
	}
	if (mode & GR_FLIP_VERTICAL) {
		yy += frameSize.y - GR_TILE_SPRITE_SIZE_Y;
		dy = -dy;
	}

//	grDispatcher::instance()->Rectangle(position.x - _frameSize.x/2, position.y - _frameSize.y/2, _frameSize.x, _frameSize.y, 0xFFFFF, 0, GR_OUTLINED);

	const uint32 *index_ptr = &_frameIndex[frameStart] + frameTileSize.x * frameTileSize.y * frame_index;

	int x = xx, y = yy;
	for (int32 i = 0; i < frameTileSize.y; i++) {
		x = xx;

		for (int32 j = 0; j < frameTileSize.x; j++) {
			grDispatcher::instance()->putTileSpr(x, y, getTile(*index_ptr++), _hasAlpha, mode);
			x += dx;
		}

		y += dy;
	}
}

void grTileAnimation::drawFrame(const Vect2i &position, int frame_index, float angle, int mode) const {
	debugC(3, kDebugGraphics, "grTileAnimation::drawFrame([%d, %d], frame: %d, angle: %f, scale: %d)", position.x, position.y, frame_index, angle, mode);

	byte *buf = decode_frame_data(frame_index, -1);
	Vect2i pos = position - _frameSize / 2;

	grDispatcher::instance()->putSpr_rot(pos, _frameSize, buf, _hasAlpha, mode, angle);
}

void grTileAnimation::drawFrame(const Vect2i &position, int frame_index, float angle, const Vect2f &scale, int mode) const {
	debugC(3, kDebugGraphics, "grTileAnimation::drawFrame([%d, %d], frame: %d, angle: %f, scale: [%f, %f], mode: %d)", position.x, position.y, frame_index, angle, scale.x, scale.y, mode);

	byte *buf = decode_frame_data(frame_index, -1);
	Vect2i pos = position - _frameSize / 2;

	grDispatcher::instance()->putSpr_rot(pos, _frameSize, buf, _hasAlpha, mode, angle, scale);
}

//////////////////////////////////////////////////////////////////////
////  New version 105 & 106 code
//////////////////////////////////////////////////////////////////////

grTileSprite grTileAnimation::getFrameTile(int frame_number, int tile_index) const {
	return getTile(_frameIndex[tile_index + frame_number * _frameTileSize.x * _frameTileSize.y]);
}

bool grTileAnimation::hit(int frame_number, Vect2i &pos) const {
	int x = _frameSize.x / 2 + pos.x;
	int y = _frameSize.y / 2 + pos.y;

	if (x < 0 || x >= _frameSize.x || y < 0 || y >= _frameSize.y)
		return false;

	const byte *tile = (const byte *)getFrameTile(frame_number, x / 16 + y / 16 * _frameTileSize.x).data();

	return tile[64 * (y % 16) + 4 * (x % 16) + 3] < 0xC8u;
}

void grTileAnimation::drawFrame_scale(const Vect2i &position, int frame_index, float scale, int mode) const {
	debugC(3, kDebugGraphics, "grTileAnimation::drawFrame_scale([%d, %d], frame: %d, scale: %f, mode: %d)", position.x, position.y, frame_index, scale, mode);

	int closest_scale = find_closest_scale(&scale);

	if (wasFrameSizeChanged(frame_index, closest_scale, scale)) {
		byte *data = decode_frame_data(frame_index, closest_scale);

		Vect2i frameSize;

		if (closest_scale == -1)
			frameSize = _frameSize;
		else
			frameSize =_scaleArray[closest_scale]._frameSize;

		int x = position.x - round(float(frameSize.x) * scale) / 2;
		int y = position.y - round(float(frameSize.y) * scale) / 2;

		grDispatcher::instance()->putSpr_a(x, y, frameSize.x, frameSize.y, data, mode, scale);
	} else {
		drawFrame(position, frame_index, mode, closest_scale);
	}
}

void grTileAnimation::drawMask(const Vect2i &pos, int frame_index, uint32 mask_colour, int mask_alpha, int mode, int closest_scale) const {
	Vect2i frameSize;

	if (closest_scale == -1)
		frameSize = _frameSize;
	else
		frameSize =_scaleArray[closest_scale]._frameSize;

	byte *buf = decode_frame_data(frame_index, closest_scale);

	grDispatcher::instance()->putSprMask_a(pos.x - frameSize.x / 2, pos.y - frameSize.y / 2, frameSize.x, frameSize.y, buf, mask_colour, mask_alpha, mode);
}

void grTileAnimation::drawMask_scale(const Vect2i &pos, int frame_index, uint32 mask_colour, int mask_alpha, float scale, int mode) const {
	int closest_scale = find_closest_scale(&scale);

	if (wasFrameSizeChanged(frame_index, closest_scale, scale)) {
		byte *buf = decode_frame_data(frame_index, closest_scale);

		Vect2i frameSize;

		if (closest_scale == -1)
			frameSize = _frameSize;
		else
			frameSize =_scaleArray[closest_scale]._frameSize;

		int x = pos.x - (int)((float)(frameSize.x / 2) * scale);
		int y = pos.y - (int)((float)(frameSize.y / 2) * scale);

		grDispatcher::instance()->putSprMask_a(x, y, frameSize.x, frameSize.y, buf, mask_colour, mask_alpha, mode, scale);
	} else {
		drawMask(pos, frame_index, mask_colour, mask_alpha, mode, closest_scale);
	}
}

void grTileAnimation::drawMask_rot(const Vect2i &pos, int frame_index, uint32 mask_colour, int mask_alpha, float angle, int mode) const {
	byte *buf = decode_frame_data(frame_index, -1);

	grDispatcher::instance()->putSprMask_rot(Vect2i(pos.x - _frameSize.x / 2, pos.y - _frameSize.y / 2), _frameSize, buf, _hasAlpha, mask_colour, mask_alpha, mode, angle);
}

void grTileAnimation::drawMask_rot(const Vect2i &pos, int frame_index, uint32 mask_colour, int mask_alpha, float angle, Vect2f scale, int mode) const {
	byte *buf = decode_frame_data(frame_index, -1);

	int x = pos.x - (int)((float)(_frameSize.x / 2) * scale.x);
	int y = pos.y - (int)((float)(_frameSize.y / 2) * scale.y);

	grDispatcher::instance()->putSprMask_rot(Vect2i(x, y), _frameSize, buf, _hasAlpha, mask_colour, mask_alpha, mode, angle, scale);
}

void grTileAnimation::drawContour(const Vect2i &pos, int frame_index, uint32 color, int mode, int closest_scale) const {
	Vect2i frameSize;

	if (closest_scale == -1)
		frameSize = _frameSize;
	else
		frameSize =_scaleArray[closest_scale]._frameSize;

	byte *buf = decode_frame_data(frame_index, closest_scale);

	grDispatcher::instance()->drawSprContour_a(pos.x - frameSize.x / 2, pos.y - frameSize.y / 2, frameSize.x, frameSize.y, buf, color, mode);
}

void grTileAnimation::drawContour(const Vect2i &pos, int frame_index, uint32 color, float scale, int mode) const {
	int closest_scale = find_closest_scale(&scale);

	if (wasFrameSizeChanged(frame_index, closest_scale, scale)) {
		byte *data = decode_frame_data(frame_index, closest_scale);

		Vect2i frameSize;

		if (closest_scale == -1)
			frameSize = _frameSize;
		else
			frameSize =_scaleArray[closest_scale]._frameSize;

		int x = pos.x - (int)((float)(frameSize.x / 2) * scale);
		int y = pos.y - (int)((float)(frameSize.y / 2) * scale);

		grDispatcher::instance()->drawSprContour_a(x, y, frameSize.x, frameSize.y, data, color, mode, scale);
	} else {
		drawContour(pos, frame_index, color, mode, closest_scale);
	}
}

void grTileAnimation::addScale(int i, float scale) {
	_scaleArray[i]._scale = scale;
	_scaleArray[i]._frameSize.x = round((double)_frameSize.x * scale);
	_scaleArray[i]._frameSize.y = round((double)_frameSize.y * scale);
	_scaleArray[i]._frameTileSize.x = (_scaleArray[i]._frameSize.x + 15) / 16;
	_scaleArray[i]._frameTileSize.y = (_scaleArray[i]._frameSize.y + 15) / 16;

	if (i == 0)
		_scaleArray[i]._frameStart = _frameTileSize.x * _frameTileSize.y * _frameCount;
	else
		_scaleArray[i]._frameStart = _scaleArray[i - 1]._frameStart
				+ _frameCount * _scaleArray[i - 1]._frameTileSize.y * _scaleArray[i - 1]._frameTileSize.x;
}

byte *grTileAnimation::decode_frame_data(int frame_index, int closest_scale) const {
	Vect2i frameSize;

	if (closest_scale == -1)
		frameSize = _frameSize;
	else
		frameSize = _scaleArray[closest_scale]._frameSize;

	Vect2i frameTileSize;
	if (closest_scale == -1)
		frameTileSize = _frameTileSize;
	else
		frameTileSize = _scaleArray[closest_scale]._frameTileSize;

	int frameStart;
	if (closest_scale == -1)
		frameStart = 0;
	else
		frameStart = _scaleArray[closest_scale]._frameStart;

	byte *buf = (byte *)grDispatcher::instance()->temp_buffer(frameSize.x * frameSize.y * 4);

	const uint32 *index_ptr = &_frameIndex[frameStart] + frameTileSize.x * frameTileSize.y * frame_index;

	for (int i = 0; i < frameTileSize.y; i++) {
		for (int j = 0; j < frameTileSize.x; j++) {
			byte *buf_ptr = buf + (i * frameSize.x * GR_TILE_SPRITE_SIZE_Y + j * GR_TILE_SPRITE_SIZE_X) * 4;

			const byte *data_ptr = (const byte *)getTile(*index_ptr++).data();
			int dx = MIN(frameSize.x - j * GR_TILE_SPRITE_SIZE_X, GR_TILE_SPRITE_SIZE_X) * 4;
			int dy = MIN(frameSize.y - i * GR_TILE_SPRITE_SIZE_Y, GR_TILE_SPRITE_SIZE_Y);
			for (int k = 0; k < dy; k++) {
				memcpy(buf_ptr, data_ptr, dx);
				data_ptr += GR_TILE_SPRITE_SIZE_X * 4;
				buf_ptr += frameSize.x * 4;
			}
		}
	}

	return buf;
}

int grTileAnimation::find_closest_scale(float *scale) const {
	int idx = -1;
	float temp = 1.0;

	for (uint i = 0; i < _scaleArray.size(); i++) {
		if (fabs(*scale - _scaleArray[i]._scale) < fabs(*scale - temp)) {
			idx = i;
			temp = _scaleArray[i]._scale;
		}
	}

	if (idx != -1)
		*scale = *scale / temp;

	return idx;
}

bool grTileAnimation::wasFrameSizeChanged(int frame_index, int scaleIdx, float scale) const {
	int sx = _frameSizeArray[frame_index].x;
	int sy = _frameSizeArray[frame_index].y;

	float newScale;

	if (scaleIdx == -1)
		newScale = 1.0;
	else
		newScale = _scaleArray[scaleIdx]._scale;

	if ((int)((float)sx * newScale * scale) == sx &&
		(int)((float)sy * newScale * scale) == sy)
		return false;

	return true;
}

Graphics::ManagedSurface *grTileAnimation::dumpFrameTiles(int frame_index, float scale) const {
	int closest_scale = find_closest_scale(&scale);

	Vect2i frameSize;

	if (closest_scale == -1)
		frameSize = _frameSize;
	else
		frameSize = _scaleArray[closest_scale]._frameSize;

	Vect2i frameTileSize;
	if (closest_scale == -1)
		frameTileSize = _frameTileSize;
	else
		frameTileSize = _scaleArray[closest_scale]._frameTileSize;

	int frameStart;
	if (closest_scale == -1)
		frameStart = 0;
	else
		frameStart = _scaleArray[closest_scale]._frameStart;

	int w = frameTileSize.x * (GR_TILE_SPRITE_SIZE_X + 1);
	int h = frameTileSize.y * (GR_TILE_SPRITE_SIZE_Y + 1);

	Graphics::ManagedSurface *dstSurf = new Graphics::ManagedSurface(w, h, g_engine->_pixelformat);

	int idx = frameStart + frameTileSize.x * frameTileSize.y * frame_index;

	for (int i = 0; i < frameTileSize.y; i++) {
		for (int j = 0; j < frameTileSize.x; j++) {
			if (idx >= (int)_frameIndex.size()) {
				warning("grTileAnimation::dumpFrameTiles(): overflow of frame index (%d > %d)", idx, _frameIndex.size());
				break;
			}

			const byte *src = (const byte *)getTile(_frameIndex[idx++]).data();

			for (int yy = 0; yy < GR_TILE_SPRITE_SIZE_Y; yy++) {
				uint16 *dst = (uint16 *)dstSurf->getBasePtr(j * (GR_TILE_SPRITE_SIZE_X + 1), i * (GR_TILE_SPRITE_SIZE_Y + 1) + yy);

				for (int xx = 0; xx < GR_TILE_SPRITE_SIZE_X; xx++) {
					*dst = grDispatcher::instance()->make_rgb565u(src[2], src[1], src[0]);
					dst++;
					src += 4;
				}
			}
		}
	}

	return dstSurf;
}


Graphics::ManagedSurface *grTileAnimation::dumpTiles(int tilesPerRow) const {
	int w = tilesPerRow;
	int h = (_tileOffsets.size() + tilesPerRow - 1) / tilesPerRow;

	Graphics::ManagedSurface *dstSurf = new Graphics::ManagedSurface(w * (GR_TILE_SPRITE_SIZE_X + 1), h * (GR_TILE_SPRITE_SIZE_Y + 1), g_engine->_pixelformat);

	int index = 0;

	int x = 0, y = 0;
	for (int32 i = 0; i < h; i++) {
		x = 0;

		for (int32 j = 0; j < w; j++) {
			grDispatcher::instance()->putTileSpr(x, y, getTile(index++), _hasAlpha, 0, dstSurf, false);
			x += GR_TILE_SPRITE_SIZE_X + 1;

			if (index >= (int)_tileOffsets.size())
				break;
		}

		if (index >= (int)_tileOffsets.size())
			break;

		y += GR_TILE_SPRITE_SIZE_X + 1;
	}

	return dstSurf;
}

void grTileAnimation::dumpTiles(Common::Path basename, int tilesPerRow) const {
	Common::Path path = Common::Path(Common::String::format("dumps/%s.tiles.png", transCyrillic(basename.baseName())));

	Graphics::ManagedSurface *dstSurf = dumpTiles(tilesPerRow);

	Common::DumpFile bitmapFile;
	bitmapFile.open(path, true);
	Image::writePNG(bitmapFile, *(dstSurf->surfacePtr()));
	bitmapFile.close();

	warning("Dumped tile %s of %d x %d", path.toString().c_str(), dstSurf->w, dstSurf->h);

	delete dstSurf;
}

} // namespace QDEngine
