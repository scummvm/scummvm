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

#ifndef CRAB_SECTIONHEADER_H
#define CRAB_SECTIONHEADER_H

#include "crab/image/ImageManager.h"
#include "crab/ui/TextData.h"
#include "crab/text/TextManager.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class SectionHeader : public TextData {
	// The content of the header
	Common::String _text;

	// This image surrounds the text like <img> text <img>, with the right image being flipped horizontally
	ImageKey _img;

	// The coordinates for drawing image
	Vector2i _left, _right;

	// Should we draw one or both or none of the images
	bool _drawL, _drawR;

public:
	SectionHeader() {
		_img = 0;
		_drawL = false;
		_drawR = false;
	}

	~SectionHeader() {}

	void load(rapidxml::xml_node<char> *node);

	void draw(const int &xOffset = 0, const int &yOffset = 0);
	void draw(const Common::String &str, const int &xOffset = 0, const int &yOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SECTIONHEADER_H
