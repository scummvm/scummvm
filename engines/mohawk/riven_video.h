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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MOHAWK_RIVEN_VIDEO_H
#define MOHAWK_RIVEN_VIDEO_H

#include "common/list.h"
#include "common/noncopyable.h"

namespace Video {
class QuickTimeDecoder;
}

namespace Mohawk {

class MohawkEngine_Riven;

/**
 * A video monitored by the VideoManager
 */
class RivenVideo : private Common::NonCopyable {
public:
	RivenVideo(MohawkEngine_Riven *vm, uint16 code);
	~RivenVideo();

	/** Load the video from the archive */
	void load(uint16 id);

	/** Free resources allocated for the movie, but allow to load it again later */
	void close();

	/** Start playing the video */
	void play();

	/** Play the video until it completes or reaches the specified timestamp */
	void playBlocking(int32 endTime = -1);

	/** Has the video reached its end? */
	bool endOfVideo() const;

	/** Is the video looping? */
	bool isLooping() const { return _loop; }

	/** Is the video enabled? (Drawing to the screen) */
	bool isEnabled() const { return _enabled; }

	/** Get the ID of the video */
	uint16 getId() const { return _id; }

	/** Get the slot used by the video in the video manager */
	uint16 getSlot() const { return _slot; }

	/** Get the current frame of the video */
	int getCurFrame() const;

	/** Get the frame count of the video */
	uint32 getFrameCount() const;

	/** Get the current time position of the video */
	uint32 getTime() const;

	/** Get the duration of the video */
	uint32 getDuration() const;

	/** Move the video to the specified coordinates */
	void moveTo(uint16 x, uint16 y) { _x = x; _y = y; }

	/** Set the video to loop (true) or not (false) */
	void setLooping(bool loop) { _loop = loop; }

	/** Enable the video */
	void enable();

	/** Disable the video */
	void disable();

	/** Seek to the given time */
	void seek(uint32 time);

	/** Pause the video */
	void pause(bool isPaused);

	/** Stop playing the video */
	void stop();

	/** Is the video playing? */
	bool isPlaying() const;

	/** Set the volume of the video */
	void setVolume(int volume);

	/** Checks if a video is playing and is waiting to display the next frame */
	bool needsUpdate() const;

	/** Draw the next frame to the system screen */
	void drawNextFrame();
private:
	// Non-changing variables
	MohawkEngine_Riven *_vm;
	Video::QuickTimeDecoder *_video;
	uint16 _id;
	uint16 _slot;

	// Playback variables
	uint16 _x;
	uint16 _y;
	bool _loop;
	bool _enabled;
	bool _playing;
};

class RivenVideoManager {
public:
	explicit RivenVideoManager(MohawkEngine_Riven *vm);
	~RivenVideoManager();

	void updateMovies();
	void pauseVideos();
	void resumeVideos();
	void closeVideos();
	void removeVideos();
	void disableAllMovies();

	RivenVideo *openSlot(uint16 slot);
	RivenVideo *getSlot(uint16 slot);

private:
	MohawkEngine_Riven *_vm;

	// Keep tabs on any videos playing
	typedef Common::List<RivenVideo *> VideoList;
	VideoList _videos;
};

} // End of namespace Mohawk

#endif
