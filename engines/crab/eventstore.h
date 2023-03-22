#pragma once

#include "StateButton.h"
#include "animation.h"
#include "common_header.h"
#include "conversationdata.h"

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