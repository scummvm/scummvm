/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
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

#include "common/debug.h"
#include "common/stream.h"
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
	stream->seek((int32)offset, whence);
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
	const uint _totalNumLoops;

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

	int32 getTotalPlayTime() const {
		if (!_totalNumLoops)
			return AudioStream::kUnknownPlayTime;

#ifdef USE_TREMOR
		return (_endTime - _startTime) * _totalNumLoops;
#else
		return (int32)((_endTime - _startTime) * 1000.0) * _totalNumLoops;
#endif
	}

protected:
	bool refill();
};

VorbisInputStream::VorbisInputStream(Common::SeekableReadStream *inStream, bool dispose, uint startTime, uint endTime, uint numLoops) :
	_inStream(inStream),
	_disposeAfterUse(dispose),
	_numLoops(numLoops),
	_totalNumLoops(numLoops),
	_bufferEnd(_buffer + ARRAYSIZE(_buffer)) {

	int res = ov_open_callbacks(inStream, &_ovFile, NULL, 0, g_stream_wrap);
	if (res < 0) {
		warning("Could not create Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return;
	}

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
	res = ov_time_seek(&_ovFile, _startTime);
	if (res < 0) {
		warning("Error seeking in Vorbis stream (%d)", res);
		_pos = _bufferEnd;
		return;
	}

	// Read in initial data
	if (!refill())
		return;

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
	int res, samples = 0;
	while (samples < numSamples && _pos < _bufferEnd) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			if (!refill())
				break;

			// If we are still out of data, and also past the end of specified
			// time range, check whether looping is enabled...
			if (_pos >= _bufferEnd && ov_time_tell(&_ovFile) >= _endTime) {
				if (_numLoops != 1) {
					// If looping is on and there are loops left, rewind to the start
					if (_numLoops != 0)
						_numLoops--;

					res = ov_time_seek(&_ovFile, _startTime);
					if (res < 0) {
						warning("Error seeking in Vorbis stream (%d)", res);
						_pos = _bufferEnd;
						break;
					}

					if (!refill())
						break;
				}
			}
		}
	}
	return samples;
}

bool VorbisInputStream::refill() {
	// Read the samples
	int res;
	uint len_left = sizeof(_buffer);
	char *read_pos = (char *)_buffer;

	while (len_left > 0) {
		if (ov_time_tell(&_ovFile) >= _endTime) {
			// If looping is on and there are loops left, rewind to the start
			if (_numLoops == 1)
				break;	// Last loop, abort
			if (_numLoops != 0)
				_numLoops--;
			res = ov_time_seek(&_ovFile, _startTime);
			if (res < 0) {
				warning("Error seeking in Vorbis stream (%d)", res);
				_pos = _bufferEnd;
				return false;
			}
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
		} else if (result == 0) {
			warning("End of file while reading from Vorbis file");
			_pos = _bufferEnd;
			return false;
		} else if (result < 0) {
			warning("Error reading from Vorbis stream (%d)", int(result));
			_pos = _bufferEnd;
			// Don't delete it yet, that causes problems in
			// the CD player emulation code.
			return false;
		} else {
			len_left -= result;
			read_pos += result;
		}
	}

	_pos = _buffer;
	_bufferEnd = (int16 *)read_pos;

	return true;
}


#pragma mark -
#pragma mark --- Ogg Vorbis factory functions ---
#pragma mark -


AudioStream *makeVorbisStream(
	Common::SeekableReadStream *stream,
	bool disposeAfterUse,
	uint32 startTime,
	uint32 duration,
	uint numLoops) {

	uint32 endTime = duration ? (startTime + duration) : 0;

	VorbisInputStream *input = new VorbisInputStream(stream, disposeAfterUse, startTime, endTime, numLoops);

	if (input->endOfData()) {
		delete input;
		return 0;
	}

	return input;
}


} // End of namespace Audio

#endif // #ifdef USE_VORBIS
