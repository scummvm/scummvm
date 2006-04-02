/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#ifndef AUDIOSTREAM_H
#define AUDIOSTREAM_H

#include "stdafx.h"
#include "bits.h"


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
	 * For stereo stream, buffer will be filled with interleaved
	 * left and right channel samples.
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
};

class AppendableAudioStream : public AudioStream {
public:
	virtual void append(const byte *data, uint32 len) = 0;
	virtual void finish() = 0;
};

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

AudioStream *makeLinearInputStream(int rate, byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen);
AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len);

#endif
