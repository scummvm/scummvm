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

/**
 * Video decoder used in engines:
 *  - agos
 *  - saga
 *  - scumm (he)
 *  - sword1
 *  - sword2
 */

// Based on http://wiki.multimedia.cx/index.php?title=Smacker
// and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
// http://svn.ffmpeg.org/ffmpeg/trunk/libavcodec/smacker.c?revision=16143&view=markup

#ifndef GRAPHICS_VIDEO_SMK_PLAYER_H
#define GRAPHICS_VIDEO_SMK_PLAYER_H

#include "graphics/video/video_player.h"
#include "sound/mixer.h"

namespace Audio {
	class AppendableAudioStream;
}

namespace Graphics {

class BigHuffmanTree;

/**
 * Implementation of a Smacker v2/v4 video decoder
 */
class SmackerDecoder : public VideoDecoder {
public:
	SmackerDecoder(Audio::Mixer *mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kSFXSoundType);
	virtual ~SmackerDecoder();

	int getHeight();
	int32 getAudioLag();

	/**
	 * Load an SMK encoded video file
	 * @param filename	the filename to load
	 */
	bool loadFile(const char *filename);

	/**
	 * Close an SMK encoded video file
	 */
	void closeFile();

	bool decodeNextFrame();

private:
	void unpackPalette();
	// Possible runs of blocks
	uint getBlockRun(int index) { return (index <= 58) ? index + 1 : 128 << (index - 59); }
	void queueCompressedBuffer(byte *buffer, uint32 bufferSize, uint32 unpackedSize, int streamNum);

	struct AudioInfo {
		bool isCompressed;
		bool hasAudio;
		bool is16Bits;
		bool isStereo;
		bool hasV2Compression;
		uint32 sampleRate;
	};

	struct {
		uint32 signature;
		uint32 flags;
		uint32 audioSize[7];
		uint32 treesSize;
		uint32 mMapSize;
		uint32 mClrSize;
		uint32 fullSize;
		uint32 typeSize;
		AudioInfo audioInfo[7];
		uint32 dummy;
	} _header;

	uint32 *_frameSizes;
	// The FrameTypes section of a Smacker file contains an array of bytes, where
	// the 8 bits of each byte describe the contents of the corresponding frame.
	// The highest 7 bits correspond to audio frames (bit 7 is track 6, bit 6 track 5
	// and so on), so there can be up to 7 different audio tracks. When the lowest bit
	// (bit 0) is set, it denotes a frame that contains a palette record
	byte *_frameTypes;
	byte *_frameData;
	// The RGB palette
	byte *_palette;

	Audio::Mixer::SoundType _soundType;
	Audio::Mixer *_mixer;
	bool _audioStarted;
	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	BigHuffmanTree *_MMapTree;
	BigHuffmanTree *_MClrTree;
	BigHuffmanTree *_FullTree;
	BigHuffmanTree *_TypeTree;
};

} // End of namespace Graphics

#endif
