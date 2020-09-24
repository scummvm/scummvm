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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/ac3.h"
#include "audio/decoders/mp3.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/memstream.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "video/mpegps_decoder.h"
#include "image/codecs/mpeg.h"

// The demuxing code is based on libav's demuxing code

namespace Video {

// --------------------------------------------------------------------------
// Decoder - This is the part that takes a packet and figures out what to do
// with it.
// --------------------------------------------------------------------------

enum {
	kStartCodePack = 0x1BA,
	kStartCodeSystemHeader = 0x1BB,
	kStartCodeProgramStreamMap = 0x1BC,
	kStartCodePrivateStream1 = 0x1BD,
	kStartCodePaddingStream = 0x1BE,
	kStartCodePrivateStream2 = 0x1BF
};

MPEGPSDecoder::MPEGPSDecoder(double decibel) {
	_decibel = decibel;
	_demuxer = new MPEGPSDemuxer();
}

MPEGPSDecoder::~MPEGPSDecoder() {
	close();
	delete _demuxer;
}

bool MPEGPSDecoder::loadStream(Common::SeekableReadStream *stream) {
	close();

	if (!_demuxer->loadStream(stream)) {
		close();
		return false;
	}

	if (!addFirstVideoTrack()) {
		close();
		return false;
	}

	return true;
}

void MPEGPSDecoder::close() {
	VideoDecoder::close();
	_demuxer->close();
	_streamMap.clear();
}

MPEGPSDecoder::MPEGStream *MPEGPSDecoder::getStream(uint32 startCode, Common::SeekableReadStream *packet) {
	MPEGStream *stream = 0;

	if (_streamMap.contains(startCode)) {
		// We already found the stream
		stream = _streamMap[startCode];
	} else {
		// We haven't seen this before

		if (startCode == kStartCodePrivateStream1) {
			PrivateStreamType streamType = detectPrivateStreamType(packet);
			packet->seek(0);

			// TODO: Handling of these types (as needed)
			bool handled = false;
			const char *typeName;

			switch (streamType) {
			case kPrivateStreamAC3: {
				typeName = "AC-3";

#ifdef USE_A52
				handled = true;
				AC3AudioTrack *ac3Track = new AC3AudioTrack(*packet, _decibel, getSoundType());
				stream = ac3Track;
				_streamMap[startCode] = ac3Track;
				addTrack(ac3Track);
#endif
				break;
			}
			case kPrivateStreamDTS:
				typeName = "DTS";
				break;
			case kPrivateStreamDVDPCM:
				typeName = "DVD PCM";
				break;
			case kPrivateStreamPS2Audio: {
				typeName = "PS2 Audio";
				handled = true;
				PS2AudioTrack *audioTrack = new PS2AudioTrack(packet, getSoundType());
				stream = audioTrack;
				_streamMap[startCode] = audioTrack;
				addTrack(audioTrack);
				break;
			}
			default:
				typeName = "Unknown";
				break;
			}

			if (!handled) {
				warning("Unhandled DVD private stream: %s", typeName);

				// Make it 0 so we don't get the warning twice
				_streamMap[startCode] = 0;
			}
		} else if (startCode >= 0x1E0 && startCode <= 0x1EF) {
			// Video stream
			// TODO: Multiple video streams
			warning("Found extra video stream 0x%04X", startCode);
			_streamMap[startCode] = 0;
		} else if (startCode >= 0x1C0 && startCode <= 0x1DF) {
#ifdef USE_MAD
			// MPEG Audio stream
			MPEGAudioTrack *audioTrack = new MPEGAudioTrack(*packet, getSoundType());
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

	return stream;
}

void MPEGPSDecoder::readNextPacket() {
	for (;;) {
		int32 startCode;
		uint32 pts, dts;
		Common::SeekableReadStream *packet = _demuxer->getNextPacket(getTime(), startCode, pts, dts);

		if (!packet) {
			// End of stream
			for (TrackListIterator it = getTrackListBegin(); it != getTrackListEnd(); it++)
				if ((*it)->getTrackType() == Track::kTrackTypeVideo)
					((MPEGVideoTrack *)*it)->setEndOfTrack();
			return;
		}

		MPEGStream *stream = getStream(startCode, packet);

		if (stream) {
			packet->seek(0);

			bool done = stream->sendPacket(packet, pts, dts);

			if (done && stream->getStreamType() == MPEGStream::kStreamTypeVideo)
				return;
		} else {
			delete packet;
		}
	}
}

bool MPEGPSDecoder::addFirstVideoTrack() {
	int32 startCode;
	uint32 pts, dts;
	Common::SeekableReadStream *packet = _demuxer->getFirstVideoPacket(startCode, pts, dts);

	if (!packet)
		return false;

	// Video stream
	// Can be MPEG-1/2 or MPEG-4/h.264. We'll assume the former and
	// I hope we never need the latter.
	MPEGVideoTrack *track = new MPEGVideoTrack(packet, getDefaultHighColorFormat());
	addTrack(track);
	_streamMap[startCode] = track;

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
	default:
		break;
	}

	return kPrivateStreamUnknown;
}

// --------------------------------------------------------------------------
// Demuxer - This is the part that reads packets from the stream and delivers
// them to the decoder.
//
// It will buffer a number of packets in advance, because otherwise it may
// not encounter any audio packets until it's far too late to decode them.
// Before I added this, there would be 9 or 10 frames of video before the
// first audio packet, even though the timestamp indicated that the audio
// should start slightly before the video.
// --------------------------------------------------------------------------

#define PREBUFFERED_PACKETS 150
#define AUDIO_THRESHOLD     100

MPEGPSDecoder::MPEGPSDemuxer::MPEGPSDemuxer() {
	_stream = 0;
}

MPEGPSDecoder::MPEGPSDemuxer::~MPEGPSDemuxer() {
	close();
}

bool MPEGPSDecoder::MPEGPSDemuxer::loadStream(Common::SeekableReadStream *stream) {
	close();

	_stream = stream;

	int queuedPackets = 0;
	while (queueNextPacket() && queuedPackets < PREBUFFERED_PACKETS) {
		queuedPackets++;
	}

	return true;
}

void MPEGPSDecoder::MPEGPSDemuxer::close() {
	delete _stream;
	_stream = 0;

	while (!_audioQueue.empty()) {
		Packet packet = _audioQueue.pop();
		delete packet._stream;
	}

	while (!_videoQueue.empty()) {
		Packet packet = _videoQueue.pop();
		delete packet._stream;
	}
}

Common::SeekableReadStream *MPEGPSDecoder::MPEGPSDemuxer::getFirstVideoPacket(int32 &startCode, uint32 &pts, uint32 &dts) {
	if (_videoQueue.empty())
		return nullptr;
	Packet packet = _videoQueue.front();
	startCode = packet._startCode;
	pts = packet._pts;
	dts = packet._dts;
	return packet._stream;
}

Common::SeekableReadStream *MPEGPSDecoder::MPEGPSDemuxer::getNextPacket(uint32 currentTime, int32 &startCode, uint32 &pts, uint32 &dts) {
	queueNextPacket();

	// The idea here is to prioritize the delivery of audio packets,
	// because when the decoder wants a frame it will keep asking until it
	// gets a frame. There is nothing like that in the decoder to ensure
	// speedy delivery of audio.

	if (!_audioQueue.empty()) {
		Packet packet = _audioQueue.front();
		bool usePacket = false;

		if (packet._pts == 0xFFFFFFFF) {
			// No timestamp? Use it just in case. This could be a
			// bad idea, but in my tests all audio packets have a
			// time stamp.
			usePacket = true;
		} else {
			uint32 packetTime = packet._pts / 90;
			if (packetTime <= currentTime || packetTime - currentTime < AUDIO_THRESHOLD || _videoQueue.empty()) {
				// The packet is overdue, or will be soon.
				//
				// TODO: We should pad or trim the first audio
				// packet based on the timestamp to get the
				// audio to start at the exact desired time.
				// But for some reason it seems to work well
				// enough anyway. For now.
				usePacket = true;
			}
		}

		if (usePacket) {
			_audioQueue.pop();
			startCode = packet._startCode;
			pts = packet._pts;
			dts = packet._dts;
			return packet._stream;
		}
	}

	if (!_videoQueue.empty()) {
		Packet packet = _videoQueue.pop();
		startCode = packet._startCode;
		pts = packet._pts;
		dts = packet._dts;
		return packet._stream;
	}

	return nullptr;
}

bool MPEGPSDecoder::MPEGPSDemuxer::queueNextPacket() {
	if (_stream->eos())
		return false;

	for (;;) {
		int32 startCode;
		uint32 pts, dts;
		int size = readNextPacketHeader(startCode, pts, dts);

		if (size < 0) {
			// End of stream
			return false;
		}

		Common::SeekableReadStream *stream = _stream->readStream(size);

		if (startCode == kStartCodePrivateStream1 || (startCode >= 0x1C0 && startCode <= 0x1DF)) {
			// Audio packet
			_audioQueue.push(Packet(stream, startCode, pts, dts));
			return true;
		}

		if (startCode >= 0x1E0 && startCode <= 0x1EF) {
			// Video packet
			_videoQueue.push(Packet(stream, startCode, pts, dts));
			return true;
		}

		delete stream;
	}
}

int MPEGPSDecoder::MPEGPSDemuxer::readNextPacketHeader(int32 &startCode, uint32 &pts, uint32 &dts) {
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

#define MAX_SYNC_SIZE 100000

int MPEGPSDecoder::MPEGPSDemuxer::findNextStartCode(uint32 &size) {
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

uint32 MPEGPSDecoder::MPEGPSDemuxer::readPTS(int c) {
	byte buf[5];

	buf[0] = (c < 0) ? _stream->readByte() : c;
	_stream->read(buf + 1, 4);

	return ((buf[0] & 0x0E) << 29) | ((READ_BE_UINT16(buf + 1) >> 1) << 15) | (READ_BE_UINT16(buf + 3) >> 1);
}

void MPEGPSDecoder::MPEGPSDemuxer::parseProgramStreamMap(int length) {
	_stream->readByte();
	_stream->readByte();

	// skip program stream info
	_stream->skip(_stream->readUint16BE());

	int esMapLength = _stream->readUint16BE();

	while (esMapLength >= 4) {
		_stream->readByte(); // type
		_stream->readByte(); // esID
		uint16 esInfoLength = _stream->readUint16BE();

		// Skip program stream info
		_stream->skip(esInfoLength);

		esMapLength -= 4 + esInfoLength;
	}

	_stream->readUint32BE(); // CRC32
}

// --------------------------------------------------------------------------
// Video track
// --------------------------------------------------------------------------

MPEGPSDecoder::MPEGVideoTrack::MPEGVideoTrack(Common::SeekableReadStream *firstPacket, const Graphics::PixelFormat &format) {
	_surface = 0;
	_endOfTrack = false;
	_curFrame = -1;
	_framePts = 0xFFFFFFFF;
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
	if (pts != 0xFFFFFFFF) {
		_framePts = pts;
	}

	uint32 framePeriod;
	bool foundFrame = _mpegDecoder->decodePacket(*packet, framePeriod, _surface);

	if (foundFrame) {
		_curFrame++;

		// If there has been a timestamp since the previous frame, use that for
		// syncing. Usually it will be the timestamp from the current packet,
		// but it might not be.

		if (_framePts != 0xFFFFFFFF) {
			_nextFrameStartTime = Audio::Timestamp(_framePts / 90, 27000000);
		} else {
			_nextFrameStartTime = _nextFrameStartTime.addFrames(framePeriod);
		}

		_framePts = 0xFFFFFFFF;
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

// --------------------------------------------------------------------------
// Audio track
// --------------------------------------------------------------------------

#ifdef USE_MAD

// The audio code here is almost entirely based on what we do in mp3.cpp

MPEGPSDecoder::MPEGAudioTrack::MPEGAudioTrack(Common::SeekableReadStream &firstPacket, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType) {
	_audStream = Audio::makePacketizedMP3Stream(firstPacket);
}

MPEGPSDecoder::MPEGAudioTrack::~MPEGAudioTrack() {
	delete _audStream;
}

bool MPEGPSDecoder::MPEGAudioTrack::sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) {
	_audStream->queuePacket(packet);
	return true;
}

Audio::AudioStream *MPEGPSDecoder::MPEGAudioTrack::getAudioStream() const {
	return _audStream;
}

#endif

#ifdef USE_A52

MPEGPSDecoder::AC3AudioTrack::AC3AudioTrack(Common::SeekableReadStream &firstPacket, double decibel, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType) {
	_audStream = Audio::makeAC3Stream(firstPacket, decibel);
	if (!_audStream)
		error("Could not create AC-3 stream");
}

MPEGPSDecoder::AC3AudioTrack::~AC3AudioTrack() {
	delete _audStream;
}

bool MPEGPSDecoder::AC3AudioTrack::sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) {
	// Skip DVD code
	packet->readUint32LE();
	if (packet->eos())
		return true;

	_audStream->queuePacket(packet);
	return true;
}

Audio::AudioStream *MPEGPSDecoder::AC3AudioTrack::getAudioStream() const {
	return _audStream;
}

#endif

MPEGPSDecoder::PS2AudioTrack::PS2AudioTrack(Common::SeekableReadStream *firstPacket, Audio::Mixer::SoundType soundType) :
		AudioTrack(soundType) {
	firstPacket->seek(12); // unknown data (4), 'SShd', header size (4)

	_soundType = firstPacket->readUint32LE();

	if (_soundType == PS2_ADPCM)
		error("Unhandled PS2 ADPCM sound in MPEG-PS video");
	else if (_soundType != PS2_PCM)
		error("Unknown PS2 sound type %x", _soundType);

	uint32 sampleRate = firstPacket->readUint32LE();
	_channels = firstPacket->readUint32LE();
	_interleave = firstPacket->readUint32LE();

	byte flags = Audio::FLAG_16BITS | Audio::FLAG_LITTLE_ENDIAN;
	if (_channels == 2)
		flags |= Audio::FLAG_STEREO;

	_blockBuffer = new byte[_interleave * _channels];
	_blockPos = _blockUsed = 0;
	_audStream = Audio::makePacketizedRawStream(sampleRate, flags);
	_isFirstPacket = true;

	firstPacket->seek(0);
}

MPEGPSDecoder::PS2AudioTrack::~PS2AudioTrack() {
	delete[] _blockBuffer;
	delete _audStream;
}

bool MPEGPSDecoder::PS2AudioTrack::sendPacket(Common::SeekableReadStream *packet, uint32 pts, uint32 dts) {
	packet->skip(4);

	if (_isFirstPacket) {
		// Skip over the header which we already parsed
		packet->skip(4);
		packet->skip(packet->readUint32LE());

		if (packet->readUint32BE() != MKTAG('S', 'S', 'b', 'd'))
			error("Failed to find 'SSbd' tag");

		packet->readUint32LE(); // body size
		_isFirstPacket = false;
	}

	uint32 size = packet->size() - packet->pos();
	uint32 bytesPerChunk = _interleave * _channels;
	uint32 sampleCount = calculateSampleCount(size);

	byte *buffer = (byte *)malloc(sampleCount * 2);
	int16 *ptr = (int16 *)buffer;

	// Handle any full chunks first
	while (size >= bytesPerChunk) {
		packet->read(_blockBuffer + _blockPos, bytesPerChunk - _blockPos);
		size -= bytesPerChunk - _blockPos;
		_blockPos = 0;

		for (uint32 i = _blockUsed; i < _interleave / 2; i++)
			for (uint32 j = 0; j < _channels; j++)
				*ptr++ = READ_UINT16(_blockBuffer + i * 2 + j * _interleave);

		_blockUsed = 0;
	}

	// Then fallback on loading any leftover
	if (size > 0) {
		packet->read(_blockBuffer, size);
		_blockPos = size;

		if (size > (_channels - 1) * _interleave) {
			_blockUsed = (size - (_channels - 1) * _interleave) / 2;

			for (uint32 i = 0; i < _blockUsed; i++)
				for (uint32 j = 0; j < _channels; j++)
					*ptr++ = READ_UINT16(_blockBuffer + i * 2 + j * _interleave);
		}
	}

	_audStream->queuePacket(new Common::MemoryReadStream(buffer, sampleCount * 2, DisposeAfterUse::YES));

	delete packet;
	return true;
}

Audio::AudioStream *MPEGPSDecoder::PS2AudioTrack::getAudioStream() const {
	return _audStream;
}

uint32 MPEGPSDecoder::PS2AudioTrack::calculateSampleCount(uint32 packetSize) const {
	uint32 bytesPerChunk = _interleave * _channels, result = 0;

	// If we have a partial block, subtract the remainder from the size. That
	// gets put towards reading the partial block
	if (_blockPos != 0) {
		packetSize -= bytesPerChunk - _blockPos;
		result += (_interleave / 2) - _blockUsed;
	}

	// Round the number of whole chunks down and then calculate how many samples that gives us
	result += (packetSize / bytesPerChunk) * _interleave / 2;

	// Total up anything we can get from the remainder
	packetSize %= bytesPerChunk;
	if (packetSize > (_channels - 1) * _interleave)
		result += (packetSize - (_channels - 1) * _interleave) / 2;

	return result * _channels;
}
} // End of namespace Video
