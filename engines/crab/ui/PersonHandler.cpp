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
		dlbox.load(node->first_node("dialog"));

	if (nodeValid("opinion", node)) {
		rapidxml::xml_node<char> *opnode = node->first_node("opinion");

		if (nodeValid("friendship", opnode))
			opinion[OPI_LIKE].load(opnode->first_node("friendship"));

		if (nodeValid("respect", opnode))
			opinion[OPI_RESPECT].load(opnode->first_node("respect"));

		if (nodeValid("fear", opnode))
			opinion[OPI_FEAR].load(opnode->first_node("fear"));
	}

	if (nodeValid("image", node)) {
		rapidxml::xml_node<char> *imgnode = node->first_node("image");
		img.load(imgnode);

		if (nodeValid("sprite_align", imgnode))
			sprite_align.load(imgnode->first_node("sprite_align"));
	}

	if (nodeValid("name", node))
		name.load(node->first_node("name"));

	if (nodeValid("journal", node))
		jb.load(node->first_node("journal"));
}

void PersonHandler::draw(pyrodactyl::event::Info &info, pyrodactyl::event::GameEvent *Event, const Common::String &person_id,
						 const bool &player, pyrodactyl::anim::Sprite *s) {
	// Draw the dialog box background
	dlbox.draw(player);

	if (s != NULL) {
		Rect r = s->DialogClip(Event->state);
		int x = img.x, y = img.y;

		if (sprite_align.x == ALIGN_CENTER)
			x -= r.w / 2;
		else if (sprite_align.x == ALIGN_RIGHT)
			x -= r.w;

		if (sprite_align.y == ALIGN_CENTER)
			y -= r.h / 2;
		else if (sprite_align.y == ALIGN_RIGHT)
			y -= r.h;

		g_engine->_imageManager->draw(x, y, s->Img(), &r);
	}

	if (info.PersonValid(person_id)) {
		name.draw(info.PersonGet(person_id).name);

		if (!player) {
			opinion[OPI_LIKE].draw(info.PersonGet(person_id).opinion.val[OPI_LIKE], OPINION_MAX);
			opinion[OPI_RESPECT].draw(info.PersonGet(person_id).opinion.val[OPI_RESPECT], OPINION_MAX);
			opinion[OPI_FEAR].draw(info.PersonGet(person_id).opinion.val[OPI_FEAR], OPINION_MAX);
		}
	}

	// Draw the journal button
	jb.draw();

	// Draw the dialog box text
	dlbox.draw(info, Event->dialog);
}

bool PersonHandler::HandleCommonEvents(const Common::Event &Event) {
	opinion[OPI_LIKE].HandleEvents(Event);
	opinion[OPI_RESPECT].HandleEvents(Event);
	opinion[OPI_FEAR].HandleEvents(Event);

	if (jb.HandleEvents(Event) == BUAC_LCLICK) {
		// User wants to open their journal
		show_journal = true;
		return true;
	}

	return false;
}

bool PersonHandler::HandleDlboxEvents(const Common::Event &Event) {
	return dlbox.HandleEvents(Event);
}

#if 0
bool PersonHandler::HandleCommonEvents(const SDL_Event &Event) {
	opinion[OPI_LIKE].HandleEvents(Event);
	opinion[OPI_RESPECT].HandleEvents(Event);
	opinion[OPI_FEAR].HandleEvents(Event);

	if (jb.HandleEvents(Event) == BUAC_LCLICK) {
		// User wants to open their journal
		show_journal = true;
		return true;
	}

	return false;
}

bool PersonHandler::HandleDlboxEvents(const SDL_Event &Event) {
	return dlbox.HandleEvents(Event);
}
#endif

void PersonHandler::internalEvents(const pyrodactyl::people::PersonState &state, pyrodactyl::anim::Sprite *s) {
	if (s != NULL)
		s->DialogUpdateClip(state);
}

void PersonHandler::OpinionChange(pyrodactyl::event::Info &info, const Common::String &id, const pyrodactyl::people::OpinionType &type, const int &val) {
	if (info.PersonValid(id)) {
		// First, get the value of the object's opinion
		int old = 0;
		info.OpinionGet(id, type, old);

		// Update the opinion value to the new one
		info.OpinionChange(id, type, val);

		// Then get the current value of the object's opinion
		int value = 0;
		info.OpinionGet(id, type, value);

		// Now, send the new and old value of the object's opinion for drawing the change effect
		opinion[type].Effect(value, old);

		prev = id;
	}
}

void PersonHandler::reset(const Common::String &id) {
	if (prev != id) {
		using namespace pyrodactyl::people;
		opinion[OPI_LIKE].reset();
		opinion[OPI_RESPECT].reset();
		opinion[OPI_FEAR].reset();
	}
}

void PersonHandler::SetUI() {
	img.SetUI();
	name.SetUI();
	dlbox.SetUI();
	jb.SetUI();

	for (auto i = 0; i < pyrodactyl::people::OPI_TOTAL; ++i)
		opinion[i].SetUI();
}

} // End of namespace Crab
