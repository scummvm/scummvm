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


#include "crab/HoverInfo.h"

namespace Crab {

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void HoverInfo::Init(const HoverInfo &h, const int &XOffset, const int &YOffset) {
	*this = h;
	x += XOffset;
	y += YOffset;
}

void HoverInfo::Load(rapidxml::xml_node<char> *node, Rect *parent) {
	if (TextData::Load(node, parent, false)) {
		LoadStr(text, "text", node);
		enabled = true;
	}
}

void HoverInfo::Draw(const int &XOffset, const int &YOffset) {
	if (enabled)
		TextData::Draw(text, XOffset, YOffset);
}

} // End of namespace Crab
