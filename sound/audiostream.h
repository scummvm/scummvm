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

namespace Audio {

/**
 * Generic audio input stream. Subclasses of this are used to feed arbitrary
 * sampled audio data into ScummVM's audio mixer.
 */
class AudioStream {
public:
	virtual ~AudioStream() {}

	/**
	 * Fill the given buffer with up to numSamples samples.
	 * Returns the actual number of samples read, or -1 if
	 * a critical error occured (note: you *must* check if
	 * this value is less than what you requested, this can
	 * happen when the stream is fully used up).
	 *
	 * Data has to be in native endianess, 16 bit per sample, signed.
	 * For stereo stream, buffer will be filled with interleaved
	 * left and right channel samples, starting with a left sample.
	 * Furthermore, the samples in the left and right are summed up.
	 * So if you request 4 samples from a stereo stream, you will get
	 * a total of two left channel and two right channel samples.
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

	/**
	 * Tries to load a file by trying all available formats.
	 * In case of an error, the file handle will be closed, but deleting
	 * it is still the responsibilty of the caller.
	 * @param basename	a filename without an extension
	 * @param startTime	the (optional) time offset in milliseconds from which to start playback
	 * @param duration	the (optional) time in milliseconds specifying how long to play
	 * @param numLoops	how often the data shall be looped (0 = infinite)
	 * @return	an Audiostream ready to use in case of success;
	 *			NULL in case of an error (e.g. invalid/nonexisting file)
	 */
	static AudioStream* openStreamFile(const Common::String &basename, uint32 startTime = 0, uint32 duration = 0, uint numLoops = 1);

	enum {
		kUnknownPlayTime = -1
	};

	/**
	 * Returns total playtime of the AudioStream object.
	 * Note that this does not require to return an playtime, if the
	 * playtime of the AudioStream is unknown it returns 'kUnknownPlayTime'.
	 * @see kUnknownPlayTime
	 *
	 * @return	playtime in milliseconds
	 */
	virtual int32 getTotalPlayTime() const { return kUnknownPlayTime; }
};


/**
 * Factory function for a raw linear AudioStream, which will simply treat all data
 * in the buffer described by ptr and len as raw sample data in the specified
 * format. It will then simply pass this data directly to the mixer, after converting
 * it to the sample format used by the mixer (i.e. 16 bit signed native endian).
 * Optionally supports (infinite) looping of a portion of the data.
 */
AudioStream *makeLinearInputStream(const byte *ptr, uint32 len, int rate, byte flags, uint loopStart, uint loopEnd);


/** Struct used to define the audio data to be played by a LinearDiskStream */

struct LinearDiskStreamAudioBlock {
	int32 pos;		///< Position in stream of the block
	int32 len;		///< Length of the block (in samples)
};


/** Factory function for a Linear Disk Stream.  This can stream linear (PCM) audio from disk.  The
 *  function takes an pointer to an array of LinearDiskStreamAudioBlock which defines the
 *  start position and length of each block of uncompressed audio in the stream.
 */

AudioStream *makeLinearDiskStream(Common::SeekableReadStream *stream, LinearDiskStreamAudioBlock *block, int 
		numBlocks, int rate, byte flags, bool disposeStream, uint loopStart, uint loopEnd);

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


} // End of namespace Audio

#endif
