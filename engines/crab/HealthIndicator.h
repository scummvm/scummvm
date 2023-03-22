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
#include "common_header.h"

#include "ImageManager.h"

namespace pyrodactyl {
namespace ui {
class HealthIndicator {
	struct HealthImage {
		ImageKey normal, glow;
		int val;
	};

	int x, y;
	std::vector<HealthImage> img;

	// Related to the pulse effect
	Uint8 alpha;
	bool inc;

public:
	HealthIndicator() {
		x = 0;
		y = 0;
		alpha = 0;
		inc = true;
	}
	~HealthIndicator() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(int num);
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_HEALTHINDICATOR_H
