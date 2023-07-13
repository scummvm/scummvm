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
#include "crab/crab.h"
#include "crab/ui/element.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void Element::Init(const int &X, const int &Y, const Align &align_x, const Align &align_y,
				   const ImageKey img, const int &W, const int &H) {
	x = X;
	y = Y;
	align.x = align_x;
	align.y = align_y;

	if (img == 0) {
		w = W;
		h = H;
	} else {
		Image dat = g_engine->_imageManager->GetTexture(img);
		w = dat.w();
		h = dat.h();
	}
}

void Element::Basicload(rapidxml::xml_node<char> *node, const bool &echo) {
	raw.load(node, echo);
	loadAlign(align.x, node, echo, "align_x");
	loadAlign(align.y, node, echo, "align_y");
}

void Element::load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo) {
	Basicload(node, echo);

	if (node->first_attribute("w") == NULL)
		w = g_engine->_imageManager->GetTexture(img).w();
	else
		loadNum(w, "w", node);

	if (node->first_attribute("h") == NULL)
		h = g_engine->_imageManager->GetTexture(img).h();
	else
		loadNum(h, "h", node);

	setUI();
}

void Element::load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo) {
	Basicload(node, echo);
	loadNum(w, "w", node, false);
	loadNum(h, "h", node, false);
	setUI(parent);
}

void Element::setUI(Rect *parent) {
	if (parent == NULL) {
		switch (align.x) {
		case ALIGN_CENTER:
			x = g_engine->_screenSettings->cur.w / 2 - w / 2 + raw.x;
			break;
		case ALIGN_RIGHT:
			x = g_engine->_screenSettings->cur.w - w + raw.x;
			break;
		default:
			x = raw.x;
			break;
		}

		switch (align.y) {
		case ALIGN_CENTER:
			y = g_engine->_screenSettings->cur.h / 2 - h / 2 + raw.y;
			break;
		case ALIGN_RIGHT:
			y = g_engine->_screenSettings->cur.h - h + raw.y;
			break;
		default:
			y = raw.y;
			break;
		}
	} else {
		switch (align.x) {
		case ALIGN_CENTER:
			x = parent->x + parent->w / 2 - w / 2 + raw.x;
			break;
		case ALIGN_RIGHT:
			x = parent->x + parent->w - w + raw.x;
			break;
		default:
			x = parent->x + raw.x;
			break;
		}

		switch (align.y) {
		case ALIGN_CENTER:
			y = parent->y + parent->h / 2 - h / 2 + raw.y;
			break;
		case ALIGN_RIGHT:
			y = parent->y + parent->h - h + raw.y;
			break;
		default:
			y = parent->y + raw.y;
			break;
		}
	}
}

} // End of namespace Crab
