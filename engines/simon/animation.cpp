/* ScummVM - Scumm Interpreter
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

#include "common/endian.h"
#include "common/system.h"

#include "simon/animation.h"

#ifdef USE_ZLIB
#include <zlib.h>
#endif

bool MoviePlayer::open(const char *filename) {
	bool opened = false;
	char filename2[100];

	_leftButtonDown = false;
	_rightButtonDown = false;

	// Change file extension to dxa
	strcpy(filename2, filename);
	int len = strlen(filename2) - 4;
	filename2[len++] = '.';
	filename2[len++] = 'd';
	filename2[len++] = 'x';
	filename2[len++] = 'a';
	
	if (_fd.open(filename2)) {
		uint32 tag = _fd.readUint32BE();
		if (tag  == MKID_BE('DEXA')) {
			_fd.readByte();
			_framesCount = _fd.readUint16BE();
			_frameTicks = _fd.readUint32BE();
			if (_frameTicks > 100) {
				_frameTicks = 100;
			}
			_width = _fd.readUint16BE();
			_height = _fd.readUint16BE();
			debug(5, "frames_count %d width %d height %d ticks %d", _framesCount, _width, _height, _frameTicks);
			_frameSize = _width * _height;
			_frameBuffer1 = (uint8 *)malloc(_frameSize);
			_frameBuffer2 = (uint8 *)malloc(_frameSize);
			if (!_frameBuffer1 || !_frameBuffer2) {
				error("error allocating frame tables, size %d\n", _frameSize);
				close();
			} else {
				tag = _fd.readUint32BE();
				if (tag  == MKID_BE('WAVE')) {
					uint32 size = _fd.readUint32BE();
					debug(5, "Wave_size = %d", size);
					// TODO: Preload wave data
					_fd.seek(size + 23);
				}
				_currentFrame = 0;
				opened = true;
			}
		}
	}
	return opened;
}

void MoviePlayer::close() {
	_fd.close();
	free(_frameBuffer1);
	free(_frameBuffer2);
}

void MoviePlayer::play() {
	g_system->clearScreen();

	while (_currentFrame < _framesCount) {
		handleNextFrame();
		++_currentFrame;
	}

	g_system->clearScreen();
}

void MoviePlayer::handleNextFrame() {
	uint32 tag = _fd.readUint32BE();
	if (tag == MKID_BE('CMAP')) {
		uint8 rgb[768];
		byte palette[1024];
		byte *p = palette;

		_fd.read(rgb, ARRAYSIZE(rgb));
		for (int i = 0; i <= 256; i++) {
			*p++ = rgb[i * 3 + 0];
			*p++ = rgb[i * 3 + 1];
			*p++ = rgb[i * 3 + 2];
			*p++ = 0;
		}
		g_system->setPalette(palette, 0, 256);
	}

	tag = _fd.readUint32BE();
	if (tag == MKID_BE('FRAM')) {
		uint8 type = _fd.readByte();
		uint32 size = _fd.readUint32BE();
		debug(5, "frame %d type %d size %d", _currentFrame, type, size);
		_fd.read(_frameBuffer2, size);
		switch (type) {
		case 2:
		case 3:
			decodeZlib(_frameBuffer2, size, _frameSize);
			break;
		case 4:
		case 5:
			decode0(_frameBuffer2, size);
			break;
		case 6:
		case 7:
			decode2(_frameBuffer2, size, _frameSize);
			break;
		}
		if (type == 2 || type == 4 || type == 6) {
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

	if (_width == 640 && _height == 480)
		g_system->copyRectToScreen(_frameBuffer1, _width, 0, 0, _width, _height);
	else
		g_system->copyRectToScreen(_frameBuffer1, _width, 128, 100, _width, _height);
	g_system->updateScreen();
	delay(_frameTicks);
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

void MoviePlayer::decode0(uint8 *data, int size) {
	error("decode0");
}

void MoviePlayer::decode2(uint8 *data, int size, int totalSize) {
	error("decode2");
}

void MoviePlayer::delay(uint amount) {
	OSystem::Event event;

	uint32 start = g_system->getMillis();
	uint32 cur = start;
	uint this_delay;

	do {
		while (g_system->pollEvent(event)) {
			switch (event.type) {
			case OSystem::EVENT_LBUTTONDOWN:
				_leftButtonDown = true;
				break;
			case OSystem::EVENT_RBUTTONDOWN:
				_rightButtonDown = true;
				break;
			case OSystem::EVENT_LBUTTONUP:
				_leftButtonDown = false;
				break;
			case OSystem::EVENT_RBUTTONUP:
				_rightButtonDown = false;
				break;
			case OSystem::EVENT_QUIT:
				g_system->quit();
				break;
			default:
				break;
			}
		}

		if (_leftButtonDown && _rightButtonDown) {
			_currentFrame = _framesCount;
			amount = 0;
		}

		if (amount == 0)
			break;

		{
			this_delay = 20 * 1;
			if (this_delay > amount)
				this_delay = amount;
			g_system->delayMillis(this_delay);
		}
		cur = g_system->getMillis();
	} while (cur < start + amount);
}
