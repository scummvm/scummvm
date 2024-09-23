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

#include "mm/xeen/locations.h"
#include "mm/xeen/dialogs/dialogs_input.h"
#include "mm/xeen/dialogs/dialogs_items.h"
#include "mm/xeen/dialogs/dialogs_party.h"
#include "mm/xeen/dialogs/dialogs_query.h"
#include "mm/xeen/dialogs/dialogs_spells.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

#define WAIT(TIME) if (_subtitles.wait(TIME)) goto exit

namespace MM {
namespace Xeen {
namespace Locations {

BaseLocation::BaseLocation(LocationAction action) : ButtonContainer(g_vm),
		_locationActionId(action), _ccNum(g_vm->_files->_ccNum),
		_vocName("hello1.voc"), _exitToUi(false) {
	_townMaxId = (action >= SPHINX) ? 0 : Res.TOWN_MAXES[_ccNum][action];
	if (action < NO_ACTION) {
		_songName = Res.TOWN_ACTION_MUSIC[_ccNum][action];
		_townSprites.resize(Res.TOWN_ACTION_FILES[_ccNum][action]);
	}

	_animFrame = 0;
	_drawFrameIndex = 0;
	_farewellTime = 0;
	_drawCtr1 = _drawCtr2 = 0;
	_animPos = Common::Point(8, 8);
}

BaseLocation::~BaseLocation() {
	Interface &intf = *g_vm->_interface;

	for (uint idx = 0; idx < _townSprites.size(); ++idx)
		_townSprites[idx].clear();
	intf.mainIconsPrint();
	intf.unhighlightChar();
}

int BaseLocation::show() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	// Play the appropriate music
	sound.stopSound();
	sound.playSong(_songName, 223);

	// Load the needed sprite sets for the location
	for (uint idx = 0; idx < _townSprites.size(); ++idx) {
		Common::Path shapesName(Common::String::format("%s%d.twn",
			Res.TOWN_ACTION_SHAPES[_locationActionId], idx + 1));
		_townSprites[idx].load(shapesName);
	}

	Character *charP = &party._activeParty[0];

	// Draw the background and the text window
	drawBackground();
	drawWindow();
	drawAnim(true);

	// Play the welcome speech
	sound.playVoice(_vocName);

	do {
		wait();
		charP = doOptions(charP);
		if (_vm->shouldExit() || _exitToUi)
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
		//_vm->_saves->saveChars();
		result = 2;
	}

	return result;
}

void BaseLocation::drawBackground() {
	Interface &intf = *g_vm->_interface;

	intf._face1UIFrame = intf._face2UIFrame = 0;
	intf._dangerSenseUIFrame = 0;
	intf._spotDoorsUIFrame = 0;
	intf._levitateUIFrame = 0;
	_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _animPos);
}

void BaseLocation::drawWindow() {
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

void BaseLocation::drawAnim(bool flag) {
	Interface &intf = *g_vm->_interface;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	// TODO: Figure out a clean way to split method into individual location classes
	if (_locationActionId == BLACKSMITH) {
		if (sound.isSoundPlaying()) {
			if (_ccNum) {
				_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _animPos);
				_townSprites[2].draw(0, _vm->getRandomNumber(11) == 1 ? 9 : 10,
					Common::Point(34, 33));
				_townSprites[2].draw(0, _vm->getRandomNumber(5) + 3,
					Common::Point(34, 54));
			}
		} else {
			_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _animPos);
			if (_ccNum) {
				_townSprites[2].draw(0, _vm->getRandomNumber(11) == 1 ? 9 : 10,
					Common::Point(34, 33));
			}
		}
	} else if (!_ccNum || _locationActionId != TRAINING) {
		if (!_townSprites[_drawFrameIndex / 8].empty())
			_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _animPos);
	}

	switch (_locationActionId) {
	case BANK:
		if (sound.isSoundPlaying() || (_ccNum && _animFrame)) {
			if (_ccNum) {
				if (sound.isSoundPlaying() || _animFrame == 1) {
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
		if (!sound.isSoundPlaying()) {
			if (_ccNum) {
				if (_animFrame) {
					_animFrame ^= 1;
					_townSprites[6].draw(0, _animFrame, Common::Point(8, 106));
				} else {
					_townSprites[6].draw(0, _vm->getRandomNumber(3), Common::Point(161, 48));
				}
			}
		}
		break;

	case TAVERN:
		if (sound.isSoundPlaying() && _ccNum) {
			_townSprites[4].draw(0, _vm->getRandomNumber(7), Common::Point(153, 49));
		}
		break;

	case TEMPLE:
		if (sound.isSoundPlaying()) {
			_townSprites[3].draw(0, _vm->getRandomNumber(2, 4), Common::Point(8, 8));

		}
		break;

	case TRAINING:
		if (sound.isSoundPlaying()) {
			if (_ccNum) {
				_townSprites[_drawFrameIndex / 8].draw(0, _drawFrameIndex % 8, _animPos);
			}
		} else {
			if (_ccNum) {
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
		int newFrame = _vm->getRandomNumber(3);

		if (!_drawCtr1 || !_drawCtr2) {
			if (--_drawCtr2 <= 0) {
				newFrame = 0;
				_drawCtr2 = 0;
			}
		}
		_drawFrameIndex = newFrame;
	} else {
		_drawFrameIndex = (_drawFrameIndex + 1) % _townMaxId;
	}

	if (_ccNum) {
		if (_locationActionId == BLACKSMITH && (_drawFrameIndex == 4 || _drawFrameIndex == 13))
			sound.playFX(45);

		if (_locationActionId == TRAINING && _drawFrameIndex == 23) {
			sound.playSound("spit1.voc");
		}
	} else {
		if (_townMaxId == 32 && _drawFrameIndex == 0)
			_drawFrameIndex = 17;
		if (_townMaxId == 26 && _drawFrameIndex == 0)
			_drawFrameIndex = 20;
		if (_locationActionId == BLACKSMITH && (_drawFrameIndex == 3 || _drawFrameIndex == 9))
			sound.playFX(45);
	}

	windows[3].update();

	if (_locationActionId == BANK)
		_animFrame = 2;
}

int BaseLocation::wait() {
	EventsManager &events = *g_vm->_events;
	Windows &windows = *g_vm->_windows;

	_buttonValue = 0;
	while (!_vm->shouldExit() && !_buttonValue) {
		events.updateGameCounter();
		while (!_vm->shouldExit() && !_buttonValue && events.timeElapsed() < 3) {
			events.pollEventsAndWait();
			checkEvents(_vm);
		}
		if (!_buttonValue)
			drawAnim(!windows[11]._enabled);
	}

	return _buttonValue;
}

/*------------------------------------------------------------------------*/

BankLocation::BankLocation() : BaseLocation(BANK) {
	_icons1.load("bank.icn");
	_icons2.load("bank2.icn");
	addButton(Common::Rect(234, 108, 259, 128), Res.KeyConstants.Locations.KEY_DEP, &_icons1);
	addButton(Common::Rect(261, 108, 285, 128), Res.KeyConstants.Locations.KEY_WITH, &_icons1);
	addButton(Common::Rect(288, 108, 312, 128), Common::KEYCODE_ESCAPE, &_icons1);
	_animFrame = 1;

	_vocName = _ccNum ? "bank1.voc" : "banker.voc";
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
	if (_ccNum) {
		_townSprites[4].draw(0, _vm->getRandomNumber(13, 18),
			Common::Point(8, 30));
	}
}

Character *BankLocation::doOptions(Character *c) {
	if (_buttonValue == Res.KeyConstants.Locations.KEY_DEP)
		_buttonValue = (int)WHERE_PARTY;
	else if (_buttonValue == Res.KeyConstants.Locations.KEY_WITH)
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
	Window &w = windows[35];
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

	if (g_vm->getLanguage() == Common::RU_RUS) {
		// In RU version sprites in wrong order
		_buttons[1]._value = Res.KeyConstants.Locations.KEY_GOLD;
		_buttons[0]._value = Res.KeyConstants.Locations.KEY_GEMS;
	} else {
		_buttons[0]._value = Res.KeyConstants.Locations.KEY_GOLD;
		_buttons[1]._value = Res.KeyConstants.Locations.KEY_GEMS;
	}
	_buttons[2]._value = Common::KEYCODE_ESCAPE;

	Common::String msg = Common::String::format(Res.GOLD_GEMS,
		Res.DEPOSIT_WITHDRAWL[whereId],
		XeenEngine::printMil(gold).c_str(),
		XeenEngine::printMil(gems).c_str());

	w.open();
	w.writeString(msg);
	drawButtons(&w);
	w.update();

	sound.stopSound();
	File voc("coina.voc");
	ConsumableType consType = CONS_GOLD;

	do {
		wait();
		if (_buttonValue == Res.KeyConstants.Locations.KEY_GOLD) {
			consType = CONS_GOLD;
		} else if (_buttonValue == Res.KeyConstants.Locations.KEY_GEMS) {
			consType = CONS_GEMS;
		} else if (_buttonValue == Common::KEYCODE_ESCAPE) {
			break;
		} else {
			continue;
		}

		if ((whereId == WHERE_BANK && !party._bankGems && consType == CONS_GEMS) ||
			(whereId == WHERE_BANK && !party._bankGold && consType == CONS_GOLD) ||
			(whereId == WHERE_PARTY && !party._gems && consType == CONS_GEMS) ||
			(whereId == WHERE_PARTY && !party._gold && consType == CONS_GOLD)) {
			party.notEnough(consType, whereId, WHERE_BANK, WT_LOC_WAIT);
		} else {
			w.writeString(Res.AMOUNT);
			int amount = NumericInput::show(_vm, 35, 10, 77);

			if (amount) {
				if (consType == CONS_GEMS) {
					if (party.subtract(CONS_GEMS, amount, whereId, WT_LOC_WAIT)) {
						if (whereId == WHERE_BANK) {
							party._gems += amount;
						} else {
							party._bankGems += amount;
						}
					}
				} else {
					if (party.subtract(CONS_GOLD, amount, whereId, WT_LOC_WAIT)) {
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
			} else {
				gold = party._gold;
				gems = party._gems;
			}

			sound.playSound(voc);
			msg = Common::String::format(Res.GOLD_GEMS_2, Res.DEPOSIT_WITHDRAWL[whereId],
				XeenEngine::printMil(gold).c_str(), XeenEngine::printMil(gems).c_str());
			w.writeString(msg);
			w.update();
		}
	} while (!g_vm->shouldExit());

	for (uint idx = 0; idx < _buttons.size(); ++idx)
		_buttons[idx]._sprites = &_icons1;
	_buttons[0]._value = Res.KeyConstants.Locations.KEY_DEP;
	_buttons[1]._value = Res.KeyConstants.Locations.KEY_WITH;
	_buttons[2]._value = Common::KEYCODE_ESCAPE;

	w.close();
	clearEvents();
}

/*------------------------------------------------------------------------*/

BlacksmithLocation::BlacksmithLocation() : BaseLocation(BLACKSMITH) {
	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), 0);
	addButton(Common::Rect(234, 64, 308, 72), Res.KeyConstants.Locations.KEY_BROWSE);
	addButton(Common::Rect(234, 74, 308, 82), 0);
	addButton(Common::Rect(234, 84, 308, 92), 0);

	_vocName = _ccNum ? "see2.voc" : "whaddayo.voc";
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
	} else if (_buttonValue == Res.KeyConstants.Locations.KEY_BROWSE) {
		c = ItemsDialog::show(_vm, c, ITEMMODE_BUY);
		_buttonValue = 0;
	}

	return c;
}

void BlacksmithLocation::farewell() {
	Sound &sound = *g_vm->_sound;

	if (_ccNum) {
		sound.stopSound();
		sound.playVoice("come1.voc", 1);

		while (sound.isSoundPlaying() && !g_vm->shouldExit())
			g_vm->_events->pollEventsAndWait();
	}
}

/*------------------------------------------------------------------------*/

GuildLocation::GuildLocation() : BaseLocation(GUILD) {
	loadStrings("spldesc.bin");
	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), 0);
	addButton(Common::Rect(234, 64, 308, 72), Res.KeyConstants.Locations.KEY_BUY_SPELLS);
	addButton(Common::Rect(234, 74, 308, 82), Res.KeyConstants.Locations.KEY_SPELL_INFO);
	addButton(Common::Rect(234, 84, 308, 92), 0);
	g_vm->_mode = MODE_INTERACTIVE7;

	_vocName = _ccNum ? "parrot1.voc" : "guild10.voc";
}

Common::String GuildLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;

	Common::String desc = !ch.guildMember() ? Res.GUILD_NOT_MEMBER_TEXT :
		Common::String::format(Res.GUILD_TEXT, ch._name.c_str());
	return Common::String::format(Res.GUILD_OPTIONS, desc.c_str(),
		g_vm->printMil(party._gold).c_str());
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
				sound.playSound(_ccNum ? "skull1.voc" : "guild11.voc", 1);
			}
		}
	} else if (_buttonValue == Res.KeyConstants.Locations.KEY_SPELL_INFO) {
		if (c->guildMember())
			c = SpellsDialog::show(_vm, this, c, SPELLS_DIALOG_INFO);
		_buttonValue = 0;
	} else if (_buttonValue == Res.KeyConstants.Locations.KEY_BUY_SPELLS) {
		if (!c->noActions()) {
			if (c->guildMember())
				c = SpellsDialog::show(_vm, this, c, SPELLS_DIALOG_BUY);
			_buttonValue = 0;
		}
	}

	return c;
}

/*------------------------------------------------------------------------*/

TavernLocation::TavernLocation() : BaseLocation(TAVERN) {
	_v21 = 0;
	_v22 = 0;
	_v23 = 0;
	_v24 = 0;

	loadStrings("tavern.bin");
	_icons1.load("tavern.icn");
	addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(242, 108, 266, 128), Res.KeyConstants.Locations.KEY_SIGN_IN, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), Res.KeyConstants.Locations.KEY_DRINK);
	addButton(Common::Rect(234, 64, 308, 72), Res.KeyConstants.Locations.KEY_FOOD);
	addButton(Common::Rect(234, 74, 308, 82), Res.KeyConstants.Locations.KEY_TIP);
	addButton(Common::Rect(234, 84, 308, 92), Res.KeyConstants.Locations.KEY_RUMORS);
	g_vm->_mode = MODE_INTERACTIVE7;

	_vocName = _ccNum ? "hello1.voc" : "hello.voc";
}

Common::String TavernLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	return Common::String::format(Res.TAVERN_TEXT, ch._name.c_str(),
		Res.FOOD_AND_DRINK, XeenEngine::printMil(party._gold).c_str());
}

Character *TavernLocation::doOptions(Character *c) {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;
	int idx = 0;

	if (
		Common::KEYCODE_F1 == _buttonValue ||
		Common::KEYCODE_F2 == _buttonValue ||
		Common::KEYCODE_F3 == _buttonValue ||
		Common::KEYCODE_F4 == _buttonValue ||
		Common::KEYCODE_F5 == _buttonValue ||
		Common::KEYCODE_F6 == _buttonValue) {
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
			_v21 = 0;
		}
	} else if (Res.KeyConstants.Locations.KEY_DRINK == _buttonValue) {
		// Drink
		if (!c->noActions()) {
			if (party.subtract(CONS_GOLD, 1, WHERE_PARTY, WT_LOC_WAIT)) {
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
	} else if (Res.KeyConstants.Locations.KEY_FOOD == _buttonValue) {
		// Food
		if (party._mazeId == (_ccNum ? 29 : 28)) {
			_v22 = party._activeParty.size() * 15;
			_v23 = 10;
			idx = 0;
		} else if (_ccNum && party._mazeId == 31) {
			_v22 = party._activeParty.size() * 60;
			_v23 = 100;
			idx = 1;
		} else if (!_ccNum && party._mazeId == 30) {
			_v22 = party._activeParty.size() * 50;
			_v23 = 50;
			idx = 1;
		} else if (_ccNum) {
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

		Common::String msg = _textStrings[(_ccNum ? 60 : 75) + idx];
		windows[10].close();
		windows[12].open();
		windows[12].writeString(msg);
		windows[12].update();

		if (YesNo::show(_vm, false, true)) {
			if (party._food >= _v22) {
				ErrorScroll::show(_vm, Res.FOOD_PACKS_FULL, WT_LOC_WAIT);
			} else if (party.subtract(CONS_GOLD, _v23, WHERE_PARTY, WT_LOC_WAIT)) {
				party._food = _v22;
				sound.stopSound();
				sound.playSound(_ccNum ? "thanks2.voc" : "thankyou.voc", 1);
			}
		}

		windows[12].close();
		windows[10].open();
		_buttonValue = 0;
	} else if (Res.KeyConstants.Locations.KEY_RUMORS == _buttonValue) {
		// Rumors
		if (party._mazeId == (_ccNum ? 29 : 28)) {
			idx = 0;
		} else if (party._mazeId == (_ccNum ? 31 : 30)) {
			idx = 10;
		} else if (_ccNum || party._mazeId == 49) {
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
	} else if (Res.KeyConstants.Locations.KEY_SIGN_IN == _buttonValue) {
		// Sign In
		// Set location and position for afterwards
		if (g_vm->getGameID() == GType_Swords) {
			switch (party._mazeId) {
			case 49:
				party._mazePosition = Common::Point(9, 2);
				party._mazeDirection = DIR_WEST;
				break;
			case 53:
				party._mazePosition = Common::Point(13, 1);
				party._mazeDirection = DIR_EAST;
				break;
			case 92:
				party._mazePosition = Common::Point(10, 1);
				party._mazeDirection = DIR_EAST;
				break;
			case 63:
				party._mazePosition = Common::Point(13, 1);
				party._mazeDirection = DIR_SOUTH;
				break;
			default:
				break;
			}
		} else {
			idx = _ccNum ? (party._mazeId - 29) >> 1 : party._mazeId - 28;
			assert(idx >= 0);
			party._mazePosition.x = Res.TAVERN_EXIT_LIST[_ccNum][_locationActionId][idx][0];
			party._mazePosition.y = Res.TAVERN_EXIT_LIST[_ccNum][_locationActionId][idx][1];

			if (!_ccNum || party._mazeId == 29)
				party._mazeDirection = DIR_WEST;
			else if (party._mazeId == 31)
				party._mazeDirection = DIR_EAST;
			else
				party._mazeDirection = DIR_SOUTH;
		}

		party._priorMazeId = party._mazeId;
		for (idx = 0; idx < (int)party._activeParty.size(); ++idx) {
			party._activeParty[idx]._savedMazeId = party._mazeId;
			party._activeParty[idx]._xeenSide = map._loadCcNum;
		}

		g_vm->_mode = MODE_INTERACTIVE7;
		party.addTime(1440);
		int partyId = party._mazeId;
		party._mazeId = 0;

		// Say farewell
		farewell();
		while (sound.isSoundPlaying())
			events.wait(1);

		// Animate closing a scroll
		doScroll(true, false);
		sound.stopAllAudio();

		// Show the party dialog
		PartyDialog::show(g_vm);

		if (party._mazeId != partyId)
			map.load(party._mazeId);
		_exitToUi = true;
	} else if (Res.KeyConstants.Locations.KEY_TIP == _buttonValue) {
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
				} else if (party.subtract(CONS_GOLD, 1, WHERE_PARTY, WT_LOC_WAIT)) {
					sound.stopSound();
					sound.playSound(_ccNum ? "thanks2.voc" : "thankyou.voc", 1);

					if (party._mazeId == (_ccNum ? 29 : 28)) {
						_v24 = 30;
					} else if (_ccNum && party._mazeId == 31) {
						_v24 = 40;
					} else if (!_ccNum && party._mazeId == 45) {
						_v24 = 45;
					} else if (!_ccNum && party._mazeId == 49) {
						_v24 = 60;
					} else if (_ccNum) {
						_v24 = 50;
					}

					Common::String msg = _textStrings[map.mazeData()._tavernTips + _v24];
					map.mazeData()._tavernTips = (map.mazeData()._tavernTips + 1) /
						(_ccNum ? 10 : 15);

					Window &w = windows[12];
					w.open();
					w.writeString(Common::String::format("\x03""c\x0B""012%s", msg.c_str()));
					w.update();
					wait();
					w.close();
				}
			}
		}
	}

	return c;
}

void TavernLocation::farewell() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	sound.stopSound();
	sound.playVoice(_ccNum ? "gdluck1.voc" : "goodbye.voc");
	while (sound.isSoundPlaying() && !g_vm->shouldExit())
		g_vm->_events->pollEventsAndWait();

	map.mazeData()._mazeNumber = party._mazeId;
}

/*------------------------------------------------------------------------*/

TempleLocation::TempleLocation() : BaseLocation(TEMPLE) {
	_currentCharLevel = 0;
	_donation = 0;
	_healCost = 0;
	_uncurseCost = 0;
	_dayOfWeek = 0;
	_v10 = _v11 = 0;
	_v12 = _v13 = 0;
	_v14 = 0;
	_blessed = false;
	_v5 = _v6 = 0;

	_icons1.load("esc.icn");
	addButton(Common::Rect(261, 108, 285, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(234, 54, 308, 62), Res.KeyConstants.Locations.KEY_HEAL);
	addButton(Common::Rect(234, 64, 308, 72), Res.KeyConstants.Locations.KEY_DONATION);
	addButton(Common::Rect(234, 74, 308, 82), Res.KeyConstants.Locations.KEY_UNCURSE);
	addButton(Common::Rect(234, 84, 308, 92), 0);

	_vocName = _ccNum ? "help2.voc" : "maywe2.voc";
}

Common::String TempleLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	_donation = 0;
	_uncurseCost = 0;
	_healCost = 0;
	_v5 = _v6 = 0;

	if (party._mazeId == (_ccNum ? 29 : 28)) {
		_v10 = _v11 = _v12 = _v13 = 0;
		_v14 = 10;
	} else if (party._mazeId == (_ccNum ? 31 : 30)) {
		_v13 = 10;
		_v12 = 50;
		_v11 = 500;
		_v10 = 100;
		_v14 = 25;
	} else if (party._mazeId == (_ccNum ? 37 : 73)) {
		_v13 = 20;
		_v12 = 100;
		_v11 = 1000;
		_v10 = 200;
		_v14 = 50;
	} else if (_ccNum || party._mazeId == 49) {
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

	bool isCursed = ch._items.hasCursedItems();
	if (isCursed || ch._conditions[CURSED])
		_uncurseCost = (_currentCharLevel * 20) + _v10;

	_donation = _blessed ? 0 : _v14;
	_healCost += _v6 + _v5;

	return Common::String::format(Res.TEMPLE_TEXT, ch._name.c_str(),
		_healCost, _donation, XeenEngine::printK(_uncurseCost).c_str(),
		XeenEngine::printMil(party._gold).c_str());
}

Character *TempleLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	if (Common::KEYCODE_F1 == _buttonValue ||
		Common::KEYCODE_F2 == _buttonValue ||
		Common::KEYCODE_F3 == _buttonValue ||
		Common::KEYCODE_F4 == _buttonValue ||
		Common::KEYCODE_F5 == _buttonValue ||
		Common::KEYCODE_F6 == _buttonValue) {
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
			_dayOfWeek = 0;
		}
	} else if (Res.KeyConstants.Locations.KEY_DONATION == _buttonValue) {
		if (_donation && party.subtract(CONS_GOLD, _donation, WHERE_PARTY, WT_LOC_WAIT)) {
			sound.stopSound();
			sound.playSound("coina.voc", 1);
			_dayOfWeek = (_dayOfWeek + 1) % 10;

			if (_dayOfWeek == (party._day % 10)) {
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
				_blessed = true;
				_donation = 0;
			}
		}
	} else if (Res.KeyConstants.Locations.KEY_HEAL == _buttonValue) {
		if (_healCost && party.subtract(CONS_GOLD, _healCost, WHERE_PARTY, WT_LOC_WAIT)) {
			c->_magicResistance._temporary = 0;
			c->_energyResistance._temporary = 0;
			c->_poisonResistance._temporary = 0;
			c->_electricityResistance._temporary = 0;
			c->_coldResistance._temporary = 0;
			c->_fireResistance._temporary = 0;
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
	} else if (Res.KeyConstants.Locations.KEY_UNCURSE == _buttonValue) {
		if (_uncurseCost && party.subtract(CONS_GOLD, _uncurseCost, WHERE_PARTY, WT_LOC_WAIT)) {
			c->_items.curseUncurse(false);
			c->_conditions[CURSED] = 0;
			_farewellTime = 1440;
			intf.drawParty(true);
			sound.stopSound();
			sound.playSound("ahh.voc", 1);
		}
	}

	return c;
}

/*------------------------------------------------------------------------*/

TrainingLocation::TrainingLocation() : BaseLocation(TRAINING) {
	Common::fill(&_charsTrained[0], &_charsTrained[6], 0);
	_experienceToNextLevel = 0;
	_charIndex = 0;

	_icons1.load("train.icn");
	addButton(Common::Rect(281, 108, 305, 128), Common::KEYCODE_ESCAPE, &_icons1);
	addButton(Common::Rect(242, 108, 266, 128), Res.KeyConstants.Locations.KEY_TRAIN, &_icons1);

	_vocName = _ccNum ? "youtrn1.voc" : "training.voc";
}

int TrainingLocation::maxLevel() const {
	Party &party = *g_vm->_party;
	if (_ccNum) {
		switch (party._mazeId) {
		case 29:
			// Castleview
			return 30;
		case 31:
			// Sandcaster
			return 50;
		case 37:
			// Olympus
			return 200;
		default:
			// Kalindra's Castle
			return 100;
		}
	} else {
		switch (party._mazeId) {
		case 28:
			// Vertigo
			return 10;
		case 30:
			// Rivercity
			return 15;
		default:
			// Newcastle
			return 20;
		}
	}
}

Common::String TrainingLocation::createLocationText(Character &ch) {
	Party &party = *g_vm->_party;
	int maxLevelAtLocation = maxLevel();
	_experienceToNextLevel = ch.experienceToNextLevel();

	Common::String msg;
	if (_experienceToNextLevel && ch._level._permanent < maxLevelAtLocation) {
		// Need more experience
		int nextLevel = ch._level._permanent + 1;
		msg = Common::String::format(Res.EXPERIENCE_FOR_LEVEL,
			ch._name.c_str(), _experienceToNextLevel, nextLevel);
	} else if (ch._level._permanent >= maxLevelAtLocation) {
		// At maximum level
		_experienceToNextLevel = 1;
		msg = Common::String::format(Res.TRAINING_LEARNED_ALL, ch._name.c_str());
	} else {
		// Eligble for level increase
		uint cost = ch._level._permanent * ch._level._permanent * 10;
		msg = Common::String::format(Res.ELIGIBLE_FOR_LEVEL,
			ch._name.c_str(), ch._level._permanent + 1, cost);
	}

	return Common::String::format(Res.TRAINING_TEXT, msg.c_str(),
		XeenEngine::printMil(party._gold).c_str());
}

Character *TrainingLocation::doOptions(Character *c) {
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Sound &sound = *g_vm->_sound;

	if (Common::KEYCODE_F1 == _buttonValue ||
		Common::KEYCODE_F2 == _buttonValue ||
		Common::KEYCODE_F3 == _buttonValue ||
		Common::KEYCODE_F4 == _buttonValue ||
		Common::KEYCODE_F5 == _buttonValue ||
		Common::KEYCODE_F6 == _buttonValue) {
		// Switch character
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			_charIndex = _buttonValue;
			c = &party._activeParty[_buttonValue];
			intf.highlightChar(_buttonValue);
		}
	} else if (Res.KeyConstants.Locations.KEY_TRAIN == _buttonValue) {
		if (_experienceToNextLevel) {
			sound.stopSound();
			_drawFrameIndex = 0;

			Common::Path name;
			if (c->_level._permanent >= maxLevel()) {
				name = _ccNum ? "gtlost.voc" : "trainin1.voc";
			} else {
				name = _ccNum ? "gtlost.voc" : "trainin0.voc";
			}

			sound.playSound(name);

		} else if (!c->noActions()) {
			if (party.subtract(CONS_GOLD, (c->_level._permanent * c->_level._permanent) * 10, WHERE_PARTY, WT_LOC_WAIT)) {
				_drawFrameIndex = 0;
				sound.stopSound();
				sound.playSound(_ccNum ? "prtygd.voc" : "trainin2.voc", 1);

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
	}

	return c;
}

/*------------------------------------------------------------------------*/

ArenaLocation::ArenaLocation() : BaseLocation(ARENA) {
}

int ArenaLocation::show() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	Windows &windows = *g_vm->_windows;
	int level, howMany;
	bool check;
	const char *SUFFIXES[10] = { "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th" };

	Common::Array<MazeMonster> &monsters = map._mobData._monsters;

	if (monsters.size() > 0) {
		for (uint idx = 0; idx < monsters.size(); ++idx) {
			MazeMonster &monster = monsters[idx];
			if (monster._position.x != 0x80 && monster._position.y != 0x80) {
				LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER,
					map._events._text[4], 300);
				goto exit;
			}
		}

		// Give each character the award
		for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
			party._activeParty[idx]._awards[WARZONE_AWARD]++;
		}

		Common::String format = map._events._text[3];
		int count = party._activeParty[0]._awards[WARZONE_AWARD];
		int suffixNum = (count < 10) ? count : 0;
		Common::String msg = Common::String::format(format.c_str(), count, SUFFIXES[suffixNum]);

		LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, msg, 1);

		map.load(28);
		goto exit;
	}

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		if (party._activeParty[idx]._awards[WARZONE_AWARD] >= 99) {
			LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, Res.WARZONE_MAXED, 1);
			map.load(28);
			goto exit;
		}
	}

	check = LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, map._events._text[0].c_str(), 0);
	if (!check) {
		LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER,
			map._events._text[1].c_str(), 300);
		windows.closeAll();
		map.load(6);
		party._mazePosition = Common::Point(12, 4);
		party._mazeDirection = DIR_WEST;
		return 0;
	}

	do {
		LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, Res.WARZONE_LEVEL, 2);
		level = NumericInput::show(g_vm, 11, 2, 200);
	} while (!g_vm->shouldExit() && level > 10);
	if (level == 0)
		goto exit;

	do {
		LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, Res.WARZONE_HOW_MANY, 2);
		howMany = NumericInput::show(g_vm, 11, 2, 200);
	} while (!g_vm->shouldExit() && howMany > 20);
	if (howMany == 0)
		goto exit;

	LocationMessage::showMessage(27, Res.WARZONE_BATTLE_MASTER, map._events._text[2], 1);

	// Clear monsters array
	party._mazeDirection = DIR_EAST;
	map._mobData.clearMonsterSprites();
	monsters.clear();
	monsters.resize(howMany);

	for (uint idx = 0; idx < monsters.size(); ++idx) {
		MazeMonster &mon = monsters[idx];
		mon._spriteId = g_vm->getRandomNumber(1, 7) + (level - 1) * 7;
		if (mon._spriteId > 67)
			mon._spriteId -= 3;
		if (mon._spriteId == 59)
			mon._spriteId = 60;
		if (mon._spriteId == 28)
			mon._spriteId = 29;

		// Set up normal and attack sprites
		map._mobData.addMonsterSprites(mon);

		mon._position.x = g_vm->getRandomNumber(3, 11);
		mon._position.y = g_vm->getRandomNumber(2, 10);
		if ((mon._position.x == 5 || mon._position.x == 10) &&
			(mon._position.y == 8 || mon._position.y == 4))
			mon._position.y = 5;

		mon._id = g_vm->getRandomNumber(4);
		const MonsterStruct &data = map._monsterData[mon._spriteId];
		mon._hp = data._hp;
		mon._frame = g_vm->getRandomNumber(7);
		mon._effect1 = mon._effect2 = data._animationEffect;
		if (data._animationEffect)
			mon._effect3 = g_vm->getRandomNumber(7);
		mon._isAttacking = true;
	}
exit:
	party._mazeDirection = DIR_EAST;
	party.moveToRunLocation();
	windows.closeAll();
	return 0;
}

/*------------------------------------------------------------------------*/

CutsceneLocation::CutsceneLocation(LocationAction action) : BaseLocation(action), _keyFound(false) {
	Party &party = *g_vm->_party;
	_mazeId = party._mazeId;
	_mazePos = party._mazePosition;
	_mazeDir = party._mazeDirection;
}

void CutsceneLocation::setNewLocation() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;
	map.load(_mazeId);
	party._mazePosition = _mazePos;
	party._mazeDirection = _mazeDir;
}

/*------------------------------------------------------------------------*/

static const int16 REAPER_X1[2][14] = {
	{ 0, -10, -20, -30, -40, -49, -49, -49, -49, -49, -49, -49, -49, -49 },
	{ 0, 2, 6, 8, 11, 14, 17, 21, 27, 35, 43, 51, 60, 67 }
};
static const int16 REAPER_Y1[2][14] = {
	{ 0, 12, 25, 37, 45, 50, 56, 61, 67, 72, 78, 83, 89, 94 },
	{ 0, 6, 12, 17, 23, 29, 36, 42, 49, 54, 61, 68, 73, 77 }
};
static const int16 REAPER_X2[14] = {
	160, 152, 146, 138, 131, 124, 117, 111, 107, 105, 103, 101, 100, 97
};
static const int16 REAPER_X3[14] = {
	0, -3, -4, -7, -9, -11, -13, -14, -13, -10, -7, -4, 0, -1
};

ReaperCutscene::ReaperCutscene() : CutsceneLocation(REAPER) {
}

int ReaperCutscene::show() {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Party &party = *g_vm->_party;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	SpriteResource sprites1(_ccNum ? "tower1.zom" : "tower.vga", _ccNum);
	SpriteResource sprites2(_ccNum ? "tower2.zom" : "freap.vga", _ccNum);

	Graphics::ManagedSurface savedBg;
	savedBg.copyFrom(screen);
	getNewLocation();

	for (int idx = 13; idx >= 0; --idx) {
		events.updateGameCounter();
		sprites1.draw(0, 0, Common::Point(REAPER_X1[_ccNum][idx], REAPER_Y1[_ccNum][idx]), 0, idx);
		if (_ccNum) {
			sprites1.draw(0, 1, Common::Point(REAPER_X2[idx], REAPER_Y1[1][idx]), 0, idx);
			sprites1.draw(0, party._isNight ? 3 : 2, Common::Point(REAPER_X3[idx], REAPER_Y1[1][idx]), 0, idx);
		}

		WAIT(1);
	}

	if (_ccNum) {
		for (int idx = -200; idx < 0; idx += 16) {
			events.updateGameCounter();
			sprites1.draw(0, 0, Common::Point(0, 0));
			sprites1.draw(0, 1, Common::Point(160, 0));
			sprites1.draw(0, 2, Common::Point(0, 0));
			sprites2.draw(0, 0, Common::Point(idx, 0), SPRFLAG_800);
			sprites2.draw(0, 5, Common::Point(160 + idx, 0), SPRFLAG_800);

			WAIT(1);
		}
	} else {
		for (int idx = 200; idx >= 0; idx -= 16) {
			events.updateGameCounter();
			sprites1.draw(0, 0, Common::Point(0, 0));
			sprites2.draw(0, 0, Common::Point(idx, 0), SPRFLAG_800);

			WAIT(1);
		}
	}

	sound.setMusicPercent(38);
	sprites1.draw(0, 0, Common::Point(0, 0));
	if (_ccNum) {
		sprites1.draw(0, 1, Common::Point(160, 0));
		sprites1.draw(0, party._isNight ? 3 : 2);
	}

	if (!_ccNum) {
		_subtitles.setLine(_keyFound ? 5 : 6);
		sound.playVoice(_keyFound ? "reaper12.voc" : "reaper14.voc");
	} else if (_keyFound) {
		_subtitles.setLine(2);
		sound.playVoice("howdid1.voc");
	}

	do {
		events.updateGameCounter();
		int frame = g_vm->getRandomNumber(4);
		if (_ccNum) {
			sprites2.draw(0, frame);
			sprites2.draw(0, frame + 5, Common::Point(160, 0));
		} else {
			sprites2.draw(0, 0);
			sprites2.draw(0, frame);
		}

		_subtitles.show();

		WAIT(1);
	} while (sound.isSoundPlaying());

	sprites2.draw(0, 0, Common::Point(0, 0));
	if (_ccNum)
		sprites2.draw(0, 5, Common::Point(160, 0));
	windows[0].update();
	WAIT(7);

	if (_keyFound) {
		sound.playVoice(_ccNum ? "goin1.voc" : "reaper13.voc");
	} else {
		if (_ccNum)
			_subtitles.setLine(3);
		sound.playVoice(_ccNum ? "needkey1.voc" : "reaper15.voc");
	}

	do {
		events.updateGameCounter();
		int frame = g_vm->getRandomNumber(4);
		if (_ccNum) {
			sprites2.draw(0, frame, Common::Point(0, 0));
			sprites2.draw(0, frame + 5, Common::Point(160, 0));
		} else {
			sprites2.draw(0, 0);
			sprites2.draw(0, frame);
		}

		WAIT(1);
	} while (_subtitles.lineActive());

	sprites2.draw(0, 0, Common::Point(0, 0));
	if (_ccNum)
		sprites2.draw(0, 5, Common::Point(160, 0));
	windows[0].update();
	WAIT(1);

	if (_keyFound) {
		for (int idx = 0; idx < 14; ++idx) {
			events.updateGameCounter();
			screen.blitFrom(savedBg);
			sprites1.draw(0, 0, Common::Point(REAPER_X1[_ccNum][idx], REAPER_Y1[_ccNum][idx]), 0, idx);

			if (_ccNum) {
				sprites1.draw(0, 1, Common::Point(REAPER_X2[idx], REAPER_Y1[1][idx]), 0, idx);
				sprites1.draw(0, party._isNight ? 3 : 2, Common::Point(REAPER_X3[idx], REAPER_Y1[1][idx]), 0, idx);
			}

			windows[0].update();
			WAIT(1);
		}

		screen.blitFrom(savedBg);
		windows[0].update();
	}
exit:
	screen.blitFrom(savedBg);
	windows[0].update();

	setNewLocation();

	// Restore game screen
	sound.stopSound();
	sound.setMusicPercent(75);

	screen.loadBackground("back.raw");
	intf.drawParty(false);
	intf.draw3d(false, false);

	events.clearEvents();
	return 0;
}

void ReaperCutscene::getNewLocation() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	if (_ccNum) {
		switch (party._mazeId) {
		case 3:
			if (party._questItems[40]) {
				_mazeId = 57;
				_mazePos = Common::Point(11, 8);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		case 12:
			if (party._questItems[3]) {
				_mazeId = 55;
				_mazePos = Common::Point(3, 8);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		case 13:
			if (party._questItems[43]) {
				_mazeId = 69;
				_mazePos = Common::Point(7, 4);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 16:
			if (party._questItems[41]) {
				_mazeId = 61;
				_mazePos = Common::Point(7, 12);
				_mazeDir = DIR_SOUTH;
				_keyFound = true;
			}
			break;

		case 23:
			if (party._questItems[42]) {
				_mazeId = 65;
				_mazePos = Common::Point(3, 8);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		case 29:
			if (party._questItems[44]) {
				_mazeId = 53;
				_mazePos = Common::Point(11, 8);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	} else {
		switch (party._mazeId) {
		case 7:
			if (party._questItems[46]) {
				map._loadCcNum = 1;
				_mazeId = 113;
				_mazePos = Common::Point(7, 4);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 12:
			if (party._questItems[3]) {
				_mazeId = 55;
				_mazePos = Common::Point(3, 8);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		case 13:
			if (party._questItems[45]) {
				map._loadCcNum = 1;
				_mazeId = 117;
				_mazePos = Common::Point(7, 4);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 15:
			if (party._questItems[2]) {
				_mazeId = 59;
				_mazePos = Common::Point(11, 8);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		case 24:
			if (party._questItems[1]) {
				_mazeId = 51;
				_mazePos = Common::Point(7, 12);
				_mazeDir = DIR_SOUTH;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

static const int16 GOLEM_X1[2][12] = {
	{ 0, -5, 0, 6, 10, 13, 17, 20, 23, 26, 29, 31 },
	{ 0, 0, 1, 1, 1, 0, -9, -20, -21, 0, 0, 0 }
};
static const int GOLEM_Y1[2][12] = {
	{ 0, 0, 0, 0, 0, 5, 10, 15, 20, 25, 30, 35 },
	{ 0, 6, 12, 18, 24, 30, 29, 23, 25, 0, 0, 0 }
};
static const int GOLEM_X2[2][12] = {
	{ 160, 145, 140, 136, 130, 123, 117, 110, 103, 96, 89, 81 },
	{ 160, 150, 141, 131, 121, 110, 91, 70, 57, 0, 0, 0 }
};

GolemCutscene::GolemCutscene() : CutsceneLocation(GOLEM) {
}

int GolemCutscene::show() {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;
	SpriteResource sprites1, sprites2[2];
	sprites1.load(_ccNum ? "dung1.zom" : "golmback.vga");
	sprites2[0].load(_ccNum ? "dung2.zom" : "golem.vga");
	if (_ccNum)
		sprites2[1].load("dung3.zom");

	// Save the screen
	Graphics::ManagedSurface savedBg;
	savedBg.copyFrom(screen);
	getNewLocation();

	for (int idx = (_ccNum ? 8 : 11); idx >= 0; --idx) {
		events.updateGameCounter();
		screen.blitFrom(savedBg);
		sprites1.draw(0, 0,
			Common::Point(GOLEM_X1[_ccNum][idx], GOLEM_Y1[_ccNum][idx]), 0, idx);
		sprites1.draw(0, 1,
			Common::Point(GOLEM_X2[_ccNum][idx], GOLEM_Y1[_ccNum][idx]), 0, idx);

		windows[0].update();
		WAIT(1);
	}

	if (!_ccNum)
		sound.playSound("ogre.voc");

	for (int idx = -200; idx < 0; idx += 16) {
		events.updateGameCounter();
		sprites1.draw(0, 0, Common::Point(0, 0));
		sprites1.draw(0, 1, Common::Point(160, 0));
		sprites2[0].draw(0, 0, Common::Point(idx, 0), SPRFLAG_800);
		sprites2[_ccNum].draw(0, 1, Common::Point(idx + 160, 0), SPRFLAG_800);

		if (!_ccNum)
			sprites2[0].draw(0, 2, Common::Point(idx + g_vm->getRandomNumber(9) - 5,
				g_vm->getRandomNumber(9) - 5), SPRFLAG_800);

		if (!_ccNum && !sound.isSoundPlaying())
			sound.playSound("ogre.voc");

		WAIT(1);
	}

	sprites1.draw(0, 0, Common::Point(0, 0));
	sprites1.draw(0, 1, Common::Point(160, 0));
	sprites2[0].draw(0, 0, Common::Point(0, 0));
	sprites2[_ccNum].draw(0, 1 - _ccNum, Common::Point(160, 0));
	if (!_ccNum)
		sprites2[0].draw(0, 2);

	windows[0].update();
	while (sound.isSoundPlaying()) {
		_subtitles.show();
		WAIT(1);
	}

	sound.setMusicPercent(38);
	if (_ccNum) {
		_subtitles.setLine(_keyFound ? 5 : 4);
		sound.playVoice("what2.voc");
	} else {
		_subtitles.setLine(_keyFound ? 8 : 7);
		sound.playVoice(_keyFound ? "golem15.voc" : "golem13.voc");
	}

	do {
		events.updateGameCounter();
		sprites1.draw(0, 0, Common::Point(0, 0));
		sprites1.draw(0, 1, Common::Point(160, 0));

		if (_ccNum) {
			int frame = g_vm->getRandomNumber(6);
			sprites2[0].draw(0, frame, Common::Point(0, 0));
			sprites2[1].draw(0, frame, Common::Point(160, 0));
		} else {
			sprites2[0].draw(0, 0, Common::Point(0, 0));
			sprites2[0].draw(0, 1, Common::Point(160, 0));
			sprites2[0].draw(0, 2, Common::Point(g_vm->getRandomNumber(5) - 3,
				g_vm->getRandomNumber(9) - 3));
		}

		_subtitles.show();
		WAIT(1);
	} while (sound.isSoundPlaying());

	sprites1.draw(0, 0, Common::Point(0, 0));
	sprites1.draw(0, 1, Common::Point(160, 0));
	sprites2[0].draw(0, 0, Common::Point(0, 0));
	sprites2[_ccNum].draw(0, 1 - _ccNum, Common::Point(160, 0));
	if (!_ccNum)
		sprites2[0].draw(0, 2);

	windows[0].update();
	events.updateGameCounter();
	if (_subtitles.wait(_ccNum ? 10 : 1))
		goto exit;

	if (_ccNum) {
		sound.playVoice(_keyFound ? "go2.voc" : "key2.voc");
	} else {
		sound.playVoice("ogre.voc");
		while (sound.isSoundPlaying())
			events.pollEventsAndWait();

		sound.playVoice(_keyFound ? "golem16.voc" : "golem14.voc");
	}

	do {
		events.updateGameCounter();
		sprites1.draw(0, 0, Common::Point(0, 0));
		sprites1.draw(0, 1, Common::Point(160, 0));

		if (_ccNum) {
			int frame = g_vm->getRandomNumber(6);
			sprites2[0].draw(0, frame, Common::Point(0, 0));
			sprites2[1].draw(0, frame, Common::Point(160, 0));
		} else {
			sprites2[0].draw(0, 0, Common::Point(0, 0));
			sprites2[0].draw(0, 1, Common::Point(160, 0));
			sprites2[0].draw(0, 2, Common::Point(g_vm->getRandomNumber(5) - 3,
				g_vm->getRandomNumber(9) - 3));
		}

		windows[0].update();
		_subtitles.show();
		WAIT(1);
	} while (_subtitles.lineActive());

	sprites1.draw(0, 0, Common::Point(0, 0));
	sprites1.draw(0, 1, Common::Point(160, 0));
	sprites2[0].draw(0, 0, Common::Point(0, 0));
	sprites2[_ccNum].draw(0, 1 - _ccNum, Common::Point(160, 0));
	if (!_ccNum)
		sprites2[0].draw(0, 2);
	windows[0].update();

	sound.setMusicPercent(75);

	if (!_keyFound) {
		for (int idx = 0; !g_vm->shouldExit() && idx < (_ccNum ? 9 : 12); ++idx) {
			events.updateGameCounter();
			screen.blitFrom(savedBg);
			sprites1.draw(0, 0,
				Common::Point(GOLEM_X1[_ccNum][idx], GOLEM_Y1[_ccNum][idx]), 0, idx);
			sprites1.draw(0, 1,
				Common::Point(GOLEM_X2[_ccNum][idx], GOLEM_Y1[_ccNum][idx]), 0, idx);

			windows[0].update();
			WAIT(1);
		}
	}

exit:
	screen.blitFrom(savedBg);
	windows[0].update();

	setNewLocation();

	// Restore game screen
	sound.setMusicPercent(75);
	sound.stopSound();

	screen.loadBackground("back.raw");
	intf.drawParty(false);
	intf.draw3d(false, false);

	events.clearEvents();
	return 0;
}

void GolemCutscene::getNewLocation() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	if (_ccNum) {
		switch (party._mazeId) {
		case 12:
			if (party._questItems[47]) {
				_mazeId = 73;
				_mazePos = Common::Point(0, 7);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 14:
			if (party._questItems[49]) {
				_mazeId = 83;
				_mazePos = Common::Point(11, 1);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 19:
			if (party._questItems[50]) {
				_mazeId = 121;
				_mazePos = Common::Point(18, 0);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 22:
			if (party._questItems[48]) {
				_mazeId = 78;
				_mazePos = Common::Point(8, 14);
				_mazeDir = DIR_SOUTH;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	} else {
		switch (party._mazeId) {
		case 8:
			if (party._questItems[6]) {
				_mazeId = 81;
				_mazePos = Common::Point(1, 17);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		case 12:
			if (party._questItems[5]) {
				_mazeId = 80;
				_mazePos = Common::Point(29, 16);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		case 19:
			if (party._questItems[50]) {
				map._loadCcNum = 1;
				_mazeId = 121;
				_mazePos = Common::Point(18, 0);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 20:
			if (party._questItems[7]) {
				_mazeId = 79;
				_mazePos = Common::Point(5, 16);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

static const int16 DWARF_X0[2][13] = {
	{  0, -5, -7, -8, -11, -9, -3, 1, 6, 10, 15, 18, 23 },
	{ 0, 4, 6, 8, 11, 12, 15, 17, 19, 22, 25, 0, 0 }
};
static const int DWARF_X1[2][13] = {
	{ 160, 145, 133, 122, 109, 101, 97, 91, 86, 80, 75, 68, 63 },
	{ 160, 154, 146, 138, 131, 122, 115, 107, 99, 92, 85, 0, 0 }
};
static const int DWARF_X2[13] = {
	0, -1, -4, -7, -9, -13, -15, -18, -21, -23, -25, 0, 0
};
static const int16 DWARF_Y[2][13] = {
	{ 0, 0, 4, 9, 13, 15, 20, 24, 30, 37, 45, 51, 58 },
	{ 0, 12, 25, 36, 38, 40, 41, 42, 44, 45, 50, 0, 0 }
};
static const int16 DWARF2_X[2][16] = {
	{ 0, -2, -4, -6, -8, -10, -12, -14, -16, -18, -20, -20, -20, -20, -20, -20 },
	{ 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150 }
};
static const int16 DWARF2_Y[2][16] = {
	{ 0, 12, 25, 37, 50, 62, 75, 87, 100, 112, 125, 137, 150, 162, 175, 187 },
	{ 0, 12, 25, 37, 50, 62, 75, 87, 100, 112, 125, 137, 150, 162, 175, 186 }
};

DwarfCutscene::DwarfCutscene() : CutsceneLocation(DWARF_MINE) {}

int DwarfCutscene::show() {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;

	SpriteResource sprites1(_ccNum ? "town1.zom" : "dwarf1.vga");
	SpriteResource sprites2(_ccNum ? "town2.zom" : "dwarf3.vga");
	SpriteResource sprites3(_ccNum ? "town3.zom" : "dwarf2.vga");
	getNewLocation();

	// Save the screen contents
	Graphics::ManagedSurface savedBg;
	savedBg.copyFrom(screen);

	// Zoom in on the mine entrance
	for (int idx = (_ccNum ? 10 : 12); idx >= 0; --idx) {
		events.updateGameCounter();

		screen.blitFrom(savedBg);
		sprites1.draw(0, 0,
			Common::Point(DWARF_X0[_ccNum][idx], DWARF_Y[_ccNum][idx]), 0, idx);
		sprites1.draw(0, 1,
			Common::Point(DWARF_X1[_ccNum][idx], DWARF_Y[_ccNum][idx]), 0, idx);
		if (_ccNum)
			sprites1.draw(0, 2,
				Common::Point(DWARF_X2[idx], DWARF_Y[_ccNum][idx]), 0, idx);

		windows[0].update();
		WAIT(1);
	}

	// Have character rise up from the bottom of the screen
	savedBg.copyFrom(screen);
	for (int idx = 15; idx >= 0; --idx) {
		if (g_vm->shouldExit())
			return 0;
		events.updateGameCounter();

		screen.blitFrom(savedBg);
		sprites2.draw(0, 0, Common::Point(DWARF2_X[_ccNum][idx], DWARF2_Y[_ccNum][idx]), 0, idx);
		windows[0].update();
		WAIT(1);
	}

	sound.setMusicPercent(38);
	screen.blitFrom(savedBg);
	sprites2.draw(0, 0);
	windows[0].update();
	if (_ccNum)
		_subtitles.setLine(_keyFound ? 7 : 8);
	else
		_subtitles.setLine(4);

	for (int idx = 0; idx < (_ccNum ? 2 : 3); ++idx) {
		switch (idx) {
		case 0:
			sound.playSound(_ccNum ? "pass2.voc" : "dwarf10.voc");
			break;

		case 1:
			if (_ccNum) {
				sprites2.draw(0, 0);
				sprites3.draw(0, 0);
				_subtitles.show();

				events.timeMark5();
				while (!g_vm->shouldExit() && events.timeElapsed5() < 7) {
					WAIT(1);
				}

				sound.playSound(_keyFound ? "ok2.voc" : "back2.voc");
			} else {
				sound.playSound("dwarf11.voc");
			}
			break;

		case 2:
			sound.playSound("dwarf12.voc");
			break;

		default:
			break;
		}

		events.updateGameCounter();
		do {
			sprites2.draw(0, 0);
			sprites3.draw(0, g_vm->getRandomNumber(_ccNum ? 8 : 9));
			_subtitles.show();

			events.timeMark5();
			while (events.timeElapsed5() < 2) {
				WAIT(1);
			}
		} while (_subtitles.lineActive());

		WAIT(3);
	}

exit:
	sprites2.draw(0, 0);
	if (!_ccNum)
		sprites3.draw(0, 1);
	windows[0].update();

	setNewLocation();

	// Restore game screen
	sound.setMusicPercent(75);
	sound.stopSound();

	screen.loadBackground("back.raw");
	intf.drawParty(false);
	intf.draw3d(false, false);

	events.clearEvents();
	return 0;
}

void DwarfCutscene::getNewLocation() {
	Party &party = *g_vm->_party;

	if (g_vm->getGameID() == GType_Swords) {
		switch (party._mazeId) {
		case 1:
			if (party._questItems[0]) {
				_mazeId = 53;
				_mazePos = Common::Point(8, 1);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		case 7:
			if (party._questItems[1]) {
				_mazeId = 92;
				_mazePos = Common::Point(8, 1);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	} else if (_ccNum) {
		switch (party._mazeId) {
		case 4:
			if (party._questItems[35]) {
				_mazeId = 29;
				_mazePos = Common::Point(15, 31);
				_mazeDir = DIR_SOUTH;
				_keyFound = true;
			}
			break;

		case 6:
			if (party._questItems[38]) {
				_mazeId = 35;
				_mazePos = Common::Point(15, 8);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		case 19:
			if (party._questItems[36]) {
				_mazeId = 31;
				_mazePos = Common::Point(31, 16);
				_mazeDir = DIR_WEST;
				_keyFound = true;
			}
			break;

		case 22:
			if (party._questItems[37]) {
				_mazeId = 33;
				_mazePos = Common::Point(0, 3);
				_mazeDir = DIR_EAST;
				_keyFound = true;
			}
			break;

		case 98:
			if (party._questItems[39]) {
				_mazeId = 37;
				_mazePos = Common::Point(7, 0);
				_mazeDir = DIR_NORTH;
				_keyFound = true;
			}
			break;

		default:
			break;
		}
	} else {
		switch (party._mazeId) {
		case 14:
			_mazeId = 37;
			_mazePos = Common::Point(1, 4);
			_mazeDir = DIR_EAST;
			_keyFound = true;
			break;

		case 18:
			if (party._mazePosition.x == 9) {
				_mazeId = 35;
				_mazePos = Common::Point(1, 12);
				_mazeDir = DIR_EAST;
			} else {
				_mazeId = 36;
				_mazePos = Common::Point(7, 1);
				_mazeDir = DIR_NORTH;
			}
			_keyFound = true;
			break;

		case 23:
			if (party._mazePosition.x == 5) {
				_mazeId = 33;
				_mazePos = Common::Point(7, 1);
				_mazeDir = DIR_NORTH;
			} else {
				_mazeId = 34;
				_mazePos = Common::Point(7, 30);
				_mazeDir = DIR_SOUTH;
			}
			_keyFound = true;
			break;

		default:
			break;
		}
	}
}

/*------------------------------------------------------------------------*/

static const int SPHINX_X1[9] = { 0, -5, -10, -15, -20, -17, -12, -7, 0 };
static const int SPHINX_Y1[9] = { 0, 0, 0, 6, 11, 16, 20, 23, 28 };
static const int SPHINX_X2[9] = { 160, 145, 130, 115, 100, 93, 88, 83, 80 };

SphinxCutscene::SphinxCutscene() : CutsceneLocation(SPHINX) {
}

int SphinxCutscene::show() {
	EventsManager &events = *g_vm->_events;
	Interface &intf = *g_vm->_interface;
	Screen &screen = *g_vm->_screen;
	Sound &sound = *g_vm->_sound;
	Windows &windows = *g_vm->_windows;
	SpriteResource sprites1("sphinx.vga");

	getNewLocation();

	// Save background
	Graphics::ManagedSurface bgSurface;
	bgSurface.copyFrom(screen);

	for (int idx = 8; idx >= 0; --idx) {
		events.updateGameCounter();
		screen.blitFrom(bgSurface);
		sprites1.draw(0, 0, Common::Point(SPHINX_X1[idx], SPHINX_Y1[idx]), 0, idx);
		sprites1.draw(0, 1, Common::Point(SPHINX_X2[idx], SPHINX_Y1[idx]), 0, idx);
		windows[0].update();
		WAIT(1);
	}

	sound.setMusicPercent(38);

	for (int idx = 0; idx < (_keyFound ? 3 : 2); ++idx) {
		switch (idx) {
		case 0:
			_subtitles.setLine(_keyFound ? 9 : 10);
			sound.playSound(_keyFound ? "sphinx10.voc" : "sphinx13.voc");
			break;
		case 1:
			sound.playSound(_keyFound ? "sphinx11.voc" : "sphinx14.voc");
			break;
		case 2:
			sound.playSound("sphinx12.voc");
			break;
		default:
			break;
		}

		do {
			sprites1.draw(0, 0, Common::Point(0, 0));
			sprites1.draw(0, 1, Common::Point(160, 0));
			sprites1.draw(0, g_vm->getRandomNumber(2, 10));
			WAIT(1);
		} while (sound.isSoundPlaying());

		sprites1.draw(0, 0, Common::Point(0, 0));
		sprites1.draw(0, 1, Common::Point(160, 0));
	}

	while (_subtitles.lineActive()) {
		WAIT(1);
	}

	sound.setMusicPercent(75);

	if (!_keyFound) {
		for (int idx = 0; idx < 8; ++idx) {
			screen.blitFrom(bgSurface);
			sprites1.draw(0, 0, Common::Point(SPHINX_X1[idx], SPHINX_Y1[idx]), 0, idx);
			sprites1.draw(0, 1, Common::Point(SPHINX_X2[idx], SPHINX_Y1[idx]), 0, idx);
			windows[0].update();
			WAIT(1);
		}

		screen.blitFrom(bgSurface);
		windows[0].update();
	}
exit:
	screen.blitFrom(bgSurface);
	windows[0].update();

	setNewLocation();

	// Restore game screen
	sound.setMusicPercent(75);
	screen.loadBackground("back.raw");
	intf.drawParty(false);
	intf.draw3d(false, false);

	events.clearEvents();
	return 0;
}

void SphinxCutscene::getNewLocation() {
	Map &map = *g_vm->_map;
	Party &party = *g_vm->_party;

	switch (party._mazeId) {
	case 2:
		if (party._questItems[51]) {
			map._loadCcNum = 1;
			_mazeId = 125;
			_mazePos = Common::Point(7, 6);
			_mazeDir = DIR_NORTH;
			_keyFound = true;
		}
		break;

	case 5:
		if (party._questItems[4]) {
			_mazeId = 82;
			_mazePos = Common::Point(7, 5);
			_mazeDir = DIR_NORTH;
			_keyFound = true;
		}
		break;

	default:
		break;
	}
}

/*------------------------------------------------------------------------*/

PyramidLocation::PyramidLocation() : BaseLocation(PYRAMID) {
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
		if (_ccNum) {
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
		map._loadCcNum = _ccNum ? 0 : 1;
		map.load(mapId);
		party._mazePosition = pt;
		party._mazeDirection = dir;
	} else {
		// Playing Clouds or Dark Side on it's own, so can't switch sides
		Window &win = windows[12];
		Common::String msg = Common::String::format(Res.MOONS_NOT_ALIGNED,
			_ccNum ? "Clouds" : "Darkside");
		win.open();
		win.writeString(msg);
		win.update();

		events.waitForPressAnimated();
		win.close();
	}

	return 0;
}

} // End of namespace Locations

/*------------------------------------------------------------------------*/

LocationManager::LocationManager() : _location(nullptr) {
}

int LocationManager::doAction(int actionId) {
	LocationAction action = (g_vm->getGameID() == GType_Swords && actionId > 13 && actionId < 18) ?
		BLACKSMITH : (LocationAction)actionId;

	// Create the desired location
	switch (action) {
	case BANK:
		_location = new Locations::BankLocation();
		break;
	case BLACKSMITH:
		_location = new Locations::BlacksmithLocation();
		break;
	case GUILD:
		_location = new Locations::GuildLocation();
		break;
	case TAVERN:
		_location = new Locations::TavernLocation();
		break;
	case TEMPLE:
		_location = new Locations::TempleLocation();
		break;
	case TRAINING:
		_location = new Locations::TrainingLocation();
		break;
	case ARENA:
		_location = new Locations::ArenaLocation();
		break;
	case REAPER:
		_location = new Locations::ReaperCutscene();
		break;
	case GOLEM:
		_location = new Locations::GolemCutscene();
		break;
	case DWARF_MINE:
	case DWARF_TOWN:
		_location = new Locations::DwarfCutscene();
		break;
	case SPHINX:
		_location = new Locations::SphinxCutscene();
		break;
	case PYRAMID:
		_location = new Locations::PyramidLocation();
		break;
	default:
		return 0;
	}

	// Show the location
	g_vm->_events->clearEvents();
	int result = _location->show();
	delete _location;
	_location = nullptr;

	return result;
}

bool LocationManager::isActive() const {
	return _location != nullptr;
}

void LocationManager::drawAnim(bool flag) {
	if (_location)
		_location->drawAnim(flag);
}

int LocationManager::wait() {
	return _location ? _location->wait() : 0;
}

/*------------------------------------------------------------------------*/

bool LocationMessage::showMessage(int portrait, const Common::String &name,
		const Common::String &text, int confirm) {
	LocationMessage *dlg = new LocationMessage();
	bool result = dlg->execute(portrait, name, text, confirm);
	delete dlg;

	return result;
}

bool LocationMessage::execute(int portrait, const Common::String &name, const Common::String &text,
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
	_animPos = Common::Point(23, 22);

	if (!confirm)
		loadButtons();

	_townSprites.resize(2);
	_townSprites[0].load(Common::Path(Common::String::format("face%02d.fac", portrait)));
	_townSprites[1].load("frame.fac");

	if (!w._enabled)
		w.open();

	int result = -1;
	Common::String msgText = text;
	do {
		Common::String msg = Common::String::format(g_vm->getLanguage() == Common::ZH_TWN ? "\r\v014\x0c""07\x03""c\t125%s\x0c""04\x03""l\t000\v044%s" : "\r\v014\x03""c\t125%s\t000\v054%s",
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
			drawButtons(&windows[0]);
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
			clearEvents();

			do {
				events.updateGameCounter();
				while (!_buttonValue && events.timeElapsed() < 3) {
					events.pollEventsAndWait();
					checkEvents(_vm);
					if (g_vm->shouldExit())
						return false;
				}

				drawAnim(false);
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

void LocationMessage::loadButtons() {
	_iconSprites.load("confirm.icn");

	addButton(Common::Rect(235, 75, 259, 95), Common::KEYCODE_y, &_iconSprites);
	addButton(Common::Rect(260, 75, 284, 95), Common::KEYCODE_n, &_iconSprites);
	addButton(Common::Rect(), Common::KEYCODE_ESCAPE);
}

} // End of namespace Xeen
} // End of namespace MM
