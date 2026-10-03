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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "scumm/smush/saud.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

namespace Scumm {

bool readSaudHeader(Common::SeekableReadStream &stream, SaudHeader &header, bool allowTruncated) {
	int64 end = stream.size();
	if (stream.pos() < 0 || end - stream.pos() < 8 || stream.readUint32BE() != MKTAG('S', 'A', 'U', 'D'))
		return false;
	const uint32 size = stream.readUint32BE();
	if (!allowTruncated && size != end - stream.pos())
		return false;
	end = MIN(end, stream.pos() + size);

	SaudHeader parsed;
	int64 dataOffset = -1;
	while (stream.pos() < end) {
		if (end - stream.pos() < 8)
			return false;
		const uint32 tag = stream.readUint32BE();
		uint32 chunkSize = stream.readUint32BE();
		const int64 offset = stream.pos();
		if (chunkSize > end - offset) {
			if (!allowTruncated || tag != MKTAG('S', 'D', 'A', 'T'))
				return false;
			chunkSize = end - offset;
		}
		if (tag == MKTAG('S', 'D', 'A', 'T')) {
			if (dataOffset >= 0 || !chunkSize)
				return false;
			dataOffset = offset;
			parsed.dataSize = chunkSize;
			if (allowTruncated)
				break;
		} else if (tag == MKTAG('S', 'H', 'D', 'R') && chunkSize == 4) {
			parsed.sampleRate = stream.readUint32LE();
		} else if (tag == MKTAG('S', 'T', 'R', 'K')) {
			if (parsed.scriptOffset >= 0)
				return false;
			parsed.scriptOffset = offset;
			parsed.scriptSize = chunkSize;
			if (chunkSize >= 14) {
				const byte code = stream.readByte(), length = stream.readByte();
				if (code == SAUD_OP_SET_OFFSET && length == 12) {
					if (!stream.skip(8))
						return false;
					parsed.sampleRate = stream.readUint32BE();
				}
			}
		}
		// SAUD chunks have no word-alignment padding.
		if (stream.err() || stream.eos() || !stream.seek(offset + chunkSize))
			return false;
	}
	if (dataOffset < 0 || stream.err() || stream.eos() || !stream.seek(dataOffset))
		return false;
	header = parsed;
	return true;
}

SaudScript::SaudScript() {
	reset(nullptr, 0, 11025);
}

void SaudScript::reset(const byte *data, uint32 size, uint32 defaultRate) {
	_data = data;
	_size = data ? size : 0;
	_position = 0;
	_defaultRate = defaultRate;
	_remaining = _elapsed = 0;
}

bool SaudScript::fail(const char *message) {
	warning("SAUD: %s at STRK offset %u", message, _position);
	_position = _size;
	return false;
}

bool SaudScript::branch(int16 displacement) {
	const int64 target = int64(_position) + displacement;
	if (target < 0 || target > _size)
		return fail("branch outside script");
	_position = target;
	return true;
}

bool SaudScript::nextSpan(SaudSpan &span, byte *variables, int &masterVolume, byte &volume, byte &pan) {
	// A loop must eventually produce samples or terminate.
	uint instructions = 0;
	while (_position < _size) {
		if (++instructions > 65536)
			return fail("script loop without audio");
		if (_size - _position < 2)
			return fail("truncated instruction");
		const byte *code = _data + _position;
		const uint length = code[1] + 2;
		if (length > _size - _position)
			return fail("truncated instruction operands");

		switch (code[0]) {
		case SAUD_OP_INIT:
		case SAUD_OP_SET_OFFSET:
		case SAUD_OP_SET_LENGTH: {
			const bool sliced = code[0] == SAUD_OP_SET_LENGTH;
			const uint required = sliced ? 20 : (code[0] == SAUD_OP_INIT ? 10 : 14);
			if (length < required)
				return fail("short sample instruction");
			span.offset = READ_BE_UINT32(code + 2);
			span.length = READ_BE_UINT32(code + 6);
			span.rate = code[0] == SAUD_OP_INIT ? _defaultRate : READ_BE_UINT32(code + 10);
			if (span.offset > 0x7fffffff || span.length > 0x7fffffff || !span.rate || span.rate > 192000)
				return fail("invalid sample span or rate");
			if (sliced) {
				if (!_remaining) {
					_remaining = span.length;
					_elapsed = 0;
				}
				if (_elapsed > 0x7fffffff - span.offset)
					return fail("sample offset overflow");
				span.offset += _elapsed;
				span.length = MIN(_remaining, READ_BE_UINT32(code + 14));
				_remaining -= span.length;
				_elapsed += span.length;
			} else {
				_remaining = _elapsed = 0;
			}
			if (span.length > 0x7fffffff - span.offset)
				return fail("sample span overflow");
			if (sliced && !_remaining) {
				if (!branch(READ_BE_INT16(code + 18)))
					return false;
			} else {
				_position += length;
			}
			if (span.length)
				return true;
			break;
		}
		case SAUD_OP_UPDATE_HEADER:
		case SAUD_OP_COMPARE_GT:
		case SAUD_OP_COMPARE_LT:
		case SAUD_OP_COMPARE_EQ:
		case SAUD_OP_COMPARE_NE: {
			if (length < 6)
				return fail("short branch instruction");
			const byte id = code[4];
			const byte value = id == SAUD_VALUEID_ALL_VOLS ? masterVolume :
				(id == SAUD_VALUEID_TRK_VOL ? volume : (id == SAUD_VALUEID_TRK_PAN ? pan : variables[id]));
			bool take = false;
			switch (code[0]) {
			case SAUD_OP_UPDATE_HEADER: take = value || id == 0; break;
			case SAUD_OP_COMPARE_GT: take = value > code[5]; break;
			case SAUD_OP_COMPARE_LT: take = value < code[5]; break;
			case SAUD_OP_COMPARE_EQ: take = value == code[5]; break;
			case SAUD_OP_COMPARE_NE: take = value != code[5]; break;
			}
			if (take) {
				if (!branch(READ_BE_INT16(code + 2)))
					return false;
			} else {
				_position += length;
			}
			break;
		}
		case SAUD_OP_SET_PARAM:
		case SAUD_OP_INCR_PARAM: {
			if (length < 4)
				return fail("short parameter instruction");
			const byte id = code[2], value = code[3];
			if (id == SAUD_VALUEID_ALL_VOLS) {
				masterVolume = code[0] == SAUD_OP_SET_PARAM ? value : (masterVolume + value) & 0xffff;
			} else {
				byte &parameter = id == SAUD_VALUEID_TRK_VOL ? volume : (id == SAUD_VALUEID_TRK_PAN ? pan : variables[id]);
				parameter = code[0] == SAUD_OP_SET_PARAM ? value : parameter + value;
			}
			_position += length;
			break;
		}
		case SAUD_OP_STOP:
			_position = _size;
			return false;
		default:
			return fail("unknown instruction");
		}
	}
	return false;
}

} // End of namespace Scumm
