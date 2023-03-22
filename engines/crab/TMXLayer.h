#pragma once

#include "Image.h"
#include "Rectangle.h"
#include "TileInfo.h"
#include "common_header.h"

namespace TMX {
enum LayerType {
	// Run of the mill layer full of tiles
	LAYER_NORMAL,

	// Layer containing a simple image instead of tiles
	LAYER_IMAGE,

	// We do not draw prop layers along with the non-prop layers,
	// because their draw order depends on the positions of the sprites
	LAYER_PROP,

	// Parallax layers are drawn differently, but are still composed of tiles
	LAYER_PARALLAX,

	// Auto-hide layers are primarily used for interiors
	// These are only visible if the player is colliding with them
	LAYER_AUTOHIDE,

	// Auto-show layers are the reverse of auto-hide layers
	// These are only visible if the player is not colliding with them
	LAYER_AUTOSHOW
};

class Layer {
public:
	// Name of the layer
	std::string name;

	// Dimensions of the layer in terms of tiles
	int w, h;

	Layer() {
		w = 0;
		h = 0;
	}
	bool Load(rapidxml::xml_node<char> *node);
};

// Currently we just use one general purpose layer object instead of multiple inherited classes and stuff
class MapLayer : public Layer {
public:
	// The tiles in the layer
	std::vector<std::vector<TileInfo> > tile;

	// The type of layer
	LayerType type;

	// The image in the layer
	pyrodactyl::image::Image img;

	// The coordinates to draw the prop in(x,y) and dimensions of the area(w,h) in terms of tiles
	// This is also the collision rectangle of the prop and auto hide layer
	Rect pos;

	// The rate of scrolling of image, used for parallax
	Vector2f rate;

	// Is the player colliding with the layer? (used for auto hide layer)
	bool collide;

	MapLayer() : rate(1, 1) {
		type = LAYER_NORMAL;
		collide = false;
	}
	bool Load(const std::string &path, rapidxml::xml_node<char> *node);
};
} // End of namespace TMX