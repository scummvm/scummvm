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

// Currently, only GOB and SCI32 games play IMDs and VMDs, so skip compiling if GOB and SCI32 is disabled.
#if !(defined(ENABLE_GOB) || defined(ENABLE_SCI32) || defined(DYNAMIC_MODULES))

// Do not compile the CoktelDecoder code

#else

#ifndef GRAPHICS_VIDEO_COKTELDECODER_H
#define GRAPHICS_VIDEO_COKTELDECODER_H

#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

#include "graphics/video/video_decoder.h"

#include "sound/mixer.h"

namespace Audio {
	class QueuingAudioStream;
}

namespace Graphics {

class Codec;

class CoktelDecoder : public FixedRateVideoDecoder {
public:
	struct State {
		/** Set accordingly to what was done. */
		uint32 flags;
		/** The id of the spoken words. */
		uint16 speechId;

		State();
	};

	CoktelDecoder(Audio::Mixer &mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~CoktelDecoder();

	virtual bool seek(int32 frame, int whence = SEEK_SET, bool restart = false) = 0;

	/** Draw directly onto the specified video memory. */
	void setSurfaceMemory(void *mem, uint16 width, uint16 height, uint8 bpp);
	/** Reset the video memory. */
	void setSurfaceMemory();

	const Surface *getSurface() const;

	/** Draw the video starting at this position within the video memory. */
	virtual void setXY(uint16 x, uint16 y);
	/** Draw the video at the default position. */
	void setXY();

	/** Override the video's frame rate. */
	void setFrameRate(Common::Rational frameRate);

	/** Get the video's default X position. */
	uint16 getDefaultX() const;
	/** Get the video's default Y position. */
	uint16 getDefaultY() const;

	/** Return a list of rectangles that changed in the last frame. */
	const Common::List<Common::Rect> &getDirtyRects() const;

	bool hasPalette() const;

	bool hasSound()       const;
	bool isSoundEnabled() const;
	bool isSoundPlaying() const;

	void enableSound();
	void disableSound();

	/** Return whether that video has any embedded files. */
	virtual bool hasEmbeddedFiles() const;

	/** Return whether that embedded file exists. */
	virtual bool hasEmbeddedFile(const Common::String &fileName) const;

	/** Return that embedded file. */
	virtual Common::MemoryReadStream *getEmbeddedFile(const Common::String &fileName) const;


	// VideoDecoder interface

	void close();

	uint16 getWidth()  const;
	uint16 getHeight() const;

	uint32 getFrameCount() const;

	byte *getPalette();
	bool  hasDirtyPalette() const;


	// FixedRateVideoDecoder interface

	uint32 getTimeToNextFrame() const;

protected:
	enum SoundStage {
		kSoundNone    = 0, ///< No sound.
		kSoundLoaded  = 1, ///< Sound loaded.
		kSoundPlaying = 2  ///< Sound is playing.
	};

	enum Features {
		kFeaturesNone        = 0x0000,
		kFeaturesPalette     = 0x0008, ///< Has an own palette.
		kFeaturesDataSize    = 0x0020, ///< Suggests a data size.
		kFeaturesSound       = 0x0040, ///< Has sound.
		kFeaturesFrameCoords = 0x0080, ///< Has specific frame coordinates.
		kFeaturesStdCoords   = 0x0100, ///< Has general standard coordinates.
		kFeaturesFramePos    = 0x0200, ///< Has a frame positions table.
		kFeaturesVideo       = 0x0400  ///< Has video.
	};

	Audio::Mixer *_mixer;
	Audio::Mixer::SoundType _soundType;

	uint16 _width;
	uint16 _height;

	uint16 _x;
	uint16 _y;

	uint16 _defaultX;
	uint16 _defaultY;

	uint32 _features;

	uint32 _frameCount;

	byte _palette[768];
	bool _paletteDirty;

	bool    _ownSurface;
	Surface _surface;

	Common::List<Common::Rect> _dirtyRects;

	Common::Rational _frameRate;

	// Current sound state
	bool       _hasSound;
	bool       _soundEnabled;
	SoundStage _soundStage;

	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	bool evaluateSeekFrame(int32 &frame, int whence) const;

	// Surface management
	bool hasSurface();
	void createSurface();
	void freeSurface();

	// Decompression
	void deLZ77(byte *dest, byte *src);

	// Block rendering
	void renderBlockWhole   (const byte *src);
	void renderBlockWhole4X (const byte *src);
	void renderBlockWhole2Y (const byte *src);
	void renderBlockSparse  (const byte *src);
	void renderBlockSparse2Y(const byte *src);

	// Sound helper functions
	inline void unsignedToSigned(byte *buffer, int length);


	// FixedRateVideoDecoder interface

	Common::Rational getFrameRate() const;
};

class PreIMDDecoder : public CoktelDecoder {
public:
	PreIMDDecoder(uint16 width, uint16 height, Audio::Mixer &mixer,
			Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~PreIMDDecoder();

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);


	// VideoDecoder interface

	bool load(Common::SeekableReadStream &stream);
	void close();

	bool isVideoLoaded() const;

	Surface *decodeNextFrame();

	PixelFormat getPixelFormat() const;

private:
	Common::SeekableReadStream *_stream;

	// Buffer for processed frame data
	byte  *_videoBuffer;
	uint32 _videoBufferSize;

	// Frame decoding
	void processFrame();
	void renderFrame();
};

class IMDDecoder : public CoktelDecoder {
public:
	IMDDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~IMDDecoder();

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);

	void setXY(uint16 x, uint16 y);


	// VideoDecoder interface

	bool load(Common::SeekableReadStream &stream);
	void close();

	bool isVideoLoaded() const;

	Surface *decodeNextFrame();

	PixelFormat getPixelFormat() const;

private:
	enum Command {
		kCommandNextSound   = 0xFF00,
		kCommandStartSound  = 0xFF01,

		kCommandBreak       = 0xFFF0,
		kCommandBreakSkip0  = 0xFFF1,
		kCommandBreakSkip16 = 0xFFF2,
		kCommandBreakSkip32 = 0xFFF3,
		kCommandBreakMask   = 0xFFF8,

		kCommandPalette     = 0xFFF4,
		kCommandVideoData   = 0xFFFC,

		kCommandJump        = 0xFFFD
	};

	struct Coord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	};

	Common::SeekableReadStream *_stream;

	byte _version;

	// Standard coordinates gives by the header
	int16 _stdX;
	int16 _stdY;
	int16 _stdWidth;
	int16 _stdHeight;

	uint32 _flags;

	uint32  _firstFramePos; ///< Position of the first frame's data within the stream.
	uint32 *_framePos;      ///< Positions of all frames.
	Coord  *_frameCoords;   ///< Coordinates of all frames.

	// Buffer for raw frame data
	byte  *_frameData;
	uint32 _frameDataSize;
	uint32 _frameDataLen;

	// Buffer for processed frame data
	byte  *_videoBuffer;
	uint32 _videoBufferSize;

	// Sound properties
	uint16 _soundFlags;
	 int16 _soundFreq;
	 int16 _soundSliceSize;
	 int16 _soundSlicesCount;

	// Loading helper functions
	bool loadCoordinates();
	bool loadFrameTableOffsets(uint32 &framePosPos, uint32 &frameCoordsPos);
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool loadFrameTables(uint32 framePosPos, uint32 frameCoordsPos);

	// Frame decoding
	void processFrame();
	void calcFrameCoords(uint32 frame);

	// Video
	void videoData(uint32 size);
	void renderFrame();

	// Sound
	void nextSoundSlice(bool hasNextCmd);
	bool initialSoundSlice(bool hasNextCmd);
	void emptySoundSlice(bool hasNextCmd);
};

class VMDDecoder : public CoktelDecoder {
public:
	VMDDecoder(Audio::Mixer &mixer, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	~VMDDecoder();

	bool seek(int32 frame, int whence = SEEK_SET, bool restart = false);

	bool hasEmbeddedFiles() const;
	bool hasEmbeddedFile(const Common::String &fileName) const;
	Common::MemoryReadStream *getEmbeddedFile(const Common::String &fileName) const;


	// VideoDecoder interface

	bool load(Common::SeekableReadStream &stream);
	void close();

	bool isVideoLoaded() const;

	Surface *decodeNextFrame();

	PixelFormat getPixelFormat() const;

private:
	enum PartType {
		kPartTypeSeparator = 0,
		kPartTypeAudio     = 1,
		kPartTypeVideo     = 2,
		kPartTypeFile      = 3,
		kPartType4         = 4,
		kPartTypeSpeech    = 5
	};

	enum AudioFormat {
		kAudioFormat8bitRaw    = 0,
		kAudioFormat16bitDPCM  = 1,
		kAudioFormat16bitADPCM = 2
	};

	struct File {
		Common::String name;

		uint32 offset;
		uint32 size;
		uint32 realSize;

		File();
	};

	struct Part {
		PartType type;
		byte     field_1;
		byte     field_E;
		uint32   size;
		int16    left;
		int16    top;
		int16    right;
		int16    bottom;
		uint16   id;
		byte     flags;

		Part();
	};

	struct Frame {
		uint32 offset;
		Part  *parts;

		Frame();
		~Frame();
	};

	// Tables for the audio decompressors
	static const uint16 _tableDPCM[128];
	static const int32  _tableADPCM[];
	static const int32  _tableADPCMStep[];

	Common::SeekableReadStream *_stream;

	byte   _version;
	uint32 _flags;

	uint32 _frameInfoOffset;
	uint16 _partsPerFrame;
	Frame *_frames;

	Common::Array<File> _files;

	// Sound properties
	uint16 _soundFlags;
	int16  _soundFreq;
	int16  _soundSliceSize;
	int16  _soundSlicesCount;
	byte   _soundBytesPerSample;
	byte   _soundStereo; // (0: mono, 1: old-style stereo, 2: new-style stereo)
	uint32 _soundHeaderSize;
	uint32 _soundDataSize;
	AudioFormat _audioFormat;

	// Video properties
	bool   _hasVideo;
	uint32 _videoCodec;
	byte   _blitMode;
	byte   _bytesPerPixel;

	uint32  _firstFramePos; ///< Position of the first frame's data within the stream.

	// Buffer for raw frame data
	byte  *_frameData;
	uint32 _frameDataSize;
	uint32 _frameDataLen;

	// Buffer for processed frame data
	byte  *_videoBuffer;
	uint32 _videoBufferSize;

	bool _externalCodec;
	Codec *_codec;

	// Loading helper functions
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool readFrameTable(int &numFiles);
	bool readFiles();

	// Frame decoding
	void processFrame();
	void renderFrame();
};

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELDECODER_H

#endif // Engine and dynamic plugins guard
