/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"
#include "graphics/dxa_player.h"
#include "common/util.h"

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace Graphics {

DXAPlayer::DXAPlayer() {
	_frameBuffer1 = 0;
	_frameBuffer2 = 0;

	_width = 0;
	_height = 0;

	_frameSize = 0;
	_framesCount = 0;
	_frameNum = 0;
	_framesPerSec = 0;
	_frameSkipped = 0;
	_frameTicks = 0;
}

DXAPlayer::~DXAPlayer() {
}

int DXAPlayer::getWidth() {
	if (!_fd.isOpen())
		return 0;
	return _width;
}

int DXAPlayer::getHeight() {
	if (!_fd.isOpen())
		return 0;
	return _height;
}

int DXAPlayer::getCurFrame() {
	if (!_fd.isOpen())
		return -1;
	return _frameNum;
}

int DXAPlayer::getFrameCount() {
	if (!_fd.isOpen())
		return 0;
	return _framesCount;
}

bool DXAPlayer::loadFile(const char *filename) {
	uint32 tag;
	int32 frameRate;

	if (!_fd.open(filename)) {
		return 0;
	}

	tag = _fd.readUint32BE();
	assert(tag == MKID_BE('DEXA'));

	_fd.readByte();
	_framesCount = _fd.readUint16BE();
	frameRate = _fd.readUint32BE();

	if (frameRate > 0)
		_framesPerSec = 1000 / frameRate;
	else if (frameRate < 0)
		_framesPerSec = 100000 / (-frameRate);
	else
		_framesPerSec = 10;

        if (frameRate < 0)
                _frameTicks = -frameRate / 100;
	else
		_frameTicks = frameRate;

	_width = _fd.readUint16BE();
	_height = _fd.readUint16BE();

	debug(2, "frames_count %d width %d height %d rate %d ticks %d", _framesCount, _width, _height, _framesPerSec, _frameTicks);

	_frameSize = _width * _height;
	_frameBuffer1 = (uint8 *)malloc(_frameSize);
	_frameBuffer2 = (uint8 *)malloc(_frameSize);
	if (!_frameBuffer1 || !_frameBuffer2) {
		error("error allocating frame tables, size %d\n", _frameSize);
	}

	_frameNum = 0;
	_frameSkipped = 0;

	return true;
}

void DXAPlayer::closeFile() {
	if (!_fd.isOpen())
		return;

	_fd.close();
	free(_frameBuffer1);
	free(_frameBuffer2);
}

void DXAPlayer::copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch) {
	uint h = _height;
	uint w = _width;

	dst += y * pitch + x;
	byte *src = _frameBuffer1;

	do {
		memcpy(dst, src, w);
		dst += pitch;
		src += _width;
	} while (--h);
}

void DXAPlayer::decodeZlib(byte *data, int size, int totalSize) {
#ifdef USE_ZLIB
	byte *temp = (byte *)malloc(size);
	if (temp) {
		memcpy(temp, data, size);

		z_stream d_stream;
		d_stream.zalloc = (alloc_func)0;
		d_stream.zfree = (free_func)0;
		d_stream.opaque = (voidpf)0;
		d_stream.next_in = temp;
		d_stream.avail_in = size;
		d_stream.total_in = size;
		d_stream.next_out = data;
		d_stream.avail_out = totalSize;
		inflateInit(&d_stream);
		inflate(&d_stream, Z_FINISH);
		inflateEnd(&d_stream);
		free(temp);
	}
#endif
}

#define BLOCKW 4
#define BLOCKH 4

void DXAPlayer::decodeNextFrame() {
	uint32 tag;

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('CMAP')) {
		byte rgb[768];

		_fd.read(rgb, ARRAYSIZE(rgb));
		setPalette(rgb);
	}

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('FRAM')) {
		byte type = _fd.readByte();
		uint32 size = _fd.readUint32BE();

		_fd.read(_frameBuffer2, size);

		switch (type) {
		case 2:
		case 3:
			decodeZlib(_frameBuffer2, size, _frameSize);
			break;
		default:
			error("decodeFrame: Unknown compression type %d", type);
		}
		if (type == 2 || type == 4) {
			memcpy(_frameBuffer1, _frameBuffer2, _frameSize);
		} else {
			for (int j = 0; j < _height; ++j) {
				for (int i = 0; i < _width; ++i) {
					const int offs = j * _width + i;
					_frameBuffer1[offs] ^= _frameBuffer2[offs];
				}
			}
		}
	}
}

} // End of namespace Graphics
