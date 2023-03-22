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

#ifndef CRAB_EVENTSTORE_H
#define CRAB_EVENTSTORE_H

#include "crab/StateButton.h"
#include "crab/animation.h"
#include "crab/common_header.h"
#include "crab/conversationdata.h"

namespace pyrodactyl {
namespace event {
// This structure is responsible for storing the special data structures for events like replies, conversations
struct GameEventStore {
	// Data related to conversation events
	std::vector<ConversationData> con;

	// Data related to animations
	std::vector<pyrodactyl::anim::Animation> anim;

	// Data related to the tones of a character
	struct ToneData {
		std::string text;
	};

	// This sets the text the player sees as "tone" during the reply menu
	std::vector<ToneData> tone;

	// We need to change player character images when switching between characters
	std::vector<pyrodactyl::ui::StateButtonImage> img;

	// The set of traits for various characters
	std::vector<pyrodactyl::people::Trait> trait;

	// Steam UserStats interface
	// ISteamUserStats *m_pSteamUserStats;

	GameEventStore() {
		// Huge number to prevent lots of resizing and stuff
		con.reserve(9999);

		trait.reserve(120);
		tone.reserve(120);
	}

	void Clear() {
		con.clear();
		anim.clear();
	}
	void Load(const std::string &filename);

	void AddConv(rapidxml::xml_node<char> *node, unsigned int &index);
	void SetAchievement(const int &id);
};

extern GameEventStore gEventStore;
} // End of namespace event
} // End of namespace pyrodactyl

#endif // CRAB_EVENTSTORE_H
