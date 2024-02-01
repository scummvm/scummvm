/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef VIDEO_SMK_PLAYER_H
#define VIDEO_SMK_PLAYER_H

#include "common/bitarray.h"
#include "common/bitstream.h"
#include "common/rational.h"
#include "common/rect.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "video/video_decoder.h"
#include "audio/mixer.h"

namespace Audio {
class QueuingAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Video {

class BigHuffmanTree;

// Because the maximum number of bits read from a bitstream is 16, and the data is 8-bit, the container only
// needs to hold up to 23 bits at any given time. As such, we use a bitstream with a 32-bit container to
// avoid the overhead of 64-bit maths on systems that don't support it natively.
typedef Common::BitStreamImpl<Common::BitStreamMemoryStream, uint32, 8, false, false> SmackerBitStream;

/**
 * Decoder for Smacker v2/v4 videos.
 *
 * Based on http://wiki.multimedia.cx/index.php?title=Smacker
 * and the FFmpeg Smacker decoder (libavcodec/smacker.c), revision 16143
 * https://git.ffmpeg.org/gitweb/ffmpeg.git/commit/40a19c443430de520d86bbd644033c8e2ca87e9b
 *
 * Video decoder used in engines:
 *  - agos
 *  - saga
 *  - scumm (he)
 *  - sword1
 *  - sword2
 *  - toon
 *  - trecision
 *  - twine
 */
class SmackerDecoder : public VideoDecoder {
public:
	SmackerDecoder();
	virtual ~SmackerDecoder();

	virtual bool loadStream(Common::SeekableReadStream *stream);
	void close();
	void forceSeekToFrame(uint frame);
	bool rewind();

	Common::Rational getFrameRate() const;

	virtual const Common::Rect *getNextDirtyRect();

protected:
	void readNextPacket();
	bool supportsAudioTrackSwitching() const { return true; }
	AudioTrack *getAudioTrack(int index);

	virtual void handleAudioTrack(byte track, uint32 chunkSize, uint32 unpackedSize);

	virtual uint32 getSignatureVersion(uint32 signature) const;

	class SmackerVideoTrack : public FixedRateVideoTrack {
	public:
		SmackerVideoTrack(uint32 width, uint32 height, uint32 frameCount, const Common::Rational &frameRate, uint32 flags, uint32 version);
		~SmackerVideoTrack();

		bool isRewindable() const { return true; }
		bool rewind() { _curFrame = -1; return true; }

		uint16 getWidth() const;
		uint16 getHeight() const;
		Graphics::PixelFormat getPixelFormat() const;
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		const Graphics::Surface *decodeNextFrame() { return _surface; }
		const byte *getPalette() const { _dirtyPalette = false; return _palette.data; }
		bool hasDirtyPalette() const { return _dirtyPalette; }

		void readTrees(SmackerBitStream &bs, uint32 mMapSize, uint32 mClrSize, uint32 fullSize, uint32 typeSize);
		void increaseCurFrame() { _curFrame++; }
		void decodeFrame(SmackerBitStream &bs);
		void unpackPalette(Common::SeekableReadStream *stream);

		Common::Rational getFrameRate() const { return _frameRate; }

		const Common::Rect *getNextDirtyRect();

	protected:
		Graphics::Surface *_surface;

	private:
		Common::Rational _frameRate;
		uint32 _flags, _version;

		Graphics::Palette _palette;
		mutable bool _dirtyPalette;

		int _curFrame;
		uint32 _frameCount;

		BigHuffmanTree *_MMapTree;
		BigHuffmanTree *_MClrTree;
		BigHuffmanTree *_FullTree;
		BigHuffmanTree *_TypeTree;

		Common::BitArray _dirtyBlocks;
		Common::Rect _lastDirtyRect;

		// Possible runs of blocks
		static uint getBlockRun(int index) { return (index <= 58) ? index + 1 : 128 << (index - 59); }
	};

	virtual SmackerVideoTrack *createVideoTrack(uint32 width, uint32 height, uint32 frameCount, const Common::Rational &frameRate, uint32 flags, uint32 version) const;

	Common::SeekableReadStream *_fileStream;

	enum AudioCompression {
		kCompressionNone,
		kCompressionDPCM,
		kCompressionRDFT,
		kCompressionDCT
	};

	struct AudioInfo {
		AudioCompression compression;
		bool hasAudio;
		bool is16Bits;
		bool isStereo;
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

private:

	class SmackerAudioTrack : public AudioTrack {
	public:
		SmackerAudioTrack(const AudioInfo &audioInfo, Audio::Mixer::SoundType soundType);
		~SmackerAudioTrack();

		bool isRewindable() const { return true; }
		bool rewind();

		void queueCompressedBuffer(byte *buffer, uint32 bufferSize, uint32 unpackedSize);
		void queuePCM(byte *buffer, uint32 bufferSize);

	protected:
		Audio::AudioStream *getAudioStream() const;

	private:
		Audio::QueuingAudioStream *_audioStream;
		AudioInfo _audioInfo;
	};

	class SmackerEmptyTrack : public Track {
		VideoDecoder::Track::TrackType getTrackType() const { return VideoDecoder::Track::kTrackTypeNone; }

		bool endOfTrack() const { return true; }

		bool isSeekable() const { return true; }
		bool seek(const Audio::Timestamp &time) { return true; }
	};

protected:
	// The FrameTypes section of a Smacker file contains an array of bytes, where
	// the 8 bits of each byte describe the contents of the corresponding frame.
	// The highest 7 bits correspond to audio frames (bit 7 is track 6, bit 6 track 5
	// and so on), so there can be up to 7 different audio tracks. When the lowest bit
	// (bit 0) is set, it denotes a frame that contains a palette record
	byte *_frameTypes;

private:
	uint32 _firstFrameStart;
};

} // End of namespace Video

#endif
