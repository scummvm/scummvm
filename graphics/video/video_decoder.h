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

#ifndef GRAPHICS_VIDEO_PLAYER_H
#define GRAPHICS_VIDEO_PLAYER_H

#include "common/events.h"
#include "common/list.h"
#include "common/rational.h"
#include "common/stream.h"

#include "graphics/surface.h"
#include "graphics/pixelformat.h"

namespace Common {
	class SeekableReadStream;
}

namespace Graphics {

/**
 * Implementation of a generic video decoder
 */
class VideoDecoder {
public:
	VideoDecoder();
	virtual ~VideoDecoder() {}

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	virtual uint16 getWidth() const = 0;

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	virtual uint16 getHeight() const = 0;

	/**
	 * Returns the current frame number of the video
	 * @return the last frame decoded by the video
	 */
	virtual int32 getCurFrame() const { return _curFrame; }

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	virtual uint32 getFrameCount() const = 0;

	/**
	 * Returns the time (in ms) that the video has been running
	 */
	virtual uint32 getElapsedTime() const;

	/**
	 * Returns whether a frame should be decoded or not
	 * @return whether a frame should be decoded or not
	 */
	virtual bool needsUpdate() const;

	/**
	 * Load a video file
	 * @param filename	the filename to load
	 */
	virtual bool loadFile(const Common::String &filename);

	/**
	 * Load a video file
	 * @param stream  the stream to load
	 */
	virtual bool load(Common::SeekableReadStream &stream) = 0;

	/**
	 * Close a video file
	 */
	virtual void close() = 0;

	/**
	 * Returns if a video file is loaded or not
	 */
	virtual bool isVideoLoaded() const = 0;

	/**
	 * Decode the next frame and return the frame's surface
	 * @note the return surface should *not* be freed
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	virtual Surface *decodeNextFrame() = 0;

	/**
	 * Get the pixel format of the video
	 */
	virtual PixelFormat getPixelFormat() const = 0;

	/**
	 * Get the palette for the video in RGB format (if 8bpp or less)
	 */
	virtual byte *getPalette() { return 0; }

	/**
	 * Returns if the palette is dirty or not
	 */
	virtual bool hasDirtyPalette() const { return false; }

	/**
	 * Add the time the video has been paused to maintain sync
	 */
	virtual void addPauseTime(uint32 ms) { _startTime += ms; }

	/**
	 * Returns if the video is finished or not
	 */
	virtual bool endOfVideo() const;

	/**
	 * Set the current palette to the system palette
	 */
	void setSystemPalette();

	/**
	 * Return the time until the next frame (in ms)
	 */
	virtual uint32 getTimeToNextFrame() const = 0;

protected:
	/**
	 * Resets _curFrame and _startTime. Should be called from every close() function.
	 */
	void reset();

	int32 _curFrame;
	uint32 _startTime;
};

/**
 * A VideoDecoder wrapper that implements getTimeToNextFrame() based on getFrameRate().
 */
class FixedRateVideoDecoder : public virtual VideoDecoder {
public:
	uint32 getTimeToNextFrame() const;

protected:
	/**
	 * Return the frame rate in frames per second
	 * This returns a Rational because videos can have rates that are not integers and
	 * there are some videos with frame rates < 1.
	 */
	virtual Common::Rational getFrameRate() const = 0;

private:
	uint32 getFrameBeginTime(uint32 frame) const;
};

/**
 * A VideoDecoder that can rewind back to the beginning.
 */
class RewindableVideoDecoder : public virtual VideoDecoder {
public:
	/**
	 * Rewind to the beginning of the video.
	 */
	virtual void rewind() = 0;
};

} // End of namespace Graphics

#endif
