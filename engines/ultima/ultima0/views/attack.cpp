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

#include "ultima/ultima0/views/attack.h"
#include "ultima/ultima0/data/data.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Views {

Attack::Attack() : View("Attack") {
	setBounds(Gfx::TextRect(1, 22, 26, 24));
}

bool Attack::msgFocus(const FocusMessage &msg) {
	_mode = WHICH_WEAPON;
	_message = "";
	MetaEngine::setKeybindingMode(KBMODE_MENUS);
	return true;
}

bool Attack::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);
	return true;
}

void Attack::draw() {
	auto s = getSurface();
	s.clear();

	if (_mode == AMULET) {
		s.writeString(Common::Point(0, 0), "1] Ladder Up");
		s.writeString(Common::Point(0, 1), "2] Ladder Down");
		s.writeString(Common::Point(0, 2), "3] Attack Monster");
		s.writeString(Common::Point(0, 3), "4] Bad Magic");
		return;
	}

	s.writeString(Common::Point(1, 1), "Which Weapon? ");
	if (_mode != WHICH_WEAPON)
		s.writeString(_weapon <= 0 ? "Hands" : OBJECT_INFO[_weapon]._name);

	if (!_message.empty())
		s.writeString(Common::Point(1, 2), _message);
}

bool Attack::msgKeypress(const KeypressMessage &msg) {
	if (_mode == WHICH_WEAPON) {
		selectObject(-1);
		return true;
	}

	return false;
}

bool Attack::msgAction(const ActionMessage &msg) {
	int objNum;

	switch (_mode) {
	case WHICH_WEAPON:
		// Check for object selection, anything but food
		objNum = -1;
		for (uint i = OB_RAPIER; i < MAX_OBJ; ++i) {
			if (msg._action == OBJECT_INFO[i]._action) {
				objNum = i;
				break;
			}
		}

		selectObject(objNum);
		break;

	case AMULET:
		if (msg._action >= KEYBIND_AMULET1 && msg._action <= KEYBIND_AMULET4) {
			selectMagic(msg._action - KEYBIND_AMULET1);
		}
		break;

	case THROW_SWING:
		if (msg._action == KEYBIND_THROW) {
			_message += "Throw\n";
			attackMissile();
		} else if (msg._action == KEYBIND_SWING) {
			_message += "Swing\n";
			attackWeapon();
		}
		break;

	default:
		break;
	}

	return true;
}

void Attack::selectObject(int objNum) {
	auto &player = g_engine->_player;

	_weapon = objNum;
	_damage = (objNum <= 0) ? 0 : OBJECT_INFO[objNum]._maxDamage;

	// Must own an object
	if (player._object[_weapon] == 0) {
		showMessage("Not owned.");
		return;
	}

	// Mages are limited
	if (player._class == 'M' && (objNum == OB_BOW || objNum == OB_RAPIER)) {
		showMessage("Mages can't use that.");
		return;
	}

	// Use an amulet
	if (objNum == OB_AMULET) {
		if (player._class == 'M') {
			// Mages can properly select the magic to use
			_mode = AMULET;

			// Amulet selection requires all four lines
			_bounds = Gfx::TextRect(0, 22, 26, 24);

			redraw();
		} else {
			// Fighters get a random effect
			selectMagic(g_engine->getRandomNumber(3));
		}
		return;
	}

	if (objNum == OB_BOW) {
		attackMissile();
	} else if (objNum == OB_AXE) {
		_mode = THROW_SWING;
		_message += "Throw or Swing ? ";
		redraw();
	} else {
		attackWeapon();
	}
}

void Attack::selectMagic(int magicNum) {
	auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;
	int i;

	// Last charge?
	Common::String msg;
	if (urand() % 5 == 0) {
		msg = "Last charge on this Amulet.\n";
		player._object[OB_AMULET]--;
	}

	switch (magicNum) {
	case 0:
		// Ladder up
		dungeon._map[player._dungeonPos.x][player._dungeonPos.y] = DT_LADDERUP;
		break;
	case 1:
		// Ladder down
		dungeon._map[player._dungeonPos.x][player._dungeonPos.y] = DT_LADDERDN;
		break;
	case 2:
		// Amulet Attack
		attackMissile();
		break;
	case 3:
		// Bad Magic
		switch (urand() % 3) {
		case 0:
			msg += "You have been turned into a Toad.\n";
			for (i = AT_STRENGTH; i <= AT_WISDOM; i++)
				player._attr[i] = 3;
			break;
		case 1:
			msg += "You have been turned into a Lizard Man.\n";
			for (i = AT_HP; i <= AT_WISDOM; i++)
				player._attr[i] = floor(player._attr[i] * 5 / 2);
			break;
		case 2:
			msg += "Backfire !!\n";
			player._attr[AT_HP] = floor(player._attr[AT_HP]) / 2;
			break;
		}
		break;
	}

	if (msg.empty()) {
		timeout();
	} else {
		redraw();
		delaySeconds(1);
	}
}

void Attack::attackMissile() {
	const auto &dungeon = g_engine->_dungeon;
	auto &player = g_engine->_player;
	Common::Point c1, c = player._dungeonPos;
	int Dist = -1;
	
	// A maximum distance of 5
	for (int y = 0; y < 5; y++) {
		c += player._dungeonDir;
		int n = dungeon.findMonster(c);		// Monster there ?
		if (n >= 0) {
			c1 = c;
			Dist = n;
		}

		// If wall, or door, stop
		if (!ISDRAWOPEN(dungeon._map[c.x][c.y]))
			break;
	}

	if (Dist < 0) {
		// Hit nothing
		_message += "You missed !!\n";
		_mode = DONE;
		delaySeconds(1);
	} else {
		attackHitMonster(c1);
	}
}

void Attack::attackWeapon() {
	const auto &player = g_engine->_player;
	Common::Point c = player._dungeonPos + player._dungeonDir;
	attackHitMonster(c);
}

void Attack::attackHitMonster(const Common::Point &c) {
	auto &player = g_engine->_player;
	auto &dungeon = g_engine->_dungeon;
	int n = 0, monNum, damage;
	MonsterEntry *m = nullptr;

	// Is there a monster there ?
	monNum = dungeon.findMonster(c);
	if (monNum >= 0) {
		// Set up a pointer
		m = &dungeon._monsters[monNum];
		n = m->_type;
	}

	// Get weaponry info
	damage = 0;
	if (_weapon >= 0 && _weapon != OB_AMULET)
		damage = OBJECT_INFO[_weapon]._maxDamage;
	if (_weapon == OB_AMULET)
		// Amulet Special Case
		damage = 10 + player._level;

	// If no, or not dexterous
	if (monNum < 0 || player._attr[AT_DEXTERITY] - ((int)urand() % 25) < n + player._level) {
		// Then a miss.
		_message += "\nMissed!";
		_mode = DONE;
		redraw();
		delaySeconds(1);
		return;
	}

	// Scored a hit
	_message += "Hit !!!\n";

	// Calculate HPs lost
	n = 0;
	if (damage > 0)
		n = (urand() % damage);
	n = n + player._attr[AT_STRENGTH] / 5;
	m->_strength = m->_strength - n;			// Lose them

	if (m->_strength < 0)
		m->_strength = 0;
	_message += Common::String::format("%s's Hit\nPoints now %d.\n",
		MONSTER_INFO[m->_type]._name, m->_strength);

	// Killed it ?
	if (m->_strength == 0) {
		m->_alive = false;						// It has ceased to be
		int gold = (m->_type + player._level);	// Amount of gold
		_message += Common::String::format("You get %d pieces of eight.\n", gold);
		player._attr[AT_GOLD] += gold;

		player._hpGain += (m->_type * player._level) / 2;	// Calculate Gain

		if (m->_type == player._task)						// Check done LB's task
			player._taskCompleted = true;
	}

	_mode = DONE;
	redraw();
	delaySeconds(1);
}

void Attack::showMessage(const Common::String &msg) {
	_message = msg;
	_mode = DONE;
	redraw();
	delaySeconds(1);
}

void Attack::timeout() {
	close();
	g_events->send("Dungeon", GameMessage("ENDOFTURN"));
}

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
