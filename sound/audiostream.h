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

#include "scummsys.h"
#include <assert.h>

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
	virtual int16 read() = 0;
	virtual int size() const = 0;
	virtual bool isStereo() const = 0;

	bool eof() const { return size() <= 0; }
};

class ZeroInputStream : public AudioInputStream {
protected:
	int _len;
public:
	ZeroInputStream(uint len) : _len(len) { }
	int16 read() { assert(_len > 0); _len--; return 0; }
	int size() const { return _len; }
	bool isStereo() const { return false; }
};

// Wrapped memory stream, to be used by the ChannelStream class (and possibly others?)
template<bool stereo, bool is16Bit, bool isUnsigned>
class WrappedMemoryStream : public AudioInputStream {
protected:
	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	
public:
	WrappedMemoryStream(const byte *buffer, uint bufferSize);
	int16 read();
	int size() const;

	void append(const byte *data, uint32 len);
};


AudioInputStream *makeInputStream(byte _flags, const byte *ptr, uint32 len);

#endif
