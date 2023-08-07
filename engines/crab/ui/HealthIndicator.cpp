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
#include "crab/ui/HealthIndicator.h"

namespace Crab {

using namespace pyrodactyl::ui;

void HealthIndicator::load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		loadXY(_x, _y, node);

		for (auto n = node->first_node("img"); n != nullptr; n = n->next_sibling("img")) {
			HealthImage hi;
			loadImgKey(hi._normal, "normal", n);
			loadImgKey(hi._glow, "glow", n);
			loadNum(hi._val, "val", n);

			_img.push_back(hi);
		}
	}
}

void HealthIndicator::draw(int num) {
	for (auto i : _img)
		if (num == i._val) {
			using namespace pyrodactyl::image;
			g_engine->_imageManager->draw(_x, _y, i._normal);
			g_engine->_imageManager->getTexture(i._glow).alpha(_alpha);
			g_engine->_imageManager->draw(_x, _y, i._glow);

			if (_inc) {
				_alpha += 2;
				if (_alpha >= 250)
					_inc = false;
			} else {
				_alpha -= 2;
				if (_alpha < 4)
					_inc = true;
			}

			break;
		}
}

} // End of namespace Crab
