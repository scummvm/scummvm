/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "stdafx.h"
#include "common/scummsys.h"
#include "common/util.h"


/**
 * Generic input stream for the resampling code.
 */
class AudioInputStream {
public:
	virtual ~AudioInputStream() {}

	/**
	 * Fill the given buffer with up to numSamples samples.
	 * Returns the actual number of samples read, or -1 if
	 * a critical error occured (note: you *must* check if
	 * this value is less than what you requested, this can
	 * happen when the stream is fully used up).
	 * For stereo stream, buffer will be filled with interleaved
	 * left and right channel samples.
	 */
	virtual int readBuffer(int16 *buffer, const int numSamples) = 0;

	/**
	 * Read a single (16 bit signed) sample from the stream.
	 */
	virtual int16 read() = 0;
	
	/** Is this a stereo stream? */
	virtual bool isStereo() const = 0;
	
	/** End of stream reached? */
	virtual bool eos() const = 0;

	/** Sample rate of the stream. */
	virtual int getRate() const = 0;
};

class WrappedAudioInputStream : public AudioInputStream {
public:
	virtual void append(const byte *data, uint32 len) = 0;
	virtual void finish() = 0;
};

class ZeroInputStream : public AudioInputStream {
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
	int16 read() { assert(_len > 0); _len--; return 0; }
	bool isStereo() const { return false; }
	bool eos() const { return _len <= 0; }
	
	int getRate() const { return -1; }
};

AudioInputStream *makeLinearInputStream(int rate, byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen);
WrappedAudioInputStream *makeWrappedInputStream(int rate, byte _flags, uint32 len);

#endif
