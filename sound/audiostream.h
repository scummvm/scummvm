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

	virtual int16 read() = 0;
	//virtual int size() const = 0;
	virtual bool isStereo() const = 0;
	virtual bool eof() const = 0;
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
	int16 read() { assert(_len > 0); _len--; return 0; }
	int size() const { return _len; }
	bool isStereo() const { return false; }
	bool eof() const { return _len <= 0; }
};

#ifdef USE_MAD
class MP3InputStream : public AudioInputStream {
	struct mad_stream _stream;
	struct mad_frame _frame;
	struct mad_synth _synth;
	uint32 _posInFrame;
	int _size;
	bool _isStereo;
	int _curChannel;
	File *_file;
	byte *_ptr;
	int _rate;
	bool _initialized;
	mad_timer_t _duration;

	bool init();
	void refill();
public:
	MP3InputStream(File *file, mad_timer_t duration);
	~MP3InputStream();
	int16 read();
	bool eof() const;
	bool isStereo() const { return _isStereo; }
	
	int getRate() const { return _rate; }
};
#endif


#ifdef USE_VORBIS
class VorbisInputStream : public AudioInputStream {
	OggVorbis_File *_ov_file;
	int _end_pos;
	bool _eof_flag;
	int _numChannels;
	int16 _buffer[4096];
	int16 *_pos;
	
	void refill();
public:
	VorbisInputStream(OggVorbis_File *file, int duration);
	int16 read();
	bool eof() const;
	bool isStereo() const { return _numChannels >= 2; }
};
#endif



AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len);
WrappedAudioInputStream *makeWrappedInputStream(byte _flags, uint32 len);


#endif
