// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003-2004 The ScummVM-Residual Team (www.scummvm.org)
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

#ifndef SMUSH_PLAYER_H
#define SMUSH_PLAYER_H

// Use experimental (probably non-functional) in-memory zlib decompression.
// Leave undefined to use WORKING tempfile version.
//#define ZLIB_MEMORY

#include "bits.h"
#include "debug.h"
#include <cstring>
#include <zlib.h>

#include "blocky16.h"
#include "mixer/mixer.h"

class File {
private:

	FILE * _handle;
	bool _ioFailed;
	uint8 _encbyte;
	char *_name;	// For debugging

	static FILE *fopenNoCase(const char *filename, const char *directory, const char *mode);

public:
	enum {
		kFileReadMode = 1,
	};
	
	File();
	virtual ~File();
	bool open(const char *filename, const char *directory = NULL, int mode = kFileReadMode, byte encbyte = 0);
	void close();
	bool isOpen();
	bool ioFailed();
	void clearIOFailed();
	bool eof();
	uint32 pos();
	uint32 size();
	const char *name() const { return _name; }
	void seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *ptr, uint32 size);
	uint8 readByte();
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint16 readUint16BE();
	uint32 readUint32BE();
	void setEnc(byte value) { _encbyte = value; }
};

class zlibFile {
private:
	FILE *_handle;
	z_stream stream;	// zlib stream
	uint32 usedBuffer;	// how much of outBuf has been processed by ::read*()
	char inBuf[1024], outBuf[1024]; // Buffers for decompression
	bool fillZlibBuffer();

public:
	zlibFile();
	virtual ~zlibFile();
	bool open(const char *filename);
	void close();
	bool isOpen();
	bool eof();
	uint32 pos();
	uint32 size();
	void seek(int32 offs, int whence = SEEK_SET);

	uint32 read(void *ptr, uint32 size);
	uint8 readByte();
	uint16 readUint16LE();
	uint32 readUint32LE();
	uint16 readUint16BE();
	uint32 readUint32BE();
};

class Smush {
private:
	int32 _nbframes;
	Blocky16 _blocky16;
	#ifdef ZLIB_MEMORY
		zlibFile _file;
	#else
		File _file;
	#endif
	PlayingSoundHandle _soundHandle;

	int32 _frame;
	bool _updateNeeded;
	int32 _speed;
	int _channels;
	int _freq;
	bool _videoFinished;
	bool _videoPause;
	int _x, _y;
	int _width, _height;
	byte *_dst;
	byte *_buf;

public:
	Smush();
	~Smush();

	bool play(const char *filename, int x, int y);
	void stop() { deinit(); }
	void pause(bool pause) { _videoPause = pause; }
	bool isPlaying() { return !_videoFinished; }
	bool isUpdateNeeded() { return _updateNeeded; }
	byte *getDstPtr() { return _buf; }
	int getX() { return _x; }
	int getY() { return _y; }
	int getWidth() {return _width; }
	int getHeight() { return _height; }
	void clearUpdateNeeded() { _updateNeeded = false; }
	bool isFullSize() { return (_width == 640 && _height == 480); }

private:
	static void timerCallback(void *ptr);
	void parseNextFrame();
	void handleFramesHeader();
	void handleFrame();
	void handleBlocky16(byte *src);
	void handleWave(const byte *src, uint32 size);
	void init();
	void deinit();
	bool setupAnim(const char *file, int x, int y);
	void updateGLScreen();
};

extern Smush *g_smush;

#endif
