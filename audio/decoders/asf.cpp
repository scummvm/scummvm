/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

// Based on xoreos' ASF code which is in turn
// Largely based on the ASF implementation found in FFmpeg.

#include "common/textconsole.h"
#include "common/stream.h"
#include "common/util.h"

#include "audio/audiostream.h"
#include "audio/decoders/asf.h"
#include "audio/decoders/wma.h"
#include "audio/decoders/wave_types.h"

namespace Audio {

class ASFGUID {
public:
	ASFGUID(Common::SeekableReadStream &stream) {
		stream.read(id, 16);
	}

	ASFGUID(byte a0, byte a1, byte a2, byte a3, byte a4, byte a5, byte a6, byte a7, byte a8, byte a9, byte a10, byte a11, byte a12, byte a13, byte a14, byte a15) {
		id[0]  = a0;   id[1]  = a1;  id[2]  = a2;  id[3]  = a3;
		id[4]  = a4;   id[5]  = a5;  id[6]  = a6;  id[7]  = a7;
		id[8]  = a8;   id[9]  = a9;  id[10] = a10; id[11] = a11;
		id[12] = a12;  id[13] = a13; id[14] = a14; id[15] = a15;
	}

	bool operator==(const ASFGUID &g) const {
		return !memcmp(g.id, id, 16);
	}

	bool operator!=(const ASFGUID &g) const {
		return memcmp(g.id, id, 16);
	}

	Common::String toString() const {
		return Common::String::format("%02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x",
				id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9], id[10], id[11], id[12], id[13], id[14], id[15]);
	}

private:
	byte id[16];
};

// GUID's that we need
static const ASFGUID s_asfHeader         = ASFGUID(0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C);
static const ASFGUID s_asfFileHeader     = ASFGUID(0xA1, 0xDC, 0xAB, 0x8C, 0x47, 0xA9, 0xCF, 0x11, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65);
static const ASFGUID s_asfHead1          = ASFGUID(0xb5, 0x03, 0xbf, 0x5f, 0x2E, 0xA9, 0xCF, 0x11, 0x8e, 0xe3, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65);
static const ASFGUID s_asfComment        = ASFGUID(0x33, 0x26, 0xb2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c);
static const ASFGUID s_asfStreamHeader   = ASFGUID(0x91, 0x07, 0xDC, 0xB7, 0xB7, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65);
static const ASFGUID s_asfCodecComment   = ASFGUID(0x40, 0x52, 0xD1, 0x86, 0x1D, 0x31, 0xD0, 0x11, 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6);
static const ASFGUID s_asfDataHeader     = ASFGUID(0x36, 0x26, 0xb2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c);
static const ASFGUID s_asfAudioStream    = ASFGUID(0x40, 0x9E, 0x69, 0xF8, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B);
static const ASFGUID s_asfExtendedHeader = ASFGUID(0x40, 0xA4, 0xD0, 0xD2, 0x07, 0xE3, 0xD2, 0x11, 0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50);
static const ASFGUID s_asfStreamBitRate  = ASFGUID(0xce, 0x75, 0xf8, 0x7b, 0x8d, 0x46, 0xd1, 0x11, 0x8d, 0x82, 0x00, 0x60, 0x97, 0xc9, 0xa2, 0xb2);

class ASFStream : public SeekableAudioStream {
public:
	ASFStream(Common::SeekableReadStream *stream,
			DisposeAfterUse::Flag disposeAfterUse);
	~ASFStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool endOfData() const;
	bool isStereo() const { return _channels == 2; }
	int getRate() const { return _sampleRate; }
	Timestamp getLength() const { return Audio::Timestamp(_duration / 10000, _sampleRate); }
	bool seek(const Timestamp &where);
	bool rewind();

private:
	// Packet data
	struct Packet {
		Packet();
		~Packet();

		byte flags;
		byte segmentType;
		uint16 packetSize;
		uint32 sendTime;
		uint16 duration;

		struct Segment {
			byte streamID;
			byte sequenceNumber;
			bool isKeyframe;
			Common::Array<Common::SeekableReadStream *> data;
		};

		Common::Array<Segment> segments;
	};

	Common::SeekableReadStream *_stream;
	DisposeAfterUse::Flag _disposeAfterUse;

	void parseStreamHeader();
	void parseFileHeader();
	Packet *readPacket();
	Codec *createCodec();
	AudioStream *createAudioStream();

	uint32 _rewindPos;
	uint64 _curPacket;
	Packet *_lastPacket;
	Codec *_codec;
	AudioStream *_curAudioStream;
	byte _curSequenceNumber;

	// Header object variables
	uint64 _packetCount;
	uint64 _duration;
	uint32 _minPacketSize, _maxPacketSize;

	// Stream object variables
	uint16 _streamID;
	uint16 _compression;
	uint16 _channels;
	int _sampleRate;
	uint32 _bitRate;
	uint16 _blockAlign;
	uint16 _bitsPerCodedSample;
	Common::SeekableReadStream *_extraData;
};

ASFStream::Packet::Packet() {
}

ASFStream::Packet::~Packet() {
	for (uint32 i = 0; i < segments.size(); i++)
		for (uint32 j = 0; j < segments[i].data.size(); j++)
				delete segments[i].data[j];
}

ASFStream::ASFStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) : _stream(stream), _disposeAfterUse(disposeAfterUse) {
	_extraData = 0;
	_lastPacket = 0;
	_curPacket = 0;
	_codec = 0;
	_curAudioStream = 0;
	_curSequenceNumber = 1; // They always start at one

	ASFGUID guid = ASFGUID(*_stream);
	if (guid != s_asfHeader)
		error("ASFStream: Missing asf header");

	_stream->readUint64LE();
	_stream->readUint32LE();
	_stream->readByte();
	_stream->readByte();

	for (;;) {
		uint64 startPos = _stream->pos();
		guid = ASFGUID(*_stream);
		uint64 size = _stream->readUint64LE();

		if (_stream->eos())
			error("ASFStream: Unexpected eos");

		// TODO: Parse each chunk
		if (guid == s_asfFileHeader) {
			parseFileHeader();
		} else if (guid == s_asfHead1) {
			// Should be safe to ignore
		} else if (guid == s_asfComment) {
			// Ignored
		} else if (guid == s_asfStreamHeader) {
			parseStreamHeader();
		} else if (guid == s_asfCodecComment) {
			// TODO
		} else if (guid == s_asfDataHeader) {
			// Done parsing the header
			break;
		} else if (guid == s_asfExtendedHeader) {
			// TODO
		} else if (guid == s_asfStreamBitRate) {
			// Ignored
		} else
			warning("Found unknown ASF GUID: %s", guid.toString().c_str());

		_stream->seek(startPos + size);
	}

	// Skip to the beginning of the packets
	_stream->skip(26);
	_rewindPos = _stream->pos();
}

ASFStream::~ASFStream() {
	if (_disposeAfterUse)
		delete _stream;

	delete _lastPacket;
	delete _curAudioStream;
	delete _codec;
}

void ASFStream::parseFileHeader() {
	_stream->skip(16); // skip client GUID
	/* uint64 fileSize = */ _stream->readUint64LE();
	/* uint64 creationTime = */ _stream->readUint64LE();
	_packetCount = _stream->readUint64LE();
	/* uint64 endTimestamp = */ _stream->readUint64LE();
	_duration = _stream->readUint64LE();
	/* uint32 startTimestamp = */ _stream->readUint32LE();
	/* uint32 unknown = */ _stream->readUint32LE();
	/* uint32 flags = */ _stream->readUint32LE();
	_minPacketSize = _stream->readUint32LE();
	_maxPacketSize = _stream->readUint32LE();
	/* uint32 uncFrameSize = */ _stream->readUint32LE();

	// We only know how to support packets of one length
	if (_minPacketSize != _maxPacketSize)
		error("ASFStream::parseFileHeader(): Mismatched packet sizes: Min = %d, Max = %d", _minPacketSize, _maxPacketSize);
}

void ASFStream::parseStreamHeader() {
	ASFGUID guid = ASFGUID(*_stream);

	if (guid != s_asfAudioStream)
		error("ASFStream::parseStreamHeader(): Found non-audio stream");

	_stream->skip(16); // skip a guid
	_stream->readUint64LE(); // total size
	uint32 typeSpecificSize = _stream->readUint32LE();
	_stream->readUint32LE();
	_streamID = _stream->readUint16LE();
	_stream->readUint32LE();

	// Parse the wave header
	_compression = _stream->readUint16LE();
	_channels = _stream->readUint16LE();
	_sampleRate = _stream->readUint32LE();
	_bitRate = _stream->readUint32LE() * 8;
	_blockAlign = _stream->readUint16LE();
	_bitsPerCodedSample = (typeSpecificSize == 14) ? 8 : _stream->readUint16LE();

	if (typeSpecificSize >= 18) {
		uint32 cbSize = _stream->readUint16LE();
		cbSize = MIN<int>(cbSize, typeSpecificSize - 18);
		_extraData = _stream->readStream(cbSize);
	}

	_codec = createCodec();
}

bool ASFStream::seek(const Timestamp &where) {
	if (where == 0) {
		return rewind();
	}

	// Seeking is not supported
	return false;
}

bool ASFStream::rewind() {
	// Seek back to the start of the packets
	_stream->seek(_rewindPos);

	// Reset our packet counter
	_curPacket = 0;
	delete _lastPacket; _lastPacket = 0;

	// Delete a stream if we have one
	delete _curAudioStream; _curAudioStream = 0;

	// Reset this too
	_curSequenceNumber = 1;

	return true;
}

ASFStream::Packet *ASFStream::readPacket() {
	if (_curPacket == _packetCount)
		error("ASFStream::readPacket(): Reading too many packets");

	uint32 packetStartPos = _stream->pos();

	// Read a single ASF packet
	if (_stream->readByte() != 0x82)
		error("ASFStream::readPacket(): Missing packet header");

	if (_stream->readUint16LE() != 0)
		error("ASFStream::readPacket(): Unknown is not zero");

	Packet *packet = new Packet();
	packet->flags = _stream->readByte();
	packet->segmentType = _stream->readByte();
	packet->packetSize = (packet->flags & 0x40) ? _stream->readUint16LE() : 0;

	uint16 paddingSize = 0;
	if (packet->flags & 0x10)
		paddingSize = _stream->readUint16LE();
	else if (packet->flags & 0x08)
		paddingSize = _stream->readByte();

	packet->sendTime = _stream->readUint32LE();
	packet->duration = _stream->readUint16LE();

	byte segmentCount = (packet->flags & 0x01) ? _stream->readByte() : 1;
	packet->segments.resize(segmentCount & 0x3F);

	for (uint32 i = 0; i < packet->segments.size(); i++) {
		Packet::Segment &segment = packet->segments[i];

		segment.streamID = _stream->readByte();
		segment.sequenceNumber = _stream->readByte();
		segment.isKeyframe = (segment.streamID & 0x80) != 0;
		segment.streamID &= 0x7F;

		uint32 fragmentOffset = 0;
		if (packet->segmentType == 0x55)
			fragmentOffset = _stream->readByte();
		else if (packet->segmentType == 0x59)
			fragmentOffset = _stream->readUint16LE();
		else if (packet->segmentType == 0x5D)
			fragmentOffset = _stream->readUint32LE();
		else
			error("ASFStream::readPacket(): Unknown packet segment type 0x%02x", packet->segmentType);

		byte flags = _stream->readByte();
		if (flags == 1) {
			//uint32 objectStartTime = fragmentOffset; // reused purpose
			_stream->readByte(); // unknown

			uint32 dataLength = (packet->segments.size() == 1) ? (_maxPacketSize - (_stream->pos() - packetStartPos) - paddingSize) : _stream->readUint16LE();
			uint32 startObjectPos = _stream->pos();

			while ((uint32)_stream->pos() < dataLength + startObjectPos)
				segment.data.push_back(_stream->readStream(_stream->readByte()));
		} else if (flags == 8) {
			/* uint32 objectLength = */ _stream->readUint32LE();
			/* uint32 objectStartTime = */ _stream->readUint32LE();

			uint32 dataLength = 0;
			if (packet->segments.size() == 1)
				dataLength = _maxPacketSize - (_stream->pos() - packetStartPos) - fragmentOffset - paddingSize;
			else if (segmentCount & 0x40)
				dataLength = _stream->readByte();
			else
				dataLength = _stream->readUint16LE();

			_stream->skip(fragmentOffset);
			segment.data.push_back(_stream->readStream(dataLength));
		} else
			error("ASFStream::readPacket(): Unknown packet flags 0x%02x", flags);
	}

	// Skip any padding
	_stream->skip(paddingSize);

	// We just read a packet
	_curPacket++;

	if ((uint32)_stream->pos() != packetStartPos + _maxPacketSize)
		error("ASFStream::readPacket(): Mismatching packet pos: %d (should be %d)", _stream->pos(), _maxPacketSize + packetStartPos);

	return packet;
}

Codec *ASFStream::createCodec() {
	switch (_compression) {
	case kWaveWMAv2:
		return new WMACodec(2, _sampleRate, _channels, _bitRate, _blockAlign, _extraData);
	default:
		error("ASFStream::createAudioStream(): Unknown compression 0x%04x", _compression);
	}

	return 0;
}

AudioStream *ASFStream::createAudioStream() {
	delete _lastPacket;
	_lastPacket = readPacket();

	// TODO
	if (_lastPacket->segments.size() != 1)
		error("ASFStream::createAudioStream(): Only single segment packets supported");

	Packet::Segment &segment = _lastPacket->segments[0];

	// We should only have one stream in a ASF audio file
	if (segment.streamID != _streamID)
		error("ASFStream::createAudioStream(): Packet stream ID mismatch");

	// TODO
	if (segment.sequenceNumber != _curSequenceNumber)
		error("ASFStream::createAudioStream(): Only one sequence number per packet supported");

	// This can overflow and needs to overflow!
	_curSequenceNumber++;

	// TODO
	if (segment.data.size() != 1)
		error("ASFStream::createAudioStream(): Packet grouping not supported");

	Common::SeekableReadStream *stream = segment.data[0];
	if (_codec)
		return _codec->decodeFrame(*stream);

	return 0;
}

int ASFStream::readBuffer(int16 *buffer, const int numSamples) {
	int samplesDecoded = 0;

	for (;;) {
		if (_curAudioStream) {
			samplesDecoded += _curAudioStream->readBuffer(buffer + samplesDecoded, numSamples - samplesDecoded);

			if (_curAudioStream->endOfData()) {
				delete _curAudioStream;
				_curAudioStream = 0;
			}
		}

		if (samplesDecoded == numSamples || endOfData())
			break;

		if (!_curAudioStream) {
			_curAudioStream = createAudioStream();
		}

	}

	return samplesDecoded;
}

bool ASFStream::endOfData() const {
	return _curPacket == _packetCount && !_curAudioStream;
}

SeekableAudioStream *makeASFStream(
	Common::SeekableReadStream *stream,
	DisposeAfterUse::Flag disposeAfterUse) {
	SeekableAudioStream *s = new ASFStream(stream, disposeAfterUse);

	if (s && s->endOfData()) {
		delete s;
		return 0;
	}

	return s;
}

} // End of namespace Audio
