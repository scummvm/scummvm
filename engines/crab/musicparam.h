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

#pragma once
#include "common_header.h"

#include "loaders.h"

namespace pyrodactyl {
namespace music {
// We use this object as key for music tracks
// Empty sounds are represented by -1
typedef unsigned int MusicKey;

// We use this object as key for sound effects
// Empty sounds are represented by -1
typedef unsigned int ChunkKey;

struct MusicData {
	// The id of this track
	MusicKey id;

	// The track data
	Mix_Music *track;

	// Sound parameters
	Uint32 fade_in_duration;

	MusicData() {
		id = -1;
		track = nullptr;
		fade_in_duration = 100;
	}

	void Load(rapidxml::xml_node<char> *node) {
		LoadNum(id, "id", node);
		LoadNum(fade_in_duration, "fade_in", node);

		if (track != nullptr)
			Mix_FreeMusic(track);

		track = Mix_LoadMUS(node->first_attribute("path")->value());
	}
};
} // End of namespace music
} // End of namespace pyrodactyl
