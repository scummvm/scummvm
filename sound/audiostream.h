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
protected:
	virtual int16 readIntern() = 0;
	virtual void advance() = 0;
public:
	int16 read() { assert(size() > 0); int16 val = readIntern(); advance(); return val; }
	virtual int size() const = 0;
	bool eof() const { return size() <= 0; }
	virtual bool isStereo() const = 0;
};

class ZeroInputStream : public AudioInputStream {
protected:
	int _len;
	int16 readIntern() { return 0; }
	void advance() { _len--; }
public:
	ZeroInputStream(uint len) : _len(len) { }
	virtual int size() const { return _len; }
	virtual bool isStereo() const { return false; }
};

AudioInputStream *makeInputStream(byte _flags, const byte *ptr, uint32 len);

#endif
