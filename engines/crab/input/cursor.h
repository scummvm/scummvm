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

//=============================================================================
// Author:   Arvind
// Purpose:  Cursor object
//=============================================================================
#ifndef CRAB_CURSOR_H
#define CRAB_CURSOR_H

#include "crab/image/Image.h"
#include "crab/common_header.h"
#include "crab/loaders.h"
#include "crab/vectors.h"

namespace Crab {

namespace pyrodactyl {
namespace input {
class Cursor {
	// Mouse images
	pyrodactyl::image::Image _img, _imgS, _imgHover, _imgHoverS;

	// The hover mouse cursor is drawn at a slight offset to the normal cursor
	Vector2i _hoverOffset;

	// Mouse image changes slightly if left click button is pressed
	bool _pressed;

	uint8 _state;

public:
	// Various coordinates
	Vector2i _motion, _button, _rel;

	// Is the mouse inside the HUD? Used to disable level mouse movement if true
	bool _insideHud;

	// Was the last click on an NPC?
	bool _hover;

	Cursor() {
		_pressed = false;
		_insideHud = false;
		_hover = false;
	}
	~Cursor() {}

	void Quit() {
		_img.deleteImage();
		_imgS.deleteImage();
		_imgHover.deleteImage();
		_imgHoverS.deleteImage();
	}

	void reset();

	void load(rapidxml::xml_node<char> *node);
	void handleEvents(const Common::Event &event);

	void draw();
	bool pressed() {
		return _pressed;
	}
};

} // End of namespace input
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_CURSOR_H
