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

#ifndef CRAB_GAMEEVENTINFO_H
#define CRAB_GAMEEVENTINFO_H

#include "crab/Inventory.h"
#include "crab/StatTemplate.h"
#include "crab/common_header.h"
#include "crab/journal.h"
#include "crab/person.h"

namespace pyrodactyl {
namespace event {
bool IsChar(char c);

class Info {
	// The characters in the game
	pyrodactyl::people::PersonMap people;

	// The stat templates used in declaring person objects
	pyrodactyl::stat::StatTemplates stem;

	// The variables set by the events so far
	typedef std::unordered_map<std::string, int> VarMap;
	VarMap var;

	// The last object player interacted with
	std::string lastobj;

	// Is the game Iron Man or not?
	// Iron man means only one save - no reloading
	bool ironman;

	// The player's current location
	struct PlayerLoc {
		// id of the player's current location
		std::string id;

		// The name of the player's current location
		std::string name;

		PlayerLoc() : id(""), name("") {}
	} loc;

	// This image changes to reflect the playable character
	int player_img;

	void LoadPeople(const std::string &filename);

public:
	// The player's one stop shop for objectives and lore
	pyrodactyl::ui::Journal journal;

	// The current player inventory
	pyrodactyl::ui::Inventory inv;

	// This structure keeps track of unread indicators
	struct UnreadData {
		bool inventory, journal, trait, map;

		UnreadData() {
			inventory = false;
			journal = false;
			trait = true;
			map = false;
		}
	} unread;

	// This is the variable corresponding to money that is drawn
	std::string money_var;

	// Has the player pressed the talk key
	bool TalkKeyDown;

	// Used so we only play one sound per event
	struct NotifySounds {
		bool notify, rep_inc, rep_dec;

		NotifySounds() {
			notify = false;
			rep_inc = false;
			rep_dec = false;
		}
	} sound;

	Info() { Init(); }
	~Info() {}

	void Init() {
		lastobj = "";
		ironman = false;
		player_img = 0;
		TalkKeyDown = false;
	}
	void Load(rapidxml::xml_node<char> *node);

	// Person related stuff
	void Type(const std::string &id, const pyrodactyl::people::PersonType &val);
	pyrodactyl::people::PersonType Type(const std::string &id);

	void State(const std::string &id, const pyrodactyl::people::PersonState &val);
	pyrodactyl::people::PersonState State(const std::string &id);

	// Opinion
	bool OpinionGet(const std::string &name, const pyrodactyl::people::OpinionType &type, int &val);
	void OpinionSet(const std::string &name, const pyrodactyl::people::OpinionType &type, int val);
	void OpinionChange(const std::string &name, const pyrodactyl::people::OpinionType &type, int val);

	// Stats
	bool StatGet(const std::string &name, const pyrodactyl::stat::StatType &type, int &num);
	void StatSet(const std::string &name, const pyrodactyl::stat::StatType &type, const int &num);
	void StatChange(const std::string &name, const pyrodactyl::stat::StatType &type, const int &num);

	// Variables
	bool VarGet(const std::string &name, int &val);
	void VarSet(const std::string &name, const std::string &val);
	void VarAdd(const std::string &name, const int &val);
	void VarSub(const std::string &name, const int &val);
	void VarMul(const std::string &name, const int &val);
	void VarDiv(const std::string &name, const int &val);
	void VarDel(const std::string &name);

	// The trait functions
	void TraitAdd(const std::string &per_id, const int &trait_id);
	void TraitDel(const std::string &per_id, const int &trait_id);

	// Player character button
	void PlayerImg(const int &val) { player_img = val; }
	int PlayerImg() { return player_img; }

	std::string CurLocID() { return loc.id; }
	void CurLocID(const std::string &id) { loc.id = id; }

	std::string CurLocName() { return loc.name; }
	void CurLocName(const std::string &name) { loc.name = name; }

	// Player stuff
	std::string LastPerson() { return lastobj; }
	void LastPerson(const std::string &name) { lastobj = name; }

	// Return the variable map for stuff like visibility checks
	VarMap &MapGet() { return var; }
	bool PersonGet(const std::string &id, pyrodactyl::people::Person &p);

	bool PersonValid(const std::string &id);
	pyrodactyl::people::Person &PersonGet(const std::string &id);

	// Is an object colliding with a trigger area
	bool CollideWithTrigger(const std::string &id, int rect_index);

	// Replace all #values with their appropriate names in a string
	void InsertName(std::string &msg);
	std::string GetName(const std::string &id);

	// Draw the inventory
	void InvDraw(const std::string &id) {
		if (var.count(money_var) > 0)
			inv.Draw(people.at(id), var.at(money_var));
		else
			inv.Draw(people.at(id), 0);
	}

	// Get whether game is iron man or not
	const bool IronMan() { return ironman; }
	void IronMan(const bool &val) { ironman = val; }
	void LoadIronMan(rapidxml::xml_node<char> *node) {
		std::string str;
		LoadStr(str, "diff", node);
		ironman = (str == "Iron Man");
	}

	void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace event
} // End of namespace pyrodactyl

#endif // CRAB_GAMEEVENTINFO_H
