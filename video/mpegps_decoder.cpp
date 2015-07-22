/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "video/mpegps_decoder.h"
#include "image/codecs/mpeg.h"

// The demuxing code is based on libav's demuxing code

namespace Video {

enum {
	kStartCodePack = 0x1BA,
	kStartCodeSystemHeader = 0x1BB,
	kStartCodeProgramStreamMap = 0x1BC,
	kStartCodePrivateStream1 = 0x1BD,
	kStartCodePaddingStream = 0x1BE,
	kStartCodePrivateStream2 = 0x1BF
};

MPEGPSDecoder::MPEGPSDecoder() {
	_stream = 0;
	memset(_psmESType, 0, 256);
}

MPEGPSDecoder::~MPEGPSDecoder() {
	close();
}

bool MPEGPSDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	if (!addFirstVideoTrack()) {
		close();
		return false;
	}

	_stream->seek(0);
	return true;
}

void MPEGPSDecoder::close() {
	VideoDecoder::close();

	delete _stream;
	_stream = 0;

	_streamMap.clear();

	memset(_psmESType, 0, 256);
}

void MPEGPSDecoder::readNextPacket() {
	if (_stream->eos())
		return;

	for (;;) {
		int32 startCode;
		uint32 pts, dts;
		int size = readNextPacketHeader(startCode, pts, dts);

		if (size < 0) {
			// End of stream
			for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
				if ((*it)->getTrackType() == Track::kTrackTypeVideo)
					((MPEGVideoTrack *)*it)->setEndOfTrack();
			return;
		}

		MPEGStream *stream = 0;
		Common::SeekableReadStream *packet = _stream->readStream(size);

		if (_streamMap.contains(startCode)) {
			// We already found the stream
			stream = _streamMap[startCode];
		} else {
			// We haven't seen this before

			if (startCode == kStartCodePrivateStream1) {
				PrivateStreamType streamType = detectPrivateStreamType(packet);
				packet->seek(0);

				// TODO: Handling of these types (as needed)

				const char *typeName;

				switch (streamType) {
				case kPrivateStreamAC3:
					typeName = "AC-3";
					break;
				case kPrivateStreamDTS:
					typeName = "DTS";
					break;
				case kPrivateStreamDVDPCM:
					typeName = "DVD PCM";
					break;
				case kPrivateStreamPS2Audio:
					typeName = "PS2 Audio";
					break;
				default:
					typeName = "Unknown";
					break;
				}

				warning("Unhandled DVD private stream: %s", typeName);

				// Make it 0 so we don't get the warning twice
				_streamMap[startCode] = 0;
			} else if (startCode >= 0x1E0 && startCode <= 0x1EF) {
				// Video stream
				// TODO: Multiple video streams
				warning("Found extra video stream 0x%04X", startCode);
				_streamMap[startCode] = 0;
			} else if (startCode >= 0x1C0 && startCode <= 0x1DF) {
#ifdef USE_MAD
				// MPEG Audio stream
				MPEGAudioTrack *audioTrack = new MPEGAudioTrack(packet);
				stream = audioTrack;
				_streamMap[startCode] = audioTrack;
				addTrack(audioTrack);
#else
				warning("Found audio stream 0x%04X, but no MAD support compiled in", startCode);
				_streamMap[startCode] = 0;
#endif
			} else {
				// Probably not relevant
				debug(0, "Found unhandled MPEG-PS stream type 0x%04x", startCode);
				_streamMap[startCode] = 0;
			}
		}

		if (stream) {
			bool done = stream->sendPacket(packet, pts, dts);

			if (done && stream->getStreamType() == MPEGStream::kStreamTypeVideo)
				return;
		} else {
			delete packet;
		}
	}
}

#define MAX_SYNC_SIZE 100000

int MPEGPSDecoder::findNextStartCode(uint32 &size) {
	size = MAX_SYNC_SIZE;
	int32 state = 0xFF;

	while (size > 0) {
		byte v = _stream->readByte();

		if (_stream->eos())
			return -1;

		size--;

		if (state == 0x1)
			return ((state << 8) | v) & 0xFFFFFF;

		state = ((state << 8) | v) & 0xFFFFFF;
	}

	return -1;
}

int MPEGPSDecoder::readNextPacketHeader(int32 &startCode, uint32 &pts, uint32 &dts) {
	for (;;) {
		uint32 size;
		startCode = findNextStartCode(size);

		if (_stream->eos())
			return -1;

		if (startCode < 0)
			continue;

		uint32 lastSync = _stream->pos();

		if (startCode == kStartCodePack || startCode == kStartCodeSystemHeader)
			continue;

		int length = _stream->readUint16BE();

		if (startCode == kStartCodePaddingStream || startCode == kStartCodePrivateStream2) {
			_stream->skip(length);
			continue;
		}

		if (startCode == kStartCodeProgramStreamMap) {
			parseProgramStreamMap(length);
			continue;
		}

		// Find matching stream
		if (!((startCode >= 0x1C0 && startCode <= 0x1DF) ||
				(startCode >= 0x1E0 && startCode <= 0x1EF) ||
				startCode == kStartCodePrivateStream1 || startCode == 0x1FD))
			continue;

		// Stuffing
		byte c;
		for (;;) {
			if (length < 1) {
				_stream->seek(lastSync);
				continue;
			}

			c = _stream->readByte();
			length--;

			// XXX: for mpeg1, should test only bit 7
			if (c != 0xFF)
				break;
		}

		if ((c & 0xC0) == 0x40) {
			// Buffer scale and size
			_stream->readByte();
			c = _stream->readByte();
			length -= 2;
		}

		pts = 0xFFFFFFFF;
		dts = 0xFFFFFFFF;

		if ((c & 0xE0) == 0x20) {
			dts = pts = readPTS(c);
			length -= 4;

			if (c & 0x10) {
				dts = readPTS(-1);
				length -= 5;
			}
		} else if ((c & 0xC0) == 0x80) {
			// MPEG-2 PES
			byte flags = _stream->readByte();
			int headerLength = _stream->readByte();
			length -= 2;

			if (headerLength > length) {
				_stream->seek(lastSync);
				continue;
			}

			length -= headerLength;

			if (flags & 0x80) {
				dts = pts = readPTS(-1);
				headerLength -= 5;

				if (flags & 0x40) {
					dts = readPTS(-1);
					headerLength -= 5;
				}
			}

			if (flags & 0x3F && headerLength == 0) {
				flags &= 0xC0;
				warning("Further flags set but no bytes left");
			}

			if (flags & 0x01) { // PES extension
				byte pesExt =_stream->readByte();
				headerLength--;

				// Skip PES private data, program packet sequence
				int skip = (pesExt >> 4) & 0xB;
				skip += skip & 0x9;

				if (pesExt & 0x40 || skip > headerLength) {
					warning("pesExt %x is invalid", pesExt);
					pesExt = skip = 0;
				} else {
					_stream->skip(skip);
					headerLength -= skip;
				}

				if (pesExt & 0x01) { // PES extension 2
					byte ext2Length = _stream->readByte();
					headerLength--;

					if ((ext2Length & 0x7F) != 0) {
						byte idExt = _stream->readByte();

						if ((idExt & 0x80) == 0)
							startCode = (startCode & 0xFF) << 8;

						headerLength--;
					}
				}
			}

			if (headerLength < 0) {
				_stream->seek(lastSync);
				continue;
			}

			_stream->skip(headerLength);
		} else if (c != 0xF) {
			continue;
		}

		if (length < 0) {
			_stream->seek(lastSync);
			continue;
		}

		return length;
	}
}

uint32 MPEGPSDecoder::readPTS(int c) {
	byte buf[5];

	buf[0] = (c < 0) ? _stream->readByte() : c;
	_stream->read(buf + 1, 4);

	return ((buf[0] & 0x0E) << 29) | ((READ_BE_UINT16(buf + 1) >> 1) << 15) | (READ_BE_UINT16(buf + 3) >> 1);
}

void MPEGPSDecoder::parseProgramStreamMap(int length) {
	_stream->readByte();
	_stream->readByte();

	// skip program stream info
	_stream->skip(_stream->readUint16BE());

	int esMapLength = _stream->readUint16BE();

	while (esMapLength >= 4) {
		byte type = _stream->readByte();
		byte esID = _stream->readByte();
		uint16 esInfoLength = _stream->readUint16BE();

		// Remember mapping from stream id to stream type
		_psmESType[esID] = type;

		// Skip program stream info
		_stream->skip(esInfoLength);

		esMapLength -= 4 + esInfoLength;
	}

	_stream->readUint32BE(); // CRC32
}

bool MPEGPSDecoder::addFirstVideoTrack() {
	for (;;) {
		int32 startCode;
		uint32 pts, dts;
		int size = readNextPacketHeader(startCode, pts, dts);

		// End of stream? We failed
		if (size < 0)
			return false;

		if (startCode >= 0x1E0 && startCode <= 0x1EF) {
			// Video stream
			// Can be MPEG-1/2 or MPEG-4/h.264. We'll assume the former and
			// I hope we never need the latter.
			Common::SeekableReadStream *firstPacket = _stream->readStream(size);
			MPEGVideoTrack *track = new MPEGVideoTrack(firstPacket, getDefaultHighColorFormat());
			addTrack(track);
			_streamMap[startCode] = track;
			delete firstPacket;
			break;
		}

		_stream->skip(size);
	}

	return true;
}

MPEGPSDecoder::PrivateStreamType MPEGPSDecoder::detectPrivateStreamType(Common::SeekableReadStream *packet) {
	uint32 dvdCode = packet->readUint32LE();
	if (packet->eos())
		return kPrivateStreamUnknown;

	uint32 ps2Header = packet->readUint32BE();
	if (!packet->eos() && ps2Header == MKTAG('S', 'S', 'h', 'd'))
		return kPrivateStreamPS2Audio;

	switch (dvdCode & 0xE0) {
	case 0x80:
		if ((dvdCode & 0xF8) == 0x88)
			return kPrivateStreamDTS;

		return kPrivateStreamAC3;
	case 0xA0:
		return kPrivateStreamDVDPCM;
	}

	return kPrivateStreamUnknown;
}

MPEGPSDecoder::MPEGVideoTrack::MPEGVideoTrack(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format) {
	_surface = 0;
	_endOfTrack = false;
	_curFrame = -1;
	_nextFrameStartTime = Audio::Timestamp(0, 27000000); // 27 MHz timer

	findDimensions(firstPacket, format);

#ifdef USE_MPEG2
	_mpegDecoder = new Image::MPEGDecoder();
#endif
}

MPEGPSDecoder::MPEGVideoTrack::~MPEGVideoTrack() {
#ifdef USE_MPEG2
	delete _mpegDecoder;
#endif

	if (_surface) {
		_surface->free();
		delete _surface;
	}
}

uint16 MPEGPSDecoder::MPEGVideoTrack::getWidth() const {
	return _surface ? _surface->w : 0;
}

uint16 MPEGPSDecoder::MPEGVideoTrack::getHeight() const {
	return _surface ? _surface->h : 0;
}

Graphics::PixelFormat MPEGPSDecoder::MPEGVideoTrack::getPixelFormat() const {
	if (!_surface)
		return Graphics::PixelFormat();

	return _surface->format;
}

const Graphics::Surface *MPEGPSDecoder::MPEGVideoTrack::decodeNextFrame() {
	return _surface;
}

bool MPEGPSDecoder::MPEGVideoTrack::sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) {
#ifdef USE_MPEG2
	uint32 framePeriod;
	bool foundFrame = _mpegDecoder->decodePacket(*packet, framePeriod, _surface);

	if (foundFrame) {
		_curFrame++;
		_nextFrameStartTime = _nextFrameStartTime.addFrames(framePeriod);
	}
#endif

	delete packet;

#ifdef USE_MPEG2
	return foundFrame;
#else
	return true;
#endif
}

void MPEGPSDecoder::MPEGVideoTrack::findDimensions(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format) {
	// First, check for the picture start code
	if (firstPacket->readUint32BE() != 0x1B3)
		error("Failed to detect MPEG sequence start");

	// This is part of the bitstream, but there's really no purpose
	// to use Common::BitStream just for this: 12 bits width, 12 bits
	// height
	uint16 width = firstPacket->readByte() << 4;
	uint16 height = firstPacket->readByte();
	width |= (height & 0xF0) >> 4;
	height = ((height & 0x0F) << 8) | firstPacket->readByte();

	debug(0, "MPEG dimensions: %dx%d", width, height);

	_surface = new Graphics::Surface();
	_surface->create(width, height, format);

	firstPacket->seek(0);
}

#ifdef USE_MAD

// The audio code here is almost entirely based on what we do in mp3.cpp

MPEGPSDecoder::MPEGAudioTrack::MPEGAudioTrack(Common::SeekableReadStream *firstPacket) {
	// The MAD_BUFFER_GUARD must always contain zeros (the reason
	// for this is that the Layer III Huffman decoder of libMAD
	// may read a few bytes beyond the end of the input buffer).
	memset(_buf + BUFFER_SIZE, 0, MAD_BUFFER_GUARD);

	_state = MP3_STATE_INIT;
	_audStream = 0;

	// Find out our audio parameters
	initStream(firstPacket);

	while (_state != MP3_STATE_EOS)
		readHeader(firstPacket);

	_audStream = Audio::makeQueuingAudioStream(_frame.header.samplerate, MAD_NCHANNELS(&_frame.header) == 2);

	deinitStream();

	firstPacket->seek(0);
	_state = MP3_STATE_INIT;
}

MPEGPSDecoder::MPEGAudioTrack::~MPEGAudioTrack() {
	deinitStream();
	delete _audStream;
}

static inline int scaleSample(mad_fixed_t sample) {
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

bool MPEGPSDecoder::MPEGAudioTrack::sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) {
	while (_state != MP3_STATE_EOS)
		decodeMP3Data(packet);

	_state = MP3_STATE_READY;
	delete packet;
	return true;
}

Audio::AudioStream *MPEGPSDecoder::MPEGAudioTrack::getAudioStream() const {
	return _audStream;
}

void MPEGPSDecoder::MPEGAudioTrack::initStream(Common::SeekableReadStream *packet) {
	if (_state != MP3_STATE_INIT)
		deinitStream();

	// Init MAD
	mad_stream_init(&_stream);
	mad_frame_init(&_frame);
	mad_synth_init(&_synth);

	// Reset the stream data
	packet->seek(0, SEEK_SET);

	// Update state
	_state = MP3_STATE_READY;

	// Read the first few sample bytes
	readMP3Data(packet);
}

void MPEGPSDecoder::MPEGAudioTrack::deinitStream() {
	if (_state == MP3_STATE_INIT)
		return;

	// Deinit MAD
	mad_synth_finish(&_synth);
	mad_frame_finish(&_frame);
	mad_stream_finish(&_stream);

	_state = MP3_STATE_EOS;
}

void MPEGPSDecoder::MPEGAudioTrack::readMP3Data(Common::SeekableReadStream *packet) {
	uint32 remaining = 0;

	// Give up immediately if we already used up all data in the stream
	if (packet->eos()) {
		_state = MP3_STATE_EOS;
		return;
	}

	if (_stream.next_frame) {
		// If there is still data in the MAD stream, we need to preserve it.
		// Note that we use memmove, as we are reusing the same buffer,
		// and hence the data regions we copy from and to may overlap.
		remaining = _stream.bufend - _stream.next_frame;
		assert(remaining < BUFFER_SIZE);	// Paranoia check
		memmove(_buf, _stream.next_frame, remaining);
	}

	memset(_buf + remaining, 0, BUFFER_SIZE - remaining);

	// Try to read the next block
	uint32 size = packet->read(_buf + remaining, BUFFER_SIZE - remaining);
	if (size == 0) {
		_state = MP3_STATE_EOS;
		return;
	}

	// Feed the data we just read into the stream decoder
	_stream.error = MAD_ERROR_NONE;
	mad_stream_buffer(&_stream, _buf, size + remaining);
}

void MPEGPSDecoder::MPEGAudioTrack::readHeader(Common::SeekableReadStream *packet) {
	if (_state != MP3_STATE_READY)
		return;

	// If necessary, load more data into the stream decoder
	if (_stream.error == MAD_ERROR_BUFLEN)
		readMP3Data(packet);

	while (_state != MP3_STATE_EOS) {
		_stream.error = MAD_ERROR_NONE;

		// Decode the next header. Note: mad_frame_decode would do this for us, too.
		// However, for seeking we don't want to decode the full frame (else it would
		// be far too slow). Hence we perform this explicitly in a separate step.
		if (mad_header_decode(&_frame.header, &_stream) == -1) {
			if (_stream.error == MAD_ERROR_BUFLEN) {
				readMP3Data(packet);  // Read more data
				continue;
			} else if (MAD_RECOVERABLE(_stream.error)) {
				debug(6, "MPEGAudioTrack::readHeader(): Recoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				continue;
			} else {
				warning("MPEGAudioTrack::readHeader(): Unrecoverable error in mad_header_decode (%s)", mad_stream_errorstr(&_stream));
				break;
			}
		}

		break;
	}

	if (_stream.error != MAD_ERROR_NONE)
		_state = MP3_STATE_EOS;
}

void MPEGPSDecoder::MPEGAudioTrack::decodeMP3Data(Common::SeekableReadStream *packet) {
	if (_state == MP3_STATE_INIT)
		initStream(packet);

	if (_state == MP3_STATE_EOS)
		return;

	do {
		// If necessary, load more data into the stream decoder
		if (_stream.error == MAD_ERROR_BUFLEN)
			readMP3Data(packet);

		while (_state == MP3_STATE_READY) {
			_stream.error = MAD_ERROR_NONE;

			// Decode the next frame
			if (mad_frame_decode(&_frame, &_stream) == -1) {
				if (_stream.error == MAD_ERROR_BUFLEN) {
					break; // Read more data
				} else if (MAD_RECOVERABLE(_stream.error)) {
					// Note: we will occasionally see MAD_ERROR_BADDATAPTR errors here.
					// These are normal and expected (caused by our frame skipping (i.e. "seeking")
					// code above).
					debug(6, "MPEGAudioTrack::decodeMP3Data(): Recoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					continue;
				} else {
					warning("MPEGAudioTrack::decodeMP3Data(): Unrecoverable error in mad_frame_decode (%s)", mad_stream_errorstr(&_stream));
					break;
				}
			}

			// Synthesize PCM data
			mad_synth_frame(&_synth, &_frame);

			// Output it to our queue
			if (_synth.pcm.length != 0) {
				byte *buffer = (byte *)malloc(_synth.pcm.length * 2 * MAD_NCHANNELS(&_frame.header));
				int16 *ptr = (int16 *)buffer;

				for (int i = 0; i < _synth.pcm.length; i++) {
					*ptr++ = (int16)scaleSample(_synth.pcm.samples[0][i]);

					if (MAD_NCHANNELS(&_frame.header) == 2)
						*ptr++ = (int16)scaleSample(_synth.pcm.samples[1][i]);
				}

				int flags = Audio::FLAG_16BITS;

				if (_audStream->isStereo())
					flags |= Audio::FLAG_STEREO;

#ifdef SCUMM_LITTLE_ENDIAN
				flags |= Audio::FLAG_LITTLE_ENDIAN;
#endif

				_audStream->queueBuffer(buffer, _synth.pcm.length * 2 * MAD_NCHANNELS(&_frame.header), DisposeAfterUse::YES, flags);
			}
			break;
		}
	} while (_state != MP3_STATE_EOS && _stream.error == MAD_ERROR_BUFLEN);

	if (_stream.error != MAD_ERROR_NONE)
		_state = MP3_STATE_EOS;
}

#endif

} // End of namespace Video
