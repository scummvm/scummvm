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

#ifndef PEGASUS_VIDEO_H
#define PEGASUS_VIDEO_H

#include "common/array.h"

namespace Common {
	class String;
}

namespace Video {
	class QuickTimeDecoder;
}

namespace Pegasus {

class PegasusEngine;

struct VideoEntry {
	Video::QuickTimeDecoder *video;
	uint16 x;
	uint16 y;
	bool loop;
	Common::String filename;

	Video::QuickTimeDecoder *operator->() const { assert(video); return video; }
};

typedef int32 VideoHandle;

enum {
	NULL_VID_HANDLE = -1
};

class VideoManager {
public:
	VideoManager(PegasusEngine *vm);
	~VideoManager();

	bool loadTimeZoneVideo(const Common::String &filename);
	void drawTimeZoneVideoFrame(uint32 time);
	void playTimeZoneVideoSegment(uint32 startTime, uint32 endTime);
	
	// Generic movie functions
	void playMovie(Common::String filename, uint16 x = 0, uint16 y = 0);
	void playMovieCentered(Common::String filename);
	VideoHandle playBackgroundMovie(Common::String filename, int x = 0, int y = 0, bool loop = false);
	bool updateBackgroundMovies();
	void pauseVideos();
	void resumeVideos();
	void stopVideos();
	void waitUntilMovieEnds(VideoHandle videoHandle);

	void seekToTime(VideoHandle handle, uint32 time);

	// Helper functions
	void copyFrameToScreen(const Graphics::Surface *frame, int width, int height, int x, int y);

private:
	PegasusEngine *_vm;

	Video::QuickTimeDecoder *_timeZoneVideo;

	// Keep tabs on any videos playing
	Common::Array<VideoEntry> _videoStreams;
	uint32 _pauseStart;
};

} // End of namespace Pegasus

#endif
