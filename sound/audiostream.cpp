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


// This used to be an inline template function, but
// buggy template function handling in MSVC6 forced
// us to go with the macro approach. So far this is
// the only template function that MSVC6 seemed to
// compile incorrectly. Knock on wood.
#define READSAMPLE(is16Bit, isUnsigned, ptr) \
	((is16Bit ? READ_BE_UINT16(ptr) : (*ptr << 8)) ^ (isUnsigned ? 0x8000 : 0))

#pragma mark -
#pragma mark --- LinearMemoryStream ---
#pragma mark -


template<bool stereo, bool is16Bit, bool isUnsigned>
class LinearMemoryStream : public AudioInputStream {
protected:
	const byte *_ptr;
	const byte *_end;
	const byte *_loopPtr;
	const byte *_loopEnd;

	inline int16 readIntern() {
		//assert(_ptr < _end);
		int16 val = READSAMPLE(is16Bit, isUnsigned, _ptr);
		_ptr += (is16Bit ? 2 : 1);
		if (_loopPtr && _ptr == _end) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
		return val;
	}
	inline bool eosIntern() const	{ return _ptr >= _end; };
public:
	LinearMemoryStream(const byte *ptr, uint len, uint loopOffset, uint loopLen)
		: _ptr(ptr), _end(ptr+len), _loopPtr(0), _loopEnd(0) {

		// Verify the buffer sizes are sane
		if (is16Bit && stereo)
			assert((len & 3) == 0 && (loopLen & 3) == 0);
		else if (is16Bit || stereo)
			assert((len & 1) == 0 && (loopLen & 1) == 0);

		if (loopLen) {
			_loopPtr = _ptr + loopOffset;
			_loopEnd = _loopPtr + loopLen;
		}
		if (stereo)	// Stereo requires even sized data
			assert(len % 2 == 0);
	}
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return stereo; }
};

template<bool stereo, bool is16Bit, bool isUnsigned>
int LinearMemoryStream<stereo, is16Bit, isUnsigned>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_end - _ptr) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READSAMPLE(is16Bit, isUnsigned, _ptr);
			_ptr += (is16Bit ? 2 : 1);
			samples++;
		}
		// Loop, if looping was specified
		if (_loopPtr && eosIntern()) {
			_ptr = _loopPtr;
			_end = _loopEnd;
		}
	}
	return samples;
}


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
	
	inline int16 readIntern();
	inline bool eosIntern() const { return _end == _pos; };
public:
	WrappedMemoryStream(uint bufferSize);
	~WrappedMemoryStream()		{ free(_bufferStart); }
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return stereo; }

	void append(const byte *data, uint32 len);
};


template<bool stereo, bool is16Bit, bool isUnsigned>
WrappedMemoryStream<stereo, is16Bit, isUnsigned>::WrappedMemoryStream(uint bufferSize) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((bufferSize & 3) == 0);
	else if (is16Bit || stereo)
		assert((bufferSize & 1) == 0);

	_bufferStart = (byte *)malloc(bufferSize);
	_pos = _end = _bufferStart;
	_bufferEnd = _bufferStart + bufferSize;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
inline int16 WrappedMemoryStream<stereo, is16Bit, isUnsigned>::readIntern() {
	//assert(_pos != _end);
	int16 val = READSAMPLE(is16Bit, isUnsigned, _pos);
	_pos += (is16Bit ? 2 : 1);

	// Wrap around?
	if (_pos >= _bufferEnd)
		_pos = _pos - (_bufferEnd - _bufferStart);

	return val;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
int WrappedMemoryStream<stereo, is16Bit, isUnsigned>::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const byte *endMarker = (_pos > _end) ? _bufferEnd : _end;
		const int len = MIN(numSamples, samples + (int)(endMarker - _pos) / (is16Bit ? 2 : 1));
		while (samples < len) {
			*buffer++ = READSAMPLE(is16Bit, isUnsigned, _pos);
			_pos += (is16Bit ? 2 : 1);
			samples++;
		}
		// Wrap around?
		if (_pos >= _bufferEnd)
			_pos = _pos - (_bufferEnd - _bufferStart);
	}
	return samples;
}

template<bool stereo, bool is16Bit, bool isUnsigned>
void WrappedMemoryStream<stereo, is16Bit, isUnsigned>::append(const byte *data, uint32 len) {

	// Verify the buffer size is sane
	if (is16Bit && stereo)
		assert((len & 3) == 0);
	else if (is16Bit || stereo)
		assert((len & 1) == 0);

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
class MP3InputStream : public MusicStream {
	struct mad_stream _stream;
	struct mad_frame _frame;
	struct mad_synth _synth;
	mad_timer_t _duration;
	uint32 _posInFrame;
	uint32 _bufferSize;
	int _size;
	bool _isStereo;
	int _curChannel;
	File *_file;
	byte *_ptr;

	bool init();
	void refill(bool first = false);
	inline int16 readIntern();
	inline bool eosIntern() const;
public:
	MP3InputStream(File *file, mad_timer_t duration, uint size = 0);
	~MP3InputStream();
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return _isStereo; }
	
	int getRate() const			{ return _frame.header.samplerate; }
};


/**
 * Playback the MP3 data in the given file for the specified duration.
 *
 * @param file		file containing the MP3 data
 * @param duration	playback duration in frames (1/75th of a second), 0 means
 *					playback until EOF
 * @param size		optional, if non-zero this limits playback based on the
 * 					number of input bytes rather then a duration
 */
MP3InputStream::MP3InputStream(File *file, mad_timer_t duration, uint size) {
	// duration == 0 means: play everything till end of file
	
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);
	
	_duration = duration;

	_posInFrame = 0;
	_bufferSize = size ? size : (128 * 1024);	// Default buffer size is 128K

	_isStereo = false;
	_curChannel = 0;
	_file = file;
	_ptr = (byte *)malloc(_bufferSize + MAD_BUFFER_GUARD);

	init();

	// If a size is specified, we do not perform any further read operations
	if (size) {
		_file = 0;
	}
	
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
	_size = _file->read(_ptr, _bufferSize);
	if (_size <= 0) {
		warning("MP3InputStream: Failed to read MP3 data");
		return false;
	}
	
	// Feed the data we just read into the stream decoder
	mad_stream_buffer(&_stream, _ptr, _size);

	// Read in initial data
	refill(true);

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
	
	return true;
}

void MP3InputStream::refill(bool first) {

	// Read the next frame (may have to retry several times, e.g.
	// to skip over ID3 information).
	while (mad_frame_decode(&_frame, &_stream)) {
		if (_stream.error == MAD_ERROR_BUFLEN) {
			int offset;

			if (!_file)
				_size = -1;
		
			// Give up immediately if we are at the EOF already
			if (_size <= 0)
				return;

			if (!_stream.next_frame) {
				offset = 0;
				memset(_ptr, 0, _bufferSize + MAD_BUFFER_GUARD);
			} else {
				offset = _stream.bufend - _stream.next_frame;
				memcpy(_ptr, _stream.next_frame, offset);
			}
			// Read in more data from the input file
			_size = _file->read(_ptr + offset, _bufferSize - offset);
			
			// Nothing read -> EOF -> bail out
			if (_size <= 0) {
				return;
			}
			_stream.error = (enum mad_error)0;

			// Feed the data we just read into the stream decoder
			mad_stream_buffer(&_stream, _ptr, _size + offset);

		} else if (MAD_RECOVERABLE(_stream.error)) {
			// FIXME: should we do anything here?
			debug(1, "MP3InputStream: Recoverable error...");
		} else {
			error("MP3InputStream: Unrecoverable error");
		}
	}
	
	// Subtract the duration of this frame from the time left to play
	mad_timer_t frame_duration = _frame.header.duration;
	mad_timer_negate(&frame_duration);
	mad_timer_add(&_duration, frame_duration);

	if (!first && _file && mad_timer_compare(_duration, mad_timer_zero) <= 0)
		_size = -1;	// Mark for EOF
	
	// Synthesise the frame into PCM samples and reset the buffer position
	mad_synth_frame(&_synth, &_frame);
	_posInFrame = 0;
}

inline bool MP3InputStream::eosIntern() const {
	return (_size < 0 || _posInFrame >= _synth.pcm.length);
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

inline int16 MP3InputStream::readIntern() {
	assert(!eosIntern());

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

	if (_posInFrame >= _synth.pcm.length) {
		refill();
	}

	return sample;
}

int MP3InputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	assert(_curChannel == 0);	// Paranoia check
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_synth.pcm.length - _posInFrame) * (_isStereo ? 2 : 1));
		while (samples < len) {
			*buffer++ = (int16)scale_sample(_synth.pcm.samples[0][_posInFrame]);
			samples++;
			if (_isStereo) {
				*buffer++ = (int16)scale_sample(_synth.pcm.samples[1][_posInFrame]);
				samples++;
			}
			_posInFrame++;
		}
		if (_posInFrame >= _synth.pcm.length) {
			refill();
		}
	}
	return samples;
}

MusicStream *makeMP3Stream(File *file, mad_timer_t duration, uint size) {
	return new MP3InputStream(file, duration, size);
}

#endif


#pragma mark -
#pragma mark --- Ogg Vorbis stream ---
#pragma mark -


#ifdef USE_VORBIS

class VorbisInputStream : public MusicStream {
	OggVorbis_File *_ov_file;
	int _end_pos;
	int _numChannels;
	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;
	
	void refill();
	inline int16 readIntern();
	inline bool eosIntern() const;
public:
	VorbisInputStream(OggVorbis_File *file, int duration);
	int readBuffer(int16 *buffer, const int numSamples);

	int16 read()				{ return readIntern(); }
	bool eos() const			{ return eosIntern(); }
	bool isStereo() const		{ return _numChannels >= 2; }
	
	int getRate() const			{ return ov_info(_ov_file, -1)->rate; }
};


#ifdef CHUNKSIZE
#define VORBIS_TREMOR
#endif


VorbisInputStream::VorbisInputStream(OggVorbis_File *file, int duration) 
	: _ov_file(file), _bufferEnd(_buffer + ARRAYSIZE(_buffer)) {

	// Check the header, determine if this is a stereo stream
	_numChannels = ov_info(_ov_file, -1)->channels;

	// Determine the end position
	if (duration)
		_end_pos = ov_pcm_tell(_ov_file) + duration;
	else
		_end_pos = ov_pcm_total(_ov_file, -1);
	
	// Read in initial data
	refill();
}

inline int16 VorbisInputStream::readIntern() {
	assert(!eosIntern());

	int16 sample = *_pos++;
	if (_pos >= _bufferEnd) {
		refill();
	}
	return sample;
}

inline bool VorbisInputStream::eosIntern() const {
	return _pos >= _bufferEnd;
}

int VorbisInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && !eosIntern()) {
		const int len = MIN(numSamples, samples + (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
		}
	}
	return samples;
}

void VorbisInputStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0 && _end_pos > ov_pcm_tell(_ov_file)) {
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
		if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result <= 0) {
			if (result < 0)
				debug(1, "Decode error %d in Vorbis file", result);
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			memset(read_pos, 0, len_left);
			break;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
	_bufferEnd = (int16 *)read_pos;
}

MusicStream *makeVorbisStream(OggVorbis_File *file, int duration) {
	return new VorbisInputStream(file, duration);
}

#endif


#pragma mark -
#pragma mark --- Input stream factories ---
#pragma mark -


template<bool stereo>
static AudioInputStream *makeLinearInputStream(const byte *ptr, uint32 len, bool is16Bit, bool isUnsigned, uint loopOffset, uint loopLen) {
	if (isUnsigned) {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, true>(ptr, len, loopOffset, loopLen);
		else
			return new LinearMemoryStream<stereo, false, true>(ptr, len, loopOffset, loopLen);
	} else {
		if (is16Bit)
			return new LinearMemoryStream<stereo, true, false>(ptr, len, loopOffset, loopLen);
		else
			return new LinearMemoryStream<stereo, false, false>(ptr, len, loopOffset, loopLen);
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

AudioInputStream *makeLinearInputStream(byte _flags, const byte *ptr, uint32 len, uint loopOffset, uint loopLen) {
	const bool is16Bit = (_flags & SoundMixer::FLAG_16BITS) != 0;
	const bool isUnsigned = (_flags & SoundMixer::FLAG_UNSIGNED) != 0;
	if (_flags & SoundMixer::FLAG_STEREO) {
		return makeLinearInputStream<true>(ptr, len, is16Bit, isUnsigned, loopOffset, loopLen);
	} else {
		return makeLinearInputStream<false>(ptr, len, is16Bit, isUnsigned, loopOffset, loopLen);
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
