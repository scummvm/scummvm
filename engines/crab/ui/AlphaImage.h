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

#ifndef CRAB_ALPHAIMAGE_H
#define CRAB_ALPHAIMAGE_H

#include "crab/image/ImageManager.h"
#include "crab/ui/element.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class AlphaImage {
	// The image used - we don't use the image manager
	// because we don't change the alpha for those images
	ImageKey _img;

	// The information for drawing the image
	Element _pos;

	// The information related to alpha modulation of the image
	struct AlphaVal {
		int _cur, _min, _max;

		// Are we increasing or decreasing the alpha
		bool _inc;

		// By how much do we change the alpha every update
		int _change;

		AlphaVal() {
			_cur = 255;
			_min = 255;
			_max = 255;
			_inc = true;
			_change = 0;
		}
	} alpha;

public:
	AlphaImage() {}
	AlphaImage(rapidxml::xml_node<char> *node) {
		load(node);
	}

	~AlphaImage() {}

	void load(rapidxml::xml_node<char> *node, const bool &echo = true);

	void draw(const int &xOffset = 0, const int &yOffset = 0);

	// This is used to vary the alpha
	void internalEvents();

	void setUI() {
		_pos.setUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ALPHAIMAGE_H
