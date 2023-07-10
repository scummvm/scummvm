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
#include "crab/animation/AnimationFrame.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::anim;

AnimationFrame::AnimationFrame(rapidxml::xml_node<char> *node) : eff(node) {
	Vector2i::Load(node);
	LoadImgKey(img, "img", node);
	LoadNum(start, "start", node);
	LoadNum(finish, "finish", node);
	//LoadColor(col, node);

	if (NodeValid("text", node, false))
		text.Load(node->first_node("text"));

	Reset();
}

void AnimationFrame::Reset() {
	switch (eff._type) {
	case FADE_IN:
		col.a = 0;
		break;
	case FADE_OUT:
		col.a = 255;
		break;
	default:
		col.a = 255;
		break;
	}
}

void AnimationFrame::Draw(const uint32 &timestamp) {
	warning("STUB: AnimationFrame::Draw()");

#if 0
	// Only draw the frame in the specified duration
	if (timestamp >= start && timestamp <= finish) {
		// Fill the screen with the color indicated
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(gRenderer, col.r, col.g, col.b, col.a);
		SDL_RenderFillRect(gRenderer, NULL);

		g_engine->_imageManager->Draw(x, y, img);
		text.Draw();
	}
#endif
}

DrawType AnimationFrame::InternalEvents(const uint32 &timestamp) {

	// Vary alpha according to the effect values in the variation time frame
	if (timestamp >= eff._start && timestamp <= eff._finish) {
		// These equations courtesy of linear algebra
		switch (eff._type) {
		case FADE_IN:
			col.a = (255 * (timestamp - eff._start)) / (eff._finish - eff._start);
			break;
		case FADE_OUT:
			col.a = (255 * (eff._finish - timestamp)) / (eff._finish - eff._start);
			break;
		default:
			break;
		}

		return eff._drawGame;
	}

	return DRAW_SAME;
}

} // End of namespace Crab
