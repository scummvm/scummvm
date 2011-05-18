/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#ifndef GRIM_SMUSH_PLAYER_H
#define GRIM_SMUSH_PLAYER_H

#include <zlib.h>

#include "common/file.h"

#include "engines/grim/movie/movie.h"
#include "engines/grim/movie/codecs/blocky8.h"
#include "engines/grim/movie/codecs/blocky16.h"

#include "audio/mixer.h"
#include "audio/audiostream.h"

namespace Grim {

class SaveGame;

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

class SmushPlayer : public MoviePlayer {
private:
	int32 _nbframes;
	Blocky8 _blocky8;
	Blocky16 _blocky16;
	zlibFile _file;

	byte _pal[0x300];
	int16 _deltaPal[0x300];
	byte _IACToutput[4096];
	int32 _IACTpos;

public:
	SmushPlayer();
	virtual ~SmushPlayer();

	bool play(const char *filename, bool looping, int x, int y);
	void stop();

	void saveState(SaveGame *state);
	void restoreState(SaveGame *state);

private:
	static void timerCallback(void *ptr);
	void parseNextFrame();
	void init();
	void deinit();
	void handleDeltaPalette(byte *src, int32 size);
	void handleFramesHeader();
	void handleFrameDemo();
	void handleFrame();
	void handleBlocky16(byte *src);
	void handleWave(const byte *src, uint32 size);
	void handleIACT(const byte *src, int32 size);
	bool setupAnim(const char *file, bool looping, int x, int y);
	bool setupAnimDemo(const char *file);
};

} // end of namespace Grim

#endif
