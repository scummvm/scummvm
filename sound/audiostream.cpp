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

#include "stdafx.h"
#include "audiostream.h"
#include "mixer.h"
#include "common/engine.h"
#include "common/util.h"


template<bool is16Bit, bool isUnsigned>
static inline int16 readSample(const byte *ptr) {
	uint16 sample = is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8);
	if (isUnsigned)
		sample ^= 0x8000;
	return (int16)sample;
}

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


template<bool stereo, bool is16Bit, bool isUnsigned>
class LinearMemoryStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;

public:
	LinearMemoryStream(const byte *ptr, uint len)
		: _ptr(ptr), _end(ptr+len) {
		if (stereo)	// Stereo requires even sized data
			assert(len % 2 == 0);
	}
	int16 read() {
		assert(_ptr < _end);
		int16 val = readSample<is16Bit, isUnsigned>(_ptr);
		_ptr += (is16Bit ? 2 : 1);
		return val;
	}
	int size() const {
		return (_end - _ptr) / (is16Bit ? 2 : 1);
	}
	bool isStereo() const {
		return stereo;
	}
};


#pragma mark -
#pragma mark --- WrappedMemoryStream ---
#pragma mark -


// Wrapped memory stream, to be used by the ChannelStream class (and possibly others?)
template<bool stereo, bool is16Bit, bool isUnsigned>
class WrappedMemoryStream : public WrappedAudioInputStream {
protected:
	byte *_bufferStart;
	byte *_bufferEnd;
	byte *_pos;
	byte *_end;
	
public:
	WrappedMemoryStream(uint bufferSize);
	~WrappedMemoryStream() { free(_bufferStart); }
	int16 read();
	int size() const;
	bool isStereo() const {
		return stereo;
	}

	void append(const byte *data, uint32 len);
};


template<bool stereo, bool is16Bit, bool isUnsigned>
WrappedMemoryStream<stereo, is16Bit, isUnsigned>::WrappedMemoryStream(uint bufferSize) {
	if (stereo)	// Stereo requires an even sized buffer
		assert(bufferSize % 2 == 0);
	_bufferStart = (byte *)malloc(bufferSize);
	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int16 WrappedMemoryStream<stereo, is16Bit, isUnsigned>::read() {
	assert(_pos != _end);
	int16 val = readSample<is16Bit, isUnsigned>(_pos);
	_pos += (is16Bit ? 2 : 1);

	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);

	return val;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int WrappedMemoryStream<stereo, is16Bit, isUnsigned>::size() const {
	int len = _end - _pos;
	if (len < 0)
		len += (_bufferEnd - _bufferStart);
	return len / (is16Bit ? 2 : 1);
}

template<bool stereo, bool is16Bit, bool isUnsigned>
void WrappedMemoryStream<stereo, is16Bit, isUnsigned>::append(const byte *data, uint32 len) {
	if (_end + len > _bufferEnd) {
		// Wrap-around case
		uint32 size_to_end_of_buffer = _bufferEnd - _end;
		len -= size_to_end_of_buffer;
		if ((_end < _pos) || (_bufferStart + len >= _pos)) {
			debug(2, "WrappedMemoryStream: buffer overflow (A)");
			return;
		}
		memcpy(_end, data, size_to_end_of_buffer);
		memcpy(_bufferStart, data + size_to_end_of_buffer, len);
		_end = _bufferStart + len;
	} else {
		if ((_end < _pos) && (_end + len >= _pos)) {
			debug(2, "WrappedMemoryStream: buffer overflow (B)");
			return;
		}
		memcpy(_end, data, len);
		_end += len;
	}
}


#pragma mark -
#pragma mark --- MP3 (MAD) stream ---
#pragma mark -


/*
#ifdef USE_MAD
class MP3InputStream : public AudioInputStream {
	struct mad_stream _stream;
	struct mad_frame _frame;
	struct mad_synth _synth;
	uint32 _posInFrame;
	int _chan;

	void refill();
public:
	// TODO
	MP3InputStream();
};

MP3InputStream::MP3InputStream() {
	_chan = 0;
}

void MP3InputStream::refill() {
	// TODO
}

static inline int scale_sample(mad_fixed_t sample) {
	// round
	sample += (1L << (MAD_F_FRACBITS - 16));

	// clip
	if (sample > MAD_F_ONE - 1)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	// quantize and scale to not saturate when mixing a lot of channels
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

int16 MP3InputStream::read() {
	if (_posInFrame >= _synth.pcm.length) {
		refill();
	}
	
	int16 sample;
	if (stereo) {
		sample = (int16)scale_sample(_synth.pcm.samples[_chan][_posInFrame];
		if (_chan == 0) {
			_chan = 1;
		} else {
			_posInFrame++;
			_chan = 0;
		}
	} else {
		sample = (int16)scale_sample(_synth.pcm.samples[0][_posInFrame];
		_posInFrame++;
	}
	
	return sample;
}

#endif
*/

#pragma mark -
#pragma mark --- Ogg Vorbis stream ---
#pragma mark -


#ifdef USE_VORBIS

#ifdef CHUNKSIZE
#define VORBIS_TREMOR
#endif


VorbisInputStream::VorbisInputStream(OggVorbis_File *file, int duration) 
	: _ov_file(file) {
	_pos = _buffer + ARRAYSIZE(_buffer);
	_channels = ov_info(_ov_file, -1)->channels;

	if (duration)
		_end_pos = ov_pcm_tell(_ov_file) + duration;
	else
		_end_pos = ov_pcm_total(_ov_file, -1);

	_eof_flag = false;
}

int16 VorbisInputStream::read() {
	if (_pos >= _buffer + ARRAYSIZE(_buffer)) {
		refill();
	}
	return *_pos++;
}

int VorbisInputStream::size() const {
	if (_eof_flag)
		return 0;
	return (_end_pos - ov_pcm_tell(_ov_file)) + (_buffer + ARRAYSIZE(_buffer) - _pos);
}

void VorbisInputStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0) {
		long result = ov_read(_ov_file, read_pos, len_left,
#ifndef VORBIS_TREMOR
#ifdef SCUMM_BIG_ENDIAN
				      1,
#else
				      0,
#endif
				      2,	// 16 bit
				      1,	// signed
#endif
					  NULL);
		if (result == 0) {
			_eof_flag = true;
			memset(read_pos, 0, len_left);
			break;
		} else if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result < 0) {
			debug(1, "Decode error %d in Vorbis file", result);
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			_eof_flag = true;
			memset(read_pos, 0, len_left);
			break;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
}

#endif


#pragma mark -
#pragma mark --- Input stream factories ---
#pragma mark -


template<bool stereo>
static AudioInputStream *makeLinearInputStream(const byte *ptr, uint32 len, bool is16Bit, bool isUnsigned) {
	if (isUnsigned) {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, true>(ptr, len);
		else
			return new LinearMemoryStream<stereo, false, true>(ptr, len);
	} else {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, false>(ptr, len);
		else
			return new LinearMemoryStream<stereo, false, false>(ptr, len);
	}
}


template<bool stereo>
static WrappedAudioInputStream *makeWrappedInputStream(uint32 len, bool is16Bit, bool isUnsigned) {
	if (isUnsigned) {
		if (is16Bit)
			return new WrappedMemoryStream<stereo, true, true>(len);
		else
			return new WrappedMemoryStream<stereo, false, true>(len);
	} else {
		if (is16Bit)
			return new WrappedMemoryStream<stereo, true, false>(len);
		else
			return new WrappedMemoryStream<stereo, false, false>(len);
	}
}


AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len) {
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeLinearInputStream<true>(ptr, len, is16Bit, isUnsigned);
	} else {
		return makeLinearInputStream<false>(ptr, len, is16Bit, isUnsigned);
	}
}

WrappedAudioInputStream *makeWrappedInputStream(byte _flags, uint32 len) {
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeWrappedInputStream<true>(len, is16Bit, isUnsigned);
	} else {
		return makeWrappedInputStream<false>(len, is16Bit, isUnsigned);
	}
}
