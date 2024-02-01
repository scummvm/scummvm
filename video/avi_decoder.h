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

#ifndef VIDEO_AVI_DECODER_H
#define VIDEO_AVI_DECODER_H

#include "common/array.h"
#include "common/rational.h"
#include "common/rect.h"
#include "common/str.h"

#include "video/video_decoder.h"
#include "audio/mixer.h"

namespace Audio {
class AudioStream;
class PacketizedAudioStream;
}

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Palette;
struct PixelFormat;
}

namespace Image {
class Codec;
}

namespace Video {

/**
 * Decoder for AVI videos.
 *
 * Video decoder used in engines:
 *  - petka
 *  - sci
 *  - sword1
 *  - sword2
 *  - titanic
 *  - vcruise
 *  - zvision
 */
class AVIDecoder : public VideoDecoder {
public:
	AVIDecoder();
	AVIDecoder(const Common::Rational &frameRateOverride);
	virtual ~AVIDecoder();

	bool loadStream(Common::SeekableReadStream *stream);
	void close();
	uint16 getWidth() const { return _header.width; }
	uint16 getHeight() const { return _header.height; }

	bool rewind();
	bool isRewindable() const { return true; }
	bool isSeekable() const;

	/**
	 * Decode the next frame into a surface and return the latter.
	 *
	 * A subclass may override this, but must still call this function. As an
	 * example, a subclass may do this to apply some global video scale to
	 * individual track's frame.
	 *
	 * Note that this will call readNextPacket() internally first before calling
	 * the next video track's decodeNextFrame() function.
	 *
	 * @return a surface containing the decoded frame, or 0
	 * @note Ownership of the returned surface stays with the VideoDecoder,
	 *       hence the caller must *not* free it.
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	virtual const Graphics::Surface *decodeNextFrame();

	/**
	 * Decodes the next transparency track frame
	 */
	const Graphics::Surface *decodeNextTransparency();
protected:
	// VideoDecoder API
	void readNextPacket();
	bool seekIntern(const Audio::Timestamp &time);
	bool supportsAudioTrackSwitching() const { return true; }
	AudioTrack *getAudioTrack(int index);

	/**
	 * Define a track to be used by this class.
	 *
	 * The pointer is then owned by this base class.
	 *
	 * @param track The track to add
	 * @param isExternal Is this an external track not found by loadStream()?
	 */
	void addTrack(Track *track, bool isExternal = false);

	struct BitmapInfoHeader {
		uint32 size;
		uint32 width;
		uint32 height;
		uint16 planes;
		uint16 bitCount;
		uint32 compression;
		uint32 sizeImage;
		uint32 xPelsPerMeter;
		uint32 yPelsPerMeter;
		uint32 clrUsed;
		uint32 clrImportant;
	};

	struct WaveFormat {
		uint16 tag;
		uint16 channels;
		uint32 samplesPerSec;
		uint32 avgBytesPerSec;
		uint16 blockAlign;
	};

	struct PCMWaveFormat : public WaveFormat {
		uint16 size;
	};

	struct WaveFormatEX : public WaveFormat {
		uint16 bitsPerSample;
		uint16 size;
	};

	struct OldIndex {
		uint32 id;
		uint32 flags;
		uint32 offset;
		uint32 size;
	};

	// Index Flags
	enum IndexFlags {
		AVIIF_INDEX = 0x10
	};

	struct AVIHeader {
		uint32 size;
		uint32 microSecondsPerFrame;
		uint32 maxBytesPerSecond;
		uint32 padding;
		uint32 flags;
		uint32 totalFrames;
		uint32 initialFrames;
		uint32 streams;
		uint32 bufferSize;
		uint32 width;
		uint32 height;
	};

	// Flags from the AVIHeader
	enum AVIFlags {
		AVIF_HASINDEX = 0x00000010,
		AVIF_MUSTUSEINDEX = 0x00000020,
		AVIF_ISINTERLEAVED = 0x00000100,
		AVIF_TRUSTCKTYPE = 0x00000800,
		AVIF_WASCAPTUREFILE = 0x00010000,
		AVIF_WASCOPYRIGHTED = 0x00020000
	};

	struct AVIStreamHeader {
		uint32 size;
		uint32 streamType;
		uint32 streamHandler;
		uint32 flags;
		uint16 priority;
		uint16 language;
		uint32 initialFrames;
		uint32 scale;
		uint32 rate;
		uint32 start;
		uint32 length;
		uint32 bufferSize;
		uint32 quality;
		uint32 sampleSize;
		Common::Rect frame;
		Common::String name;
	};

	class AVIVideoTrack : public FixedRateVideoTrack {
	public:
		AVIVideoTrack(int frameCount, const AVIStreamHeader &streamHeader, const BitmapInfoHeader &bitmapInfoHeader, Graphics::Palette *initialPalette = nullptr);
		~AVIVideoTrack();

		void decodeFrame(Common::SeekableReadStream *stream);
		void forceTrackEnd();

		uint16 getWidth() const { return _bmInfo.width; }
		uint16 getHeight() const { return _bmInfo.height; }
		uint16 getBitCount() const { return _bmInfo.bitCount; }
		Graphics::PixelFormat getPixelFormat() const;
		bool setOutputPixelFormat(const Graphics::PixelFormat &format);
		int getCurFrame() const { return _curFrame; }
		int getFrameCount() const { return _frameCount; }
		Common::String &getName() { return _vidsHeader.name; }
		const Graphics::Surface *decodeNextFrame() { return _lastFrame; }

		const byte *getPalette() const;
		bool hasDirtyPalette() const;
		void setCurFrame(int frame) { _curFrame = frame; }
		void loadPaletteFromChunk(Common::SeekableReadStream *chunk);
		void loadPaletteFromChunkRaw(Common::SeekableReadStream *chunk, int firstEntry, int numEntries);
		void useInitialPalette();
		bool canDither() const;
		void setDither(const byte *palette);
		bool isValid() const { return _videoCodec != nullptr; }

		bool isTruemotion1() const;
		void forceDimensions(uint16 width, uint16 height);

		bool isRewindable() const { return true; }
		bool rewind();

		/**
		 * Set the video track to play in reverse or forward.
		 *
		 * By default, a VideoTrack must decode forward.
		 *
		 * @param reverse true for reverse, false for forward
		 * @return true for success, false for failure
		 */
		virtual bool setReverse(bool reverse);

		/**
		 * Is the video track set to play in reverse?
		 */
		virtual bool isReversed() const { return _reversed; }

		/**
		 * Returns true if at the end of the video track
		 */
		virtual bool endOfTrack() const;

		/**
		 * Get track frame rate
		 */
		Common::Rational getFrameRate() const { return Common::Rational(_vidsHeader.rate, _vidsHeader.scale); }

		/**
		 * Force sets a new frame rate
		 */
		void setFrameRate(const Common::Rational &r) {
			_vidsHeader.rate = r.getNumerator();
			_vidsHeader.scale = r.getDenominator();
		}
	private:
		AVIStreamHeader _vidsHeader;
		BitmapInfoHeader _bmInfo;
		Graphics::Palette *_palette;
		Graphics::Palette *_initialPalette;
		mutable bool _dirtyPalette;
		int _frameCount, _curFrame;
		bool _reversed;

		Image::Codec *_videoCodec;
		const Graphics::Surface *_lastFrame;
		Image::Codec *createCodec();
	};

	class AVIAudioTrack : public AudioTrack {
	public:
		AVIAudioTrack(const AVIStreamHeader &streamHeader, const PCMWaveFormat &waveFormat, Audio::Mixer::SoundType soundType);
		~AVIAudioTrack();

		virtual void createAudioStream();
		virtual void queueSound(Common::SeekableReadStream *stream);
		void skipAudio(const Audio::Timestamp &time, const Audio::Timestamp &frameTime);
		virtual void resetStream();
		uint32 getCurChunk() const { return _curChunk; }
		Common::String &getName() { return _audsHeader.name; }
		void setCurChunk(uint32 chunk) { _curChunk = chunk; }

		bool isRewindable() const { return true; }
		bool rewind();

	protected:
		Audio::AudioStream *getAudioStream() const { return _audioStream; }

		AVIStreamHeader _audsHeader;
		PCMWaveFormat _wvInfo;
		Audio::AudioStream *_audioStream;
		Audio::PacketizedAudioStream *_packetStream;
		uint32 _curChunk;
	};

	struct TrackStatus {
		TrackStatus();

		Track *track;
		uint32 index;
		uint32 chunkSearchOffset;
	};

	class IndexEntries : public Common::Array<OldIndex> {
	public:
		OldIndex *find(uint index, uint frameNumber);
	};

	AVIHeader _header;

	void readOldIndex(uint32 size);
	IndexEntries _indexEntries;

	Common::SeekableReadStream *_fileStream;
	bool _decodedHeader;
	bool _foundMovieList;
	uint32 _movieListStart, _movieListEnd;

	Common::Rational _frameRateOverride;

	int _videoTrackCounter, _audioTrackCounter;
	Track *_lastAddedTrack;

	void initCommon();

	bool parseNextChunk();
	void skipChunk(uint32 size);
	void handleList(uint32 listSize);
	void handleStreamHeader(uint32 size);
	void readStreamName(uint32 size);
	void readPalette8(uint32 size);
	uint16 getStreamType(uint32 tag) const { return tag & 0xFFFF; }
	static byte getStreamIndex(uint32 tag);
	void checkTruemotion1();
	uint getVideoTrackOffset(uint trackIndex, uint frameNumber = 0);

	void handleNextPacket(TrackStatus& status);
	bool shouldQueueAudio(TrackStatus& status);
	void seekTransparencyFrame(int frame);

	Common::Array<TrackStatus> _videoTracks, _audioTracks;
	TrackStatus _transparencyTrack;
public:
	virtual AVIAudioTrack *createAudioTrack(AVIStreamHeader sHeader, PCMWaveFormat wvInfo);

	/**
	 * Seek to a given frame.
	 *
	 * This only works when the video track(s) supports getFrameTime().
	 * This calls seek() internally.
	 */
	virtual bool seekToFrame(uint frame);
};

} // End of namespace Video

#endif
