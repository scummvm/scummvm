#pragma once

#include "Shape.h"
#include "common_header.h"
#include "musicparam.h"

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