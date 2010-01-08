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

#ifndef SOUND_AUDIOSTREAM_H
#define SOUND_AUDIOSTREAM_H

#include "common/util.h"
#include "common/scummsys.h"
#include "common/stream.h"

#include "sound/timestamp.h"

namespace Audio {

class SeekableAudioStream;

/**
 * Generic audio input stream. Subclasses of this are used to feed arbitrary
 * sampled audio data into ScummVM's audio mixer.
 */
class AudioStream {
public:
	virtual ~AudioStream() {}

	/**
	 * Fill the given buffer with up to numSamples samples. Returns the actual
	 * number of samples read, or -1 if a critical error occured (note: you
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
	virtual int readBuffer(int16 *buffer, const int numSamples) = 0;

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
	virtual bool endOfStream() const { return endOfData(); }
};

/**
 * A rewindable audio stream. This allows for restting the AudioStream
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
 * A looping audio stream. This object does nothing beides using
 * a RewindableAudioStream to play a stream in a loop.
 */
class LoopingAudioStream : public AudioStream {
public:
	/**
	 * Creates a looping audio stream object.
	 *
	 * @see makeLoopingAudioStream
	 *
	 * @param stream Stream to loop
	 * @param loops How often to loop (0 = infinite)
	 * @param disposeAfteruse Destroy the stream after the LoopingAudioStream has finished playback.
	 */
	LoopingAudioStream(RewindableAudioStream *stream, uint loops, bool disposeAfterUse = true);
	~LoopingAudioStream();

	int readBuffer(int16 *buffer, const int numSamples);
	bool endOfData() const;

	bool isStereo() const { return _parent->isStereo(); }
	int getRate() const { return _parent->getRate(); }

	/** 
	 * Returns number of loops the stream has played.
	 * @param numLoops number of loops to play, 0 - infinite
	 */
	uint getCompleteIterations() const { return _completeIterations; }
private:
	RewindableAudioStream *_parent;
	bool _disposeAfterUse;

	uint _loops;
	uint _completeIterations;
};

/**
 * Wrapper functionallity to efficiently create a stream, which might be looped.
 *
 * Note that this function does not return a LoopingAudioStream, because it does
 * not create one, when the loop count is "1". This allows to keep the runtime
 * overhead down, when the code does not require any functionallity only offered
 * by LoopingAudioStream.
 *
 * @param stream Stream to loop (will be automatically destroyed, when the looping is done)
 * @param loops How often to loop (0 = infinite)
 * @return A new AudioStream, which offers the desired functionallity.
 */
AudioStream *makeLoopingAudioStream(RewindableAudioStream *stream, uint loops);

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
	 * it is still the responsibilty of the caller.
	 * @param basename	a filename without an extension
	 * @return	an SeekableAudioStream ready to use in case of success;
	 *			NULL in case of an error (e.g. invalid/nonexisting file)
	 */
	static SeekableAudioStream *openStreamFile(const Common::String &basename);

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

	virtual bool rewind() { return seek(0); }
};

/**
 * Wrapper functionallity to efficiently create a stream, which might be looped
 * in a certain interval.
 *
 * This automatically starts the stream at time "start"!
 *
 * Note that this function does not return a LoopingAudioStream, because it does
 * not create one, when the loop count is "1". This allows to keep the runtime
 * overhead down, when the code does not require any functionallity only offered
 * by LoopingAudioStream.
 *
 * @param stream Stream to loop (will be automatically destroyed, when the looping is done)
 * @param start Starttime of the stream interval to be looped
 * @param end End of the stream interval to be looped (a zero time, means till end)
 * @param loops How often to loop (0 = infinite)
 * @return A new AudioStream, which offers the desired functionallity.
 */
AudioStream *makeLoopingAudioStream(SeekableAudioStream *stream, Timestamp start, Timestamp end, uint loops);

/**
 * A SubSeekableAudioStream provides access to a SeekableAudioStream
 * just in the range [start, end).
 * The same caveats apply to SubSeekableAudioStream as do to SeekableAudioStream.
 * 
 * Manipulating the parent stream directly /will/ mess up a substream.
 *
 * IMPORTANT:
 * Note for engine authors. This object is currently under inspection. In case
 * we need to revise the looping API we might drop this. So if you really need
 * something like this object, please drop a mail to LordHoto.
 */
class SubSeekableAudioStream : public SeekableAudioStream {
public:
	/**
	 * Creates a new SubSeekableAudioStream.
	 *
	 * @param parent parent stream object.
	 * @param start Start time.
	 * @param end End time.
	 * @param disposeAfterUse Whether the parent stream object should be destroied on desctruction of the SubSeekableAudioStream.
	 */
	SubSeekableAudioStream(SeekableAudioStream *parent, const Timestamp start, const Timestamp end, bool disposeAfterUse = true);
	~SubSeekableAudioStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return _parent->isStereo(); }

	int getRate() const { return _parent->getRate(); }

	bool endOfData() const { return (_pos >= _length) || _parent->endOfStream(); }

	bool seek(const Timestamp &where);

	Timestamp getLength() const { return _length; }
private:
	SeekableAudioStream *_parent;
	bool _disposeAfterUse;

	const Timestamp _start;
	Timestamp _pos, _length;
};

/**
 * Factory function for a raw linear AudioStream, which will simply treat all
 * data in the buffer described by ptr and len as raw sample data in the
 * specified format. It will then simply pass this data directly to the mixer,
 * after converting it to the sample format used by the mixer (i.e. 16 bit
 * signed native endian). Optionally supports (infinite) looping of a portion
 * of the data.
 */
SeekableAudioStream *makeLinearInputStream(const byte *ptr, uint32 len, int rate,
		byte flags, uint loopStart, uint loopEnd);


/**
 * Struct used to define the audio data to be played by a LinearDiskStream.
 */
struct LinearDiskStreamAudioBlock {
	int32 pos;		///< Position in stream of the block
	int32 len;		///< Length of the block (in samples)
};


/**
 * Factory function for a Linear Disk Stream.  This can stream linear (PCM)
 * audio from disk. The function takes an pointer to an array of
 * LinearDiskStreamAudioBlock which defines the start position and length of
 * each block of uncompressed audio in the stream.
 */
SeekableAudioStream *makeLinearDiskStream(Common::SeekableReadStream *stream, LinearDiskStreamAudioBlock *block,
		int numBlocks, int rate, byte flags, bool disposeStream, uint loopStart, uint loopEnd);

/**
 * An audio stream to which additional data can be appended on-the-fly.
 * Used by SMUSH, iMuseDigital, the Kyrandia 3 VQA player, etc.
 */
class AppendableAudioStream : public Audio::AudioStream {
public:

	/**
	 * Queue another audio data buffer for playback. The stream
	 * will playback all queued buffers, in the order they were
	 * queued. After all data contained in them has been played,
	 * the buffer will be delete[]'d (so make sure to allocate them
	 * with new[], not with malloc).
	 */
	virtual void queueBuffer(byte *data, uint32 size) = 0;

	/**
	 * Mark the stream as finished, that is, signal that no further data
	 * will be appended to it. Only after this has been done can the
	 * AppendableAudioStream ever 'end'
	 */
	virtual void finish() = 0;
};

/**
 * Factory function for an AppendableAudioStream. The rate and flags
 * parameters are analog to those used in makeLinearInputStream.
 */
AppendableAudioStream *makeAppendableAudioStream(int rate, byte flags);

/**
 * Calculates the sample, which the timestamp describes in a
 * AudioStream with the given framerate.
 *
 * @param where point in time
 * @param rate rate of the AudioStream
 * @return sample index
 */
uint32 calculateSampleOffset(const Timestamp &where, int rate);

} // End of namespace Audio

#endif
