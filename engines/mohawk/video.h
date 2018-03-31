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

#ifndef MOHAWK_VIDEO_H
#define MOHAWK_VIDEO_H

#include "audio/mixer.h"
#include "audio/timestamp.h"
#include "common/array.h"
#include "common/list.h"
#include "common/noncopyable.h"
#include "common/ptr.h"
#include "common/rational.h"
#include "graphics/pixelformat.h"

namespace Video {
class VideoDecoder;
}

namespace Mohawk {

class MohawkEngine;

/**
 * A video monitored by the VideoManager
 */
class VideoEntry : private Common::NonCopyable {
	// The private members should be able to be manipulated by VideoManager
	friend class VideoManager;

private:
	// Hide the destructor/constructor
	// Only VideoManager should be allowed
	VideoEntry();
	VideoEntry(Video::VideoDecoder *video, const Common::String &fileName);
	VideoEntry(Video::VideoDecoder *video, int id);

public:
	~VideoEntry();

	/**
	 * Convenience implicit cast to bool
	 */
	operator bool() const { return isOpen(); }

	/**
	 * Is the video open?
	 */
	bool isOpen() const { return _video != 0; }

	/**
	 * Close the video
	 */
	void close();

	/**
	 * Has the video reached its end?
	 */
	bool endOfVideo() const;

	/**
	 * Get the X position of where the video is displayed
	 */
	uint16 getX() const { return _x; }

	/**
	 * Get the Y position of where the video is displayed
	 */
	uint16 getY() const { return _y; }

	/**
	 * Is the video looping?
	 */
	bool isLooping() const { return _loop; }

	/**
	 * Is the video enabled? (Drawing to the screen)
	 */
	bool isEnabled() const { return _enabled; }

	/**
	 * Get the start time of the video bounds
	 */
	const Audio::Timestamp &getStart() const { return _start; }

	/**
	 * Get the file name of the video, or empty if by ID
	 */
	const Common::String &getFileName() const { return _fileName; }

	/**
	 * Get the ID of the video, or -1 if by file name
	 */
	int getID() const { return _id; }

	/**
	 * Get the current frame of the video
	 */
	int getCurFrame() const;

	/**
	 * Get the frame count of the video
	 */
	uint32 getFrameCount() const;

	/**
	 * Get the current time position of the video
	 */
	uint32 getTime() const;

	/**
	 * Get the duration of the video
	 */
	Audio::Timestamp getDuration() const;

	/**
	 * Get the current playback rate of the videos
	 */
	Common::Rational getRate() const;

	/**
	 * Move the x position of the video
	 */
	void setX(uint16 x) { _x = x; }

	/**
	 * Move the y position of the video
	 */
	void setY(uint16 y) { _y = y; }

	/**
	 * Move the video to the specified coordinates
	 */
	void moveTo(uint16 x, uint16 y) { setX(x); setY(y); }

	/**
	 * Center the video on the screen
	 */
	void center();

	/**
	 * Set the start time when using video bounds
	 */
	void setStart(const Audio::Timestamp &time) { _start = time; }

	/**
	 * Set the video to loop (true) or not (false)
	 */
	void setLooping(bool loop) { _loop = loop; }

	/**
	 * Set the video's enabled status
	 */
	void setEnabled(bool enabled) { _enabled = enabled; }

	/**
	 * Set the bounds of the video
	 *
	 * This automatically seeks to the start time
	 */
	void setBounds(const Audio::Timestamp &startTime, const Audio::Timestamp &endTime);

	/**
	 * Seek to the given time
	 */
	void seek(const Audio::Timestamp &time);

	/**
	 * Set the playback rate
	 */
	void setRate(const Common::Rational &rate);

	/**
	 * Pause the video
	 */
	void pause(bool isPaused);

	/**
	 * Start playing the video
	 */
	void start();

	/**
	 * Stop playing the video
	 */
	void stop();

	/**
	 * Is the video playing?
	 */
	bool isPlaying() const;

	/**
	 * Get the volume of the video
	 */
	int getVolume() const;

	/**
	 * Set the volume of the video
	 */
	void setVolume(int volume);

private:
	// Non-changing variables
	Video::VideoDecoder *_video;
	Common::String _fileName; // External video files
	int _id;                  // Internal Mohawk files

	// Playback variables
	uint16 _x;
	uint16 _y;
	bool _loop;
	bool _enabled;
	Audio::Timestamp _start;
};

typedef Common::SharedPtr<VideoEntry> VideoEntryPtr;

class VideoManager {
public:
	explicit VideoManager(MohawkEngine *vm);
	virtual ~VideoManager();

	// Generic movie functions
	VideoEntryPtr playMovie(const Common::String &filename, Audio::Mixer::SoundType soundType = Audio::Mixer::kPlainSoundType);
	VideoEntryPtr playMovie(uint16 id);
	bool updateMovies();
	void pauseVideos();
	void resumeVideos();
	void stopVideos();
	bool isVideoPlaying();

	// Handle functions
	VideoEntryPtr findVideo(uint16 id);
	VideoEntryPtr findVideo(const Common::String &fileName);
	void drawVideoFrame(const VideoEntryPtr &video, const Audio::Timestamp &time);
	void removeEntry(const VideoEntryPtr &video);

protected:
	MohawkEngine *_vm;

	// Keep tabs on any videos playing
	typedef Common::List<VideoEntryPtr> VideoList;
	VideoList _videos;

	// Utility functions for managing entries
	VideoEntryPtr open(uint16 id);
	VideoEntryPtr open(const Common::String &fileName, Audio::Mixer::SoundType soundType);

	VideoList::iterator findEntry(VideoEntryPtr ptr);

	bool drawNextFrame(VideoEntryPtr videoEntry);

	// Dithering control
	bool _enableDither;
	void checkEnableDither(VideoEntryPtr &entry);
};

} // End of namespace Mohawk

#endif
