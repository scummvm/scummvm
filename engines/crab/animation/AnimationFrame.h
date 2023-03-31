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

#ifndef CRAB_ANIMATIONFRAME_H
#define CRAB_ANIMATIONFRAME_H

#include "crab/animation/AnimationEffect.h"
#include "crab/ui/HoverInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/text/TextManager.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {

// Make linker happy by temporarily defining SDL_COLOR
struct SDL_Color {
	uint32 r;
	uint32 g;
	uint32 b;
	uint32 a;
};

struct AnimationFrame : public Vector2i {
	// The image drawn in this frame
	ImageKey img;

	// This is the time in we draw the frame milliseconds relative to the start of the entire animation
	uint32 start, finish;

	// The effect applied to the image
	AnimationEffect eff;

	// In case we want to display any words during the animation
	pyrodactyl::ui::HoverInfo text;

	// The color drawn on the screen
	SDL_Color col;

	AnimationFrame() {
		img = 0;
		start = 0;
		finish = 0;
		col.r = 0;
		col.g = 0;
		col.b = 0;
		col.a = 255;
	}
	AnimationFrame(rapidxml::xml_node<char> *node);

	void Reset();
	void Draw(const uint32 &timestamp);
	DrawType InternalEvents(const uint32 &timestamp);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ANIMATIONFRAME_H
