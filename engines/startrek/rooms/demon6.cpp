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

namespace StarTrek {

void Room::demon6Tick1() {
	playVoc("DEM6LOOP");
	if (_vm->_awayMission.demon.gotBerries || (!_vm->_awayMission.demon.field3e && _vm->_awayMission.demon.field37)) {
		loadActorAnim(8, "oldman", 0x5f, 0xb1, 0);
		_roomVar.demon6.stephenInRoom = true;
	}
	loadActorAnim(10, "s0r6d2", 0xa0, 0x92, 0);
	loadActorAnim(9, "scrnan", 0, 0xc7, 0);
}

void Room::demon6Tick30() {
	if (!_vm->_awayMission.demon.gotBerries)
		return;
	if (_vm->_awayMission.demon.field3e)
		return;
	if (!_vm->_awayMission.demon.field37)
		return;
	if (_vm->_awayMission.demon.stephenWelcomedToStudy)
		return;

	showText(TX_SPEAKER_STEPHEN, TX_DEM6_042);
	_vm->_awayMission.demon.stephenWelcomedToStudy = true;
}

void Room::demon6SpockReachedComputer() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_014);
}

void Room::demon6WalkToDoor() {
	_vm->_awayMission.disableInput = true;
	_roomVar.demon6.movingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x94, 4);
}

void Room::demon6TouchedDoorOpenTrigger() {
	if (!_roomVar.demon6.movingToDoor)
		return;
	loadActorAnim(10, "s0r6d1", 0xa0, 0x92, 2);
	playSoundEffectIndex(5);
}

void Room::demon6DoorOpenedOrReachedDoor() {
	_roomVar.demon6.doorCounter++;
	if (_roomVar.demon6.doorCounter == 2)
		loadRoomIndex(0, 2);
}

void Room::demon6UsePhaserOnStephen() {
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_028);
}

void Room::demon6UsePhaserOnCase() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_017);
}

void Room::demon6LookAtWorkspace() {
	showText(TX_DEM6N003);
}

void Room::demon6LookAtCase() {
	showText(TX_DEM6N001);
}

void Room::demon6LookAtTable() {
	showText(TX_DEM6N007);
}

void Room::demon6LookAtMineral() {
	showText(TX_DEM6N016);
}

void Room::demon6LookAtShells() {
	showText(TX_DEM6N011);
}

void Room::demon6LookAtSkull() {
	showText(TX_DEM6N017);
}

void Room::demon6LookAtMetal() {
	showText(TX_DEM6N005);
}

void Room::demon6LookAtMeteor() {
	showText(TX_DEM6N000);
}

void Room::demon6LookAtMountain() {
	showText(TX_DEM6N002);
}

void Room::demon6LookAtSynthesizer() {
	showText(TX_DEM6N004);
}

void Room::demon6LookAtKirk() {
	showText(TX_DEM6N008);
}

void Room::demon6LookAtSpock() {
	showText(TX_DEM6N015);
}

void Room::demon6LookAtMccoy() {
	showText(TX_DEM6N009);
}

void Room::demon6LookAtRedshirt() {
	showText(TX_DEM6N010);
}

void Room::demon6LookAtComputer() {
	showText(TX_DEM6N006);
	if (!_vm->_awayMission.demon.lookedAtComputer) {
		_vm->_awayMission.demon.lookedAtComputer = true;
		_vm->_awayMission.demon.missionScore++;
	}
}

void Room::demon6LookAnywhere() {
	showText(TX_DEM6N024);
}

void Room::demon6LookAtStephen() {
	showText(TX_DEM6N021);
}

void Room::demon6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_020);
}

void Room::demon6TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_025);
}

void Room::demon6TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM6_002);
}

void Room::demon6TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, TX_DEM6_053);
}

void Room::demon6TalkToStephen() {
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_035);
}

void Room::demon6UseBerryOnStephen() {
	if (_vm->_awayMission.demon.knowAboutHypoDytoxin) {
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_027);
	}
	else {
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_026);
	}
}

void Room::demon6UseBerryOnSynthesizer() {
	if (!_vm->_awayMission.demon.knowAboutHypoDytoxin)
		return;
	walkCrewman(OBJECT_MCCOY, 0x104, 0xa5, 1);
}

void Room::demon6MccoyReachedSynthesizer() {
	loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 1);
	playVoc("LD1REPLI");
}

void Room::demon6FinishedMakingHypoDytoxin() {
	playMidiMusicTracks(-1, -1);
	showText(TX_DEM6N023);
	showText(TX_SPEAKER_MCCOY, TX_DEM6_018);

	giveItem(OBJECT_IDETOXIN);
	_vm->_awayMission.demon.madeHypoDytoxin = true;
	loseItem(OBJECT_IBERRY);

	if (!_vm->_awayMission.demon.gavePointsForDytoxin) {
		_vm->_awayMission.demon.gavePointsForDytoxin = true;
		_vm->_awayMission.demon.missionScore++;
	}
}

void Room::demon6UseHandOnWorkspace() {
	if (_vm->_awayMission.demon.repairedHand) {
		showText(TX_SPEAKER_SPOCK, TX_DEM6N018); // FIXME
	}
	else {
		_vm->_awayMission.disableInput = true;
		showText(TX_SPEAKER_KIRK, TX_DEM6_005);
		walkCrewman(OBJECT_SPOCK, 0xb3, 0xbb, 2);
	}
}

void Room::demon6SpockReachedWorkspace() {
	loadActorAnim(OBJECT_SPOCK, "spokop", 0xb3, 0xbb, 3);
}

void Room::demon6SpockFinishedRepairingHand() {
	showText(TX_SPEAKER_SPOCK, TX_DEM6_024);
	_vm->_awayMission.demon.repairedHand = true;
	_vm->_awayMission.demon.missionScore += 2;
	loadActorStandAnim(OBJECT_SPOCK);
	_vm->_awayMission.disableInput = false;
}

// FIXME: doesn't work
void Room::demon6UseAnythingOnWorkspace() {
	showText(TX_DEM6N020);
}

void Room::demon6UseCrewmanOnCase() {
	if (_roomVar.demon6.stephenInRoom) {
		if (_roomVar.demon6.insultedStephenRecently)
			return;
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_044);
		int choices1[] = {TX_SPEAKER_KIRK, TX_DEM6_001, TX_DEM6_006, TX_DEM6_003, TX_BLANK};
		int choice = showText(choices1);

		if (choice == 0) {
insult:
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_030);
			_roomVar.demon6.insultedStephenRecently = true;
			_vm->_awayMission.demon.insultedStephen = true;
		}
		else if (choice == 1) {
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_034);
explain:
			while (true) {
				showText(TX_SPEAKER_STEPHEN, TX_DEM6_051);
				int choices2[] = {TX_SPEAKER_STEPHEN, TX_DEM6_045, TX_DEM6_046, TX_DEM6_047, TX_DEM6_048, TX_DEM6_049, TX_DEM6_050, TX_BLANK};
				choice = showText(choices2);

				switch (choice) {
				case 0:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_040);
					break;
				case 1:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_033);
					break;
				case 2:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_036);
					break;
				case 3:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_038);
					break;
				case 4:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_039);
					break;
				case 5:
					showText(TX_SPEAKER_STEPHEN, TX_DEM6_041);
					_roomVar.demon6.caseOpened = true;
					return;
				default:
					goto error;
				}
			}
		}
		else if (choice == 2) {
			showText(TX_SPEAKER_STEPHEN, TX_DEM6_031);
			int choices3[] = {TX_SPEAKER_KIRK, TX_DEM6_001, TX_DEM6_006, TX_BLANK};
			choice = showText(choices3);

			if (choice == 0)
				goto insult;
			else if (choice == 1)
				goto explain;
			else
				goto error;
		}
		else {
error:
			showText(TX_DIALOG_ERROR);
		}
	}
	else {
		showText(TX_DEM6N012);
		showText(TX_SPEAKER_MCCOY, TX_DEM6_019);
	}
}

void Room::demon6UseKirkOnComputer() {
	showText(TX_SPEAKER_KIRK, TX_DEM6_004);
	demon6UseSpockOnComputer();
}

void Room::demon6UseMccoyOnComputer() {
	showText(TX_SPEAKER_MCCOY, TX_DEM6_015);
}

void Room::demon6UseRedshirtOnComputer() {
	showText(TX_SPEAKER_EVERTS, TX_DEM6_052);
}

void Room::demon6UseSpockOnComputer() {
	walkCrewman(OBJECT_SPOCK, 0x3d, 0xc5, 3);
}

void Room::demon6UseMineralOnStephen() {
	_vm->_awayMission.demon.itemsTakenFromCase &= ~8;
	demon6ReturnItemToStephen(OBJECT_IMINERAL);
}

void Room::demon6UseShellsOnStephen() {
	_vm->_awayMission.demon.itemsTakenFromCase &= ~2;
	demon6ReturnItemToStephen(OBJECT_ISHELLS);
}

void Room::demon6UseMeteorOnStephen() {
	_vm->_awayMission.demon.itemsTakenFromCase &= ~4;
	demon6ReturnItemToStephen(OBJECT_IMETEOR);
}

void Room::demon6UseSkullOnStephen() {
	_vm->_awayMission.demon.itemsTakenFromCase &= ~16;
	demon6ReturnItemToStephen(OBJECT_ISKULL);
}

void Room::demon6UseMetalOnStephen() {
	_vm->_awayMission.demon.itemsTakenFromCase &= ~1;
	demon6ReturnItemToStephen(OBJECT_IMETAL);
}

void Room::demon6ReturnItemToStephen(int item) {
	loseItem(item);
	showText(TX_DEM6N019);
	showText(TX_SPEAKER_STEPHEN, TX_DEM6_029);
}

void Room::demon6UseHandOnStephen() {
	if (_vm->_awayMission.demon.repairedHand)
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_037);
	else
		showText(TX_SPEAKER_STEPHEN, TX_DEM6_043);
}

void Room::demon6UseMTricoderOnStephen() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_MCCOY, TX_DEM6_016);
}

void Room::demon6UseSTricoderOnTable() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_009);
}

void Room::demon6UseSTricoderOnComputer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_007);
}

void Room::demon6UseSTricoderOnMineral() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_013);
}

void Room::demon6UseSTricoderOnShells() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_021);
}

void Room::demon6UseSTricoderOnSkull() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_012);
}

void Room::demon6UseSTricoderOnMetal() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_011);
}

void Room::demon6UseSTricoderOnMeteor() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_010);
}

void Room::demon6UseSTricoderOnCase() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_023);
}

void Room::demon6UseSTricoderOnSynthesizer() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM6_008);
}

void Room::demon6GetCase() {
	if (!_roomVar.demon6.caseOpened)
		showText(TX_DEM6N022);
	else
		walkCrewman(OBJECT_KIRK, 0xff, 0xba, 5);
}

void Room::demon6KirkReachedCase() {
	const int items[] = { OBJECT_IMETAL, OBJECT_ISHELLS, OBJECT_IMETEOR, OBJECT_IMINERAL, OBJECT_ISKULL };
	byte ret = 0x1f ^ demon6ShowCase(_vm->_awayMission.demon.itemsTakenFromCase ^ 0x1f);
	_vm->_awayMission.demon.itemsTakenFromCase = ret;

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
	_vm->_gfx->addSprite(&buttonSprite);

	// BUGFIX: use draw mode 2 so the entire button is clickable (not just non-transparent parts)
	buttonSprite.drawMode = 2;

	buttonSprite.pos.x = 0xe1;
	buttonSprite.pos.y = 0x19;
	buttonSprite.drawPriority = 2;
	buttonSprite.bitmapChanged = true;
	buttonSprite.bitmap = _vm->_gfx->loadBitmap("donebutt");

	_vm->_gfx->forceDrawAllSprites();
	_vm->_gfx->fadeinScreen();

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
lclick:
			Sprite *clickedSprite = _vm->_gfx->getSpriteAt(_vm->_gfx->getMousePos());
			if (clickedSprite == &buttonSprite)
				exitLoop = true;
			else if (clickedSprite != nullptr) {
				while (clickedSprite->pos.y < SCREEN_HEIGHT) { // Move the selected item down and off the screen.
					clickedSprite->drawPriority = 8;
					clickedSprite->bitmapChanged = true;
					_vm->_gfx->drawAllSprites();

					// WORKAROUND: original game had no bound on how fast the items move
					// off the screen. Here I bind it to the tick event.
					// (This was probably the intended behaviour since the original game
					// does check the clock cycle, but doesn't do a proper comparison with
					// it.)
					while (event.type != TREKEVENT_TICK) {
						if (!_vm->popNextEvent(&event))
							continue;
					}
					clickedSprite->pos.y++;
				}

				clickedSprite->dontDrawNextFrame();
				_vm->_gfx->drawAllSprites();
				_vm->_gfx->delSprite(clickedSprite);
				clickedSprite->bitmap.reset();
				int i = clickedSprite - sprites;
				visible ^= (0x10 >> i);
			}
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
			case Common::KEYCODE_F1:
				goto lclick;

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

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->popSprites();
	_vm->_gfx->loadPri(_vm->_screenName);
	_vm->_gfx->setBackgroundImage(_vm->_gfx->loadBitmap(_vm->_screenName));
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->forceDrawAllSprites();

	return visible;
}

}
