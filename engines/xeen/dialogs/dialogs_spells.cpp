/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "xeen/dialogs/dialogs_spells.h"
#include "xeen/dialogs/dialogs_input.h"
#include "xeen/dialogs/dialogs_query.h"
#include "xeen/resources.h"
#include "xeen/spells.h"
#include "xeen/sprites.h"
#include "xeen/xeen.h"

namespace Xeen {

Character *SpellsDialog::show(XeenEngine *vm, ButtonContainer *priorDialog,
		Character *c, SpellDialogMode mode) {
	SpellsDialog *dlg = new SpellsDialog(vm);
	Character *result = dlg->execute(priorDialog, c, mode);
	delete dlg;

	return result;
}

Character *SpellsDialog::execute(ButtonContainer *priorDialog, Character *c, int mode) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[25];
	int ccNum = _vm->_files->_ccNum;

	loadButtons();
	loadStrings("spldesc.bin");

	int modeCopy = mode;
	mode &= 0x7f;
	int selection = -1;
	int topIndex = 0;
	int newSelection;
	w.open();

	do {
		if (!mode) {
			if (!c->guildMember()) {
				sound.stopSound();
				intf._overallFrame = 5;
				sound.playSound(ccNum ? "skull1.voc" : "guild11.voc", 1);
				break;
			}

			Common::String title = Common::String::format(Res.BUY_SPELLS, c->_name.c_str());
			Common::String msg = Common::String::format(Res.GUILD_OPTIONS,
				title.c_str(), XeenEngine::printMil(party._gold).c_str());
			windows[10].writeString(msg);
			priorDialog->drawButtons(&windows[10]);
		}

		_spells.clear();
		const char *errorMsg = setSpellText(c, modeCopy);
		w.writeString(Common::String::format(Res.SPELLS_FOR,
			errorMsg == nullptr ? Res.SPELL_LINES_0_TO_9 : "",
			c->_name.c_str()));

		// Setup and write out spell list
		const char *names[10];
		int colors[10];
		Common::String emptyStr = "";
		Common::fill(&names[0], &names[10], emptyStr.c_str());
		Common::fill(&colors[0], &colors[10], 9);

		for (int idx = 0; idx < 10; ++idx) {
			if ((topIndex + idx) < (int)_spells.size()) {
				names[idx] = _spells[topIndex + idx]._name.c_str();
				colors[idx] = _spells[topIndex + idx]._color;
			}
		}

		if (selection >= topIndex && selection < (topIndex + 10))
			colors[selection - topIndex] = 15;
		if (_spells.size() == 0)
			names[0] = errorMsg;

		windows[37].writeString(Common::String::format(Res.SPELLS_DIALOG_SPELLS,
			colors[0], names[0], colors[1], names[1], colors[2], names[2],
			colors[3], names[3], colors[4], names[4], colors[5], names[5],
			colors[6], names[6], colors[7], names[7], colors[8], names[8],
			colors[9], names[9],
			mode ? Res.SPELL_PTS : Res.GOLD,
			mode ? c->_currentSp : party._gold
		));

		_scrollSprites.draw(0, 4, Common::Point(39, 26));
		_scrollSprites.draw(0, 0, Common::Point(187, 26));
		_scrollSprites.draw(0, 2, Common::Point(187, 111));
		if (mode)
			_scrollSprites.draw(w, 5, Common::Point(132, 123));

		w.update();

		_buttonValue = 0;
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);
		} while (!_vm->shouldExit() && !_buttonValue);

		switch (_buttonValue) {
		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			if (_vm->_mode != MODE_COMBAT) {
				_buttonValue -= Common::KEYCODE_F1;
				if (_buttonValue < (int)party._activeParty.size()) {
					c = &party._activeParty[_buttonValue];
					spells._lastCaster = _buttonValue;
					intf.highlightChar(_buttonValue);

					if (_vm->_mode == MODE_INTERACTIVE7) {
						windows[10].writeString(Common::String::format(Res.GUILD_OPTIONS,
							XeenEngine::printMil(party._gold).c_str(), Res.GUILD_TEXT, c->_name.c_str()));
					} else {
						SpellsCategory category = c->getSpellsCategory();
						int spellIndex = (c->_currentSpell == -1) ? SPELLS_PER_CLASS : c->_currentSpell;
						int spellId = (category == SPELLCAT_INVALID) ? NO_SPELL : Res.SPELLS_ALLOWED[category][spellIndex];

						windows[10].writeString(Common::String::format(Res.CAST_SPELL_DETAILS,
							c->_name.c_str(), spells._spellNames[spellId].c_str(),
							spells.calcSpellPoints(spellId, c->getCurrentLevel()),
							Res.SPELL_GEM_COST[spellId], c->_currentSp));
					}

					if (priorDialog != nullptr)
						priorDialog->drawButtons(&windows[0]);
					windows[10].update();
				}
			}
			break;

		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
		case Common::KEYCODE_s:
			if (selection != -1)
				_buttonValue = Common::KEYCODE_ESCAPE;
			break;

		case Common::KEYCODE_ESCAPE:
			selection = -1;
			_buttonValue = Common::KEYCODE_ESCAPE;
			break;

		case Common::KEYCODE_0:
		case Common::KEYCODE_1:
		case Common::KEYCODE_2:
		case Common::KEYCODE_3:
		case Common::KEYCODE_4:
		case Common::KEYCODE_5:
		case Common::KEYCODE_6:
		case Common::KEYCODE_7:
		case Common::KEYCODE_8:
		case Common::KEYCODE_9:
			newSelection = topIndex + ((_buttonValue == Common::KEYCODE_0) ? 9 :
				(_buttonValue - Common::KEYCODE_1));

			if (newSelection < (int)_spells.size()) {
				SpellsCategory category = c->getSpellsCategory();
				int expenseFactor = c->getSpellsExpenseFactor();

				int spellIndex = _spells[newSelection]._spellIndex;
				int spellId = Res.SPELLS_ALLOWED[category][spellIndex];
				int spellCost = spells.calcSpellCost(spellId, expenseFactor);

				if (mode) {
					// Casting
					selection = newSelection;
				} else {
					// Guild spells dialog: Spells Info or Buy
					const Common::String &spellName = spells._spellNames[spellId];
					const Common::String &spellDesc = _textStrings[spellId];

					Common::String msg = (modeCopy & 0x80) ?
						Common::String::format(Res.SPELL_INFO, spellName.c_str(), spellDesc.c_str()) :
						Common::String::format(Res.SPELL_PURCHASE, spellDesc.c_str(), spellName.c_str(), spellCost);

					if (Confirm::show(_vm, msg, modeCopy + 1)) {
						if (party.subtract(CONS_GOLD, spellCost, WHERE_PARTY, WT_FREEZE_WAIT)) {
							c->_spells[spellIndex] = true;
							sound.stopSound();
							intf._overallFrame = 0;
							sound.playSound(ccNum ? "parrot2.voc" : "guild12.voc", 1);
						} else {
							sound.playFX(21);
						}
					}
				}
			}
			break;

		case Common::KEYCODE_PAGEUP:
		case Common::KEYCODE_KP9:
			topIndex = MAX((int)topIndex - 10, 0);
			break;

		case Common::KEYCODE_PAGEDOWN:
		case Common::KEYCODE_KP3:
			topIndex = MIN(topIndex + 10, (((int)_spells.size() - 1) / 10) * 10);
			break;

		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			if (topIndex > 0)
				--topIndex;
			break;

		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			if (topIndex < ((int)_spells.size() - 10))
				++topIndex;
			break;

		default:
			break;
		}
	} while (!_vm->shouldExit() && _buttonValue != Common::KEYCODE_ESCAPE);

	w.close();

	if (_vm->shouldExit())
		selection = -1;
	if (mode && selection != -1)
		c->_currentSpell = _spells[selection]._spellIndex;

	return c;
}

void SpellsDialog::loadButtons() {
	_iconSprites.load("main.icn");
	_scrollSprites.load("scroll.icn");
	addButton(Common::Rect(187, 26, 198, 36), Common::KEYCODE_UP, &_scrollSprites);
	addButton(Common::Rect(187, 111, 198, 121), Common::KEYCODE_DOWN, &_scrollSprites);
	addButton(Common::Rect(40, 28, 187, 36), Common::KEYCODE_1);
	addButton(Common::Rect(40, 37, 187, 45), Common::KEYCODE_2);
	addButton(Common::Rect(40, 46, 187, 54), Common::KEYCODE_3);
	addButton(Common::Rect(40, 55, 187, 63), Common::KEYCODE_4);
	addButton(Common::Rect(40, 64, 187, 72), Common::KEYCODE_5);
	addButton(Common::Rect(40, 73, 187, 81), Common::KEYCODE_6);
	addButton(Common::Rect(40, 82, 187, 90), Common::KEYCODE_7);
	addButton(Common::Rect(40, 91, 187, 99), Common::KEYCODE_8);
	addButton(Common::Rect(40, 100, 187, 108), Common::KEYCODE_9);
	addButton(Common::Rect(40, 109, 187, 117), Common::KEYCODE_0);
	addButton(Common::Rect(174, 123, 198, 133), Common::KEYCODE_ESCAPE);
	addButton(Common::Rect(187, 35, 198, 73), Common::KEYCODE_PAGEUP);
	addButton(Common::Rect(187, 74, 198, 112), Common::KEYCODE_PAGEDOWN);
	addButton(Common::Rect(132, 123, 168, 133), Common::KEYCODE_s);
	addPartyButtons(_vm);
}

const char *SpellsDialog::setSpellText(Character *c, int mode) {
	Party &party = *_vm->_party;
	Spells &spells = *_vm->_spells;
	int ccNum = _vm->_files->_ccNum;
	int currLevel = c->getCurrentLevel();
	SpellsCategory category = c->getSpellsCategory();
	int expenseFactor = c->getSpellsExpenseFactor();

	if ((mode & 0x7f) == 0) {
		if (category != SPELLCAT_INVALID) {
			if (_vm->getGameID() == GType_Swords && party._mazeId == 49) {
				for (int spellId = 0; spellId < 10; ++spellId) {
					int idx = 0;
					while (idx < SPELLS_PER_CLASS && Res.SPELLS_ALLOWED[category][idx] !=
						Res.DARK_SPELL_OFFSETS[category][spellId])
						++idx;

					if (idx < SPELLS_PER_CLASS) {
						if (!c->_spells[idx] || (mode & 0x80)) {
							int cost = spells.calcSpellCost(Res.SPELLS_ALLOWED[category][idx], expenseFactor);
							_spells.push_back(SpellEntry(Common::String::format("\x3l%s\x3r\x9""000%u",
								spells._spellNames[Res.SPELLS_ALLOWED[category][idx]].c_str(), cost),
								idx, spellId));
						}
					}
				}
			} else if (party._mazeId == 49 || party._mazeId == 37) {
				for (uint spellId = 0; spellId < TOTAL_SPELLS; ++spellId) {
					int idx = 0;
					while (idx < SPELLS_PER_CLASS && Res.SPELLS_ALLOWED[category][idx] != (int)spellId)
						++idx;

					// Handling if the spell is appropriate for the character's class
					if (idx < SPELLS_PER_CLASS) {
						if (!c->_spells[idx] || (mode & 0x80)) {
							int cost = spells.calcSpellCost(Res.SPELLS_ALLOWED[category][idx], expenseFactor);
							_spells.push_back(SpellEntry(Common::String::format("\x3l%s\x3r\x9""000%u",
								spells._spellNames[Res.SPELLS_ALLOWED[category][idx]].c_str(), cost),
								idx, spellId));
						}
					}
				}
			} else if (ccNum) {
				const int *RANGE;

				if (_vm->getGameID() == GType_Swords) {
					// Set subset of spells to sell in each Swords of Xeen guild
					int groupIndex;
					switch (party._mazeId) {
					case 92:
						groupIndex = 1;
						break;
					case 63:
						groupIndex = 2;
						break;
					case 53:
					default:
						groupIndex = 0;
						break;
					}
					RANGE = Res.SWORDS_SPELL_RANGES[category * 4 + groupIndex];
				} else {
					int groupIndex = (party._mazeId - 29) / 2;
					RANGE = Res.DARK_SPELL_RANGES[category * 4 + groupIndex];
				}

				for (int spellId = RANGE[0]; spellId < RANGE[1]; ++spellId) {
					int idx = 0;
					while (idx < SPELLS_PER_CLASS && Res.SPELLS_ALLOWED[category][idx] !=
							Res.DARK_SPELL_OFFSETS[category][spellId])
						++idx;

					if (idx < SPELLS_PER_CLASS) {
						if (!c->_spells[idx] || (mode & 0x80)) {
							int cost = spells.calcSpellCost(Res.SPELLS_ALLOWED[category][idx], expenseFactor);
							_spells.push_back(SpellEntry(Common::String::format("\x3l%s\x3r\x9""000%u",
								spells._spellNames[Res.SPELLS_ALLOWED[category][idx]].c_str(), cost),
								idx, spellId));
						}
					}
				}
			} else {
				for (int spellId = 0; spellId < 20; ++spellId) {
					int idx = 0;
					while (idx < SPELLS_PER_CLASS && Res.CLOUDS_GUILD_SPELLS[party._mazeId - 28][spellId] !=
							(int)Res.SPELLS_ALLOWED[category][idx])
						++idx;

					if (idx < SPELLS_PER_CLASS) {
						if (!c->_spells[idx] || (mode & 0x80)) {
							int cost = spells.calcSpellCost(Res.SPELLS_ALLOWED[category][idx], expenseFactor);
							_spells.push_back(SpellEntry(Common::String::format("\x3l%s\x3r\x9""000%u",
								spells._spellNames[Res.SPELLS_ALLOWED[category][idx]].c_str(), cost),
								idx, spellId));
						}
					}
				}
			}
		}

		if (c->getMaxSP() == 0)
			return Res.NOT_A_SPELL_CASTER;

	} else if ((mode & 0x7f) == 1) {
		if (c->getMaxSP() == 0) {
			return Res.NOT_A_SPELL_CASTER;
		} else {
			for (int spellIndex = 0; spellIndex < SPELLS_PER_CLASS; ++spellIndex) {
				if (c->_spells[spellIndex]) {
					int spellId = Res.SPELLS_ALLOWED[category][spellIndex];
					int gemCost = Res.SPELL_GEM_COST[spellId];
					int spCost = spells.calcSpellPoints(spellId, currLevel);

					Common::String msg = Common::String::format("\x3l%s\x3r\x9""000%u/%u",
						spells._spellNames[spellId].c_str(), spCost, gemCost);
					_spells.push_back(SpellEntry(msg, spellIndex, spellId));
				}
			}
		}
	}

	return _spells.empty() ? Res.SPELLS_LEARNED_ALL : nullptr;
}

/*------------------------------------------------------------------------*/

CastSpell::CastSpell(XeenEngine *vm) : ButtonContainer(vm) {
	Windows &windows = *_vm->_windows;
	_oldMode = _vm->_mode;
	_vm->_mode = MODE_3;

	windows[10].open();
	loadButtons();
}

CastSpell::~CastSpell() {
	Interface &intf = *_vm->_interface;
	Windows &windows = *_vm->_windows;

	windows[10].close();
	intf.unhighlightChar();

	_vm->_mode = (Mode)_oldMode;
}


int CastSpell::show(XeenEngine *vm) {
	Combat &combat = *vm->_combat;
	Interface &intf = *vm->_interface;
	Party &party = *vm->_party;
	Spells &spells = *vm->_spells;
	int result = 0, spellId = 0;
	int charNum;

	// Get which character is doing the casting
	if (vm->_mode == MODE_COMBAT) {
		charNum = combat._whosTurn;
	} else if (spells._lastCaster >= 0 && spells._lastCaster < (int)party._activeParty.size()) {
		charNum = spells._lastCaster;
	} else {
		for (charNum = (int)party._activeParty.size() - 1; charNum >= 0; --charNum) {
			if (party._activeParty[charNum]._hasSpells) {
				spells._lastCaster = charNum;
				break;
			}
		}
	}

	// Highlight the character
	Character *c = &party._activeParty[charNum];
	intf.highlightChar(c);

	CastSpell *dlg = new CastSpell(vm);
	do {
		spellId = dlg->execute(c);

		if (g_vm->shouldExit() || spellId == -1) {
			result = -1;
			break;
		} else {
			result = spells.castSpell(c, (MagicSpell)spellId);
		}
	} while (result == -1);

	delete dlg;
	return result;
}

int CastSpell::execute(Character *&c) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[10];

	int spellId = -1;
	bool redrawFlag = true;
	do {
		if (redrawFlag) {
			SpellsCategory category = c->getSpellsCategory();

			int spellIndex = c->_currentSpell != -1 ? c->_currentSpell : 39;
			spellId = (category == SPELLCAT_INVALID) ? NO_SPELL : Res.SPELLS_ALLOWED[category][spellIndex];
			int gemCost = Res.SPELL_GEM_COST[spellId];
			int spCost = spells.calcSpellPoints(spellId, c->getCurrentLevel());

			w.writeString(Common::String::format(Res.CAST_SPELL_DETAILS,
				c->_name.c_str(), spells._spellNames[spellId].c_str(),
				spCost, gemCost, c->_currentSp));
			drawButtons(&windows[0]);
			w.update();

			redrawFlag = false;
		}

		events.updateGameCounter();
		intf.draw3d(true);

		// Wait for event or time expiry
		do {
			events.pollEventsAndWait();
			checkEvents(_vm);
		} while (!_vm->shouldExit() && events.timeElapsed() < 1 && !_buttonValue);

		switch (_buttonValue) {
		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			// Only allow changing character if the party is not in combat
			if (_oldMode != MODE_COMBAT) {
				_vm->_mode = (Mode)_oldMode;
				_buttonValue -= Common::KEYCODE_F1;

				if (_buttonValue < (int)party._activeParty.size()) {
					c = &party._activeParty[_buttonValue];
					intf.highlightChar(_buttonValue);
					spells._lastCaster = _buttonValue;
					redrawFlag = true;
					break;
				}
			}
			break;

		case Common::KEYCODE_ESCAPE:
			spellId = -1;
			break;

		case Common::KEYCODE_c:
			// Cast spell - return the selected spell Id to be cast
			if (c->_currentSpell != -1 && !c->noActions())
				_buttonValue = Common::KEYCODE_ESCAPE;
			break;

		case Common::KEYCODE_n:
			// Select new spell
			_vm->_mode = (Mode)_oldMode;
			c = SpellsDialog::show(_vm, this, c, SPELLS_DIALOG_SELECT);
			redrawFlag = true;
			break;

		default:
			break;
		}
	} while (!_vm->shouldExit() && _buttonValue != Common::KEYCODE_ESCAPE);

	if (_vm->shouldExit())
		spellId = -1;
	return spellId;
}

void CastSpell::loadButtons() {
	_iconSprites.load("cast.icn");
	addButton(Common::Rect(234, 108, 259, 128), Common::KEYCODE_c, &_iconSprites);
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_n, &_iconSprites);
	addButton(Common::Rect(288, 108, 312, 128), Common::KEYCODE_ESCAPE, &_iconSprites);
	addPartyButtons(_vm);
}

/*------------------------------------------------------------------------*/

Character *SpellOnWho::show(XeenEngine *vm, int spellId) {
	SpellOnWho *dlg = new SpellOnWho(vm);
	int result = dlg->execute(spellId);
	delete dlg;

	if (result == -1)
		return nullptr;

	Combat &combat = *vm->_combat;
	Party &party = *vm->_party;
	return combat._combatMode == 2 ? combat._combatParty[result] :
		&party._activeParty[result];
}

int SpellOnWho::execute(int spellId) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[16];
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_3;
	int result = 999;

	w.open();
	w.writeString(Res.ON_WHO);
	w.update();
	addPartyButtons(_vm);

	while (result == 999) {
		do {
			events.updateGameCounter();
			intf.draw3d(true);

			do {
				events.pollEventsAndWait();
				if (_vm->shouldExit())
					return -1;

				checkEvents(_vm);
			} while (!_buttonValue && events.timeElapsed() < 1);
		} while (!_buttonValue);

		switch (_buttonValue) {
		case Common::KEYCODE_ESCAPE:
			result = -1;
			spells.addSpellCost(*combat._oldCharacter, spellId);
			break;

		case Common::KEYCODE_F1:
		case Common::KEYCODE_F2:
		case Common::KEYCODE_F3:
		case Common::KEYCODE_F4:
		case Common::KEYCODE_F5:
		case Common::KEYCODE_F6:
			_buttonValue -= Common::KEYCODE_F1;
			if (_buttonValue < (int)(combat._combatMode == 2 ? combat._combatParty.size() :
					party._activeParty.size())) {
				result = _buttonValue;
			}
			break;

		default:
			break;
		}
	}

	w.close();
	_vm->_mode = oldMode;
	return result;
}

/*------------------------------------------------------------------------*/

int SelectElement::show(XeenEngine *vm, int spellId) {
	SelectElement *dlg = new SelectElement(vm);
	int result = dlg->execute(spellId);
	delete dlg;

	return result;
}

int SelectElement::execute(int spellId) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[15];
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_3;
	int result = 999;

	loadButtons();

	w.open();
	w.writeString(Res.WHICH_ELEMENT1);
	drawButtons(&windows[0]);
	w.update();

	while (result == 999) {
		do {
			events.updateGameCounter();
			intf.draw3d(true, false);
			w.frame();
			w.writeString(Res.WHICH_ELEMENT2);
			drawButtons(&windows[0]);
			w.update();

			do {
				events.pollEventsAndWait();
				if (_vm->shouldExit())
					return -1;

				checkEvents(_vm);
			} while (!_buttonValue && events.timeElapsed() < 1);
		} while (!_buttonValue);

		switch (_buttonValue) {
		case Common::KEYCODE_ESCAPE:
			result = -1;
			spells.addSpellCost(*combat._oldCharacter, spellId);
			break;

		case Common::KEYCODE_a:
			result = DT_POISON;
			break;
		case Common::KEYCODE_c:
			result = DT_COLD;
			break;
		case Common::KEYCODE_e:
			result = DT_ELECTRICAL;
			break;
		case Common::KEYCODE_f:
			result = DT_FIRE;
			break;
		default:
			break;
		}
	}

	w.close();
	_vm->_mode = oldMode;
	return result;
}

void SelectElement::loadButtons() {
	_iconSprites.load("element.icn");
	addButton(Common::Rect(60, 92, 84, 112), Common::KEYCODE_f, &_iconSprites);
	addButton(Common::Rect(90, 92, 114, 112), Common::KEYCODE_e, &_iconSprites);
	addButton(Common::Rect(120, 92, 144, 112), Common::KEYCODE_c, &_iconSprites);
	addButton(Common::Rect(150, 92, 174, 112), Common::KEYCODE_a, &_iconSprites);
}

/*------------------------------------------------------------------------*/

void NotWhileEngaged::show(XeenEngine *vm, int spellId) {
	NotWhileEngaged *dlg = new NotWhileEngaged(vm);
	dlg->execute(spellId);
	delete dlg;
}

void NotWhileEngaged::execute(int spellId) {
	EventsManager &events = *_vm->_events;
	Spells &spells = *_vm->_spells;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_3;

	w.open();
	w.writeString(Common::String::format(Res.CANT_CAST_WHILE_ENGAGED,
		spells._spellNames[spellId].c_str()));
	w.update();

	while (!_vm->shouldExit() && !events.isKeyMousePressed())
		events.pollEventsAndWait();
	events.clearEvents();

	w.close();
	_vm->_mode = oldMode;
}

/*------------------------------------------------------------------------*/

bool LloydsBeacon::show(XeenEngine *vm) {
	LloydsBeacon *dlg = new LloydsBeacon(vm);
	bool result = dlg->execute();
	delete dlg;

	return result;
}

bool LloydsBeacon::execute() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[10];
	int ccNum = _vm->_files->_ccNum;
	Character &c = *combat._oldCharacter;

	loadButtons();

	if (!c._lloydMap) {
		// No destination previously set, so have a default ready
		if (ccNum) {
			c._lloydSide = 1;
			c._lloydPosition = Common::Point(25, 21);
			c._lloydMap = 29;
		} else {
			c._lloydSide = 0;
			c._lloydPosition = Common::Point(18, 4);
			c._lloydMap = 28;
		}
	}

	// Get the destination map name
	Common::String mapName = Map::getMazeName(c._lloydMap, c._lloydSide);

	// Display the dialog
	w.open();
	w.writeString(Common::String::format(Res.LLOYDS_BEACON, mapName.c_str(),
		c._lloydPosition.x, c._lloydPosition.y));
	drawButtons(&w);
	w.update();

	bool result = true;
	do {
		do {
			events.updateGameCounter();
			intf.draw3d(true);

			do {
				events.pollEventsAndWait();
				if (_vm->shouldExit())
					return true;

				checkEvents(_vm);
			} while (!_buttonValue && events.timeElapsed() < 1);
		} while (!_buttonValue);

		switch (_buttonValue) {
		case Common::KEYCODE_r:
			if (!ccNum && c._lloydMap >= XEEN_CASTLE1 && c._lloydMap <= XEEN_CASTLE4 && party._cloudsCompleted) {
				// Xeen's Castle has already been destroyed
				result = false;
			} else {
				sound.playFX(51);
				if (c._lloydMap != party._mazeId || c._lloydSide != ccNum) {
					map._loadCcNum = c._lloydSide;
					map.load(c._lloydMap);
				}

				party._mazePosition = c._lloydPosition;
			}

			_buttonValue = Common::KEYCODE_ESCAPE;
			break;

		case Common::KEYCODE_s:
		case Common::KEYCODE_t:
			sound.playFX(20);
			c._lloydMap = party._mazeId;
			c._lloydPosition = party._mazePosition;
			c._lloydSide = ccNum;

			_buttonValue = Common::KEYCODE_ESCAPE;
			break;

		default:
			break;
		}
	} while (_buttonValue != Common::KEYCODE_ESCAPE);

	w.close();
	return result;
}

void LloydsBeacon::loadButtons() {
	_iconSprites.load("lloyds.icn");

	addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_r, &_iconSprites);
	addButton(Common::Rect(242, 108, 266, 128), Common::KEYCODE_t, &_iconSprites);
}

/*------------------------------------------------------------------------*/

int Teleport::show(XeenEngine *vm) {
	Teleport *dlg = new Teleport(vm);
	int result = dlg->execute();
	delete dlg;

	return result;
}

int Teleport::execute() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Window &w = windows[6];
	Common::String num;

	w.open();
	w.writeString(Common::String::format(Res.HOW_MANY_SQUARES,
		Res.DIRECTION_TEXT[party._mazeDirection]));
	w.update();
	int lineSize = Input::show(_vm, &w, num, 1, 200, true);
	w.close();

	if (!lineSize)
		return -1;
	int numSquares = atoi(num.c_str());
	Common::Point pt = party._mazePosition;
	int v;

	switch (party._mazeDirection) {
	case DIR_NORTH:
		pt.y += numSquares;
		break;
	case DIR_EAST:
		pt.x += numSquares;
		break;
	case DIR_SOUTH:
		pt.y -= numSquares;
		break;
	case DIR_WEST:
		pt.x -= numSquares;
		break;
	default:
		break;
	}

	v = map.mazeLookup(pt, 0, map._isOutdoors ? 0xF : 0xFFFF);

	if ((v != (map._isOutdoors ? 0 : INVALID_CELL)) && (!map._isOutdoors || v != SURFTYPE_DWATER)) {
		party._mazePosition = pt;
		return 1;
	} else {
		return 0;
	}
}

/*------------------------------------------------------------------------*/

int TownPortal::show(XeenEngine *vm) {
	TownPortal *dlg = new TownPortal(vm);
	int townNumber = dlg->execute();
	delete dlg;

	return townNumber;
}

int TownPortal::execute() {
	Map &map = *_vm->_map;
	Windows &windows = *_vm->_windows;
	Window &w = windows[20];
	Common::String townNames[5];
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_FF;

	w.open();

	if (_vm->getGameID() == GType_Swords) {
		// Build up a lsit of the names of the towns on the current side of Xeen
		for (int idx = 0; idx < 3; ++idx) {
			Common::String txtName = Common::String::format("%s%04d.txt", "dark", Res.TOWN_MAP_NUMBERS[2][idx]);
			File f(txtName, 1);
			townNames[idx] = f.readString();
			f.close();
		}

		w.writeString(Common::String::format(Res.TOWN_PORTAL_SWORDS, townNames[0].c_str(), townNames[1].c_str(),
			townNames[2].c_str()));
	} else {
		// Build up a lsit of the names of the towns on the current side of Xeen
		for (int idx = 0; idx < 5; ++idx) {
			Common::String txtName = Common::String::format("%s%04d.txt", map._sideTownPortal ? "dark" : "xeen",
				Res.TOWN_MAP_NUMBERS[map._sideTownPortal][idx]);
			File f(txtName, 1);
			townNames[idx] = f.readString();
			f.close();
		}

		w.writeString(Common::String::format(Res.TOWN_PORTAL,
			townNames[0].c_str(), townNames[1].c_str(), townNames[2].c_str(),
			townNames[3].c_str(), townNames[4].c_str()
		));
	}

	w.update();

	// Get the town number
	int townNumber;
	Common::String num;
	do {
		int result = Input::show(_vm, &w, num, 1, 160, true);
		townNumber = !result ? 0 : atoi(num.c_str());
	} while (townNumber > (_vm->getGameID() == GType_Swords ? 3 : 5));

	w.close();
	_vm->_mode = oldMode;

	return townNumber;
}

/*------------------------------------------------------------------------*/

void IdentifyMonster::show(XeenEngine *vm) {
	IdentifyMonster *dlg = new IdentifyMonster(vm);
	dlg->execute();
	delete dlg;
}

void IdentifyMonster::execute() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[17];
	Common::String monsterDesc[3];

	for (int monIndex = 0; monIndex < 3; ++monIndex) {
		if (combat._attackMonsters[monIndex] == -1)
			continue;

		MazeMonster &monster = map._mobData._monsters[combat._attackMonsters[monIndex]];
		MonsterStruct &monsterData = *monster._monsterData;

		monsterDesc[monIndex] = Common::String::format(Res.MONSTER_DETAILS,
			monsterData._name.c_str(),
			_vm->printK2(monster._hp).c_str(),
			monsterData._armorClass, monsterData._numberOfAttacks,
			Res.MONSTER_SPECIAL_ATTACKS[monsterData._specialAttack]
		);
	}

	sound.playFX(20);
	w.open();
	w.writeString(Common::String::format(Res.IDENTIFY_MONSTERS,
		monsterDesc[0].c_str(), monsterDesc[1].c_str(), monsterDesc[2].c_str()));
	w.update();

	do {
		events.updateGameCounter();
		intf.draw3d(false, false);
		w.frame();
		windows[3].update();

		events.wait(1, false);
	} while (!events.isKeyMousePressed());

	w.close();
}


/*------------------------------------------------------------------------*/

void DetectMonsters::show(XeenEngine *vm) {
	DetectMonsters *dlg = new DetectMonsters(vm);
	dlg->execute();
	delete dlg;
}

void DetectMonsters::execute() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Resources &res = *_vm->_resources;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[19];
	int ccNum = _vm->_files->_ccNum;
	int grid[7][7];

	SpriteResource sprites(ccNum ? "detectmn.icn" : "detctmon.icn");
	Common::fill(&grid[0][0], &grid[6][6], 0);

	w.open();
	w.writeString(Res.DETECT_MONSTERS);
	sprites.draw(w, 0, Common::Point(243, 80));

	for (int yDiff = 3; yDiff >= -3; --yDiff) {
		for (int xDiff = -3; xDiff <= 3; ++xDiff) {
			for (uint monIndex = 0; monIndex < map._mobData._monsters.size(); ++monIndex) {
				MazeMonster &monster = map._mobData._monsters[monIndex];
				Common::Point pt = party._mazePosition + Common::Point(xDiff, yDiff);
				if (monster._position == pt) {
					int &gridEntry = grid[yDiff + 3][xDiff + 3];
					if (++gridEntry > 3)
						gridEntry = 3;

					sprites.draw(w, gridEntry, Common::Point(271 + xDiff * 9, 102 - yDiff * 7));
				}
			}
		}
	}

	res._globalSprites.draw(w, party._mazeDirection + 1, Common::Point(270, 101));
	sound.playFX(20);
	w.update();

	while (!g_vm->shouldExit() && !events.isKeyMousePressed()) {
		events.updateGameCounter();
		intf.draw3d(true);

		events.wait(1, false);
	}

	w.close();
}

} // End of namespace Xeen
