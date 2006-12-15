/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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


#if !defined(CUP_PLAYER_HE_H) && !defined(DISABLE_HE)
#define CUP_PLAYER_HE_H

namespace Scumm {

struct CUP_Sfx {
	int num;
	int mode;
	uint16 flags;
};

struct CUP_Player {
	Common::File _fd;
	uint32 _dataSize;
	int _playbackRate;
	int _width, _height;
	uint8 _paletteData[256 * 4];
	uint8 *_offscreenBuffer;
	uint8 _tempLzssBuffer[0x1000];
	uint8 *_currentChunkData;
	uint32 _currentChunkSize;
	uint8 *_bufferLzssData;
	uint32 _bufferLzssSize;
	bool _paletteChanged;
	int _sfxCount;
	uint8 *_sfxBuffer;
	Audio::SoundHandle *_sfxHandleTable;
	CUP_Sfx _sfxQueue[16];
	int _sfxQueuePos;

	ScummEngine_vCUPhe *_vm;
	Audio::Mixer *_mixer;
	OSystem *_system;

	CUP_Player(OSystem *sys, ScummEngine_vCUPhe *vm, Audio::Mixer *mixer) {
		_currentChunkData = 0;
		_currentChunkSize = 0;
		_bufferLzssData = 0;
		_bufferLzssSize = 0;
		_vm = vm;
		_mixer = mixer;
		_system = sys;
	}

	bool open(const char *filename);
	void close();
	uint32 loadNextChunk();
	void parseHeaderTags();
	void play();
	void setDirtyScreenRect(const Common::Rect &r);
	void updateScreen();
	void updateSfx();
	void parseNextTag(const uint8 *data, uint32 &tag, uint32 &size);
	void handleHEAD(const uint8 *data, uint32 dataSize);
	void handleSFXB(const uint8 *data, uint32 dataSize);
	void handleRGBS(const uint8 *data, uint32 dataSize);
	void handleFRAM(uint8 *dst, const uint8 *data, uint32 size);
	void decodeFRAM(uint8 *dst, Common::Rect &dstRect, const uint8 *data, int type);
	void handleSRLE(uint8 *dst, const uint8 *data, uint32 size);
	void decodeSRLE(uint8 *dst, const uint8 *colorMap, const uint8 *data, int unpackedSize);
	uint8 *handleLZSS(const uint8 *data, uint32 dataSize);
	void decodeLzssData(uint8 *dst1, const uint8 *src1, const uint8 *src2, const uint8 *src3, uint8 *dst2);
	void handleRATE(const uint8 *data, uint32 dataSize);
	void handleSNDE(const uint8 *data, uint32 dataSize);
	void handleTOIL(const uint8 *data, uint32 dataSize);
};

} // End of namespace Scumm

#endif
