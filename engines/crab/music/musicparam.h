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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_MUSICPARAM_H
#define CRAB_MUSICPARAM_H
#include "audio/audiostream.h"
#include "audio/decoders/vorbis.h"
#include "common/file.h"
#include "crab/loaders.h"
#include "crab/filesystem.h"

namespace Crab {

namespace pyrodactyl {
namespace music {
// We use this object as key for music tracks
// Empty sounds are represented by -1
typedef int MusicKey;

// We use this object as key for sound effects
// Empty sounds are represented by -1
typedef int ChunkKey;

struct MusicData {
	// The id of this track
	MusicKey _id;
	Audio::AudioStream *_track;
	Common::File _file;

	// Sound parameters
	uint32 _fadeInDuration;

	MusicData() {
		reset();
	}

	void reset() {
		_id = -1;
		_track = nullptr;
		_fadeInDuration = 100;

		if (_file.isOpen()) {
			_file.close();
		}
	}

	void load(rapidxml::xml_node<char> *node) {
		loadNum(_id, "id", node);
		loadNum(_fadeInDuration, "fade_in", node);

		if (_file.open(cleansePath(node->first_attribute("path")->value()))) {
			Audio::SeekableAudioStream *stream = Audio::makeVorbisStream(&_file, DisposeAfterUse::NO);
			// loops=0 means infinite here.
			_track = Audio::makeLoopingAudioStream(stream, 0, 0, 0);
		} else {
			warning("Could not open file %s", node->first_attribute("path")->value());
		}
	}
};
} // End of namespace music
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MUSICPARAM_H
