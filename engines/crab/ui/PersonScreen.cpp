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

#include "crab/ui/PersonScreen.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;

void PersonScreen::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("character");
		if (nodeValid(node)) {
			if (nodeValid("bg", node))
				_bg.load(node->first_node("bg"));

			if (nodeValid("name", node))
				_name.load(node->first_node("name"));

			if (nodeValid("img", node))
				_img.load(node->first_node("img"));

			if (nodeValid("menu", node))
				_menu.load(node->first_node("menu"));
		}
	}
}

void PersonScreen::handleEvents(pyrodactyl::event::Info &info, const Common::String &id, const Common::Event &event) {
	if (info.personValid(id))
		_menu.handleEvents(&info.personGet(id), event);
	else
		_menu.handleEvents(nullptr, event);
}

#if 0
void PersonScreen::handleEvents(pyrodactyl::event::Info &info, const Common::String &id, const SDL_Event &Event) {
	if (info.PersonValid(id))
		menu.handleEvents(&info.PersonGet(id), Event);
	else
		menu.handleEvents(nullptr, Event);
}
#endif

void PersonScreen::internalEvents() {
	if (_curSp != nullptr)
		_curSp->dialogUpdateClip(PST_NORMAL);
}

void PersonScreen::draw(pyrodactyl::event::Info &info, const Common::String &id) {
	_bg.draw();

	if (info.personValid(id)) {
		_name.draw(info.personGet(id)._name);
		_menu.draw(&info.personGet(id));
	} else
		_menu.draw(nullptr);

	if (_curSp != nullptr) {
		Rect clip = _curSp->dialogClip(PST_NORMAL);
		g_engine->_imageManager->draw(_img.x, _img.y, _curSp->img(), &clip);
	}
}

void PersonScreen::Cache(Info &info, const Common::String &id, pyrodactyl::level::Level &level) {
	_curSp = level.getSprite(id);

	if (info.personValid(id))
		_menu.Cache(info.personGet(id));
	else
		_menu.Clear();
}

void PersonScreen::setUI() {
	_bg.setUI();
	_name.setUI();
	_img.setUI();
	_menu.setUI();
}

} // End of namespace Crab
