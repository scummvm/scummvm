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
#include "mm/mm1/data/text.h"
#include "mm/mm1/views/character_view_combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/mm1.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

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


#define MONSTERS_X 120
#define BOTTOM_Y 120
#define LINE_H 8
#define MONSTER_H 7
#define COMBAT_LOG_VISIBLE_LINES 16
#define COMBAT_LOG_MAX_LINES 240

Combat::Combat() : ScrollView("Combat") {
}

void Combat::setMode(Mode newMode) {
	_mode = newMode;
	_combatModeLogged = false;
	_modeDrawn = false;
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
	_firstDraw = true;

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
		clearCombatLog();

		// The combat loop may request a partial redraw before this view
		// receives focus, so ensure the first draw is a complete one
		_firstDraw = true;

		loadMonsters();
		addEncounterToCombatLog();
		if (!g_globals->_encounters._preCombatLog.empty()) {
			addCombatLog(g_globals->_encounters._preCombatLog);
			g_globals->_encounters._preCombatLog.clear();
		}
		setupCanAttacks();
		setupHandicap();

		addView();
		combatLoop();
		return true;

	} else if (msg._name == "SPELL_RESULT") {
		assert(msg._value >= 0 && msg._value < 40);
		_spellResult._lines.clear();
		_spellResult._callback = nullptr;
		_spellResult._lines.push_back(Line(msg._value, 1, msg._stringValue));
		_spellResult._delaySeconds = 3;

		setMode(SPELL_RESULT);
		return true;

	} else if (msg._name == "EXCHANGE" && msg._value != -1) {
		int charNum = msg._value;
		if (g_globals->_combatParty[charNum] != g_globals->_currCharacter) {
			addCombatLog(Common::String::format("%s exchanges places with %s.",
				g_globals->_currCharacter->_name,
				g_globals->_combatParty[charNum]->_name));
			exchangeWith(charNum);
		}
		return true;

	} else if (msg._name == "SELECT_SPELL_TARGET") {
		setOption(OPTION_SPELL_TARGET);
		return true;

	} else if (msg._name == "COMBAT_LOG") {
		addCombatLog(msg._stringValue);
		return true;

	} else if (msg._name == "DISABLE_ATTACKS") {
		disableAttacks();
		return true;
	}

	return false;
}

void Combat::draw() {
	_modeDrawn = true;
	if (_combatLogVisible) {
		drawCombatLog();
		return;
	}

	if (_firstDraw) {
		_firstDraw = false;
		if (_mode != SELECT_OPTION) {
			// Do an initial screen draw to get everything displayed
			Mode mode = _mode;
			_mode = SELECT_OPTION;
			draw();
			_mode = mode;
		}
	}

	switch (_mode) {
	case NEXT_ROUND:
		writeMonsters();
		resetBottom();
		highlightNextRound();
		if (beginCombatLogEvent())
			addCombatLog(Common::String::format("-- Round %d --", _roundNum));
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTER_ADVANCES:
		writeBottomText(0, 0, _monsterName);
		writeString(" advances!");
		writeRound();
		writeMonsters();
		if (beginCombatLogEvent())
			addCombatLog(Common::String::format("%s  advances!",
				_monsterName.c_str()));
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTERS_AFFECTED:
		writeMonsterEffects();
		delayFrames(combatMessageDelayFrames());
		return;
	case MONSTER_SPELL:
		// Monster spell effects can cycle several messages through the same row.
		// Rebuild the complete combat frame so the previous message (or an
		// underlying dungeon/outdoor redraw) cannot remain on screen.
		clearSurface();
		writeStaticContent();
		writeHandicap();
		writeRound();
		writePartyNumbers();
		writeMonsters();
		writeParty();
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
	if (!_modeDrawn)
		return true;


	if (_combatLogVisible) {
		switch (msg.keycode) {
		case Common::KEYCODE_l:
		case Common::KEYCODE_ESCAPE:
			closeCombatLog();
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_PAGEUP:
			scrollCombatLog(msg.keycode == Common::KEYCODE_UP ? -1 :
				-COMBAT_LOG_VISIBLE_LINES);
			break;
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_PAGEDOWN:
			scrollCombatLog(msg.keycode == Common::KEYCODE_DOWN ? 1 :
				COMBAT_LOG_VISIBLE_LINES);
			break;
		case Common::KEYCODE_HOME:
			_combatLogTop = 0;
			redraw();
			break;
		case Common::KEYCODE_END:
			_combatLogTop = MAX(0,
				(int)_combatLogLines.size() - COMBAT_LOG_VISIBLE_LINES);
			redraw();
			break;
		default:
			break;
		}
		return true;
	}

	if (_mode == SELECT_OPTION && _option == OPTION_NONE &&
			msg.keycode == Common::KEYCODE_l) {
		openCombatLog();
		return true;
	}
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

		case OPTION_SPELL_TARGET:
			if (msg.keycode >= Common::KEYCODE_a &&
					msg.keycode < (int)(Common::KEYCODE_a +
					_remainingMonsters.size())) {
				int monsterNum = msg.keycode - Common::KEYCODE_a;

				if (setSpellTarget(monsterNum)) {
					_option = OPTION_NONE;
					send("CastSpell",
						GameMessage("MONSTER_SELECTED", monsterNum));
				}
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
		// Match timeout() semantics. Some combat spells, notably Holy Word
		// and Turn Undead, intentionally use a result message without a
		// callback. In that case the casting character is simply done.
		if (_spellResult._callback)
			_spellResult._callback();
		else
			block();

	} else if (isDelayActive()) {
		// In all other modes, if a delay is active, any keypress
		// will cause the delay to end immediately
		endDelay();
	}

	return true;
}

bool Combat::msgAction(const ActionMessage &msg) {
	if (!_modeDrawn)
		return true;


	if (_combatLogVisible) {
		switch (msg._action) {
		case KEYBIND_COMBAT_LOG:
		case KEYBIND_ESCAPE:
			closeCombatLog();
			break;
		case KEYBIND_COMBAT_LOG_UP:
			scrollCombatLog(-1);
			break;
		case KEYBIND_COMBAT_LOG_DOWN:
			scrollCombatLog(1);
			break;
		default:
			break;
		}
		return true;
	}

	if (_mode == SELECT_OPTION && _option == OPTION_NONE &&
			msg._action == KEYBIND_COMBAT_LOG) {
		openCombatLog();
		return true;
	}
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
		addCombatLog(Common::String::format("%s blocks.",
			g_globals->_currCharacter->_name));
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
	case KEYBIND_BESTIARY:
		addView("Bestiary");
		break;
	case KEYBIND_PROTECT:
		addView("Protect");
		break;
	case KEYBIND_QUICKREF:
		addView("QuickRef");
		break;
	case KEYBIND_COMBAT_RETREAT:
		addCombatLog(Common::String::format("%s attempts to retreat.",
			g_globals->_currCharacter->_name));
		retreat();
		break;
	case KEYBIND_COMBAT_SHOOT:
		shoot();
		break;
	case KEYBIND_COMBAT_USE:
		addCombatLog(Common::String::format(
			"%s prepares to use an item.",
			g_globals->_currCharacter->_name));
		use();
		break;
	default:
		break;
	}

	return true;
}

bool Combat::msgMouseUp(const MouseUpMessage &msg) {
	if (_combatLogVisible) {
		if (msg._button == MouseMessage::MB_RIGHT)
			closeCombatLog();
		return true;
	}

	if (msg._button != MouseMessage::MB_LEFT)
		return ScrollView::msgMouseUp(msg);

	const KeybindingAction BTN_ACTIONS[8] = {
		KEYBIND_COMBAT_ATTACK, KEYBIND_COMBAT_FIGHT,
		KEYBIND_COMBAT_RETREAT, KEYBIND_COMBAT_EXCHANGE,
		KEYBIND_COMBAT_USE, KEYBIND_COMBAT_BLOCK,
		KEYBIND_COMBAT_SHOOT, KEYBIND_COMBAT_CAST
	};

	Common::Point localPos = msg._pos;
	localPos.x -= _innerBounds.left;
	localPos.y -= _innerBounds.top;

	if (_mode == SELECT_OPTION && _option == OPTION_NONE) {
		// Check for option buttons being pressed
		Common::Rect logButton = getOptionButtonRect(2, 2);
		if (logButton.contains(localPos)) {
			msgAction(ActionMessage(KEYBIND_COMBAT_LOG));
			return true;
		}

		for (int col = 0; col < 3; ++col) {
			for (int row = 0; row < 3; ++row) {
				if (col != 2 || row != 2) {
					Common::Rect r = getOptionButtonRect(col, row);
					if (r.contains(localPos)) {
						msgAction(ActionMessage(BTN_ACTIONS[col * 3 + row]));
						return true;
					}
				}
			}
		}
	}

	if (_mode == SELECT_OPTION && _option == OPTION_NONE) {
		const KeybindingAction STATIC_ACTIONS[4] = {
			KEYBIND_DELAY, KEYBIND_PROTECT, KEYBIND_QUICKREF,
			(KeybindingAction)(KEYBIND_VIEW_PARTY1 + _currentChar)
		};

		// Delay, Protect, Quick Reference and View Character occupy
		// the four static command rows on the left side of combat.
		for (int row = 0; row < 4; ++row) {
			Common::Rect r(0, (7 + row) * LINE_H,
				80, (8 + row) * LINE_H);

			if (r.contains(localPos)) {
				msgAction(ActionMessage(STATIC_ACTIONS[row]));
				return true;
			}
		}
	}

	if (_mode == SELECT_OPTION && (_option == OPTION_SHOOT ||
			_option == OPTION_FIGHT ||
			_option == OPTION_SPELL_TARGET)) {
		// Check for entries in the monster list being pressed
		if (localPos.x >= MONSTERS_X && localPos.x < 310
				&& localPos.y >= 0 && localPos.y < 100) {
			uint monsterNum = localPos.y / MONSTER_H;
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
		writeString(40, 170, "Go back");

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
	case OPTION_SPELL_TARGET:
		writeSpellTargetSelect();
		break;
	default:
		break;
	}
}

void Combat::writeAllOptions() {
	const Character &c = *g_globals->_currCharacter;
	writeBottomText(0, 0, "Options for: ");
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

	writeOption(0, 2, 'R', "\x01""37Retreat");

	writeOption(1, 0, 'E', "\x01""37Exchange");
	writeOption(1, 1, 'U', "\x01""37Use");
	writeOption(1, 2, 'B', "\x01""37Block");

	writeOption(2, 2, 'L', "\x01""37Log");
}

void Combat::drawCombatLog() {
	ScrollView::draw();
	setReduced(false);
	setTextColor(0);

	writeString(0, 0, "Combat Log", ALIGN_MIDDLE);

	if (_combatLogLines.empty()) {
		writeString(0, 36, "No combat actions have been recorded yet.",
			ALIGN_MIDDLE);
	} else {
		const int end = MIN((int)_combatLogLines.size(),
			_combatLogTop + COMBAT_LOG_VISIBLE_LINES);
		for (int i = _combatLogTop; i < end; ++i)
			writeString(4, 18 + (i - _combatLogTop) * 9,
				_combatLogLines[i]);

		writeString(300, 164, Common::String::format("%d-%d / %d",
			_combatLogTop + 1, end, (int)_combatLogLines.size()),
			ALIGN_RIGHT);
	}

	writeString(0, 174, "Up/Down: Scroll    L/Esc: Return", ALIGN_MIDDLE);
}

void Combat::openCombatLog() {
	_combatLogVisible = true;
	_combatLogTop = MAX(0,
		(int)_combatLogLines.size() - COMBAT_LOG_VISIBLE_LINES);
	redraw();
}

void Combat::closeCombatLog() {
	_combatLogVisible = false;
	redraw();
}

void Combat::scrollCombatLog(int delta) {
	const int maxTop = MAX(0,
		(int)_combatLogLines.size() - COMBAT_LOG_VISIBLE_LINES);
	_combatLogTop = CLIP(_combatLogTop + delta, 0, maxTop);
	redraw();
}

void Combat::clearCombatLog() {
	_combatLogLines.clear();
	_combatLogTop = 0;
	_combatLogVisible = false;
	_combatModeLogged = false;
}

void Combat::addCombatLog(const Common::String &text) {
	if (text.empty())
		return;

	const bool wasReduced = _fontReduced;
	setReduced(false);
	Common::StringArray wrapped = splitLines(text, 292);
	setReduced(wasReduced);

	for (const Common::String &line : wrapped)
		_combatLogLines.push_back(line);

	while (_combatLogLines.size() > COMBAT_LOG_MAX_LINES)
		_combatLogLines.remove_at(0);
}

void Combat::addCombatLog(const LineArray &lines) {
	for (const Line &line : lines) {
		if (!line._text.empty())
			addCombatLog(line._text);
	}
}

void Combat::addEncounterToCombatLog() {
	addCombatLog("Combat begins.");
	addCombatLog("Enemies:");

	for (uint i = 0; i < _remainingMonsters.size(); ++i) {
		bool alreadyListed = false;
		for (uint j = 0; j < i; ++j) {
			if (_remainingMonsters[j]->_name == _remainingMonsters[i]->_name) {
				alreadyListed = true;
				break;
			}
		}
		if (alreadyListed)
			continue;

		uint count = 0;
		for (uint j = i; j < _remainingMonsters.size(); ++j) {
			if (_remainingMonsters[j]->_name == _remainingMonsters[i]->_name)
				++count;
		}

		addCombatLog(Common::String::format("  %u x %s", count,
			_remainingMonsters[i]->_name.c_str()));
	}

	addCombatLog(Common::String::format("-- Round %d --", _roundNum));
}

bool Combat::beginCombatLogEvent() {
	if (_combatModeLogged)
		return false;

	_combatModeLogged = true;
	return true;
}


void Combat::writeDelaySelect() {
	writeBottomText(0, 0, "Set delay (0-9):");
	writeBottomText(40, 3, Common::String::format(
		"(currently= %d)",
		g_globals->_delay));
}

void Combat::writeExchangeSelect() {
	writeBottomText(0, 1, Common::String::format(
		"Exchange places with (F1-F%d)",
		(int)g_globals->_combatParty.size()),
		ALIGN_MIDDLE);
}

void Combat::writeFightSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeBottomText(0, 1, Common::String::format(
		"Fight which 'A'-'%c'?", 'A' + _attackableCount - 1),
		ALIGN_MIDDLE);
}

void Combat::writeShootSelect() {
	_attackableCount = MIN(_attackersCount, (int)_remainingMonsters.size());

	writeBottomText(0, 1, Common::String::format(
		"Shoot which 'A'-'%c'?", 'A' + _attackableCount - 1),
		ALIGN_MIDDLE);
}

void Combat::writeSpellTargetSelect() {
	_attackableCount = _remainingMonsters.size();

	writeBottomText(0, 1, Common::String::format(
			"Target which (A-%c)?", 'A' + _attackableCount - 1),
			ALIGN_MIDDLE);
}


void Combat::writeAttackOptions() {
	writeOption(0, 0, 'A', "\x01""37Attack(A)");
	writeOption(0, 1, 'F', "\x01""37Fight(+)");
}

void Combat::writeCastOption() {
	writeOption(2, 1, 'C', "\x01""37Cast");
}

void Combat::writeShootOption() {
	writeOption(2, 0, 'S', "\x01""37Shoot");
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

	s.fillRect(area, SCROLL_BG_COLOR);
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
	return Common::Rect(x, y, x + 80, y + BTN_SIZE);
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
	writeString(0, 0, "Combat");
	writeString(0, 7 * LINE_H, "D Delay");
	writeString(0, 8 * LINE_H, "P Protect");
	writeString(0, 9 * LINE_H, "Q QuickRef");
	writeString(0, 10 * LINE_H, "# View Ch");
}

void Combat::writeHandicap() {
	writeString(0, 12 * LINE_H, "Handicap");

	clearArea(Common::Rect(0, 13 * LINE_H, 100, 14 * LINE_H));

	_textPos = Common::Point(0, 13 * LINE_H);

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
	writeString(0, LINE_H, Common::String::format("Round #%d", _roundNum));
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
		writeString(MONSTERS_X, i * MONSTER_H, (i < _attackersCount) ? "+" : " ");

		unsigned char c = 'A' + i;
		if ((i == _activeMonsterNum) && (_mode == MONSTER_ADVANCES ||
			_mode == MONSTER_ATTACK || _mode == MONSTER_SPELL))
			c |= 0x80;
		mStr.setChar(c, 0);
		writeString(MONSTERS_X + 16, i * MONSTER_H, mStr, ALIGN_RIGHT);

		writeString(MONSTERS_X + 22, i * MONSTER_H, _remainingMonsters[i]->_name);
		writeMonsterStatus(i);
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
	}

	_monsterP = currMonster;
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
	const Common::Rect oldBounds = _bounds;
	Common::String line1 = "! For defeating the monsters !";
	Common::String line2 = "!   each survivor receives   !";
	line1 = Common::String(line1.c_str() + 1, line1.c_str() + line1.size() - 2);
	line2 = Common::String(line2.c_str() + 1, line2.c_str() + line2.size() - 2);
	Common::String line3 = Common::String::format("%d experience points.",
		_totalExperience);

	setBounds(Common::Rect(50, 40, 270, 100));
	ScrollView::draw();
	writeLine(0, line1, ALIGN_MIDDLE);
	writeLine(2, line2, ALIGN_MIDDLE);
	writeLine(4, line3, ALIGN_MIDDLE);

	// Restore the combat view's size for any later combat screens
	setBounds(oldBounds);

	if (beginCombatLogEvent())
		addCombatLog(Common::String::format(
			"Victory. Each active character receives %d XP.",
			_totalExperience));
}

void Combat::highlightNextRound() {
	Common::String s = Common::String::format("Round #%d",
		_roundNum);

	for (uint i = 0; i < s.size(); ++i)
		s.setChar(s[i] | 0x80, i);

	setReduced(false);
	writeString(0, LINE_H, s);
}

void Combat::writeMonsterEffects() {
	resetBottom();

	int line = 0;
	if (_monstersRegenerate)
		writeBottomText(0, line++,
			"some monsters regenerate!");

	if (_monstersResistSpells)
		writeBottomText(0, line,
			"some monsters overcome spells!");

	writeMonsters();

	if (beginCombatLogEvent()) {
		if (_monstersRegenerate)
			addCombatLog("The monsters regenerate.");
		if (_monstersResistSpells)
			addCombatLog("The monsters overcome spell effects.");
	}
}

void Combat::writeMonsterAction(bool flees) {
	resetBottom();

	const Common::String message = Common::String::format(
			"%s %s", _monsterName.c_str(),
			flees ? "runs away..." : "wanders aimlessly");

	writeBottomText(0, 0, message);

	if (beginCombatLogEvent())
		addCombatLog(message);
}

void Combat::writeMonsterSpell() {
	resetBottom();

	for (int i = 0, y = -2; i < (int)_monsterSpellLines.size() &&
			_monsterSpellLines[i].y > y;
			y = _monsterSpellLines[i].y, ++i) {
		const Line &line = _monsterSpellLines[i];
		writeBottomText(MAX<int>(line.x, 0), MAX<int>(line.y, 0),
				line._text, line._align);
	}

	if (beginCombatLogEvent())
		addCombatLog(_monsterSpellLines);
}

void Combat::writeMonsterAttack() {
	resetBottom();

	Common::String monsterName = _monsterP->_name;
	Common::String attackStyle = _monsterAttackStyle == 99 ?
		"shoots at" : ATTACK_TYPES[_monsterAttackStyle - 1];
	Character &c = *g_globals->_currCharacter;

	Common::String line = Common::String::format("%s %s %s",
		monsterName.c_str(),
		attackStyle.c_str(),
		c._name
	);
	writeBottomText(0, 0, line);
	const Common::String attackResult = getAttackString();
	writeBottomText(0, 1, attackResult);

	Common::StringArray combatLogEvent;
	combatLogEvent.push_back(line);
	combatLogEvent.push_back(attackResult);

	if (_damage) {
		// Attacks wake up sleeping characters
		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~ASLEEP;

		// Also check for secondary monster touch action here
		// This returns a text line to display, and can also
		// adjust the damage amount. Another reason why we
		// can't actually apply damage until here
		int yp = 2;
		if (monsterTouch(line)) {
			writeBottomText(0, yp++, line);
			combatLogEvent.push_back(line);
		}

		Common::String damageStr = subtractDamageFromChar();
		if (!damageStr.empty()) {
			writeBottomText(0, yp, damageStr);
			combatLogEvent.push_back(damageStr);
		}
	}

	if (beginCombatLogEvent()) {
		for (const Common::String &logLine : combatLogEvent)
			addCombatLog(logLine);
	}
}

void Combat::writeInfiltration() {
	Common::String line = Common::String::format("%s infiltrates the ranks!",
		_monsterP->getDisplayName().c_str());

	resetBottom();
	writeBottomText(0, 0, line);
	Sound::sound(SOUND_2);
	Sound::sound(SOUND_2);

	if (beginCombatLogEvent())
		addCombatLog(line);
}

void Combat::writeWaitsForOpening() {
	Common::String line = Common::String::format("%s infiltrates the ranks!",
		_monsterP->getDisplayName().c_str());

	resetBottom();
	writeBottomText(0, 0, line);

	if (beginCombatLogEvent())
		addCombatLog(line);
}

void Combat::writeSpellResult() {
	for (uint i = 0; i < _spellResult._lines.size(); ++i) {
		const Line &l = _spellResult._lines[i];
		writeBottomText(l.x, l.y, l._text);
	}

	if (beginCombatLogEvent())
		addCombatLog(_spellResult._lines);
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
	if (_allowShoot)
		setOption(OPTION_SHOOT);
}

void Combat::writeMessage() {
	resetBottom();
	for (const auto &line : _message)
		writeString(line.x, line.y, line._text);
}

void Combat::writeCharAttackDamage() {
	resetBottom();
	const bool wasShooting = _isShooting;

	writeBottomText(0, 0, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		_isShooting ? "shoots" : "attacks",
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeBottomText(0, 1, getAttackString());

	if (_monsterP->_status == MONFLAG_DEAD) {
		writeBottomText(0, 2, Common::String::format("%s goes down!!!",
			_monsterP->_name.c_str()));
	}

	if (beginCombatLogEvent()) {
		addCombatLog(Common::String::format("%s %s %s",
			g_globals->_currCharacter->_name,
			wasShooting ? "shoots" : "attacks",
			_monsterP->_name.c_str()));
		addCombatLog(getAttackString());
		if (_monsterP->_status == MONFLAG_DEAD)
			addCombatLog(Common::String::format("%s goes down!!!",
				_monsterP->_name.c_str()));
	}
}

void Combat::writeCharAttackNoEffect() {
	resetBottom();
	const bool wasShooting = _isShooting;

	writeBottomText(0, 0, Common::String::format("%s %s %s",
		g_globals->_currCharacter->_name,
		_isShooting ? "shoots" : "attacks",
		_monsterP->_name.c_str()
	));
	_isShooting = false;

	writeBottomText(0, 1, "Weapon has no effect!");

	if (beginCombatLogEvent()) {
		addCombatLog(Common::String::format("%s %s %s",
			g_globals->_currCharacter->_name,
			wasShooting ? "shoots" : "attacks",
			_monsterP->_name.c_str()));
		addCombatLog("Weapon has no effect!");
	}
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
	// Okrim's map script expects a second processing of his square after
	// victory, but the square was already cleared before combat. Detect a
	// real victory here and hand off directly to his ghost
	const bool okrimVictory = _mode == DEFEATED_MONSTERS &&
		g_maps->_mapId == 37 && g_maps->_mapPos == Common::Point(12, 12);

	if (okrimVictory) {
		// Map37::VAL1. Set only after victory so fleeing cannot satisfy it.
		(*g_maps->_currentMap)[232] = 1;
		g_maps->clearSpecial();
	}

	Game::Combat::combatDone();

	close();
	g_events->send("Game", GameMessage("UPDATE"));

	if (okrimVictory)
		g_events->addView("Ghost");
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
