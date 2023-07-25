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


#include "crab/event/EventSeqGroup.h"

namespace Crab {

using namespace pyrodactyl::event;

void EventSeqGroup::endSeq(const unsigned int &id) {
	_seq.erase(id);
	_end.push_back(id);
}

void EventSeqGroup::addSeq(const unsigned int &id, Common::String &path) {
	_seq[id].load(path);
}

bool EventSeqGroup::eventInProgress(const unsigned int &id) {
	return _seq.contains(id) > 0 && _seq[id].eventInProgress();
}

GameEvent *EventSeqGroup::curEvent(const unsigned int &id) {
	return _seq[id].currentEvent();
}

void EventSeqGroup::nextEvent(const unsigned int &id, Info &info, const Common::String &playerId,
							  Common::Array<EventResult> &result, Common::Array<EventSeqInfo> &endSeq, const int choice) {
	return _seq[id].nextEvent(info, playerId, result, endSeq, choice);
}

void EventSeqGroup::internalEvents(Info &info) {
	for (auto it = _seq.begin(); it != _seq.end(); ++it)
		it->_value.internalEvents(info);
}

bool EventSeqGroup::activeSeq(unsigned int &activeSeq) {
	for (auto i = _seq.begin(); i != _seq.end(); ++i)
		if (i->_value.eventInProgress()) {
			activeSeq = i->_key;
			return true;
		}

	activeSeq = UINT_MAX;
	return false;
}

void EventSeqGroup::saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = _end.begin(); i != _end.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "end");
		child->value(g_engine->_stringPool->Get(*i));
		root->append_node(child);
	}

	for (auto i = _seq.begin(); i != _seq.end(); ++i)
		i->_value.saveState(doc, root, g_engine->_stringPool->Get(i->_key));
}

void EventSeqGroup::loadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *i = node->first_node("end"); i != NULL; i = i->next_sibling("end"))
		endSeq(stringToNumber<unsigned int>(i->value()));

	for (auto n = node->first_node("set"); n != NULL; n = n->next_sibling("set"))
		if (n->first_attribute("name") != NULL) {
			unsigned int id = stringToNumber<unsigned int>(n->first_attribute("name")->value());
			if (_seq.contains(id) > 0)
				_seq[id].loadState(n);
		}
}

} // End of namespace Crab
