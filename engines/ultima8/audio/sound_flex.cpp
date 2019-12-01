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

#include "ultima8/misc/pent_include.h"

#include "SoundFlex.h"
#include "SonarcAudioSample.h"
#include "ultima8/filesys/idata_source.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(SoundFlex, Pentagram::Archive)


SoundFlex::SoundFlex(IDataSource *ds) : Archive(ds) {
	samples = 0;
}

SoundFlex::~SoundFlex() {
	Archive::uncache();
	delete [] samples;
}

void SoundFlex::cache(uint32 index) {
	if (index >= count) return;

	if (!samples) {
		samples = new Pentagram::AudioSample * [count];
		std::memset(samples, 0, sizeof(Pentagram::AudioSample *) * count);
	}

	if (samples[index]) return;

	// This will cache the data
	uint32 size;
	uint8 *buf = getRawObject(index, &size);

	if (!buf || !size) return;

	samples[index] = new Pentagram::SonarcAudioSample(buf, size);
}

void SoundFlex::uncache(uint32 index) {
	if (index >= count) return;
	if (!samples) return;

	delete samples[index];
	samples[index] = 0;
}

bool SoundFlex::isCached(uint32 index) {
	if (index >= count) return false;
	if (!samples) return false;

	return (samples[index] != 0);
}

} // End of namespace Ultima8
