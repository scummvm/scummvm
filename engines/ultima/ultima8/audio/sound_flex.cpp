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
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SoundFlex, Archive)


SoundFlex::SoundFlex(IDataSource *ds) : Archive(ds), _samples(nullptr) {
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

	_samples[index] = new SonarcAudioSample(buf, size);
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
