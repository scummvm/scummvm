/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "sound/mp3.h"

#ifdef USE_MAD

#include "common/file.h"
#include "common/util.h"

#include "sound/audiocd.h"
#include "sound/audiostream.h"

#include <mad.h>


namespace Audio {


#pragma mark -
#pragma mark --- MP3 (MAD) stream ---
#pragma mark -


class MP3InputStream : public AudioStream {
protected:
	mad_stream _stream;
	mad_frame _frame;
	mad_synth _synth;

	mad_timer_t _startTime;	
	mad_timer_t _endTime;
	mad_timer_t _totalTime;
	
	// TODO: For looping, we will have to use a SeekableReadStream here
	Common::ReadStream *_inStream;
	bool _disposeAfterUse;
	
	enum {
		BUFFER_SIZE = 5 * 8192
	};
	
	// This buffer contains a slab of input data
	byte _buf[BUFFER_SIZE + MAD_BUFFER_GUARD];


	uint32 _posInFrame;
	int _curChannel;
	
	bool _eos;
	
public:
	MP3InputStream(Common::ReadStream *inStream,
	               bool dispose,
	               mad_timer_t start = mad_timer_zero,
	               mad_timer_t end = mad_timer_zero);
	~MP3InputStream();
	
	bool init();
	
	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _eos; }
	bool isStereo() const		{ return MAD_NCHANNELS(&_frame.header) == 2; }
	int getRate() const			{ return _frame.header.samplerate; }

protected:
	void decodeMP3Data();
	bool readMP3Data();
};

MP3InputStream::MP3InputStream(Common::ReadStream *inStream, bool dispose, mad_timer_t start, mad_timer_t end) :
	_inStream(inStream),
	_disposeAfterUse(dispose),
	_startTime(start),
	_endTime(end),
	_totalTime(mad_timer_zero),
	_eos(false) {

	// Make sure that either start < end, or end is zero (indicating "play until end")
	assert(mad_timer_compare(_startTime, _endTime) < 0 || mad_timer_sign(_endTime) == 0);

	_posInFrame = 0;
	_curChannel = 0;

	// The MAD_BUFFER_GUARD must always contain zeros (the reason
	// for this is that the Layer III Huffman decoder of libMAD
	// may read a few bytes beyond the end of the input buffer).
	memset(_buf + BUFFER_SIZE, 0, MAD_BUFFER_GUARD);

	// Init MAD
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);

	// Decode the first chunk of data.
	decodeMP3Data();
}

MP3InputStream::~MP3InputStream() {
	// Deinit MAD
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	if (_disposeAfterUse)
		delete _inStream;
}

void MP3InputStream::decodeMP3Data() {
	if (_eos)
		return;

	do {

		// If necessary, load more data
		if (_stream.buffer == NULL || _stream.error == MAD_ERROR_BUFLEN) {
			if (!readMP3Data()) {
				// We tried to read more data but failed -> end of stream reached
				_eos = true;
				break;
			}
		}

		while (true) {
			_stream.error = MAD_ERROR_NONE;

			// Decode the next header. Note: mad_frame_decode would do this for us, too.
			// However, for seeking we don't want to decode the full frame (else it would
			// be far too slow). Hence we perform this explicitly in a separate step.
			if (mad_header_decode(&_frame.header, &_stream) == -1) {
				if (_stream.error == MAD_ERROR_BUFLEN) {
					break; // Read more data
				} else if (MAD_RECOVERABLE(_stream.error)) {
					debug(1, "MP3InputStream: Recoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
					continue;
				} else {
					warning("MP3InputStream: Unrecoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
					break;
				}
			}
		
			// Sum up the total playback time so far
			mad_timer_add(&_totalTime, _frame.header.duration);
			
			// If we have not yet reached the start point, skip to the next frame
			if (mad_timer_compare(_totalTime, _startTime) < 0)
				continue;

			// If an end time is specified and we are past it, stop
			if (mad_timer_sign(_endTime) > 0 && mad_timer_compare(_totalTime, _endTime) > 0) {
				_eos = true;
				break;
			}
			
			// Decode the next frame
			if (mad_frame_decode(&_frame, &_stream) == -1) {
				if (_stream.error == MAD_ERROR_BUFLEN) {
					break; // Read more data
				} else if (MAD_RECOVERABLE(_stream.error)) {
					// FIXME: should we do anything here?
					debug(1, "MP3InputStream: Recoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					continue;
				} else {
					warning("MP3InputStream: Unrecoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					break;
				}
			}
			
			// Synthesize PCM data
			mad_synth_frame(&_synth, &_frame);
			_posInFrame = 0;
			break;
		}

	} while (_stream.error == MAD_ERROR_BUFLEN);
	
	if (_stream.error != MAD_ERROR_NONE)
		_eos = true;
}

bool MP3InputStream::readMP3Data() {
	uint32 remaining = 0;

	// Give up immediately if we already used up all data in the stream
	if (_inStream->eos())
		return false;

	if (_stream.next_frame) {
		// If there is still data in the MAD stream, we need to preserve it.
		// Note that we use memmove, as we are reusing the same buffer,
		// and hence the data regions we copy from and to may overlap.
		remaining = _stream.bufend - _stream.next_frame;
		assert(remaining < BUFFER_SIZE);	// Paranoia check
		memmove(_buf, _stream.next_frame, remaining);
	}

	// Try to read the next block
	uint32 size = _inStream->read(_buf + remaining, BUFFER_SIZE - remaining);
	if (size <= 0) {
		return false;
	}
	
	// Feed the data we just read into the stream decoder
	_stream.error = MAD_ERROR_NONE;
	mad_stream_buffer(&_stream, _buf, size + remaining);

	return true;
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
	// Keep going as long as we have input available
	while (samples < numSamples && !_eos) {
		const int len = MIN(numSamples, samples + (int)(_synth.pcm.length - _posInFrame) * MAD_NCHANNELS(&_frame.header));
		while (samples < len) {
			*buffer++ = (int16)scale_sample(_synth.pcm.samples[0][_posInFrame]);
			samples++;
			if (MAD_NCHANNELS(&_frame.header) == 2) {
				*buffer++ = (int16)scale_sample(_synth.pcm.samples[1][_posInFrame]);
				samples++;
			}
			_posInFrame++;
		}
		if (_posInFrame >= _synth.pcm.length) {
			// We used up all PCM data in the current frame -- read & decode more
			decodeMP3Data();
		}
	}
	return samples;
}


AudioStream *makeMP3Stream(Common::File *file, uint32 size) {
	assert(file);

	// FIXME: For now, just read the whole data into memory, and be done
	// with it. Of course this is in general *not* a nice thing to do...

	// If no size was specified, read the whole remainder of the file
	if (!size)
		size = file->size() - file->pos();

	// Read 'size' bytes of data (or until EOF is reached) into a MemoryReadStream
	Common::MemoryReadStream *stream = file->readStream(size);

	// .. and create a MP3InputStream from all this
	return new MP3InputStream(stream, true);
}


#pragma mark -
#pragma mark --- MP3 Audio CD emulation ---
#pragma mark -


class MP3TrackInfo : public DigitalTrackInfo {
private:
	Common::String _filename;
	bool _errorFlag;

public:
	MP3TrackInfo(const char *filename);
	bool error() { return _errorFlag; }
	void play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration);
};


MP3TrackInfo::MP3TrackInfo(const char *filename) :
	_filename(filename),
	_errorFlag(false) {

	Common::File file;
	
	// Try to open the file
	if (!file.open(_filename)) {
		_errorFlag = true;
		return;
	}
	
	// Next, try to create a MP3InputStream from it
	
	MP3InputStream *mp3Stream = new MP3InputStream(&file, false);
	
	// If we see EOS here then that means that not (enough) valid input
	// data was given.
	_errorFlag = mp3Stream->endOfData();
	
	// Clean up again	
	delete mp3Stream;
}

void MP3TrackInfo::play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration) {
	mad_timer_t start;
	mad_timer_t end;
	
	// Both startFrame and duration are given in frames, where 75 frames are one second.
	// Calculate the appropriate mad_timer_t values from them.
	mad_timer_set(&start, startFrame / 75, startFrame % 75, 75);
	if (duration == 0) {
		end = mad_timer_zero;
	} else {
		int endFrame = startFrame + duration;
		mad_timer_set(&end, endFrame / 75, endFrame % 75, 75);
	}

	// Open the file
	Common::File *file = new Common::File();
	if (!file || !file->open(_filename)) {
		warning("MP3TrackInfo::play: failed to open '%s'", _filename.c_str());
		return;
	}

	// Play it
	MP3InputStream *input = new MP3InputStream(file, true, start, end);
	
	mixer->playInputStream(Audio::Mixer::kMusicSoundType, handle, input);
}

DigitalTrackInfo *getMP3Track(int track) {
	char trackName[2][32];

	sprintf(trackName[0], "track%d.mp3", track);
	sprintf(trackName[1], "track%02d.mp3", track);

	for (int i = 0; i < 2; ++i) {
		if (Common::File::exists(trackName[i])) {
			MP3TrackInfo *trackInfo = new MP3TrackInfo(trackName[i]);
			if (!trackInfo->error())
				return trackInfo;
			delete trackInfo;
		}
	}
	return NULL;
}


// Closing note: we added File::incRef and File::decRef mainly for the sake of the input streams
// If we could but get rid of it...


} // End of namespace Audio

#endif // #ifdef USE_MAD
