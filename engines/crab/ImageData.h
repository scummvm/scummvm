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

#include "ImageManager.h"
#include "common_header.h"
#include "element.h"

namespace pyrodactyl {
namespace ui {
class ImageData : public Element {
	// The image
	ImageKey key;

	// Should we clip the image? (usually used for large background images)
	bool crop;

	// The clip rectangle, used only when clip is true
	Rect clip;

public:
	ImageData() {
		key = 0;
		crop = false;
	}
	~ImageData() {}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void Draw(const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_IMAGEDATA_H
