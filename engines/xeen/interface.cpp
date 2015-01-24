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

#include "xeen/interface.h"
#include "xeen/dialogs_error.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Interface::Interface(XeenEngine *vm) : ButtonContainer(), InterfaceMap(vm), _vm(vm) {
	Common::fill(&_partyFaces[0], &_partyFaces[MAX_ACTIVE_PARTY], (SpriteResource *)nullptr);
	_buttonsLoaded = false;
	_hiliteChar = -1;
	_intrIndex1 = 0;
	_steppingFX = 0;

	Common::fill(&_combatCharIds[0], &_combatCharIds[8], 0);
	initDrawStructs();
}


void Interface::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);

	_mainList[0] = DrawStruct(7, 232, 74);
	_mainList[1] = DrawStruct(0, 235, 75);
	_mainList[2] = DrawStruct(2, 260, 75);
	_mainList[3] = DrawStruct(4, 286, 75);
	_mainList[4] = DrawStruct(6, 235, 96);
	_mainList[5] = DrawStruct(8, 260, 96);
	_mainList[6] = DrawStruct(10, 286, 96);
	_mainList[7] = DrawStruct(12, 235, 117);
	_mainList[8] = DrawStruct(14, 260, 117);
	_mainList[9] = DrawStruct(16, 286, 117);
	_mainList[10] = DrawStruct(20, 235, 148);
	_mainList[11] = DrawStruct(22, 260, 148);
	_mainList[12] = DrawStruct(24, 286, 148);
	_mainList[13] = DrawStruct(26, 235, 169);
	_mainList[14] = DrawStruct(28, 260, 169);
	_mainList[15] = DrawStruct(30, 286, 169);
}

void Interface::setup() {
	InterfaceMap::setup();
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_uiSprites.load("inn.icn");

	// Get mappings to the active characters in the party
	_vm->_party->_activeParty.resize(_vm->_party->_partyCount);
	for (int i = 0; i < _vm->_party->_partyCount; ++i) {
		_vm->_party->_activeParty[i] = _vm->_roster[_vm->_party->_partyMembers[i]];
	}

	_vm->_party->_newDay = _vm->_party->_minutes >= 300;
}

void Interface::manageCharacters(bool soundPlayed) {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Screen &screen = *_vm->_screen;
	bool flag = false;

start:
	if (_vm->_party->_mazeId != 0) {
		_vm->_mode = MODE_0;
		_buttonsLoaded = true;
	} else {
		if (!soundPlayed) {
			warning("TODO: loadSound?");
		}

		if (!_partyFaces[0]) {
			// Xeen only uses 24 of possible 30 character slots
			loadCharIcons();

			for (int i = 0; i < _vm->_party->_partyCount; ++i)
				_partyFaces[i] = &_charFaces[_vm->_party->_partyMembers[i]];
		}

		_vm->_mode = MODE_1;
		Common::Array<int> xeenSideChars;

		// Build up a list of characters on the same Xeen side being loaded
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			Character &player = _vm->_roster[i];
			if (player._name.empty() || player._xeenSide != (map._loadDarkSide ? 1 : 0))
				continue;

			xeenSideChars.push_back(i);
		}

		// Add in buttons for the UI
		_interfaceText = "";
		_buttonsLoaded = true;
		addButton(Common::Rect(16, 100, 40, 120), 242, &_uiSprites, true);
		addButton(Common::Rect(52, 100, 76, 120), 243, &_uiSprites, true);
		addButton(Common::Rect(87, 100, 111, 120), 68, &_uiSprites, true);
		addButton(Common::Rect(122, 100, 146, 120), 82, &_uiSprites, true);
		addButton(Common::Rect(157, 100, 181, 120), 67, &_uiSprites, true);
		addButton(Common::Rect(192, 100, 216, 120), 88, &_uiSprites, true);
		addButton(Common::Rect(), 27, &_uiSprites, false);
		addButton(Common::Rect(16, 16, 48, 48), 49, &_uiSprites, false);
		addButton(Common::Rect(117, 16, 139, 48), 50, &_uiSprites, false);
		addButton(Common::Rect(16, 59, 48, 81), 51, &_uiSprites, false);
		addButton(Common::Rect(117, 59, 149, 81), 52, &_uiSprites, false);

		setupBackground();
		Window &w = screen._windows[11];
		w.open();
		setupFaces(0, xeenSideChars, false);
		w.writeString(_interfaceText);
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));

		screen.loadPalette("mm4.pal");

		if (flag) {
			screen._windows[0].update();
			events.setCursor(0);
			screen.fadeIn(4);
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut(4);
				screen._windows[0].update();
			}

			doScroll(_vm, false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn(4);
			}
		}

		// TODO
		bool breakFlag = false;
		while (!_vm->shouldQuit() && !breakFlag) {
			events.pollEventsAndWait();
			checkEvents(_vm);

			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_e:
			case Common::KEYCODE_x:
				if (_vm->_party->_partyCount == 0) {
					ErrorScroll::show(_vm, NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (_intrIndex1 = 4; _intrIndex1 >= 0; --_intrIndex1) {
							events.updateGameCounter();
							drawViewBackground(_intrIndex1);
							w.update();

							while (events.timeElapsed() < 1)
								events.pollEventsAndWait();
						}
					}

					w.close();
					_vm->_party->_realPartyCount = _vm->_party->_partyCount;
					_vm->_party->_mazeId = _vm->_party->_priorMazeId;

					_vm->_party->copyPartyToRoster(_vm->_roster);
					_vm->_saves->writeCharFile();
					breakFlag = true;
					break;
				}
				break;
			case Common::KEYCODE_1:
				break;
			case Common::KEYCODE_2:
				break;
			case Common::KEYCODE_3:
				break;
			case Common::KEYCODE_4:
				break;
			case Common::KEYCODE_c:
				if (xeenSideChars.size() == 24) {
					ErrorScroll::show(_vm, YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut(4);
					w.close();
					moveCharacterToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut(4);
					flag = true;
					_buttonsLoaded = true;
					goto start;
				}
				break;
			case Common::KEYCODE_d:
				break;
			case Common::KEYCODE_r:
				if (_vm->_party->_partyCount > 0) {
					// TODO
				}
				break;
			case 201:
				// TODO
				break;
			case 202:
				// TODO
				break;
			case 203:
				// TODO
				break;
			case 204:
				// TODO
				break;
			case 205:
				// TODO
				break;
			case 206:
				// TODO
				break;
			case 242:
				// TODO
				break;
			case 243:
				// TODO
				break;
			default:
				break;
			}
		}
	}

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		_charFaces[i].clear();
}

void Interface::loadCharIcons() {
	for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
		// Load new character resource
		Common::String name = Common::String::format("char%02d.fac", i + 1);
		_charFaces[i].load(name);
	}

	_dseFace.load("dse.fac");
}

void Interface::loadPartyIcons() {
	for (int i = 0; i < _vm->_party->_partyCount; ++i)
		_partyFaces[i] = &_charFaces[_vm->_party->_partyMembers[i]];
}

void Interface::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	assembleBorder();
}

void Interface::setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag) {
	Common::String playerNames[4];
	Common::String playerRaces[4];
	Common::String playerSex[4];
	Common::String playerClass[4];
	int posIndex;
	int charId;

	for (posIndex = 0; posIndex < 4; ++posIndex) {
		charId = xeenSideChars[charIndex];
		bool isInParty = _vm->_party->isInParty(charId);

		if (charId == 0xff) {
			while ((int)_buttons.size() > (7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		Character &ps = _vm->_roster[xeenSideChars[charIndex + posIndex]];
		playerNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		playerRaces[posIndex] = RACE_NAMES[ps._race];
		playerSex[posIndex] = SEX_NAMES[ps._sex];
		playerClass[posIndex] = CLASS_NAMES[ps._class];
	}

	charIconsPrint(updateFlag);

	// Set up the sprite set to use for each face
	charId = xeenSideChars[charIndex];
	_faceDrawStructs[0]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 1];
	_faceDrawStructs[1]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 2];
	_faceDrawStructs[2]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 3];
	_faceDrawStructs[3]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];

	_interfaceText = Common::String::format(PARTY_DETAILS,
		playerNames[0].c_str(), playerRaces[0].c_str(), playerSex[0].c_str(), playerClass[0].c_str(),
		playerNames[1].c_str(), playerRaces[1].c_str(), playerSex[1].c_str(), playerClass[1].c_str(),
		playerNames[2].c_str(), playerRaces[2].c_str(), playerSex[2].c_str(), playerClass[2].c_str(),
		playerNames[3].c_str(), playerRaces[3].c_str(), playerSex[3].c_str(), playerClass[3].c_str()
	);
}

void Interface::charIconsPrint(bool updateFlag) {
	Screen &screen = *_vm->_screen;
	bool stateFlag = _vm->_mode == MODE_2;
	_restoreSprites.draw(screen, 0, Common::Point(8, 149));

	// Handle drawing the party faces
	for (int idx = 0; idx < (stateFlag ? _vm->_party->_combatPartyCount : 
			_vm->_party->_partyCount); ++idx) {
		int charIndex = stateFlag ? _combatCharIds[idx] : idx;
		Character &ps = _vm->_party->_activeParty[charIndex];
		Condition charCondition = ps.worstCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];
		
		SpriteResource *sprites = (charFrame > 4 && !_charFaces[0].empty()) ?
			&_dseFace : _partyFaces[charIndex];
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(screen, charFrame, Common::Point(CHAR_FACES_X[idx], 150));
	}

	if (!_hpSprites.empty()) {
		for (int idx = 0; idx < (stateFlag ? _vm->_party->_combatPartyCount :
			_vm->_party->_partyCount); ++idx) {
			int charIndex = stateFlag ? _combatCharIds[idx] : idx;
			Character &ps = _vm->_party->_activeParty[charIndex];

			// Draw the Hp bar
			int maxHp = ps.getMaxHP();
			int frame;
			if (ps._currentHp < 1)
				frame = 4;
			else if (ps._currentHp > maxHp)
				frame = 3;
			else if (ps._currentHp == maxHp)
				frame = 0;
			else if (ps._currentHp < (maxHp / 4))
				frame = 2;
			else
				frame = 1;

			_hpSprites.draw(screen, frame, Common::Point(HP_BARS_X[idx], 182));
		}
	}

	if (_hiliteChar != -1)
		_globalSprites.draw(screen, 8, Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));

	if (updateFlag)
		screen._windows[33].update();
}

void Interface::drawViewBackground(int bgType) {
	if (bgType >= 4)
		return;

	if (bgType == 0) {
		// Totally black background
		_vm->_screen->fillRect(Common::Rect(8, 8, 224, 140), 0);
	} else {
		const byte *lookup = BACKGROUND_XLAT + bgType;
		for (int yp = 8; yp < 140; ++yp) {
			byte *destP = (byte *)_vm->_screen->getBasePtr(8, yp);
			for (int xp = 8; xp < 224; ++xp, ++destP)
				*destP = lookup[*destP];
		}
	}
}

void Interface::moveCharacterToRoster() {
	error("TODO");
}

void Interface::startup() {
	Screen &screen = *_vm->_screen;
	loadCharIcons();
	_iconSprites.load("main.icn");

	animate3d();
	if (_vm->_map->_isOutdoors) {
		setIndoorsMonsters();
		setIndoorsObjects();
	} else {
		setOutdoorsMonsters();
		setOutdoorsObjects();
	}
	draw3d(false);

	_globalSprites.draw(screen._windows[1], 5, Common::Point(232, 9));
	charIconsPrint(false);

	_mainList[0]._sprites = &_globalSprites;
	for (int i = 1; i < 16; ++i)
		_mainList[i]._sprites = &_iconSprites;

	setMainButtons();

	_tillMove = false;
}

void Interface::mainIconsPrint() {
	Screen &screen = *_vm->_screen;
	screen._windows[38].close();
	screen._windows[12].close();
	screen._windows[0].drawList(_mainList, 16);
	screen._windows[34].update();
}

void Interface::setMainButtons() {
	clearButtons();

	addButton(Common::Rect(235,  75, 259,  95),  Common::KEYCODE_s, &_iconSprites);
	addButton(Common::Rect(260,  75, 284,  95),  Common::KEYCODE_c, &_iconSprites);
	addButton(Common::Rect(286,  75, 310,  95),  Common::KEYCODE_r, &_iconSprites);
	addButton(Common::Rect(235,  96, 259, 116),  Common::KEYCODE_b, &_iconSprites);
	addButton(Common::Rect(260,  96, 284, 116),  Common::KEYCODE_d, &_iconSprites);
	addButton(Common::Rect(286,  96, 310, 116),  Common::KEYCODE_v, &_iconSprites);
	addButton(Common::Rect(235, 117, 259, 137),  Common::KEYCODE_m, &_iconSprites);
	addButton(Common::Rect(260, 117, 284, 137),  Common::KEYCODE_i, &_iconSprites);
	addButton(Common::Rect(286, 117, 310, 137),  Common::KEYCODE_q, &_iconSprites);
	addButton(Common::Rect(109, 137, 122, 147), Common::KEYCODE_TAB, &_iconSprites);
	addButton(Common::Rect(235, 148, 259, 168), Common::KEYCODE_LEFT, &_iconSprites);
	addButton(Common::Rect(260, 148, 284, 168), Common::KEYCODE_UP, &_iconSprites);
	addButton(Common::Rect(286, 148, 310, 168), Common::KEYCODE_RIGHT, &_iconSprites);
	addButton(Common::Rect(235, 169, 259, 189), (Common::KBD_CTRL << 16) |Common::KEYCODE_LEFT, &_iconSprites);
	addButton(Common::Rect(260, 169, 284, 189), Common::KEYCODE_DOWN, &_iconSprites);
	addButton(Common::Rect(286, 169, 310, 189), (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT, &_iconSprites);
	addButton(Common::Rect(236,  11, 308,  69),  Common::KEYCODE_EQUALS, &_iconSprites, false);
	addButton(Common::Rect(239,  27, 312,  37),  Common::KEYCODE_1, &_iconSprites, false);
	addButton(Common::Rect(239, 37, 312, 47), Common::KEYCODE_2, &_iconSprites, false);
	addButton(Common::Rect(239, 47, 312, 57), Common::KEYCODE_3, &_iconSprites, false);
}

/**
 * Waits for a keypress or click, whilst still allowing the game scene to
 * be animated.
 */
void Interface::perform() {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Scripts &scripts = *_vm->_scripts;
	const Common::Rect waitBounds(8, 8, 224, 140);

	events.updateGameCounter();
	draw3d(true);

	// Wait for a frame
	do {
		events.pollEventsAndWait();
		checkEvents(_vm);
	} while (!_buttonValue && events.timeElapsed() < 1 && !_vm->_party->_partyDead);

	if (!_buttonValue && !_vm->_party->_partyDead)
		return;

	if (_buttonValue == Common::KEYCODE_SPACE ||
			(events._leftButton && waitBounds.contains(events._mousePos))) {
		int lookupId = map.mazeLookup(party._mazePosition, 
			WALL_SHIFTS[party._mazeDirection][2]);

		bool eventsFlag = true;
		switch (lookupId) {
		case 1:
			if (!map._isOutdoors) {
				scripts.openGrate(13, 1);
				eventsFlag = _buttonValue != 0;
			}

		case 6:
			if (!map._isOutdoors) {
				scripts.openGrate(9, 0);
				eventsFlag = _buttonValue != 0;
			}
			break;
		case 9:
			if (!map._isOutdoors) {
				scripts.openGrate(6, 0);
				eventsFlag = _buttonValue != 0;
			}
			break;
		case 13:
			if (!map._isOutdoors) {
				scripts.openGrate(1, 1);
				eventsFlag = _buttonValue != 0;
			}
			break;
		default:
			break;
		}
		if (eventsFlag) {
			scripts.checkEvents();
			if (_vm->shouldQuit())
				return;
		}
	}

	switch (_buttonValue) {
	case Common::KEYCODE_TAB:
		// Stop mosters doing any movement
		_vm->_moveMonsters = false;
		warning("TODO: showControlPanel");
		break;

	case Common::KEYCODE_SPACE:
	case Common::KEYCODE_w:
		// Wait one turn
		chargeStep();
		moveMonsters();
		_upDoorText = false;
		_flipDefaultGround = !_flipDefaultGround;
		_flipGround = !_flipGround;

		stepTime();
		break;

	case (Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP4:
		if (checkMoveDirection((Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT)) {
			switch (party._mazeDirection) {
			case DIR_NORTH:
				--party._mazePosition.x;
				break;
			case DIR_SOUTH:
				++party._mazePosition.x;
				break;
			case DIR_EAST:
				++party._mazePosition.y;
				break;
			case DIR_WEST:
				--party._mazePosition.y;
				break;
			default:
				break;
			}

			chargeStep();
			_isAnimReset = true;
			party._mazeDirection = (Direction)((int)party._mazeDirection & 3);
			_flipSky = !_flipSky;
			stepTime();
		}
		break;

	case (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP6:
		if (checkMoveDirection((Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT)) {
			switch (party._mazeDirection) {
			case DIR_NORTH:
				++party._mazePosition.x;
				break;
			case DIR_SOUTH:
				--party._mazePosition.x;
				break;
			case DIR_EAST:
				--party._mazePosition.y;
				break;
			case DIR_WEST:
				++party._mazePosition.y;
				break;
			default:
				break;
			}

			chargeStep();
			_isAnimReset = true;
			party._mazeDirection = (Direction)((int)party._mazeDirection & 3);
			_flipSky = !_flipSky;
			stepTime();
		}
		break;

	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP7:
		party._mazeDirection = (Direction)((int)party._mazeDirection - 1);
		_isAnimReset = true;
		party._mazeDirection = (Direction)((int)party._mazeDirection & 3);
		_flipSky = !_flipSky;
		stepTime();
		break;

	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP9:
		party._mazeDirection = (Direction)((int)party._mazeDirection + 1);
		_isAnimReset = true;
		party._mazeDirection = (Direction)((int)party._mazeDirection & 3);
		_flipSky = !_flipSky;
		stepTime();
		break;

	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		if (checkMoveDirection(Common::KEYCODE_UP)) {
			switch (party._mazeDirection) {
			case DIR_NORTH:
				++party._mazePosition.y;
				break;
			case DIR_SOUTH:
				--party._mazePosition.y;
				break;
			case DIR_EAST:
				++party._mazePosition.x;
				break;
			case DIR_WEST:
				--party._mazePosition.x;
				break;
			default:
				break;
			}

			chargeStep();
			stepTime();
		}
		break;

	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		if (checkMoveDirection(Common::KEYCODE_DOWN)) {
			switch (party._mazeDirection) {
			case DIR_NORTH:
				--party._mazePosition.y;
				break;
			case DIR_SOUTH:
				++party._mazePosition.y;
				break;
			case DIR_EAST:
				--party._mazePosition.x;
				break;
			case DIR_WEST:
				++party._mazePosition.x;
				break;
			default:
				break;
			}

			chargeStep();
			stepTime();
		}
		break;

	case Common::KEYCODE_EQUALS:
	case Common::KEYCODE_KP_EQUALS:
		// Toggle minimap
		party._automapOn = !party._automapOn;
		break;
	default:
		break;
	}
}

void Interface::chargeStep() {
	if (_vm->_party->_partyDead) {
		_vm->_party->changeTime(_vm->_map->_isOutdoors ? 10 : 1);
		if (!_tillMove) {
			moveMonsters();
		}

		_tillMove = 3;
	}
}

/**
 * Handles incrementing game time
 */
void Interface::stepTime() {
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;
	doStepCode();
	
	if (++party._ctr24 == 24)
		party._ctr24 = 0;

	if (_buttonValue != Common::KEYCODE_SPACE && _buttonValue != Common::KEYCODE_w) {
		_steppingFX ^= 1;
		sound.playFX(_steppingFX + 7);
	}

	_upDoorText = false;
	_flipDefaultGround = !_flipDefaultGround;
	_flipGround = !_flipGround;
}

void Interface::doStepCode() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int damage = 0;

	party._stepped = true;
	_upDoorText = false;

	map.getCell(2);
	int surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];

	switch (surfaceId) {
	case SURFTYPE_SPACE:
		// Wheeze.. can't breathe in space! Explosive decompression, here we come 
		party._partyDead = true;
		break;
	case SURFTYPE_LAVA:
		// It burns, it burns! 
		damage = 100;
		party._damageType = DT_FIRE;
		break;
	case SURFTYPE_SKY:
		// We can fly, we can.. oh wait, we can't!
		damage = 100;
		party._damageType = DT_PHYSICAL;
		party._falling = true;
		break;
	case SURFTYPE_DESERT:
		// Without navigation skills, simulate getting lost by adding extra time
		if (map._isOutdoors && !party.checkSkill(NAVIGATOR))
			party.addTime(170);
		break;
	case SURFTYPE_CLOUD:
		if (!party._levitateActive) {
			party._damageType = DT_PHYSICAL;
			party._falling = true;
			damage = 100;
		}
		break;
	default:
		break;
	}

	if (_vm->_files->_isDarkCc && party._gameFlags[374]) {
		party._falling = false;
	} else {
		if (party._falling)
			doFalling();

		if ((party._mazePosition.x & 16) || (party._mazePosition.y & 16)) {
			if (map._isOutdoors)
				map.getNewMaze();
		}

		if (damage) {
			_flipGround = !_flipGround;
			draw3d(true);

			warning("TODO: apply damage");

			_flipGround = !_flipGround;
		} else if (party._partyDead) {
			draw3d(true);
		}
	}
}

void Interface::doFalling() {
	// TODO
}

bool Interface::checkMoveDirection(int key) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;
	Direction dir = party._mazeDirection;

	switch (_buttonValue) {
	case (Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT:
		party._mazeDirection = (party._mazeDirection == DIR_NORTH) ? DIR_WEST :
			(Direction)(party._mazeDirection - 1);
		break;
	case (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT:
		party._mazeDirection = (party._mazeDirection == DIR_WEST) ? DIR_NORTH :
			(Direction)(party._mazeDirection + 1);
		break;
	case Common::KEYCODE_DOWN:
		party._mazeDirection = (party._mazeDirection == DIR_NORTH) ? DIR_SOUTH : DIR_NORTH;
		break;
	default:
		break;
	}
	
	map.getCell(7);
	int startSurfaceId = map._currentSurfaceId;
	int surfaceId;

	if (map._isOutdoors) {
		party._mazeDirection = dir;

		switch ((int)map._currentWall._outdoors._surfaceId) {
		case 5:
			if (_vm->_files->_isDarkCc)
				goto check;

			// Deliberate FAll-through
		case 0:
		case 2:
		case 4:
		case 8:
		case 11:
		case 13:
		case 14:
			surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];
			if (surfaceId == SURFTYPE_WATER) {
				if (party.checkSkill(SWIMMING) || party._walkOnWaterActive)
					return true;
			} else if (surfaceId == SURFTYPE_DWATER) {
				if (party._walkOnWaterActive)
					return true;
			} else if (surfaceId != SURFTYPE_SPACE) {
				return true;
			}

			sound.playFX(21);
			return false;

		case 1:
		case 7:
		case 9:
		case 10:
		case 12:
			check:
			if (party.checkSkill(MOUNTAINEER))
				return true;

			sound.playFX(21);
			return false;

		default:
			break;
		}
	} else {
		int surfaceId = map.getCell(2);
		if (surfaceId >= map.mazeData()._difficulties._wallNoPass) {
			party._mazeDirection = dir;
		} else {
			party._mazeDirection = dir;

			if (startSurfaceId == SURFTYPE_SWAMP || party.checkSkill(SWIMMING) ||
					party._walkOnWaterActive) {
				sound.playFX(46);
				return false;
			} else {
				if (_buttonValue == 242 && _wo[107]) {
					_vm->_openDoor = true;
					sound.playFX(47);
					draw3d(true);
					_vm->_openDoor = false;
				}
				return true;
			}
		}
	}

	return true;
}

} // End of namespace Xeen
