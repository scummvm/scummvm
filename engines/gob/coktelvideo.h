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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef GOB_COKTELVIDEO_H
#define GOB_COKTELVIDEO_H

#include "common/stream.h"
#include "sound/mixer.h"
#include "sound/audiostream.h"

namespace Gob {

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
		kFeaturesFramesPos = 0x200
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
		kStateNoData = 0x800,
		/** Updated according to the general standard coordinates. */
		kStateStdCoords = 0x1000,
		/** Had to explicitely seek to the frame. */
		kStateSeeked = 0x2000,
		/** Reached a break-point. */
		kStateBreak = 0x8000
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
	};

	virtual ~CoktelVideo() { }

	/** Returns the features the loaded video possesses. */
	virtual uint16 getFeatures() const = 0;
	/** Returns the x coordinate of the video. */
	virtual int16 getX() const = 0;
	/** Returns the y coordinate of the video. */
	virtual int16 getY() const = 0;
	/** Returns the width of the video. */
	virtual int16 getWidth() const = 0;
	/** Returns the height of the video. */
	virtual int16 getHeight() const = 0;
	/** Returns the number of frames the loaded video has. */
	virtual int16 getFramesCount() const = 0;
	/** Returns the current frame number. */
	virtual int16 getCurrentFrame() const = 0;
	/** Returns the frame rate. */
	virtual int16 getFrameRate() const = 0;
	/** Returns the number of frames the video lags behind the audio. */
	virtual uint32 getSyncLag() const = 0;
	/** Returns the current frame's palette. */
	virtual const byte *getPalette() const = 0;

	/** Load a video out of a stream. */
	virtual bool load(Common::SeekableReadStream &stream) = 0;
	/** Unload the currently loaded video. */
	virtual void unload() = 0;

	/** Set the coordinations where to draw the video. */
	virtual void setXY(int16 x, int16 y) = 0;
	/** Use a specific memory block as video memory. */
	virtual void setVideoMemory(byte *vidMem, uint16 width, uint16 height) = 0;
	/** Use an own memory block as video memory. */
	virtual void setVideoMemory() = 0;

	/** Play sound (if the IMD has sound). */
	virtual void enableSound(Audio::Mixer &mixer) = 0;
	/** Don't play sound or stop currently playing sound. */
	virtual void disableSound() = 0;

	/** Seek to a specific frame.
	 *
	 *  @param frame The frame to which to seek.
	 *  @param whence The offset from whence the frame is given.
	 *  @param restart Restart the video to reach an otherwise inaccessible frame?
	 */
	virtual void seekFrame(int16 frame, int16 whence = SEEK_SET, bool restart = false) = 0;

	/** Render the next frame. */
	virtual State nextFrame() = 0;
	/** Look at what a frame would do/have, without actually rendering the frame. */
	virtual State peekFrame(int16 frame) = 0;
	/** Wait for the frame to end. */
	virtual void waitEndFrame() = 0;

	/** Copy the current frame.
	 *
	 *  @param dest The memory to which to copy the current frame
	 *  @param x The x position to where to copy.
	 *  @param y The y position to where to copy.
	 *  @param pitch The buffer's width.
	 *  @param transp Which color should be seen as transparent?
	 */
	virtual void copyCurrentFrame(byte *dest, uint16 x, uint16 y, uint16 width, int16 transp = -1) = 0;
};

/** Coktel Vision's IMD files.
 */
class Imd : public CoktelVideo {
public:
	Imd();
	~Imd();

	uint16 getFeatures() const { return _features; }
	int16 getX() const { return _x; }
	int16 getY() const { return _y; }
	int16 getWidth() const { return _width; }
	int16 getHeight() const { return _height; }
	int16 getFramesCount() const { return _framesCount; }
	int16 getCurrentFrame() const { return _curFrame; }
	int16 getFrameRate() const { if (_hasSound) return 1000 / _soundSliceLength; return 12; }
	uint32 getSyncLag() const { return _skipFrames; }
	const byte *getPalette() const { return _palette; }

	bool load(Common::SeekableReadStream &stream);
	void unload();

	void setXY(int16 x, int16 y);
	void setVideoMemory(byte *vidMem, uint16 width, uint16 height);
	void setVideoMemory();

	void enableSound(Audio::Mixer &mixer);
	void disableSound();

	void seekFrame(int16 frame, int16 whence = SEEK_SET, bool restart = false);

	State nextFrame();
	State peekFrame(int16 frame);
	void waitEndFrame();

	void copyCurrentFrame(byte *dest, uint16 x, uint16 y, uint16 width, int16 transp = -1);

protected:
	struct Coord {
		int16 left;
		int16 top;
		int16 right;
		int16 bottom;
	} PACKED_STRUCT;

	Common::SeekableReadStream *_stream;
	uint8 _version;
	uint16 _features;
	int16 _flags;
	int16 _x, _y, _width, _height;
	int16 _stdX, _stdY, _stdWidth, _stdHeight;
	int16 _framesCount, _curFrame;
	int32 *_framesPos;
	int32 _firstFramePos;
	Coord *_frameCoords;

	int32 _frameDataSize, _vidBufferSize;
	byte *_frameData, *_vidBuffer;

	byte _palette[768];

	bool _hasOwnVidMem;
	byte *_vidMem;
	uint16 _vidMemWidth, _vidMemHeight;

	bool _hasSound;
	bool _soundEnabled;
	uint8 _soundStage; // (0: no sound, 1: loaded, 2: playing)
	uint32 _soundStartTime;
	uint32 _skipFrames;

	int16 _soundFreq;
	uint16 _soundSliceSize;
	int16 _soundSlicesCount;
	uint16 _soundSliceLength;

	Audio::AppendableAudioStream *_audioStream;
	Audio::SoundHandle _audioHandle;
	
	uint32 _frameLength;
	uint32 _lastFrameTime;

	Audio::Mixer *_mixer;

	void unsignedToSigned(byte *buffer, int length) {
		while (length-- > 0) *buffer++ ^= 0x80;
	}

	void deleteVidMem(bool del = true);
	void clear(bool del = true);

	State processFrame(int16 frame);
	uint32 renderFrame();
	void frameUncompressor(byte *dest, byte *src);
};

} // End of namespace Gob

#endif // GOB_COKTELVIDEO_H
