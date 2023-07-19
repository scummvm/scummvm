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

#include "crab/ui/PersonHandler.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;

void PersonHandler::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("dialog", node))
		_dlbox.load(node->first_node("dialog"));

	if (nodeValid("opinion", node)) {
		rapidxml::xml_node<char> *opnode = node->first_node("opinion");

		if (nodeValid("friendship", opnode))
			_opinion[OPI_LIKE].load(opnode->first_node("friendship"));

		if (nodeValid("respect", opnode))
			_opinion[OPI_RESPECT].load(opnode->first_node("respect"));

		if (nodeValid("fear", opnode))
			_opinion[OPI_FEAR].load(opnode->first_node("fear"));
	}

	if (nodeValid("image", node)) {
		rapidxml::xml_node<char> *imgnode = node->first_node("image");
		_img.load(imgnode);

		if (nodeValid("sprite_align", imgnode))
			_spriteAlign.load(imgnode->first_node("sprite_align"));
	}

	if (nodeValid("name", node))
		_name.load(node->first_node("name"));

	if (nodeValid("journal", node))
		_jb.load(node->first_node("journal"));
}

void PersonHandler::draw(pyrodactyl::event::Info &info, pyrodactyl::event::GameEvent *event, const Common::String &personId,
						 const bool &player, pyrodactyl::anim::Sprite *s) {
	// Draw the dialog box background
	_dlbox.draw(player);

	if (s != NULL) {
		Rect r = s->dialogClip(event->_state);
		int x = _img.x, y = _img.y;

		if (_spriteAlign._x == ALIGN_CENTER)
			x -= r.w / 2;
		else if (_spriteAlign._x == ALIGN_RIGHT)
			x -= r.w;

		if (_spriteAlign._y == ALIGN_CENTER)
			y -= r.h / 2;
		else if (_spriteAlign._y == ALIGN_RIGHT)
			y -= r.h;

		g_engine->_imageManager->draw(x, y, s->img(), &r);
	}

	if (info.personValid(personId)) {
		_name.draw(info.personGet(personId)._name);

		if (!player) {
			_opinion[OPI_LIKE].draw(info.personGet(personId)._opinion._val[OPI_LIKE], OPINION_MAX);
			_opinion[OPI_RESPECT].draw(info.personGet(personId)._opinion._val[OPI_RESPECT], OPINION_MAX);
			_opinion[OPI_FEAR].draw(info.personGet(personId)._opinion._val[OPI_FEAR], OPINION_MAX);
		}
	}

	// Draw the journal button
	_jb.draw();

	// Draw the dialog box text
	_dlbox.draw(info, event->_dialog);
}

bool PersonHandler::handleCommonEvents(const Common::Event &event) {
	_opinion[OPI_LIKE].handleEvents(event);
	_opinion[OPI_RESPECT].handleEvents(event);
	_opinion[OPI_FEAR].handleEvents(event);

	if (_jb.handleEvents(event) == BUAC_LCLICK) {
		// User wants to open their journal
		_showJournal = true;
		return true;
	}

	return false;
}

bool PersonHandler::handleDlboxEvents(const Common::Event &event) {
	return _dlbox.handleEvents(event);
}

#if 0
bool PersonHandler::HandleCommonEvents(const SDL_Event &Event) {
	opinion[OPI_LIKE].handleEvents(Event);
	opinion[OPI_RESPECT].handleEvents(Event);
	opinion[OPI_FEAR].handleEvents(Event);

	if (jb.handleEvents(Event) == BUAC_LCLICK) {
		// User wants to open their journal
		show_journal = true;
		return true;
	}

	return false;
}

bool PersonHandler::HandleDlboxEvents(const SDL_Event &Event) {
	return dlbox.handleEvents(Event);
}
#endif

void PersonHandler::internalEvents(const pyrodactyl::people::PersonState &state, pyrodactyl::anim::Sprite *s) {
	if (s != NULL)
		s->dialogUpdateClip(state);
}

void PersonHandler::opinionChange(pyrodactyl::event::Info &info, const Common::String &id, const pyrodactyl::people::OpinionType &type, const int &val) {
	if (info.personValid(id)) {
		// First, get the value of the object's opinion
		int old = 0;
		info.opinionGet(id, type, old);

		// Update the opinion value to the new one
		info.opinionChange(id, type, val);

		// Then get the current value of the object's opinion
		int value = 0;
		info.opinionGet(id, type, value);

		// Now, send the new and old value of the object's opinion for drawing the change effect
		_opinion[type].Effect(value, old);

		_prev = id;
	}
}

void PersonHandler::reset(const Common::String &id) {
	if (_prev != id) {
		using namespace pyrodactyl::people;
		_opinion[OPI_LIKE].reset();
		_opinion[OPI_RESPECT].reset();
		_opinion[OPI_FEAR].reset();
	}
}

void PersonHandler::setUI() {
	_img.setUI();
	_name.setUI();
	_dlbox.setUI();
	_jb.setUI();

	for (auto i = 0; i < pyrodactyl::people::OPI_TOTAL; ++i)
		_opinion[i].setUI();
}

} // End of namespace Crab
