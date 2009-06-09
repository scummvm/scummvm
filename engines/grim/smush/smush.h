/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GRIM_SMUSH_PLAYER_H
#define GRIM_SMUSH_PLAYER_H

#ifdef __SYMBIAN32__
#include <zlib\zlib.h>
#else
#include <zlib.h>
#endif

#include "engines/grim/smush/blocky16.h"

#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Grim {

struct SavePos {
	uint32 filePos;
	z_stream streamBuf;
	byte *tmpBuf;
};

class zlibFile {
private:
	Common::File *_handle;
	z_stream _stream;	// Zlib stream
	byte *_inBuf;		// Buffer for decompression
	bool _fileDone;

public:
	zlibFile();
	~zlibFile();
	bool setPos(struct SavePos *pos);
	bool open(const char *filename);
	struct SavePos *getPos();
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
	Audio::SoundHandle _soundHandle;
	Audio::AppendableAudioStream *_stream;

	int32 _frame;
	bool _updateNeeded;
	int32 _speed;
	int32 _movieTime;
	int _channels;
	int _freq;
	bool _videoFinished;
	bool _videoPause;
	bool _videoLooping;
	struct SavePos *_startPos;
	int _x, _y;
	int _width, _height;
	byte *_internalBuffer, *_externalBuffer;

public:
	Smush();
	~Smush();

	bool play(const char *filename, bool looping, int x, int y);
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
	bool setupAnim(const char *file, bool looping, int x, int y);
	void updateGLScreen();
};

extern Smush *g_smush;

} // end of namespace Grim

#endif
