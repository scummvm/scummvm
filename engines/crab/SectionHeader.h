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

#pragma once

#include "ImageManager.h"
#include "TextData.h"
#include "TextManager.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
class SectionHeader : public TextData {
	// The content of the header
	std::string text;

	// This image surrounds the text like <img> text <img>, with the right image being flipped horizontally
	ImageKey img;

	// The coordinates for drawing image
	Vector2i left, right;

	// Should we draw one or both or none of the images
	bool draw_l, draw_r;

public:
	SectionHeader() {
		img = 0;
		draw_l = false;
		draw_r = false;
	}
	~SectionHeader() {}

	void Load(rapidxml::xml_node<char> *node);

	void Draw(const int &XOffset = 0, const int &YOffset = 0);
	void Draw(const std::string &str, const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl
