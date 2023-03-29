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

#ifndef CRAB_EVENTSEQGROUP_H
#define CRAB_EVENTSEQGROUP_H
#include "crab/common_header.h"

#include "crab/event/EventSequence.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
class EventSeqGroup {
	typedef std::unordered_map<unsigned int, EventSequence> SeqMap;

	// The event sequences in this group
	SeqMap seq;

	// The sequences that have ended in this group
	std::vector<unsigned int> end;

public:
	EventSeqGroup(void) {}
	~EventSeqGroup(void) {}

	void AddSeq(const unsigned int &id, std::string &path);
	void EndSeq(const unsigned int &id);
	bool EventInProgress(const unsigned int &id);
	bool ActiveSeq(unsigned int &active_seq);

	GameEvent *CurEvent(const unsigned int &id);
	void NextEvent(const unsigned int &id, Info &info, const std::string &player_id, std::vector<EventResult> &result,
				   std::vector<EventSeqInfo> &end_seq, const int choice = -1);

	void InternalEvents(Info &info);

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_EVENTSEQGROUP_H
