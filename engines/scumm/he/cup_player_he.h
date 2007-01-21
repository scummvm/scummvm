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
	int16 num;
	uint32 flags;
};

struct CUP_SfxChannel {
	Audio::SoundHandle handle;
	int16 sfxNum;
	uint32 flags;
};

class CUP_Player {
public:

	enum {
		kSfxFlagLoop    = 1 << 0,
		kSfxFlagRestart = 1 << 1
	};

	enum {
		kSfxChannels = 8,
		kSfxQueueSize = 16,
		kDefaultPlaybackRate = 66,
		kDefaultVideoWidth = 640,
		kDefaultVideoHeight = 480
	};

	CUP_Player(OSystem *sys, ScummEngine_vCUPhe *vm, Audio::Mixer *mixer);

	bool open(const char *filename);
	void close();
	uint32 loadNextChunk();
	void parseHeaderTags();
	void play();
	void setDirtyScreenRect(const Common::Rect &r);
	void updateScreen();
	void updateSfx();
	void waitForSfxChannel(int channel);
	void parseNextTag(const uint8 *data, uint32 &tag, uint32 &size);
	void handleHEAD(const uint8 *data, uint32 dataSize);
	void handleSFXB(const uint8 *data, uint32 dataSize);
	void handleRGBS(const uint8 *data, uint32 dataSize);
	void handleFRAM(uint8 *dst, const uint8 *data, uint32 size);
	void decodeFRAM(uint8 *dst, Common::Rect &dstRect, const uint8 *data, int type);
	void handleSRLE(uint8 *dst, const uint8 *data, uint32 size);
	void decodeSRLE(uint8 *dst, const uint8 *colorMap, const uint8 *data, int unpackedSize);
	uint8 *handleLZSS(const uint8 *data, uint32 dataSize);
	void decodeLZSS(uint8 *dst, const uint8 *src1, const uint8 *src2, const uint8 *src3);
	void handleRATE(const uint8 *data, uint32 dataSize);
	void handleSNDE(const uint8 *data, uint32 dataSize);
	void handleTOIL(const uint8 *data, uint32 dataSize);

protected:

	ScummEngine_vCUPhe *_vm;
	Audio::Mixer *_mixer;
	OSystem *_system;

	Common::File _fd;
	uint32 _dataSize;

	int _playbackRate;
	int _width, _height;

	uint8 _paletteData[256 * 4];
	bool _paletteChanged;
	uint8 *_offscreenBuffer;

	uint8 *_currentChunkData;
	uint32 _currentChunkSize;
	uint8 *_bufferLzssData;
	uint32 _bufferLzssSize;

	int _sfxCount;
	uint8 *_sfxBuffer;
	CUP_SfxChannel _sfxChannels[kSfxChannels];
	CUP_Sfx _sfxQueue[kSfxQueueSize];
	int _sfxQueuePos;
	int _lastSfxChannel;
};

} // End of namespace Scumm

#endif
