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


#include "crab/event/EventSequence.h"

namespace Crab {

using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void EventSequence::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node("events");
		for (auto n = node->first_node("event"); n != NULL; n = n->next_sibling("event")) {
			GameEvent e(n);
			_events.push_back(e);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Check for events happening
//------------------------------------------------------------------------
void EventSequence::internalEvents(pyrodactyl::event::Info &info) {
	for (auto nxe = _next.begin(); nxe != _next.end(); ++nxe)
		if (*nxe < _events.size()) {
			if (_events[*nxe]._trig.evaluate(info)) {
				_eventInProgress = true;
				_cur = *nxe;
				break;
			}
		}
}

//------------------------------------------------------------------------
// Purpose: Go to next event
//------------------------------------------------------------------------
void EventSequence::nextEvent(pyrodactyl::event::Info &info, const Common::String &player_id, Common::Array<EventResult> &result,
							  Common::Array<EventSeqInfo> &end_seq, int NextEventChoice) {
	bool sync = false;
	_eventInProgress = false;

	// Execute all effects associated with the event
	for (auto i = _events[_cur]._effect.begin(); i != _events[_cur]._effect.end(); ++i)
		if (i->execute(info, player_id, result, end_seq))
			sync = true;

	// Play a notification sound
	using namespace pyrodactyl::music;

	if (info._sound._repDec)
		g_engine->_musicManager->playEffect(g_engine->_musicManager->_repDec, 0);
	else if (info._sound._repInc)
		g_engine->_musicManager->playEffect(g_engine->_musicManager->_repInc, 0);
	else if (info._sound._notify)
		g_engine->_musicManager->playEffect(g_engine->_musicManager->_notify, 0);

	info._sound._notify = false;
	info._sound._repDec = false;
	info._sound._repInc = false;

	if (!result.empty() || sync) {
		EventResult r;
		r._type = ER_SYNC;
		result.push_back(r);
	}

	// Clear the next event list
	_next.clear();

	// Add the next event to the event list
	if (NextEventChoice != -1)
		_next.push_back(NextEventChoice);
	else {
		for (auto i = _events[_cur]._next.begin(); i != _events[_cur]._next.end(); ++i)
			_next.push_back(*i);
	}
}

//------------------------------------------------------------------------
// Purpose: Save the state of the object
//------------------------------------------------------------------------
void EventSequence::saveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *seqnode = doc.allocate_node(rapidxml::node_element, "set");

	// Write current event id and name to node
	seqnode->append_attribute(doc.allocate_attribute("name", name));
	seqnode->append_attribute(doc.allocate_attribute("current", gStrPool->Get(_cur)));

	// Prepare strings of next events and write them
	for (unsigned int i = 0; i < _next.size(); i++) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "next");
		child->value(gStrPool->Get(_next[i]));
		seqnode->append_node(child);
	}

	root->append_node(seqnode);
}

//------------------------------------------------------------------------
// Purpose: Load the state of the object
//------------------------------------------------------------------------
void EventSequence::loadState(rapidxml::xml_node<char> *node) {
	rapidxml::xml_attribute<char> *curid = node->first_attribute("current");
	if (curid != NULL)
		_cur = stringToNumber<unsigned int>(curid->value());

	_next.clear();
	for (auto n = node->first_node("next"); n != NULL; n = n->next_sibling("next"))
		_next.push_back(stringToNumber<unsigned int>(n->value()));

	if (_next.empty())
		_next.push_back(0);
}

} // End of namespace Crab
