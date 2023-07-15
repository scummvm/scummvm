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
#include "crab/ui/AlphaImage.h"
#include "crab/text/TextManager.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void AlphaImage::load(rapidxml::xml_node<char> *node, const bool &echo) {
	_pos.load(node, echo);
	loadImgKey(_img, "img", node, echo);

	loadNum(alpha._min, "min", node);
	loadNum(alpha._max, "max", node);
	loadNum(alpha._change, "inc", node);

	alpha._cur = alpha._max; //alpha.cur = alpha.min + gRandom.Num() % (alpha.max - alpha.min - 1);
}

void AlphaImage::internalEvents() {
	if (alpha._inc) {
		alpha._cur += alpha._change;
		if (alpha._cur >= alpha._max) {
			alpha._cur = alpha._max;
			alpha._inc = false;
		}
	} else {
		alpha._cur -= alpha._change;
		if (alpha._cur <= alpha._min) {
			alpha._cur = alpha._min;
			alpha._inc = true;
		}
	}

	if (g_engine->_imageManager->validTexture(_img))
		g_engine->_imageManager->getTexture(_img).alpha(alpha._cur);
}

void AlphaImage::draw(const int &xOffset, const int &yOffset) {
	if (g_engine->_imageManager->validTexture(_img))
		g_engine->_imageManager->getTexture(_img).draw(_pos.x + xOffset, _pos.y + yOffset);
}

} // End of namespace Crab
