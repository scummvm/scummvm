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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/audio/sound_flex.h"
#include "ultima/ultima8/audio/sonarc_audio_sample.h"
#include "ultima/ultima8/audio/raw_audio_sample.h"
#include "ultima/ultima8/filesys/idata_source.h"

#include "common/memstream.h"

namespace Ultima {
namespace Ultima8 {

SoundFlex::SoundFlex(Common::SeekableReadStream *rs) : Archive(rs), _samples(nullptr) {
	uint32 size;
	uint8 *buf = getRawObject(0, &size);

	Common::MemoryReadStream st(buf, size);

	_index.push_back(SoundFlexEntry(""));
	if (buf[0] == 0xFF) {
		// Crusader flex has an index in the first object with the format:
		// [00 or FF] [ 3 bytes, often 'oB0' or 'pB0' ] [ null-terminated name ]
		// read this data in and work out how to interpet it - probably tells
		// some info about how to play back the raw sounds (eg, loop points?)
		while (!st.eos() && _index.size() < _count) {
			uint32 data = st.readUint32LE();
			Std::string str;
			char c = st.readByte();
			while (c != 0 && !st.eos()) {
				str.push_back(c);
				c = st.readByte();
			}
			_index.push_back(SoundFlexEntry(str.c_str(), data));
		}
	} else {
		// In U8 the first object just has 8-byte names.
		char name[9] = {0};
		int entries = MIN(size / 8, _count);
		for (int i = 0; i < entries; i++) {
			st.read(name, 8);
			_index.push_back(SoundFlexEntry(name));
		}
	}
}

SoundFlex::~SoundFlex() {
	Archive::uncache();
	delete [] _samples;
}

AudioSample *SoundFlex::getSample(uint32 index) {
	if (index >= _count)
		return nullptr;
	cache(index);
	return _samples[index];
}

void SoundFlex::cache(uint32 index) {
	if (index >= _count) return;

	if (!_samples) {
		_samples = new AudioSample * [_count];
		Std::memset(_samples, 0, sizeof(AudioSample *) * _count);
	}

	if (_samples[index]) return;

	// This will cache the data
	uint32 size;
	uint8 *buf = getRawObject(index, &size);

	if (!buf || !size) return;

	if (Std::strncmp(reinterpret_cast<const char *>(buf), "ASFX", 4) == 0) {
		// After the 32 byte header, ASFX (crusader audio) is just raw 11025 data
		const SoundFlexEntry &entry = _index[index];
		debug(6, "SoundFlex: Playing sfx %d (%s) with data 0x%04X", index, entry._name.c_str(), entry._data);
		_samples[index] = new RawAudioSample(buf + 32, size - 32, 11025, true, false);
	} else {
		_samples[index] = new SonarcAudioSample(buf, size);
	}
}

void SoundFlex::uncache(uint32 index) {
	if (index >= _count) return;
	if (!_samples) return;

	delete _samples[index];
	_samples[index] = nullptr;
}

bool SoundFlex::isCached(uint32 index) const {
	if (index >= _count) return false;
	if (!_samples) return false;

	return (_samples[index] != nullptr);
}

} // End of namespace Ultima8
} // End of namespace Ultima
