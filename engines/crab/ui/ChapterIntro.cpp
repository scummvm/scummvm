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

#include "crab/ui/ChapterIntro.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ChapterIntro::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("dialog", node))
		dialog.Load(node->first_node("dialog"));

	if (NodeValid("image", node))
		pos.Load(node->first_node("image"));

	if (NodeValid("trait", node))
		traits.Load(node->first_node("trait"));
}

bool ChapterIntro::HandleEvents(SDL_Event &Event) {
	if (traits.HandleEvents(Event))
		show_traits = true;

	return dialog.HandleEvents(Event);
}

void ChapterIntro::Draw(pyrodactyl::event::Info &info, std::string &text,
						pyrodactyl::anim::Sprite *cur_sp, const pyrodactyl::people::PersonState &state) {
	dialog.Draw(false);
	dialog.Draw(info, text);

	traits.Draw();

	if (cur_sp != nullptr) {
		Rect clip = cur_sp->DialogClip(state);
		gImageManager.Draw(pos.x, pos.y, cur_sp->Img(), &clip);
	}
}

} // End of namespace Crab
