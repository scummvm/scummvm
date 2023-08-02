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
// Purpose:  Button class
//=============================================================================
#ifndef CRAB_BUTTON_H
#define CRAB_BUTTON_H

#include "crab/ui/Caption.h"
#include "crab/ui/HoverInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/music/MusicManager.h"
#include "crab/text/TextManager.h"
#include "crab/common_header.h"
#include "crab/ui/element.h"
#include "crab/input/hotkey.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
enum ButtonAction {
	BUAC_IGNORE,
	BUAC_LCLICK,
	BUAC_RCLICK,
	BUAC_GRABBED
};

struct ButtonImage {
	ImageKey _normal, _select, _hover;

	bool operator==(const ButtonImage &img) {
		return _normal == img._normal && _select == img._select && _hover == img._hover; }


	ButtonImage() {
		_normal = 0;
		_select = 0;
		_hover = 0;
	}

	void load(rapidxml::xml_node<char> *node, const bool &echo = true) {
		if (nodeValid(node)) {
			loadImgKey(_normal, "img_b", node, echo);
			loadImgKey(_select, "img_s", node, echo);
			loadImgKey(_hover, "img_h", node, echo);
		}
	}

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
		root->append_attribute(doc.allocate_attribute("img_b", g_engine->_stringPool->Get(_normal)));
		root->append_attribute(doc.allocate_attribute("img_s", g_engine->_stringPool->Get(_select)));
		root->append_attribute(doc.allocate_attribute("img_h", g_engine->_stringPool->Get(_hover)));
	}
};

class Button : public Element {
public:
	bool _visible, _mousePressed;

	// We need to keep track of keyboard and mouse hovering separately
	bool _hoverMouse, _hoverKey, _hoverPrev;

	// Can the player move this button?
	bool _canmove;

	// The button images
	ButtonImage _img;

	// The sound effect played when button is clicked
	pyrodactyl::music::ChunkKey _seClick, _seHover;

	// Text shown when mouse is hovered over the button
	HoverInfo _tooltip;

	// Text shown all times on the button
	Caption _caption;

	// A hotkey is a keyboard key(s) that are equivalent to pressing a button
	pyrodactyl::input::HotKey _hotkey;

	Button();
	~Button() {}
	void reset();

	void setUI(Rect *parent = nullptr);

	void load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void init(const Button &ref, const int &xOffset = 0, const int &yOffset = 0);

	void img(Button &b) {
		_img = b._img;
	}

	void img(ButtonImage &image) {
		_img = image;
	}

	ButtonImage img() {
		return _img;
	}

	void draw(const int &xOffset = 0, const int &yOffset = 0, Rect *clip = nullptr);

	ButtonAction handleEvents(const Common::Event &event, const int &xOffset = 0, const int &yOffset = 0);

	// Special functions to only draw parts of a button (used in special situations like world map)
	void imageCaptionOnlyDraw(const int &xOffset = 0, const int &yOffset = 0, Rect *clip = nullptr);
	void hoverInfoOnlyDraw(const int &xOffset = 0, const int &yOffset = 0, Rect *clip = nullptr);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_BUTTON_H
