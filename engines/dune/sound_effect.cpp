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
 */

#include "dune/sound_effect.h"
#include "common/endian.h"
#include "common/util.h"

namespace Dune {

// Creative 4-bit ADPCM, as also used by Sherlock's Sound::decodeSample.
// Command 75 consumes the reference byte without emitting it, then reads
// high nibble first. Each VOC type-1 block supplies a new reference/scale.
static int16 decodeNibble(byte nibble, int &reference, int &scale) {
	const int magnitude = nibble & 7;
	const int delta = (magnitude << scale) + (scale ? 1 << (scale - 1) : 0);
	reference = CLIP(reference + (nibble & 8 ? -delta : delta), 0, 255);
	if (magnitude >= 5)
		scale = MIN(scale + 1, 3);
	else if (!magnitude)
		scale = MAX(scale - 1, 0);
	return (reference - 128) * 256;
}

bool SoundEffectStream::load(const byte *data, uint32 size) {
	_blocks.clear();
	_loops.clear();
	_current = 0;
	_position = 0;
	_rate = 0;
	// These are the formats actually present in SN1..SNA, not a general VOC
	// decoder. Reject malformed programs rather than looping without samples.
	if (size < 26 || memcmp(data, "Creative Voice File\x1a", 20) ||
			READ_LE_UINT16(data + 20) < 26 || READ_LE_UINT16(data + 20) > size ||
			(uint16)(~READ_LE_UINT16(data + 22) + 0x1234) != READ_LE_UINT16(data + 24))
		return false;
	uint32 at = READ_LE_UINT16(data + 20);
	uint rate = 0;
	byte codec = 0;
	Common::Array<uint> starts;
	while (at < size && data[at]) {
		if (size - at < 4)
			return false;
		const uint32 length = data[at + 1] | (data[at + 2] << 8) | (data[at + 3] << 16);
		if (length > size - at - 4)
			return false;
		Block block;
		block.kind = data[at];
		const byte *payload = data + at + 4;
		uint32 count = length;
		switch (block.kind) {
		case 1:
			if (count <= 2 || payload[1] > 1)
				return false;
			rate = 1000000 / (256 - payload[0]);
			codec = payload[1];
			payload += 2;
			count -= 2;
			// Fall through to the sample block.
			// fallthrough
		case 2:
			if (!rate || !count || (block.kind == 2 && codec))
				return false; // No compressed continuations occur in these assets.
			block.rate = rate;
			if (codec) {
				if (count < 2)
					return false;
				int reference = *payload++;
				int scale = 0;
				block.samples.resize(2 * (count - 1));
				for (uint32 i = 0; i < count - 1; i++) {
					block.samples[2 * i] = decodeNibble(payload[i] >> 4, reference, scale);
					block.samples[2 * i + 1] = decodeNibble(payload[i] & 15, reference, scale);
				}
			} else {
				block.samples.resize(count);
				for (uint32 i = 0; i < count; i++)
					block.samples[i] = ((int)payload[i] - 128) * 256;
			}
			break;
		case 3:
			if (length != 3)
				return false;
			block.rate = 1000000 / (256 - payload[2]);
			block.samples.resize(READ_LE_UINT16(payload) + 1);
			for (uint i = 0; i < block.samples.size(); i++)
				block.samples[i] = 0;
			break;
		case 4:
			if (length != 2)
				return false;
			break; // Marker; DNSDB skips it.
		case 5:
			break; // Text; DNSDB skips it.
		case 6:
			if (length != 2 || starts.size() >= 8)
				return false;
			block.repeats = READ_LE_UINT16(payload);
			starts.push_back(_blocks.size());
			break;
		case 7: {
			if (length || starts.empty())
				return false;
			bool audible = false;
			for (uint i = starts.back() + 1; i < _blocks.size(); i++)
				audible |= !_blocks[i].samples.empty();
			if (!audible)
				return false;
			starts.pop_back();
			break;
		}
		default:
			return false;
		}
		_rate = MAX(_rate, block.rate);
		_blocks.push_back(block);
		at += 4 + length;
	}
	if (!starts.empty() || !_rate)
		return false;
	advanceBlock(); // DNSDB consumes loop headers before submitting first DMA.
	return _current < _blocks.size();
}

void SoundEffectStream::advanceBlock() {
	_position = 0;
	while (_current < _blocks.size()) {
		const Block &block = _blocks[_current];
		if (!block.samples.empty())
			return;
		if (block.kind == 6) {
			Loop loop = {_current + 1, block.repeats};
			_loops.push_back(loop);
		} else if (block.kind == 7) {
			Loop &loop = _loops.back();
			if (loop.remaining) {
				if (loop.remaining != 0xffff)
					loop.remaining--;
				_current = loop.start;
				continue;
			}
			_loops.pop_back();
		}
		_current++;
	}
}

int SoundEffectStream::readBuffer(int16 *buffer, const int numSamples) {
	Common::StackLock lock(_mutex);
	int read = 0;
	while (read < numSamples && _current < _blocks.size()) {
		const Block &block = _blocks[_current];
		// The mixer expects one fixed rate. Preserve changes of VOC divisor
		// with a zero-order hold at the highest rate present in this effect.
		const uint64 sample = _position * block.rate / _rate;
		if (sample >= block.samples.size()) {
			_current++;
			advanceBlock();
			continue;
		}
		buffer[read++] = block.samples[(uint)sample];
		_position++;
	}
	// Complete a boundary even when the caller asks for exactly one block.
	if (_current < _blocks.size() &&
			_position * _blocks[_current].rate / _rate >= _blocks[_current].samples.size()) {
		_current++;
		advanceBlock();
	}
	return read;
}

bool SoundEffectStream::endOfData() const {
	Common::StackLock lock(_mutex);
	return _current >= _blocks.size();
}

void SoundEffectStream::finishLoop() {
	Common::StackLock lock(_mutex);
	if (!_loops.empty())
		_loops.back().remaining = 0;
}

SoundEffectStream *makeSoundEffectStream(const byte *data, uint32 size) {
	SoundEffectStream *stream = new SoundEffectStream();
	if (!stream->load(data, size)) {
		delete stream;
		return nullptr;
	}
	return stream;
}

} // namespace Dune
