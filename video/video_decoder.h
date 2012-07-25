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

#include "audio/mixer.h"
#include "audio/timestamp.h"	// TODO: Move this to common/ ?
#include "common/array.h"
#include "common/str.h"

namespace Audio {
class AudioStream;
class RewindableAudioStream;
class SeekableAudioStream;
}

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
 * @note This class is now deprecated in favor of AdvancedVideoDecoder.
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
	 * @note This function is now deprecated. There is no replacement.
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
	 * Returns the time position (in ms) of the current video.
	 * This can be based on the "wall clock" time as determined by
	 * OSystem::getMillis() or the current time of any audio track
	 * running in the video, and takes pausing the video into account.
	 *
	 * As such, it will differ from what multiplying getCurFrame() by
	 * some constant would yield, e.g. for a video with non-constant
	 * frame rate.
	 *
	 * Due to the nature of the timing, this value may not always be
	 * completely accurate (since our mixer does not have precise
	 * timing).
	 */
	virtual uint32 getTime() const;

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

	/**
	 * Get the current volume at which the audio in the video is being played
	 * @return the current volume at which the audio in the video is being played
	 */
	virtual byte getVolume() const { return _audioVolume; }

	/**
	 * Set the volume at which the audio in the video should be played.
	 * This setting remains until reset() is called (which may be called
	 * from loadStream() or close()). The default volume is the maximum.
	 *
	 * @note This function calls updateVolume() by default.
	 *
	 * @param volume The volume at which to play the audio in the video
	 */
	virtual void setVolume(byte volume);

	/**
	 * Get the current balance at which the audio in the video is being played
	 * @return the current balance at which the audio in the video is being played
	 */
	virtual int8 getBalance() const { return _audioBalance; }

	/**
	 * Set the balance at which the audio in the video should be played.
	 * This setting remains until reset() is called (which may be called
	 * from loadStream() or close()). The default balance is 0.
	 *
	 * @note This function calls updateBalance() by default.
	 *
	 * @param balance The balance at which to play the audio in the video
	 */
	virtual void setBalance(int8 balance);

protected:
	/**
	 * Resets _curFrame and _startTime. Should be called from every close() function.
	 * @note This function is now deprecated. There is no replacement.
	 */
	void reset();

	/**
	 * Actual implementation of pause by subclasses. See pause()
	 * for details.
	 * @note This function is now deprecated. There is no replacement.
	 */
	virtual void pauseVideoIntern(bool pause) {}

	/**
	 * Add the time the video has been paused to maintain sync
	 * @note This function is now deprecated. There is no replacement.
	 */
	virtual void addPauseTime(uint32 ms) { _startTime += ms; }

	/**
	 * Reset the pause start time (which should be called when seeking)
	 * @note This function is now deprecated. There is no replacement.
	 */
	void resetPauseStartTime();

	/**
	 * Update currently playing audio tracks with the new volume setting
	 * @note This function is now deprecated. There is no replacement.
	 */
	virtual void updateVolume() {}

	/**
	 * Update currently playing audio tracks with the new balance setting
	 * @note This function is now deprecated. There is no replacement.
	 */
	virtual void updateBalance() {}

	int32 _curFrame;
	int32 _startTime;

// FIXME: These are protected until the new API takes over this one
//private:
	uint32 _pauseLevel;
	uint32 _pauseStartTime;
	byte _audioVolume;
	int8 _audioBalance;
};

/**
 * Improved interface for video decoder classes.
 */
class AdvancedVideoDecoder : public VideoDecoder {
public:
	AdvancedVideoDecoder();
	virtual ~AdvancedVideoDecoder() {}

	// Old API Non-changing
	// loadFile()
	// loadStream()
	// needsUpdate()

	// Old API Changing
	virtual void close();
	bool isVideoLoaded() const;
	virtual uint16 getWidth() const;
	virtual uint16 getHeight() const;
	virtual Graphics::PixelFormat getPixelFormat() const;
	virtual const Graphics::Surface *decodeNextFrame();
	const byte *getPalette();
	bool hasDirtyPalette() const { return _dirtyPalette; }
	int getCurFrame() const;
	uint32 getFrameCount() const;
	uint32 getTime() const;
	uint32 getTimeToNextFrame() const;
	bool endOfVideo() const;

	// New API
	/**
	 * Returns if a video is rewindable or not. The default implementation
	 * polls each track for rewindability.
	 */
	virtual bool isRewindable() const;

	/**
	 * Rewind a video to its beginning.
	 *
	 * If the video is playing, it will continue to play. The default
	 * implementation will rewind each track.
	 *
	 * @return true on success, false otherwise
	 */
	virtual bool rewind();

	/**
	 * Returns if a video is seekable or not. The default implementation
	 * polls each track for seekability.
	 */
	virtual bool isSeekable() const;

	/**
	 * Seek to a given time in the video.
	 *
	 * If the video is playing, it will continue to play. The default
	 * implementation will seek each track.
	 *
	 * @param time The time to seek to
	 * @return true on success, false otherwise
	 */
	virtual bool seek(const Audio::Timestamp &time);

	/**
	 * Begin playback of the video.
	 *
	 * @note This has no effect is the video is already playing.
	 */
	void start();

	/**
	 * Stop playback of the video.
	 *
	 * @note This will close() the video if it is not rewindable.
	 */
	void stop();

	/**
	 * Returns if the video is currently playing or not.
	 * @todo Differentiate this function from endOfVideo()
	 */
	bool isPlaying() const { return _isPlaying; }

	/**
	 * Get the duration of the video.
	 *
	 * If the duration is unknown, this will return 0.
	 */
	virtual Audio::Timestamp getDuration() const;

	/**
	 * Add an audio track from a stream file.
	 */
	bool addStreamFileTrack(const Common::String &baseName);

	// Future API
	//void setRate(const Common::Rational &rate);
	//Common::Rational getRate() const;
	//void setStartTime(const Audio::Timestamp &startTime);
	//Audio::Timestamp getStartTime() const;
	//void setStopTime(const Audio::Timestamp &stopTime);
	//Audio::Timestamp getStopTime() const;
	//void setSegment(const Audio::Timestamp &startTime, const Audio::Timestamp &stopTime);

protected:
	// Old API
	void pauseVideoIntern(bool pause);
	void updateVolume();
	void updateBalance();

	// New API

	/**
	 * An abstract representation of a track in a movie.
	 */
	class Track {
	public:
		Track();
		virtual ~Track() {}

		/**
		 * The types of tracks this class can be.
		 */
		enum TrackType {
			kTrackTypeNone,
			kTrackTypeVideo,
			kTrackTypeAudio
		};

		/**
		 * Get the type of track.
		 */
		virtual TrackType getTrackType() const = 0;

		/**
		 * Return if the track has finished.
		 */
		virtual bool endOfTrack() const = 0;

		/**
		 * Return if the track is rewindable.
		 */
		virtual bool isRewindable() const;

		/**
		 * Rewind the video to the beginning.
		 * @return true on success, false otherwise.
		 */
		virtual bool rewind();

		/**
		 * Return if the track is seekable.
		 */
		virtual bool isSeekable() const { return false; }

		/**
		 * Seek to the given time.
		 * @param time The time to seek to, from the beginning of the video.
		 * @return true on success, false otherwise.
		 */
		virtual bool seek(const Audio::Timestamp &time) { return false; }

		/**
		 * Start playback of the track.
		 */
		virtual void start() {}

		/**
		 * Stop playback of the track.
		 */
		virtual void stop() {}

		/**
		 * Set the pause status of the track.
		 */
		void pause(bool shouldPause) {}

		/**
		 * Return if the track is paused.
		 */
		bool isPaused() const { return _paused; }

		/**
		 * Get the start time of the track (starting from the beginning of the
		 * movie).
		 */
		virtual Audio::Timestamp getStartTime() const;

		/**
		 * Get the duration of the track (starting from this track's start time).
		 *
		 * By default, this returns 0 for unknown.
		 */
		virtual Audio::Timestamp getDuration() const;

	protected:
		/**
		 * Function called by pause() for subclasses to implement.
		 */
		void pauseIntern(bool pause);

	private:
		bool _paused;
	};

	/**
	 * An abstract representation of a video track.
	 */
	class VideoTrack : public Track {
	public:
		VideoTrack() {}
		virtual ~VideoTrack() {}

		TrackType getTrackType() const  { return kTrackTypeVideo; }

		// TODO: Document
		virtual uint16 getWidth() const = 0;
		virtual uint16 getHeight() const = 0;
		virtual Graphics::PixelFormat getPixelFormat() const = 0;
		virtual int getCurFrame() const = 0;
		virtual int getFrameCount() const { return 0; }
		virtual uint32 getNextFrameStartTime() const = 0;
		virtual const Graphics::Surface *decodeNextFrame() = 0;
		virtual const byte *getPalette() const { return 0; }
		virtual bool hasDirtyPalette() const { return false; }
	};

	/**
	 * A VideoTrack that is played at a constant rate.
	 *
	 * If the frame count is unknown, you must override endOfTrack().
	 */
	class FixedRateVideoTrack : public VideoTrack {
	public:
		FixedRateVideoTrack() {}
		virtual ~FixedRateVideoTrack() {}

		virtual bool endOfTrack() const;
		uint32 getNextFrameStartTime() const;
		virtual Audio::Timestamp getDuration() const;

	protected:
		/**
		 * Get the rate at which this track is played.
		 */
		virtual Common::Rational getFrameRate() const = 0;
	};

	/**
	 * An abstract representation of an audio track.
	 */
	class AudioTrack : public Track {
	public:
		AudioTrack() {}
		virtual ~AudioTrack() {}

		TrackType getTrackType() const { return kTrackTypeAudio; }

		virtual bool endOfTrack() const;
		void start();
		void stop();

		// TODO: Document
		byte getVolume() const { return _volume; }
		void setVolume(byte volume);
		int8 getBalance() const { return _balance; }
		void setBalance(int8 balance);
		uint32 getRunningTime() const;

		virtual Audio::Mixer::SoundType getSoundType() const { return Audio::Mixer::kPlainSoundType; }

	protected:
		void pauseIntern(bool pause);

		// TODO: Document
		virtual Audio::AudioStream *getAudioStream() const = 0;

	private:
		Audio::SoundHandle _handle;
		byte _volume;
		int8 _balance;
	};

	/**
	 * An AudioTrack that implements isRewindable() and rewind() using
	 * the RewindableAudioStream API.
	 */
	class RewindableAudioTrack : public AudioTrack {
	public:
		RewindableAudioTrack() {}
		virtual ~RewindableAudioTrack() {}

		bool isRewindable() const { return true; }
		bool rewind();

	protected:
		Audio::AudioStream *getAudioStream() const;

		// TODO: Document
		virtual Audio::RewindableAudioStream *getRewindableAudioStream() const = 0;
	};

	/**
	 * An AudioTrack that implements isSeekable() and seek() using
	 * the SeekableAudioStream API.
	 */
	class SeekableAudioTrack : public AudioTrack {
	public:
		SeekableAudioTrack() {}
		virtual ~SeekableAudioTrack() {}

		bool isSeekable() const { return true; }
		bool seek(const Audio::Timestamp &time);

		Audio::Timestamp getDuration() const;

	protected:
		Audio::AudioStream *getAudioStream() const;

		// TODO: Document
		virtual Audio::SeekableAudioStream *getSeekableAudioStream() const = 0;
	};

	/**
	 * A SeekableAudioTrack that constructs its SeekableAudioStream using
	 * SeekableAudioStream::openStreamFile()
	 */
	class StreamFileAudioTrack : public SeekableAudioTrack {
	public:
		StreamFileAudioTrack();
		~StreamFileAudioTrack();

		/**
		 * Load the track from a file with the given base name.
		 *
		 * @return true on success, false otherwise
		 */
		bool loadFromFile(const Common::String &baseName);

	protected:
		Audio::SeekableAudioStream *_stream;
		Audio::SeekableAudioStream *getSeekableAudioStream() const { return _stream; }
	};

	/**
	 * Decode enough data for the next frame and enough audio to last that long.
	 *
	 * This function is used by the default decodeNextFrame() function. A subclass
	 * of a Track may decide to just have its decodeNextFrame() function read
	 * and decode the frame.
	 */
	virtual void readNextPacket() {}

	/**
	 * Define a track to be used by this class.
	 *
	 * The pointer is then owned by this base class.
	 */
	void addTrack(Track *track);

	/**
	 * Whether or not getTime() will sync with a playing audio track.
	 *
	 * A subclass should override this to disable this feature.
	 */
	virtual bool useAudioSync() const { return true; }

	/**
	 * Get the given track based on its index.
	 *
	 * @return A valid track pointer on success, 0 otherwise
	 */
	Track *getTrack(uint track);

	/**
	 * Get the given track based on its index
	 *
	 * @return A valid track pointer on success, 0 otherwise
	 */
	const Track *getTrack(uint track) const;

	/**
	 * Find out if all video tracks have finished
	 *
	 * This is useful if one wants to figure out if they need to buffer all
	 * remaining audio in a file.
	 */
	bool endOfVideoTracks() const;

private:
	// Tracks owned by this AdvancedVideoDecoder
	typedef Common::Array<Track *> TrackList;
	TrackList _tracks;
	VideoTrack *findNextVideoTrack();
	const VideoTrack *findNextVideoTrack() const;

	// Current playback status
	bool _isPlaying, _needsRewind;
	Audio::Timestamp _audioStartOffset;

	// Palette settings from individual tracks
	mutable bool _dirtyPalette;
	const byte *_palette;

	// Internal helper functions
	void stopAllTracks();
	void startAllTracks();
};

/**
 * A VideoDecoder wrapper that implements getTimeToNextFrame() based on getFrameRate().
 * @note This class is now deprecated. Use AdvancedVideoDecoder instead.
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
 * @note This class is now deprecated. Use AdvancedVideoDecoder instead.
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
 * @note This class is now deprecated. Use AdvancedVideoDecoder instead.
 */
class SeekableVideoDecoder : public virtual RewindableVideoDecoder {
public:
	/**
	 * Seek to the specified time.
	 */
	virtual void seekToTime(const Audio::Timestamp &time) = 0;

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
