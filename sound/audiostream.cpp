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
#include "common/file.h"
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
	bool eof() const {
		return _end <= _ptr;
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
	bool eof() const;
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
bool WrappedMemoryStream<stereo, is16Bit, isUnsigned>::eof() const {
	return _end == _pos;
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


#ifdef USE_MAD

#define MP3_BUFFER_SIZE 131072

/**
 * Playback the MP3 data in the given file for the specified duration.
 *
 * @param file		file containing the MP3 data
 * @param duration	playback duration in frames (1/75th of a second), 0 means playback until EOF
 */
MP3InputStream::MP3InputStream(File *file, mad_timer_t duration) {
	// duration == 0 means: play everything till end of file
	
	_isStereo = false;
	_curChannel = 0;
	_file = file;
	_rate = 0;
	_posInFrame = 0;
	
	_duration = duration;

	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
	
	_ptr = (byte *)malloc(MP3_BUFFER_SIZE + MAD_BUFFER_GUARD);
	
	_initialized = init();
}

MP3InputStream::~MP3InputStream() {
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);
	
	free(_ptr);
}

bool MP3InputStream::init() {
	// TODO
	
	// Read in the first chunk of the MP3 file
	_size = _file->read(_ptr, MP3_BUFFER_SIZE);
	if (_size <= 0) {
		warning("MP3InputStream: Failed to read MP3 data");
		return false;
	}
	
	// Feed the data we just read into the stream decoder
	mad_stream_buffer(&_stream, _ptr, _size);

	// Read in initial data
	refill();

	// Check the header, determine if this is a stereo stream
	int num;
	switch(_frame.header.mode)
	{
		case MAD_MODE_SINGLE_CHANNEL:
		case MAD_MODE_DUAL_CHANNEL:
		case MAD_MODE_JOINT_STEREO:
		case MAD_MODE_STEREO:
			num = MAD_NCHANNELS(&_frame.header);
			assert(num == 1 || num == 2);
			_isStereo = (num == 2);
			break;
		default:
			warning("MP3InputStream: Cannot determine number of channels");
			return false;
	}
	
	// Determine the sample rate
	_rate = _frame.header.samplerate;

	return true;
}

void MP3InputStream::refill() {

	// Read the next frame (may have to retry several times, e.g.
	// to skip over ID3 information).
	while (mad_frame_decode(&_frame, &_stream)) {
		if (_stream.error == MAD_ERROR_BUFLEN) {
			int offset;

			// Give up immediately if we are at the EOF already
			if (_size <= 0)
				return;

			if (!_stream.next_frame) {
				offset = 0;
				memset(_ptr, 0, MP3_BUFFER_SIZE + MAD_BUFFER_GUARD);
			} else {
				offset = _stream.bufend - _stream.next_frame;
				memcpy(_ptr, _stream.next_frame, offset);
			}
			// Read in more data from the input file
			_size = _file->read(_ptr + offset, MP3_BUFFER_SIZE - offset);
			
			// Nothing read -> EOF -> bail out
			if (_size <= 0) {
				return;
			}
			_stream.error = (enum mad_error)0;

			// Feed the data we just read into the stream decoder
			mad_stream_buffer(&_stream, _ptr, _size + offset);

		} else if (MAD_RECOVERABLE(_stream.error)) {
			// FIXME: should we do anything here?
			warning("MP3InputStream: Recoverable error...");
		} else {
			error("MP3InputStream: Unrecoverable error");
		}
	}
	
	// Subtract the duration of this frame from the time left to play
	mad_timer_t frame_duration = _frame.header.duration;
	mad_timer_negate(&frame_duration);
	mad_timer_add(&_duration, _frame.header.duration);
	
	// Synthesise the frame into PCM samples and reset the buffer position
	mad_synth_frame(&_synth, &_frame);
	_posInFrame = 0;
}

bool MP3InputStream::eof() const {
	// Time over -> input steam ends
	if (mad_timer_compare(_duration, mad_timer_zero) <= 0)
		return true;
	// Data left in the PCM buffer -> we are not yet done! 
	if (_posInFrame < _synth.pcm.length)
		return false;
	// EOF of the input file, we are done
	if (_size < 0)
		return true;
	// Otherwise, we are still good to go
	return false;
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
		if (_size < 0)	// EOF
			return 0;
	}

	
	int16 sample;
	if (_isStereo) {
		sample = (int16)scale_sample(_synth.pcm.samples[_curChannel][_posInFrame]);
		if (_curChannel == 0) {
			_curChannel = 1;
		} else {
			_posInFrame++;
			_curChannel = 0;
		}
	} else {
		sample = (int16)scale_sample(_synth.pcm.samples[0][_posInFrame]);
		_posInFrame++;
	}
	
	return sample;
}

#endif


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
	_numChannels = ov_info(_ov_file, -1)->channels;

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

bool VorbisInputStream::eof() const {
	if (_eof_flag)
		return true;
	if (_pos < _buffer + ARRAYSIZE(_buffer))
		return false;
	return (_end_pos <= ov_pcm_tell(_ov_file));
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
