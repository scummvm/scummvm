/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2004 The ScummVM project
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

	/**
	 * Read a single (16 bit signed) sample from the stream.
	 */
//	virtual int16 read() = 0;
	
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
	 * This function returns the number of samples that were delivered to
	 * the mixer which is a rough estimate of how moch time of the stream
	 * has been played.
	 * The exact value is not available as it needs information from the
	 * audio device on how many samples have been already played
	 * As our buffer is relatively short the estimate is exact enough
	 * The return -1 is kind of a hack as this function is only required
	 * for the video audio sync in the bs2 cutscenes I am to lazy to
	 * implement it for all subclasses
	 */
	virtual int getSamplesPlayed() const { return -1; }
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
	int16 read() { assert(_len > 0); _len--; return 0; }
	bool isStereo() const { return false; }
	bool eos() const { return _len <= 0; }
	
	int getRate() const { return -1; }
};

AudioStream *makeLinearInputStream(int rate, byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen);
AppendableAudioStream *makeAppendableAudioStream(int rate, byte _flags, uint32 len);

#endif
