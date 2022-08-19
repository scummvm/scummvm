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

#include "bladerunner/zbuffer.h"

#include "bladerunner/decompress_lzo.h"

#include "common/debug.h"

namespace BladeRunner {

void ZBufferDirtyRects::reset() {
	_count = 0;
}

bool ZBufferDirtyRects::add(Common::Rect rect) {
	if (_count == MAX_DIRTY_RECTS)
		return false;

	_rects[_count++] = rect;
	if (_count > 1) {
		extendExisting();
	}
	return true;
}

void ZBufferDirtyRects::extendExisting() {
	if (_count < 2)
		return;

	Common::Rect last = _rects[_count - 1];

	int i;
	for (i = 0; i != _count - 1; ++i) {
		if (last.intersects(_rects[i])) {
			_rects[i].extend(last);
			--_count;
			break;
		}
	}
}

int ZBufferDirtyRects::getCount() const {
	return _count;
}

bool ZBufferDirtyRects::popRect(Common::Rect *rect) {
	if (_count == 0)
		return false;

	*rect = _rects[--_count];
	return true;
}

ZBuffer::ZBuffer() {
	_zbuf1 = nullptr;
	_zbuf2 = nullptr;
	_dirtyRects = new ZBufferDirtyRects();
	_width = 0;
	_height = 0;
	enable();
}

ZBuffer::~ZBuffer() {
	delete[] _zbuf2;
	delete[] _zbuf1;
	delete _dirtyRects;
}

void ZBuffer::init(int width, int height) {
	_width = width;
	_height = height;

	_zbuf1 = new uint16[width * height];
	_zbuf2 = new uint16[width * height];
}

static int decodePartialZBuffer(const uint8 *src, uint16 *curZBUF, uint32 srcLen) {
	const uint32 dstSize = BladeRunnerEngine::kOriginalGameWidth * BladeRunnerEngine::kOriginalGameHeight;
	uint32 dstRemain = dstSize;

	uint16 *curzp = curZBUF;
	const uint8 *inp8 = src;

	while (dstRemain && (inp8 - src) < (ptrdiff_t)srcLen) {
		uint32 count = READ_LE_UINT16(inp8);
		inp8+=2;

		if (count & 0x8000) {
			count = MIN(count & 0x7fff, dstRemain);
			dstRemain -= count;

			while (count--) {
				uint16 value = READ_LE_UINT16(inp8);
				inp8+=2;
				if (value)
					*curzp = value;
				++curzp;
			}
		} else {
			count = MIN(count, dstRemain);
			dstRemain -= count;
			uint16 value = READ_LE_UINT16(inp8);
			inp8+=2;

			if (!value) {
				curzp += count;
			} else {
				while (count--)
					*curzp++ = value;
			}
		}
	}
	return dstSize - dstRemain;
}

bool ZBuffer::decodeData(const uint8 *data, int size) {
	if (_disabled) {
		return false;
	}

	uint32 width, height, complete;// , unk0;

	width    = READ_LE_UINT32(data + 0);
	height   = READ_LE_UINT32(data + 4);
	complete = READ_LE_UINT32(data + 8);
	/*unk0 =*/ READ_LE_UINT32(data + 12);

	if (width != (uint32)_width || height != (uint32)_height) {
		warning("zbuffer size mismatch (%d, %d) != (%d, %d)", _width, _height, width, height);
		return false;
	}

	data += 16;
	size -= 16;

	if (complete) {
		resetUpdates();
		size_t zbufOutSize;
		decompress_lzo1x(data, size, (uint8 *)_zbuf1, &zbufOutSize);
#ifdef SCUMM_BIG_ENDIAN
		// As the compression is working with 8-bit data, on big-endian architectures we have to switch order of bytes in uncompressed data
		uint8 *rawZbuf = (uint8 *)_zbuf1;
		for (size_t i = 0; i < zbufOutSize - 1; i += 2) {
			SWAP(rawZbuf[i], rawZbuf[i + 1]);
		}
#endif
		memcpy(_zbuf2, _zbuf1, 2 * _width * _height);
	} else {
		clean();
		decodePartialZBuffer(data, _zbuf1, size);
		decodePartialZBuffer(data, _zbuf2, size);
	}

	return true;
}

uint16 *ZBuffer::getData() const {
	return _zbuf2;
}

uint16 ZBuffer::getZValue(int x, int y) const {
	assert(x >= 0 && x < _width);
	assert(y >= 0 && y < _height);

	if (_zbuf2 == nullptr) {
		return 0;
	}

	return _zbuf2[y * _width + x];
}

void ZBuffer::blit(Common::Rect rect) {
	int line_width = rect.width();

	for (int y = rect.top; y != rect.bottom; ++y) {
		int offset = y * _width + rect.left;
		memcpy(_zbuf2 + offset, _zbuf1 + offset, 2 * line_width);
	}
}

void ZBuffer::mark(Common::Rect rect) {
	assert(rect.isValidRect());

	// debug("mark %d, %d, %d, %d", rect.top, rect.right, rect.bottom, rect.left);
	rect.clip(_width, _height);
	_dirtyRects->add(rect);
}

void ZBuffer::clean() {
	Common::Rect rect;
	while (_dirtyRects->popRect(&rect)) {
		// debug("blit %d, %d, %d, %d", rect.top, rect.right, rect.bottom, rect.left);
		blit(rect);
	}
}

void ZBuffer::resetUpdates() {
	_dirtyRects->reset();
}

void ZBuffer::disable() {
	_disabled = true;
}

void ZBuffer::enable() {
	_disabled = false;
}

} // End of namespace BladeRunner
