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
#include "common/stream.h"

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
	virtual ~VideoDecoder();

	/**
	 * Returns the width of the video
	 * @return the width of the video
	 */
	virtual int getWidth();

	/**
	 * Returns the height of the video
	 * @return the height of the video
	 */
	virtual int getHeight();

	/**
	 * Returns the current frame number of the video
	 * @return the current frame number of the video
	 */
	virtual int32 getCurFrame();

	/**
	 * Returns the amount of frames in the video
	 * @return the amount of frames in the video
	 */
	virtual int32 getFrameCount();

	/**
	 * Returns the frame rate of the video
	 * @return the frame rate of the video
	 */
	virtual int32 getFrameRate();

	/**
	 * Returns the time to wait for each frame in 1/100 ms (to avoid rounding errors)
	 * @return the time to wait for each frame in 1/100 ms (to avoid rounding errors)
	 */
	virtual int32 getFrameDelay();

	/**
	 * Returns the current A/V lag in 1/100 ms (to avoid rounding errors)
	 * If > 0, audio lags behind
	 * If < 0, video lags behind
	 * @return the current A/V lag in 1/100 ms (to avoid rounding errors)
	 */
	virtual int32 getAudioLag();

	/**
	 * Returns the time to wait until the next frame in ms, minding any lag
	 * @return the time to wait until the next frame in ms
	 */
	virtual uint32 getFrameWaitTime();

	/**
	 * Load a video file
	 * @param filename	the filename to load
	 */
	virtual bool loadFile(const char *filename) = 0;

	/**
	 * Close a video file
	 */
	virtual void closeFile() = 0;

	/**
	 * Returns if a video file is loaded or not
	 */
	bool isVideoLoaded() { return (_fileStream != NULL); }

	/**
	 * Set RGB palette, based on current frame
	 * @param pal		the RGB palette data
	 */
	virtual void setPalette(byte *pal);

	/**
	 * Gets the value of the pixel at the specified x and y coordinates
	 * Note: This method assumes that the video's pitch equals its width, and that
	 * the video has an 8bpp palette
	 * @param x	the x coordinate of the pixel
	 * @param y	the y coordinate of the pixel
	 */
	byte getPixel(int x, int y) {
		return *(_videoFrameBuffer + y * _videoInfo.width + x * 1);
	}

	/**
	 * Gets the value of the pixel at the specified offset
	 * @param offset	the offset of the pixel in the video buffer
	 */
	byte getPixel(int offset) { return getPixel(offset, 0); }

	/**
	 * Return the black palette color for the current frame
	 */
	byte getBlack() { return _curFrameBlack; }

	/**
	 * Return the white palette color for the current frame
	 */
	byte getWhite() { return _curFrameWhite; }

	/**
	 * Copy current frame into the specified position of the destination
	 * buffer.
	 * @param dst		the buffer
	 * @param x		the x position of the buffer
	 * @param y		the y position of the buffer
	 * @param pitch		the pitch of buffer
	 */
	void copyFrameToBuffer(byte *dst, uint x, uint y, uint pitch);

	/**
	 * Decode the next frame to _videoFrameBuffer
	 */
	virtual bool decodeNextFrame() = 0;

protected:
	struct {
		uint32 width;
		uint32 height;
		uint32 frameCount;
		int32 frameRate;
		int32 frameDelay;		// 1/100 ms (to avoid rounding errors)
		uint32 firstframeOffset;
		uint32 currentFrame;
		uint32 startTime;
	} _videoInfo;

	byte _curFrameBlack, _curFrameWhite;

	Common::SeekableReadStream *_fileStream;
	byte *_videoFrameBuffer;
};

class VideoPlayer {
public:
	VideoPlayer(VideoDecoder* decoder) : _skipVideo(false), _decoder(decoder)
		{ }
	virtual ~VideoPlayer() { }
	/**
	 * A default implementation of a video player
	 * Plays a non-interactive full screen video till it's stopped by a
	 * specific event
	 * @param filename		the name of the file to play
	 * @param stopEvents	a list of events that can stop the video
	 *
	 * Returns true if the video was played to the end, false if skipped
	 */
	bool playVideo(Common::List<Common::Event> &stopEvents);

	/**
	 * Provides the same functionality as the video player, and it adds the
	 * event of skipping the video with the escape key by default
	 */
	bool playVideo();

protected:
	/**
	 * Perform postprocessing once the frame data is copied to the screen,
	 * right before the frame is drawn. Called by playVideo()
	 */
	virtual void performPostProcessing(byte *screen);

	bool _skipVideo;
	VideoDecoder* _decoder;

	void processVideoEvents(Common::List<Common::Event> &stopEvents);
};

} // End of namespace Graphics

#endif
