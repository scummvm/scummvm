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

void Element::init(const int &xCord, const int &yCord, const Align &alignX, const Align &alignY,
				   const ImageKey img, const int &width, const int &height) {
	x = xCord;
	y = yCord;
	_align.x = alignX;
	_align.y = alignY;

	if (img == 0) {
		w = width;
		h = height;
	} else {
		Image dat = g_engine->_imageManager->getTexture(img);
		w = dat.w();
		h = dat.h();
	}
}

void Element::basicload(rapidxml::xml_node<char> *node, const bool &echo) {
	_raw.load(node, echo);
	loadAlign(_align.x, node, echo, "align_x");
	loadAlign(_align.y, node, echo, "align_y");
}

void Element::load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo) {
	basicload(node, echo);

	if (node->first_attribute("w") == nullptr)
		w = g_engine->_imageManager->getTexture(img).w();
	else
		loadNum(w, "w", node);

	if (node->first_attribute("h") == nullptr)
		h = g_engine->_imageManager->getTexture(img).h();
	else
		loadNum(h, "h", node);

	setUI();
}

void Element::load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo) {
	basicload(node, echo);
	loadNum(w, "w", node, false);
	loadNum(h, "h", node, false);
	setUI(parent);
}

void Element::setUI(Rect *parent) {
	if (parent == nullptr) {
		switch (_align.x) {
		case ALIGN_CENTER:
			x = g_engine->_screenSettings->_cur.w / 2 - w / 2 + _raw.x;
			break;
		case ALIGN_RIGHT:
			x = g_engine->_screenSettings->_cur.w - w + _raw.x;
			break;
		default:
			x = _raw.x;
			break;
		}

		switch (_align.y) {
		case ALIGN_CENTER:
			y = g_engine->_screenSettings->_cur.h / 2 - h / 2 + _raw.y;
			break;
		case ALIGN_RIGHT:
			y = g_engine->_screenSettings->_cur.h - h + _raw.y;
			break;
		default:
			y = _raw.y;
			break;
		}
	} else {
		switch (_align.x) {
		case ALIGN_CENTER:
			x = parent->x + parent->w / 2 - w / 2 + _raw.x;
			break;
		case ALIGN_RIGHT:
			x = parent->x + parent->w - w + _raw.x;
			break;
		default:
			x = parent->x + _raw.x;
			break;
		}

		switch (_align.y) {
		case ALIGN_CENTER:
			y = parent->y + parent->h / 2 - h / 2 + _raw.y;
			break;
		case ALIGN_RIGHT:
			y = parent->y + parent->h - h + _raw.y;
			break;
		default:
			y = parent->y + _raw.y;
			break;
		}
	}
}

} // End of namespace Crab
