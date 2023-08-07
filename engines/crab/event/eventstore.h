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

#include "crab/ui/StateButton.h"
#include "crab/animation/animation.h"
#include "crab/event/conversationdata.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
// This structure is responsible for storing the special data structures for events like replies, conversations
struct GameEventStore {
	// Data related to conversation events
	Common::Array<ConversationData> _con;

	// Data related to animations
	Common::Array<pyrodactyl::anim::Animation> _anim;

	// Data related to the tones of a character
	struct ToneData {
		Common::String _text;
	};

	// This sets the text the player sees as "tone" during the reply menu
	Common::Array<ToneData> _tone;

	// We need to change player character images when switching between characters
	Common::Array<pyrodactyl::ui::StateButtonImage> _img;

	// The set of traits for various characters
	Common::Array<pyrodactyl::people::Trait> _trait;

	// Steam UserStats interface
	// ISteamUserStats *m_pSteamUserStats;

	GameEventStore() {
		// Huge number to prevent lots of resizing and stuff
		_con.reserve(9999);

		_trait.reserve(120);
		_tone.reserve(120);
	}

	void clear() {
		_con.clear();
		_anim.clear();
	}

	void load(const Common::String &filename);

	void addConv(rapidxml::xml_node<char> *node, uint &index);
	void setAchievement(const int &id);
};

} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_EVENTSTORE_H
