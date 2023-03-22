#include "eventstore.h"
#include "stdafx.h"

namespace pyrodactyl {
namespace event {
GameEventStore gEventStore;
}
} // End of namespace pyrodactyl

using namespace pyrodactyl::event;

void GameEventStore::AddConv(rapidxml::xml_node<char> *node, unsigned int &index) {
	ConversationData c;
	if (NodeValid("talk", node))
		c.Load(node->first_node("talk"));

	index = con.size();
	con.push_back(c);
}

void GameEventStore::Load(const std::string &filename) {
	// Request current user stats from Steam
	// m_pSteamUserStats = SteamUserStats();

	// if (m_pSteamUserStats != nullptr)
	// m_pSteamUserStats->RequestCurrentStats();

	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node("store");

		if (NodeValid("animations", node)) {
			rapidxml::xml_node<char> *animnode = node->first_node("animations");
			for (auto n = animnode->first_node("anim"); n != NULL; n = n->next_sibling("anim"))
				anim.push_back(n);
		}

		if (NodeValid("tones", node)) {
			rapidxml::xml_node<char> *tonenode = node->first_node("tones");
			for (auto n = tonenode->first_node("tone"); n != NULL; n = n->next_sibling("tone")) {
				ToneData dat;
				LoadStr(dat.text, "text", n);
				tone.push_back(dat);
			}
		}

		if (NodeValid("images", node)) {
			rapidxml::xml_node<char> *imgnode = node->first_node("images");
			for (auto n = imgnode->first_node("img"); n != NULL; n = n->next_sibling("img"))
				img.push_back(n);
		}

		if (NodeValid("traits", node)) {
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
				trait.push_back(n);
		}
	}
}

void GameEventStore::SetAchievement(const int &id) {
	/*
	if (m_pSteamUserStats != nullptr)
	{
		if (id == 0)
			m_pSteamUserStats->SetAchievement("a0");
		else
			m_pSteamUserStats->SetAchievement(trait.at(id).id_str.c_str());

		m_pSteamUserStats->StoreStats();
	}
	*/
}