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

#ifndef CRAB_IMAGEDATA_H
#define CRAB_IMAGEDATA_H

#include "crab/image/ImageManager.h"
#include "crab/common_header.h"
#include "crab/ui/element.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class ImageData : public Element {
	// The image
	ImageKey _key;

	// Should we clip the image? (usually used for large background images)
	bool _crop;

	// The clip rectangle, used only when clip is true
	Rect _clip;

public:
	ImageData() {
		_key = 0;
		_crop = false;
	}

	~ImageData() {}

	void load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void draw(const int &xOffset = 0, const int &yOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_IMAGEDATA_H
