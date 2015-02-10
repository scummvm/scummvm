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

#include "common/scummsys.h"
#include "xeen/dialogs_party.h"
#include "xeen/character.h"
#include "xeen/events.h"
#include "xeen/party.h"
#include "xeen/xeen.h"

namespace Xeen {

	void PartyDialog::show(XeenEngine *vm) {
	PartyDialog *dlg = new PartyDialog(vm);
	dlg->execute();
	delete dlg;
}

void PartyDialog::execute() {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Screen &screen = *_vm->_screen;

	loadButtons();
	setupBackground();

	_vm->_mode = MODE_1;
	Common::Array<int> xeenSideChars;

	// Build up a list of characters on the same Xeen side being loaded
	for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
		Character &player = _vm->_roster[i];
		if (player._name.empty() || player._xeenSide != (map._loadDarkSide ? 1 : 0))
			continue;

		xeenSideChars.push_back(i);
	}

	Window &w = screen._windows[11];
	w.open();
	setupFaces(0, xeenSideChars, false);
	w.writeString(_displayText);
	w.drawList(&_faceDrawStructs[0], 4);

	_iconSprites.draw(w, 0, Common::Point(16, 100));
	_iconSprites.draw(w, 2, Common::Point(52, 100));
	_iconSprites.draw(w, 4, Common::Point(87, 100));
	_iconSprites.draw(w, 6, Common::Point(122, 100));
	_iconSprites.draw(w, 8, Common::Point(157, 100));
	_iconSprites.draw(w, 10, Common::Point(192, 100));

	screen.loadPalette("mm4.pal");
	// TODO
}

void PartyDialog::loadButtons() {
	_iconSprites.load("inn.icn");
	addButton(Common::Rect(16, 100, 40, 120), Common::KEYCODE_UP, &_iconSprites);
	addButton(Common::Rect(52, 100, 76, 120), Common::KEYCODE_DOWN, &_iconSprites);
	addButton(Common::Rect(87, 100, 111, 120), Common::KEYCODE_d, &_iconSprites);
	addButton(Common::Rect(122, 100, 146, 120), Common::KEYCODE_r, &_iconSprites);
	addButton(Common::Rect(157, 100, 181, 120), Common::KEYCODE_c, &_iconSprites);
	addButton(Common::Rect(192, 100, 116, 120), Common::KEYCODE_x, &_iconSprites);
	addButton(Common::Rect(0, 0, 0, 0), Common::KEYCODE_ESCAPE, &_iconSprites, false);
	addButton(Common::Rect(16, 16, 48, 48), Common::KEYCODE_1, &_iconSprites, false);
	addButton(Common::Rect(117, 16, 149, 48), Common::KEYCODE_2, &_iconSprites, false);
	addButton(Common::Rect(59, 59, 91, 91), Common::KEYCODE_3, &_iconSprites, false);
	addButton(Common::Rect(117, 59, 151, 91), Common::KEYCODE_4, &_iconSprites, false);
}

void PartyDialog::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);
}

void PartyDialog::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	_vm->_interface->assembleBorder();
}

/**
 * Sets up the faces for display in the party dialog
 */
void PartyDialog::setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag) {
	Resources &res = *_vm->_resources;
	Common::String charNames[4];
	Common::String charRaces[4];
	Common::String charSex[4];
	Common::String charClasses[4];
	int posIndex;
	int charId;

	for (posIndex = 0; posIndex < 4; ++posIndex) {
		charId = xeenSideChars[charIndex];
		bool isInParty = _vm->_party->isInParty(charId);

		if (charId == 0xff) {
			while ((int)_buttons.size() >(7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		Character &ps = _vm->_roster[xeenSideChars[charIndex + posIndex]];
		charNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		charRaces[posIndex] = RACE_NAMES[ps._race];
		charSex[posIndex] = SEX_NAMES[ps._sex];
		charClasses[posIndex] = CLASS_NAMES[ps._class];
	}

	charIconsPrint(updateFlag);

	// Set up the sprite set to use for each face
	charId = xeenSideChars[charIndex];
	_faceDrawStructs[0]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &res._charFaces[charId];
	charId = xeenSideChars[charIndex + 1];
	_faceDrawStructs[1]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &res._charFaces[charId];
	charId = xeenSideChars[charIndex + 2];
	_faceDrawStructs[2]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &res._charFaces[charId];
	charId = xeenSideChars[charIndex + 3];
	_faceDrawStructs[3]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &res._charFaces[charId];

	_displayText = Common::String::format(PARTY_DETAILS,
		charNames[0].c_str(), charRaces[0].c_str(), charSex[0].c_str(), charClasses[0].c_str(),
		charNames[1].c_str(), charRaces[1].c_str(), charSex[1].c_str(), charClasses[1].c_str(),
		charNames[2].c_str(), charRaces[2].c_str(), charSex[2].c_str(), charClasses[2].c_str(),
		charNames[3].c_str(), charRaces[3].c_str(), charSex[3].c_str(), charClasses[3].c_str()
		);
}

} // End of namespace Xeen
