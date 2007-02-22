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

#include "sound/vorbis.h"

#ifdef USE_VORBIS

#include "common/file.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"

#ifdef USE_TREMOR
#ifdef __GP32__ // GP32 uses custom libtremor
#include <ivorbisfile.h>
#else
#include <tremor/ivorbisfile.h>
#endif
#else
#include <vorbis/vorbisfile.h>
#endif


using Common::File;


namespace Audio {

// These are wrapper functions to allow using a SeekableReadStream object to
// provide data to the OggVorbis_File object.

static size_t read_stream_wrap(void *ptr, size_t size, size_t nmemb, void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;

	uint32 result = stream->read(ptr, size * nmemb);
	
	return result / size;
}

static int seek_stream_wrap(void *datasource, ogg_int64_t offset, int whence) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	stream->seek(offset, whence);
	return stream->pos();
}

static int close_stream_wrap(void *datasource) {
	// Do nothing -- we leave it up to the VorbisInputStream to free memory as appropriate.
	return 0;
}

static long tell_stream_wrap(void *datasource) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)datasource;
	return stream->pos();
}

static ov_callbacks g_stream_wrap = {
	read_stream_wrap, seek_stream_wrap, close_stream_wrap, tell_stream_wrap
};



#pragma mark -
#pragma mark --- Ogg Vorbis stream ---
#pragma mark -


class VorbisInputStream : public AudioStream {
protected:
	Common::SeekableReadStream *_inStream;
	bool _disposeAfterUse;

	bool _isStereo;
	int _rate;
	uint _numLoops;
	
#ifdef USE_TREMOR
	ogg_int64_t _startTime;
	ogg_int64_t _endTime;
#else
	double _startTime;
	double _endTime;
#endif

	OggVorbis_File _ovFile;

	int16 _buffer[4096];
	const int16 *_bufferEnd;
	const int16 *_pos;
	
public:
	// startTime / duration are in milliseconds
	VorbisInputStream(Common::SeekableReadStream *inStream, bool dispose, uint startTime = 0, uint endTime = 0, uint numLoops = 1);
	~VorbisInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const		{ return _pos >= _bufferEnd; }
	bool isStereo() const		{ return _isStereo; }
	int getRate() const			{ return _rate; }

protected:
	void refill();
};

VorbisInputStream::VorbisInputStream(Common::SeekableReadStream *inStream, bool dispose, uint startTime, uint endTime, uint numLoops) :
	_inStream(inStream),
	_disposeAfterUse(dispose),
	_numLoops(numLoops),
	_bufferEnd(_buffer + ARRAYSIZE(_buffer)) {

	bool err = (ov_open_callbacks(inStream, &_ovFile, NULL, 0, g_stream_wrap) < 0);
	// FIXME: proper error handling!
	assert(!err);

#ifdef USE_TREMOR
	/* TODO: Symbian may have to use scumm_fixdfdi here? To quote:
	 "SumthinWicked says: fixing "relocation truncated to fit: ARM_26 __fixdfdi" during linking on GCC, see portdefs.h"
	*/

	// In Tremor, the ov_time_seek() and ov_time_seek_page() calls take seeking
	// positions in milliseconds as 64 bit integers, rather than in seconds as
	// doubles as in Vorbisfile.
	ogg_int64_t totalTime;
	_startTime = startTime;
	_endTime = endTime;
#else
	double totalTime;
	_startTime = startTime / 1000.0;
	_endTime = endTime / 1000.0;
#endif
	
	// If endTime was 0, or is past the end of the file, set it to the maximal time possible
	totalTime = ov_time_total(&_ovFile, -1);
	if (_endTime == 0 || _endTime > totalTime)
		_endTime = totalTime;

	// If the specified time range is empty, abort early.
	if (_startTime >= _endTime) {
		_pos = _bufferEnd;
		return;
	}
	
	// Seek to the start position
	ov_time_seek(&_ovFile, _startTime);

	// Read in initial data
	refill();
	
	// Setup some header information
	_isStereo = ov_info(&_ovFile, -1)->channels >= 2;
	_rate = ov_info(&_ovFile, -1)->rate;
}

VorbisInputStream::~VorbisInputStream() {
	ov_clear(&_ovFile);
	if (_disposeAfterUse)
		delete _inStream;
}

int VorbisInputStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && _pos < _bufferEnd) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			refill();
			// If we are still out of data, and also past the end of specified
			// time range, check whether looping is enabled...
			if (_pos >= _bufferEnd && ov_time_tell(&_ovFile) >= _endTime) {
				if (_numLoops != 1) {
					// If looping is on and there are loops left, rewind to the start
					if (_numLoops != 0)
						_numLoops--;
					ov_time_seek(&_ovFile, _startTime);
					refill();
				}
			}
		}
	}
	return samples;
}

void VorbisInputStream::refill() {
	// Read the samples
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0) {
		if (ov_time_tell(&_ovFile) >= _endTime) {
			// If looping is on and there are loops left, rewind to the start
			if (_numLoops == 1)
				break;	// Last loop, abort
			if (_numLoops != 0)
				_numLoops--;
			ov_time_seek(&_ovFile, _startTime);
		}

		long result;
#ifdef USE_TREMOR
		// Tremor ov_read() always returns data as signed 16 bit interleaved PCM
		// in host byte order. As such, it does not take arguments to request 
		// specific signedness, byte order or bit depth as in Vorbisfile.
		result = ov_read(&_ovFile, read_pos, len_left,
						NULL);
#else
#ifdef SCUMM_BIG_ENDIAN
		result = ov_read(&_ovFile, read_pos, len_left,
						1,
						2,	// 16 bit
						1,	// signed
						NULL);
#else
		result = ov_read(&_ovFile, read_pos, len_left,
						0,
						2,	// 16 bit
						1,	// signed
						NULL);
#endif
#endif
		if (result == OV_HOLE) {
			// Possibly recoverable, just warn about it
			warning("Corrupted data in Vorbis file");
		} else if (result <= 0) {
			if (result < 0)
				debug(1, "Decode error %ld in Vorbis file", result);
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

AudioStream *makeVorbisStream(File *file, uint32 size) {
	assert(file);

	// FIXME: For now, just read the whole data into memory, and be done
	// with it. Of course this is in general *not* a nice thing to do...

	// If no size was specified, read the whole remainder of the file
	if (!size)
		size = file->size() - file->pos();

	// Read 'size' bytes of data into a MemoryReadStream
	Common::MemoryReadStream *stream = file->readStream(size);

	// .. and create a VorbisInputStream from all this
	return new VorbisInputStream(stream, true);
}

AudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 startTime,
	uint32 duration,
	uint numLoops) {

	uint32 endTime = duration ? (startTime + duration) : 0;

	return new VorbisInputStream(stream, disposeAfterUse, startTime, endTime, numLoops);
}



#pragma mark -
#pragma mark --- Ogg Vorbis Audio CD emulation ---
#pragma mark -

class VorbisTrackInfo : public DigitalTrackInfo {
private:
	Common::String _filename;
	bool _errorFlag;

public:
	VorbisTrackInfo(const char *filename);
	bool error() { return _errorFlag; }
	void play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration);
};


VorbisTrackInfo::VorbisTrackInfo(const char *filename) :
	_filename(filename),
	_errorFlag(false) {
	

	// Try to open the file
	Common::File file;
	if (!file.open(_filename)) {
		_errorFlag = true;
		return;
	}
	
	// Next, try to create a VorbisInputStream from it
	VorbisInputStream *tempStream = new VorbisInputStream(&file, false);

	// If an error occured...
	// TODO: add an error or init method to VorbisInputStream
	_errorFlag = tempStream->endOfData();
	
	// Clean up again	
	delete tempStream;
}

void VorbisTrackInfo::play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration) {
	assert(!_errorFlag);

	// Open the file
	Common::File *file = new Common::File();
	if (!file || !file->open(_filename)) {
		warning("VorbisTrackInfo::play: failed to open '%s'", _filename.c_str());
		delete file;
		return;
	}
	
	// Convert startFrame & duration from frames (1/75 s) to milliseconds (1/1000s)
	uint start = startFrame * 1000 / 75;
	uint end = duration ? ((startFrame + duration) * 1000 / 75) : 0;

	// ... create an AudioStream ...
	VorbisInputStream *input = new VorbisInputStream(file, true, start, end);
	
	// ... and play it
	mixer->playInputStream(Audio::Mixer::kMusicSoundType, handle, input);
}

DigitalTrackInfo *getVorbisTrack(int track) {
	char trackName[2][32];

	sprintf(trackName[0], "track%d.ogg", track);
	sprintf(trackName[1], "track%02d.ogg", track);

	for (int i = 0; i < 2; ++i) {
		if (Common::File::exists(trackName[i])) {
			VorbisTrackInfo *trackInfo = new VorbisTrackInfo(trackName[i]);
			if (!trackInfo->error())
				return trackInfo;
			delete trackInfo;
		}
	}
	return NULL;
}



} // End of namespace Audio

#endif // #ifdef USE_VORBIS
