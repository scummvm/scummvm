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
#ifdef USE_MAD
#include <mad.h>
#endif
#ifdef USE_VORBIS
#include <vorbis/vorbisfile.h>
#endif

class File;


// TODO:
// * maybe make readIntern return 16.16 or 24.8 fixed point values
//   since MAD (and maybe OggVorbis?) gives us those -> higher quality.
//   The rate converters should be able to deal with those just fine, too.
// * possibly add MADInputStream and VorbisInputStream

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
	 * happend when the stream is fully used up).
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
	int readBuffer(int16 *buffer, int numSamples) {
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

#ifdef USE_MAD
MusicStream *makeMP3Stream(File *file, mad_timer_t duration, uint size = 0);
#endif

#ifdef USE_VORBIS
MusicStream *makeVorbisStream(OggVorbis_File *file, int duration);
#endif


#endif
