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

#include "mm/mm1/views_enh/combat.h"
#include "mm/mm1/views/character_view_combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define MONSTERS_X 120
#define BOTTOM_Y 120
#define LINE_H 8

Combat::Combat() : ScrollView("Combat") {
}

void Combat::setMode(Mode newMode) {
	_mode = newMode;
	clearButtons();

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
	return ScrollView::msgUnfocus(msg);
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

		setMode(SPELL_RESULT);
		return true;

	} else if (msg._name == "EXCHANGE" && msg._value != -1) {
		int charNum = msg._value;
		if (g_globals->_combatParty[charNum] != g_globals->_currCharacter)
			exchangeWith(charNum);
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
		delaySeconds(1);
		return;
	case MONSTER_ADVANCES:
		writeBottomText(0, 0, _monsterName);
		writeString(STRING["dialogs.combat.advances"]);
		writeRound();
		writeMonsters();
		delaySeconds(2);
		return;
	case MONSTERS_AFFECTED:
		writeMonsterEffects();
		delaySeconds(2);
		return;
	case MONSTER_SPELL:
		writeMonsterSpell();
		delaySeconds(2);
		return;
	case INFILTRATION:
		writeInfiltration();
		delaySeconds(3);
		return;
	case WAITS_FOR_OPENING:
		writeWaitsForOpening();
		delaySeconds(2);
		return;
	case CHAR_ATTACKS:
		writeMonsters();
		writeCharAttackDamage();
		delaySeconds(3);
		return;
	case NO_EFFECT:
		writeCharAttackNoEffect();
		delaySeconds(3);
		return;
	case DEFEATED_MONSTERS:
		writeDefeat();
		Sound::sound2(SOUND_3);
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

	case SPELL_RESULT:
		writeSpellResult();
		if (_spellResult._delaySeconds)
			delaySeconds(_spellResult._delaySeconds);
		break;

	case MONSTER_ATTACK:
		writeMonsterAttack();
		delaySeconds(2);
		break;

	case MONSTER_FLEES:
	case MONSTER_WANDERS:
		writeMonsterAction(_mode == MONSTER_FLEES);
		delaySeconds(2);
		break;

	default:
		break;
	}
}

void Combat::timeout() {
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
		if (_spellResult._timeoutCallback)
			_spellResult._timeoutCallback();
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
		assert(_spellResult._timeoutCallback);
		_spellResult._timeoutCallback();

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
				clearBottom();
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

bool Combat::msgMouseUp(const MouseUpMessage &msg) {
	const KeybindingAction BTN_ACTIONS[8] = {
		KEYBIND_COMBAT_ATTACK, KEYBIND_COMBAT_FIGHT,
		KEYBIND_COMBAT_RETREAT, KEYBIND_COMBAT_EXCHANGE,
		KEYBIND_COMBAT_USE, KEYBIND_COMBAT_BLOCK,
		KEYBIND_COMBAT_SHOOT, KEYBIND_COMBAT_CAST
	};

	if (_mode == SELECT_OPTION && _option == OPTION_NONE) {
		// Check for option buttons being pressed
		for (int col = 0; col < 3; ++col) {
			for (int row = 0; row < 3; ++row) {
				if (col != 2 || row != 2) {
					Common::Rect r = getOptionButtonRect(col, row);
					if (r.contains(msg._pos)) {
						msgAction(ActionMessage(BTN_ACTIONS[col * 3 + row]));
						return true;
					}
				}
			}
		}
	}

	if (_mode == SELECT_OPTION && (_option == OPTION_SHOOT ||
			_option == OPTION_FIGHT)) {
		// Check for entries in the monster list being pressed
		if (msg._pos.x >= MONSTERS_X && msg._pos.x < 310
				&& msg._pos.y >= _innerBounds.top && msg._pos.y < 100) {
			uint monsterNum = (msg._pos.y - _innerBounds.top) / LINE_H;
			if (monsterNum < _remainingMonsters.size()) {
				char c = 'a' + monsterNum;
				msgKeypress(KeypressMessage(Common::KeyState(
					(Common::KeyCode)(Common::KEYCODE_a + (c - 'a')), c
				)));
				return true;
			}
		}
	}

	return ScrollView::msgMouseUp(msg);
}

void Combat::writeOptions() {
	if (_option != OPTION_NONE)
		writeString(30, 170, STRING["enhdialogs.misc.go_back"]);

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
	const Character &c = *g_globals->_currCharacter;
	writeBottomText(0, 0, STRING["dialogs.combat.options_for"]);
	writeBottomText(0, 2, c._name);

	// Highlight the currently active character
	writeChar((2 + 4 * (_currentChar % 2)) * 8 + 8, (3 + (_currentChar / 2)) * LINE_H,
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

	writeOption(0, 2, 'R', STRING["enhdialogs.combat.retreat"]);

	writeOption(1, 0, 'E', STRING["enhdialogs.combat.exchange"]);
	writeOption(1, 1, 'U', STRING["enhdialogs.combat.use"]);
	writeOption(1, 2, 'B', STRING["enhdialogs.combat.block"]);
}

void Combat::writeDelaySelect() {
	writeBottomText(0, 0, STRING["dialogs.combat.set_delay"]);
	writeBottomText(0, 3, Common::String::format(
		STRING["dialogs.combat.delay_currently"].c_str(),
		g_globals->_delay));
}

void Combat::writeExchangeSelect() {
	writeBottomText(0, 1, Common::String::format(
		STRING["dialogs.combat.exchange_places"].c_str(),
		'0' + g_globals->_combatParty.size()),
		ALIGN_MIDDLE);
}

void Combat::writeFightSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeBottomText(0, 1, Common::String::format(
		STRING["dialogs.combat.fight_which"].c_str(), 'A' + _attackableCount - 1),
		ALIGN_MIDDLE);
}

void Combat::writeShootSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeBottomText(0, 1, Common::String::format(
		STRING["dialogs.combat.shoot_which"].c_str(), 'A' + _attackableCount - 1),
		ALIGN_MIDDLE);
}

void Combat::writeAttackOptions() {
	writeOption(0, 0, 'A', STRING["dialogs.combat.attack"]);
	writeOption(0, 1, 'F', STRING["dialogs.combat.fight"]);
}

void Combat::writeCastOption() {
	writeOption(2, 1, 'C', STRING["dialogs.combat.cast"]);
}

void Combat::writeShootOption() {
	writeOption(2, 0, 'S', STRING["dialogs.combat.shoot"]);
}

void Combat::clearSurface() {
	frame();
	fill();

	clearBottom();

	drawButtons();
}

void Combat::clearBottom() {
	_bounds = Common::Rect(0, BOTTOM_Y, 320, 200);
	frame();
	fill();

	_bounds = Common::Rect(0, 0, 320, 200);
}

void Combat::clearArea(const Common::Rect &r) {
	Graphics::ManagedSurface s = getSurface();
	Common::Rect area = r;
	area.translate(_innerBounds.left, _innerBounds.top);
	area.right = MIN(area.right, _innerBounds.right);
	area.bottom = MIN(area.bottom, _innerBounds.bottom);

	s.fillRect(area, 0x99);
}


void Combat::resetBottom() {
	clearArea(Common::Rect(0, 19 * LINE_H, 320, 200));
	_allowFight = _allowShoot = false;
	_allowCast = _allowAttack = false;
}

void Combat::writeBottomText(int x, int line, const Common::String &msg,
		TextAlign align) {
	writeString(x, (line + 19) * LINE_H, msg, align);
}

#define BTN_SIZE 10

Common::Rect Combat::getOptionButtonRect(uint col, uint row) {
	assert(col < 3 && row < 3);

	const int x = 80 + col * 80;
	const int y = (19 * LINE_H) + row * BTN_SIZE;
	return Common::Rect(x, y, x + BTN_SIZE, y + BTN_SIZE);
}

void Combat::writeOption(uint col, uint row, char c, const Common::String &msg) {
	Common::Rect r = getOptionButtonRect(col, row);
	const int x = r.left;
	const int y = r.top;
	const int textY = y + (BTN_SIZE - 8) / 2 + 1;

	// Create a blank button
	Graphics::ManagedSurface btnSmall(BTN_SIZE, BTN_SIZE);
	btnSmall.blitFrom(g_globals->_blankButton, Common::Rect(0, 0, 20, 20),
		Common::Rect(0, 0, BTN_SIZE, BTN_SIZE));

	// Display button and write character in the middle
	Graphics::ManagedSurface s = getSurface();
	s.blitFrom(btnSmall, Common::Point(x + _innerBounds.left,
		y + _innerBounds.top));
	writeString(x + (BTN_SIZE / 2) + 1, textY,
		Common::String::format("%c", c), ALIGN_MIDDLE);

	// Write text to the right of the button
	writeString(x + BTN_SIZE + 4, textY, msg);
}

void Combat::writeStaticContent() {
	setReduced(false);
	writeString(0, 0, STRING["dialogs.combat.combat"]);
	writeString(0, 7 * LINE_H, STRING["dialogs.combat.delay"]);
	writeString(0, 8 * LINE_H, STRING["dialogs.combat.protect"]);
	writeString(0, 9 * LINE_H, STRING["dialogs.combat.quickref"]);
	writeString(0, 10 * LINE_H, STRING["dialogs.combat.view_char"]);
}

void Combat::writeHandicap() {
	writeString(0, 12 * LINE_H, STRING["dialogs.combat.handicap"]);

	clearArea(Common::Rect(0, 13 * LINE_H, 100, 14 * LINE_H));

	_textPos = Common::Point(0, 13 * LINE_H);

	switch (_handicap) {
	case HANDICAP_EVEN:
		writeString(STRING["dialogs.combat.even"]);
		break;
	case HANDICAP_PARTY:
		writeString(STRING["dialogs.combat.party_plus"]);
		writeNumber(_handicapDelta);
		break;
	case HANDICAP_MONSTER:
		writeString(STRING["dialogs.combat.monster_plus"]);
		writeNumber(_handicapDelta);
		break;
	}
}

void Combat::writeRound() {
	writeString(0, LINE_H, Common::String::format("%s%d",
		STRING["dialogs.combat.round"].c_str(), _roundNum));
}

void Combat::writePartyNumbers() {
	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		writeChar((2 + 4 * (i % 2)) * 8, (3 + (i / 2)) * LINE_H,
			g_globals->_combatParty[i]->_canAttack ? '+' : ' ');
		writeChar('1' + i);
	}
}

void Combat::writeMonsters() {
	Common::String mStr = "A)";
	setReduced(true);
	clearArea(Common::Rect(MONSTERS_X, 0, 320, 100));

	for (int i = 0; i < (int)_remainingMonsters.size(); ++i) {
		writeString(MONSTERS_X, i * LINE_H, (i < _attackersCount) ? "+" : " ");

		unsigned char c = 'A' + i;
		if ((i == _activeMonsterNum) && (_mode == MONSTER_ADVANCES ||
			_mode == MONSTER_ATTACK || _mode == MONSTER_SPELL))
			c |= 0x80;
		mStr.setChar(c, 0);
		writeString(MONSTERS_X + 16, i * LINE_H, mStr, ALIGN_RIGHT);

		writeString(MONSTERS_X + 22, i * LINE_H, _remainingMonsters[i]->_name);
		writeMonsterStatus(i);
	}
}

void Combat::writeMonsterStatus(int monsterNum) {
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

		writeString(STRING[Common::String::format("dialogs.combat.status.%d",
			status)]);
	} else if (_monsterP->_hp != _monsterP->_defaultHP) {
		writeDots();
		writeString(STRING["dialogs.combat.status.wounded"]);
	}
}

void Combat::writeDots() {
	const int dotWidth = getStringWidth(".");
	_textPos.x = ((_textPos.x + dotWidth - 1) / dotWidth) * dotWidth;

	while (_textPos.x < 240)
		writeChar('.');
}

void Combat::writeParty() {
	clearPartyArea();

	for (uint i = 0; i < g_globals->_combatParty.size(); ++i) {
		const Character &c = *g_globals->_combatParty[i];
		const int x = 160 * (i % 2);
		const int y = (15 + (i / 2)) * LINE_H;

		writeChar(x, y, (c._condition == 0) ? ' ' : '*');
		writeString(x + 15, y, Common::String::format("%c)", '1' + i), ALIGN_RIGHT);
		writeChar(' ');
		writeString(c._name);
	}
}

void Combat::clearPartyArea() {
	clearArea(Common::Rect(0, 15 * LINE_H, 320, 18 * LINE_H));
}

void Combat::writeDefeat() {
	writeString(10, 0, "+----------------------------+");
	for (int y = 1; y < 8; ++y)
		writeString(10, y, "!                            !");
	writeString(10, 8, "+----------------------------+");

	writeString(10, 2, STRING["dialogs.combat.defeating1"]);
	writeString(10, 4, STRING["dialogs.combat.defeating2"]);
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

void Combat::writeMonsterAction(bool flees) {
	resetBottom();
	writeString(0, 20, _monsterName);
	writeChar(' ');
	writeString(STRING[flees ?
		"dialogs.combat.monster_flees" : "dialogs.combat.monster_wanders"
	]);
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
	Common::String attackStyle = STRING[Common::String::format(
		"dialogs.combat.attack_types.%d", _monsterAttackStyle)];
	Character &c = *g_globals->_currCharacter;

	Common::String line = Common::String::format("%s %s %s",
		monsterName.c_str(),
		attackStyle.c_str(),
		c._name
	);
	writeBottomText(0, 0, line);
	writeBottomText(0, 1, getAttackString());

	if (_damage) {
		// Attacks wake up sleeping characters
		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~ASLEEP;

		// Also check for secondary monster touch action here
		// This returns a text line to display, and can also
		// adjust the damage amount. Another reason why we
		// can't actually apply damage until here
		int yp = 2;
		if (monsterTouch(line))
			writeBottomText(0, yp++, line);

		Common::String damageStr = subtractDamageFromChar();
		if (!damageStr.empty())
			writeBottomText(0, yp, damageStr);
	}
}

void Combat::writeInfiltration() {
	Common::String line = Common::String::format("%s %s",
		_monsterP->getDisplayName().c_str(),
		STRING["dialogs.combat.infiltration"].c_str());

	resetBottom();
	writeBottomText(0, 0, line);
	Sound::sound(SOUND_2);
	Sound::sound(SOUND_2);
}

void Combat::writeWaitsForOpening() {
	Common::String line = Common::String::format("%s %s",
		_monsterP->getDisplayName().c_str(),
		STRING["dialogs.combat.infiltration"].c_str()
	);

	resetBottom();
	writeBottomText(0, 0, line);
}

void Combat::writeSpellResult() {
	for (uint i = 0; i < _spellResult._lines.size(); ++i) {
		const Line &l = _spellResult._lines[i];
		writeBottomText(l.x, l.y, l._text);
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

	writeBottomText(0, 0, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		STRING[_isShooting ? "dialogs.combat.shoots" :
		"dialogs.combat.attacks"].c_str(),
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeBottomText(0, 1, getAttackString());

	if (_monsterP->_status == MONFLAG_DEAD) {
		writeBottomText(0, 2, Common::String::format("%s %s",
			_monsterP->_name.c_str(),
			STRING["dialogs.combat.goes_down"].c_str()));
	}
}

void Combat::writeCharAttackNoEffect() {
	resetBottom();

	writeBottomText(0, 0, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		STRING[_isShooting ? "dialogs.combat.shoots" :
		"dialogs.combat.attacks"].c_str(),
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeBottomText(0, 1, STRING["dialogs.combat.weapon_no_effect"]);
}

Common::String Combat::getAttackString() {
	Common::String line1;
	if (_numberOfTimes == 1) {
		line1 = STRING["dialogs.combat.once"];
	} else {
		line1 = Common::String::format("%d %s", _numberOfTimes,
			STRING["dialogs.combat.times"].c_str());
	}

	line1 += Common::String::format(" %s ", STRING["dialogs.combat.and"].c_str());

	if (_displayedDamage == 0) {
		line1 += STRING["dialogs.combat.misses"];
	} else {
		line1 += STRING["dialogs.combat.hit"];

		if (_numberOfTimes > 1) {
			line1 += ' ';

			if (_timesHit == 1) {
				line1 += STRING["dialogs.combat.once"];
			} else {
				line1 += Common::String::format("%d %s", _timesHit,
					STRING["dialogs.combat.times"].c_str());
			}
		}

		line1 += Common::String::format(" %s %d %s",
			STRING["dialogs.combat.for"].c_str(), _displayedDamage,
			STRING[_damage == 1 ? "dialogs.combat.point" : "dialogs.combat.points"].c_str());

		if (line1.size() < 30) {
			line1 += ' ';
			line1 += STRING["dialogs.combat.of_damage"];
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

	if (option == OPTION_EXCHANGE) {
		// Show the view to select which character
		_option = OPTION_NONE;
		addView("WhichCharacter");

	} else {
		clearButtons();
		if (option != OPTION_NONE) {
			addButton(&g_globals->_escSprites, Common::Point(0, 164),
				0, KEYBIND_ESCAPE);
		}

		redraw();
	}
}

void Combat::displaySpellResult(const InfoMessage &msg) {
	assert(msg._delaySeconds);
	_spellResult = msg;

	setMode(SPELL_RESULT);
}

void Combat::combatDone() {
	Game::Combat::combatDone();

	close();
	g_events->send("Game", GameMessage("UPDATE"));
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
