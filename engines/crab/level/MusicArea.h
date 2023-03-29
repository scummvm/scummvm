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

#ifndef CRAB_MUSICAREA_H
#define CRAB_MUSICAREA_H

#include "crab/Shape.h"
#include "crab/common_header.h"
#include "crab/musicparam.h"

namespace Crab {

namespace pyrodactyl {
namespace level {
struct MusicInfo {
	// Are we playing a sound effect or a music track
	bool track;

	// The id of the music track or sound effect being played
	int id;

	// The amount of loops
	int loops;

	MusicInfo() {
		track = false;
		id = -1;
		loops = -1;
	}
	~MusicInfo() {}
};

class MusicArea : public Shape, public MusicInfo {
public:
	MusicArea() {}
	~MusicArea() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MUSICAREA_H
