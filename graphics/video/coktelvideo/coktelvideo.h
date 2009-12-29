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

// Do not compile the CoktelVideo code

#else

#ifndef GRAPHICS_VIDEO_COKTELVIDEO_H
#define GRAPHICS_VIDEO_COKTELVIDEO_H

#include "common/scummsys.h"
#include "common/stream.h"
#include "common/array.h"
#include "graphics/dither.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Graphics {

#ifdef USE_INDEO3
class Indeo3;
#endif

/** Common interface for handling Coktel Vision videos and derivated formats. */
class CoktelVideo {
public:
	enum Features {
		kFeaturesNone = 0,
		/** Has an own palette. */
		kFeaturesPalette = 8,
		/** Suggests a data size. */
		kFeaturesDataSize = 0x20,
		/** Has sound. */
		kFeaturesSound = 0x40,
		/** Has specific frame coordinates. */
		kFeaturesFrameCoords = 0x80,
		/** Has general standard coordinates. */
		kFeaturesStdCoords = 0x100,
		/** Has a frame positions table. */
		kFeaturesFramesPos = 0x200,
		/** Has video. */
		kFeaturesVideo = 0x400,
		/** Is a full color (non-paletted) video. */
		kFeaturesFullColor = 0x4000,
		/** Supports automatic doubling. */
		kFeaturesSupportsDouble = 0x40000000
	};

	enum StateFlags {
		kStateNone = 0,
		/** Changed the palette. */
		kStatePalette = 0x10,
		/** Performed a jump to another frame. */
		kStateJump = 0x200,
		/** Updated according to the specific frame coordinates. */
		kStateFrameCoords = 0x400,
		/** Got no frame data. */
		kStateNoVideoData = 0x800,
		/** Updated according to the general standard coordinates. */
		kStateStdCoords = 0x1000,
		/** Had to explicitely seek to the frame. */
		kStateSeeked = 0x2000,
		/** Reached a break-point. */
		kStateBreak = 0x8000,
		/** Frame marks the beginning of speech. */
		kStateSpeech = 0x4000000
	};

	struct State {
		/** Left-most value of the updated rectangle. */
		int16 left;
		/** Top-most value of the updated rectangle. */
		int16 top;
		/** Right-most value of the updated rectangle. */
		int16 right;
		/** Bottom-most value of the updated rectangle. */
		int16 bottom;
		/** Set accordingly to what was done. */
		uint32 flags;
		/** The id of the spoken words. */
		uint16 speechId;

		State() : left(0), top(0), right(0), bottom(0), flags(0), speechId(0) { }
	};

	virtual ~CoktelVideo() { }

	/** Returns the features the loaded video possesses. */
	virtual uint32 getFeatures() const = 0;
	/** Returns the flags the loaded video possesses. */
	virtual uint16 getFlags() const = 0;
	/** Returns the x coordinate of the video. */
	virtual int16 getX() const = 0;
	/** Returns the y coordinate of the video. */
	virtual int16 getY() const = 0;
	/** Returns the width of the video. */
	virtual int16 getWidth() const = 0;
	/** Returns the height of the video. */
	virtual int16 getHeight() const = 0;
	/** Returns the number of frames the loaded video has. */
	virtual uint16 getFramesCount() const = 0;
	/** Returns the current frame number.
	 *
	 *  This is the current frame after the last nextFrame()-call,
	 *  i.e. it's 0 after loading, 1 after the first nextFrame()-call, etc..
	 */
	virtual uint16 getCurrentFrame() const = 0;
	/** Returns the frame rate. */
	virtual int16 getFrameRate() const = 0;
	/** Returns the number of frames the video lags behind the audio. */
	virtual uint32 getSyncLag() const = 0;
	/** Returns the current frame's palette. */
	virtual const byte *getPalette() const = 0;

	/** Returns the frame's coordinates */
	virtual bool getFrameCoords(int16 frame,
			int16 &x, int16 &y, int16 &width, int16 &height) = 0;

	/** Returns whether any extra data files are embedded in this video. */
	virtual bool hasExtraData() const = 0;
	/** Returns whether that extra data file exists */
	virtual bool hasExtraData(const char *fileName) const = 0;
	/** Returns an extra data file */
	virtual Common::MemoryReadStream *getExtraData(const char *fileName) = 0;

	/** Load a video out of a stream. */
	virtual bool load(Common::SeekableReadStream &stream) = 0;
	/** Unload the currently loaded video. */
	virtual void unload() = 0;

	/** Set the frame rate. */
	virtual void setFrameRate(int16 frameRate) = 0;

	/** Set the coordinations where to draw the video. */
	virtual void setXY(int16 x, int16 y) = 0;
	/** Use a specific memory block as video memory. */
	virtual void setVideoMemory(byte *vidMem, uint16 width, uint16 height) = 0;
	/** Use an own memory block as video memory. */
	virtual void setVideoMemory() = 0;

	/** Double the video's resolution. */
	virtual void setDoubleMode(bool doubleMode) = 0;

	/** Play sound (if the video has sound). */
	virtual void enableSound(Audio::Mixer &mixer) = 0;
	/** Don't play sound or stop currently playing sound. */
	virtual void disableSound() = 0;

	/** Is sound currently playing? */
	virtual bool isSoundPlaying() const = 0;

	/** Seek to a specific frame.
	 *
	 *  @param frame The frame to which to seek.
	 *  @param whence The offset from whence the frame is given.
	 *  @param restart Restart the video to reach an otherwise inaccessible frame?
	 */
	virtual void seekFrame(int32 frame, int16 whence = SEEK_SET, bool restart = false) = 0;

	/** Render the next frame. */
	virtual State nextFrame() = 0;
	/** Get the time in ms until the next frame can be displayed. Already includes A/V sync measures. */
	virtual uint32 getFrameWaitTime() = 0;
	/** Wait for the frame to end. */
	virtual void waitEndFrame() = 0;

	/** Copy the current frame.
	 *
	 *  @param dest The memory to which to copy the current frame.
	 *  @param left The x position within the frame.
	 *  @param top The y position within the frame.
	 *  @param width The width of the area to copy.
	 *  @param height The height of the area to copy.
	 *  @param x The x position to where to copy.
	 *  @param y The y position to where to copy.
	 *  @param pitch The buffer's width.
	 *  @param transp Which color should be seen as transparent?
	 */
	virtual void copyCurrentFrame(byte *dest,
			uint16 left, uint16 top, uint16 width, uint16 height,
			uint16 x, uint16 y, uint16 pitch, int16 transp = -1) = 0;
};

/** Coktel Vision's IMD files.
 */
class Imd : public CoktelVideo {
public:
	Imd();
	~Imd();

	uint32 getFeatures() const;
	uint16 getFlags()    const;

	int16  getX()      const;
	int16  getY()      const;
	int16  getWidth()  const;
	int16  getHeight() const;

	uint16 getFramesCount()  const;
	uint16 getCurrentFrame() const;
	int16  getFrameRate()    const;
	uint32 getSyncLag()      const;

	const byte *getPalette() const;

	bool getFrameCoords(int16 frame,
			int16 &x, int16 &y, int16 &width, int16 &height);

	bool hasExtraData() const;
	bool hasExtraData(const char *fileName) const;
	Common::MemoryReadStream *getExtraData(const char *fileName);

	void setFrameRate(int16 frameRate);

	bool load(Common::SeekableReadStream &stream);
	void unload();

	void setXY(int16 x, int16 y);
	void setVideoMemory(byte *vidMem, uint16 width, uint16 height);
	void setVideoMemory();

	void setDoubleMode(bool doubleMode);

	void enableSound(Audio::Mixer &mixer);
	void disableSound();

	bool isSoundPlaying() const;

	void seekFrame(int32 frame, int16 whence = SEEK_SET, bool restart = false);

	State nextFrame();
	uint32 getFrameWaitTime();
	void waitEndFrame();

	void copyCurrentFrame(byte *dest,
			uint16 left, uint16 top, uint16 width, uint16 height,
			uint16 x, uint16 y, uint16 pitch, int16 transp = -1);

protected:
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
	} PACKED_STRUCT;

	Common::SeekableReadStream *_stream;

	// Properties
	uint16 _version;
	uint32 _features;
	uint16 _flags;

	// Current coordinates
	int16 _x;
	int16 _y;
	int16 _width;
	int16 _height;

	// Standard coordinates gives by the header
	int16 _stdX;
	int16 _stdY;
	int16 _stdWidth;
	int16 _stdHeight;

	uint16 _framesCount;
	uint16 _curFrame;

	uint32 *_framesPos;
	uint32  _firstFramePos;
	Coord *_frameCoords;

	// Buffer for raw frame data
	byte  *_frameData;
	uint32 _frameDataSize;
	uint32 _frameDataLen;

	// Buffer for uncompressed raw frame data
	byte  *_vidBuffer;
	uint32 _vidBufferSize;

	byte _palette[768];

	// Video memory
	bool  _hasOwnVidMem;
	byte  *_vidMem;
	uint16 _vidMemWidth;
	uint16 _vidMemHeight;

	// Sound properties
	uint16 _soundFlags;
	int16  _soundFreq;
	int16  _soundSliceSize;
	int16  _soundSlicesCount;
	uint32 _soundSliceLength;

	// Current sound state
	bool   _hasSound;
	bool   _soundEnabled;
	uint8  _soundStage; // (0: no sound, 1: loaded, 2: playing)
	uint32 _skipFrames;

	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;

	// Current video state
	int16  _frameRate;
	uint32 _frameLength;
	uint32 _lastFrameTime;

	Audio::Mixer *_mixer;

	void unsignedToSigned(byte *buffer, int length);

	void deleteVidMem(bool del = true);
	void clear(bool del = true);

	bool loadCoordinates();
	bool loadFrameTableOffsets(uint32 &framesPosPos, uint32 &framesCoordsPos);
	bool assessVideoProperties();
	bool assessAudioProperties();
	bool loadFrameTables(uint32 framesPosPos, uint32 framesCoordsPos);

	State processFrame(uint16 frame);
	uint32 renderFrame(int16 left, int16 top, int16 right, int16 bottom);
	void deLZ77(byte *dest, byte *src);

	void renderBlockWhole   (byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);
	void renderBlockWhole4X (byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);
	void renderBlockWhole2Y (byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);
	void renderBlockSparse  (byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);
	void renderBlockSparse2Y(byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);

	void calcFrameCoords(uint16 frame, State &state);

	void nextSoundSlice(bool hasNextCmd);
	bool initialSoundSlice(bool hasNextCmd);
	void emptySoundSlice(bool hasNextCmd);

	void videoData(uint32 size, State &state);
};

class Vmd : public Imd {
public:
	Vmd(Graphics::PaletteLUT *palLUT = 0);
	~Vmd();

	bool getFrameCoords(int16 frame,
			int16 &x, int16 &y, int16 &width, int16 &height);

	bool hasExtraData() const;
	bool hasExtraData(const char *fileName) const;
	Common::MemoryReadStream *getExtraData(const char *fileName);

	bool load(Common::SeekableReadStream &stream);
	void unload();

	int16 getWidth() const;

	void setXY(int16 x, int16 y);

	void setDoubleMode(bool doubleMode);

	void seekFrame(int32 frame, int16 whence = SEEK_SET, bool restart = false);

	State nextFrame();

protected:
	enum PartType {
		kPartTypeSeparator = 0,
		kPartTypeAudio     = 1,
		kPartTypeVideo     = 2,
		kPartTypeExtraData = 3,
		kPartType4         = 4,
		kPartTypeSpeech    = 5
	};

	enum AudioFormat {
		kAudioFormat8bitDirect = 0,
		kAudioFormat16bitDPCM  = 1,
		kAudioFormat16bitADPCM = 2
	};

	struct ExtraData {
		char   name[16];
		uint32 offset;
		uint32 size;
		uint32 realSize;

		ExtraData();
	} PACKED_STRUCT;

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
	} PACKED_STRUCT;

	struct Frame {
		uint32 offset;
		Part  *parts;

		Frame();
		~Frame();
	} PACKED_STRUCT;

	// Tables for the audio decompressors
	static const uint16 _tableDPCM[128];
	static const int32  _tableADPCM[];
	static const int32  _tableADPCMStep[];

	bool   _hasVideo;
	uint32 _videoCodec;

	uint32 _frameInfoOffset;
	uint16 _partsPerFrame;
	Frame *_frames;

	Common::Array<ExtraData> _extraData;

	// Sound properties
	byte   _soundBytesPerSample;
	byte   _soundStereo; // (0: mono, 1: old-style stereo, 2: new-style stereo)
	uint32 _soundHeaderSize;
	uint32 _soundDataSize;
	AudioFormat _audioFormat;

	// Video properties
	bool _externalCodec;
	byte _blitMode;
	byte _bytesPerPixel;
	byte _preScaleX;
	byte _postScaleX;
	byte _scaleExternalX;
	byte *_vidMemBuffer;

	bool _doubleMode;

	Graphics::PaletteLUT *_palLUT;

#ifdef USE_INDEO3
	Indeo3 *_codecIndeo3;
#endif

	void clear(bool del = true);

	bool getPartCoords(int16 frame, PartType type,
			int16 &x, int16 &y, int16 &width, int16 &height);

	bool assessVideoProperties();
	bool assessAudioProperties();
	void readFrameTable(int &numExtraData);
	void readExtraData();

	State processFrame(uint16 frame);
	uint32 renderFrame(int16 &left, int16 &top, int16 &right, int16 &bottom);

	void renderBlockRLE(byte *dest, const byte *src, int16 width, int16 height,
			int16 destWidth, int16 destHeight);

	void deRLE(byte *&destPtr, const byte *&srcPtr, int16 len);

	inline int32 preScaleX(int32 x) const;
	inline int32 postScaleX(int32 x) const;

	void blit(byte *dest, byte *src, int16 width, int16 height);
	void blit16(byte *dest, byte *src, int16 srcPitch, int16 width, int16 height);
	void blit24(byte *dest, byte *src, int16 srcPitch, int16 width, int16 height);

	byte *deDPCM(const byte *data, uint32 &size, int32 init[2]);
	byte *deADPCM(const byte *data, uint32 &size, int32 init, int32 v28);

	byte *soundEmpty(uint32 &size);
	byte *sound8bitDirect(uint32 &size);
	byte *sound16bitDPCM(uint32 &size);
	byte *sound16bitADPCM(uint32 &size);

	uint8 evaluateMask(uint32 mask, bool *fillInfo, uint8 &max);
	void emptySoundSlice(uint32 size);
	void filledSoundSlice(uint32 size);
	void filledSoundSlices(uint32 size, uint32 mask);
};

} // End of namespace Graphics

#endif // GRAPHICS_VIDEO_COKTELVIDEO_H

#endif // Engine and dynamic plugins guard
