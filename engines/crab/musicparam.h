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