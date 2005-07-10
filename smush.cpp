// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2005 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "smush.h"
#include "timer.h"
#include "resource.h"
#include "engine.h"
#include "driver.h"

#include "mixer/mixer.h"

#include <cstring>
#include <zlib.h>

Smush *g_smush;
static uint16 smushDestTable[5786];

void Smush::timerCallback(void *) {
	g_smush->handleFrame();
}

Smush::Smush() {
	g_smush = this;
	_nbframes = 0;
	_internalBuffer = NULL;
	_externalBuffer = NULL;
	_width = 0;
	_height = 0;
	_speed = 0;
	_channels = -1;
	_freq = 22050;
	_videoFinished = false;
	_videoPause = true;
	_updateNeeded = false;
	_stream = NULL;
	_movieTime = 0;
	_frame = 0;
}

Smush::~Smush() {
	deinit();
}

void Smush::init() {
	_stream = NULL;
	_frame = 0;
	_movieTime = 0;
	_videoFinished = false;
	_videoPause = false;
	_updateNeeded = false;

	assert(!_internalBuffer);
	assert(!_externalBuffer);

	_internalBuffer = (byte *)malloc(_width * _height * 2);
	_externalBuffer = (byte *)malloc(_width * _height * 2);

	vimaInit(smushDestTable);
	g_timer->installTimerProc(&timerCallback, _speed, NULL);
}

void Smush::deinit() {
	g_timer->removeTimerProc(&timerCallback);

	if (_internalBuffer) {
		free(_internalBuffer);
		_internalBuffer = NULL;
	}
	if (_externalBuffer) {
		free(_externalBuffer);
		_externalBuffer = NULL;
	}

	_videoFinished = true;
	_videoPause = true;
	if (_stream) {
		_stream->finish();
		_stream = NULL;
		g_mixer->stopHandle(_soundHandle);
	}
 	_file.close();
}

void Smush::handleWave(const byte *src, uint32 size) {
	int16 *dst = new int16[size * _channels];
	decompressVima(src, dst, size * _channels * 2, smushDestTable);

	int flags = SoundMixer::FLAG_16BITS | SoundMixer::FLAG_AUTOFREE;
	if (_channels == 2)
		flags |= SoundMixer::FLAG_STEREO;

	if (!_stream) {
		_stream = makeAppendableAudioStream(_freq, flags, 500000);
		g_mixer->playInputStream(&_soundHandle, _stream, true);
	}
 	if (_stream)
		_stream->append((byte *)dst, size * _channels * 2);
}

void Smush::handleFrame() {
	uint32 tag;
	int32 size;
	int pos = 0;

	if (_videoPause)
		return;

	if (_videoFinished) {
		_videoPause = true;
		return;
	}

	tag = _file.readUint32BE();
	if (tag == MKID_BE('ANNO')) {
printf("Announcement!\n");
		size = _file.readUint32BE();
		for (int l = 0; l < size; l++)
			_file.readByte();
		tag = _file.readUint32BE();
	}

	assert(tag == MKID_BE('FRME'));
	size = _file.readUint32BE();
	byte *frame = (byte *)malloc(size);
	_file.read(frame, size);

	do {
		if (READ_BE_UINT32(frame + pos) == MKID_BE('Bl16')) {
			_blocky16.decode(_internalBuffer, frame + pos + 8);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (READ_BE_UINT32(frame + pos) == MKID_BE('Wave')) {
			int decompressed_size = READ_BE_UINT32(frame + pos + 8);

			if (decompressed_size < 0)
				handleWave(frame + pos + 8 + 4 + 8, READ_BE_UINT32(frame + pos + 8 + 8));
			else

				handleWave(frame + pos + 8 + 4, decompressed_size);
			pos += READ_BE_UINT32(frame + pos + 4) + 8;
		} else if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL) {
			error("Smush::handleFrame() unknown tag");
		}
	} while (pos < size);
	free(frame);

	memcpy(_externalBuffer, _internalBuffer, _width * _height * 2);
	_updateNeeded = true;

	_frame++;
	if (_frame == _nbframes) {
		_videoFinished = true;
		g_engine->setMode(ENGINE_MODE_NORMAL);
	}
	
	_movieTime += _speed / 1000;
}

void Smush::handleFramesHeader() {
	uint32 tag;
	int32 size;
	int pos = 0;

	tag = _file.readUint32BE();
	assert(tag == MKID_BE('FLHD'));
	size = _file.readUint32BE();
	byte *f_header = (byte*)malloc(size);
	_file.read(f_header, size);

	do {
		if (READ_BE_UINT32(f_header + pos) == MKID_BE('Bl16')) {
			pos += READ_BE_UINT32(f_header + pos + 4) + 8;
		} else if (READ_BE_UINT32(f_header + pos) == MKID_BE('Wave')) {
			_freq = READ_LE_UINT32(f_header + pos + 8);
			_channels = READ_LE_UINT32(f_header + pos + 12);
			pos += 20;
		} else if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL){
			error("Smush::handleFramesHeader() unknown tag");
		}
	} while (pos < size);
	free(f_header);
}

bool Smush::setupAnim(const char *file, int x, int y) {
	uint32 tag;
	int32 size;

	if (!_file.open(file))
		return false;

	tag = _file.readUint32BE();
	assert(tag == MKID_BE('SANM'));

	size = _file.readUint32BE();
	tag = _file.readUint32BE();
	assert(tag == MKID_BE('SHDR'));

	size = _file.readUint32BE();
	byte *s_header = (byte *)malloc(size);
	_file.read(s_header, size);
	_nbframes = READ_LE_UINT32(s_header + 2);

	int width = READ_LE_UINT16(s_header + 8);
	int height = READ_LE_UINT16(s_header + 10);
	if ((_width != width) || (_height != height)) {
		_blocky16.init(width, height);
	}

	_x = x;
	_y = y;
	_width = width;
	_height = height;

	_speed = READ_LE_UINT32(s_header + 14);
	free(s_header);

	return true;
}

void Smush::stop() { 
	deinit();
 	g_engine->setMode(ENGINE_MODE_NORMAL);
}

bool Smush::play(const char *filename, int x, int y) {
	deinit();

	// Load the video
	if (!setupAnim(filename, x, y))
		return false;

	handleFramesHeader();
	init();

	return true;
}

zlibFile::zlibFile() {
	_handle = NULL;
	_inBuf = NULL;
}

zlibFile::~zlibFile() {
	close();
}

bool zlibFile::open(const char *filename) {
	char flags = 0;
	_inBuf = (char *)calloc(1, 16385);

	if (_handle) {
		if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("zlibFile::open() File %s already opened", filename);
		return false;
	}

	if (filename == NULL || *filename == 0)
		return false;

	_handle = g_resourceloader->openNewStream(filename);
	if (!_handle) {
		if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
			warning("zlibFile::open() zlibFile %s not found", filename);
		return false;
	}

	// Read in the GZ header
	fread(_inBuf, 2, sizeof(char), _handle);				// Header
	fread(_inBuf, 1, sizeof(char), _handle);				// Method
	fread(_inBuf, 1, sizeof(char), _handle); flags = _inBuf[0];		// Flags
	fread(_inBuf, 6, sizeof(char), _handle);				// XFlags

	// Xtra & Comment
	if (((((flags & 0x04) != 0) || ((flags & 0x10) != 0))) && (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL))
		error("zlibFile::open() Unsupported header flag");

	if ((flags & 0x08) != 0) {					// Orig. Name
		do {
			fread(_inBuf, 1, sizeof(char), _handle);
		} while(_inBuf[0] != 0);
	}

	if ((flags & 0x02) != 0) // CRC
		fread(_inBuf, 2, sizeof(char), _handle);

	memset(_inBuf, 0, 16384);			// Zero buffer (debug)
	_stream.zalloc = NULL;
	_stream.zfree = NULL;
	_stream.opaque = Z_NULL;

	if (inflateInit2(&_stream, -15) != Z_OK && (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL))
		error("zlibFile::open() inflateInit2 failed");

	_stream.next_in = NULL;
	_stream.next_out = NULL;
	_stream.avail_in = 0;
	_stream.avail_out = 16384;

	return true;
}

void zlibFile::close() {
	if (_handle) {
		fclose(_handle);
		_handle = NULL;
	}

	if (_inBuf) {
 		free(_inBuf);
 		_inBuf = NULL;
	}
}

bool zlibFile::isOpen() {
	return _handle != NULL;
}

uint32 zlibFile::read(void *ptr, uint32 len) {
	int result = Z_OK;
	bool fileEOF = false;

	if (_handle == NULL) {
		if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_ERROR || debugLevel == DEBUG_ALL)
			error("zlibFile::read() File is not open!");
		return 0;
	}

	if (len == 0)
		return 0;

	_stream.next_out = (Bytef *)ptr;
	_stream.avail_out = len;

	_fileDone = false;
	while (_stream.avail_out != 0) {
		if (_stream.avail_in == 0) {	// !eof
	        	_stream.avail_in = fread(_inBuf, 1, 16384, _handle);
			if (_stream.avail_in == 0) {
				fileEOF = true;
				break;
			}
			_stream.next_in = (Byte *)_inBuf;
		}

		result = inflate(&_stream, Z_NO_FLUSH);
		if (result == Z_STREAM_END) {	// EOF
			if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("zlibFile::read() Stream ended");
			_fileDone = true;
			break;
		}
		if (result == Z_DATA_ERROR) {
			if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("zlibFile::read() Decompression error");
			_fileDone = true;
			break;
		}
		if (result != Z_OK || fileEOF) {
			if (debugLevel == DEBUG_SMUSH || debugLevel == DEBUG_WARN || debugLevel == DEBUG_ALL)
				warning("zlibFile::read() Unknown decomp result: %d/%d\n", result, fileEOF);
			_fileDone = true;
			break;
		}
	}

	return (int)(len - _stream.avail_out);
}
 
byte zlibFile::readByte() {
	unsigned char c;

	read(&c, 1);
	return c;
}

uint16 zlibFile::readUint16LE() {
	uint16 a = readByte();
	uint16 b = readByte();
	return a | (b << 8);
}

uint32 zlibFile::readUint32LE() {
	uint32 a = readUint16LE();
	uint32 b = readUint16LE();
	return (b << 16) | a;
}

uint16 zlibFile::readUint16BE() {
	uint16 b = readByte();
	uint16 a = readByte();
	return a | (b << 8);
}

uint32 zlibFile::readUint32BE() {
	uint32 b = readUint16BE();
	uint32 a = readUint16BE();
	return (b << 16) | a;
}

