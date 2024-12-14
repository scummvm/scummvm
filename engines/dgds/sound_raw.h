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

#ifndef DGDS_SOUND_RAW_H
#define DGDS_SOUND_RAW_H

#include "audio/mixer.h"

#include "common/str.h"
#include "common/array.h"

#include "dgds/resource.h"
#include "dgds/decompress.h"

namespace Dgds {

/**
 * A simple raw PCM file format which seems to only be used in
 * Willy Beamish CD version intro.
 */
class SoundRaw {
public:
	SoundRaw(ResourceManager *resourceMan, Decompressor *decompressor);
	~SoundRaw();

	void load(const Common::String &filename);
	void play();
	void stop();
	bool isPlaying() const;
	void loadFromStream(Common::SeekableReadStream *stream, int size);
	uint32 playedOffset() const;

private:
	Common::Array<byte> _data;
	ResourceManager *_resourceMan;
	Decompressor *_decompressor;
	Audio::SoundHandle _handle;
};

} // end namespace Dgds

#endif // DGDS_SOUND_RAW_H
