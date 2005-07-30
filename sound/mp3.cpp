/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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

#include "sound/mp3.h"

#ifdef USE_MAD

#include "common/file.h"
#include "common/util.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"

#include <mad.h>


using Common::File;


#pragma mark -
#pragma mark --- MP3 (MAD) stream ---
#pragma mark -


class MP3InputStream : public AudioStream {
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
	inline bool eosIntern() const;
public:
	MP3InputStream(File *file, mad_timer_t duration);
	MP3InputStream(File *file, uint32 size);
	~MP3InputStream();
	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return eosIntern(); }
	bool isStereo() const		{ return _isStereo; }

	int getRate() const			{ return _frame.header.samplerate; }
#ifdef __SYMBIAN32__
	// Used to store the last position stream was read for symbian
	int _lastReadPosition;
#endif
};


/**
 * Play the MP3 data in the given file for the specified duration.
 *
 * @param file		file containing the MP3 data
 * @param duration	playback duration in frames (1/75th of a second), 0 means
 *					playback until EOF
 */
MP3InputStream::MP3InputStream(File *file, mad_timer_t duration) {
	// duration == 0 means: play everything till end of file

	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);

	_duration = duration;

#if defined(__SYMBIAN32__)
	// Symbian can't share filehandles between different threads.
	// So create a new file  and seek that to the other filehandles position
	_file= new (ELeave)File;
	_file->open(file->name());
	_file->seek(file->pos());
#else
	_file = file;
#endif

	_posInFrame = 0;
	_bufferSize = 128 * 1024;	// Default buffer size is 128K

	init();
	_file->incRef();
}

/**
 * Play the MP3 data in the given file, using at most the given number of bytes.
 *
 * @param file		file containing the MP3 data
 * @param size		limits playback based on the number of input bytes, 0 means
 *					playback until EOF
 */
MP3InputStream::MP3InputStream(File *file, uint32 size) {
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);

	_duration = mad_timer_zero;

#if defined(__SYMBIAN32__)
	// Symbian can't share filehandles between different threads.
	// So create a new file  and seek that to the other filehandles position
	_file= new (ELeave)File;
	_file->open(file->name());
	_file->seek(file->pos());
#else
	_file = file;
#endif

	_posInFrame = 0;
	_bufferSize = size ? size : (128 * 1024);	// Default buffer size is 128K

	init();

	// If a size is specified, we do not perform any further read operations
	if (size) {
#ifdef __SYMBIAN32__
		 delete _file;
#endif
		_file = 0;
	} else {
		_file->incRef();
	}
}

MP3InputStream::~MP3InputStream() {
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	free(_ptr);

	if (_file)
		_file->decRef();
#ifdef __SYMBIAN32__
	delete _file;
#endif
}

bool MP3InputStream::init() {
	_isStereo = false;
	_curChannel = 0;
	_ptr = (byte *)malloc(_bufferSize + MAD_BUFFER_GUARD);

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
	switch(_frame.header.mode) {
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
	// must reopen file at current position
#ifdef __SYMBIAN32__
	// For symbian we must check that an alternative file pointer is created, see if its open
	// If not re-open file and seek to the last read position
	if(_file && !_file->isOpen()){
		_file->open(_file->name());
		_file->seek(_lastReadPosition);
	}
#endif

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
			debug(6, "MP3InputStream: Recoverable error...");
		} else {
			error("MP3InputStream: Unrecoverable error");
		}
	}

	// If a time limit was specified (i.e. if _duration is non-zero),
	// check the play back time to determine whether we have to stop now.
	if (_file && mad_timer_compare(_duration, mad_timer_zero) > 0) {
		// Subtract the duration of this frame from the time left to play
		mad_timer_t frame_duration = _frame.header.duration;
		mad_timer_negate(&frame_duration);
		mad_timer_add(&_duration, frame_duration);

		if (!first && mad_timer_compare(_duration, mad_timer_zero) <= 0)
			_size = -1;	// Mark for EOF
	}

	// Synthesise the frame into PCM samples and reset the buffer position
	mad_synth_frame(&_synth, &_frame);
	_posInFrame = 0;

#ifdef __SYMBIAN32__
	// For symbian we now store the last read position and then close the file
	if(_file){
		_lastReadPosition=_file->pos();
		_file->close();
		}
#endif
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

AudioStream *makeMP3Stream(File *file, uint32 size) {
	return new MP3InputStream(file, size);
}


#pragma mark -
#pragma mark --- MP3 Audio CD emulation ---
#pragma mark -


class MP3TrackInfo : public DigitalTrackInfo {
private:
	struct mad_header _mad_header;
	long _size;
	File *_file;
	bool _error_flag;

public:
	MP3TrackInfo(File *file);
	~MP3TrackInfo();
	bool error() { return _error_flag; }
	void play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration);
};


MP3TrackInfo::MP3TrackInfo(File *file) {
#ifdef __SYMBIAN32__
	// This data is taking a rather big room on symbians limited stack
	// Create the buffers on the heap instead and let the stream and frame be references instead
	struct mad_stream* streamptr = (struct mad_stream*)malloc(sizeof(struct mad_stream));
	struct mad_frame* frameptr = (struct mad_frame*)malloc(sizeof(struct mad_frame));
	unsigned char* bufferptr = (unsigned char*)malloc(8192);

	struct mad_stream& stream = *streamptr;
	struct mad_frame& frame = *frameptr;
	unsigned char* buffer = bufferptr;
	uint sizeofbuffer = 8192;
#else
	struct mad_stream stream;
	struct mad_frame frame;
	unsigned char buffer[8192];
	uint sizeofbuffer = sizeof(buffer);
#endif
	unsigned int buflen = 0;
	int count = 0;

	// Check the format and bitrate
	mad_stream_init(&stream);
	mad_frame_init(&frame);

	while (1) {
		if (buflen < sizeofbuffer) {
			int bytes;

			bytes = file->read(buffer + buflen, sizeofbuffer - buflen);
			if (bytes <= 0) {
				if (bytes == -1) {
					warning("Invalid file format");
					goto error;
				}
				break;
			}

			buflen += bytes;
		}

		mad_stream_buffer(&stream, buffer, buflen);

		while (1) {
			if (mad_frame_decode(&frame, &stream) == -1) {
				if (!MAD_RECOVERABLE(stream.error))
					break;

				if (stream.error != MAD_ERROR_BADCRC)
					continue;
			}

			if (count++)
				break;
		}

		if (count || stream.error != MAD_ERROR_BUFLEN)
			break;

		memmove(buffer, stream.next_frame,
		        buflen = &buffer[buflen] - stream.next_frame);
	}

	if (count)
		memcpy(&_mad_header, &frame.header, sizeof(mad_header));
	else {
		warning("Invalid file format");
		goto error;
	}

	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	// Get file size
	_size = file->size();
	_file = file;
	_error_flag = false;
#ifdef __SYMBIAN32__
	// Free the heap reservations
	free(frameptr);
	free(streamptr);
	free(bufferptr);
#endif
	return;

error:
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
	_error_flag = true;
	delete file;
#ifdef __SYMBIAN32__
	// Free the heap reservations
	free(frameptr);
	free(streamptr);
	free(bufferptr);
#endif
}

void MP3TrackInfo::play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration) {
	unsigned int offset;
	mad_timer_t durationTime;

	// Calc offset. As all bitrates are in kilobit per seconds, the division by 200 is always exact
	offset = (startFrame * (_mad_header.bitrate / (8 * 25))) / 3;
#ifdef __SYMBIAN32__
	// Reopen the file if it is not open yet
	if(!_file->isOpen())
		_file->open(_file->name());
#endif
	_file->seek(offset, SEEK_SET);

	// Calc delay
	if (!duration) {
		// FIXME: Using _size here is a problem if offset (or equivalently
		// startFrame) is non-zero.
		mad_timer_set(&durationTime, (_size * 8) / _mad_header.bitrate,
					(_size * 8) % _mad_header.bitrate, _mad_header.bitrate);
	} else {
		mad_timer_set(&durationTime, duration / 75, duration % 75, 75);
	}

	// Play it
	AudioStream *input = new MP3InputStream(_file, durationTime);
	mixer->playInputStream(Audio::Mixer::kMusicSoundType, handle, input);
}

MP3TrackInfo::~MP3TrackInfo() {
	if (! _error_flag)
		_file->close();
}

DigitalTrackInfo *getMP3Track(int track) {
	char track_name[32];
	File *file = new File();

	sprintf(track_name, "track%d.mp3", track);
	file->open(track_name);

	if (file->isOpen()) {
		MP3TrackInfo *trackInfo = new MP3TrackInfo(file);
		if (!trackInfo->error())
			return trackInfo;
		delete trackInfo;
	}
	delete file;
	return NULL;
}


#endif
