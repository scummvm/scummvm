/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#include "scumm/he/animation_he.h"
#include "scumm/he/intern_he.h"

#ifdef USE_ZLIB
#include <zlib.h>
#endif

namespace Scumm {

MoviePlayer::MoviePlayer(ScummEngine_v90he *vm)
	: _vm(vm) {

	_frameBuffer1 = 0;
	_frameBuffer2 = 0;

	_width = 0;
	_height = 0;

	_frameSize = 0;
	_framesCount = 0;
	_frameNum = 0;
	_framesPerSec = 0;
	_frameTicks = 0;

	_flags = 0;
	_wizResNum = 0;
}

MoviePlayer::~MoviePlayer() {
}

int MoviePlayer::getWidth() {
	if (_fd.isOpen() == false)
		return 0;
	return _width;
}

int MoviePlayer::getHeight() {
	if (_fd.isOpen() == false)
		return 0;
	return _height;
}

int MoviePlayer::getCurFrame() {
	if (_fd.isOpen() == false)
		return -1;
	return _frameNum;
}

int MoviePlayer::getFrameCount() {
	if (_fd.isOpen() == false)
		return 0;
	return _framesCount;
}

int MoviePlayer::getImageNum() {
	if (_fd.isOpen() == false)
		return 0;
	return _wizResNum;
}

int MoviePlayer::load(const char *filename, int flags, int image) {
	char videoName[100];
	uint32 tag;
	int32 frameRate;

	if (_fd.isOpen() == true) {
		close();
	}

	// Change file extension to dxa
	strcpy(videoName, filename);
	int len = strlen(videoName) - 3;
	videoName[len++] = 'd';
	videoName[len++] = 'x';
	videoName[len++] = 'a';
	
	if (_fd.open(videoName) == false) {
		warning("Failed to load video file %s", videoName);
		return -1;
	} 
	debug(1, "Playing video %s", videoName);

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

	// Skip sound tag
	_fd.readUint32BE();

	_frameSize = _width * _height;
	_frameBuffer1 = (uint8 *)malloc(_frameSize);
	_frameBuffer2 = (uint8 *)malloc(_frameSize);
	if (!_frameBuffer1 || !_frameBuffer2) {
		error("error allocating frame tables, size %d\n", _frameSize);
	}

	if (flags & 2) {
		_vm->_wiz->createWizEmptyImage(image, 0, 0, _width, _height);
	}

	_frameNum = 0;

	_flags = flags;
	_wizResNum = image;

	return 0;
}

void MoviePlayer::close() {
	if (_fd.isOpen() == false)
		return;

	_fd.close();
	free(_frameBuffer1);
	free(_frameBuffer2);
}

void MoviePlayer::handleNextFrame() {
	if (_fd.isOpen() == false) {
		return;
	}

	VirtScreen *pvs = &_vm->virtscr[kMainVirtScreen];
	uint8 *dst;

	decodeFrame();

	if (_flags & 2) {
		uint8 *dstPtr = _vm->getResourceAddress(rtImage, _wizResNum);
		assert(dstPtr);
		dst = _vm->findWrappedBlock(MKID_BE('WIZD'), dstPtr, 0, 0);
		assert(dst);
		copyFrame(dst, 0, 0);
	} else if (_flags & 1) {
		dst = pvs->getBackPixels(0, 0);
		copyFrame(dst, 0, 0);
		
		Common::Rect imageRect(_width, _height);
		_vm->gdi.copyVirtScreenBuffers(imageRect);
	} else {
		dst = pvs->getPixels(0, 0);
		copyFrame(dst, 0, 0);

		_vm->markRectAsDirty(kMainVirtScreen, 0, 0, _width, _height);
	}

	_frameNum++;
	if (_frameNum == _framesCount) {
		close();
	}
}

void MoviePlayer::copyFrame(byte *dst, uint x, uint y) {
	uint h = _height;
	uint w = _width;

	dst += y * _vm->_screenWidth + x;
	byte *src = _frameBuffer1;

	do {
		memcpy(dst, src, w);
		dst += _vm->_screenWidth;
		src += _width;
	} while (--h);
}

void MoviePlayer::decodeZlib(uint8 *data, int size, int totalSize) {
#ifdef USE_ZLIB
	uint8 *temp = (uint8 *)malloc(size);
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

void MoviePlayer::decodeFrame() {
	uint32 tag;

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('CMAP')) {
		uint8 rgb[768];

		_fd.read(rgb, ARRAYSIZE(rgb));
		_vm->setPaletteFromPtr(rgb, 256);
	}

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('FRAM')) {
		uint8 type = _fd.readByte();
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
		if (type == 2) {
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

} // End of namespace Simon
