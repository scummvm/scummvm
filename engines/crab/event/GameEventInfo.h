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

#include "crab/ui/Inventory.h"
#include "crab/stat/StatTemplate.h"
#include "crab/common_header.h"
#include "crab/ui/journal.h"
#include "crab/people/person.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
bool IsChar(char c);

class Info {
	// The characters in the game
	pyrodactyl::people::PersonMap _people;

	// The stat templates used in declaring person objects
	pyrodactyl::stat::StatTemplates _stem;

	// The variables set by the events so far
	typedef Common::HashMap<Common::String, int> VarMap;
	VarMap _var;

	// The last object player interacted with
	Common::String _lastobj;

	// Is the game Iron Man or not?
	// Iron man means only one save - no reloading
	bool _ironman;

	// The player's current location
	struct PlayerLoc {
		// id of the player's current location
		Common::String _id;

		// The name of the player's current location
		Common::String _name;

		PlayerLoc() : _id(""), _name("") {}
	} _loc;

	// This image changes to reflect the playable character
	int _playerImg;

	void loadPeople(const Common::String &filename);

public:
	// The player's one stop shop for objectives and lore
	pyrodactyl::ui::Journal _journal;

	// The current player inventory
	pyrodactyl::ui::Inventory _inv;

	// This structure keeps track of unread indicators
	struct UnreadData {
		bool _inventory, _journal, _trait, _map;

		UnreadData() {
			_inventory = false;
			_journal = false;
			_trait = true;
			_map = false;
		}
	} _unread;

	// This is the variable corresponding to money that is drawn
	Common::String _moneyVar;

	// Has the player pressed the talk key
	bool _talkKeyDown;

	// Used so we only play one sound per event
	struct NotifySounds {
		bool _notify, _repInc, _repDec;

		NotifySounds() {
			_notify = false;
			_repInc = false;
			_repDec = false;
		}
	} _sound;

	Info() {
		Init();
	}

	~Info() {}

	void Init() {
		_lastobj = "";
		_ironman = false;
		_playerImg = 0;
		_talkKeyDown = false;
	}

	void load(rapidxml::xml_node<char> *node);

	// Person related stuff
	void type(const Common::String &id, const pyrodactyl::people::PersonType &val);
	pyrodactyl::people::PersonType type(const Common::String &id);

	void state(const Common::String &id, const pyrodactyl::people::PersonState &val);
	pyrodactyl::people::PersonState state(const Common::String &id);

	// Opinion
	bool opinionGet(const Common::String &name, const pyrodactyl::people::OpinionType &type, int &val);
	void opinionSet(const Common::String &name, const pyrodactyl::people::OpinionType &type, int val);
	void opinionChange(const Common::String &name, const pyrodactyl::people::OpinionType &type, int val);

	// Stats
	bool statGet(const Common::String &name, const pyrodactyl::stat::StatType &type, int &num);
	void statSet(const Common::String &name, const pyrodactyl::stat::StatType &type, const int &num);
	void statChange(const Common::String &name, const pyrodactyl::stat::StatType &type, const int &num);

	// Variables
	bool varGet(const Common::String &name, int &val);
	void varSet(const Common::String &name, const Common::String &val);
	void varAdd(const Common::String &name, const int &val);
	void varSub(const Common::String &name, const int &val);
	void varMul(const Common::String &name, const int &val);
	void varDiv(const Common::String &name, const int &val);
	void varDel(const Common::String &name);

	// The trait functions
	void traitAdd(const Common::String &perId, const int &traitId);
	void traitDel(const Common::String &perId, const int &traitId);

	// Player character button
	void playerImg(const int &val) {
		_playerImg = val;
	}

	int playerImg() {
		return _playerImg;
	}

	Common::String curLocID() {
		return _loc._id;
	}

	void curLocID(const Common::String &id) {
		_loc._id = id;
	}

	Common::String curLocName() {
		return _loc._name;
	}

	void curLocName(const Common::String &name) {
		_loc._name = name;
	}

	// Player stuff
	Common::String lastPerson() {
		return _lastobj;
	}

	void lastPerson(const Common::String &name) {
		_lastobj = name;
	}

	// Return the variable map for stuff like visibility checks
	VarMap &mapGet() {
		return _var;
	}

	bool personGet(const Common::String &id, pyrodactyl::people::Person &p);

	bool personValid(const Common::String &id);
	pyrodactyl::people::Person &personGet(const Common::String &id);

	// Is an object colliding with a trigger area
	bool collideWithTrigger(const Common::String &id, int rectIndex);

	// Replace all #values with their appropriate names in a string
	void insertName(Common::String &msg);
	Common::String getName(const Common::String &id);

	// Draw the inventory
	void invDraw(const Common::String &id) {
		if (_var.contains(_moneyVar))
			_inv.draw(_people[id], _var[_moneyVar]);
		else
			_inv.draw(_people[id], 0);
	}

	// Get whether game is iron man or not
	bool ironMan() {
		return _ironman;
	}

	void ironMan(const bool &val) {
		_ironman = val;
	}

	void loadIronMan(rapidxml::xml_node<char> *node) {
		Common::String str;
		loadStr(str, "diff", node);
		_ironman = (str == "Iron Man");
	}

	void saveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_GAMEEVENTINFO_H
