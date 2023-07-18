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

#ifndef CRAB_HEALTHINDICATOR_H
#define CRAB_HEALTHINDICATOR_H
#include "crab/common_header.h"

#include "crab/image/ImageManager.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class HealthIndicator {
	struct HealthImage {
		ImageKey _normal, _glow;
		int _val;
	};

	int _x, _y;
	Common::Array<HealthImage> _img;

	// Related to the pulse effect
	uint8 _alpha;
	bool _inc;

public:
	HealthIndicator() {
		_x = 0;
		_y = 0;
		_alpha = 0;
		_inc = true;
	}

	~HealthIndicator() {}

	void load(rapidxml::xml_node<char> *node);
	void draw(int num);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_HEALTHINDICATOR_H
