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
	typedef Common::HashMap<uint, EventSequence> SeqMap;

	// The event sequences in this group
	SeqMap _seq;

	// The sequences that have ended in this group
	Common::Array<uint> _end;

public:
	EventSeqGroup(void) {}
	~EventSeqGroup(void) {}

	void addSeq(const uint &id, Common::String &path);
	void endSeq(const uint &id);
	bool eventInProgress(const uint &id);
	bool activeSeq(uint &activeSeq);

	GameEvent *curEvent(const uint &id);
	void nextEvent(const uint &id, Info &info, const Common::String &playerId, Common::Array<EventResult> &result,
				   Common::Array<EventSeqInfo> &endSeq, const int choice = -1);

	void internalEvents(Info &info);

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_EVENTSEQGROUP_H
