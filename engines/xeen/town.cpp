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

#include "xeen/town.h"
#include "xeen/dialogs_input.h"
#include "xeen/dialogs_items.h"
#include "xeen/dialogs_query.h"
#include "xeen/dialogs_spells.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

TownLocation::TownLocation(TownAction action) : ButtonContainer(g_vm),
		_townActionId(action), _isDarkCc(g_vm->_files->_isDarkCc),
		_vocName("hello1.voc") {
	_townMaxId = Res.TOWN_MAXES[_isDarkCc][action];
	_songName = Res.TOWN_ACTION_MUSIC[_isDarkCc][action];
	_townSprites.resize(Res.TOWN_ACTION_FILES[_isDarkCc][action]);

	_animFrame = 0;
	_drawFrameIndex = 0;
	_farewellTime = 0;
	_drawCtr1 = _drawCtr2 = 0;
	_townPos = Common::Point(8, 8);
}

TownLocation::~TownLocation() {
	Interface &intf = *g_vm->_interface;

	for (uint idx = 0; idx < _townSprites.size(); ++idx)
		_townSprites[idx].clear();
	intf.mainIconsPrint();
}

void TownLocation::loadStrings(const Common::String &name) {
	File f(name);
	_textStrings.clear();
	while (f.pos() < f.size())
		_textStrings.push_back(f.readString());
	f.close();
}

int TownLocation::show() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	// Play the appropriate music
	sound.stopSound();
	sound.playSong(_songName, 223);

	// Load the needed sprite sets for the location
	for (uint idx = 0; idx < _townSprites.size(); ++idx) {
		Common::String shapesName = Common::String::format("%s%d.twn",
			Res.TOWN_ACTION_SHAPES[_townActionId], idx + 1);
		_townSprites[idx].load(shapesName);
	}

	Character *charP = &party._activeParty[0];

	// Draw the background and the text window
	drawBackground();
	drawWindow();
	drawAnim(true);

	// Play the welcome speech
	sound.playSound(_vocName, 1);

	do {
		wait();
		charP = doOptions(charP);
		if (_vm->shouldQuit())
			return 0;

		Common::String msg = createLocationText(*charP);
		windows[10].writeString(msg);
		drawButtons(&windows[0]);
	} while (_buttonValue != Common::KEYCODE_ESCAPE);

	// Handle any farewell message
	farewell();

	int result;
	if (party._mazeId != 0) {
		map.load(party._mazeId);
		_farewellTime += 1440;
		party.addTime(_farewellTime);
		result = 0;
	} else {
		_vm->_saves->saveChars();
		result = 2;
	}

	return result;
}

void TownLocation::drawBackground() {
	Interface &intf = *g_vm->_interface;

	intf._face1UIFrame = intf._face2UIFrame = 0;
	intf._dangerSenseUIFrame = 0;
	intf._spotDoorsUIFrame = 0;
	intf._levitateUIFrame = 0;
	_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _townPos);
}

void TownLocation::drawWindow() {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Windows &windows = *g_vm->_windows;

	Character *charP = &party._activeParty[0];
	Common::String title = createLocationText(*charP);

	// Open up the window and write the string
	intf.assembleBorder();
	windows[10].open();
	windows[10].writeString(title);
	drawButtons(&windows[0]);

	windows[0].update();
	intf.highlightChar(0);
}

void TownLocation::drawAnim(bool flag) {
	Interface &intf = *g_vm->_interface;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	// TODO: Figure out a clean way to split method into individual location classes
	if (_townActionId == BLACKSMITH) {
		if (sound.isPlaying()) {
			if (_isDarkCc) {
				_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _townPos);
				_townSprites[2].draw(0, _vm->getRandomNumber(11) == 1 ? 9 : 10,
					Common::Point(34, 33));
				_townSprites[2].draw(0, _vm->getRandomNumber(5) + 3,
					Common::Point(34, 54));
			}
		} else {
			_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _townPos);
			if (_isDarkCc) {
				_townSprites[2].draw(0, _vm->getRandomNumber(11) == 1 ? 9 : 10,
					Common::Point(34, 33));
			}
		}
	} else if (!_isDarkCc || _townActionId != TRAINING) {
		if (!_townSprites[_drawFrameIndex / 8].empty())
			_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _townPos);
	}

	switch (_townActionId) {
	case BANK:
		if (sound.isPlaying() || (_isDarkCc && _animFrame)) {
			if (_isDarkCc) {
				if (sound.isPlaying() || _animFrame == 1) {
					_townSprites[4].draw(0, _vm->getRandomNumber(13, 18),
						Common::Point(8, 30));
				} else if (_animFrame > 1) {
					_townSprites[4].draw(0, 13 - _animFrame++,
						Common::Point(8, 30));
					if (_animFrame > 14)
						_animFrame = 0;
				}
			} else {
				_townSprites[2].draw(0, _vm->getRandomNumber(7, 11), Common::Point(8, 8));
			}
		}
		break;

	case GUILD:
		if (sound.isPlaying()) {
			if (_isDarkCc) {
				if (_animFrame) {
					_animFrame ^= 1;
					_townSprites[6].draw(0, _animFrame, Common::Point(8, 106));
				} else {
					_townSprites[6].draw(0, _vm->getRandomNumber(3), Common::Point(16, 48));
				}
			}
		}
		break;

	case TAVERN:
		if (sound.isPlaying() && _isDarkCc) {
			_townSprites[4].draw(0, _vm->getRandomNumber(7), Common::Point(153, 49));
		}
		break;

	case TEMPLE:
		if (sound.isPlaying()) {
			_townSprites[3].draw(0, _vm->getRandomNumber(2, 4), Common::Point(8, 8));

		}
		break;

	case TRAINING:
		if (sound.isPlaying()) {
			if (_isDarkCc) {
				_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _townPos);
			}
		} else {
			if (_isDarkCc) {
				_townSprites[0].draw(0, ++_animFrame % 8, Common::Point(8, 8));
				_townSprites[5].draw(0, _vm->getRandomNumber(5), Common::Point(61, 74));
			} else {
				_townSprites[1].draw(0, _vm->getRandomNumber(8, 12), Common::Point(8, 8));
			}
		}
		break;

	default:
		break;
	}

	if (flag) {
		intf._face1UIFrame = 0;
		intf._face2UIFrame = 0;
		intf._dangerSenseUIFrame = 0;
		intf._spotDoorsUIFrame = 0;
		intf._levitateUIFrame = 0;

		intf.assembleBorder();
	}

	if (windows[11]._enabled) {
		_drawCtr1 = (_drawCtr1 + 1) % 2;
		if (!_drawCtr1 || !_drawCtr2) {
			_drawFrameIndex = 0;
			_drawCtr2 = 0;
		} else {
			_drawFrameIndex = _vm->getRandomNumber(3);
		}
	} else {
		_drawFrameIndex = (_drawFrameIndex + 1) % _townMaxId;
	}

	if (_isDarkCc) {
		if (_townActionId == BLACKSMITH && (_drawFrameIndex == 4 || _drawFrameIndex == 13))
			sound.playFX(45);

		if (_townActionId == TRAINING && _drawFrameIndex == 23) {
			sound.playSound("spit1.voc");
		}
	} else {
		if (_townMaxId == 32 && _drawFrameIndex == 0)
			_drawFrameIndex = 17;
		if (_townMaxId == 26 && _drawFrameIndex == 0)
			_drawFrameIndex = 20;
		if (_townActionId == BLACKSMITH && (_drawFrameIndex == 3 || _drawFrameIndex == 9))
			sound.playFX(45);
	}

	windows[3].update();

	if (_townActionId == BANK)
		_animFrame = 2;
}

int TownLocation::wait() {
	EventsManager &events = *g_vm->_events;
	Windows &windows = *g_vm->_windows;

	_buttonValue = 0;
	while (!_vm->shouldQuit() && !_buttonValue) {
		events.updateGameCounter();
		while (!_vm->shouldQuit() && !_buttonValue && events.timeElapsed() < 3) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}
		if (!_buttonValue)
			drawAnim(!windows[11]._enabled);
	}

	return _buttonValue;
}

/*------------------------------------------------------------------------*/

BankLocation::BankLocation() : TownLocation(BANK) {
	_icons1.load("bank.icn");
	_icons2.load("bank2.icn");
	addButton(Common::Rect(234, 108, 259, 128), Common::KEYCODE_d, &_icons1);
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_w, &_icons1);
	addButton(Common::Rect(288, 108, 312, 128), Common::KEYCODE_ESCAPE, &_icons1);
	_animFrame = 1;

	_vocName = _isDarkCc ? "bank1.voc" : "banker.voc";
}

Common::String BankLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	return Common::String::format(Res.BANK_TEXT,
		XeenEngine::printMil(party._bankGold).c_str(),
		XeenEngine::printMil(party._bankGems).c_str(),
		XeenEngine::printMil(party._gold).c_str(),
		XeenEngine::printMil(party._gems).c_str());
}

void BankLocation::drawBackground() {
	if (_isDarkCc) {
		_townSprites[4].draw(0, _vm->getRandomNumber(13, 18),
			Common::Point(8, 30));
	}
}

Character *BankLocation::doOptions(Character *c) {
	if (_buttonValue == Common::KEYCODE_d)
		_buttonValue = (int)WHERE_PARTY;
	else if (_buttonValue == Common::KEYCODE_w)
		_buttonValue = (int)WHERE_BANK;
	else
		return c;

	depositWithdrawl((PartyBank)_buttonValue);
	return c;
}

void BankLocation::depositWithdrawl(PartyBank whereId) {
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;
	int gold, gems;

	if (whereId == WHERE_BANK) {
		gold = party._bankGold;
		gems = party._bankGems;
	} else {
		gold = party._gold;
		gems = party._gems;
	}

	for (uint idx = 0; idx < _buttons.size(); ++idx)
		_buttons[idx]._sprites = &_icons2;
	_buttons[0]._value = Common::KEYCODE_o;
	_buttons[1]._value = Common::KEYCODE_e;
	_buttons[2]._value = Common::KEYCODE_ESCAPE;

	Common::String msg = Common::String::format(Res.GOLD_GEMS,
		Res.DEPOSIT_WITHDRAWL[whereId],
		XeenEngine::printMil(gold).c_str(),
		XeenEngine::printMil(gems).c_str());

	windows[35].open();
	windows[35].writeString(msg);
	drawButtons(&windows[35]);
	windows[35].update();

	sound.stopSound();
	File voc("coina.voc");
	ConsumableType consType = CONS_GOLD;

	do {
		switch (wait()) {
		case Common::KEYCODE_o:
			consType = CONS_GOLD;
			break;
		case Common::KEYCODE_e:
			consType = CONS_GEMS;
			break;
		case Common::KEYCODE_ESCAPE:
			break;
		default:
			continue;
		}

		if ((whereId == WHERE_BANK && !party._bankGems && consType == CONS_GEMS) ||
			(whereId == WHERE_BANK && !party._bankGold && consType == CONS_GOLD) ||
			(whereId == WHERE_PARTY && !party._gems && consType == CONS_GEMS) ||
			(whereId == WHERE_PARTY && !party._gold && consType == CONS_GOLD)) {
			party.notEnough(consType, whereId, WHERE_BANK, WT_2);
		} else {
			windows[35].writeString(Res.AMOUNT);
			int amount = NumericInput::show(_vm, 35, 10, 77);

			if (amount) {
				if (consType == CONS_GEMS) {
					if (party.subtract(CONS_GEMS, amount, whereId, WT_2)) {
						if (whereId == WHERE_BANK) {
							party._gems += amount;
						} else {
							party._bankGems += amount;
						}
					}
				} else {
					if (party.subtract(CONS_GOLD, amount, whereId, WT_2)) {
						if (whereId == WHERE_BANK) {
							party._gold += amount;
						} else {
							party._bankGold += amount;
						}
					}
				}
			}

			if (whereId == WHERE_BANK) {
				gold = party._bankGold;
				gems = party._bankGems;
			}
			else {
				gold = party._gold;
				gems = party._gems;
			}

			sound.playSound(voc);
			msg = Common::String::format(Res.GOLD_GEMS_2, Res.DEPOSIT_WITHDRAWL[whereId],
				XeenEngine::printMil(gold).c_str(), XeenEngine::printMil(gems).c_str());
			windows[35].writeString(msg);
			windows[35].update();
		}
		// TODO
	} while (!g_vm->shouldQuit() && _buttonValue != Common::KEYCODE_ESCAPE);

	for (uint idx = 0; idx < _buttons.size(); ++idx)
		_buttons[idx]._sprites = &_icons1;
	_buttons[0]._value = Common::KEYCODE_d;
	_buttons[1]._value = Common::KEYCODE_w;
	_buttons[2]._value = Common::KEYCODE_ESCAPE;
}

/*------------------------------------------------------------------------*/

BlacksmithLocation::BlacksmithLocation() : TownLocation(BLACKSMITH) {
	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), 0);
	addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_b);
	addButton(Common::Rect(234, 74, 308, 82), 0);
	addButton(Common::Rect(234, 84, 308, 92), 0);

	_vocName = _isDarkCc ? "see2.voc" : "whaddayo.voc";
}

Common::String BlacksmithLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	return Common::String::format(Res.BLACKSMITH_TEXT,
		ch._name.c_str(), XeenEngine::printMil(party._gold).c_str());
}

Character *BlacksmithLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;

	if (_buttonValue >= Common::KEYCODE_F1 && _buttonValue <= Common::KEYCODE_F6) {
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
		}
	} else if (_buttonValue == Common::KEYCODE_b) {
		c = ItemsDialog::show(_vm, c, ITEMMODE_BLACKSMITH);
		_buttonValue = 0;
	}

	return c;
}

void BlacksmithLocation::farewell() {
	Sound &sound = *g_vm->_sound;

	if (_isDarkCc) {
		sound.stopSound();
		sound.playSound("come1.voc", 1);
	}
}

/*------------------------------------------------------------------------*/

GuildLocation::GuildLocation() : TownLocation(GUILD) {
	loadStrings("spldesc.bin");
	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), 0);
	addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_b);
	addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_s);
	addButton(Common::Rect(234, 84, 308, 92), 0);
	g_vm->_mode = MODE_17;

	_vocName = _isDarkCc ? "parrot1.voc" : "guild10.voc";
}

Common::String GuildLocation::createLocationText(Character &ch) {
	return !ch.guildMember() ? Res.GUILD_NOT_MEMBER_TEXT :
		Common::String::format(Res.GUILD_TEXT, ch._name.c_str());
}

Character *GuildLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	if (_buttonValue >= Common::KEYCODE_F1 && _buttonValue <= Common::KEYCODE_F6) {
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);

			if (!c->guildMember()) {
				sound.stopSound();
				_animFrame = 5;
				sound.playSound(_isDarkCc ? "skull1.voc" : "guild11.voc", 1);
			}
		}
	} else if (_buttonValue == Common::KEYCODE_s) {
		if (c->guildMember())
			c = SpellsDialog::show(_vm, nullptr, c, 0x80);
		_buttonValue = 0;
	} else if (_buttonValue == Common::KEYCODE_c) {
		if (!c->noActions()) {
			if (c->guildMember())
				c = SpellsDialog::show(_vm, nullptr, c, 0);
			_buttonValue = 0;
		}
	}

	return c;
}

/*------------------------------------------------------------------------*/

TavernLocation::TavernLocation() : TownLocation(TAVERN) {
	_v21 = 0;
	_v22 = 0;
	_v23 = 0;
	_v24 = 0;

	loadStrings("tavern.bin");
	_icons1.load("tavern.icn");
	addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(242, 108, 266, 128), Common::KEYCODE_s, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), Common::KEYCODE_d);
	addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_f);
	addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_t);
	addButton(Common::Rect(234, 84, 308, 92), Common::KEYCODE_r);
	g_vm->_mode = MODE_17;

	_vocName = _isDarkCc ? "hello1.voc" : "hello.voc";
}

Common::String TavernLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	return Common::String::format(Res.TAVERN_TEXT, ch._name.c_str(),
		Res.FOOD_AND_DRINK, XeenEngine::printMil(party._gold).c_str());
}

Character *TavernLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;
	int idx = 0;

	switch (_buttonValue) {
	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
			_v21 = 0;
		}
		break;

	case Common::KEYCODE_d:
		// Drink
		if (!c->noActions()) {
			if (party.subtract(CONS_GOLD, 1, WHERE_PARTY, WT_2)) {
				sound.stopSound();
				sound.playSound("gulp.voc");
				_v21 = 1;

				windows[10].writeString(Common::String::format(Res.TAVERN_TEXT,
					c->_name.c_str(), Res.GOOD_STUFF,
					XeenEngine::printMil(party._gold).c_str()));
				drawButtons(&windows[0]);
				windows[10].update();

				if (_vm->getRandomNumber(100) < 26) {
					++c->_conditions[DRUNK];
					intf.drawParty(true);
					sound.playFX(28);
				}

				wait();
			}
		}
		break;

	case Common::KEYCODE_f: {
		// Food
		if (party._mazeId == (_isDarkCc ? 29 : 28)) {
			_v22 = party._activeParty.size() * 15;
			_v23 = 10;
			idx = 0;
		} else if (_isDarkCc && party._mazeId == 31) {
			_v22 = party._activeParty.size() * 60;
			_v23 = 100;
			idx = 1;
		} else if (!_isDarkCc && party._mazeId == 30) {
			_v22 = party._activeParty.size() * 50;
			_v23 = 50;
			idx = 1;
		} else if (_isDarkCc) {
			_v22 = party._activeParty.size() * 120;
			_v23 = 250;
			idx = 2;
		} else if (party._mazeId == 49) {
			_v22 = party._activeParty.size() * 120;
			_v23 = 100;
			idx = 2;
		} else {
			_v22 = party._activeParty.size() * 15;
			_v23 = 10;
			idx = 0;
		}

		Common::String msg = _textStrings[(_isDarkCc ? 60 : 75) + idx];
		windows[10].close();
		windows[12].open();
		windows[12].writeString(msg);
		windows[12].update();

		if (YesNo::show(_vm, false, true)) {
			if (party._food >= _v22) {
				ErrorScroll::show(_vm, Res.FOOD_PACKS_FULL, WT_2);
			} else if (party.subtract(CONS_GOLD, _v23, WHERE_PARTY, WT_2)) {
				party._food = _v22;
				sound.stopSound();
				sound.playSound(_isDarkCc ? "thanks2.voc" : "thankyou.voc", 1);
			}
		}

		windows[12].close();
		windows[10].open();
		_buttonValue = 0;
		break;
	}

	case Common::KEYCODE_r: {
		// Rumors
		if (party._mazeId == (_isDarkCc ? 29 : 28)) {
			idx = 0;
		} else if (party._mazeId == (_isDarkCc ? 31 : 30)) {
			idx = 10;
		} else if (_isDarkCc || party._mazeId == 49) {
			idx = 20;
		}

		Common::String msg = Common::String::format("\x03""c\x0B""012%s",
			_textStrings[(party._day % 10) + idx].c_str());
		Window &w = windows[12];
		w.open();
		w.writeString(msg);
		w.update();

		wait();
		w.close();
		break;
	}

	case Common::KEYCODE_s: {
		// Sign In
		idx = _isDarkCc ? (party._mazeId - 29) >> 1 : party._mazeId - 28;
		assert(idx >= 0);
		party._mazePosition.x = Res.TAVERN_EXIT_LIST[_isDarkCc ? 1 : 0][_townActionId][idx][0];
		party._mazePosition.y = Res.TAVERN_EXIT_LIST[_isDarkCc ? 1 : 0][_townActionId][idx][1];

		if (!_isDarkCc || party._mazeId == 29)
			party._mazeDirection = DIR_WEST;
		else if (party._mazeId == 31)
			party._mazeDirection = DIR_EAST;
		else
			party._mazeDirection = DIR_SOUTH;

		party._priorMazeId = party._mazeId;
		for (idx = 0; idx < (int)party._activeParty.size(); ++idx) {
			party._activeParty[idx]._savedMazeId = party._mazeId;
			party._activeParty[idx]._xeenSide = map._loadDarkSide;
		}

		party.addTime(1440);
		party._mazeId = 0;
		_vm->_quitMode = 2;
		break;
	}

	case Common::KEYCODE_t:
		if (!c->noActions()) {
			if (!_v21) {
				windows[10].writeString(Common::String::format(Res.TAVERN_TEXT,
					c->_name.c_str(), Res.HAVE_A_DRINK,
					XeenEngine::printMil(party._gold).c_str()));
				drawButtons(&windows[0]);
				windows[10].update();
				wait();
			} else {
				_v21 = 0;
				if (c->_conditions[DRUNK]) {
					windows[10].writeString(Common::String::format(Res.TAVERN_TEXT,
						c->_name.c_str(), Res.YOURE_DRUNK,
						XeenEngine::printMil(party._gold).c_str()));
					drawButtons(&windows[0]);
					windows[10].update();
					wait();
				} else if (party.subtract(CONS_GOLD, 1, WHERE_PARTY, WT_2)) {
					sound.stopSound();
					sound.playSound(_isDarkCc ? "thanks2.voc" : "thankyou.voc", 1);

					if (party._mazeId == (_isDarkCc ? 29 : 28)) {
						_v24 = 30;
					} else if (_isDarkCc && party._mazeId == 31) {
						_v24 = 40;
					} else if (!_isDarkCc && party._mazeId == 45) {
						_v24 = 45;
					} else if (!_isDarkCc && party._mazeId == 49) {
						_v24 = 60;
					} else if (_isDarkCc) {
						_v24 = 50;
					}

					Common::String msg = _textStrings[map.mazeData()._tavernTips + _v24];
					map.mazeData()._tavernTips = (map.mazeData()._tavernTips + 1) /
						(_isDarkCc ? 10 : 15);

					Window &w = windows[12];
					w.open();
					w.writeString(Common::String::format("\x03""c\x0B""012%s", msg.c_str()));
					w.update();
					wait();
					w.close();
				}
			}
		}
		break;

	default:
		break;
	}

	return c;
}

void TavernLocation::farewell() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	sound.stopSound();
	sound.playSound(_isDarkCc ? "gdluck1.voc" : "goodbye.voc", 1);

	map.mazeData()._mazeNumber = party._mazeId;
}

/*------------------------------------------------------------------------*/

TempleLocation::TempleLocation() : TownLocation(TEMPLE) {
	_currentCharLevel = 0;
	_donation = 0;
	_healCost = 0;
	_uncurseCost = 0;
	_dayOfWeek = 0;
	_v10 = _v11 = 0;
	_v12 = _v13 = 0;
	_v14 = 0;
	_flag1 = false;
	_v5 = _v6 = 0;

	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), Common::KEYCODE_h);
	addButton(Common::Rect(234, 64, 308, 72), Common::KEYCODE_d);
	addButton(Common::Rect(234, 74, 308, 82), Common::KEYCODE_u);
	addButton(Common::Rect(234, 84, 308, 92), 0);

	_vocName = _isDarkCc ? "help2.voc" : "maywe2.voc";
}

Common::String TempleLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;

	if (party._mazeId == (_isDarkCc ? 29 : 28)) {
		_v10 = _v11 = _v12 = _v13 = 0;
		_v14 = 10;
	} else if (party._mazeId == (_isDarkCc ? 31 : 30)) {
		_v13 = 10;
		_v12 = 50;
		_v11 = 500;
		_v10 = 100;
		_v14 = 25;
	} else if (party._mazeId == (_isDarkCc ? 37 : 73)) {
		_v13 = 20;
		_v12 = 100;
		_v11 = 1000;
		_v10 = 200;
		_v14 = 50;
	} else if (_isDarkCc || party._mazeId == 49) {
		_v13 = 100;
		_v12 = 500;
		_v11 = 5000;
		_v10 = 300;
		_v14 = 100;
	}

	_currentCharLevel = ch.getCurrentLevel();
	if (ch._currentHp < ch.getMaxHP()) {
		_healCost = _currentCharLevel * 10 + _v13;
	}

	for (int attrib = HEART_BROKEN; attrib <= UNCONSCIOUS; ++attrib) {
		if (ch._conditions[attrib])
			_healCost += _currentCharLevel * 10;
	}

	_v6 = 0;
	if (ch._conditions[DEAD]) {
		_v6 += (_currentCharLevel * 100) + (ch._conditions[DEAD] * 50) + _v12;
	}
	if (ch._conditions[STONED]) {
		_v6 += (_currentCharLevel * 100) + (ch._conditions[STONED] * 50) + _v12;
	}
	if (ch._conditions[ERADICATED]) {
		_v5 = (_currentCharLevel * 1000) + (ch._conditions[ERADICATED] * 500) + _v11;
	}

	for (int idx = 0; idx < 9; ++idx) {
		_uncurseCost |= ch._weapons[idx]._bonusFlags & 0x40;
		_uncurseCost |= ch._armor[idx]._bonusFlags & 0x40;
		_uncurseCost |= ch._accessories[idx]._bonusFlags & 0x40;
		_uncurseCost |= ch._misc[idx]._bonusFlags & 0x40;
	}

	if (_uncurseCost || ch._conditions[CURSED])
		_v5 = (_currentCharLevel * 20) + _v10;

	_donation = _flag1 ? 0 : _v14;
	_healCost += _v6 + _v5;

	return Common::String::format(Res.TEMPLE_TEXT, ch._name.c_str(),
		_healCost, _donation, XeenEngine::printK(_uncurseCost).c_str(),
		XeenEngine::printMil(party._gold).c_str());
}

Character *TempleLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	switch (_buttonValue) {
	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
			_dayOfWeek = 0;
		}
		break;

	case Common::KEYCODE_d:
		if (_donation && party.subtract(CONS_GOLD, _donation, WHERE_PARTY, WT_2)) {
			sound.stopSound();
			sound.playSound("coina.voc", 1);
			_dayOfWeek = (_dayOfWeek + 1) / 10;

			if (_dayOfWeek == (party._day / 10)) {
				party._clairvoyanceActive = true;
				party._lightCount = 1;

				int amt = _dayOfWeek ? _dayOfWeek : 10;
				party._heroism = amt;
				party._holyBonus = amt;
				party._powerShield = amt;
				party._blessed = amt;

				intf.drawParty(true);
				sound.stopSound();
				sound.playSound("ahh.voc");
				_flag1 = true;
				_donation = 0;
			}
		}
		break;

	case Common::KEYCODE_h:
		if (_healCost && party.subtract(CONS_GOLD, _healCost, WHERE_PARTY, WT_2)) {
			c->_magicResistence._temporary = 0;
			c->_energyResistence._temporary = 0;
			c->_poisonResistence._temporary = 0;
			c->_electricityResistence._temporary = 0;
			c->_coldResistence._temporary = 0;
			c->_fireResistence._temporary = 0;
			c->_ACTemp = 0;
			c->_level._temporary = 0;
			c->_luck._temporary = 0;
			c->_accuracy._temporary = 0;
			c->_speed._temporary = 0;
			c->_endurance._temporary = 0;
			c->_personality._temporary = 0;
			c->_intellect._temporary = 0;
			c->_might._temporary = 0;
			c->_currentHp = c->getMaxHP();
			Common::fill(&c->_conditions[HEART_BROKEN], &c->_conditions[NO_CONDITION], 0);

			_farewellTime = 1440;
			intf.drawParty(true);
			sound.stopSound();
			sound.playSound("ahh.voc", 1);
		}
		break;

	case Common::KEYCODE_u:
		if (_uncurseCost && party.subtract(CONS_GOLD, _uncurseCost, WHERE_PARTY, WT_2)) {
			for (int idx = 0; idx < 9; ++idx) {
				c->_weapons[idx]._bonusFlags &= ~ITEMFLAG_CURSED;
				c->_armor[idx]._bonusFlags &= ~ITEMFLAG_CURSED;
				c->_accessories[idx]._bonusFlags &= ~ITEMFLAG_CURSED;
				c->_misc[idx]._bonusFlags &= ~ITEMFLAG_CURSED;
			}

			_farewellTime = 1440;
			intf.drawParty(true);
			sound.stopSound();
			sound.playSound("ahh.voc", 1);
		}
		break;

	default:
		break;
	}

	return c;
}

/*------------------------------------------------------------------------*/

TrainingLocation::TrainingLocation() : TownLocation(TRAINING) {
	Common::fill(&_charsTrained[0], &_charsTrained[6], 0);
	_maxLevel = 0;
	_experienceToNextLevel = 0;

	_icons1.load("train.icn");
	addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(242, 108, 266, 128), Common::KEYCODE_t, &_icons1);

	_vocName = _isDarkCc ? "training.voc" : "youtrn1.voc";
}

Common::String TrainingLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	if (_isDarkCc) {
		switch (party._mazeId) {
		case 29:
			// Castleview
			_maxLevel = 30;
			break;
		case 31:
			// Sandcaster
			_maxLevel = 50;
			break;
		case 37:
			// Olympus
			_maxLevel = 200;
			break;
		default:
			// Kalindra's Castle
			_maxLevel = 100;
			break;
		}
	} else {
		switch (party._mazeId) {
		case 28:
			// Vertigo
			_maxLevel = 10;
			break;
		case 30:
			// Rivercity
			_maxLevel = 15;
			break;
		default:
			// Newcastle
			_maxLevel = 20;
			break;
		}
	}

	_experienceToNextLevel = ch.experienceToNextLevel();

	Common::String msg;
	if (_experienceToNextLevel && ch._level._permanent < _maxLevel) {
		// Need more experience
		int nextLevel = ch._level._permanent + 1;
		msg = Common::String::format(Res.EXPERIENCE_FOR_LEVEL,
			ch._name.c_str(), _experienceToNextLevel, nextLevel);
	} else if (ch._level._permanent >= _maxLevel) {
		// At maximum level
		_experienceToNextLevel = 1;
		msg = Common::String::format(Res.LEARNED_ALL, ch._name.c_str());
	} else {
		// Eligble for level increase
		msg = Common::String::format(Res.ELIGIBLE_FOR_LEVEL,
			ch._name.c_str(), ch._level._permanent + 1);
	}

	return Common::String::format(Res.TRAINING_TEXT, msg.c_str(),
		XeenEngine::printMil(party._gold).c_str());
}

Character *TrainingLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	switch (_buttonValue) {
	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			_charIndex = _buttonValue;
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
		}
		break;

	case Common::KEYCODE_t:
		if (_experienceToNextLevel) {
			sound.stopSound();
			_drawFrameIndex = 0;

			Common::String name;
			if (c->_level._permanent >= _maxLevel) {
				name = _isDarkCc ? "gtlost.voc" : "trainin1.voc";
			} else {
				name = _isDarkCc ? "gtlost.voc" : "trainin0.voc";
			}

			sound.playSound(name);

		} else if (!c->noActions()) {
			if (party.subtract(CONS_GOLD, (c->_level._permanent * c->_level._permanent) * 10, WHERE_PARTY, WT_2)) {
				_drawFrameIndex = 0;
				sound.stopSound();
				sound.playSound(_isDarkCc ? "prtygd.voc" : "trainin2.voc", 1);

				c->_experience -=  c->nextExperienceLevel() -
					(c->getCurrentExperience() - c->_experience);
				c->_level._permanent++;

				if (!_charsTrained[_charIndex]) {
					party.addTime(1440);
					_charsTrained[_charIndex] = true;
				}

				party.resetTemps();
				c->_currentHp = c->getMaxHP();
				c->_currentSp = c->getMaxSP();
				intf.drawParty(true);
			}
		}
		break;

	default:
		break;
	}

	return c;
}

/*------------------------------------------------------------------------*/

ArenaLocation::ArenaLocation() : TownLocation(ARENA) {
	// TODO
}

/*------------------------------------------------------------------------*/

ReaperLocation::ReaperLocation() : TownLocation(REAPER) {
	// TODO
}

/*------------------------------------------------------------------------*/

GolemLocation::GolemLocation() : TownLocation(GOLEM) {
	// TODO
}

/*------------------------------------------------------------------------*/

DwarfLocation::DwarfLocation(bool isDwarf) : TownLocation(NO_ACTION) {
	_townMaxId = Res.TOWN_MAXES[_isDarkCc][isDwarf ? DWARF1 : DWARF2];
}

/*------------------------------------------------------------------------*/

SphinxLocation::SphinxLocation() : TownLocation(SPHINX) {
	// TODO
}

/*------------------------------------------------------------------------*/

PyramidLocation::PyramidLocation() : TownLocation(PYRAMID) {
}

int PyramidLocation::show() {
	EventsManager &events = *g_vm->_events;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Windows &windows = *g_vm->_windows;
	int mapId;
	Direction dir = DIR_NORTH;
	Common::Point pt;

	if (g_vm->getGameID() == GType_WorldOfXeen) {
		if (_isDarkCc) {
			if (party._mazeId == 52) {
				mapId = 49;
				pt = Common::Point(7, 14);
				dir = DIR_SOUTH;
			} else {
				mapId = 23;
				pt = Common::Point(8, 10);
			}
		} else {
			if (party._mazeId == 49) {
				mapId = 52;
				pt = Common::Point(2, 2);
			} else {
				mapId = 29;
				pt = Common::Point(25, 21);
			}
		}

		// Load the destination map and set position and direction
		map._loadDarkSide = !_isDarkCc;
		map.load(mapId);
		party._mazePosition = pt;
		party._mazeDirection = dir;
	} else {
		// Playing Clouds or Dark Side on it's own, so can't switch sides
		Window &win = windows[12];
		Common::String msg = Common::String::format(Res.MOONS_NOT_ALIGNED,
			_isDarkCc ? "Clouds" : "Darkside");
		win.open();
		win.writeString(msg);
		win.update();

		events.waitForPressAnimated();
		win.close();
	}

	return 0;
}

/*------------------------------------------------------------------------*/

Town::Town() : _location(nullptr) {
}

int Town::townAction(TownAction actionId) {
	// Create the desired location
	switch (actionId) {
	case BANK:
		_location = new BankLocation();
		break;
	case BLACKSMITH:
		_location = new BlacksmithLocation();
		break;
	case GUILD:
		_location = new GuildLocation();
		break;
	case TAVERN:
		_location = new TavernLocation();
		break;
	case TEMPLE:
		_location = new TempleLocation();
		break;
	case TRAINING:
		_location = new TrainingLocation();
		break;
	case ARENA:
		_location = new ArenaLocation();
		break;
	case REAPER:
		_location = new ReaperLocation();
		break;
	case GOLEM:
		_location = new GolemLocation();
		break;
	case DWARF1:
		_location = new DwarfLocation(true);
		break;
	case DWARF2:
		_location = new DwarfLocation(false);
		break;
	case SPHINX:
		_location = new SphinxLocation();
		break;
	case PYRAMID:
		_location = new PyramidLocation();
		break;
	default:
		return 0;
	}

	// Show the location
	int result = _location->show();
	delete _location;
	_location = nullptr;

	return result;
}

bool Town::isActive() const {
	return _location != nullptr;
}

void Town::drawAnim(bool flag) {
	if (_location)
		_location->drawAnim(flag);
}

/*------------------------------------------------------------------------*/

bool TownMessage::show(int portrait, const Common::String &name,
		const Common::String &text, int confirm) {
	TownMessage *dlg = new TownMessage();
	bool result = dlg->execute(portrait, name, text, confirm);
	delete dlg;

	return result;
}

bool TownMessage::execute(int portrait, const Common::String &name, const Common::String &text,
		int confirm) {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Resources &res = *g_vm->_resources;
	Windows &windows = *g_vm->_windows;
	Window &w = windows[11];

	_townMaxId = 4;
	_drawFrameIndex = 0;
	_townPos = Common::Point(23, 22);

	if (!confirm)
		loadButtons();

	_townSprites.resize(2);
	_townSprites[0].load(Common::String::format("face%02d.fac", portrait));
	_townSprites[1].load("frame.fac");

	if (!w._enabled)
		w.open();

	int result = -1;
	Common::String msgText = text;
	do {
		Common::String msg = Common::String::format("\r\v014\x03""c\t125%s\t000\v054%s",
			name.c_str(), msgText.c_str());

		// Count the number of words
		const char *msgEnd = w.writeString(msg);
		int wordCount = 0;

		for (const char *msgP = msg.c_str(); msgP != msgEnd && *msgP; ++msgP) {
			if (*msgP == ' ')
				++wordCount;
		}

		_drawCtr2 = wordCount * 2;	// Set timeout
		_townSprites[1].draw(0, 0, Common::Point(16, 16));
		_townSprites[0].draw(0, _drawFrameIndex, Common::Point(23, 22));
		w.update();

		if (!msgEnd && !confirm) {
			res._globalSprites.draw(0, 7, Common::Point(232, 74));
			res._globalSprites.draw(0, 0, Common::Point(235, 75));
			res._globalSprites.draw(0, 2, Common::Point(260, 75));
			windows[34].update();

			intf._face1State = map._headData[party._mazePosition.y][party._mazePosition.x]._left;
			intf._face2State = map._headData[party._mazePosition.y][party._mazePosition.x]._right;
		}

		if (confirm == 2) {
			intf._face1State = intf._face2State = 2;
			return false;
		}

		do {
			events.clearEvents();
			events.updateGameCounter();
			if (msgEnd)
				clearButtons();

			do {
				events.pollEventsAndWait();
				checkEvents(_vm);

				if (_vm->shouldQuit())
					return false;

				while (events.timeElapsed() >= 3) {
					drawAnim(false);
					events.updateGameCounter();
				}
			} while (!_buttonValue);

			if (msgEnd)
				// Another screen of text remaining
				break;

			if (confirm || _buttonValue == Common::KEYCODE_ESCAPE ||
					_buttonValue == Common::KEYCODE_n)
				result = 0;
			else if (_buttonValue == Common::KEYCODE_y)
				result = 1;
		} while (result == -1);

		if (msgEnd) {
			// Text remaining, so cut off already displayed page's
			msgText = Common::String(msgEnd);
			_drawCtr2 = wordCount;
			continue;
		}
	} while (result == -1);

	intf._face1State = intf._face2State = 2;
	if (!confirm)
		intf.mainIconsPrint();

	_townSprites[0].clear();
	_townSprites[1].clear();
	events.clearEvents();
	return result == 1;
}

void TownMessage::loadButtons() {
	_iconSprites.load("confirm.icn");

	addButton(Common::Rect(235, 75, 259, 95), Common::KEYCODE_y, &_iconSprites);
	addButton(Common::Rect(260, 75, 284, 95), Common::KEYCODE_n, &_iconSprites);
	addButton(Common::Rect(), Common::KEYCODE_ESCAPE);
}

} // End of namespace Xeen
