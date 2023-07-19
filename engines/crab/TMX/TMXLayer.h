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

#ifndef CRAB_TMXLAYER_H
#define CRAB_TMXLAYER_H

#include "crab/image/Image.h"
#include "crab/Rectangle.h"
#include "crab/TMX/TileInfo.h"
#include "crab/common_header.h"

namespace Crab {

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
	Common::String _name;

	// Dimensions of the layer in terms of tiles
	int _w, _h;

	Layer() {
		_w = 0;
		_h = 0;
	}
	bool load(rapidxml::xml_node<char> *node);
};

// Currently we just use one general purpose layer object instead of multiple inherited classes and stuff
class MapLayer : public Layer {
public:
	// The tiles in the layer
	Common::Array<Common::Array<TileInfo>> _tile;

	// The type of layer
	LayerType _type;

	// The image in the layer
	pyrodactyl::image::Image _img;

	// The coordinates to draw the prop in(x,y) and dimensions of the area(w,h) in terms of tiles
	// This is also the collision rectangle of the prop and auto hide layer
	Rect _pos;

	Common::List<Rect> _boundRect;

	// The rate of scrolling of image, used for parallax
	Vector2f _rate;

	// Is the player colliding with the layer? (used for auto hide layer)
	bool _collide;

	MapLayer() : _rate(1, 1) {
		_type = LAYER_NORMAL;
		_collide = false;
	}
	bool load(const Common::String &path, rapidxml::xml_node<char> *node);
};
} // End of namespace TMX

} // End of namespace Crab

#endif // CRAB_TMXLAYER_H
