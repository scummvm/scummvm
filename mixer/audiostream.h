// Residual - Virtual machine to run LucasArts' 3D adventure games
// Copyright (C) 2003 The ScummVM-Residual Team (www.scummvm.org)
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "../stdafx.h"
#include "../bits.h"

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
	 *
	 * For maximum efficency, subclasses should always override
	 * the default implementation!
	 */
	virtual int readBuffer(int16 *buffer, const int numSamples) {
		int samples;
		for (samples = 0; samples < numSamples && !eos(); samples++) {
			*buffer++ = read();
		}
		return samples;
	}

	/** Read a singel (16 bit signed) sample from the stream. */
	virtual int16 read() = 0;
	
	/** Is this a stereo stream? */
	virtual bool isStereo() const = 0;
	
	/* End of stream reached? */
	virtual bool eos() const = 0;

	virtual int getRate() const { return -1; }
};

class WrappedAudioInputStream : public AudioInputStream {
public:
	virtual void append(const byte *data, uint32 len) = 0;
};

class ZeroInputStream : public AudioInputStream {
protected:
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
	int size() const { return _len; }
	bool isStereo() const { return false; }
	bool eos() const { return _len <= 0; }
};

class MusicStream : public AudioInputStream {
public:
	virtual int getRate() const = 0;
};


AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen);
WrappedAudioInputStream *makeWrappedInputStream(byte _flags, uint32 len);

#endif
