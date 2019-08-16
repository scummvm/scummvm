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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

void Room::demon6Tick1() {
	playVoc("DEM6LOOP");
	if (_awayMission->demon.gotBerries || (!_awayMission->demon.field3e && _awayMission->demon.field37)) {
		loadActorAnim(8, "oldman", 0x5f, 0xb1, 0);
		_roomVar.demon.stephenInRoom = true;
	}
	loadActorAnim(10, "s0r6d2", 0xa0, 0x92, 0);
	loadActorAnim(9, "scrnan", 0, 0xc7, 0);
}

void Room::demon6Tick30() {
	if (!_awayMission->demon.gotBerries)
		return;
	if (_awayMission->demon.field3e)
		return;
	if (!_awayMission->demon.field37)
		return;
	if (_awayMission->demon.stephenWelcomedToStudy)
		return;

	showText(TX_SPEAKER_STEPHEN, 42, true);
	_awayMission->demon.stephenWelcomedToStudy = true;
}

void Room::demon6SpockReachedComputer() {
	showText(TX_SPEAKER_SPOCK, 14, true);
}

void Room::demon6WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.demon.movingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x94, 4);
}

void Room::demon6TouchedDoorOpenTrigger() {
	if (!_roomVar.demon.movingToDoor)
		return;
	loadActorAnim(10, "s0r6d1", 0xa0, 0x92, 2);
	playSoundEffectIndex(5);
}

void Room::demon6DoorOpenedOrReachedDoor() {
	_roomVar.demon.doorCounter++;
	if (_roomVar.demon.doorCounter == 2)
		loadRoomIndex(0, 2);
}

void Room::demon6UsePhaserOnStephen() {
	showText(TX_SPEAKER_STEPHEN, 28, true);
}

void Room::demon6UsePhaserOnCase() {
	showText(TX_SPEAKER_MCCOY, 17, true);
}

void Room::demon6LookAtWorkspace() {
	showDescription(3, true);
}

void Room::demon6LookAtCase() {
	showDescription(1, true);
}

void Room::demon6LookAtTable() {
	showDescription(7, true);
}

void Room::demon6LookAtMineral() {
	showDescription(16, true);
}

void Room::demon6LookAtShells() {
	showDescription(11, true);
}

void Room::demon6LookAtSkull() {
	showDescription(17, true);
}

void Room::demon6LookAtMetal() {
	showDescription(5, true);
}

void Room::demon6LookAtMeteor() {
	showDescription(0, true);
}

void Room::demon6LookAtMountain() {
	showDescription(2, true);
}

void Room::demon6LookAtSynthesizer() {
	showDescription(4, true);
}

void Room::demon6LookAtKirk() {
	showDescription(8, true);
}

void Room::demon6LookAtSpock() {
	showDescription(15, true);
}

void Room::demon6LookAtMccoy() {
	showDescription(9, true);
}

void Room::demon6LookAtRedshirt() {
	showDescription(10, true);
}

void Room::demon6LookAtComputer() {
	showDescription(6, true);
	if (!_awayMission->demon.lookedAtComputer) {
		_awayMission->demon.lookedAtComputer = true;
		_awayMission->demon.missionScore++;
	}
}

void Room::demon6LookAnywhere() {
	showDescription(24, true);
}

void Room::demon6LookAtStephen() {
	showDescription(21, true);
}

void Room::demon6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 20, true);
}

void Room::demon6TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 25, true);
}

void Room::demon6TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 2, true);
}

void Room::demon6TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, 53, true);
}

void Room::demon6TalkToStephen() {
	showText(TX_SPEAKER_STEPHEN, 35, true);
}

void Room::demon6UseBerryOnStephen() {
	if (_awayMission->demon.knowAboutHypoDytoxin) {
		showText(TX_SPEAKER_STEPHEN, 27, true);
	} else {
		showText(TX_SPEAKER_STEPHEN, 26, true);
	}
}

void Room::demon6UseBerryOnSynthesizer() {
	if (!_awayMission->demon.knowAboutHypoDytoxin)
		return;
	walkCrewman(OBJECT_MCCOY, 0x104, 0xa5, 1);
}

void Room::demon6MccoyReachedSynthesizer() {
	loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 1);
	playVoc("LD1REPLI");
}

void Room::demon6FinishedMakingHypoDytoxin() {
	playMidiMusicTracks(-1, -1);
	showDescription(23, true);
	showText(TX_SPEAKER_MCCOY, 18, true);

	giveItem(OBJECT_IDETOXIN);
	_awayMission->demon.madeHypoDytoxin = true;
	loseItem(OBJECT_IBERRY);

	if (!_awayMission->demon.gavePointsForDytoxin) {
		_awayMission->demon.gavePointsForDytoxin = true;
		_awayMission->demon.missionScore++;
	}
}

void Room::demon6UseHandOnWorkspace() {
	if (_awayMission->demon.repairedHand) {
		showText(TX_SPEAKER_SPOCK, 18, true); // FIXME
	} else {
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_KIRK, 5, true);
		walkCrewman(OBJECT_SPOCK, 0xb3, 0xbb, 2);
	}
}

void Room::demon6SpockReachedWorkspace() {
	loadActorAnim(OBJECT_SPOCK, "spokop", 0xb3, 0xbb, 3);
}

void Room::demon6SpockFinishedRepairingHand() {
	showText(TX_SPEAKER_SPOCK, 24, true);
	_awayMission->demon.repairedHand = true;
	_awayMission->demon.missionScore += 2;
	loadActorStandAnim(OBJECT_SPOCK);
	_awayMission->disableInput = false;
}

// FIXME: doesn't work
void Room::demon6UseAnythingOnWorkspace() {
	showDescription(20, true);
}

void Room::demon6StephenIsInsulted() {
	showText(TX_SPEAKER_STEPHEN, 30, true);
	_roomVar.demon.insultedStephenRecently = true;
	_awayMission->demon.insultedStephen = true;
}

void Room::demon6StephenDescribesItemsInCase() {
	while (true) {
		showText(TX_SPEAKER_STEPHEN, 51, true);
		TextRef choices2[] = { TX_SPEAKER_STEPHEN, 45, 46, 47, 48, 49, 50, TX_BLANK };
		TextRef choice = showMultipleTexts(choices2, true);

		switch (choice) {
		case 0:
			showText(TX_SPEAKER_STEPHEN, 40, true);
			break;
		case 1:
			showText(TX_SPEAKER_STEPHEN, 33, true);
			break;
		case 2:
			showText(TX_SPEAKER_STEPHEN, 36, true);
			break;
		case 3:
			showText(TX_SPEAKER_STEPHEN, 38, true);
			break;
		case 4:
			showText(TX_SPEAKER_STEPHEN, 39, true);
			break;
		case 5:
			showText(TX_SPEAKER_STEPHEN, 41, true);
			_roomVar.demon.caseOpened = true;
			return;
		default:
			showDescription(TX_DIALOG_ERROR);
		}
	}
}

void Room::demon6UseCrewmanOnCase() {
	if (_roomVar.demon.stephenInRoom) {
		if (_roomVar.demon.insultedStephenRecently)
			return;
		showText(TX_SPEAKER_STEPHEN, 44, true);
		TextRef choices1[] = {TX_SPEAKER_KIRK, 1, 6, 3, TX_BLANK};
		TextRef choice = showMultipleTexts(choices1, true);

		if (choice == 0) {
			demon6StephenIsInsulted();
		} else if (choice == 1) {
			showText(TX_SPEAKER_STEPHEN, 34, true);
			demon6StephenDescribesItemsInCase();
		} else if (choice == 2) {
			showText(TX_SPEAKER_STEPHEN, 31, true);
			TextRef choices3[] = {TX_SPEAKER_KIRK, 1, 6, TX_BLANK};
			choice = showMultipleTexts(choices3, true);

			if (choice == 0)
				demon6StephenIsInsulted();
			else if (choice == 1)
				demon6StephenDescribesItemsInCase();
			else
				showDescription(TX_DIALOG_ERROR);
		} else {
			showDescription(TX_DIALOG_ERROR);
		}
	} else {
		showDescription(12, true);
		showText(TX_SPEAKER_MCCOY, 19, true);
	}
}

void Room::demon6UseKirkOnComputer() {
	showText(TX_SPEAKER_KIRK, 4, true);
	demon6UseSpockOnComputer();
}

void Room::demon6UseMccoyOnComputer() {
	showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::demon6UseRedshirtOnComputer() {
	showText(TX_SPEAKER_EVERTS, 52, true);
}

void Room::demon6UseSpockOnComputer() {
	walkCrewman(OBJECT_SPOCK, 0x3d, 0xc5, 3);
}

void Room::demon6UseMineralOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~8;
	demon6ReturnItemToStephen(OBJECT_IMINERAL);
}

void Room::demon6UseShellsOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~2;
	demon6ReturnItemToStephen(OBJECT_ISHELLS);
}

void Room::demon6UseMeteorOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~4;
	demon6ReturnItemToStephen(OBJECT_IMETEOR);
}

void Room::demon6UseSkullOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~16;
	demon6ReturnItemToStephen(OBJECT_ISKULL);
}

void Room::demon6UseMetalOnStephen() {
	_awayMission->demon.itemsTakenFromCase &= ~1;
	demon6ReturnItemToStephen(OBJECT_IMETAL);
}

void Room::demon6ReturnItemToStephen(int item) {
	loseItem(item);
	showDescription(19, true);
	showText(TX_SPEAKER_STEPHEN, 29, true);
}

void Room::demon6UseHandOnStephen() {
	if (_awayMission->demon.repairedHand)
		showText(TX_SPEAKER_STEPHEN, 37, true);
	else
		showText(TX_SPEAKER_STEPHEN, 43, true);
}

void Room::demon6UseMTricoderOnStephen() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::demon6UseSTricoderOnTable() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 9, true);
}

void Room::demon6UseSTricoderOnComputer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::demon6UseSTricoderOnMineral() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 13, true);
}

void Room::demon6UseSTricoderOnShells() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 21, true);
}

void Room::demon6UseSTricoderOnSkull() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 12, true);
}

void Room::demon6UseSTricoderOnMetal() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 11, true);
}

void Room::demon6UseSTricoderOnMeteor() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 10, true);
}

void Room::demon6UseSTricoderOnCase() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 23, true);
}

void Room::demon6UseSTricoderOnSynthesizer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, 8, true);
}

void Room::demon6GetCase() {
	if (!_roomVar.demon.caseOpened)
		showDescription(22, true);
	else
		walkCrewman(OBJECT_KIRK, 0xff, 0xba, 5);
}

void Room::demon6KirkReachedCase() {
	const int items[] = { OBJECT_IMETAL, OBJECT_ISHELLS, OBJECT_IMETEOR, OBJECT_IMINERAL, OBJECT_ISKULL };
	byte ret = 0x1f ^ demon6ShowCase(_awayMission->demon.itemsTakenFromCase ^ 0x1f);
	_awayMission->demon.itemsTakenFromCase = ret;

	const int *i = items;
	while (ret != 0) {
		if (ret & 1)
			giveItem(*i);
		i++;
		ret >>= 1;
	}
}

int Room::demon6ShowCase(int visible) {
	const Common::Point spritePositions[] = {
		Common::Point(0x29, 0x80),
		Common::Point(0x58, 0x7d),
		Common::Point(0x82, 0x7d),
		Common::Point(0xae, 0x79),
		Common::Point(0xe1, 0x7d)
	};

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->pushSprites();

	_vm->_gfx->setBackgroundImage(_vm->_gfx->loadBitmap("stlcase"));
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->clearPri();

	Sprite sprites[5];

	for (int i = 0; i < 5; i++) {
		if (!(visible & (0x10 >> i)))
			continue;
		_vm->_gfx->addSprite(&sprites[i]);
		sprites[i].pos.x = spritePositions[i].x;
		sprites[i].pos.y = spritePositions[i].y;
		sprites[i].drawPriority = 2;
		sprites[i].bitmapChanged = true;
		sprites[i].bitmap = _vm->_gfx->loadBitmap(Common::String::format("stlitem%d", i));
	}

	Sprite buttonSprite;

	// BUGFIX: use draw mode 2 so the entire button is clickable (not just non-transparent parts)
	buttonSprite.drawMode = 2;

	buttonSprite.pos.x = 0xe1;
	buttonSprite.pos.y = 0x19;
	buttonSprite.drawPriority = 2;
	buttonSprite.bitmapChanged = true;
	buttonSprite.bitmap = _vm->_gfx->loadBitmap("donebutt");

	_vm->_gfx->addSprite(&buttonSprite);
	_vm->_gfx->forceDrawAllSprites();
	_vm->_gfx->fadeinScreen();

	visible = demon6ShowCaseProcessInput(sprites, &buttonSprite, visible);

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->popSprites();
	_vm->_gfx->loadPri(_vm->getScreenName());
	_vm->_gfx->setBackgroundImage(_vm->_gfx->loadBitmap(_vm->getScreenName()));
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->forceDrawAllSprites();

	return visible;
}

int Room::demon6ShowCaseProcessInput(Sprite *sprites, Sprite *buttonSprite, int visible) {
	bool exitLoop = false;

	while (!exitLoop) {
		TrekEvent event;
		if (!_vm->popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK:
			_vm->_gfx->drawAllSprites();
			break;

		case TREKEVENT_LBUTTONDOWN: {
			Sprite *clickedSprite = _vm->_gfx->getSpriteAt(_vm->_gfx->getMousePos());
			if (clickedSprite == buttonSprite)
				exitLoop = true;
			else if (clickedSprite != nullptr)
				visible = demon6ShowCaseProcessSelection(sprites, clickedSprite, visible);
			break;
		}

		case TREKEVENT_RBUTTONDOWN:
			exitLoop = true;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				exitLoop = true;
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1: {
				Sprite *clickedSprite = _vm->_gfx->getSpriteAt(_vm->_gfx->getMousePos());
				if (clickedSprite == buttonSprite)
					exitLoop = true;
				else if (clickedSprite != nullptr)
					visible = demon6ShowCaseProcessSelection(sprites, clickedSprite, visible);
				break;
			}

			case Common::KEYCODE_F2:
				exitLoop = true;
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
	}

	return visible;
}

int Room::demon6ShowCaseProcessSelection(Sprite *sprites, Sprite *clickedSprite, int visible) {
	while (clickedSprite->pos.y < SCREEN_HEIGHT) { // Move the selected item down and off the screen.
		clickedSprite->drawPriority = 8;
		clickedSprite->bitmapChanged = true;
		_vm->_gfx->drawAllSprites();

		// WORKAROUND: original game had no bound on how fast the items move
		// off the screen. Here I bind it to the tick event.
		// (This was probably the intended behaviour since the original game
		// does check the clock cycle, but doesn't do a proper comparison with
		// it.)
		TrekEvent event;
		do {
			if (!_vm->popNextEvent(&event))
				continue;
		} while (event.type != TREKEVENT_TICK);
		clickedSprite->pos.y++;
	}

	clickedSprite->dontDrawNextFrame();
	_vm->_gfx->drawAllSprites();
	_vm->_gfx->delSprite(clickedSprite);
	clickedSprite->bitmap.reset();
	int i = clickedSprite - sprites;
	visible ^= (0x10 >> i);
	
	return visible;
}

}
