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

#include "mm/mm1/views/combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

Combat::Combat() : TextView("Combat") {
}

void Combat::setMode(Mode newMode) {
	_mode = newMode;

	if (newMode == SELECT_OPTION) {
		_option = OPTION_NONE;
		MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_COMBAT);
	} else {
		MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	}

	if (_mode == MONSTER_SPELL)
		// Make a copy of monster spell 
		_monsterSpellLines = getMonsterSpellMessage();

	redraw();
}

void Combat::disableAttacks() {
	_allowFight = false;
	_allowShoot = false;
	_allowCast = false;
	_allowAttack = false;
}

bool Combat::msgFocus(const FocusMessage &msg) {
	// Clear combat data
	clear();

	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_COMBAT);

	loadArrays();
	setupCanAttacks();
	setupHandicap();

	combatLoop();

	return true;
}

bool Combat::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

void Combat::draw() {
	switch (_mode) {
	case NEXT_ROUND:
		resetBottom();
		highlightNextRound();
		delaySeconds(1);
		return;
	case MONSTER_ADVANCES:
		writeString(0, 20, _monsterName);
		writeString(STRING["dialogs.combat.advances"]);
		writeSpaces(30);
		writeRound();
		writeMonsters();
		delaySeconds(2);
		return;
	case MONSTERS_AFFECTED:
		writeMonsterEffects();
		delaySeconds(3);
		return;
	case MONSTER_FLEES:
	case MONSTER_WANDERS:
		writeMonsterAction();
		delaySeconds(3);
		return;
	case MONSTER_SPELL:
		writeMonsterSpell();
		delaySeconds(2);
		return;
	case CHAR_ATTACKS:
		writeMonsters();
		writeCharAttackDamage();
		delaySeconds(3);
		return;
	default:
		break;
	}

	clearSurface();
	writeStaticContent();
	writeHandicap();
	writeRound();
	writePartyNumbers();
	writeMonsters();
	writeParty();

	switch (_mode) {
	case SELECT_OPTION:
		writeOptions();
		break;

	case DEFEATED_MONSTERS:
		writeDefeat();
		Sound::sound2(SOUND_3);
		delaySeconds(3);
		break;

	default:
		break;
	}
}

void Combat::timeout() {
	switch (_mode) {
	case NEXT_ROUND:
		nextRound2();
		return;
	case MONSTER_ADVANCES:
		nextRound3();
		return;
	case MONSTERS_AFFECTED:
	case CHAR_ATTACKS:
		combatLoop();
		return;
	case MONSTER_FLEES:
		checkMonsterSpells();
		return;
	case MONSTER_WANDERS:
		writeParty();
		writeMonsters();
		checkParty();
		return;
	case MONSTER_SPELL:
		checkMonsterSpellDone();
		return;
	case DEFEATED_MONSTERS: {
		auto &spells = g_globals->_activeSpells;
		spells._s.bless = 0;
		spells._s.invisbility = 0;
		spells._s.shield = 0;
		spells._s.power_shield = 0;

		close();
		g_events->send("Game", GameMessage("UPDATE"));
		return;
	}
	default:
		 break;
	}

//	redraw();
}

bool Combat::msgKeypress(const KeypressMessage &msg) {
	if (_mode == SELECT_OPTION && _option != OPTION_NONE) {
		if (msg.keycode == Common::KEYCODE_ESCAPE) {
			combatLoop();
			return true;
		}

		switch (_option) {
		case OPTION_FIGHT:
		case OPTION_SHOOT:
			if (msg.keycode >= Common::KEYCODE_a &&
				msg.keycode < (int)(Common::KEYCODE_a + _fightCount)) {
				if (_option == OPTION_FIGHT)
					fightMonster(msg.keycode - Common::KEYCODE_a);
				else
					shootMonster(msg.keycode - Common::KEYCODE_a);
			}
			break;

		case OPTION_DELAY:
			if (msg.keycode >= Common::KEYCODE_0 &&
				msg.keycode <= Common::KEYCODE_9) {
				g_globals->_delay = msg.keycode - Common::KEYCODE_0;
				combatLoop();
			}
			break;

		default:
			break;
		}
	}

	return true;
}

bool Combat::msgAction(const ActionMessage &msg) {
	if (_mode != SELECT_OPTION || _option != OPTION_NONE)
		return false;

	switch (msg._action) {
	case KEYBIND_VIEW_PARTY1:
	case KEYBIND_VIEW_PARTY2:
	case KEYBIND_VIEW_PARTY3:
	case KEYBIND_VIEW_PARTY4:
	case KEYBIND_VIEW_PARTY5:
	case KEYBIND_VIEW_PARTY6: {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;
		if (charNum < _party.size()) {
			g_globals->_currCharacter = _party[charNum];
			addView("CharacterInfo");
			return true;
		}
		break;
	}

	case KEYBIND_COMBAT_ATTACK:
		attack();
		break;
	case KEYBIND_COMBAT_BLOCK:
		block();
		break;
	case KEYBIND_COMBAT_CAST:
		cast();
		break;
	case KEYBIND_DELAY:
		delay();
		break;
	case KEYBIND_COMBAT_EXCHANGE:
		exchange();
		break;
	case KEYBIND_COMBAT_FIGHT:
		fight();
		break;
	case KEYBIND_PROTECT:
		addView("Protect");
		break;
	case KEYBIND_QUICKREF:
		addView("QuickRef");
		break;
	case KEYBIND_COMBAT_RETREAT:
		retreat();
		break;
	case KEYBIND_COMBAT_SHOOT:
		shoot();
		break;
	case KEYBIND_COMBAT_USE:
		use();
		break;
	default:
		// TODO: Character and quickref views
		break;
	}

	return true;
}

void Combat::writeOptions() {
	resetBottom();

	switch (_option) {
	case OPTION_NONE:
		writeAllOptions();
		break;
	case OPTION_DELAY:
		writeDelaySelect();
		break;
	case OPTION_FIGHT:
		writeFightSelect();
		break;
	case OPTION_SHOOT:
		writeShootSelect();
		break;
	default:
		break;
	}
}

void Combat::writeAllOptions() {
	writeString(0, 20, STRING["dialogs.combat.options_for"]);
	writeString(0, 22, g_globals->_currCharacter->_name);

	switch (_currentChar) {
	case 0:
		writeChar(3, 3, (unsigned char)'\xB1');
		break;
	case 1:
		writeChar(7, 3, (unsigned char)'\xB2');
		break;
	case 2:
		writeChar(3, 4, (unsigned char)'\xB3');
		break;
	case 3:
		writeChar(7, 4, (unsigned char)'\xB4');
		break;
	case 4:
		writeChar(3, 5, (unsigned char)'\xB5');
		break;
	case 5:
		writeChar(7, 5, (unsigned char)'\xB6');
		break;
	default:
		break;
	}

	bool canAttack = _canAttack[_currentChar];
	if (canAttack) {
		writeAttackOptions();
		_allowAttack = true;
		_allowFight = true;

		// Archers can always attack
		canAttack = g_globals->_currCharacter->_class == ARCHER;
	}
	if (canAttack && g_globals->_currCharacter->_missileAttr) {
		_allowShoot = true;
		writeShootOption();
	}

	if (g_globals->_currCharacter->_sp._current) {
		writeCastOption();
		_allowCast = true;
	}

	writeString(16, 22, STRING["dialogs.combat.exchange_use"]);
	writeString(16, 23, STRING["dialogs.combat.retreat_block"]);
}

void Combat::writeDelaySelect() {
	resetBottom();
	writeString(0, 0, STRING["dialogs.combat.set_delay"]);
	writeString(0, 26, Common::String::format(
		STRING["dialogs.combat.delay_currently"].c_str(),
		g_globals->_delay));
	escToGoBack(0, 3);
}

void Combat::writeFightSelect() {
	_fightCount = MIN(_attackerVal, (int)_monsterList.size());

	writeString(10, 0, Common::String::format(
		STRING["dialogs.combat.fight_which"].c_str(), '@' + _fightCount));
	escToGoBack(12, 3);
}

void Combat::writeShootSelect() {
	_fightCount = MIN(_attackerVal, (int)_monsterList.size());

	writeString(10, 0, Common::String::format(
		STRING["dialogs.combat.shoot_which"].c_str(), '@' + _fightCount));
	escToGoBack(12, 3);
}

void Combat::writeAttackOptions() {
	writeString(16, 20, STRING["dialogs.combat.attack"]);
	writeString(16, 21, STRING["dialogs.combat.fight"]);
}

void Combat::writeCastOption() {
	writeString(30, 21, STRING["dialogs.combat.cast"]);
}

void Combat::writeShootOption() {
	writeString(30, 20, STRING["dialogs.combat.shoot"]);
}

void Combat::resetBottom() {
	clearLines(20, 24);
	_allowFight = _allowShoot = false;
	_allowCast = _allowAttack = false;
}

void Combat::writeStaticContent() {
	writeString(0, 0, STRING["dialogs.combat.combat"]);
	writeString(0, 1, STRING["dialogs.combat.round"]);
	writeString(0, 7, STRING["dialogs.combat.delay"]);
	writeString(0, 8, STRING["dialogs.combat.protect"]);
	writeString(0, 9, STRING["dialogs.combat.quickref"]);
	writeString(0, 10, STRING["dialogs.combat.view_char"]);
	writeString(0, 12, STRING["dialogs.combat.handicap"]);

	_textPos = Common::Point(0, 15);
	for (int i = 0; i < 40; ++i)
		writeChar('-');
}

void Combat::writeHandicap() {
	writeString(0, 13, "          ");
	_textPos.x = 0;

	switch (_handicap) {
	case HANDICAP_EVEN:
		writeString(STRING["dialogs.combat.even"]);
		break;
	case HANDICAP_PARTY:
		writeString(STRING["dialogs.combat.party_plus"]);
		writeNumber(_handicap4);
		break;
	case HANDICAP_MONSTER:
		writeString(STRING["dialogs.combat.monster_plus"]);
		writeNumber(_handicap4);
		break;
	}
}

void Combat::writeRound() {
	writeNumber(7, 1, _roundNum);
}

void Combat::writePartyNumbers() {
	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		writeChar(2 + 4 * (i % 2), 3 + (i / 2),
			_canAttack[i] ? '+' : ' ');
		writeChar('1' + i);
	}
}

void Combat::writeMonsters() {
	if (_monsterList.empty()) {
		_textPos = Common::Point(10, 0);
		writeSpaces(30);
	} else {
		for (int i = 0; i < (int)_monsterList.size(); ++i) {
			_textPos = Common::Point(11, i);
			writeChar(i < _attackerVal ? '+' : ' ');
			writeChar('A' + i);
			writeString(") ");
			writeString(_monsterList[i]._name);
			writeMonsterStatus(i);
		}
	}

	for (; _textPos.y < 15; _textPos.y++) {
		_textPos.x = 10;
		writeSpaces(30);
	}
}

void Combat::writeMonsterStatus(int monsterNum) {
	monsterSetPtr(monsterNum);
	byte statusBits = _monsterStatus[monsterNum];

	if (statusBits) {
		writeDots();

		int status;
		if (statusBits == MONFLAG_DEAD) {
			status = MON_DEAD;
		} else {	
			for (status = MON_PARALYZED; !(statusBits & 0x80);
					++status, statusBits <<= 1) {
			}
		}

		writeString(STRING[Common::String::format("dialogs.combat.status.%d",
			status)]);
	} else if (_monsterHP[monsterNum] != _monsterP->_field11) {
		writeDots();
		writeString(STRING["dialogs.combat.status.wounded"]);
	} else {
		writeSpaces(40 - _textPos.x);
	}
}

void Combat::writeDots() {
	while (_textPos.x < 30)
		writeChar('.');
}

void Combat::writeParty() {
	clearPartyArea();

	for (uint i = 0; i < g_globals->_party.size(); ++i) {
		writeChar(1 + 21 * (i % 2), 16 + (i / 2), '1' + i);
		writeString(") ");
		writeString(g_globals->_party[i]._name);
	}
}

void Combat::clearPartyArea() {
	clearLines(16, 18);
}

void Combat::writeDefeat() {
	resetBottom();
	writeString(10, 0, "+----------------------------+");
	for (int y = 1; y < 8; ++y) {
		writeChar(10, y, '!');
		writeChar(37, y, '!');
	}
	writeString(10, 0, "+----------------------------+");

	writeString(10, 2, STRING["dialogs.combat.defeating1"]);
	writeString(10, 4, STRING["dialogs.combat.defeating1"]);
	writeNumber(14, 6, _totalExperience);
	_textPos.x++;
	writeString(STRING["dialogs.combat.xp"]);
}

void Combat::highlightNextRound() {
	Common::String s = Common::String::format("%s%d",
		STRING["dialogs.combat.round"].c_str(),
		_roundNum);
	
	for (uint i = 0; i < s.size(); ++i)
		s.setChar(s[i] | 0x80, i);

	writeString(0, 1, s);
}

void Combat::writeMonsterEffects() {
	if (_monstersRegenerate)
		writeString(0, 21, STRING["dialogs.combat.regenerate"]);

	if (_monstersResistSpells) {
		if (_textPos.y != 21)
			_textPos.y = 20;

		writeString(0, _textPos.y + 1, STRING["dialogs.combat.overcome"]);
	}

	writeMonsters();
}

void Combat::writeMonsterAction() {
	writeChar(12, _monsterIndex, 0xC1);
	resetBottom();
	writeString(0, 20, _monsterName);
	writeString(STRING["dialogs.combat.monster_flees"]);
}

void Combat::writeMonsterSpell() {
	resetBottom();

	for (int i = 0, y = 0; i < (int)_monsterSpellLines.size() &&
		_monsterSpellLines[i].y > y;
		y = _monsterSpellLines[i].y, ++i) {
		Common::String text = _monsterSpellLines[i]._text;
		size_t idx;
		while ((idx = text.findFirstOf(' ')) != Common::String::npos)
			text.deleteChar(idx);

		writeString(0, _monsterSpellLines[i].y, text);
	}
}

void Combat::checkMonsterSpellDone() {
	for (uint i = 0; i < _monsterSpellLines.size(); ++i) {
		if (i > 0 && _monsterSpellLines[i].y ==
				_monsterSpellLines[i - 1].y) {
			// Remove the message line just displayed, and redraw
			// so the next one can be shown
			_monsterSpellLines.remove_at(i - 1);
			redraw();
			return;
		}
	}

	checkParty();
}

void Combat::attack() {
	if (_allowAttack)
		attackMonsterPhysical();
}

void Combat::block() {
}

void Combat::cast() {
}

void Combat::delay() {
	setOption(OPTION_DELAY);
}

void Combat::exchange() {
}

void Combat::fight() {
	if (_allowFight) {
		if (_monsterList.size() < 2) {
			attackMonsterPhysical();
		} else {
			setOption(OPTION_FIGHT);
		}
	}
}

void Combat::retreat() {
}

void Combat::shoot() {
	if (_allowShoot) {
		if (_monsterList.size() < 2) {
			attackMonsterPhysical();
		} else {
			setOption(OPTION_SHOOT);
		}
	}
}

void Combat::use() {
	// Show the character info view in USE mode
	g_events->send("CharacterInfo", GameMessage("USE"));
}

void Combat::writeMessage() {
	size_t idx;

	resetBottom();
	for (const auto &line : _message) {
		Common::String text = line._text;
		while ((idx = text.findFirstOf('|')) != Common::String::npos)
			text.deleteChar(idx);

		writeString(line.x, line.y, text);
	}
}

void Combat::writeCharAttackDamage() {
	writeString(0, 0, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		STRING[_isShooting ? "dialogs.combat.shoots" :
		"dialogs.combat.attacks"].c_str(),
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	Common::String line1;
	if (_numberOfTimes == 1) {
		line1 = STRING["dialogs.combat.once"];
	} else {
		line1 = Common::String::format("%d %s", _numberOfTimes,
			STRING["dialogs.combat.times"].c_str());
	}

	line1 += Common::String::format(" %s ", STRING["dialogs.combat.and"].c_str());

	if (_damage == 0) {
		line1 += STRING["dialogs.combat.misses"];
	} else {
		line1 += STRING["dialogs.combat.hit"];
		line1 += ' ';

		if (_numberOfTimes > 1) {
			if (_timesHit == 1) {
				line1 += STRING["dialogs.combat.once"];
			} else {
				line1 += Common::String::format("%d %s", _timesHit,
					STRING["dialogs.combat.times"].c_str());
			}
		}

		line1 += Common::String::format(" %s %d %s",
			STRING["dialogs.combat.for"].c_str(), _damage,
			STRING[_damage == 1 ? "dialogs.combat.point" : "dialogs.combat.points"].c_str());

		if (line1.size() < 30) {
			line1 += ' ';
			line1 += STRING["dialogs.combat.of_damage"];
		} else {
			line1 += '!';
		}
	}

	writeString(0, 1, line1);
}

void Combat::setOption(SelectedOption option) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	_option = option;
}

void Combat::displaySpellResult(const InfoMessage &msg) {
	assert(msg._timeoutCallback || msg._keyCallback);
	assert(!msg._delaySeconds || !msg._timeoutCallback);

	// TODO: Display the spell result
	warning("TODO: displaySpellResult");
}

} // namespace Views
} // namespace MM1
} // namespace MM
