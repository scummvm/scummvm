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

#ifndef AUDIO_AUDIOSTREAM_H
#define AUDIO_AUDIOSTREAM_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/types.h"

#include "audio/timestamp.h"

namespace Common {
class SeekableReadStream;
}

namespace Audio {

/**
 * @defgroup audio_audiostream Audio streams
 * @ingroup audio
 *
 * @brief API for managing audio input streams.
 * @{
 */

/**
 * Generic audio input stream. Subclasses of this are used to feed arbitrary
 * sampled audio data into ScummVM's audio mixer.
 */
class AudioStream {
public:
	virtual ~AudioStream() {}

	/**
	 * Fill the given buffer with up to @p numSamples samples.
	 *
	 * Data must be in native endianness, 16 bits per sample, signed. For stereo
	 * stream, the buffer will be filled with interleaved left and right channel
	 * samples, starting with the left sample. Furthermore, the samples in the
	 * left and right are summed up. So if you request 4 samples from a stereo
	 * stream, you will get a total of two left channel and two right channel
	 * samples.
	 *
	 * @return The actual number of samples read, or -1 if a critical error occurred.
	 *
	 * @note You *must* check whether the returned value is less than what you requested.
	 *       This indicates that the stream is fully used up.
	 * 
	 */
	virtual int readBuffer(int16 *buffer, const int numSamples) = 0;

	/** Check whether this is a stereo stream. */
	virtual bool isStereo() const = 0;

	/** Sample rate of the stream. */
	virtual int getRate() const = 0;

	/**
	 * Check whether end of data has been reached.
	 *
	 * If this returns true, it indicates that at this time there is no data
	 * available in the stream. However, there might be more data in the future.
	 * 
	 * This is used by e.g. a rate converter to decide whether to keep on
	 * converting data or to stop.
	 */
	virtual bool endOfData() const = 0;

	/**
	 * Check whether end of stream has been reached.
	 *
	 * If this returns true, it indicates that all data in this stream is used up
	 * and no additional data will appear in it in the future.
	 * 
	 * This is used by the mixer to decide whether a given stream shall be
	 * removed from the list of active streams (and thus be destroyed).
	 * By default, this maps to endOfData().
	 */
	virtual bool endOfStream() const { return endOfData(); }
};

/**
 * A rewindable audio stream.
 *
 * This allows for resetting the AudioStream to its initial state.
 * Note that rewinding itself is not required to be working when the stream
 * is being played by the mixer.
 */
class RewindableAudioStream : public virtual AudioStream {
public:
	/**
	 * Rewind the stream to its start.
	 *
	 * @return True on success, false otherwise.
	 */
	virtual bool rewind() = 0;
};

/**
 * A looping audio stream.
 *
 * This object does nothing besides using a RewindableAudioStream
 * to play a stream in a loop.
 */
class LoopingAudioStream : public AudioStream {
public:
	/**
	 * Create a looping audio stream object.
	 *
	 * On creation of the LoopingAudioStream object, the underlying stream will be rewound.
	 * 
	 * @see makeLoopingAudioStream
	 *
	 * @param stream  The stream to loop.
	 * @param loops   How often to loop (0 = infinite).
	 * @param disposeAfterUse  Destroy the stream after the LoopingAudioStream has finished playback.
	 * @param rewind  If true, rewind the underlying stream.
	 */
	LoopingAudioStream(RewindableAudioStream *stream, uint loops, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES, bool rewind = true);

	int readBuffer(int16 *buffer, const int numSamples);
	bool endOfData() const;
	bool endOfStream() const;

	bool isStereo() const { return _parent->isStereo(); }
	int getRate() const { return _parent->getRate(); }

	/**
	 * Return the number of loops that the stream has played.
	 */
	uint getCompleteIterations() const { return _completeIterations; }
private:
	Common::DisposablePtr<RewindableAudioStream> _parent;

	uint _loops;
	uint _completeIterations;
};

/**
 * Wrapper functionality to efficiently create a stream that might be looped.
 *
 * This function does not return a LoopingAudioStream, because it does
 * not create one when the loop count is "1". This allows to keep the runtime
 * overhead down when the code does not require any functionality that is only offered
 * by LoopingAudioStream.
 *
 * @param stream  The stream to loop (will be automatically destroyed, when the looping is done).
 * @param loops   How often to loop (0 = infinite).
 *
 * @return A new AudioStream that offers the desired functionality.
 */
AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint loops);

/**
 * A seekable audio stream.
 *
 * Subclasses of this class implement an interface for seeking.
 * The seeking itself is not required to be working while the stream
 * is being played by the mixer.
 */
class SeekableAudioStream : public virtual RewindableAudioStream {
public:
	/**
	 * Attempt to load a file by trying all available formats.
	 *
	 * In case of an error, the file handle will be closed, but deleting
	 * it is still the responsibility of the caller.
	 *
	 * @param basename  File name without an extension.
	 *
	 * @return  A SeekableAudioStream ready to use in case of success.
	 *          NULL in case of an error (e.g. invalid/non-existing file).
	 */
	static SeekableAudioStream *openStreamFile(const Common::String &basename);

	/**
	 * Seek to a given offset in the stream.
	 *
	 * @param where  Offset in milliseconds.
	 *
	 * @return True on success, false on failure.
	 */
	bool seek(uint32 where) {
		return seek(Timestamp(where, getRate()));
	}

	/**
	 * Seek to a given offset in the stream.
	 *
	 * @param where  Offset as a timestamp.
	 *
	 * @return True on success, false on failure.
	 */
	virtual bool seek(const Timestamp &where) = 0;

	/**
	 * Return the length of the stream.
	 *
	 * @return Length as a timestamp.
	 */
	virtual Timestamp getLength() const = 0;

	virtual bool rewind() { return seek(0); }
};

/**
 * Wrapper functionality to efficiently create a stream that might be looped
 * in a certain interval.
 *
 * This automatically starts the stream at time "start"!
 *
 * This function does not return a LoopingAudioStream, because it does
 * not create one when the loop count is "1". This allows to keep the runtime
 * overhead down when the code does not require any functionality that is only offered
 * by LoopingAudioStream.
 *
 * @param stream  The stream to loop (will be automatically destroyed when the looping is done).
 * @param start   Start time of the stream interval to be looped.
 * @param end     End of the stream interval to be looped (a zero time means till the end).
 * @param loops   How often to loop (0 = infinite).
 *
 * @return A new AudioStream that offers the desired functionality.
 */
AudioStream *makeLoopingAudioStream(SeekableAudioStream *stream, Timestamp start, Timestamp end, uint loops);

/**
 * A looping audio stream that features looping of a nested part of the
 * stream.
 *
 * @note Currently this implementation stops after the nested loop finishes
 *       playback.
 *
 * @b Important: This can be merged with SubSeekableAudioStream for playback purposes.
 *               To do this, it must be extended to accept a start time.
 */
class SubLoopingAudioStream : public AudioStream {
public:
	/**
	 * Constructor for a SubLoopingAudioStream.
	 *
	 * On creation of the SubLoopingAudioStream object,
	 * the underlying stream will be rewound.
	 *
	 * @param stream          The stream to loop.
	 * @param loops           How often the stream should be looped (0 means infinite).
	 * @param loopStart       Start of the loop (this must be smaller than @p loopEnd).
	 * @param loopEnd         End of the loop (thus must be greater than @p loopStart).
	 * @param disposeAfterUse Whether the stream should be disposed when the
	 *                        SubLoopingAudioStream is destroyed.
	 */
	SubLoopingAudioStream(SeekableAudioStream *stream, uint loops,
	                      const Timestamp loopStart,
	                      const Timestamp loopEnd,
	                      DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	int readBuffer(int16 *buffer, const int numSamples);
	bool endOfData() const;
	bool endOfStream() const;

	bool isStereo() const { return _parent->isStereo(); }
	int getRate() const { return _parent->getRate(); }
private:
	Common::DisposablePtr<SeekableAudioStream> _parent;

	uint _loops;
	Timestamp _pos;
	Timestamp _loopStart, _loopEnd;

	bool _done;
};


/**
 * A SubSeekableAudioStream class that provides access to a SeekableAudioStream
 * just in the range [start, end).
 *
 * The same caveats apply to SubSeekableAudioStream as do to SeekableAudioStream.
 *
 * Manipulating the parent stream directly will break the substream.
 */
class SubSeekableAudioStream : public SeekableAudioStream {
public:
	/**
	 * Create a new SubSeekableAudioStream.
	 *
	 * @param parent          Parent stream object.
	 * @param start           Start time.
	 * @param end             End time.
	 * @param disposeAfterUse Whether the parent stream object should be destroyed on destruction of the SubSeekableAudioStream.
	 */
	SubSeekableAudioStream(SeekableAudioStream *parent, const Timestamp start, const Timestamp end, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return _parent->isStereo(); }

	int getRate() const { return _parent->getRate(); }

	bool endOfData() const { return (_pos >= _length) || _parent->endOfData(); }
	bool endOfStream() const { return (_pos >= _length) || _parent->endOfStream(); }

	bool seek(const Timestamp &where);

	Timestamp getLength() const { return _length; }
private:
	Common::DisposablePtr<SeekableAudioStream> _parent;

	const Timestamp _start;
	const Timestamp _length;
	Timestamp _pos;
};

/**
 * A QueuingAudioStream class that allows for queuing multiple audio streams for playback.
 */

class QueuingAudioStream : public Audio::AudioStream {
public:

	/**
	 * Queue an audio stream for playback.
	 *
	 * This stream plays all queued streams, in the order in which they were queued.
	 * If disposeAfterUse is set to DisposeAfterUse::YES, then the queued stream
	 * is deleted after all data contained in it has been played.
	 */
	virtual void queueAudioStream(Audio::AudioStream *audStream,
	                              DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES) = 0;

	/**
	 * Queue a block of raw audio data for playback.
	 *
	 * This stream plays all queued blocks, in the order in which they were queued.
	 * If disposeAfterUse is set to DisposeAfterUse::YES, then the queued block is
	 * released using free() after all data contained in it has been played.
	 *
	 * @note Make sure to allocate the data block with malloc(), not with new[].
	 *
	 * @param data             Pointer to the audio data block.
	 * @param size             Length of the audio data block.
	 * @param disposeAfterUse  If equal to DisposeAfterUse::YES, the block is released using free() after use.
	 * @param flags            A bit-ORed combination of RawFlags describing the audio data format.
	 */
	void queueBuffer(byte *data, uint32 size, DisposeAfterUse::Flag disposeAfterUse, byte flags);

	/**
	 * Mark this stream as finished.
	 *
	 * This is used to signal that no further data will be queued to the stream.
	 * The stream is only considered as ended after this has been done.
	 */
	virtual void finish() = 0;

	/**
	 * Return the number of streams still queued for playback (including
	 * the currently playing stream).
	 */
	virtual uint32 numQueuedStreams() const = 0;
};

/**
 * Factory function for a QueuingAudioStream.
 */
QueuingAudioStream *makeQueuingAudioStream(int rate, bool stereo);

/**
 * Convert a point in time to a precise sample offset
 * with the given parameters.
 *
 * @param where     Point in time.
 * @param rate      Rate of the stream.
 * @param isStereo  Whether the stream is a stereo stream.
 */
Timestamp convertTimeToStreamPos(const Timestamp &where, int rate, bool isStereo);

/**
 * Factory function for an AudioStream wrapper that cuts off the amount of samples read after a
 * given time length is reached.
 *
 * @param parentStream     The stream to limit.
 * @param length           The time length to limit the stream to.
 * @param disposeAfterUse  Whether the parent stream object should be destroyed on destruction of the returned stream.
 */
AudioStream *makeLimitingAudioStream(AudioStream *parentStream, const Timestamp &length, DisposeAfterUse::Flag disposeAfterUse = DisposeAfterUse::YES);

/**
 * An AudioStream designed to work in terms of packets.
 *
 * It is similar in concept to the QueuingAudioStream, but does not
 * necessarily rely on the data from each queued AudioStream
 * being separate.
 */
class PacketizedAudioStream : public virtual AudioStream {
public:
	virtual ~PacketizedAudioStream() {}

	/**
	 * Queue the next packet to be decoded.
	 */
	virtual void queuePacket(Common::SeekableReadStream *data) = 0;

	/**
	 * Mark this stream as finished. That is, signal that no further data
	 *
	 * This is used to signal that no further data will be queued to the stream.
	 * The stream is only considered as ended after this has been done.
	 */
	virtual void finish() = 0;
};

/**
 * A PacketizedAudioStream that works closer to a QueuingAudioStream.
 *
 * It queues individual packets as a whole AudioStream to an internal
 * QueuingAudioStream. This is used for writing quick wrappers against
 * e.g. RawStream, which can be then made into PacketizedAudioStreams.
 */
class StatelessPacketizedAudioStream : public PacketizedAudioStream {
public:
	/**
	 * Create a StatelessPacketizedAudioStream with the specified sample rate
	 * and for the specified number of channels.
	 */
	StatelessPacketizedAudioStream(uint rate, uint channels) :
		_rate(rate), _channels(channels), _stream(makeQueuingAudioStream(rate, channels == 2)) {}
	virtual ~StatelessPacketizedAudioStream() {}

	// AudioStream API
	bool isStereo() const { return _channels == 2; }
	int getRate() const { return _rate; }
	int readBuffer(int16 *data, const int numSamples) { return _stream->readBuffer(data, numSamples); }
	bool endOfData() const { return _stream->endOfData(); }
	bool endOfStream() const { return _stream->endOfStream(); }

	// PacketizedAudioStream API
	void queuePacket(Common::SeekableReadStream *data) { _stream->queueAudioStream(makeStream(data)); }
	void finish() { _stream->finish(); }

	/**
	 * Return how many channels this stream is using.
	 */
	uint getChannels() const { return _channels; }

protected:
	/**
	 * Create an AudioStream for a given packet.
	 */
	virtual AudioStream *makeStream(Common::SeekableReadStream *data) = 0;

private:
	uint _rate;
	uint _channels;
	Common::ScopedPtr<QueuingAudioStream> _stream;
};

/**
 * Create an AudioStream that plays nothing and immediately returns that
 * endOfStream() has been reached.
 */
AudioStream *makeNullAudioStream();
/** @} */
} // End of namespace Audio

#endif
