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
#include "mm/mm1/data/text.h"
#include "mm/mm1/views/character_view_combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

static uint combatMessageDelayFrames() {
	// MM1's DELAY command is an ordinal 0-9 message-duration control:
	// 0 is shortest, 5 is the default and 9 is longest. The original
	// manual does not define those values as literal seconds.
	//
	// Preserve ScummVM's existing default action-result pace by mapping
	// setting 5 to 3.0 seconds, then scale in half-second steps.
	// This gives 0 -> 0.5 sec ... 5 -> 3.0 sec ... 9 -> 5.0 sec.
	return ((uint)g_globals->_delay + 1) * FRAME_RATE / 2;
}


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

	if (_mode != MONSTER_ADVANCES && _mode != MONSTER_ATTACK &&
			_mode != MONSTER_SPELL)
		_activeMonsterNum = -1;

	redraw();
}

void Combat::disableAttacks() {
	_allowFight = false;
	_allowShoot = false;
	_allowCast = false;
	_allowAttack = false;
}

bool Combat::msgFocus(const FocusMessage &msg) {
	g_globals->_currCharacter = g_globals->_combatParty[_currentChar];
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_COMBAT);

	return true;
}

bool Combat::msgUnfocus(const UnfocusMessage &msg) {
	MetaEngine::setKeybindingMode(KeybindingMode::KBMODE_MENUS);
	return TextView::msgUnfocus(msg);
}

bool Combat::msgGame(const GameMessage &msg) {
	if (msg._name == "COMBAT") {
		// Clear combat data
		clear();

		loadMonsters();
		setupCanAttacks();
		setupHandicap();

		addView();
		combatLoop();
		return true;

	} else if (msg._name == "SPELL_RESULT") {
		assert(msg._value >= 0 && msg._value < 40);
		_spellResult._lines.clear();
		_spellResult._lines.push_back(Line(msg._value, 1, msg._stringValue));
		_spellResult._delaySeconds = 3;
		Sound::sound(SOUND_2);

		setMode(SPELL_RESULT);
		return true;

	} else if (msg._name == "DISABLE_ATTACKS") {
		disableAttacks();
		return true;
	}

	return false;
}

void Combat::draw() {
	switch (_mode) {
	case NEXT_ROUND:
		writeMonsters();
		resetBottom();
		highlightNextRound();
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTER_ADVANCES:
		writeString(0, 20, _monsterName);
		writeString(" advances!");
		writeSpaces(30);
		writeRound();
		writeMonsters();
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTERS_AFFECTED:
		writeMonsterEffects();
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTER_SPELL:
		writeMonsterSpell();
		delayFrames(combatMessageDelayFrames());
		return;
	case INFILTRATION:
		writeInfiltration();
		delayFrames(combatMessageDelayFrames());
		return;
	case WAITS_FOR_OPENING:
		writeWaitsForOpening();
		delayFrames(combatMessageDelayFrames());
		return;
	case CHAR_ATTACKS:
		writeMonsters();
		writeCharAttackDamage();
		delayFrames(combatMessageDelayFrames());
		return;
	case NO_EFFECT:
		writeCharAttackNoEffect();
		delayFrames(combatMessageDelayFrames());
		return;
	case DEFEATED_MONSTERS:
		writeDefeat();
		Sound::sound2(SOUND_3);
		delayFrames(combatMessageDelayFrames());
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

	case SPELL_RESULT:
		writeSpellResult();
		if (_spellResult._delaySeconds)
			delayFrames(combatMessageDelayFrames());
		break;

	case MONSTER_ATTACK:
		writeMonsterAttack();
		delayFrames(combatMessageDelayFrames());
		break;

	case MONSTER_FLEES:
	case MONSTER_WANDERS:
		writeMonsterAction(_mode == MONSTER_FLEES);
		delayFrames(combatMessageDelayFrames());
		break;

	default:
		break;
	}
}

void Combat::timeout() {
	if (!isFocused())
		return;

	switch (_mode) {
	case NEXT_ROUND:
		nextRound2();
		break;
	case MONSTER_ADVANCES:
		nextRound3();
		break;
	case MONSTERS_AFFECTED:
	case CHAR_ATTACKS:
	case NO_EFFECT:
	case MONSTER_FLEES:
		removeDeadMonsters();
		combatLoop();
		break;
	case MONSTER_WANDERS:
	case INFILTRATION:
	case MONSTER_ATTACK:
		writeParty();
		writeMonsters();
		checkParty();
		break;
	case MONSTER_SPELL:
		checkMonsterSpellDone();
		break;
	case WAITS_FOR_OPENING:
		combatLoop(true);
		break;
	case DEFEATED_MONSTERS:
		combatDone();
		break;
	case SPELL_RESULT:
		if (_spellResult._callback)
			_spellResult._callback();
		else
			// Character is done
			block();
		break;
	default:
		 break;
	}
}

bool Combat::msgKeypress(const KeypressMessage &msg) {
	if (endDelay())
		return true;

	if (_mode == SELECT_OPTION && _option != OPTION_NONE) {
		switch (_option) {
		case OPTION_FIGHT:
		case OPTION_SHOOT:
			if (msg.keycode >= Common::KEYCODE_a &&
				msg.keycode < (int)(Common::KEYCODE_a + _attackableCount)) {
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
	} else if (_mode == SPELL_RESULT && !isDelayActive()) {
		// Displaying a spell result that required waiting for keypress
		assert(_spellResult._callback);
		_spellResult._callback();

	} else if (isDelayActive()) {
		// In all other modes, if a delay is active, any keypress
		// will cause the delay to end immediately
		endDelay();
	}

	return true;
}

bool Combat::msgAction(const ActionMessage &msg) {
	if (endDelay())
		return true;

	if (_mode == SELECT_OPTION && _option != OPTION_NONE &&
		msg._action == KEYBIND_ESCAPE) {
		_option = OPTION_NONE;
		combatLoop();
		return true;
	}

	if (_mode != SELECT_OPTION || (_option != OPTION_NONE &&
		_option != OPTION_EXCHANGE))
		return false;

	switch (msg._action) {
	case KEYBIND_VIEW_PARTY1:
	case KEYBIND_VIEW_PARTY2:
	case KEYBIND_VIEW_PARTY3:
	case KEYBIND_VIEW_PARTY4:
	case KEYBIND_VIEW_PARTY5:
	case KEYBIND_VIEW_PARTY6: {
		uint charNum = msg._action - KEYBIND_VIEW_PARTY1;
		if (charNum < g_globals->_combatParty.size()) {
			if (_option == OPTION_EXCHANGE) {
				if (g_globals->_combatParty[charNum] != g_globals->_currCharacter)
					exchangeWith(charNum);
			} else {
				g_globals->_currCharacter = g_globals->_combatParty[charNum];
				addView("CharacterViewCombat");
			}
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
	case OPTION_EXCHANGE:
		writeExchangeSelect();
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
	assert(g_globals->_currCharacter &&
		g_globals->_currCharacter == g_globals->_combatParty[_currentChar]);
	const Character &c = *g_globals->_currCharacter;
	writeString(0, 20, "Options for: ");
	writeString(0, 22, c._name);

	// Highlight the currently active character
	writeChar(3 + 4 * (_currentChar % 2), 3 + (_currentChar / 2),
		(unsigned char)'1' + _currentChar + 0x80);

	bool testShoot;
	if (c._canAttack) {
		writeAttackOptions();
		_allowAttack = true;
		_allowFight = true;

		// Archers can always attack
		testShoot = c._class == ARCHER;
	} else {
		testShoot = true;
	}
	if (testShoot && c._missileAttr._base) {
		_allowShoot = true;
		writeShootOption();
	}

	if (c._sp._current) {
		writeCastOption();
		_allowCast = true;
	}

	writeString(16, 22, "'E' Exchange  'U' Use");
	writeString(16, 23, "'R' Retreat   'B' Block");
}

void Combat::writeDelaySelect() {
	resetBottom();
	writeString(0, 20, "Set delay (0-9):");
	writeString(0, 23, Common::String::format(
		"(currently= %d)",
		g_globals->_delay));
	escToGoBack(0, 23);
}

void Combat::writeExchangeSelect() {
	resetBottom();
	writeString(7, 20, Common::String::format(
		"Exchange places with (F1-F%d)",
		(int)g_globals->_combatParty.size()));
	escToGoBack(12, 23);
}

void Combat::writeFightSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeString(10, 20, Common::String::format(
		"Fight which 'A'-'%c'?", 'A' + _attackableCount - 1));
	escToGoBack(12, 23);
}

void Combat::writeShootSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeString(10, 20, Common::String::format(
		"Shoot which 'A'-'%c'?", 'A' + _attackableCount - 1));
	escToGoBack(12, 23);
}

void Combat::writeAttackOptions() {
	writeString(16, 20, "'A' Attack(a)");
	writeString(16, 21, "'F' Fight(+)");
}

void Combat::writeCastOption() {
	writeString(30, 21, "'C' Cast");
}

void Combat::writeShootOption() {
	writeString(30, 20, "'S' Shoot");
}

void Combat::resetBottom() {
	clearLines(20, 24);
	_allowFight = _allowShoot = false;
	_allowCast = _allowAttack = false;
}

void Combat::writeStaticContent() {
	writeString(0, 0, "Combat");
	writeString(0, 1, "Round #");
	writeString(0, 7, "D Delay");
	writeString(0, 8, "P Protect");
	writeString(0, 9, "Q QuickRef");
	writeString(0, 10, "# View Ch");
	writeString(0, 12, "Handicap");

	_textPos = Common::Point(0, 15);
	for (int i = 0; i < 40; ++i)
		writeChar('-');
}

void Combat::writeHandicap() {
	writeString(0, 13, "          ");
	_textPos.x = 0;

	switch (_handicap) {
	case HANDICAP_EVEN:
		writeString("Even");
		break;
	case HANDICAP_PARTY:
		writeString("Party +");
		writeNumber(_handicapDelta);
		break;
	case HANDICAP_MONSTER:
		writeString("Monster +");
		writeNumber(_handicapDelta);
		break;
	}
}

void Combat::writeRound() {
	writeNumber(7, 1, _roundNum);
}

void Combat::writePartyNumbers() {
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		writeChar(2 + 4 * (i % 2), 3 + (i / 2),
			g_globals->_combatParty[i]->_canAttack ? '+' : ' ');
		writeChar('1' + i);
	}
}

void Combat::writeMonsters() {
	if (_remainingMonsters.empty()) {
		_textPos = Common::Point(10, 0);
		writeSpaces(30);
	} else {
		for (int i = 0; i < (int)_remainingMonsters.size(); ++i) {
			_textPos = Common::Point(11, i);
			writeChar(i < _attackersCount ? '+' : ' ');
			unsigned char c = 'A' + i;
			if ((i == _activeMonsterNum) && (_mode == MONSTER_ADVANCES ||
				_mode == MONSTER_ATTACK || _mode == MONSTER_SPELL))
				c |= 0x80;

			writeChar(c);
			writeString(") ");
			writeString(_remainingMonsters[i]->_name);
			writeMonsterStatus(i);
		}
	}

	for (; _textPos.y < 15; _textPos.y++) {
		_textPos.x = 10;
		writeSpaces(30);
	}
}

void Combat::writeMonsterStatus(int monsterNum) {
	auto *currMonster = _monsterP;
	_monsterP = _remainingMonsters[monsterNum];
	monsterIndexOf();
	byte statusBits = _remainingMonsters[monsterNum]->_status;

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

		writeString(COMBAT_STATUS[status]);
	} else if (_monsterP->_hp != _monsterP->_defaultHP) {
		writeDots();
		writeString("(wounded) ");
	} else {
		writeSpaces(40 - _textPos.x);
	}

	_monsterP = currMonster;
}

void Combat::writeDots() {
	while (_textPos.x < 30)
		writeChar('.');
}

void Combat::writeParty() {
	clearPartyArea();

	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		const Character &c = *g_globals->_combatParty[i];
		writeString(21 * (i % 2), 16 + (i / 2),
			Common::String::format("%c%c) %s",
				(c._condition == 0) ? ' ' : '*',
				'1' + i,
				c._name
			)
		);
	}
}

void Combat::clearPartyArea() {
	clearLines(16, 18);
}

void Combat::writeDefeat() {
	writeString(10, 0, "+----------------------------+");
	for (int y = 1; y < 8; ++y)
		writeString(10, y, "!                            !");
	writeString(10, 8, "+----------------------------+");

	writeString(10, 2, "! For defeating the monsters !");
	writeString(10, 4, "!   each survivor receives   !");
	writeNumber(14, 6, _totalExperience);
	_textPos.x++;
	writeString("experience points.");
}

void Combat::highlightNextRound() {
	Common::String s = Common::String::format("Round #%d",
		_roundNum);
	
	for (uint i = 0; i < s.size(); ++i)
		s.setChar(s[i] | 0x80, i);

	writeString(0, 1, s);
}

void Combat::writeMonsterEffects() {
	if (_monstersRegenerate)
		writeString(0, 21, "some monsters regenerate!");

	if (_monstersResistSpells) {
		if (_textPos.y != 21)
			_textPos.y = 20;

		writeString(0, _textPos.y + 1, "some monsters overcome spells!");
	}

	writeMonsters();
}

void Combat::writeMonsterAction(bool flees) {
	resetBottom();
	writeString(0, 20, _monsterName);
	writeChar(' ');
	writeString(flees ? "runs away..." : "wanders aimlessly");
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

void Combat::writeMonsterAttack() {
	Common::String monsterName = _monsterP->_name;
	Common::String attackStyle = _monsterAttackStyle == 99 ?
		"shoots at" : ATTACK_TYPES[_monsterAttackStyle - 1];
	Character &c = *g_globals->_currCharacter;

	Common::String line = Common::String::format("%s %s %s",
		monsterName.c_str(),
		attackStyle.c_str(),
		c._name
	);
	writeString(0, 20, line);
	writeString(0, 21, getAttackString());

	if (_damage) {
		// Attacks wake up sleeping characters
		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~ASLEEP;

		// Also check for secondary monster touch action here
		// This returns a text line to display, and can also
		// adjust the damage amount. Another reason why we
		// can't actually apply damage until here
		int yp = 22;
		if (monsterTouch(line))
			writeString(0, yp++, line);

		Common::String damageStr = subtractDamageFromChar();
		if (!damageStr.empty())
			writeString(0, yp, damageStr);
	}
}

void Combat::writeInfiltration() {
	Common::String line = Common::String::format("%s infiltrates the ranks!",
		_monsterP->getDisplayName().c_str());

	resetBottom();
	writeString(0, 20, line);
	Sound::sound(SOUND_2);
	Sound::sound(SOUND_2);
}

void Combat::writeWaitsForOpening() {
	Common::String line = Common::String::format("%s infiltrates the ranks!",
		_monsterP->getDisplayName().c_str());

	resetBottom();
	writeString(0, 20, line);
}

void Combat::writeSpellResult() {
	for (uint i = 0; i < _spellResult._lines.size(); ++i) {
		const Line &l = _spellResult._lines[i];
		writeString(l.x, l.y + 20, l._text);
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

void Combat::delay() {
	setOption(OPTION_DELAY);
}

void Combat::exchange() {
	if (g_globals->_combatParty.size() > 1)
		setOption(OPTION_EXCHANGE);
}

void Combat::fight() {
	if (_allowFight) {
		if (_remainingMonsters.size() < 2) {
			attackMonsterPhysical();
		} else {
			setOption(OPTION_FIGHT);
		}
	}
}

void Combat::shoot() {
	if (_allowShoot) {
		if (_remainingMonsters.size() < 2) {
			attackMonsterPhysical();
		} else {
			setOption(OPTION_SHOOT);
		}
	}
}

void Combat::writeMessage() {
	resetBottom();
	for (const auto &line : _message)
		writeString(line.x, line.y, line._text);
}

void Combat::writeCharAttackDamage() {
	resetBottom();

	writeString(0, 20, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		_isShooting ? "shoots" : "attacks",
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeString(0, 21, getAttackString());

	if (_monsterP->_status == MONFLAG_DEAD) {
		writeString(0, 22, Common::String::format("%s goes down!!!",
			_monsterP->_name.c_str()));
	}
}

void Combat::writeCharAttackNoEffect() {
	resetBottom();

	writeString(0, 20, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		_isShooting ? "shoots" : "attacks",
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeString(0, 21, "Weapon has no effect!");
}

Common::String Combat::getAttackString() {
	Common::String line1;
	if (_numberOfTimes == 1) {
		line1 = "once";
	} else {
		line1 = Common::String::format("%d times", _numberOfTimes);
	}

	line1 += " and ";

	if (_displayedDamage == 0) {
		line1 += "misses";
	} else {
		line1 += "hit";

		if (_numberOfTimes > 1) {
			line1 += ' ';

			if (_timesHit == 1) {
				line1 += "once";
			} else {
				line1 += Common::String::format("%d times", _timesHit);
			}
		}

		line1 += Common::String::format(" for %d %s", _displayedDamage,
			_damage == 1 ? "point" : "points");

		if (line1.size() < 30) {
			line1 += ' ';
			line1 += "of damage!";
		} else {
			line1 += '!';
		}
	}

	return line1;
}

void Combat::setOption(SelectedOption option) {
	MetaEngine::setKeybindingMode((option == OPTION_EXCHANGE) ?
		KeybindingMode::KBMODE_PARTY_MENUS :
		KeybindingMode::KBMODE_MENUS);
	_option = option;
	redraw();
}

void Combat::displaySpellResult(const InfoMessage &msg) {
	if (msg._delaySeconds) {
		_spellResult = msg;
	} else {
		InfoMessage tmp = msg;
		tmp._delaySeconds = 3;
		_spellResult = tmp;
	}

	setMode(SPELL_RESULT);
}

void Combat::combatDone() {
	// Okrim's map script expects a second processing of his square after
	// victory, but the square was already cleared before combat. Detect a
	// real victory here and hand off directly to his ghost
	const bool okrimVictory = _mode == DEFEATED_MONSTERS &&
		g_maps->_mapId == 37 && g_maps->_mapPos == Common::Point(12, 12);

	if (okrimVictory) {
		// Map37::VAL1. Set only after victory so fleeing can't satisfy it
		(*g_maps->_currentMap)[232] = 1;
		g_maps->clearSpecial();
	}

	Game::Combat::combatDone();

	close();
	g_events->send("Game", GameMessage("UPDATE"));

	if (okrimVictory)
		g_events->addView("Ghost");
}

} // namespace Views
} // namespace MM1
} // namespace MM
