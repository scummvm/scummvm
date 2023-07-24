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

void ChapterIntro::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("dialog", node))
		_dialog.load(node->first_node("dialog"));

	if (nodeValid("image", node))
		_pos.load(node->first_node("image"));

	if (nodeValid("trait", node))
		_traits.load(node->first_node("trait"));
}

bool ChapterIntro::handleEvents(Common::Event &event) {
	if (_traits.handleEvents(event))
		_showTraits = true;

	return _dialog.handleEvents(event);
}

void ChapterIntro::draw(pyrodactyl::event::Info &info, Common::String &text,
						pyrodactyl::anim::Sprite *curSp, const pyrodactyl::people::PersonState &state) {
	_dialog.draw(false);
	_dialog.draw(info, text);

	_traits.draw();

	if (curSp != nullptr) {
		Rect clip = curSp->dialogClip(state);
		g_engine->_imageManager->draw(_pos.x, _pos.y, curSp->img(), &clip);
	}
}

} // End of namespace Crab
