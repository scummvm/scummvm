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

#include "crab/level/level_objects.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::level;

void PlayerDestMarker::Load(rapidxml::xml_node<char> *node) {
	LoadImgKey(img, "dest", node);
	size.x = gImageManager.GetTexture(img).W();
	size.y = gImageManager.GetTexture(img).H();
}

void PlayerDestMarker::Draw(const Vector2i &pos, const Rect &camera) {
	gImageManager.Draw(pos.x - (size.x / 2) - camera.x, pos.y - (size.y / 2) - camera.y, img);
}

} // End of namespace Crab
