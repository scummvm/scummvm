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

#ifndef SMUSH_PLAYER_H
#define SMUSH_PLAYER_H

#include "stdafx.h"
#include "bits.h"
#include "debug.h"
#include "blocky16.h"

#include "mixer/mixer.h"
#include "mixer/audiostream.h"

#include <zlib.h>
#include <cstring>

struct SAVEPOS {
	long filePos;
	z_stream streamBuf;
	char *tmpBuf;
};

class zlibFile {
private:
	FILE *_handle;
	z_stream _stream;	// Zlib stream
	char *_inBuf;		// Buffer for decompression
	bool _fileDone;

public:
	zlibFile();
	~zlibFile();
	bool setPos(struct SAVEPOS *pos);
	bool open(const char *filename);
	struct SAVEPOS *getPos();
	void close();
	bool isOpen();

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
	zlibFile _file;
	PlayingSoundHandle _soundHandle;
	AppendableAudioStream *_stream;

 	int32 _frame;
	bool _updateNeeded;
	int32 _speed;
	int32 _movieTime;
	int _channels;
	int _freq;
	bool _videoFinished;
	bool _videoPause;
	bool _videoLooping;
	struct SAVEPOS *_startPos;
	int _x, _y;
	int _width, _height;
	byte *_internalBuffer, *_externalBuffer;

public:
	Smush();
	~Smush();

	bool play(const char *filename, int x, int y);
	void stop();
	void pause(bool pause) { _videoPause = pause; }
	bool isPlaying() { return !_videoFinished; }
	bool isUpdateNeeded() { return _updateNeeded; }
	byte *getDstPtr() { return _externalBuffer; }
	int getX() { return _x; }
	int getY() { return _y; }
	int getWidth() {return _width; }
	int getHeight() { return _height; }
	int getFrame() { return _frame; }
	void clearUpdateNeeded() { _updateNeeded = false; }
	int32 getMovieTime() { return _movieTime; }

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
