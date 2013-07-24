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

#ifndef ZVISION_RENDER_MANAGER_H
#define ZVISION_RENDER_MANAGER_H

#include "common/types.h"
#include "common/rect.h"

#include "graphics/pixelformat.h"

#include "zvision/render_table.h"

class OSystem;

namespace Common {
class String;
}

namespace Video {
class VideoDecoder;
}

namespace ZVision {

class RenderManager {
public:
	RenderManager(OSystem *system, const int width, const int height);

private:
	OSystem *_system;
	const int _width;
	const int _height;
	const Graphics::PixelFormat _pixelFormat;
	RenderTable _renderTable;

	Video::VideoDecoder *_currentVideo;
	byte *_scaledVideoFrameBuffer;

	bool _needsScreenUpdate;

public:
	void initialize();
	void updateScreen(bool isConsoleActive);

	/**
	 * Start a video playing. It will also load the first frame of the video.
	 *
	 * @param videoDecoder    The video to play
	 */
	void startVideo(Video::VideoDecoder *videoDecoder);
	/**
	 * @return    Is a video currently being played
	 */
	bool isVideoPlaying() { return _currentVideo == 0; }
	/**
	 * Cancels a video prematurely. Any sound remaining in the queue will continue to play.
	 * The last frame of the video will remain on the screen until something else overwrites it
	 */
	void cancelVideo();

	/**
	 * Blits the image to the screen. Actual screen updates won't happen until the end of the frame.
	 * The image will be clipped to fit inside the window.
	 *
	 * @param fileName                   Name of the image file
	 * @param x                          X position where the image should be put
	 * @param y                          Y position where the image should be put
	 */
	void renderImageToScreen(const Common::String &fileName, uint32 x, uint32 y);

	/**
	 * Set how the frame should be rendered
	 *
	 * @param state    One of the RenderStates
	 */
	void setRenderState(RenderTable::RenderState state);

	bool needsScreenUpdate() { return _needsScreenUpdate; };

private:
	/**
	 * Checks the time since the last video frame, and blits the next frame to the screen
	 */
	void continueVideo();
};

} // End of namespace ZVision

#endif
