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
 */

#ifndef VIDEO_DECODER_H
#define VIDEO_DECODER_H

#include "common/str.h"

#include "audio/timestamp.h"	// TODO: Move this to common/ ?


namespace Common {
class Rational;
class SeekableReadStream;
}

namespace Graphics {
struct PixelFormat;
struct Surface;
}

namespace Video {

/**
 * Generic interface for video decoder classes.
 */
class VideoDecoder {
public:
	VideoDecoder();
	virtual ~VideoDecoder() {}

	/**
	 * Load a video from a file with the given name.
	 *
	 * A default implementation using loadStream is provided.
	 *
	 * @param filename	the filename to load
	 * @return whether loading the file succeeded
	 */
	virtual bool loadFile(const Common::String &filename);

	/**
	 * Load a video from a generic read stream. The ownership of the
	 * stream object transfers to this VideoDecoder instance, which is
	 * hence also responsible for eventually deleting it.
	 * @param stream  the stream to load
	 * @return whether loading the stream succeeded
	 */
	virtual bool loadStream(Common::SeekableReadStream *stream) = 0;

	/**
	 * Close the active video stream and free any associated resources.
	 */
	virtual void close() = 0;

	/**
	 * Returns if a video stream is currently loaded or not.
	 */
	virtual bool isVideoLoaded() const = 0;



	/**
	 * Returns the width of the video's frames.
	 * @return the width of the video's frames
	 */
	virtual uint16 getWidth() const = 0;

	/**
	 * Returns the height of the video's frames.
	 * @return the height of the video's frames
	 */
	virtual uint16 getHeight() const = 0;

	/**
	 * Get the pixel format of the currently loaded video.
	 */
	virtual Graphics::PixelFormat getPixelFormat() const = 0;

	/**
	 * Get the palette for the video in RGB format (if 8bpp or less).
	 */
	virtual const byte *getPalette() { return 0; }

	/**
	 * Returns if the palette is dirty or not.
	 */
	virtual bool hasDirtyPalette() const { return false; }

	/**
	 * Set the system palette to the palette returned by getPalette.
	 * @see getPalette
	 */
	void setSystemPalette();

	/**
	 * Returns the current frame number of the video.
	 * @return the last frame decoded by the video
	 */
	virtual int32 getCurFrame() const { return _curFrame; }

	/**
	 * Returns the number of frames in the video.
	 * @return the number of frames in the video
	 */
	virtual uint32 getFrameCount() const = 0;

	/**
	 * Returns the time (in ms) that the video has been running.
	 * This is based on the "wall clock" time as determined by
	 * OSystem::getMillis, and takes pausing the video into account.
	 *
	 * As such, it can differ from what multiplying getCurFrame() by
	 * some constant would yield, e.g. for a video with non-constant
	 * frame rate.
	 */
	virtual uint32 getElapsedTime() const;

	/**
	 * Return the time (in ms) until the next frame should be displayed.
	 */
	virtual uint32 getTimeToNextFrame() const = 0;

	/**
	 * Check whether a new frame should be decoded, i.e. because enough
	 * time has elapsed since the last frame was decoded.
	 * @return whether a new frame should be decoded or not
	 */
	virtual bool needsUpdate() const;

	/**
	 * Decode the next frame into a surface and return the latter.
	 * @return a surface containing the decoded frame, or 0
	 * @note Ownership of the returned surface stays with the VideoDecoder,
	 *       hence the caller must *not* free it.
	 * @note this may return 0, in which case the last frame should be kept on screen
	 */
	virtual const Graphics::Surface *decodeNextFrame() = 0;

	/**
	 * Returns if the video has finished playing or not.
	 * @return true if the video has finished playing or if none is loaded, false otherwise
	 */
	virtual bool endOfVideo() const;

	/**
	 * Pause or resume the video. This should stop/resume any audio playback
	 * and other stuff. The initial pause time is kept so that any timing
	 * variables can be updated appropriately.
	 *
	 * This is a convenience method which automatically keeps track on how
	 * often the video has been paused, ensuring that after pausing an video
	 * e.g. twice, it has to be unpaused twice before actuallying resuming.
	 *
	 * @param pause		true to pause the video, false to resume it
	 */
	void pauseVideo(bool pause);

	/**
	 * Return whether the video is currently paused or not.
	 */
	bool isPaused() const { return _pauseLevel != 0; }

protected:
	/**
	 * Resets _curFrame and _startTime. Should be called from every close() function.
	 */
	void reset();

	/**
	 * Actual implementation of pause by subclasses. See pause()
	 * for details.
	 */
	virtual void pauseVideoIntern(bool pause) {}

	/**
	 * Add the time the video has been paused to maintain sync
	 */
	virtual void addPauseTime(uint32 ms) { _startTime += ms; }

	/**
	 * Reset the pause start time (which should be called when seeking)
	 */
	void resetPauseStartTime();

	int32 _curFrame;
	int32 _startTime;

private:
	uint32 _pauseLevel;
	uint32 _pauseStartTime;
};

/**
 * A VideoDecoder wrapper that implements getTimeToNextFrame() based on getFrameRate().
 */
class FixedRateVideoDecoder : public virtual VideoDecoder {
public:
	uint32 getTimeToNextFrame() const;

protected:
	/**
	 * Return the frame rate in frames per second.
	 * This returns a Rational because videos can have rates that are not integers and
	 * there are some videos with frame rates < 1.
	 */
	virtual Common::Rational getFrameRate() const = 0;

private:
	uint32 getFrameBeginTime(uint32 frame) const;
};

/**
 * A VideoDecoder that can be rewound back to the beginning.
 */
class RewindableVideoDecoder : public virtual VideoDecoder {
public:
	/**
	 * Rewind to the beginning of the video.
	 */
	virtual void rewind() = 0;
};

/**
 * A VideoDecoder that can seek to a frame or point in time.
 */
class SeekableVideoDecoder : public virtual RewindableVideoDecoder {
public:
	/**
	 * Seek to the specified time.
	 *
	 * This will round to the previous frame showing. If the time would happen to
	 * land while a frame is showing, this function will seek to the beginning of that
	 * frame. In other words, there is *no* subframe accuracy. This may change in a
	 * later revision of the API.
	 */
	virtual void seekToTime(Audio::Timestamp time) = 0;

	/**
	 * Seek to the specified time (in ms).
	 */
	void seekToTime(uint32 msecs) { seekToTime(Audio::Timestamp(msecs, 1000)); }

	/**
	 * Implementation of RewindableVideoDecoder::rewind().
	 */
	virtual void rewind() { seekToTime(0); }

	/**
	 * Get the total duration of the video (in ms).
	 */
	virtual uint32 getDuration() const = 0;
};

} // End of namespace Video

#endif
