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

struct MLSTRecord {
	uint16 index;
	uint16 movieID;
	uint16 code;
	uint16 left;
	uint16 top;
	uint16 u0[3];
	uint16 loop;
	uint16 volume;
	uint16 u1;
};

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

/**
 * A handle for manipulating a video
 */
class VideoHandle {
	// The private members should be able to be manipulated by VideoManager
	friend class VideoManager;

public:
	/**
	 * Default constructor
	 */
	VideoHandle() {}

	/**
	 * Copy constructor
	 */
	VideoHandle(const VideoHandle &handle);

	/**
	 * Is this handle pointing to a valid video entry?
	 */
	bool isValid() const { return _ptr && _ptr->isOpen(); }

	/**
	 * Convenience implicit cast to bool
	 */
	operator bool() const { return isValid(); }

	/**
	 * Simple equality operator
	 */
	bool operator==(const VideoHandle &other) const { return _ptr.get() == other._ptr.get(); }

	/**
	 * Simple inequality operator
	 */
	bool operator!=(const VideoHandle &other) const { return !(*this == other); }

	/**
	 * Convenience operator-> override to give direct access to the VideoEntry
	 */
	VideoEntryPtr operator->() const { return _ptr; }

private:
	/**
	 * Constructor for internal VideoManager use
	 */
	VideoHandle(VideoEntryPtr ptr);

	/**
	 * The video entry this is associated with
	 */
	VideoEntryPtr _ptr;
};

class VideoManager {
public:
	VideoManager(MohawkEngine *vm);
	~VideoManager();

	// Generic movie functions
	void playMovieBlocking(const Common::String &filename, uint16 x = 0, uint16 y = 0, bool clearScreen = false);
	void playMovieBlockingCentered(const Common::String &filename, bool clearScreen = true);
	VideoHandle playMovie(const Common::String &filename);
	VideoHandle playMovie(uint16 id);
	bool updateMovies();
	void pauseVideos();
	void resumeVideos();
	void stopVideos();
	bool isVideoPlaying();

	// Riven-related functions
	void activateMLST(uint16 mlstId, uint16 card);
	void clearMLST();
	void disableAllMovies();
	VideoHandle playMovieRiven(uint16 id);
	void playMovieBlockingRiven(uint16 id);
	VideoHandle findVideoHandleRiven(uint16 id);
	void stopMovieRiven(uint16 id);

	// Handle functions
	VideoHandle findVideoHandle(uint16 id);
	VideoHandle findVideoHandle(const Common::String &fileName);
	void waitUntilMovieEnds(VideoHandle handle);
	void delayUntilMovieEnds(VideoHandle handle);
	void drawVideoFrame(VideoHandle handle, const Audio::Timestamp &time);

private:
	MohawkEngine *_vm;

	// Riven-related variables
	Common::Array<MLSTRecord> _mlstRecords;

	// Keep tabs on any videos playing
	typedef Common::List<VideoEntryPtr> VideoList;
	VideoList _videos;

	// Utility functions for managing entries
	VideoEntryPtr open(uint16 id);
	VideoEntryPtr open(const Common::String &fileName);
	
	VideoList::iterator findEntry(VideoEntryPtr ptr);
	void removeEntry(VideoEntryPtr ptr);

	// Dithering control
	bool _enableDither;
	void checkEnableDither(VideoEntryPtr &entry);
};

} // End of namespace Mohawk

#endif
