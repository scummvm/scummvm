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

#include "awe/aifc_player.h"
#include "awe/detection.h"

namespace Awe {

static uint32 READ_IEEE754(const uint8 *p) {
	const uint32 m = READ_BE_UINT32(p + 2);
	const int exp = 30 - p[1];
	return (m >> exp);
}

AifcPlayer::AifcPlayer() {
}

bool AifcPlayer::play(int mixRate, const char *path, uint32 startOffset) {
	_ssndSize = 0;
	if (_f.open(path)) {
		_f.seek(startOffset);
		uint8 buf[12];
		_f.read(buf, sizeof(buf));
		if (memcmp(buf, "FORM", 4) == 0 && memcmp(buf + 8, "AIFC", 4) == 0) {
			const uint32 size = READ_BE_UINT32(buf + 4);
			for (uint32 offset = 12; offset + 8 < size; ) {
				_f.seek(startOffset + offset);
				_f.read(buf, 8);
				const uint32 sz = READ_BE_UINT32(buf + 4);
				if (memcmp(buf, "COMM", 4) == 0) {
					const int channels = _f.readUint16BE();
					_f.readUint32BE(); // samples per frame
					const int bits = _f.readUint16BE();
					_f.read(buf, 10);
					const int rate = READ_IEEE754(buf);
					if (channels != 2) {
						warning("Unsupported AIFF-C channels %d rate %d (%s)", channels, rate, path);
						break;
					}
					_f.read(buf, 4);
					if (memcmp(buf, "SDX2", 4) != 0) {
						warning("Unsupported compression");
						break;
					}
					debugC(kDebugSound, "AIFF-C channels %d rate %d bits %d", channels, rate, bits);
					_rate.reset(rate, mixRate);
				} else if (memcmp(buf, "SSND", 4) == 0) {
					_f.readUint32BE(); // block offset
					_f.readUint32BE(); // block size
					_ssndOffset = startOffset + offset + 8 + 8;
					_ssndSize = sz;
					debugC(kDebugSound, "AIFF-C ssnd size %d", _ssndSize);
					break;
				} else if (memcmp(buf, "FVER", 4) == 0) {
					const uint32 version = _f.readUint32BE();
					if (version != 0xA2805140) {
						warning("Unexpected AIFF-C version 0x%x (%s)", version, path);
					}
				} else if (memcmp(buf, "INST", 4) == 0) {
					// unused
				} else if (memcmp(buf, "MARK", 4) == 0) {
					const int count = _f.readUint16BE();
					for (int i = 0; i < count; ++i) {
						_f.readUint16BE(); // marker_id
						_f.readUint32BE(); // marker_position
						const int len = _f.readByte();
						if (len != 0) {
							char name[256];
							_f.read(name, len);
							name[len] = 0;
						}
						// pad ((len + 1) & 1)
					}
				} else {
					warning("Unhandled AIFF-C tag '%02x%02x%02x%02x' size %d offset 0x%x path %s", buf[0], buf[1], buf[2], buf[3], sz, startOffset + offset, path);
					break;
				}
				offset += sz + 8;
			}
		}
	}
	_pos = 0;
	_sampleL = _sampleR = 0;
	return _ssndSize != 0;
}

void AifcPlayer::stop() {
	_f.close();
}

static int16 decodeSDX2(int16 prev, int8 data) {
	const int sqr = data * ABS(data) * 2;
	return (data & 1) != 0 ? prev + sqr : sqr;
}

int8 AifcPlayer::readSampleData() {
	if (_pos >= _ssndSize) {
		_pos = 0;
		_f.seek(_ssndOffset);
	}
	const int8 data = _f.readByte();
	++_pos;
	return data;
}

void AifcPlayer::decodeSamples() {
	for (const uint32 pos = _rate.getInt(); pos == _rate.getInt(); _rate.offset += _rate.inc) {
		_sampleL = decodeSDX2(_sampleL, readSampleData());
		_sampleR = decodeSDX2(_sampleR, readSampleData());
	}
}

void AifcPlayer::readSamples(int16 *buf, int len) {
	for (int i = 0; i < len; i += 2) {
		decodeSamples();
		*buf++ = _sampleL;
		*buf++ = _sampleR;
	}
}

} // namespace Awe
