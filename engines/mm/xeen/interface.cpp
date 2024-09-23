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

#include "mm/xeen/interface.h"
#include "mm/xeen/dialogs/dialogs_char_info.h"
#include "mm/xeen/dialogs/dialogs_control_panel.h"
#include "mm/xeen/dialogs/dialogs_dismiss.h"
#include "mm/xeen/dialogs/dialogs_message.h"
#include "mm/xeen/dialogs/dialogs_quick_fight.h"
#include "mm/xeen/dialogs/dialogs_info.h"
#include "mm/xeen/dialogs/dialogs_items.h"
#include "mm/xeen/dialogs/dialogs_map.h"
#include "mm/xeen/dialogs/dialogs_query.h"
#include "mm/xeen/dialogs/dialogs_quests.h"
#include "mm/xeen/dialogs/dialogs_quick_ref.h"
#include "mm/xeen/dialogs/dialogs_spells.h"
#include "mm/xeen/resources.h"
#include "mm/xeen/xeen.h"

#include "mm/xeen/dialogs/dialogs_party.h"

namespace MM {
namespace Xeen {

enum {
	SCENE_WINDOW = 11, SCENE_WIDTH = 216, SCENE_HEIGHT = 132
};

PartyDrawer::PartyDrawer(XeenEngine *vm): _vm(vm) {
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_dseFace.load("dse.fac");
	_hiliteChar = HILIGHT_CHAR_NONE;
}

static inline int clipToFaceWidth(int i) {
	const int faceWidth = 32;
	return CLIP(i, 0, faceWidth);
}

static inline int unzero(int i) {
	if (i) return i;
	return 1;
}

void PartyDrawer::drawHitPoints(int charIndex) {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	bool inCombat = _vm->_mode == MODE_COMBAT;
	Window &win = windows[0];
	Character &c = inCombat ? *combat._combatParty[charIndex] : party._activeParty[charIndex];

	enum {
		BLACK = 0,
		STONE = 20,

		BRIGHT_GREEN = 80,
		GREEN = 85,
		YELLOW = 55,
		RED = 185,
		DARK_BLUE = 78,

		BRIGHT_BLUE = 70,
		BLUE = 75
	};

	int hp = c._currentHp;
	int sp = c._currentSp;

	int maxHp = c.getMaxHP();
	int maxSp =  c.getMaxSP();

	int gemFrame;
	int hpColor = GREEN;

	if (hp < 1) {
		gemFrame = 4;
	} else if (hp > maxHp) {
		gemFrame = 3;
	} else if (hp == maxHp) {
		gemFrame = 0;
	} else if (hp < (maxHp / 4)) {
		gemFrame = 2;
		hpColor = RED;
	} else{
		hpColor = YELLOW;
		gemFrame = 1;
	}
	if (!g_vm->_extOptions._showHpSpBars) {
		_hpSprites.draw(0, gemFrame, Common::Point(Res.HP_BARS_X[charIndex], 182));
	} else {
		const int faceWidth = 32;

		const int barLeft = Res.CHAR_FACES_X[charIndex];
		const int barRight = Res.CHAR_FACES_X[charIndex] + 32;
		const int barHeight = 3;

		const int frameTop = 183;
		const int frameBottom = 190;
		const int frameColor = STONE;
		const int hpBarTop = 184;
		const int hpBarBottom = hpBarTop + barHeight;

		const int spBarTop = 188;
		const int spBarBottom = spBarTop + barHeight;

		int hpPart = clipToFaceWidth( (hp * faceWidth) / unzero(maxHp) );
		int boostedHpPart = 0;
		if (hp > maxHp) {
			boostedHpPart = clipToFaceWidth(faceWidth * (hp - maxHp) / unzero(hp + maxHp) );
		}

		int spPart = clipToFaceWidth( (sp * faceWidth) / unzero(maxSp) );
		int boostedSpPart = 0;
		if (sp > maxSp) {
			boostedSpPart = clipToFaceWidth( faceWidth * (sp - maxSp) / unzero(sp + maxSp) );
		}

		int negativeHpPart = 32;
		if (hp < 0) {
			negativeHpPart = faceWidth - clipToFaceWidth((-hp * faceWidth) / unzero(maxHp));
		}

		win.fillRect(Common::Rect(barLeft, frameTop, barRight, frameBottom), frameColor);

		win.fillRect(Common::Rect(barLeft, hpBarTop, barRight, hpBarBottom), BLACK);
		win.fillRect(Common::Rect(barLeft, hpBarTop, barLeft + hpPart, hpBarBottom), hpColor);
		if (boostedHpPart != 0) {
			win.fillRect(Common::Rect(barLeft, hpBarTop, barLeft + boostedHpPart, hpBarBottom), BRIGHT_GREEN);
		}

		if (negativeHpPart != 32) {
			win.fillRect(Common::Rect(barLeft + negativeHpPart, hpBarTop, barLeft + faceWidth, hpBarBottom), DARK_BLUE);
		}

		if (maxSp != 0) {
			win.fillRect(Common::Rect(barLeft, spBarTop, barRight, spBarBottom), BLACK);
			win.fillRect(Common::Rect(barLeft, spBarTop, barLeft + spPart, spBarBottom), BLUE);
			if (boostedSpPart) {
				win.fillRect(Common::Rect(barLeft, spBarTop, barLeft + boostedSpPart, spBarBottom), BRIGHT_BLUE);
			}
		}
	}
}

void PartyDrawer::drawParty(bool updateFlag) {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Resources &res = *_vm->_resources;
	Windows &windows = *_vm->_windows;
	bool inCombat = _vm->_mode == MODE_COMBAT;

	_restoreSprites.draw(0, 0, Common::Point(8, 149));

	// Handle drawing the party faces
	uint partyCount = inCombat ? combat._combatParty.size() : party._activeParty.size();
	for (uint idx = 0; idx < partyCount; ++idx) {
		Character &c = inCombat ? *combat._combatParty[idx] : party._activeParty[idx];
		Condition charCondition = c.worstCondition();
		int charFrame = Res.FACE_CONDITION_FRAMES[charCondition];

		SpriteResource *sprites = (charFrame > 4) ? &_dseFace : c._faceSprites;
		assert(sprites);
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(0, charFrame, Common::Point(Res.CHAR_FACES_X[idx], 150));
	}

	for (uint idx = 0; idx < partyCount; ++idx) {
		drawHitPoints(idx);
	}

	if (_hiliteChar != HILIGHT_CHAR_NONE)
		res._globalSprites.draw(0, 8, Common::Point(Res.CHAR_FACES_X[_hiliteChar] - 1, 149));

	if (updateFlag)
		windows[33].update();
}

void PartyDrawer::highlightChar(int charId) {
	Resources &res = *_vm->_resources;
	Windows &windows = *_vm->_windows;
	assert(charId < MAX_ACTIVE_PARTY);

	if (charId != _hiliteChar && _hiliteChar != HILIGHT_CHAR_DISABLED) {
		// Handle deselecting any previusly selected char
		if (_hiliteChar != HILIGHT_CHAR_NONE) {
			res._globalSprites.draw(0, 9 + _hiliteChar,
				Common::Point(Res.CHAR_FACES_X[_hiliteChar] - 1, 149));
		}

		// Highlight new character
		res._globalSprites.draw(0, 8, Common::Point(Res.CHAR_FACES_X[charId] - 1, 149));
		_hiliteChar = charId;
		windows[33].update();
	}
}

void PartyDrawer::highlightChar(const Character *c) {
	int charNum = _vm->_party->_activeParty.indexOf(*c);
	if (charNum != -1)
		highlightChar(charNum);
}

void PartyDrawer::unhighlightChar() {
	Resources &res = *_vm->_resources;
	Windows &windows = *_vm->_windows;

	if (_hiliteChar != HILIGHT_CHAR_NONE) {
		res._globalSprites.draw(0, _hiliteChar + 9,
			Common::Point(Res.CHAR_FACES_X[_hiliteChar] - 1, 149));
		_hiliteChar = HILIGHT_CHAR_NONE;
		windows[33].update();
	}
}

void PartyDrawer::resetHighlight() {
	_hiliteChar = HILIGHT_CHAR_NONE;
}

/*------------------------------------------------------------------------*/

Interface::Interface(XeenEngine *vm) : ButtonContainer(vm), InterfaceScene(vm),
		PartyDrawer(vm), _vm(vm) {
	_buttonsLoaded = false;
	_obscurity = OBSCURITY_NONE;
	_steppingFX = 0;
	_falling = FALL_NONE;
	_blessedUIFrame = 0;
	_powerShieldUIFrame = 0;
	_holyBonusUIFrame = 0;
	_heroismUIFrame = 0;
	_flipUIFrame = 0;
	_face1UIFrame = 0;
	_face2UIFrame = 0;
	_levitateUIFrame = 0;
	_spotDoorsUIFrame = 0;
	_dangerSenseUIFrame = 0;
	_face1State = _face2State = 2;
	_upDoorText = false;
	_tillMove = 0;
	_iconsMode = ICONS_STANDARD;
	Common::fill(&_charFX[0], &_charFX[MAX_ACTIVE_PARTY], 0);
	setWaitBounds();
}

void Interface::setup() {
	_borderSprites.load("border.icn");
	_spellFxSprites.load("spellfx.icn");
	_fecpSprites.load("fecp.brd");
	_blessSprites.load("bless.icn");
	_charPowSprites.load("charpow.icn");
	_uiSprites.load("inn.icn");
	_stdIcons.load("main.icn");
	_combatIcons.load("combat.icn");

	Party &party = *_vm->_party;
	party.loadActiveParty();
	party._newDay = party._minutes < 300;
}

void Interface::startup() {
	Resources &res = *_vm->_resources;

	animate3d();
	if (_vm->_map->_isOutdoors) {
		setOutdoorsMonsters();
		setOutdoorsObjects();
	} else {
		setIndoorsMonsters();
		setIndoorsObjects();
	}

	draw3d(false);

	if (g_vm->getGameID() == GType_Swords)
		res._logoSprites.draw(1, 0, Common::Point(232, 9));
	else
		res._globalSprites.draw(1, 5, Common::Point(232, 9));

	drawParty(false);
	setMainButtons();

	_tillMove = false;
}

void Interface::mainIconsPrint() {
	Resources &res = *_vm->_resources;
	Windows &windows = *_vm->_windows;
	windows[38].close();
	windows[12].close();

	res._globalSprites.draw(0, 7, Common::Point(232, 74));
	drawButtons(&windows[0]);
	windows[34].update();
}

void Interface::setMainButtons(IconsMode mode) {
	clearButtons();
	_iconsMode = mode;
	SpriteResource *spr = mode == ICONS_COMBAT ? &_combatIcons : &_stdIcons;

	addButton(Common::Rect(235,  75, 259,  95),  Common::KEYCODE_s, spr);
	addButton(Common::Rect(260,  75, 284,  95),  Common::KEYCODE_c, spr);
	addButton(Common::Rect(286,  75, 310,  95),  Common::KEYCODE_r, spr);
	addButton(Common::Rect(235,  96, 259, 116),  Common::KEYCODE_b, spr);
	addButton(Common::Rect(260,  96, 284, 116),  Common::KEYCODE_d, spr);
	addButton(Common::Rect(286,  96, 310, 116),  Common::KEYCODE_v, spr);
	addButton(Common::Rect(235, 117, 259, 137),  Common::KEYCODE_m, spr);
	addButton(Common::Rect(260, 117, 284, 137),  Common::KEYCODE_i, spr);
	addButton(Common::Rect(286, 117, 310, 137),  Common::KEYCODE_q, spr);
	addButton(Common::Rect(109, 137, 122, 147), Common::KEYCODE_TAB, spr);
	addButton(Common::Rect(235, 148, 259, 168), Common::KEYCODE_LEFT, spr);
	addButton(Common::Rect(260, 148, 284, 168), Common::KEYCODE_UP, spr);
	addButton(Common::Rect(286, 148, 310, 168), Common::KEYCODE_RIGHT, spr);
	addButton(Common::Rect(235, 169, 259, 189), (Common::KBD_CTRL << 16) |Common::KEYCODE_LEFT, spr);
	addButton(Common::Rect(260, 169, 284, 189), Common::KEYCODE_DOWN, spr);
	addButton(Common::Rect(286, 169, 310, 189), (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT, spr);
	addButton(Common::Rect(236,  11, 308,  69),  Common::KEYCODE_EQUALS);
	addButton(Common::Rect(239,  27, 312,  37),  Common::KEYCODE_1);
	addButton(Common::Rect(239, 37, 312, 47), Common::KEYCODE_2);
	addButton(Common::Rect(239, 47, 312, 57), Common::KEYCODE_3);
	addPartyButtons(_vm);

	if (mode == ICONS_COMBAT) {
		_buttons[0]._value = Common::KEYCODE_f;
		_buttons[1]._value = Common::KEYCODE_c;
		_buttons[2]._value = Common::KEYCODE_a;
		_buttons[3]._value = Common::KEYCODE_u;
		_buttons[4]._value = Common::KEYCODE_r;
		_buttons[5]._value = Common::KEYCODE_b;
		_buttons[6]._value = Common::KEYCODE_o;
		_buttons[7]._value = Common::KEYCODE_i;
		_buttons[16]._value = 0;
	}
}

void Interface::perform() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Scripts &scripts = *_vm->_scripts;
	Sound &sound = *_vm->_sound;

	do {
		// Draw the next frame
		events.updateGameCounter();
		draw3d(true);

		// Wait for a frame or a user event
		_buttonValue = 0;
		do {
			events.pollEventsAndWait();
			if (g_vm->shouldExit() || g_vm->isLoadPending() || party._dead)
				return;

			checkEvents(g_vm);
		} while (!_buttonValue && events.timeElapsed() < 1);
	} while (!_buttonValue);

	if (_buttonValue == Common::KEYCODE_SPACE) {
		int lookupId = map.mazeLookup(party._mazePosition,
			Res.WALL_SHIFTS[party._mazeDirection][2]);

		bool eventsFlag = true;
		switch (lookupId) {
		case 1:
			if (!map._isOutdoors) {
				eventsFlag = !scripts.openGrate(13, 1);
			}
			break;
		case 6:
			// Open grate being closed
			if (!map._isOutdoors) {
				eventsFlag = !scripts.openGrate(9, 0);
			}
			break;
		case 9:
			// Closed grate being opened
			if (!map._isOutdoors) {
				eventsFlag = !scripts.openGrate(6, 0);
			}
			break;
		case 13:
			if (!map._isOutdoors) {
				eventsFlag = !scripts.openGrate(1, 1);
			}
			break;
		default:
			break;
		}
		if (eventsFlag) {
			scripts.checkEvents();
			if (_vm->shouldExit())
				return;
		} else {
			clearEvents();
		}
	}

	switch (_buttonValue) {
	case Common::KEYCODE_TAB:
		// Show control panel
		combat._moveMonsters = false;
		ControlPanel::show(_vm);
		if (!g_vm->shouldExit() && !g_vm->_gameMode)
			combat._moveMonsters = true;
		break;

	case Common::KEYCODE_SPACE:
	case Common::KEYCODE_w:
		// Wait one turn
		chargeStep();
		combat.moveMonsters();
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

	case (Common::KBD_CTRL << 16) | Common::KEYCODE_DOWN:
		party._mazeDirection = (Direction)((int)party._mazeDirection ^ 2);
		_flipSky = !_flipSky;
		_isAnimReset = true;
		stepTime();
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
				combat.moveMonsters();
		}
		break;

	case Common::KEYCODE_EQUALS:
	case Common::KEYCODE_KP_EQUALS:
		// Toggle minimap
		party._automapOn = !party._automapOn;
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
			default:
				break;
			}
			chargeStep();
			stepTime();
		} else {
			bash(party._mazePosition, party._mazeDirection);
		}
		break;

	case Common::KEYCODE_c:
		// Cast spell
		if (_tillMove) {
			combat.moveMonsters();
			draw3d(true);
		}

		if (CastSpell::show(_vm) != -1) {
			chargeStep();
			doStepCode();
			// update spell point bar
			if (g_vm->_extOptions._showHpSpBars) {
				drawParty(true);
			}
		}
		break;

	case Common::KEYCODE_d:
		// Show dismiss dialog
		Dismiss::show(_vm);
		break;

	case Common::KEYCODE_i:
		// Show Info dialog
		combat._moveMonsters = false;
		InfoDialog::show(_vm);
		combat._moveMonsters = true;
		break;

	case Common::KEYCODE_m:
		// Show map dialog
		MapDialog::show(_vm);
		break;

	case Common::KEYCODE_q:
		// Show the quick reference dialog
		QuickReferenceDialog::show(_vm);
		break;

	case Common::KEYCODE_r:
		// Rest
		rest();
		break;

	case Common::KEYCODE_s:
		// Shoot
		if (!party.canShoot()) {
			sound.playFX(21);
		} else {
			if (_tillMove) {
				combat.moveMonsters();
				draw3d(true);
			}

			if (combat._attackMonsters[0] != -1 || combat._attackMonsters[1] != -1
					|| combat._attackMonsters[2] != -1) {
				if ((_vm->_mode == MODE_INTERACTIVE || _vm->_mode == MODE_SLEEPING)
						&& !combat._monstersAttacking && !_charsShooting) {
					doCombat();
				}
			}

			combat.shootRangedWeapon();
			chargeStep();
			doStepCode();
		}
		break;

	case Common::KEYCODE_v:
		// Show the quests dialog
		Quests::show(_vm);
		break;

	default:
		break;
	}
}

void Interface::chargeStep() {
	if (!_vm->_party->_dead) {
		_vm->_party->changeTime(_vm->_map->_isOutdoors ? 10 : 1);
		if (_tillMove) {
			_vm->_combat->moveMonsters();
		}

		_tillMove = 3;
	}
}

void Interface::stepTime() {
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
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
	Combat &combat = *_vm->_combat;
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
		party._dead = true;
		break;
	case SURFTYPE_LAVA:
		// It burns, it burns!
		damage = 100;
		combat._damageType = DT_FIRE;
		break;
	case SURFTYPE_SKY:
		// We can fly, we can.. oh wait, we can't!
		damage = 100;
		combat._damageType = DT_PHYSICAL;
		_falling = FALL_IN_PROGRESS;
		break;
	case SURFTYPE_DESERT:
		// Without navigation skills, simulate getting lost by adding extra time
		if (map._isOutdoors && !party.checkSkill(NAVIGATOR))
			party.addTime(170);
		break;
	case SURFTYPE_CLOUD:
		if (!party._levitateCount) {
			combat._damageType = DT_PHYSICAL;
			_falling = FALL_IN_PROGRESS;
			damage = 100;
		}
		break;
	default:
		break;
	}

	if (_vm->getGameID() != GType_Swords && _vm->_files->_ccNum && party._gameFlags[1][118]) {
		_falling = FALL_NONE;
	} else {
		if (_falling != FALL_NONE)
			startFalling(false);

		if ((party._mazePosition.x & 16) || (party._mazePosition.y & 16)) {
			if (map._isOutdoors)
				map.getNewMaze();
		}

		if (damage) {
			_flipGround = !_flipGround;
			draw3d(true);

			int oldTarget = combat._combatTarget;
			combat._combatTarget = 0;

			// WORKAROUND: Stepping into combat whilst on lava results in damageType being lost
			combat._damageType = (surfaceId == SURFTYPE_LAVA) ? DT_FIRE : DT_PHYSICAL;
			combat.giveCharDamage(damage, combat._damageType, 0);

			combat._combatTarget = oldTarget;
			_flipGround = !_flipGround;
		} else if (party._dead) {
			draw3d(true);
		}
	}
}

void Interface::startFalling(bool flag) {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int ccNum = _vm->_files->_ccNum;

	if (ccNum && party._gameFlags[1][118]) {
		_falling = FALL_NONE;
		return;
	}

	_falling = FALL_NONE;
	draw3d(true);
	_falling = FALL_START;
	draw3d(false);

	if (!flag && g_vm->getGameID() == GType_Swords) {
		if (party._mazeId != 52) {
			party._mazeId = 49;
			party._mazePosition = Common::Point(7, 0);
		} else if (party._mazePosition.x == 8 && party._mazePosition.y == 7) {
			party._mazeId = 78;
			party._mazePosition = Common::Point(8, 2);
		} else {
			party._mazeId = 22;
			party._mazePosition = Common::Point(7, 9);
		}
	} else if (flag && (!ccNum || g_vm->getGameID() == GType_Swords || party._fallMaze != 0)) {
		party._mazeId = party._fallMaze;
		party._mazePosition = party._fallPosition;
	} else if (!ccNum) {
		switch (party._mazeId - 25) {
		case 0:
		case 26:
		case 27:
		case 28:
		case 29:
			party._mazeId = 24;
			party._mazePosition = Common::Point(11, 9);
			break;
		case 1:
		case 30:
		case 31:
		case 32:
		case 33:
			party._mazeId = 12;
			party._mazePosition = Common::Point(6, 15);
			break;
		case 2:
		case 34:
		case 35:
		case 36:
		case 37:
		case 51:
		case 52:
		case 53:
			party._mazeId = 15;
			party._mazePosition = Common::Point(4, 12);
			party._mazeDirection = DIR_SOUTH;
			break;
		case 40:
		case 41:
			party._mazeId = 14;
			party._mazePosition = Common::Point(8, 3);
			break;
		case 44:
		case 45:
			party._mazeId = 1;
			party._mazePosition = Common::Point(8, 7);
			party._mazeDirection = DIR_NORTH;
			break;
		case 49:
			party._mazeId = 12;
			party._mazePosition = Common::Point(11, 13);
			party._mazeDirection = DIR_SOUTH;
			break;
		case 57:
		case 58:
		case 59:
			party._mazeId = 5;
			party._mazePosition = Common::Point(12, 7);
			party._mazeDirection = DIR_NORTH;
			break;
		case 60:
			party._mazeId = 6;
			party._mazePosition = Common::Point(12, 3);
			party._mazeDirection = DIR_NORTH;
			break;
		default:
			party._mazeId = 23;
			party._mazePosition = Common::Point(12, 10);
			party._mazeDirection = DIR_NORTH;
			break;
		}
	} else {
		if (party._mazeId > 88 && party._mazeId < 114) {
			party._mazeId -= 88;
		} else {
			switch (party._mazeId - 25) {
			case 0:
				party._mazeId = 89;
				party._mazePosition = Common::Point(2, 14);
				break;
			case 1:
				party._mazeId = 109;
				party._mazePosition = Common::Point(13, 14);
				break;
			case 2:
				party._mazeId = 112;
				party._mazePosition = Common::Point(13, 3);
				break;
			case 3:
				party._mazeId = 92;
				party._mazePosition = Common::Point(2, 3);
				break;
			case 12:
			case 13:
				party._mazeId = 14;
				party._mazePosition = Common::Point(10, 2);
				break;
			case 16:
			case 17:
			case 18:
				party._mazeId = 4;
				party._mazePosition = Common::Point(5, 14);
				break;
			case 20:
			case 21:
			case 22:
				party._mazeId = 21;
				party._mazePosition = Common::Point(9, 11);
				break;
			case 24:
			case 25:
			case 26:
				party._mazeId = 1;
				party._mazePosition = Common::Point(10, 4);
				break;
			case 28:
			case 29:
			case 30:
			case 31:
				party._mazeId = 26;
				party._mazePosition = Common::Point(12, 10);
				break;
			case 32:
			case 33:
			case 34:
			case 35:
				party._mazeId = 3;
				party._mazePosition = Common::Point(4, 9);
				break;
			case 36:
			case 37:
			case 38:
			case 39:
				party._mazeId = 16;
				party._mazePosition = Common::Point(2, 7);
				break;
			case 40:
			case 41:
			case 42:
			case 43:
				party._mazeId = 23;
				party._mazePosition = Common::Point(10, 9);
				break;
			case 44:
			case 45:
			case 46:
			case 47:
				party._mazeId = 13;
				party._mazePosition = Common::Point(2, 10);
				break;
			case 103:
			case 104:
				map._loadCcNum = 0;
				party._mazeId = 8;
				party._mazePosition = Common::Point(11, 15);
				party._mazeDirection = DIR_NORTH;
				break;
			case 105:
				party._mazeId = 24;
				party._mazePosition = Common::Point(11, 9);
				break;
			case 106:
				party._mazeId = 12;
				party._mazePosition = Common::Point(6, 15);
				break;
			case 107:
				party._mazeId = 15;
				party._mazePosition = Common::Point(4, 12);
				break;
			default:
				party._mazeId = 29;
				party._mazePosition = Common::Point(25, 21);
				party._mazeDirection = DIR_NORTH;
				break;
			}
		}
	}

	_falling = FALL_IN_PROGRESS;
	map.load(party._mazeId);

	if (flag) {
		if (map._isOutdoors && ((party._mazePosition.x & 16) || (party._mazePosition.y & 16)))
			map.getNewMaze();

		_flipGround ^= 1;
		draw3d(true);
		int oldTarget = combat._combatTarget;
		combat._combatTarget = 0;
		combat.giveCharDamage(party._fallDamage, DT_PHYSICAL, 0);

		combat._combatTarget = oldTarget;
		_flipGround ^= 1;
	}
}

bool Interface::checkMoveDirection(int key) {
	Debugger &debugger = *g_vm->_debugger;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	// If intangibility is turned on in the debugger, allow any movement
	if (debugger._intangible)
		return true;

	// For strafing or moving backwards, temporarily move to face the direction being checked,
	// since the call to getCell will the adjacent cell details in the direction being faced
	Direction dir = party._mazeDirection;
	switch (key) {
	case (Common::KBD_CTRL << 16) | Common::KEYCODE_LEFT:
		party._mazeDirection = (party._mazeDirection == DIR_NORTH) ? DIR_WEST :
			(Direction)(party._mazeDirection - 1);
		break;
	case (Common::KBD_CTRL << 16) | Common::KEYCODE_RIGHT:
		party._mazeDirection = (party._mazeDirection == DIR_WEST) ? DIR_NORTH :
			(Direction)(party._mazeDirection + 1);
		break;
	case Common::KEYCODE_DOWN:
		party._mazeDirection = (Direction)((int)party._mazeDirection ^ 2);
		break;
	default:
		break;
	}

	// Get next facing tile information
	map.getCell(7);

	int startSurfaceId = map._currentSurfaceId;
	int surfaceId;

	if (map._isOutdoors) {
		// Reset direction back to original facing, if it was changed for strafing checks
		party._mazeDirection = dir;

		switch (map._currentWall) {
		case 5:
			if (_vm->_files->_ccNum)
				goto check;

			// fall through
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
		surfaceId = map.getCell(2);

		// Reset direction back to original facing, if it was changed for strafing checks
		party._mazeDirection = dir;

		if (surfaceId >= map.mazeData()._difficulties._wallNoPass) {
			sound.playFX(46);
			return false;
		} else {
			if (startSurfaceId != SURFTYPE_SWAMP || party.checkSkill(SWIMMING) ||
					party._walkOnWaterActive) {
				if (_buttonValue == Common::KEYCODE_UP && _wo[107]) {
					_openDoor = true;
					sound.playFX(47);
					draw3d(true);
					_openDoor = false;
				}
				return true;
			} else {
				sound.playFX(46);
				return false;
			}
		}
	}

	return true;
}

void Interface::rest() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	map.cellFlagLookup(party._mazePosition);

	if ((map._currentCantRest || (map.mazeData()._mazeFlags & RESTRICTION_REST))
			&& _vm->_mode != MODE_INTERACTIVE2) {
		ErrorScroll::show(_vm, Res.TOO_DANGEROUS_TO_REST, WT_NONFREEZED_WAIT);
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
			if (!Confirm::show(_vm, Res.SOME_CHARS_MAY_DIE))
				return;
		}

		// Mark all the players as being asleep
		for (uint charIdx = 0; charIdx < party._activeParty.size(); ++charIdx) {
			party._activeParty[charIdx]._conditions[ASLEEP] = 1;
		}
		drawParty(true);

		Mode oldMode = _vm->_mode;
		_vm->_mode = MODE_SLEEPING;

		if (oldMode == MODE_INTERACTIVE2) {
			party.changeTime(8 * 60);
		} else {
			for (int idx = 0; idx < 10; ++idx) {
				chargeStep();
				draw3d(true);

				if (_vm->_mode == MODE_INTERACTIVE) {
					_vm->_mode = oldMode;
					return;
				}
			}

			party.changeTime(map._isOutdoors ? 380 : 470);
		}

		if (_vm->getRandomNumber(1, 20) == 1)
			_vm->dream();

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

					// WORKAROUND: Resting curing weakness only originally worked due to a bug in changeTime
					// resetting WEAK if party wasn't drunk. With that resolved, we have to reset WEAK here
					c._conditions[WEAK] = 0;
				}
			}
		}

		drawParty(true);
		_vm->_mode = oldMode;
		doStepCode();
		draw3d(true);

		ErrorScroll::show(_vm, Common::String::format(Res.REST_COMPLETE,
			starving ? Res.PARTY_IS_STARVING : Res.HIT_SPELL_POINTS_RESTORED,
			foodConsumed));
		party.checkPartyDead();
	}
}

void Interface::bash(const Common::Point &pt, Direction direction) {
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;

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
	_charPowSprites.draw(windows[0], 0,
		Common::Point(Res.CHAR_FACES_X[charNum1 - 1], 150));
	windows[0].update();

	if (charNum2) {
		party._activeParty[charNum2 - 1].subtractHitPoints(2);
		_charPowSprites.draw(windows[0], 0,
			Common::Point(Res.CHAR_FACES_X[charNum2 - 1], 150));
		windows[0].update();
	}

	int cell = map.mazeLookup(Common::Point(pt.x + Res.SCREEN_POSITIONING_X[direction][7],
		pt.y + Res.SCREEN_POSITIONING_Y[direction][7]), 0, 0xffff);
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
				default:
					break;
				}
			}
		}
	}

	party.checkPartyDead();
	events.ipause(2);
	drawParty(true);
}

void Interface::draw3d(bool updateFlag, bool pauseFlag) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Scripts &scripts = *_vm->_scripts;
	Windows &windows = *_vm->_windows;

	events.timeMark5();
	if (windows[SCENE_WINDOW]._enabled)
		return;

	_flipUIFrame = (_flipUIFrame + 1) % 4;
	if (_flipUIFrame == 0)
		_flipWater = !_flipWater;
	if (_tillMove && (_vm->_mode == MODE_INTERACTIVE || _vm->_mode == MODE_COMBAT) &&
		!combat._monstersAttacking && combat._moveMonsters) {
		if (--_tillMove == 0)
			combat.moveMonsters();
	}

	// Draw the game scene
	drawScene();

	// Draw the minimap
	drawMinimap();

	// Handle any darkness-based oscurity
	obscureScene(_obscurity);

	if (_falling == FALL_IN_PROGRESS)
		handleFalling();

	if (_falling == FALL_START) {
		setupFallSurface(true);
	}

	assembleBorder();

	// Draw any on-screen text if flagged to do so
	if (_upDoorText && combat._attackMonsters[0] == -1) {
		windows[3].writeString(_screenText);
	}

	if (updateFlag) {
		windows[1].update();
		windows[3].update();
	}

	if (combat._attackMonsters[0] != -1 || combat._attackMonsters[1] != -1
			|| combat._attackMonsters[2] != -1) {
		if ((_vm->_mode == MODE_INTERACTIVE || _vm->_mode == MODE_SLEEPING) &&
				!combat._monstersAttacking && !_charsShooting && combat._moveMonsters) {
			doCombat();
			if (scripts._eventSkipped)
				scripts.checkEvents();
		}
	}

	party._stepped = false;
	if (pauseFlag)
		events.ipause5(2);
}

void Interface::handleFalling() {
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	Window &w = windows[3];

	// Set the bottom half of the fall surface (area that is being fallen to)
	setupFallSurface(false);

	// Update character faces and start scream
	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		party._activeParty[idx]._faceSprites->draw(0, 4,
			Common::Point(Res.CHAR_FACES_X[idx], 150));
	}

	windows[33].update();
	sound.playFX(11);
	sound.playSound("scream.voc");

	// Fall down to the ground
	#define YINDEX (SCENE_HEIGHT / 2)
	const int Y_LIST[] = {
		SCENE_HEIGHT, SCENE_HEIGHT - 5, SCENE_HEIGHT, SCENE_HEIGHT - 3, SCENE_HEIGHT
	};
	for (int idx = 1; idx < YINDEX + 5; ++idx) {
		fall((idx < YINDEX) ? idx * 2 : Y_LIST[idx - YINDEX]);
		assembleBorder();
		w.update();
		screen.update();
		g_system->delayMillis(5);

		if (idx == YINDEX) {
			sound.stopSound();
			sound.playSound("unnh.voc");
			sound.playFX(31);
		}
	}

	shake(10);

	_falling = FALL_NONE;
	drawParty(true);
}

void Interface::setupFallSurface(bool isTop) {
	Window &w = (*g_vm->_windows)[SCENE_WINDOW];

	if (_fallSurface.empty())
		_fallSurface.create(SCENE_WIDTH, SCENE_HEIGHT * 2);
	_fallSurface.blitFrom(w, w.getBounds(), Common::Point(0, isTop ? 0 : SCENE_HEIGHT));
}

void Interface::fall(int yp) {
	Window &w = (*g_vm->_windows)[SCENE_WINDOW];
	w.blitFrom(_fallSurface, Common::Rect(0, yp, SCENE_WIDTH, yp + SCENE_HEIGHT), Common::Point(8, 8));
}

void Interface::shake(int count) {
	Screen &screen = *g_vm->_screen;
	byte b;

	for (int idx = 0; idx < count * 2; ++idx) {
		for (int yp = 0; yp < screen.h; ++yp) {
			byte *lineP = (byte *)screen.getBasePtr(0, yp);
			if (idx % 2) {
				// Shift back right
				b = lineP[SCREEN_WIDTH - 1];
				Common::copy_backward(lineP, lineP + SCREEN_WIDTH - 1, lineP + SCREEN_WIDTH);
				lineP[0] = b;
			} else {
				// Scroll left one pixel
				b = lineP[0];
				Common::copy(lineP + 1, lineP + SCREEN_WIDTH, lineP);
				lineP[SCREEN_WIDTH - 1] = b;
			}
		}

		screen.markAllDirty();
		screen.update();
		g_system->delayMillis(5);
	}
}

void Interface::assembleBorder() {
	Combat &combat = *_vm->_combat;
	Resources &res = *_vm->_resources;
	Windows &windows = *_vm->_windows;

	// Draw the outer frame
	res._globalSprites.draw(windows[0], 0, Common::Point(8, 8));

	// Draw the animating bat character on the left screen edge to indicate
	// that the party is being levitated
	_borderSprites.draw(windows[0], _vm->_party->_levitateCount ? _levitateUIFrame + 16 : 16,
		Common::Point(0, 82));
	_levitateUIFrame = (_levitateUIFrame + 1) % 12;

	// Draw UI element to indicate whether can spot hidden doors
	_borderSprites.draw(0,
		(_thinWall && _vm->_party->checkSkill(SPOT_DOORS)) ? _spotDoorsUIFrame + 28 : 28,
		Common::Point(194, 91));
	_spotDoorsUIFrame = (_spotDoorsUIFrame + 1) % 12;

	// Draw UI element to indicate whether can sense danger
	_borderSprites.draw(0,
		(combat._dangerPresent && _vm->_party->checkSkill(DANGER_SENSE)) ? _spotDoorsUIFrame + 40 : 40,
		Common::Point(107, 9));
	_dangerSenseUIFrame = (_dangerSenseUIFrame + 1) % 12;

	// Handle the face UI elements for indicating clairvoyance status
	_face1UIFrame = (_face1UIFrame + 1) % 4;
	if (_face1State == 0)
		_face1UIFrame += 4;
	else if (_face1State == 2)
		_face1UIFrame = 0;

	_face2UIFrame = (_face2UIFrame + 1) % 4 + 12;
	if (_face2State == 0)
		_face2UIFrame -= 3;
	else if (_face2State == 2)
		_face2UIFrame = 8;

	if (!_vm->_party->_clairvoyanceActive) {
		_face1UIFrame = 0;
		_face2UIFrame = 8;
	}

	_borderSprites.draw(0, _face1UIFrame, Common::Point(0, 32));
	_borderSprites.draw(0,
		windows[10]._enabled || windows[2]._enabled ? 52 : _face2UIFrame,
		Common::Point(215, 32));

	// Draw resistance indicators
	if (!windows[10]._enabled && !windows[2]._enabled
		&& !windows[38]._enabled) {
		_fecpSprites.draw(0, _vm->_party->_fireResistance ? 1 : 0,
			Common::Point(2, 2));
		_fecpSprites.draw(0, _vm->_party->_electricityResistance ? 3 : 2,
			Common::Point(219, 2));
		_fecpSprites.draw(0, _vm->_party->_coldResistance ? 5 : 4,
			Common::Point(2, 134));
		_fecpSprites.draw(0, _vm->_party->_poisonResistance ? 7 : 6,
			Common::Point(219, 134));
	} else {
		_fecpSprites.draw(0, _vm->_party->_fireResistance ? 9 : 8,
			Common::Point(8, 8));
		_fecpSprites.draw(0, _vm->_party->_electricityResistance ? 11 : 10,
			Common::Point(219, 8));
		_fecpSprites.draw(0, _vm->_party->_coldResistance ? 13 : 12,
			Common::Point(8, 134));
		_fecpSprites.draw(0, _vm->_party->_poisonResistance ? 15 : 14,
			Common::Point(219, 134));
	}

	// Draw UI element for blessed
	_blessSprites.draw(0, 16, Common::Point(33, 137));
	if (_vm->_party->_blessed) {
		_blessedUIFrame = (_blessedUIFrame + 1) % 4;
		_blessSprites.draw(0, _blessedUIFrame, Common::Point(33, 137));
	}

	// Draw UI element for power shield
	if (_vm->_party->_powerShield) {
		_powerShieldUIFrame = (_powerShieldUIFrame + 1) % 4;
		_blessSprites.draw(0, _powerShieldUIFrame + 4,
			Common::Point(55, 137));
	}

	// Draw UI element for holy bonus
	if (_vm->_party->_holyBonus) {
		_holyBonusUIFrame = (_holyBonusUIFrame + 1) % 4;
		_blessSprites.draw(0, _holyBonusUIFrame + 8, Common::Point(160, 137));
	}

	// Draw UI element for heroism
	if (_vm->_party->_heroism) {
		_heroismUIFrame = (_heroismUIFrame + 1) % 4;
		_blessSprites.draw(0, _heroismUIFrame + 12, Common::Point(182, 137));
	}

	// Draw direction character if direction sense is active
	if (_vm->_party->checkSkill(DIRECTION_SENSE) && !_vm->_noDirectionSense) {
		const char *dirText = Res.DIRECTION_TEXT_UPPER[_vm->_party->_mazeDirection];
		Common::String msg = Common::String::format("\x2\f08\x3""c\v139\t116%c\fd\x1", *dirText);
		windows[0].writeString(msg);
	}

	// Draw view frame
	if (windows[12]._enabled)
		windows[12].frame();
}

void Interface::doCombat() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Scripts &scripts = *_vm->_scripts;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	bool upDoorText = _upDoorText;
	bool reloadMap = false;
	int index = 0;

	_upDoorText = false;
	combat._combatMode = COMBATMODE_2;
	_vm->_mode = MODE_COMBAT;

	// Set the combat buttons
	IconsMode oldMode = _iconsMode;
	setMainButtons(ICONS_COMBAT);
	mainIconsPrint();

	combat._combatParty.clear();
	combat.clearBlocked();
	combat._pow[0]._duration = 0;
	combat._pow[1]._duration = 0;
	combat._pow[2]._duration = 0;
	combat._monstersAttacking = false;
	combat._partyRan = false;

	// Set up the combat party
	combat.setupCombatParty();
	combat.setSpeedTable();

	// Initialize arrays for character/monster states
	Common::fill(&combat._charsGone[0], &combat._charsGone[PARTY_AND_MONSTERS], 0);
	Common::fill(&combat._charsBlocked[0], &combat._charsBlocked[PARTY_AND_MONSTERS], false);

	combat._whosSpeed = -1;
	combat._whosTurn = -1;
	resetHighlight();

	nextChar();

	if (!party._dead) {
		combat.setSpeedTable();

		if (_tillMove) {
			combat.moveMonsters();
			draw3d(true);
		}

		Window &w = windows[2];
		w.open();
		bool breakFlag = false;

		while (!_vm->shouldExit() && !breakFlag && !party._dead && _vm->_mode == MODE_COMBAT) {
			// FIXME: I've had a rare issue where the loop starts with a non-party _whosTurn. Unfortunately,
			// I haven't been able to consistently replicate and diagnose the problem, so for now,
			// I'm simply detecting if it happens and resetting the combat round
			if (combat._whosTurn >= (int)party._activeParty.size())
				goto new_round;

			highlightChar(combat._whosTurn);
			combat.setSpeedTable();

			// Write out the description of the monsters being battled
			w.writeString(combat.getMonsterDescriptions());
			_combatIcons.draw(0, 32, Common::Point(233, combat._attackDurationCtr * 10 + 27),
				SPRFLAG_800, 0);
			w.update();

			// Wait for keypress
			index = 0;
			do {
				events.updateGameCounter();
				draw3d(true);

				if (++index == 5 && combat._attackMonsters[0] != -1) {
					MazeMonster &monster = map._mobData._monsters[combat._monster2Attack];
					MonsterStruct &monsterData = *monster._monsterData;
					sound.playFX(monsterData._fx);
				}

				do {
					events.pollEventsAndWait();
					checkEvents(_vm);
				} while (!_vm->shouldExit() && events.timeElapsed() < 1 && !_buttonValue);
			} while (!_vm->shouldExit() && !_buttonValue);
			if (_vm->shouldExit())
				goto exit;

			switch (_buttonValue) {
			case Common::KEYCODE_TAB:
				// Show the control panel
				if (ControlPanel::show(_vm) == 2) {
					reloadMap = true;
					breakFlag = true;
				} else {
					highlightChar(combat._whosTurn);
				}
				break;

			case Common::KEYCODE_1:
			case Common::KEYCODE_2:
			case Common::KEYCODE_3:
				_buttonValue -= Common::KEYCODE_1;
				if (combat._attackMonsters[_buttonValue] != -1) {
					combat._monster2Attack = combat._attackMonsters[_buttonValue];
					combat._attackDurationCtr = _buttonValue;
				}
				break;

			case Common::KEYCODE_a:
				// Attack
				combat.attack(*combat._combatParty[combat._whosTurn], RT_SINGLE);
				nextChar();
				break;

			case Common::KEYCODE_b:
				// Block
				combat.block();
				nextChar();
				break;

			case Common::KEYCODE_c: {
				// Cast spell
				if (CastSpell::show(_vm) != -1) {
					nextChar();
					// update spell point bar
					if (g_vm->_extOptions._showHpSpBars) {
						drawParty(true);
					}
				} else {
					highlightChar(combat._whosTurn);
				}
				break;
			}

			case Common::KEYCODE_f:
				// Quick Fight
				combat.quickFight();
				nextChar();
				break;

			case Common::KEYCODE_i:
				// Info dialog
				InfoDialog::show(_vm);
				highlightChar(combat._whosTurn);
				break;

			case Common::KEYCODE_o:
				// Quick Fight Options
				QuickFight::show(_vm, combat._combatParty[combat._whosTurn]);
				highlightChar(combat._whosTurn);
				break;

			case Common::KEYCODE_q:
				// Quick Reference dialog
				QuickReferenceDialog::show(_vm);
				highlightChar(combat._whosTurn);
				break;

			case Common::KEYCODE_r:
				// Run from combat
				combat.run();
				nextChar();

				if (_vm->_mode == MODE_INTERACTIVE) {
					party._treasure._gems = 0;
					party._treasure._gold = 0;
					party._treasure._hasItems = false;
					party.moveToRunLocation();
					breakFlag = true;
				}
				break;

			case Common::KEYCODE_u: {
				int whosTurn = combat._whosTurn;
				ItemsDialog::show(_vm, combat._combatParty[combat._whosTurn], ITEMMODE_COMBAT);
				if (combat._whosTurn == whosTurn) {
					highlightChar(combat._whosTurn);
				} else {
					combat._whosTurn = whosTurn;
					nextChar();
				}
				break;
			}

			case Common::KEYCODE_F1:
			case Common::KEYCODE_F2:
			case Common::KEYCODE_F3:
			case Common::KEYCODE_F4:
			case Common::KEYCODE_F5:
			case Common::KEYCODE_F6:
				// Show character info
				_buttonValue -= Common::KEYCODE_F1;
				if (_buttonValue < (int)combat._combatParty.size()) {
					CharacterInfo::show(_vm, _buttonValue);
				}
				highlightChar(combat._whosTurn);
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_RIGHT:
				// Rotate party direction left or right
				if (_buttonValue == Common::KEYCODE_LEFT) {
					party._mazeDirection = (party._mazeDirection == DIR_NORTH) ?
						DIR_WEST : (Direction)((int)party._mazeDirection - 1);
				} else {
					party._mazeDirection = (party._mazeDirection == DIR_WEST) ?
					DIR_NORTH : (Direction)((int)party._mazeDirection + 1);
				}

				_flipSky ^= 1;
				if (_tillMove)
					combat.moveMonsters();
				party._stepped = true;
				break;

			default:
				break;
			}

			// Handling for if the combat turn is complete
			if (combat.allHaveGone()) {
new_round:
				Common::fill(&combat._charsGone[0], &combat._charsGone[PARTY_AND_MONSTERS], false);
				combat.clearBlocked();
				combat.setSpeedTable();
				combat._whosTurn = -1;
				combat._whosSpeed = -1;
				nextChar();

				for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
					MazeMonster &monster = map._mobData._monsters[idx];
					if (monster._spriteId == 53) {
						// For Medusa sprites, their HP keeps getting reset
						MonsterStruct &monsData = map._monsterData[53];
						monster._hp = monsData._hp;
					}
				}

				combat.moveMonsters();
				setIndoorsMonsters();
				party.changeTime(1);
			}

			if (combat._attackMonsters[0] == -1 && combat._attackMonsters[1] == -1
					&& combat._attackMonsters[2] == -1) {
				party.changeTime(1);
				draw3d(true);

				if (combat._attackMonsters[0] == -1 && combat._attackMonsters[1] == -1
						&& combat._attackMonsters[2] == -1)
					break;
			}

			party.checkPartyDead();
		}

		_vm->_mode = MODE_INTERACTIVE;
		if (combat._partyRan && (combat._attackMonsters[0] != -1 ||
				combat._attackMonsters[1] != -1 || combat._attackMonsters[2] != -1)) {
			party.checkPartyDead();
			if (!party._dead) {
				party.moveToRunLocation();

				for (uint idx = 0; idx < combat._combatParty.size(); ++idx) {
					Character &c = *combat._combatParty[idx];
					if (c.isDisabled())
						c._conditions[DEAD] = 1;
				}
			}
		}
exit:
		w.close();
		events.clearEvents();

		_vm->_mode = MODE_COMBAT;
		draw3d(true);
		party.giveTreasure();
		_vm->_mode = MODE_INTERACTIVE;
		party._stepped = true;
		unhighlightChar();

		combat.setupCombatParty();
		drawParty(true);
	}

	// Restore old icons
	setMainButtons(oldMode);
	mainIconsPrint();
	combat._monster2Attack = -1;

	if (!g_vm->isLoadPending()) {
		if (upDoorText) {
			map.cellFlagLookup(party._mazePosition);
			if (map._currentIsEvent)
				scripts.checkEvents();
		}

		if (reloadMap) {
			sound.playFX(51);
			map._loadCcNum = _vm->getGameID() != GType_WorldOfXeen ? 1 : 0;
			map.load(_vm->getGameID() == GType_WorldOfXeen ? 28 : 29);
			party._mazeDirection = _vm->getGameID() == GType_WorldOfXeen ?
				DIR_EAST : DIR_SOUTH;
		}
	}

	combat._combatMode = COMBATMODE_INTERACTIVE;
}

void Interface::nextChar() {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;

	if (combat.allHaveGone())
		return;
	if ((combat._attackMonsters[0] == -1 && combat._attackMonsters[1] == -1 &&
		combat._attackMonsters[2] == -1) || combat._combatParty.size() == 0) {
		_vm->_mode = MODE_INTERACTIVE;
		return;
	}

	// Loop for potentially multiple monsters attacking until it's time
	// for one of the party's turn
	for (;;) {
		// Check if party is dead
		party.checkPartyDead();
		if (party._dead) {
			_vm->_mode = MODE_INTERACTIVE;
			break;
		}

		int idx;
		for (idx = 0; idx < (int)combat._speedTable.size(); ++idx) {
			if (combat._whosTurn != -1) {
				combat._charsGone[combat._whosTurn] = true;
			}

			combat._whosSpeed = (combat._whosSpeed + 1) % combat._speedTable.size();
			combat._whosTurn = combat._speedTable[combat._whosSpeed];
			if (combat.allHaveGone()) {
				idx = -1;
				break;
			}

			if (combat._whosTurn < (int)combat._combatParty.size()) {
				// If it's a party member, only allow them to become active if
				// they're still conscious
				if (combat._combatParty[combat._whosTurn]->isDisabledOrDead())
					continue;
			}

			break;
		}

		if (idx == -1) {
			if (!combat.charsCantAct())
				return;

			combat.setSpeedTable();
			combat._whosTurn = -1;
			combat._whosSpeed = -1;
			Common::fill(&combat._charsGone[0], &combat._charsGone[PARTY_AND_MONSTERS], false);
			continue;
		}

		if (combat._whosTurn < (int)combat._combatParty.size()) {
			// It's a party character's turn now, so highlight the character
			if (!combat.allHaveGone()) {
				highlightChar(combat._whosTurn);
			}
			break;
		} else {
			// It's a monster's turn to attack
			combat.doMonsterTurn(0);
			if (!party._dead) {
				party.checkPartyDead();
				if (party._dead)
					break;
			}
		}
	}
}

void Interface::spellFX(Character *c) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;

	// Ensure there's no alraedy running effect for the given character
	uint charIndex;
	for (charIndex = 0; charIndex < party._activeParty.size(); ++charIndex) {
		if (&party._activeParty[charIndex] == c)
			break;
	}
	if (charIndex == party._activeParty.size() || _charFX[charIndex])
		return;

	if (windows[12]._enabled)
		windows[12].close();

	if (combat._combatMode == COMBATMODE_2) {
		for (uint idx = 0; idx < combat._combatParty.size(); ++idx) {
			if (combat._combatParty[idx]->_rosterId == c->_rosterId) {
				charIndex = idx;
				break;
			}
		}
	}

	int tillMove = _tillMove;
	_tillMove = 0;
	sound.playFX(20);

	for (int frameNum = 0; frameNum < 4; ++frameNum) {
		events.updateGameCounter();
		_spellFxSprites.draw(0, frameNum, Common::Point(
			Res.CHAR_FACES_X[charIndex], 150));

		if (!windows[SCENE_WINDOW]._enabled)
			draw3d(false);

		windows[0].update();
		events.wait(windows[SCENE_WINDOW]._enabled ? 2 : 1,false);
	}

	drawParty(true);
	_tillMove = tillMove;
	++_charFX[charIndex];
}

void Interface::obscureScene(Obscurity obscurity) {
	Screen &screen = *g_vm->_screen;
	const byte *lookup;

	switch (obscurity) {
	case OBSCURITY_BLACK:
		// Totally dark (black) background
		screen.fillRect(Common::Rect(8, 8, 224, 140), 0);
		break;

	case OBSCURITY_1:
	case OBSCURITY_2:
	case OBSCURITY_3:
		lookup = &Res.DARKNESS_XLAT[obscurity - 1][0];
		for (int yp = 8; yp < 140; ++yp) {
			byte *destP = (byte *)screen.getBasePtr(8, yp);
			for (int xp = 8; xp < 224; ++xp, ++destP)
				*destP = lookup[*destP];
		}
		break;

	default:
		// Full daylight, so no obscurity
		break;
	}
}

} // End of namespace Xeen
} // End of namespace MM
