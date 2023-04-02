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
enum ButtonAction { BUAC_IGNORE,
					BUAC_LCLICK,
					BUAC_RCLICK,
					BUAC_GRABBED };

struct ButtonImage {
	ImageKey normal, select, hover;

	bool operator==(const ButtonImage &img) { return normal == img.normal && select == img.select && hover == img.hover; }

	ButtonImage() {
		normal = 0;
		select = 0;
		hover = 0;
	}

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true) {
		if (NodeValid(node)) {
			LoadImgKey(normal, "img_b", node, echo);
			LoadImgKey(select, "img_s", node, echo);
			LoadImgKey(hover, "img_h", node, echo);
		}
	}

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
		root->append_attribute(doc.allocate_attribute("img_b", gStrPool.Get(normal)));
		root->append_attribute(doc.allocate_attribute("img_s", gStrPool.Get(select)));
		root->append_attribute(doc.allocate_attribute("img_h", gStrPool.Get(hover)));
	}
};

class Button : public Element {
public:
	bool visible, mousepressed;

	// We need to keep track of keyboard and mouse hovering separately
	bool hover_mouse, hover_key, hover_prev;

	// Can the player move this button?
	bool canmove;

	// The button images
	ButtonImage img;

	// The sound effect played when button is clicked
	pyrodactyl::music::ChunkKey se_click, se_hover;

	// Text shown when mouse is hovered over the button
	HoverInfo tooltip;

	// Text shown all times on the button
	Caption caption;

	// A hotkey is a keyboard key(s) that are equivalent to pressing a button
	pyrodactyl::input::HotKey hotkey;

	Button();
	~Button() {}
	void Reset();
	void SetUI(Rect *parent = NULL);

	void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
	void Init(const Button &ref, const int &XOffset = 0, const int &YOffset = 0);

	void Img(Button &b) { img = b.img; }
	void Img(ButtonImage &image) { img = image; }
	ButtonImage Img() { return img; }

	void Draw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);

#if 0
	ButtonAction HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);
#endif

	// Special functions to only draw parts of a button (used in special situations like world map)
	void ImageCaptionOnlyDraw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
	void HoverInfoOnlyDraw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_BUTTON_H
