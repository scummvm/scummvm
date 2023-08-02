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
	_id = 0;
	_type = EVENT_DIALOG;
	_special = 0;
	_state = PST_NORMAL;
}

void GameEvent::load(rapidxml::xml_node<char> *node) {
	if (!LoadEventID(_id, "id", node))
		_id = 0;

	loadStr(_title, "title", node);
	loadStr(_dialog, "dialog", node);
	loadEnum(_state, "state", node, false);

	Common::String type;
	loadStr(type, "type", node);

	if (type == "dlg") {
		_type = EVENT_DIALOG;
		_special = 0;
	} else if (type == "reply") {
		_type = EVENT_REPLY;
		g_engine->_eventStore->addConv(node, _special);
	} else if (type == "animation") {
		_type = EVENT_ANIM;
		loadNum(_special, "anim", node);
	} else if (type == "silent") {
		_type = EVENT_SILENT;
		_special = 0;
	} else if (type == "text") {
		_type = EVENT_TEXT;
		_special = 0;
	} else {
		_type = EVENT_SPLASH;
		_special = 0;
	}

	_trig.load(node);

	_next.clear();
	for (rapidxml::xml_node<char> *i = node->first_node("next"); i != nullptr; i = i->next_sibling("next"))
		if (i->first_attribute("id") != nullptr)
			_next.push_back(stringToNumber<EventID>(i->first_attribute("id")->value()));

	_effect.clear();
	for (rapidxml::xml_node<char> *it = node->first_node("effect"); it != nullptr; it = it->next_sibling("effect")) {
		Effect e;
		e.load(it);
		_effect.push_back(e);
	}
}

} // End of namespace Crab
