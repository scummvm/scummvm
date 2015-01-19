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
	_batUIFrame = 0;
	_spotDoorsUIFrame = 0;
	_dangerSenseUIFrame = 0;
	_face1UIFrame = 0;
	_face2UIFrame = 0;
	_blessedUIFrame = 0;
	_powerShieldUIFrame = 0;
	_holyBonusUIFrame = 0;
	_heroismUIFrame = 0;
	_flipUIFrame = 0;
	_newDay = false;
	_buttonsLoaded = false;
	_hiliteChar = -1;
	_intrIndex1 = 0;
	_flag1 = false;
	_animCounter = 0;
	_isAnimReset = false;
	_tillMove = 0;
	_thinWall = false;
	_overallFrame = 0;

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
	_globalSprites.load("global.icn");
	_borderSprites.load("border.icn");
	_spellFxSprites.load("spellfx.icn");
	_fecpSprites.load("fecp.brd");
	_blessSprites.load("bless.icn");
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_uiSprites.load("inn.icn");
	_charPowSprites.load("charpow.icn");

	// Get mappings to the active characters in the party
	_vm->_party._activeParty.resize(_vm->_party._partyCount);
	for (int i = 0; i < _vm->_party._partyCount; ++i) {
		_vm->_party._activeParty[i] = _vm->_roster[_vm->_party._partyMembers[i]];
	}

	_newDay = _vm->_party._minutes >= 300;
}

void Interface::manageCharacters(bool soundPlayed) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	bool flag = false;

start:
	if (_vm->_party._mazeId != 0) {
		_vm->_mode = MODE_0;
		_buttonsLoaded = true;
	} else {
		if (!soundPlayed) {
			warning("TODO: loadSound?");
		}

		if (!_partyFaces[0]) {
			// Xeen only uses 24 of possible 30 character slots
			loadCharIcons();

			for (int i = 0; i < _vm->_party._partyCount; ++i)
				_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
		}

		_vm->_mode = MODE_1;
		Common::Array<int> xeenSideChars;

		// Build up a list of characters on the same Xeen side being loaded
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			PlayerStruct &player = _vm->_roster[i];
			if (player._name.empty() || player._xeenSide != _vm->_loadDarkSide)
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
				if (_vm->_party._partyCount == 0) {
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
					_vm->_party._realPartyCount = _vm->_party._partyCount;
					_vm->_party._mazeId = _vm->_party._priorMazeId;

					_vm->_party.copyPartyToRoster(_vm->_roster);
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
				if (_vm->_party._partyCount > 0) {
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
	_globalSprites.clear();
	_borderSprites.clear();
	_spellFxSprites.clear();
	_fecpSprites.clear();
	_blessSprites.clear();
	_restoreSprites.clear();
	_hpSprites.clear();
	_uiSprites.clear();
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
	for (int i = 0; i < _vm->_party._partyCount; ++i)
		_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
}

void Interface::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	assembleBorder();
}

void Interface::assembleBorder() {
	Screen &screen = *_vm->_screen;

	// Draw the outer frame
	_globalSprites.draw(screen._windows[0], 0, Common::Point(8, 8));

	// Draw the animating bat character used to show when levitate is active
	_borderSprites.draw(screen._windows[0], _vm->_party._levitateActive ? _batUIFrame + 16 : 16,
		Common::Point(0, 82));
	_batUIFrame = (_batUIFrame + 1) % 12;

	// Draw UI element to indicate whether can spot hidden doors
	_borderSprites.draw(screen,
		(_thinWall && _vm->_party.checkSkill(SPOT_DOORS)) ? _spotDoorsUIFrame + 28 : 28,
		Common::Point(194, 91));
	_spotDoorsUIFrame = (_spotDoorsUIFrame + 1) % 12;

	// Draw UI element to indicate whether can sense danger
	_borderSprites.draw(screen,
		(_vm->_dangerSenseAllowed && _vm->_party.checkSkill(DANGER_SENSE)) ? _spotDoorsUIFrame + 40 : 40,
		Common::Point(107, 9));
	_dangerSenseUIFrame = (_dangerSenseUIFrame + 1) % 12;

	// Handle the face UI elements for indicating clairvoyance status
	_face1UIFrame = (_face1UIFrame + 1) % 4;
	if (_vm->_face1State == 0)
		_face1UIFrame += 4;
	else if (_vm->_face1State == 2)
		_face1UIFrame = 0;

	_face2UIFrame = (_face2UIFrame + 1) % 4 + 12;
	if (_vm->_face2State == 0)
		_face2UIFrame += 252;
	else if (_vm->_face2State == 2)
		_face2UIFrame = 0;

	if (!_vm->_party._clairvoyanceActive) {
		_face1UIFrame = 0;
		_face2UIFrame = 8;
	}

	_borderSprites.draw(screen, _face1UIFrame, Common::Point(0, 32));
	_borderSprites.draw(screen,
		screen._windows[10]._enabled || screen._windows[2]._enabled ?
		52 : _face2UIFrame,
		Common::Point(215, 32));

	// Draw resistence indicators
	if (!screen._windows[10]._enabled && !screen._windows[2]._enabled
		&& screen._windows[38]._enabled) {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 1 : 0,
			Common::Point(2, 2));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 3 : 2,
			Common::Point(219, 2));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 5 : 4,
			Common::Point(2, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 7 : 6,
			Common::Point(219, 134));
	} else {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 9 : 8,
			Common::Point(8, 8));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 10 : 11,
			Common::Point(219, 8));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 12 : 13,
			Common::Point(8, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 14 : 15,
			Common::Point(219, 134));
	}

	// Draw UI element for blessed
	_blessSprites.draw(screen, 16, Common::Point(33, 137));
	if (_vm->_party._blessedActive) {
		_blessedUIFrame = (_blessedUIFrame + 1) % 4;
		_blessSprites.draw(screen, _blessedUIFrame, Common::Point(33, 137));
	}

	// Draw UI element for power shield
	if (_vm->_party._powerShieldActive) {
		_powerShieldUIFrame = (_powerShieldUIFrame + 1) % 4;
		_blessSprites.draw(screen, _powerShieldUIFrame + 4,
			Common::Point(55, 137));
	}

	// Draw UI element for holy bonus
	if (_vm->_party._holyBonusActive) {
		_holyBonusUIFrame = (_holyBonusUIFrame + 1) % 4;
		_blessSprites.draw(screen, _holyBonusUIFrame + 8, Common::Point(160, 137));
	}

	// Draw UI element for heroism
	if (_vm->_party._heroismActive) {
		_heroismUIFrame = (_heroismUIFrame + 1) % 4;
		_blessSprites.draw(screen, _heroismUIFrame + 12, Common::Point(182, 137));
	}

	// Draw direction character if direction sense is active
	if (_vm->_party.checkSkill(DIRECTION_SENSE) && !_vm->_noDirectionSense) {
		const char *dirText = DIRECTION_TEXT[_vm->_party._mazeDirection];
		Common::String msg = Common::String::format(
			"\002""08\003""c\013""139\011""116%c\014""d\001", *dirText);
		screen._windows[0].writeString(msg);
	}

	// Draw view frame
	if (screen._windows[12]._enabled)
		screen._windows[12].frame();
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
		bool isInParty = _vm->_party.isInParty(charId);

		if (charId == 0xff) {
			while ((int)_buttons.size() > (7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		PlayerStruct &ps = _vm->_roster[xeenSideChars[charIndex + posIndex]];
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
	for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount : 
			_vm->_party._partyCount); ++idx) {
		int charIndex = stateFlag ? _combatCharIds[idx] : idx;
		PlayerStruct &ps = _vm->_party._activeParty[charIndex];
		Condition charCondition = ps.worstCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];
		
		SpriteResource *sprites = (charFrame > 4 && !_charFaces[0].empty()) ?
			&_dseFace : _partyFaces[charIndex];
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(screen, charFrame, Common::Point(CHAR_FACES_X[idx], 150));
	}

	if (!_hpSprites.empty()) {
		for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount :
			_vm->_party._partyCount); ++idx) {
			int charIndex = stateFlag ? _combatCharIds[idx] : idx;
			PlayerStruct &ps = _vm->_party._activeParty[charIndex];

			// Draw the Hp bar
			int maxHp = ps.getMaxHp();
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

void Interface::draw3d(bool updateFlag) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Screen &screen = *_vm->_screen;

	if (screen._windows[11]._enabled)
		return;

	_flipUIFrame = (_flipUIFrame + 1) % 4;
	if (_flipUIFrame == 0)
		_flipWater = !_flipWater;
	if (_tillMove && (_vm->_mode == MODE_1 || _vm->_mode == MODE_2) &&
			!_flag1 && _vm->_moveMonsters) {
		if (--_tillMove == 0)
			moveMonsters();
	}

	MazeObject &objObject = map._mobData._objects[_objNumber];
	Direction partyDirection = _vm->_party._mazeDirection;
	int objNum = _objNumber - 1;

	// Loop to update the frame numbers for each maze object, applying the animation frame
	// limits as specified by the map's _animationInfo listing
	for (uint idx = 0; idx < map._mobData._objects.size(); ++idx) {
		MazeObject &mazeObject = map._mobData._objects[idx];
		AnimationEntry &animEntry = map._animationInfo[mazeObject._spriteId];
		int directionIndex = DIRECTION_ANIM_POSITIONS[mazeObject._direction][partyDirection];

		if (_isAnimReset) {
			mazeObject._frame = animEntry._frame1._frames[directionIndex];
		} else {
			++mazeObject._frame;
			if ((int)idx == objNum && _animCounter > 0 && (
					objObject._spriteId == (_vm->_files->_isDarkCc ? 15 : 16) ||
					objObject._spriteId == 58 || objObject._spriteId == 73)) {
				if (mazeObject._frame > 4 || mazeObject._spriteId == 58)
					mazeObject._frame = 1;
			} else if (mazeObject._frame >= animEntry._frame2._frames[directionIndex]) {
				mazeObject._frame = animEntry._frame1._frames[directionIndex];
			}
		}

		mazeObject._flipped = animEntry._flipped._flags[directionIndex];
	}

	if (map._isOutdoors) {
		error("TODO: draw3d outdoors handling");
	} else {
		// Default all the parts of draw struct not to be drawn by default
		for (int idx = 3; idx < _indoorList.size(); ++idx)
			_indoorList[idx]._frame = -1;

		if (_flag1) {
			for (int idx = 0; idx < 96; ++idx) {
				if (_indoorList[79 + idx]._sprites != nullptr) {
					_indoorList[79 + idx]._frame = 0;
				} else if (_indoorList[111 + idx]._sprites != nullptr) {
					_indoorList[111 + idx]._frame = 1;
				} else if (_indoorList[135 + idx]._sprites != nullptr) {
					_indoorList[135 + idx]._frame = 2;
				} else if (_indoorList[162 + idx]._sprites != nullptr) {
					_indoorList[162 + idx]._frame = 0;
				}
			}
		} else if (_charsShooting) {
			for (int idx = 0; idx < 96; ++idx) {
				if (_indoorList[162 + idx]._sprites != nullptr) {
					_indoorList[162 + idx]._frame = 0;
				} else if (_indoorList[135 + idx]._sprites != nullptr) {
					_indoorList[135 + idx]._frame = 1;
				} else if (_indoorList[111 + idx]._sprites != nullptr) {
					_indoorList[111 + idx]._frame = 2;
				} else if (_indoorList[79 + idx]._sprites != nullptr) {
					_indoorList[79 + idx]._frame = 0;
				}
			}
		}

		setMazeBits();
		_isAnimReset = false;
		const int INDOOR_INDEXES[3] = { 157, 151, 154 };
		const int INDOOR_COMBAT_POS[3][2] = { { 102, 134 }, { 36, 67 }, { 161, 161 } };
		const int INDOOR_COMBAT_POS2[4] = { 8, 6, 4, 2 };

		// Double check this, since it's not being used?
		//MazeObject &objObject = map._mobData._objects[_objNumber - 1];

		for (int idx = 0; idx < 3; ++idx) {
			DrawStruct &ds1 = _indoorList[INDOOR_INDEXES[idx]];
			DrawStruct &ds2 = _indoorList[INDOOR_INDEXES[idx] + 1];
			ds1._sprites = nullptr;
			ds2._sprites = nullptr;

			if (combat._charsArray1[idx]) {
				int posIndex= combat._attackMonsters[1] && !combat._attackMonsters[2] ? 1 : 0;
				--combat._charsArray1[idx];

				if (combat._monPow[idx]) {
					ds1._x = INDOOR_COMBAT_POS[idx][0];
					ds1._frame = 0;
					ds1._scale = combat._monsterScale[idx];
					if (ds1._scale == 0x8000) {
						ds1._x /= 3;
						ds1._y = 60;
					} else {
						ds1._y = 73;
					}

					ds1._flags = SPRFLAG_4000 | SPRFLAG_2000;
					ds1._sprites = &_charPowSprites;
				}

				if (combat._elemPow[idx]) {
					ds2._x = INDOOR_COMBAT_POS[idx][posIndex] + INDOOR_COMBAT_POS2[idx];
					ds2._frame = combat._elemPow[idx];
					ds2._scale = combat._elemScale[idx];
					if (ds2._scale == 0x8000)
						ds2._x /= 3;
					ds2._flags = SPRFLAG_4000 | SPRFLAG_2000;
					ds2._sprites = &_charPowSprites;
				}
			}
		}

		setIndoorsMonsters();
		setIndoorsObjects();
		setIndoorsWallPics();

		_indoorList[161]._sprites = nullptr;
		_indoorList[160]._sprites = nullptr;
		_indoorList[159]._sprites = nullptr;

		// Handle attacking monsters
		int monsterIndex = 0;
		if (combat._attackMonsters[0] != -1 && map._mobData._monsters[combat._attackMonsters[0]]._frame >= 0) {
			_indoorList[159] = _indoorList[156];
			_indoorList[160] = _indoorList[157];
			_indoorList[161] = _indoorList[158];
			_indoorList[158]._sprites = nullptr;
			_indoorList[156]._sprites = nullptr;
			_indoorList[157]._sprites = nullptr;
			monsterIndex = 1;
		} else if (combat._attackMonsters[1] != -1 && map._mobData._monsters[combat._attackMonsters[1]]._frame >= 0) {
			_indoorList[159] = _indoorList[150];
			_indoorList[160] = _indoorList[151];
			_indoorList[161] = _indoorList[152];
			_indoorList[152]._sprites = nullptr;
			_indoorList[151]._sprites = nullptr;
			_indoorList[150]._sprites = nullptr;
			monsterIndex = 2;
		} else if (combat._attackMonsters[2] != -1 &&  map._mobData._monsters[combat._attackMonsters[2]]._frame >= 0) {
			_indoorList[159] = _indoorList[153];
			_indoorList[160] = _indoorList[154];
			_indoorList[161] = _indoorList[155];
			_indoorList[153]._sprites = nullptr;
			_indoorList[154]._sprites = nullptr;
			_indoorList[155]._sprites = nullptr;
			monsterIndex = 3;
		}

		drawIndoors();

		switch (monsterIndex) {
		case 1:
			_indoorList[156] = _indoorList[159];
			_indoorList[157] = _indoorList[160];
			_indoorList[158] = _indoorList[161];
			break;
		case 2:
			_indoorList[150] = _indoorList[159];
			_indoorList[151] = _indoorList[160];
			_indoorList[152] = _indoorList[161];
			break;
		case 3:
			_indoorList[153] = _indoorList[159];
			_indoorList[154] = _indoorList[160];
			_indoorList[155] = _indoorList[161];
			break;
		default:
			break;
		}
	}

	animate3d();
	updateAutoMap();

	if (_vm->_falling == 1) {
		error("TODO: Indoor falling");
	}

	if (_vm->_falling == 2) {
		screen.saveBackground(1);
	}

	assembleBorder();

	// TODO: write strings

	if (updateFlag) {
		screen._windows[1].update();
		screen._windows[3].update();
	}

	// TODO: more stuff

	events.wait(2);
}

void Interface::animate3d() {

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

void Interface::moveMonsters() {

}

void Interface::setMainButtons() {
	clearButtons();

	addButton(Common::Rect(235,  75, 259,  95),  83, &_iconSprites);
	addButton(Common::Rect(260,  75, 284,  95),  67, &_iconSprites);
	addButton(Common::Rect(286,  75, 310,  95),  82, &_iconSprites);
	addButton(Common::Rect(235,  96, 259, 116),  66, &_iconSprites);
	addButton(Common::Rect(260,  96, 284, 116),  68, &_iconSprites);
	addButton(Common::Rect(286,  96, 310, 116),  86, &_iconSprites);
	addButton(Common::Rect(235, 117, 259, 137),  77, &_iconSprites);
	addButton(Common::Rect(260, 117, 284, 137),  73, &_iconSprites);
	addButton(Common::Rect(286, 117, 310, 137),  81, &_iconSprites);
	addButton(Common::Rect(109, 137, 122, 147),   9, &_iconSprites);
	addButton(Common::Rect(235, 148, 259, 168), 240, &_iconSprites);
	addButton(Common::Rect(260, 148, 284, 168), 242, &_iconSprites);
	addButton(Common::Rect(286, 148, 310, 168), 241, &_iconSprites);
	addButton(Common::Rect(235, 169, 259, 189), 176, &_iconSprites);
	addButton(Common::Rect(260, 169, 284, 189), 243, &_iconSprites);
	addButton(Common::Rect(286, 169, 310, 189), 177, &_iconSprites);
	addButton(Common::Rect(236,  11, 308,  69),  61, &_iconSprites, false);
	addButton(Common::Rect(239,  27, 312,  37),  49, &_iconSprites, false);
	addButton(Common::Rect(239,  37, 312,  47),  50, &_iconSprites, false);
	addButton(Common::Rect(239,  47, 312,  57),  51, &_iconSprites, false);
}

void Interface::setMazeBits() {
	Common::fill(&_wo[0], &_wo[308], 0);

	switch (_vm->_map->getCell(0) - 1) {
	case 0:
		++_wo[125];
		break;
	case 1:
		++_wo[69];
		break;
	case 2:
		++_wo[185];
		break;
	case 3:
	case 12:
		++_wo[105];
		break;
	case 4:
	case 7:
		++_wo[25];
		break;
	case 5:
		++_wo[225];
		break;
	case 6:
		++_wo[205];
		break;
	case 8:
		++_wo[145];
		break;
	case 9:
		++_wo[305];
		break;
	case 10:
		++_wo[245];
		break;
	case 11:
		++_wo[165];
		break;
	case 13:
		++_wo[265];
		break;
	case 14:
		++_wo[285];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(1) - 1) {
	case 1:
		++_wo[72];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[28];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(2) - 1) {
	case 0:
		++_wo[127];
		break;
	case 1:
		++_wo[71];
		break;
	case 2:
		++_wo[187];
		break;
	case 3:
	case 12:
		++_wo[107];
		break;
	case 4:
	case 7:
		++_wo[27];
		break;
	case 5:
		++_wo[227];
		break;
	case 6:
		++_wo[207];
		break;
	case 8:
		++_wo[147];
		break;
	case 9:
		++_wo[307];
		break;
	case 10:
		++_wo[247];
		break;
	case 11:
		++_wo[167];
		break;
	case 13:
		++_wo[267];
		break;
	case 14:
		++_wo[287];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(3) - 1) {
	case 1:
		++_wo[73];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[29];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(4) - 1) {
	case 0:
		++_wo[126];
		break;
	case 1:
		++_wo[70];
		break;
	case 2:
		++_wo[186];
		break;
	case 3:
	case 12:
		++_wo[106];
		break;
	case 4:
	case 7:
		++_wo[26];
		break;
	case 5:
		++_wo[226];
		break;
	case 6:
		++_wo[206];
	case 8:
		++_wo[146];
		break;
	case 9:
		++_wo[306];
		break;
	case 10:
		++_wo[246];
		break;
		break;
	case 11:
		++_wo[166];
		break;
	case 13:
		++_wo[266];
		break;
	case 14:
		++_wo[286];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(5) - 1) {
	case 0:
		++_wo[122];
		break;
	case 1:
		++_wo[64];
		break;
	case 2:
		++_wo[182];
		break;
	case 3:
	case 12:
		++_wo[102];
		break;
	case 5:
		++_wo[222];
		break;
	case 6:
		++_wo[202];
		break;
	case 8:
		++_wo[142];
		break;
	case 9:
		++_wo[302];
		break;
	case 10:
		++_wo[242];
		break;
	case 11:
		++_wo[162];
		break;
	case 13:
		++_wo[262];
		break;
	case 14:
		++_wo[282];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(6) - 1) {
	case 1:
		++_wo[67];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[23];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(7) - 1) {
	case 0:
		++_wo[124];
		break;
	case 1:
		++_wo[66];
		break;
	case 2:
		++_wo[184];
		break;
	case 3:
	case 12:
		++_wo[104];
		break;
	case 4:
	case 7:
		++_wo[22];
		break;
	case 5:
		++_wo[224];
		break;
	case 6:
		++_wo[204];
		break;
	case 8:
		++_wo[144];
		break;
	case 9:
		++_wo[304];
		break;
	case 10:
		++_wo[244];
		break;
	case 11:
		++_wo[164];
		break;
	case 13:
		++_wo[264];
		break;
	case 14:
		++_wo[284];
		break;
	default:
		break;
	}

	_thinWall = (_vm->_map->_currentWall._data != 0x8888) && _wo[27];

	switch (_vm->_map->getCell(8) - 1) {
	case 1:
		++_wo[68];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[24];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(9) - 1) {
	case 0:
		++_wo[123];
		break;
	case 1:
		++_wo[65];
		break;
	case 2:
		++_wo[183];
		break;
	case 3:
	case 12:
		++_wo[103];
		break;
	case 4:
	case 7:
		++_wo[21];
		break;
	case 5:
		++_wo[223];
		break;
	case 6:
		++_wo[203];
		break;
	case 8:
		++_wo[143];
		break;
	case 9:
		++_wo[3033];
		break;
	case 10:
		++_wo[243];
		break;
	case 11:
		++_wo[163];
		break;
	case 13:
		++_wo[263];
		break;
	case 14:
		++_wo[283];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(10) - 1) {
	case 0:
		++_wo[117];
		break;
	case 1:
		++_wo[55];
		break;
	case 2:
		++_wo[177];
		break;
	case 3:
	case 12:
		++_wo[97];
		break;
	case 4:
	case 7:
		++_wo[11];
		break;
	case 5:
		++_wo[217];
		break;
	case 6:
		++_wo[197];
		break;
	case 8:
		++_wo[137];
		break;
	case 9:
		++_wo[297];
		break;
	case 10:
		++_wo[237];
	case 11:
		++_wo[157];
		break;
	case 13:
		++_wo[257];
		break;
	case 14:
		++_wo[277];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(11) - 1) {
	case 1:
		++_wo[60];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[16];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(12) - 1) {
	case 0:
		++_wo[118];
		break;
	case 1:
		++_wo[56];
		break;
	case 2:
		++_wo[178];
		break;
	case 3:
	case 12:
		++_wo[98];
		break;
	case 4:
	case 7:
		++_wo[12];
		break;
	case 5:
		++_wo[218];
		break;
	case 6:
		++_wo[198];
		break;
	case 8:
		++_wo[138];
		break;
	case 9:
		++_wo[298];
		break;
	case 10:
		++_wo[238];
		break;
	case 11:
		++_wo[158];
		break;
	case 13:
		++_wo[258];
		break;
	case 14:
		++_wo[278];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(13) - 1) {
	case 1:
		++_wo[61];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[17];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(14) - 1) {
	case 0:
		++_wo[121];
		break;
	case 1:
		++_wo[59];
		break;
	case 2:
		++_wo[181];
		break;
	case 3:
	case 12:
		++_wo[101];
		break;
	case 4:
	case 7:
		++_wo[15];
		break;
	case 5:
		++_wo[221];
		break;
	case 6:
		++_wo[201];
		break;
	case 8:
		++_wo[141];
		break;
	case 9:
		++_wo[301];
		break;
	case 10:
		++_wo[241];
		break;
	case 11:
		++_wo[161];
		break;
	case 13:
		++_wo[261];
		break;
	case 14:
		++_wo[281];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(15) - 1) {
	case 1:
		++_wo[63];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[19];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(16) - 1) {
	case 0:
		++_wo[120];
		break;
	case 1:
		++_wo[58];
		break;
	case 2:
		++_wo[180];
		break;
	case 3:
	case 12:
		++_wo[100];
		break;
	case 4:
	case 7:
		++_wo[14];
		break;
	case 5:
		++_wo[220];
		break;
	case 6:
		++_wo[200];
		break;
	case 8:
		++_wo[140];
		break;
	case 9:
		++_wo[300];
		break;
	case 10:
		++_wo[240];
		break;
	case 11:
		++_wo[160];
		break;
	case 13:
		++_wo[260];
		break;
	case 14:
		++_wo[280];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(17) - 1) {
	case 1:
		++_wo[62];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[18];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(18) - 1) {
	case 0:
		++_wo[119];
		break;
	case 1:
		++_wo[57];
		break;
	case 2:
		++_wo[179];
		break;
	case 3:
	case 12:
		++_wo[99];
		break;
	case 4:
	case 7:
		++_wo[13];
		break;
	case 5:
		++_wo[219];
		break;
	case 6:
		++_wo[199];
		break;
	case 8:
		++_wo[139];
		break;
	case 9:
		++_wo[299];
		break;
	case 10:
		++_wo[239];
		break;
	case 11:
		++_wo[159];
		break;
	case 13:
		++_wo[259];
		break;
	case 14:
		++_wo[279];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(19) - 1) {
	case 0:
		++_wo[108];
		break;
	case 1:
		++_wo[78];
		break;
	case 2:
		++_wo[168];
	case 3:
	case 12:
		++_wo[88];
		break;
	case 4:
	case 7:
		++_wo[34];
		break;
	case 5:
		++_wo[208];
		break;
	case 6:
		++_wo[188];
		break;
	case 8:
		++_wo[128];
		break;
	case 9:
		++_wo[288];
		break;
	case 10:
		++_wo[228];
		break;
	case 11:
		++_wo[148];
		break;
	case 13:
		++_wo[248];
		break;
	case 14:
		++_wo[268];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(20) - 1) {
	case 1:
		++_wo[76];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[32];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(21) - 1) {
	case 0:
		++_wo[109];
		break;
	case 1:
		++_wo[44];
		break;
	case 2:
		++_wo[169];
		break;
	case 3:
	case 12:
		++_wo[89];
		break;
	case 4:
	case 7:
		++_wo[0];
		break;
	case 5:
		++_wo[209];
		break;
	case 6:
		++_wo[189];
		break;
	case 8:
		++_wo[129];
		break;
	case 9:
		++_wo[289];
		break;
	case 10:
		++_wo[229];
		break;
	case 11:
		++_wo[149];
		break;
	case 13:
		++_wo[249];
		break;
	case 14:
		++_wo[269];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(22) - 1) {
	case 1:
		++_wo[74];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[30];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(23) - 1) {
	case 0:
		++_wo[110];
		break;
	case 1:
		++_wo[45];
		break;
	case 2:
		++_wo[170];
		break;
	case 3:
	case 12:
		++_wo[90];
		break;
	case 4:
	case 7:
		++_wo[1];
		break;
	case 5:
		++_wo[210];
		break;
	case 6:
		++_wo[190];
		break;
	case 8:
		++_wo[130];
		break;
	case 9:
		++_wo[290];
		break;
	case 10:
		++_wo[230];
		break;
	case 11:
		++_wo[150];
		break;
	case 13:
		++_wo[250];
		break;
	case 14:
		++_wo[270];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(24) - 1) {
	case 1:
		++_wo[52];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[8];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(25) - 1) {
	case 0:
		++_wo[111];
		break;
	case 1:
		++_wo[46];
		break;
	case 2:
		++_wo[171];
		break;
	case 3:
	case 12:
		++_wo[91];
		break;
	case 4:
	case 7:
		++_wo[2];
		break;
	case 5:
		++_wo[211];
		break;
	case 6:
		++_wo[191];
		break;
	case 8:
		++_wo[131];
		break;
	case 9:
		++_wo[291];
		break;
	case 10:
		++_wo[231];
		break;
	case 11:
		++_wo[151];
		break;
	case 13:
		++_wo[251];
		break;
	case 14:
		++_wo[271];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(26) - 1) {
	case 1:
		++_wo[51];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[7];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(27) - 1) {
	case 0:
		++_wo[116];
		break;
	case 1:
		++_wo[50];
		break;
	case 2:
		++_wo[176];
		break;
	case 3:
	case 12:
		++_wo[96];
		break;
	case 4:
	case 7:
		++_wo[6];
		break;
	case 5:
		++_wo[216];
		break;
	case 6:
		++_wo[196];
		break;
	case 8:
		++_wo[136];
		break;
	case 9:
		++_wo[296];
		break;
	case 10:
		++_wo[236];
		break;
	case 11:
		++_wo[156];
		break;
	case 13:
		++_wo[256];
		break;
	case 14:
		++_wo[276];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(28) - 1) {
	case 1:
		++_wo[53];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[9];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(29) - 1) {
	case 0:
		++_wo[115];
		break;
	case 1:
		++_wo[49];
		break;
	case 2:
		++_wo[175];
		break;
	case 3:
	case 12:
		++_wo[95];
		break;
	case 4:
	case 7:
		++_wo[5];
		break;
	case 5:
		++_wo[215];
		break;
	case 6:
		++_wo[195];
		break;
	case 8:
		++_wo[135];
		break;
	case 9:
		++_wo[295];
		break;
	case 10:
		++_wo[235];
		break;
	case 11:
		++_wo[155];
		break;
	case 13:
		++_wo[255];
		break;
	case 14:
		++_wo[275];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(30) - 1) {
	case 1:
		++_wo[54];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[10];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(31) - 1) {
	case 0:
		++_wo[114];
		break;
	case 1:
		++_wo[48];
		break;
	case 2:
		++_wo[174];
		break;
	case 3:
	case 12:
		++_wo[94];
		break;
	case 4:
		++_wo[4];
		break;
	case 5:
		++_wo[214];
		break;
	case 6:
		++_wo[194];
		break;
	case 8:
		++_wo[134];
		break;
	case 9:
		++_wo[294];
		break;
	case 10:
		++_wo[234];
		break;
	case 11:
		++_wo[154];
		break;
	case 13:
		++_wo[254];
		break;
	case 14:
		++_wo[274];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(32) - 1) {
	case 1:
		++_wo[75];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[31];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(33) - 1) {
	case 0:
		++_wo[112];
		break;
	case 1:
		++_wo[47];
		break;
	case 2:
		++_wo[172];
		break;
	case 3:
	case 12:
		++_wo[92];
		break;
	case 4:
	case 7:
		++_wo[3];
		break;
	case 5:
		++_wo[212];
		break;
	case 6:
		++_wo[192];
		break;
	case 8:
		++_wo[132];
		break;
	case 9:
		++_wo[292];
		break;
	case 10:
		++_wo[232];
		break;
	case 11:
		++_wo[152];
		break;
	case 13:
		++_wo[252];
		break;
	case 14:
		++_wo[272];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(34) - 1) {
	case 1:
		++_wo[77];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[33];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(35) - 1) {
	case 0:
		++_wo[113];
		break;
	case 1:
		++_wo[79];
		break;
	case 2:
		++_wo[173];
		break;
	case 3:
	case 12:
		++_wo[93];
		break;
	case 4:
	case 7:
		++_wo[35];
		break;
	case 5:
		++_wo[213];
		break;
	case 6:
		++_wo[193];
		break;
	case 8:
		++_wo[133];
		break;
	case 9:
		++_wo[293];
		break;
	case 10:
		++_wo[233];
		break;
	case 11:
		++_wo[153];
		break;
	case 13:
		++_wo[253];
		break;
	case 14:
		++_wo[273];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(36) - 1) {
	case 1:
		++_wo[83];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[39];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(37) - 1) {
	case 1:
		++_wo[82];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[38];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(38) - 1) {
	case 1:
		++_wo[81];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[37];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(34) - 1) {
	case 1:
		++_wo[80];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[36];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(40) - 1) {
	case 1:
		++_wo[84];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[40];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(41) - 1) {
	case 1:
		++_wo[85];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[41];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(42) - 1) {
	case 1:
		++_wo[86];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[42];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(43) - 1) {
	case 1:
		++_wo[87];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[43];
		break;
	default:
		break;
	}
}

void Interface::updateAutoMap() {
	// TODO
}

} // End of namespace Xeen
