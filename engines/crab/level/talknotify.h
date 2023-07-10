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

#ifndef CRAB_TALKNOTIFY_H
#define CRAB_TALKNOTIFY_H

#include "crab/event/GameEventInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/common_header.h"
#include "crab/animation/sprite.h"

namespace Crab {

namespace pyrodactyl {
namespace level {
class TalkNotify {
	// The offset to the sprite at which the name is drawn - along with alignment, font and color used to draw the name
	Vector2i offset;
	FontKey font;
	int col;
	Align align;

public:
	TalkNotify() {
		font = 0;
		col = 0;
		align = ALIGN_CENTER;
	}

	void load(rapidxml::xml_node<char> *node);
	void Draw(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s, const Rect &camera);
};
} // End of namespace level
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TALKNOTIFY_H
