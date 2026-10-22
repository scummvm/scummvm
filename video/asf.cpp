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

/*
 * ASF demuxer.
 *
 * This is a ScummVM port of FFmpeg's libavformat/asfdec_f.c at revision
 * c23123630e6a7e645c199599b8ade3fe7e9ab3db:
 * https://git.ffmpeg.org/gitweb/ffmpeg.git/commit/c23123630e6a7e645c199599b8ade3fe7e9ab3db
 * The original code was Copyright (c) 2000, 2001 Fabrice Bellard and
 * distributed under the GNU LGPL version 2.1 or later. The migrated path
 * includes stream properties, payload extensions, packet synchronization,
 * fragmented and compressed payloads, timestamps and audio descrambling.
 * Audio handling is based on xoreos' ASF code, in turn based on FFmpeg.
 */

#include "video/asf.h"
#include "audio/decoders/wave_types.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Video {

static const byte kHeader[16] = {0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C};
static const byte kFile[16] = {0xA1, 0xDC, 0xAB, 0x8C, 0x47, 0xA9, 0xCF, 0x11, 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65};
static const byte kStream[16] = {0x91, 0x07, 0xDC, 0xB7, 0xB7, 0xA9, 0xCF, 0x11, 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65};
static const byte kDataGuid[16] = {0x36, 0x26, 0xb2, 0x75, 0x8E, 0x66, 0xCF, 0x11, 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c};
static const byte kAudioGuid[16] = {0x40, 0x9E, 0x69, 0xF8, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B};
static const byte kVideoGuid[16] = {0xC0, 0xEF, 0x19, 0xBC, 0x4D, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B};
static const byte kExtendedStream[16] = {0xCB, 0xA5, 0xE6, 0x14, 0x72, 0xC6, 0x32, 0x43, 0x83, 0x99, 0xA9, 0x69, 0x52, 0x06, 0x5B, 0x5A};
static const byte kHeaderExtension[16] = {0xb5, 0x03, 0xbf, 0x5f, 0x2E, 0xA9, 0xCF, 0x11, 0x8e, 0xe3, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65};
static const byte kCommand[16] = {0xC0, 0xCF, 0xDA, 0x59, 0xE6, 0x59, 0xD0, 0x11, 0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6};
static const byte kJFIF[16] = {0x00, 0xE1, 0x1B, 0xB6, 0x4E, 0x5B, 0xCF, 0x11, 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B};
static const byte kEmbeddedStream[16] = {0xe2, 0x65, 0xfb, 0x3a, 0xEF, 0x47, 0xF2, 0x40, 0xac, 0x2c, 0x70, 0xa9, 0x0d, 0x71, 0xd3, 0x43};
static const byte kEmbeddedAudio[16] = {0x9d, 0x8c, 0x17, 0x31, 0xE1, 0x03, 0x28, 0x45, 0xb5, 0x82, 0x3d, 0xf9, 0xdb, 0x22, 0xf5, 0x03};

struct ASFDemuxer::Context {
	struct Payload {
		byte type;
		uint16 size;
	};
	struct StreamState {
		int index = -1;
		Object object;
		uint32 fragOffset = 0;
		uint32 objectSize = 0;
		bool clean = false;
		bool discardZeroMPEG2 = false, paletteChanged = false;
		Common::Array<uint32> palette;
		uint8 dsSpan = 0;
		uint16 dsPacketSize = 0, dsChunkSize = 0;
		uint64 frameTime = 0;
		Common::Array<Payload> payloads;
	};
	struct PacketState {
		int sizeLeft = 0, flags = 0, property = 0, timestamp = 0;
		int segSizeType = 0, segments = 0, seq = 0, replicaSize = 0;
		int keyFrame = 0, padding = 0, multiSize = 0, timeDelta = 0;
		uint32 fragOffset = 0, fragSize = 0;
		int64 fragTimestamp = 0, timeStart = 0, position = 0;
	};

	Common::SeekableReadStream *input = nullptr;
	Common::Array<Stream> streams;
	StreamState states[128];
	StreamState *current = nullptr;
	PacketState packet;
	uint32 minPacketSize = 0, maxPacketSize = 0, flags = 0;
	uint64 dataStart = 0, dataEnd = 0, duration = 0, sendDuration = 0;
	uint32 preroll = 0;
	int usesStdECC = 0, streamID = 0;
	bool timestampIsPTS = false, eof = false;

	bool readHeader();
	bool readStream(uint64 end);
	bool readExtendedStream(uint64 end);
	bool getPacket();
	bool readFrameHeader();
	int parsePacket(Object &object);
	bool atEnd() const { return input->eos() || input->pos() >= input->size(); }
	uint32 readValue(int bits, uint32 fallback, int &size);
	bool skip(int64 amount) {
		int64 position = input->pos();
		if (amount < -position)
			return false;
		return seek(MIN<uint64>(position + amount, input->size()));
	}
	bool seek(uint64 position) { return position <= (uint64)input->size() && input->seek(position); }
};

uint32 ASFDemuxer::Context::readValue(int bits, uint32 fallback, int &size) {
	switch (bits & 3) {
	case 3: size += 4; return input->readUint32LE();
	case 2: size += 2; return input->readUint16LE();
	case 1: ++size; return input->readByte();
	default: return fallback;
	}
}

bool ASFDemuxer::Context::readHeader() {
	byte guid[16];
	if (input->read(guid, 16) != 16 || memcmp(guid, kHeader, 16))
		return false;
	skip(14);
	bool haveFile = false;
	while (input->size() - input->pos() >= 24) {
		uint64 start = input->pos();
		input->read(guid, 16);
		uint64 size = input->readUint64LE();
		if (!memcmp(guid, kDataGuid, 16)) {
			if (!haveFile || streams.empty() || input->size() - input->pos() < 26)
				return false;
			dataEnd = (!(flags & 1) && size >= 100 && size <= (uint64)-1 - start)
				? start + size : (uint64)input->size();
			skip(26);
			dataStart = input->pos();
			return true;
		}
		if (size < 24 || size > (uint64)input->size() - start)
			return false;
		uint64 end = start + size;
		if (!memcmp(guid, kFile, 16)) {
			if (size < 104)
				return false;
			skip(40);
			uint64 playTime = input->readUint64LE();
			sendDuration = input->readUint64LE() / 10000;
			preroll = input->readUint32LE();
			input->readUint32LE();
			flags = input->readUint32LE();
			minPacketSize = input->readUint32LE();
			maxPacketSize = input->readUint32LE();
			if (minPacketSize >= (1U << 29))
				return false;
			duration = playTime / 10000 > preroll ? playTime / 10000 - preroll : 0;
			haveFile = true;
		} else if (!memcmp(guid, kStream, 16)) {
			if (!readStream(end))
				return false;
		} else if (!memcmp(guid, kExtendedStream, 16)) {
			if (!readExtendedStream(end))
				return false;
			continue;
		} else if (!memcmp(guid, kHeaderExtension, 16)) {
			if (end - input->pos() < 22)
				return false;
			skip(22);
			continue;
		}
		if (!seek(end))
			return false;
	}
	return false;
}

bool ASFDemuxer::Context::readStream(uint64 end) {
	if (end - input->pos() < 54 || streams.size() == 127)
		return false;
	Stream stream;
	byte guid[16];
	input->read(guid, 16);
	bool embedded = !memcmp(guid, kEmbeddedStream, 16);
	if (!memcmp(guid, kAudioGuid, 16))
		stream.type = kAudio;
	else if (!memcmp(guid, kVideoGuid, 16) || !memcmp(guid, kJFIF, 16))
		stream.type = kVideo;
	else if (!memcmp(guid, kCommand, 16))
		stream.type = kData;
	else if (!embedded)
		return false;
	skip(16);
	stream.timeOffset = input->readUint64LE();
	uint32 typeSize = input->readUint32LE();
	input->readUint32LE();
	stream.id = input->readUint16LE() & 0x7f;
	skip(4);
	if (embedded) {
		if (end - input->pos() < 16)
			return false;
		input->read(guid, 16);
		if (!memcmp(guid, kEmbeddedAudio, 16)) {
			if (end - input->pos() < 48)
				return false;
			stream.type = kAudio;
			skip(48);
		}
	}
	if (stream.type == kVideo && end - input->pos() >= 51) {
		stream.typeData.resize(51);
		if (input->read(stream.typeData.data(), 51) != 51)
			return false;
		uint32 bitmapSize = READ_LE_UINT32(stream.typeData.data() + 11);
		uint32 extra = bitmapSize > 40 ? bitmapSize - 40 : 0;
		if (extra > end - input->pos())
			return false;
		stream.typeData.resize(51 + extra);
		if (input->read(stream.typeData.data() + 51, extra) != extra)
			return false;
	} else {
		if (typeSize > end - input->pos())
			return false;
		stream.typeData.resize(typeSize);
		if (input->read(stream.typeData.data(), typeSize) != typeSize)
			return false;
	}
	uint32 errorSize = stream.type == kAudio && end - input->pos() >= 8 ? 8 : 0;
	stream.errorData.resize(errorSize);
	if (input->read(stream.errorData.data(), errorSize) != errorSize)
		return false;
	StreamState &state = states[stream.id];
	state.index = streams.size();
	stream.frameTime = state.frameTime;
	if (stream.type == kAudio && errorSize >= 8) {
		state.dsSpan = stream.errorData[0];
		state.dsPacketSize = READ_LE_UINT16(stream.errorData.data() + 1);
		state.dsChunkSize = READ_LE_UINT16(stream.errorData.data() + 3);
		if (state.dsSpan > 1 && (!state.dsChunkSize || state.dsPacketSize / state.dsChunkSize <= 1 ||
		                        state.dsPacketSize % state.dsChunkSize))
			state.dsSpan = 0;
	}
	if (stream.type == kVideo && stream.typeData.size() >= 51) {
		const byte *bitmap = stream.typeData.data() + 11;
		uint32 tag = READ_BE_UINT32(bitmap + 16);
		switch (tag) {
		case MKTAG('m', 'p', 'g', '2'):
		case MKTAG('M', 'P', 'E', 'G'):
		case MKTAG('P', 'I', 'M', '2'):
		case MKTAG('V', 'C', 'R', '2'):
		case MKTAG( 2 ,  0 ,  0 ,  16):
		case MKTAG('D', 'V', 'R', ' '):
		case MKTAG('M', 'M', 'E', 'S'):
		case MKTAG('L', 'M', 'P', '2'):
		case MKTAG('s', 'l', 'i', 'f'):
		case MKTAG('E', 'M', '2', 'V'):
		case MKTAG('M', '7', '0', '1'):
		case MKTAG('M', '7', '0', '2'):
		case MKTAG('M', '7', '0', '3'):
		case MKTAG('M', '7', '0', '4'):
		case MKTAG('M', '7', '0', '5'):
		case MKTAG('m', 'p', 'g', 'v'):
			state.discardZeroMPEG2 = true;
			break;
		default:
			break;
		}
		uint32 bitmapSize = READ_LE_UINT32(bitmap);
		uint16 bits = READ_LE_UINT16(bitmap + 14);
		if (bitmapSize > 40 && bits <= 8) {
			uint32 bytes = MIN<uint32>(MIN<uint32>(bitmapSize - 40, stream.typeData.size() - 51), 1024);
			state.palette.resize(256);
			for (uint i = 0; i < bytes / 4; ++i)
				state.palette[i] = READ_LE_UINT32(bitmap + 40 + i * 4);
			state.paletteChanged = true;
		}
	}
	streams.push_back(stream);
	return true;
}

bool ASFDemuxer::Context::readExtendedStream(uint64 end) {
	if (end - input->pos() < 64)
		return false;
	skip(48);
	uint16 id = input->readUint16LE();
	input->readUint16LE();
	uint64 frameTime = input->readUint64LE();
	uint16 names = input->readUint16LE();
	uint16 extensions = input->readUint16LE();
	if (id < 128) {
		states[id].frameTime = frameTime;
		states[id].payloads.clear();
		if (states[id].index >= 0)
			streams[states[id].index].frameTime = frameTime;
	}
	for (uint i = 0; i < names; ++i) {
		if (end - input->pos() < 4)
			return false;
		input->readUint16LE();
		uint16 length = input->readUint16LE();
		if (length > end - input->pos())
			return false;
		skip(length);
	}
	for (uint i = 0; i < extensions; ++i) {
		if (end - input->pos() < 22)
			return false;
		byte guid[16];
		input->read(guid, 16);
		Payload payload = {guid[0], input->readUint16LE()};
		uint32 length = input->readUint32LE();
		if (length > end - input->pos())
			return false;
		skip(length);
		if (id < 128 && i < 8)
			states[id].payloads.push_back(payload);
	}
	return true;
}

bool ASFDemuxer::Context::getPacket() {
	int size = 8;
	int c = -1, d = -1, e = -1;
	if (usesStdECC > 0) {
		int left = 32768;
		while (left-- > 0) {
			c = d;
			d = e;
			e = input->readByte();
			if (c == 0x82 && !d && !e)
				break;
		}
		if ((c & 0x8f) == 0x82) {
			if (d || e)
				return false;
			c = input->readByte();
			d = input->readByte();
			size += 3;
		} else if (!atEnd()) {
			skip(-1);
		}
	} else {
		c = input->readByte();
		if (c & 0x80) {
			++size;
			if (!(c & 0x60)) {
				d = input->readByte();
				e = input->readByte();
				skip((c & 0xf) - 2);
				size += c & 0xf;
			}
			if (!usesStdECC)
				usesStdECC = (c == 0x82 && !d && !e) ? 1 : -1;
			c = input->readByte();
		} else {
			usesStdECC = -1;
		}
		d = input->readByte();
	}
	packet.flags = c;
	packet.property = d;
	uint32 length = readValue(packet.flags >> 5, maxPacketSize, size);
	readValue(packet.flags >> 1, 0, size);
	uint32 padding = readValue(packet.flags >> 3, 0, size);
	if (!length || length >= (1U << 29) || padding >= length)
		return false;
	packet.timestamp = input->readUint32LE();
	input->readUint16LE();
	if (packet.flags & 1) {
		packet.segSizeType = input->readByte();
		++size;
		packet.segments = packet.segSizeType & 0x3f;
	} else {
		packet.segments = 1;
		packet.segSizeType = 0x80;
	}
	if ((uint32)size > length - padding) {
		packet.sizeLeft = 0;
		return false;
	}
	packet.sizeLeft = length - padding - size;
	if (length < minPacketSize)
		padding += minPacketSize - length;
	packet.padding = padding;
	return true;
}

bool ASFDemuxer::Context::readFrameHeader() {
	int size = 1;
	int num = input->readByte();
	--packet.segments;
	packet.keyFrame = num >> 7;
	streamID = num & 0x7f;
	StreamState &state = states[streamID];
	packet.seq = readValue(packet.property >> 4, 0, size);
	packet.fragOffset = readValue(packet.property >> 2, 0, size);
	packet.replicaSize = readValue(packet.property, 0, size);
	if (size + (int64)packet.replicaSize > packet.sizeLeft)
		return false;
	if (packet.replicaSize >= 8) {
		uint64 end = input->pos() + packet.replicaSize;
		state.objectSize = input->readUint32LE();
		if (state.objectSize >= (1U << 24)) {
			state.objectSize = 0;
			return false;
		}
		packet.fragTimestamp = input->readUint32LE();
		for (const auto &payload : state.payloads) {
			int length = payload.size;
			if (length == 0xffff)
				length = input->readUint16LE();
			uint64 payloadEnd = input->pos() + length;
			if (payloadEnd > end)
				break;
			switch (payload.type) {
			case 0x54: {
				int aspectNum = input->readByte();
				int den = input->readByte();
				if (aspectNum > 0 && den > 0 && state.index >= 0) {
					streams[state.index].aspectNum = aspectNum;
					streams[state.index].aspectDen = den;
				}
				break;
			}
			case 0x2a: {
				skip(8);
				int64 ts = input->readSint64LE();
				input->readUint64LE();
				packet.fragTimestamp = ts != -1 ? ts / 10000 : kNoTimestamp;
				timestampIsPTS = true;
				break;
			}
			default:
				break;
			}
			if (!seek(payloadEnd))
				return false;
		}
		if (!seek(end))
			return false;
		size += packet.replicaSize;
	} else if (packet.replicaSize == 1) {
		packet.timeStart = packet.fragOffset;
		packet.fragOffset = 0;
		packet.fragTimestamp = packet.timestamp;
		packet.timeDelta = input->readByte();
		++size;
	} else if (packet.replicaSize != 0) {
		return false;
	}
	if (packet.flags & 1) {
		packet.fragSize = readValue(packet.segSizeType >> 6, 0, size);
		if (size > packet.sizeLeft)
			return false;
		if (packet.fragSize > (uint32)(packet.sizeLeft - size)) {
			if (packet.fragSize > (uint32)(packet.sizeLeft - size + packet.padding))
				return false;
			int diff = packet.fragSize - (packet.sizeLeft - size);
			packet.sizeLeft += diff;
			packet.padding -= diff;
		}
	} else {
		packet.fragSize = packet.sizeLeft - size;
	}
	if (packet.replicaSize == 1) {
		packet.multiSize = packet.fragSize;
		if (packet.multiSize > packet.sizeLeft)
			return false;
	}
	packet.sizeLeft -= size;
	return true;
}

int ASFDemuxer::Context::parsePacket(Object &object) {
	for (;;) {
		if (atEnd())
			return -1;
		if (packet.sizeLeft < 6 || (packet.segments < 1 && packet.timeStart == 0)) {
			int remaining = packet.sizeLeft + packet.padding;
			if (remaining < 0 || !skip(remaining))
				return -1;
			packet.position = input->pos();
			if ((uint64)packet.position >= dataEnd)
				return -1;
			return 1;
		}
		if (packet.timeStart == 0) {
			if (!readFrameHeader()) {
				packet.timeStart = packet.segments = 0;
				continue;
			}
			if (states[streamID].index < 0) {
				packet.timeStart = 0;
				skip(packet.fragSize);
				packet.sizeLeft -= packet.fragSize;
				continue;
			}
			current = &states[streamID];
		}
		StreamState &state = *current;
		if (!state.fragOffset && packet.fragOffset) {
			skip(packet.fragSize);
			packet.sizeLeft -= packet.fragSize;
			continue;
		}
		if (packet.replicaSize == 1) {
			packet.fragTimestamp = packet.timeStart;
			packet.timeStart += packet.timeDelta;
			state.objectSize = packet.fragSize = input->readByte();
			--packet.sizeLeft;
			--packet.multiSize;
			if (packet.multiSize < (int)state.objectSize) {
				packet.timeStart = 0;
				skip(packet.multiSize);
				packet.sizeLeft -= packet.multiSize;
				continue;
			}
			packet.multiSize -= state.objectSize;
		}
		if (state.object.data.size() != state.objectSize ||
		    state.fragOffset + packet.fragSize > state.object.data.size()) {
			if (!state.object.data.empty()) {
				state.fragOffset = 0;
				state.object = Object();
			}
			state.object.data.resize(state.objectSize);
			state.object.sequence = packet.seq;
			if (packet.fragTimestamp != kNoTimestamp) {
				if (timestampIsPTS)
					state.object.pts = packet.fragTimestamp - preroll;
				else
					state.object.dts = packet.fragTimestamp - preroll;
			}
			state.object.stream = streams[state.index].id;
			state.object.position = packet.position;
			state.clean = false;
			if (state.paletteChanged) {
				state.object.palette = state.palette;
				state.paletteChanged = false;
			}
			if (streams[state.index].type == kAudio)
				packet.keyFrame = 1;
			state.object.keyframe = packet.keyFrame != 0;
		}
		packet.sizeLeft -= packet.fragSize;
		if (packet.sizeLeft < 0)
			continue;
		if (packet.fragOffset >= state.object.data.size() ||
		    packet.fragSize > state.object.data.size() - packet.fragOffset)
			continue;
		if (packet.fragOffset != state.fragOffset && !state.clean) {
			memset(state.object.data.data() + state.fragOffset, 0, state.object.data.size() - state.fragOffset);
			state.clean = true;
		}
		uint32 read = input->read(state.object.data.data() + packet.fragOffset, packet.fragSize);
		if (read != packet.fragSize) {
			if (packet.fragOffset + read == 0)
				return -1;
			if (state.dsSpan > 1) {
				memset(state.object.data.data() + packet.fragOffset + read, 0, packet.fragSize - read);
				read = packet.fragSize;
			} else {
				state.object.data.resize(packet.fragOffset + read);
			}
		}
		state.fragOffset += read;
		if (state.fragOffset != state.object.data.size())
			continue;
		if (state.discardZeroMPEG2 && state.object.data.size() > 100) {
			uint i = 0;
			while (i < state.object.data.size() && !state.object.data[i])
				++i;
			if (i == state.object.data.size()) {
				state.fragOffset = 0;
				state.object = Object();
				continue;
			}
		}
		if (state.dsSpan > 1 && state.object.data.size() == (uint32)state.dsPacketSize * state.dsSpan) {
			Common::Array<byte> descrambled;
			descrambled.resize(state.object.data.size());
			for (uint offset = 0; offset < descrambled.size(); offset += state.dsChunkSize) {
				uint off = offset / state.dsChunkSize;
				uint row = off / state.dsSpan;
				uint col = off % state.dsSpan;
				uint index = row + col * state.dsPacketSize / state.dsChunkSize;
				memcpy(descrambled.data() + offset, state.object.data.data() + index * state.dsChunkSize, state.dsChunkSize);
			}
			state.object.data = descrambled;
		}
		state.fragOffset = 0;
		object = state.object;
		state.object = Object();
		return 0;
	}
}

ASFDemuxer::ASFDemuxer() : _ctx(new Context()) {}
ASFDemuxer::~ASFDemuxer() { delete _ctx; }

void ASFDemuxer::close() {
	delete _ctx;
	_ctx = new Context();
}

bool ASFDemuxer::open(Common::SeekableReadStream *stream) {
	close();
	_ctx->input = stream;
	if (!stream || !stream->seek(0) || !_ctx->readHeader()) {
		close();
		return false;
	}
	return true;
}

bool ASFDemuxer::rewind() {
	if (!_ctx->input || !_ctx->input->seek(_ctx->dataStart))
		return false;
	_ctx->input->clearErr();
	_ctx->packet = Context::PacketState();
	_ctx->current = nullptr;
	_ctx->eof = false;
	for (auto &state : _ctx->states) {
		state.object = Object();
		state.objectSize = state.fragOffset = 0;
	}
	return true;
}

ASFDemuxer::ReadResult ASFDemuxer::readObject(Object &object) {
	if (!_ctx->input)
		return kError;
	for (;;) {
		int result = _ctx->parsePacket(object);
		if (!result)
			return kObject;
		if (result < 0) {
			_ctx->eof = true;
			return _ctx->input->err() ? kError : kEnd;
		}
		int64 before = _ctx->input->pos();
		bool ok = _ctx->getPacket();
		_ctx->packet.timeStart = 0;
		if (!ok && _ctx->input->pos() == before && !_ctx->atEnd())
			return kError;
	}
}

bool ASFDemuxer::endOfData() const {
	if (!_ctx->input || _ctx->eof)
		return true;
	const Context::PacketState &p = _ctx->packet;
	return (p.sizeLeft < 6 || (p.segments < 1 && !p.timeStart)) &&
		(uint64)(_ctx->input->pos() + p.sizeLeft + p.padding) >= MIN<uint64>(_ctx->dataEnd, _ctx->input->size());
}

const Common::Array<ASFDemuxer::Stream> &ASFDemuxer::getStreams() const { return _ctx->streams; }
uint64 ASFDemuxer::getDuration() const { return _ctx->duration; }
uint64 ASFDemuxer::getSendDuration() const { return _ctx->sendDuration; }
uint64 ASFDemuxer::getPreroll() const { return _ctx->preroll; }

bool ASFAudioCodec::open(const ASFDemuxer::Stream &stream) {
	if (stream.type != ASFDemuxer::kAudio || stream.typeData.size() < 14)
		return false;
	Common::MemoryReadStream format(stream.typeData.data(), stream.typeData.size());
	uint16 compression = format.readUint16LE();
	_channels = format.readUint16LE();
	_rate = format.readUint32LE();
	uint32 bitRate = format.readUint32LE() * 8;
	uint16 blockAlign = format.readUint16LE();
	if (compression != Audio::kWaveFormatWMAv2 || !_channels || _channels > 2 || !_rate || _rate > 50000)
		return false;
	if (stream.typeData.size() != 14)
		format.readUint16LE();
	Common::SeekableReadStream *extra = nullptr;
	if (stream.typeData.size() >= 18) {
		uint32 size = MIN<uint32>(format.readUint16LE(), stream.typeData.size() - 18);
		extra = format.readStream(size);
	}
	delete _codec;
	_codec = new Audio::WMACodec(2, _rate, _channels, bitRate, blockAlign, extra);
	delete extra;
	return true;
}

Audio::AudioStream *ASFAudioCodec::decode(const ASFDemuxer::Object &object) {
	Common::MemoryReadStream packet(object.data.data(), object.data.size());
	return _codec->decodeFrame(packet);
}

class ASFAudioStream : public Audio::SeekableAudioStream {
public:
	ASFAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag dispose)
		: _input(stream), _dispose(dispose), _current(nullptr), _id(0), _ended(false) {}
	~ASFAudioStream() override {
		delete _current;
		if (_dispose == DisposeAfterUse::YES)
			delete _input;
	}
	bool open() {
		if (!_demux.open(_input))
			return false;
		for (const auto &stream : _demux.getStreams()) {
			if (stream.type == ASFDemuxer::kAudio) {
				_id = stream.id;
				return _codec.open(stream) && !_demux.endOfData();
			}
		}
		return false;
	}
	int readBuffer(int16 *buffer, int count) override {
		int read = 0;
		while (read < count && !endOfData()) {
			if (_current) {
				read += _current->readBuffer(buffer + read, count - read);
				if (_current->endOfData()) {
					delete _current;
					_current = nullptr;
				}
			} else {
				ASFDemuxer::Object object;
				ASFDemuxer::ReadResult result = _demux.readObject(object);
				if (result != ASFDemuxer::kObject) {
					_ended = true;
					if (result == ASFDemuxer::kError)
						warning("ASF: Invalid audio packet");
				} else if (object.stream == _id) {
					_current = _codec.decode(object);
				}
			}
		}
		return read;
	}
	bool endOfData() const override { return !_current && (_ended || _demux.endOfData()); }
	bool isStereo() const override { return _codec.isStereo(); }
	int getRate() const override { return _codec.getRate(); }
	Audio::Timestamp getLength() const override { return Audio::Timestamp(_demux.getSendDuration(), getRate()); }
	bool seek(const Audio::Timestamp &time) override { return time == 0 && rewind(); }
	bool rewind() override {
		if (!_demux.rewind())
			return false;
		delete _current;
		_current = nullptr;
		_ended = false;
		return true;
	}
private:
	Common::SeekableReadStream *_input;
	DisposeAfterUse::Flag _dispose;
	ASFDemuxer _demux;
	ASFAudioCodec _codec;
	Audio::AudioStream *_current;
	uint8 _id;
	bool _ended;
};

Audio::SeekableAudioStream *makeASFAudioStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	ASFAudioStream *audio = new ASFAudioStream(stream, disposeAfterUse);
	if (!audio->open()) {
		delete audio;
		return nullptr;
	}
	return audio;
}

} // End of namespace Video
