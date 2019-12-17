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

#ifndef SOUND_AUDIOSTREAM_H
#define SOUND_AUDIOSTREAM_H

//#include "common/util.h"
//#include "common/scummsys.h"
#include <vector>
#include "ultima/shared/std/string.h"
#include "ultima/ultima6/core/nuvie_defs.h"
#include "types.h"

#include "timestamp.h"


namespace Audio {

//class SeekableAudioStream;

/**
 * Generic audio input stream. Subclasses of this are used to feed arbitrary
 * sampled audio data into ScummVM's audio mixer.
 */
class AudioStream {
public:
	virtual ~AudioStream() {}

	/**
	 * Fill the given buffer with up to numSamples samples. Returns the actual
	 * number of samples read, or -1 if a critical error occurred (note: you
	 * *must* check if this value is less than what you requested, this can
	 * happen when the stream is fully used up).
	 *
	 * Data has to be in native endianess, 16 bit per sample, signed. For stereo
	 * stream, buffer will be filled with interleaved left and right channel
	 * samples, starting with a left sample. Furthermore, the samples in the
	 * left and right are summed up. So if you request 4 samples from a stereo
	 * stream, you will get a total of two left channel and two right channel
	 * samples.
	 */
	virtual int readBuffer(sint16 *buffer, const int numSamples) = 0;

	/** Is this a stereo stream? */
	virtual bool isStereo() const = 0;

	/** Sample rate of the stream. */
	virtual int getRate() const = 0;

	/**
	 * End of data reached? If this returns true, it means that at this
	 * time there is no data available in the stream. However there may be
	 * more data in the future.
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or stop.
	 */
	virtual bool endOfData() const = 0;

	/**
	 * End of stream reached? If this returns true, it means that all data
	 * in this stream is used up and no additional data will appear in it
	 * in the future.
	 * This is used by the mixer to decide whether a given stream shall be
	 * removed from the list of active streams (and thus be destroyed).
	 * By default this maps to endOfData()
	 */
	virtual bool endOfStream() const {
		return endOfData();
	}

	virtual uint32 getLengthInMsec() {
		return 0;
	}
};

/**
 * A rewindable audio stream. This allows for reseting the AudioStream
 * to its initial state. Note that rewinding itself is not required to
 * be working when the stream is being played by Mixer!
 */
class RewindableAudioStream : public AudioStream {
public:
	/**
	 * Rewinds the stream to its start.
	 *
	 * @return true on success, false otherwise.
	 */
	virtual bool rewind() = 0;
};

/**
 * A looping audio stream. This object does nothing besides using
 * a RewindableAudioStream to play a stream in a loop.
 */
class LoopingAudioStream : public AudioStream {
public:
	/**
	 * Creates a looping audio stream object.
	 *
	 * Note that on creation of the LoopingAudioStream object
	 * the underlying stream will be rewound.
	 *
	 * @see makeLoopingAudioStream
	 *
	 * @param stream Stream to loop
	 * @param loops How often to loop (0 = infinite)
	 * @param disposeAfterUse Destroy the stream after the LoopingAudioStream has finished playback.
	 */
	LoopingAudioStream(RewindableAudioStream *stream, uint32 loops, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);
	~LoopingAudioStream();

	int readBuffer(sint16 *buffer, const int numSamples);
	bool endOfData() const;

	bool isStereo() const {
		return _parent->isStereo();
	}
	int getRate() const {
		return _parent->getRate();
	}

	/**
	 * Returns number of loops the stream has played.
	 *
	 * @param numLoops number of loops to play, 0 - infinite
	 */
	uint32 getCompleteIterations() const {
		return _completeIterations;
	}
private:
	RewindableAudioStream *_parent;
	DisposeAfterUse::Flag _disposeAfterUse;

	uint32 _loops;
	uint32 _completeIterations;
};

/**
 * Wrapper functionality to efficiently create a stream, which might be looped.
 *
 * Note that this function does not return a LoopingAudioStream, because it does
 * not create one when the loop count is "1". This allows to keep the runtime
 * overhead down, when the code does not require any functionality only offered
 * by LoopingAudioStream.
 *
 * @param stream  Stream to loop (will be automatically destroyed, when the looping is done)
 * @param loops   How often to loop (0 = infinite)
 * @return A new AudioStream, which offers the desired functionality.
 */
AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint32 loops);

class QueuingAudioStream : public Audio::AudioStream {
public:

	/**
	 * Queue an audio stream for playback. This stream plays all queued
	 * streams, in the order they were queued. If disposeAfterUse is set to
	 * DisposeAfterUse::YES, then the queued stream is deleted after all data
	 * contained in it has been played.
	 */
	virtual void queueAudioStream(Audio::AudioStream *audStream,
	                              DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) = 0;

	/**
	 * Mark this stream as finished. That is, signal that no further data
	 * will be queued to it. Only after this has been done can this
	 * stream ever 'end'.
	 */
	virtual void finish() = 0;

	/**
	 * Return the number of streams still queued for playback (including
	 * the currently playing stream).
	 */
	virtual uint32 numQueuedStreams() const = 0;
};

/**
 * Factory function for an QueuingAudioStream.
 */
QueuingAudioStream *makeQueuingAudioStream(int rate, bool stereo);


class RandomCollectionAudioStream : public Audio::AudioStream {
public:
	/**
	 * Mark this stream as finished. That is, signal that no further data
	 * will be queued to it. Only after this has been done can this
	 * stream ever 'end'.
	 */
	virtual void finish() = 0;
};

/**
 * Factory function for an QueuingAudioStream.
 */
RandomCollectionAudioStream *makeRandomCollectionAudioStream(int rate, bool stereo, std::vector<RewindableAudioStream *>streams, DisposeAfterUse::Flag disposeAfterUse);


/**
 * A seekable audio stream. Subclasses of this class implement an
 * interface for seeking. The seeking itself is not required to be
 * working while the stream is being played by Mixer!
 */
class SeekableAudioStream : public RewindableAudioStream {
public:
	/**
	 * Tries to load a file by trying all available formats.
	 * In case of an error, the file handle will be closed, but deleting
	 * it is still the responsibility of the caller.
	 *
	 * @param basename a filename without an extension
	 * @return  an SeekableAudioStream ready to use in case of success;
	 *          NULL in case of an error (e.g. invalid/nonexisting file)
	 */
	static SeekableAudioStream *openStreamFile(const std::string &basename);

	/**
	 * Seeks to a given offset in the stream.
	 *
	 * @param where offset in milliseconds
	 * @return true on success, false on failure.
	 */
	bool seek(uint32 where) {
		return seek(Timestamp(where, getRate()));
	}

	/**
	 * Seeks to a given offset in the stream.
	 *
	 * @param where offset as timestamp
	 * @return true on success, false on failure.
	 */
	virtual bool seek(const Timestamp &where) = 0;

	/**
	 * Returns the length of the stream.
	 *
	 * @return length as Timestamp.
	 */
	virtual Timestamp getLength() const = 0;

	virtual bool rewind() {
		return seek(0);
	}
};

/**
 * Converts a point in time to a precise sample offset
 * with the given parameters.
 *
 * @param where    Point in time.
 * @param rate     Rate of the stream.
 * @param isStereo Is the stream a stereo stream?
 */
Timestamp convertTimeToStreamPos(const Timestamp &where, int rate, bool isStereo);

} // End of namespace Audio

#endif
