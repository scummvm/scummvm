// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
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

#include "bits.h"
#include "debug.h"
#include <cstring>
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

class Smush {
private:
	int32 _nbframes;
	Blocky16 _blocky16;
	File _file;
	PlayingSoundHandle _soundHandle;

	int32 _frame;
	bool _alreadyInit;
	int32 _speed;
	int _channels;
	int _freq;
	bool _videoFinished;
	int _width, _height;
	uint8 *_dst;

public:
	Smush();
	~Smush();

	void play(const char *filename, const char *directory);

private:
	static void timerCallback(void *ptr);
	void parseNextFrame();
	void handleFramesHeader();
	void handleFrame();
	void handleBlocky16(byte *src);
	void handleWave(const byte *src, uint32 size);
	void init();
	void deinit();
	void setupAnim(const char *file, const char *directory);
	void updateGLScreen();
};

#endif
