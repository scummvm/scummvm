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


#include "crab/EventSeqGroup.h"

namespace Crab {

using namespace pyrodactyl::event;

void EventSeqGroup::EndSeq(const unsigned int &id) {
	seq.erase(id);
	end.push_back(id);
}

void EventSeqGroup::AddSeq(const unsigned int &id, std::string &path) {
	seq[id].Load(path);
}

bool EventSeqGroup::EventInProgress(const unsigned int &id) {
	return seq.count(id) > 0 && seq.at(id).EventInProgress();
}

GameEvent *EventSeqGroup::CurEvent(const unsigned int &id) {
	return seq.at(id).CurrentEvent();
}

void EventSeqGroup::NextEvent(const unsigned int &id, Info &info, const std::string &player_id,
							  std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq, const int choice) {
	return seq.at(id).NextEvent(info, player_id, result, end_seq, choice);
}

void EventSeqGroup::InternalEvents(Info &info) {
	for (auto it = seq.begin(); it != seq.end(); ++it)
		it->second.InternalEvents(info);
}

bool EventSeqGroup::ActiveSeq(unsigned int &active_seq) {
	for (auto i = seq.begin(); i != seq.end(); ++i)
		if (i->second.EventInProgress()) {
			active_seq = i->first;
			return true;
		}

	active_seq = UINT_MAX;
	return false;
}

void EventSeqGroup::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = end.begin(); i != end.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "end");
		child->value(gStrPool.Get(*i));
		root->append_node(child);
	}

	for (auto i = seq.begin(); i != seq.end(); ++i)
		i->second.SaveState(doc, root, gStrPool.Get(i->first));
}

void EventSeqGroup::LoadState(rapidxml::xml_node<char> *node) {
	for (rapidxml::xml_node<char> *i = node->first_node("end"); i != NULL; i = i->next_sibling("end"))
		EndSeq(StringToNumber<unsigned int>(i->value()));

	for (auto n = node->first_node("set"); n != NULL; n = n->next_sibling("set"))
		if (n->first_attribute("name") != NULL) {
			unsigned int id = StringToNumber<unsigned int>(n->first_attribute("name")->value());
			if (seq.count(id) > 0)
				seq[id].LoadState(n);
		}
}

} // End of namespace Crab
