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
#include "xeen/dialogs_char_info.h"
#include "xeen/dialogs_error.h"
#include "xeen/dialogs_automap.h"
#include "xeen/dialogs_info.h"
#include "xeen/dialogs_query.h"
#include "xeen/dialogs_quests.h"
#include "xeen/dialogs_quick_ref.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

#include "xeen/dialogs_party.h"

namespace Xeen {

PartyDrawer::PartyDrawer(XeenEngine *vm): _vm(vm) {
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_dseFace.load("dse.fac");
	_hiliteChar = -1;
}

void PartyDrawer::drawParty(bool updateFlag) {
	Party &party = *_vm->_party;
	Resources &res = *_vm->_resources;
	Screen &screen = *_vm->_screen;
	bool inCombat = _vm->_mode == MODE_COMBAT;
	_restoreSprites.draw(screen, 0, Common::Point(8, 149));

	// Handle drawing the party faces
	uint partyCount = inCombat ? party._combatParty.size() : party._activeParty.size();
	for (uint idx = 0; idx < partyCount; ++idx) {
		Character &ps = inCombat ? *party._combatParty[idx] : party._activeParty[idx];
		Condition charCondition = ps.worstCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];
		
		SpriteResource *sprites = (charFrame > 4) ? &_dseFace : ps._faceSprites;
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(screen, charFrame, Common::Point(CHAR_FACES_X[idx], 150));
	}

	for (uint idx = 0; idx < partyCount; ++idx) {
		Character &ps = inCombat ? *party._combatParty[idx] : party._activeParty[idx];

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

	if (_hiliteChar != -1)
		res._globalSprites.draw(screen, 8, Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));

	if (updateFlag)
		screen._windows[33].update();
}

void PartyDrawer::highlightChar(int charId) {
	Resources &res = *_vm->_resources;
	Screen &screen = *_vm->_screen;

	if (charId != _hiliteChar && _hiliteChar != HILIGHT_CHAR_DISABLED) {
		// Handle deselecting any previusly selected char
		if (_hiliteChar != -1) {
			res._globalSprites.draw(screen, 9 + _hiliteChar,
				Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));
		}

		// Highlight new character
		res._globalSprites.draw(screen, 8, Common::Point(CHAR_FACES_X[charId] - 1, 149));
		_hiliteChar = charId;
		screen._windows[33].update();
	}
}

void PartyDrawer::unhighlightChar() {
	Resources &res = *_vm->_resources;
	Screen &screen = *_vm->_screen;

	if (_hiliteChar != -1) {
		res._globalSprites.draw(screen, _hiliteChar + 9,
			Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));
		_hiliteChar = -1;
		screen._windows[33].update();
	}
}

/*------------------------------------------------------------------------*/

Interface::Interface(XeenEngine *vm) : ButtonContainer(), InterfaceMap(vm), 
		PartyDrawer(vm), _vm(vm) {
	_buttonsLoaded = false;
	_intrIndex1 = 0;
	_steppingFX = 0;

	Common::fill(&_combatCharIds[0], &_combatCharIds[8], 0);
	initDrawStructs();
}


void Interface::initDrawStructs() {
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
	_uiSprites.load("inn.icn");

	Party &party = *_vm->_party;
	party.loadActiveParty();
	party._newDay = party._minutes >= 300;
}

void Interface::startup() {
	Resources &res = *_vm->_resources;
	Screen &screen = *_vm->_screen;
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

	res._globalSprites.draw(screen._windows[1], 5, Common::Point(232, 9));
	drawParty(false);

	_mainList[0]._sprites = &res._globalSprites;
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
	addPartyButtons(_vm);
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

	case Common::KEYCODE_F1:
	case Common::KEYCODE_F2:
	case Common::KEYCODE_F3:
	case Common::KEYCODE_F4:
	case Common::KEYCODE_F5:
	case Common::KEYCODE_F6:
		_buttonValue -= Common::KEYCODE_F1;
		if (_buttonValue < (int)party._activeParty.size()) {
			CharacterInfo::show(_vm, _buttonValue);
			if (party._stepped)
				moveMonsters();
		}
		break;

	case Common::KEYCODE_EQUALS:
	case Common::KEYCODE_KP_EQUALS:
		// Toggle minimap
		_vm->_moveMonsters = false;
		party._automapOn = !party._automapOn;
		_vm->_moveMonsters = true;
		break;

	case Common::KEYCODE_b:
		chargeStep();
		
		if (map.getCell(2) < map.mazeData()._difficulties._wallNoPass
				&& !map._isOutdoors) {
			switch (party._mazeDirection) {
			case DIR_NORTH:
				++party._mazePosition.y;
				break;
			case DIR_EAST:
				++party._mazePosition.x;
				break;
			case DIR_SOUTH:
				--party._mazePosition.y;
				break;
			case DIR_WEST:
				--party._mazePosition.x;
				break;
			}
			chargeStep();
			stepTime();
		} else {
			bash(party._mazePosition, party._mazeDirection);
		}
		break;

	case Common::KEYCODE_i:
		// Show Info dialog
		_vm->_moveMonsters = false;
		InfoDialog::show(_vm);
		_vm->_moveMonsters = true;
		break;

	case Common::KEYCODE_m:
		// Show map dialog
		AutoMapDialog::show(_vm);
		break;

	case Common::KEYCODE_q:
		// Show the quick reference dialog
		QuickReferenceDialog::show(_vm);
		break;

	case Common::KEYCODE_r:
		// Rest
		rest();
		break;

	case Common::KEYCODE_v:
		// Show the quests dialog
		Quests::show(_vm);
		break;

	case Common::KEYCODE_x:
		// ****DEBUG***
		PartyDialog::show(_vm); //***DEBUG****
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

		switch (map._currentWall) {
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
				if (_buttonValue == Common::KEYCODE_UP && _wo[107]) {
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

void Interface::rest() {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;

	map.cellFlagLookup(party._mazePosition);

	if ((map._currentCantRest || (map.mazeData()._mazeFlags & RESTRICTION_REST))
			&& _vm->_mode != MODE_12) {
		ErrorScroll::show(_vm, TOO_DANGEROUS_TO_REST, WT_NONFREEZED_WAIT);
	} else {
		// Check whether any character is in danger of dying
		bool dangerFlag = false;
		for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
			for (int attrib = MIGHT; attrib <= LUCK; ++attrib) {
				if (party._activeParty[charIdx].getStat((Attribute)attrib) < 1)
					dangerFlag = true;
			}
		}

		if (dangerFlag) {
			if (!Confirm::show(_vm, SOME_CHARS_MAY_DIE))
				return;
		}

		// Mark all the players as being asleep
		for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
			party._activeParty[charIdx]._conditions[ASLEEP] = 1;
		}
		drawParty(true);

		Mode oldMode = _vm->_mode;
		_vm->_mode = MODE_SLEEPING;

		if (oldMode == MODE_12) {
			party.changeTime(8 * 60);
		} else {
			for (int idx = 0; idx < 10; ++idx) {
				chargeStep();
				draw3d(true);
				
				if (_vm->_mode == MODE_1) {
					_vm->_mode = oldMode;
					return;
				}
			}

			party.changeTime(map._isOutdoors ? 380 : 470);
		}

		if (_vm->getRandomNumber(1, 20) == 1) {
			// Show dream
			screen.saveBackground();
			screen.fadeOut(4);
			events.hideCursor();

			screen.loadBackground("scene1.raw");
			screen._windows[0].update();
			screen.fadeIn(4);

			events.updateGameCounter();
			while (!_vm->shouldQuit() && events.timeElapsed() < 7)
				events.pollEventsAndWait();

			File f("dreams2.voc");
			sound.playSample(&f, 1);
			while (!_vm->shouldQuit() && sound.playSample(1, 0))
				events.pollEventsAndWait();
			f.close();

			f.openFile("laff1.voc");
			sound.playSample(&f, 1);
			while (!_vm->shouldQuit() && sound.playSample(1, 0))
				events.pollEventsAndWait();
			f.close();

			events.updateGameCounter();
			while (!_vm->shouldQuit() && events.timeElapsed() < 7)
				events.pollEventsAndWait();

			screen.fadeOut(4);
			events.setCursor(0);
			screen.restoreBackground();
			screen._windows[0].update();

			screen.fadeIn(4);
		}

		party.resetTemps();

		// Wake up the party
		bool starving = false;
		int foodConsumed = 0;
		for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
			Character &c = party._activeParty[charIdx];
			c._conditions[ASLEEP] = 0;

			if (party._food == 0) {
				starving = true;
			} else {
				party._rested = true;
				Condition condition = c.worstCondition();

				if (condition < DEAD || condition > ERADICATED) {
					--party._food;
					++foodConsumed;
					party._heroism = 0;
					party._holyBonus = 0;
					party._powerShield = 0;
					party._blessed = 0;
					c._conditions[UNCONSCIOUS] = 0;
					c._currentHp = c.getMaxHP();
					c._currentSp = c.getMaxSP();
				}
			}
		}

		drawParty(true);
		_vm->_mode = oldMode;
		doStepCode();
		draw3d(true);

		ErrorScroll::show(_vm, Common::String::format(REST_COMPLETE,
			starving ? PARTY_IS_STARVING : HIT_SPELL_POINTS_RESTORED,
			foodConsumed));
		party.checkPartyDead();
	}
}

void Interface::bash(const Common::Point &pt, Direction direction) {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;

	if (map._isOutdoors)
		return;

	sound.playFX(31);

	uint charNum1 = 0, charNum2 = 0;
	for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
		Character &c = party._activeParty[charIdx];
		Condition condition = c.worstCondition();

		if (!(condition == ASLEEP || (condition >= PARALYZED &&
				condition <= ERADICATED))) {
			if (charNum1) {
				charNum2 = charIdx + 1;
				break;
			} else {
				charNum1 = charIdx + 1;
			}
		}
	}

	party._activeParty[charNum1 - 1].subtractHitPoints(2);
	_charPowSprites.draw(screen._windows[0], 0, 
		Common::Point(CHAR_FACES_X[charNum1 - 1], 150));
	screen._windows[0].update();

	if (charNum2) {
		party._activeParty[charNum2 - 1].subtractHitPoints(2);
		_charPowSprites.draw(screen._windows[0], 0,
			Common::Point(CHAR_FACES_X[charNum2 - 1], 150));
		screen._windows[0].update();
	}

	int cell = map.mazeLookup(Common::Point(pt.x + SCREEN_POSITIONING_X[direction][7],
		pt.y + SCREEN_POSITIONING_Y[direction][7]), 0, 0xffff);
	if (cell != INVALID_CELL) {
		int v = map.getCell(2);

		if (v == 7) {
			++_wo[207];
			++_wo[267];
			++_wo[287];
		} else if (v == 14) {
			++_wo[267];
			++_wo[287];
		} else if (v == 15) {
			++_wo[287];
		} else {
			int might = party._activeParty[charNum1 - 1].getStat(MIGHT) +
				_vm->getRandomNumber(1, 30);
			if (charNum2)
				might += party._activeParty[charNum2 - 1].getStat(MIGHT);

			int bashThreshold = (v == 9) ? map.mazeData()._difficulties._bashGrate :
				map.mazeData()._difficulties._bashWall;
			if (might >= bashThreshold) {
				// Remove the wall on the current cell, and the reverse wall
				// on the cell we're bashing through to
				map.setWall(pt, direction, 3);
				switch (direction) {
				case DIR_NORTH:
					map.setWall(Common::Point(pt.x, pt.y + 1), DIR_SOUTH, 3);
					break;
				case DIR_EAST:
					map.setWall(Common::Point(pt.x + 1, pt.y), DIR_WEST, 3);
					break;
				case DIR_SOUTH:
					map.setWall(Common::Point(pt.x, pt.y - 1), DIR_NORTH, 3);
					break;
				case DIR_WEST:
					map.setWall(Common::Point(pt.x - 1, pt.y), DIR_EAST, 3);
					break;
				}
			}
		}
	}

	party.checkPartyDead();
	events.ipause(2);
	drawParty(true);
}

void Interface::draw3d(bool updateFlag) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	
	if (screen._windows[11]._enabled)
		return;

	// Draw the map
	drawMap(updateFlag);

	// Draw the minimap
	drawMiniMap();

	if (party._falling == 1)
		handleFalling();

	if (party._falling == 2) {
		screen.saveBackground(1);
	}

	assembleBorder();

	// Draw any on-screen text if flagged to do so
	if (_upDoorText && combat._attackMonsters[0] == -1) {
		screen._windows[3].writeString(_screenText);
	}

	if (updateFlag) {
		screen._windows[1].update();
		screen._windows[3].update();
	}

	// TODO: more stuff

	_vm->_party->_stepped = false;
	if (_vm->_mode == MODE_9) {
		// TODO
	}

	// TODO: Check use of updateFlag here. Original doesn't have it, but I
	// wanted to ensure in places like the AutoMapDialog, that the draw3d
	// doesn't result in the screen updating until the dialog has had
	// a chance to full render itself
	if (updateFlag)
		events.wait(2);
}

void Interface::handleFalling() {
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;
	Window &w = screen._windows[3];
	File voc1("scream.voc");
	File voc2("unnh.voc");
	saveFall();

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		party._activeParty[idx]._faceSprites->draw(screen._windows[0], 4,
			Common::Point(CHAR_FACES_X[idx], 150));
	}

	screen._windows[33].update();
	sound.playFX(11);
	sound.playSample(&voc1, 0);

	for (int idx = 0, incr = 2; idx < 133; ++incr, idx += incr) {
		fall(idx);
		assembleBorder();
		w.update();
	}

	fall(132);
	assembleBorder();
	w.update();

	sound.playSample(nullptr, 0);
	sound.playSample(&voc2, 0);
	sound.playFX(31);

	fall(127);
	assembleBorder();
	w.update();

	fall(132);
	assembleBorder();
	w.update();

	fall(129);
	assembleBorder();
	w.update();

	fall(132);
	assembleBorder();
	w.update();

	shake();
}

void Interface::saveFall() {

}

void Interface::fall(int v) {

}

void Interface::shake() {

}

} // End of namespace Xeen
