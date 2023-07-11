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

#include "crab/event/gameevent.h"

namespace Crab {

using namespace pyrodactyl::event;
using namespace pyrodactyl::people;

GameEvent::GameEvent() {
	id = 0;
	type = EVENT_DIALOG;
	special = 0;
	state = PST_NORMAL;
}

void GameEvent::load(rapidxml::xml_node<char> *node) {
	if (!LoadEventID(id, "id", node))
		id = 0;

	loadStr(title, "title", node);
	loadStr(dialog, "dialog", node);
	loadEnum(state, "state", node, false);

	Common::String Type;
	loadStr(Type, "type", node);

	if (Type == "dlg") {
		type = EVENT_DIALOG;
		special = 0;
	} else if (Type == "reply") {
		type = EVENT_REPLY;
		g_engine->_eventStore->addConv(node, special);
	} else if (Type == "animation") {
		type = EVENT_ANIM;
		loadNum(special, "anim", node);
	} else if (Type == "silent") {
		type = EVENT_SILENT;
		special = 0;
	} else if (Type == "text") {
		type = EVENT_TEXT;
		special = 0;
	} else {
		type = EVENT_SPLASH;
		special = 0;
	}

	trig.load(node);

	next.clear();
	for (rapidxml::xml_node<char> *i = node->first_node("next"); i != NULL; i = i->next_sibling("next"))
		if (i->first_attribute("id") != NULL)
			next.push_back(StringToNumber<EventID>(i->first_attribute("id")->value()));

	effect.clear();
	for (rapidxml::xml_node<char> *it = node->first_node("effect"); it != NULL; it = it->next_sibling("effect")) {
		Effect e;
		e.load(it);
		effect.push_back(e);
	}
}

} // End of namespace Crab
