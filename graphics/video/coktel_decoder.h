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
#include "common/rect.h"

#include "graphics/video/video_decoder.h"

#include "sound/mixer.h"

namespace Audio {
	class QueuingAudioStream;
}

namespace Graphics {

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

	// VideoDecoder interface

	void close();

	uint16 getWidth()  const;
	uint16 getHeight() const;

	uint32 getFrameCount() const;

	byte *getPalette();
	bool  hasDirtyPalette() const;

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

	bool hasSurface();
	void createSurface();
	void freeSurface();

	void deLZ77(byte *dest, byte *src);

	void renderBlockWhole   (const byte *src);
	void renderBlockWhole4X (const byte *src);
	void renderBlockWhole2Y (const byte *src);
	void renderBlockSparse  (const byte *src);
	void renderBlockSparse2Y(const byte *src);

	inline void unsignedToSigned(byte *buffer, int length);

public:
	// FixedRateVideoDecoder interface
	Common::Rational getFrameRate() const;

	uint32 getTimeToNextFrame() const;
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

	bool loadCoordinates();
	bool loadFrameTableOffsets(uint32 &framePosPos, uint32 &frameCoordsPos);
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool loadFrameTables(uint32 framePosPos, uint32 frameCoordsPos);

	void processFrame();

	void calcFrameCoords(uint32 frame);

	void videoData(uint32 size);
	void renderFrame();

	void nextSoundSlice(bool hasNextCmd);
	bool initialSoundSlice(bool hasNextCmd);
	void emptySoundSlice(bool hasNextCmd);
};

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELDECODER_H

#endif // Engine and dynamic plugins guard
