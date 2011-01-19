/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef PRISONER_MUXPLAYER_H
#define PRISONER_MUXPLAYER_H

#include "common/file.h"
#include "sound/audiostream.h"

class OSystem;

namespace Prisoner {

class MuxPlayer {
public:

	MuxPlayer(PrisonerEngine *vm);
	~MuxPlayer();

	bool open(const char *filename);
	void close();
	bool play();

protected:

	enum ChunkType {
		kEndOfFile		= 0x04,
		kEndOfChunk		= 0x05,
		kAudio1			= 0x06,
		kAudio2			= 0x07,
		kVideo			= 0x6F,
		kPalette		= 0x70
	};

	struct MuxHeader {
		uint32 id;
		uint16 startOfs;
		uint32 frameCount;
		uint16 u3, u4;
		uint16 soundFreq;
		uint32 u5;
		uint16 soundBufSize;
		uint16 u6, u7, u8;
		uint16 width, height;
	};

	PrisonerEngine *_vm;

	Audio::QueuingAudioStream *_stream;
	Audio::SoundHandle _sound;

	MuxHeader _header;
	Common::File _fd;
	bool _opened;

	byte *_frameBuffer;
	byte _palette[768];

	uint32 *_chunkTable;
	int _chunkTableCount;

	void handleFrame();
	void handleEndOfChunk(uint32 chunkSize);
	void handleAudio(uint32 chunkSize);
	void handleVideo(uint32 chunkSize);
	void handlePalette(uint32 chunkSize);

	void decompress(byte *source, byte *dest, uint32 sourceSize, uint32 destSize);
	void decodeFrame(byte *buf1, byte *buf2, byte *dest, uint32 bufSize1, uint32 bufSize2,
		bool isKeyframe);

};

} // End of namespace Prisoner

#endif /* PRISONER_MUXPLAYER_H */
