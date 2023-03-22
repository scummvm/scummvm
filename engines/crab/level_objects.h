#pragma once

#include "ImageManager.h"
#include "common_header.h"

namespace pyrodactyl {
namespace level {
// Is the world map accessible from this level?
struct MapVis {
	bool normal, current;

	MapVis() { Set(true); }
	void Set(bool val) {
		normal = val;
		current = val;
	}
};

// Used for drawing the destination marker for point and click movement
struct PlayerDestMarker {
	ImageKey img;
	Vector2i size;

	PlayerDestMarker() { img = 0; }

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const Vector2i &pos, const Rect &camera);
};
} // End of namespace level
} // End of namespace pyrodactyl