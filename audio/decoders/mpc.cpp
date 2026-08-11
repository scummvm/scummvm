/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define FORBIDDEN_SYMBOL_EXCEPTION_FILE
#include "audio/decoders/mpc.h"

#ifdef USE_MPCDEC

#ifdef USE_MPCDEC_OLD_API
#include <mpcdec/mpcdec.h>
#else
#include <mpc/mpcdec.h>
#endif

#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"

#include "audio/audiostream.h"

namespace Audio {

// These are wrapper functions to allow using a SeekableReadStream object to
// provide data to the mpc_reader object.

#ifdef USE_MPCDEC_OLD_API
static mpc_int32_t read_stream(void *data, void *ptr, mpc_int32_t size) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)data;
#else
static mpc_int32_t read_stream(mpc_reader *p_reader, void *ptr, mpc_int32_t size) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)p_reader->data;
#endif

	return stream->read(ptr, size);
}

/// Seeks to byte position offset.
#ifdef USE_MPCDEC_OLD_API
static mpc_bool_t seek_stream(void *data, mpc_int32_t offset) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)data;
#else
static mpc_bool_t seek_stream(mpc_reader *p_reader, mpc_int32_t offset) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)p_reader->data;
#endif

	return stream->seek(offset);
}

/// Returns the current byte offset in the stream.
#ifdef USE_MPCDEC_OLD_API
static mpc_int32_t tell_stream(void *data) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)data;
#else
static mpc_int32_t tell_stream(mpc_reader *p_reader) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)p_reader->data;
#endif

	return stream->pos();
}

/// Returns the total length of the source stream, in bytes.
#ifdef USE_MPCDEC_OLD_API
static mpc_int32_t get_size_stream(void *data) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)data;
#else
static mpc_int32_t get_size_stream(mpc_reader *p_reader) {
	Common::SeekableReadStream *stream = (Common::SeekableReadStream *)p_reader->data;
#endif

	return stream->size();
}

/// True if the stream is a seekable stream.
#ifdef USE_MPCDEC_OLD_API
static mpc_bool_t canseek_stream(void *p_reader) {
	return TRUE;
}
#else
static mpc_bool_t canseek_stream(mpc_reader *p_reader) {
	return MPC_TRUE;
}
#endif


#pragma mark -
#pragma mark --- Musepack stream ---
#pragma mark -


class MPCStream : public SeekableAudioStream {
protected:
	Common::DisposablePtr<Common::SeekableReadStream> _inStream;

	bool _isStereo;
	int _rate;

	Timestamp _length;

	mpc_reader _reader;
	mpc_streaminfo _si;

#ifdef USE_MPCDEC_OLD_API
	mpc_decoder _decoder;
#else
	mpc_demux *_demux;
#endif

	MPC_SAMPLE_FORMAT _bufferDec[MPC_DECODER_BUFFER_LENGTH];
	uint16 _buffer[MPC_DECODER_BUFFER_LENGTH];
	const uint16 *_bufferEnd;
	const uint16 *_pos;

public:
	// startTime / duration are in milliseconds
	MPCStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose);
	~MPCStream();

	int readBuffer(int16 *buffer, const int numSamples) override;

	bool endOfData() const override		{ return _pos >= _bufferEnd; }
	bool isStereo() const override		{ return _isStereo; }
	int getRate() const override		{ return _rate; }

	bool seek(const Timestamp &where) override;
	Timestamp getLength() const override { return _length; }
protected:
	bool refill();
};

MPCStream::MPCStream(Common::SeekableReadStream *inStream, DisposeAfterUse::Flag dispose) :
	_inStream(inStream, dispose),
	_length(0, 1000),
	_bufferEnd(ARRAYEND(_buffer)) {

	_pos = _bufferEnd;	// This will return endOfBuffer() if we're not properly inited

	_reader.read = read_stream;
	_reader.seek = seek_stream;
	_reader.tell = tell_stream;
	_reader.get_size = get_size_stream;
	_reader.canseek = canseek_stream;
	_reader.data = (void *)inStream;

#ifdef USE_MPCDEC_OLD_API
	mpc_streaminfo_init(&_si);
	if (mpc_streaminfo_read(&_si, &_reader) < 0) {
		warning("Cannot read musepack stream info");
		return;
	}
	mpc_decoder_setup(&_decoder, &_reader);
	mpc_decoder_scale_output (&_decoder, 1.0);
	if (!mpc_decoder_initialize(&_decoder, &_si)) {
		warning("Cannot initialize musepack decoder");
		return;
	}
#else
	_demux = mpc_demux_init(&_reader);

	if (!_demux) {
		warning("Cannot init musepack demuxer");
		return;
	}

	mpc_demux_get_info(_demux, &_si);
#endif

	_isStereo = _si.channels >= 2;
	_rate = _si.sample_freq;
	_length = Timestamp(uint32(mpc_streaminfo_get_length(&_si) * 1000.0), getRate());

	int time = (int)mpc_streaminfo_get_length(&_si);
	int minutes = time / 60;
	int seconds = time % 60;

	debug(9, "stream version %d", _si.stream_version);
	debug(9, "encoder: %s", _si.encoder);
#ifdef USE_MPCDEC_OLD_API
	debug(9, "profile: %s (q=%d)", _si.profile_name, _si.profile);
#else
	debug(9, "profile: %s (q=%0.2f)", _si.profile_name, _si.profile - 5);
	debug(9, "PNS: %s", _si.pns == 0xFF ? "unknow" : _si.pns ? "on" : "off");
#endif
	debug(9, "mid/side stereo: %s", _si.ms ? "on" : "off");
	debug(9, "gapless: %s", _si.is_true_gapless ? "on" : "off");
	debug(9, "average bitrate: %6.1f kbps", _si.average_bitrate * 1.e-3);
	debug(9, "samplerate: %d Hz", _si.sample_freq);
	debug(9, "channels: %d", _si.channels);
	debug(9, "length: %d:%.2d (%u samples)", minutes, seconds, (mpc_uint32_t)mpc_streaminfo_get_length_samples(&_si));
	debug(9, "file size: %d Bytes", _si.total_file_length);
	debug(9, "track peak: %2.2f dB", _si.peak_title / 256.f);
	debug(9, "track gain: %2.2f dB / %2.2f dB", _si.gain_title / 256.f, _si.gain_title == 0 ? 0 : 64.82f - _si.gain_title / 256.f);
	debug(9, "album peak: %2.2f dB", _si.peak_album / 256.f);
	debug(9, "album gain: %2.2f dB / %2.2f dB", _si.gain_album / 256.f, _si.gain_album == 0 ? 0 : 64.82f - _si.gain_album / 256.f);

	if (!refill())
		return;
}

MPCStream::~MPCStream() {
#ifndef USE_MPCDEC_OLD_API
	mpc_demux_exit(_demux);
#endif
}

int MPCStream::readBuffer(int16 *buffer, const int numSamples) {
	int samples = 0;
	while (samples < numSamples && _pos < _bufferEnd) {
		const int len = MIN(numSamples - samples, (int)(_bufferEnd - _pos));
		memcpy(buffer, _pos, len * 2);
		buffer += len;
		_pos += len;
		samples += len;
		if (_pos >= _bufferEnd) {
			if (!refill())
				break;
		}
	}
	return samples;
}

bool MPCStream::seek(const Timestamp &where) {
#ifdef USE_MPCDEC_OLD_API
	bool res = (mpc_decoder_seek_seconds(&_decoder, (double)where.msecs() / 1000.0) == TRUE);
#else
	bool res = (mpc_demux_seek_second(_demux, (double)where.msecs() / 1000.0) == MPC_STATUS_OK);
#endif
	if (!res) {
		warning("Error seeking in musepack stream");
		_pos = _bufferEnd;
		return false;
	}

	return refill();
}

bool MPCStream::refill() {
	bool result;
	uint32 samples;

#ifdef USE_MPCDEC_OLD_API
	uint32 vbr_update_acc, vbr_update_bits;
	samples = mpc_decoder_decode(&_decoder, _bufferDec, &vbr_update_acc, &vbr_update_bits);

	if (samples == 0) { // End of stream
		_pos = _buffer;
		_bufferEnd = _buffer;

		return false;
	}

	if (samples == -1u) { // Corruptd stream
		result = false;
		samples = 0;
	} else {
		result = true;
	}
#else
	mpc_frame_info frame;
	frame.buffer = _bufferDec;

	result = (mpc_demux_decode(_demux, &frame) == MPC_STATUS_OK);

	if (frame.bits == -1) { // End of stream
		_pos = _buffer;
		_bufferEnd = _buffer;

		return false;
	}

	samples = frame.samples;
#endif

	if (!result) {
		// Possibly recoverable, just warn about it
		warning("Corrupted data in musepack file");
	}

#ifdef MPC_FIXED_POINT
	for(int i = 0; i < MPC_DECODER_BUFFER_LENGTH; i++) {
		int tmp = _bufferDec[i] >> MPC_FIXED_POINT_FRACTPART;
		if (tmp > ((1 << 15) - 1)) tmp = ((1 << 15) - 1);
		if (tmp < -(1 << 15)) tmp = -(1 << 15);
		_buffer[i] = tmp;
	}
#else
	for (int i = 0; i < MPC_DECODER_BUFFER_LENGTH; i++) {
		int tmp = nearbyintf(_bufferDec[i] * (1 << 15));
		if (tmp > ((1 << 15) - 1))
			tmp = ((1 << 15) - 1);
		if (tmp < -(1 << 15))
			tmp = -(1 << 15);
		_buffer[i] = (uint16)tmp;
	}
#endif

	_pos = _buffer;
	_bufferEnd = &_buffer[samples * _si.channels];

	return true;
}

#pragma mark -
#pragma mark --- Ogg Vorbis factory functions ---
#pragma mark -

SeekableAudioStream *makeMPCStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {
	SeekableAudioStream *s = new MPCStream(stream, disposeAfterUse);
	if (s && s->endOfData()) {
		delete s;
		return nullptr;
	} else {
		return s;
	}
}

} // End of namespace Audio

#endif // #ifdef USE_MPCDEC
