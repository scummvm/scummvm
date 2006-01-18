/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2006 The ScummVM project
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
 * $Header$
 *
 */

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "common/stdafx.h"
#include "common/util.h"
#include "common/scummsys.h"


/**
 * Generic input stream for the resampling code.
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

	/** Sample rate of the stream. */
	virtual int getRate() const = 0;

	/**
	 * Tries to load a file by trying all available formats.
	 * In case of an error, the file handle will be closed, but deleting
	 * it is still the responsibilty of the caller.
	 * @param filename	a filename without an extension
	 * @return	an Audiostream ready to use in case of success;
	 *			NULL in case of an error (e.g. invalid/nonexisting file)
	 */
	static AudioStream* openStreamFile(const char *filename);
};

/**
 * A simple AudioStream which represents a 'silent' stream,
 * containing the specified number of zero samples.
 */
class ZeroInputStream : public AudioStream {
private:
	int _len;
public:
	ZeroInputStream(uint len) : _len(len) { }
	int readBuffer(int16 *buffer, const int numSamples) {
		int samples = MIN(_len, numSamples);
		memset(buffer, 0, samples * 2);
		_len -= samples;
		return samples;
	}
	bool isStereo() const { return false; }
	bool eos() const { return _len <= 0; }

	int getRate() const { return -1; }
};

AudioStream *makeLinearInputStream(int rate, byte flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen);


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READSAMPLE(is16Bit, isUnsigned, ptr) \
	((is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

#define READ_ENDIAN_SAMPLE(is16Bit, isUnsigned, ptr, isLE) \
	((is16Bit ? (isLE ? READ_LE_UINT16(ptr) : READ_BE_UINT16(ptr)) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))


#endif
