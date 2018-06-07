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

// TODO: this room should have animations on computer terminals, but this isn't where
// they're implemented. Could be related to ".BAN" files which rooms can load?

void Room::demon4Tick1() {
	playVoc("DEM4LOOP");

	if (!_vm->_awayMission.demon.healedMiner)
		_vm->_awayMission.demon.minerDead = true;

	if (_vm->_awayMission.demon.solvedSunPuzzle) {
		loadActorAnim(10, "bxrise2", 0x122, 0x91, 0);
		loadActorAnim(8, "stpout2", 0x107, 0x92, 0);
		_roomVar.demon4.nauianEmerged = true;

		if ((_vm->_awayMission.demon.itemsTakenFromCase & 0x10) && !_vm->_awayMission.demon.gaveSkullToNauian)
			_vm->_awayMission.timers[6] = 20;
	}

	if (!_vm->_awayMission.demon.foundAlienRoom) {
		_vm->_awayMission.demon.foundAlienRoom = true;
		playMidiMusicTracks(1, -1);
	}
}

// Alien emerged
void Room::demon4FinishedAnimation1() {
	loadActorAnim(8, "stpout", 0x107, 0x92, 5);
	_roomVar.demon4.nauianEmerged = true;
	if (_vm->_awayMission.demon.itemsTakenFromCase & 0x10)
		_vm->_awayMission.timers[6] = 45;
}

void Room::demon4FinishedAnimation2() {
	if (!_roomVar.demon4.cd) {
		_roomVar.demon4.cd = true;
		showText(TX_DEM4N011);
	}

	showGameOverMenu();

	// WORKAROUND: original game has the below line; however, it's never executed anyway
	// since the game over menu manipulates the stack. Here, the menu could delete this
	// room object, so it should be the last statement...
	//_vm->_awayMission.disableInput = false;
}

void Room::demon4FinishedAnimation3() {
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_034);
	showText(TX_SPEAKER_KIRK,   TX_DEM4_010);
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_035);
	showText(TX_SPEAKER_SPOCK,  TX_DEM4_030);

	_vm->_awayMission.disableInput = true;
	_vm->_awayMission.demon.missionScore += 5;
	_vm->_awayMission.timers[1] = 20;
}

// Just solved sun puzzle
void Room::demon4Timer0Expired() {
	loadActorAnim(10, "bxrise", 0x122, 0x91, 1);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);
	playVoc("SE2POWER");
}

// Mission end
void Room::demon4Timer1Expired() {
	showText(TX_SPEAKER_KIRK, TX_DEM4_011);

	walkCrewman(OBJECT_KIRK, 0xbe, 0xa5, 4);
	walkCrewman(OBJECT_SPOCK, 0xb4, 0x9b, 4);
	walkCrewman(OBJECT_MCCOY, 0xc8, 0x9b, 4);

	if (!_vm->_awayMission.redshirtDead)
		walkCrewman(OBJECT_REDSHIRT, 0xc1, 0x91, 4);
	else
		_roomVar.demon4.crewReadyToBeamOut++;
}

void Room::demon4CrewmanReachedBeamoutPosition() {
	_roomVar.demon4.crewReadyToBeamOut++;
	if (_roomVar.demon4.crewReadyToBeamOut != 4)
		return;

	if (!_vm->_awayMission.demon.insultedStephen)
		_vm->_awayMission.demon.missionScore += 3;
	if (!_vm->_awayMission.redshirtDead)
		_vm->_awayMission.demon.missionScore += 2;

	endMission(_vm->_awayMission.demon.missionScore, 0x24, 0);
}

void Room::demon4Timer2Expired() {
	loadActorAnim2(OBJECT_SPOCK, "skille", -1, -1, 0);
}

void Room::demon4Timer3Expired() {
	loadActorAnim2(OBJECT_REDSHIRT, "rkille", -1, -1, 0);
}

void Room::demon4Timer4Expired() {
	loadActorAnim2(OBJECT_MCCOY, "mkille", -1, -1, 0);
}

void Room::demon4Timer5Expired() {
	loadActorAnim2(OBJECT_KIRK, "kkille", -1, -1, 2);
}

void Room::demon4Timer6Expired() {
	showText(TX_SPEAKER_MCCOY, TX_DEM4_017);
}

void Room::demon4UsePhaserOnNauian() {
	if (!_roomVar.demon4.triedToShootNauian) { // Lenient the first time
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_033);
		_roomVar.demon4.triedToShootNauian = true;
	}
	else { // Murdery the second time
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_032);
		loadActorAnim2(10, "bxfire", 0, 0, 2);
		playVoc("V0SPOCKT");
		_vm->_awayMission.disableInput = true;
		_vm->_awayMission.timers[2] = 7;
		_vm->_awayMission.timers[3] = 8;
		_vm->_awayMission.timers[4] = 7;
		_vm->_awayMission.timers[5] = 8;
	}
}

void Room::demon4UseMetalOnSecurityEquipment() {
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xf5, 0x90, 5);
}

void Room::demon4KirkReachedSecurityEquipment() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 4);
}

void Room::demon4KirkFinishedUsingSecurityEquipment() {
	_vm->_awayMission.disableInput = false;
	showText(TX_DEM4N014);
}

void Room::demon4UseMetalOnNauian() {
	walkCrewman(OBJECT_KIRK, 0xe9, 0x90, 1);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
}

void Room::demon4KirkReachedNauian() {
	loadActorAnim2(8, "usekey", 0x107, 0x8e, 3);
	loseItem(OBJECT_IMETAL);
	_vm->_awayMission.demon.missionScore += 2;
	_vm->_awayMission.demon.itemsTakenFromCase &= ~1;
}

void Room::demon4UseSkullOnNauian() {
	walkCrewman(OBJECT_KIRK, 0xe9, 0x90, 2);
}

void Room::demon4KirkReachedNauianWithSkull() {
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_036);

	const int choices[] = { TX_SPEAKER_KIRK, TX_DEM4_006, TX_DEM4_003, TX_DEM4_005, TX_BLANK };
	int choice = showText(choices);

	switch (choice) {
	case 0:
		_vm->_awayMission.demon.missionScore++;
		loadActorAnim2(8, "takesk", 0x107, 0x8e, 0);
		loseItem(OBJECT_ISKULL);
		_vm->_awayMission.demon.itemsTakenFromCase &= ~16; // BUG: skull reappears in case? Can abuse for infinite ponits?
		_vm->_awayMission.demon.gaveSkullToNauian = true;
		break;

	case 1:
	case 2:
		break;

	default:
		showText(TX_DIALOG_ERROR);
		break;
	}
}

void Room::demon4UsePhaserOnPanel() {
	showText(TX_SPEAKER_SPOCK, TX_DEM4_013);
}

void Room::demon4UsePhaserOnPattern() {
	showText(TX_SPEAKER_SPOCK, TX_DEM4_045);
}

void Room::demon4UsePhaserOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_DEM4_018);
}

void Room::demon4TalkToNauian() {
	_vm->_awayMission.disableInput = false;

	if (!_vm->_awayMission.demon.metNauian) {
		_vm->_awayMission.demon.metNauian = true;
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_044);

		const int choices[] = { TX_SPEAKER_KIRK, TX_DEM4_002, TX_DEM4_004, TX_DEM4_009, TX_BLANK };
		int choice = showText(choices);

		switch (choice) {
		case 0:
			_vm->_awayMission.demon.field2d = true;
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_042);
			break;

		case 1:
			break;

		case 2:
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_039);
			break;

		default:
			showText(TX_DIALOG_ERROR);
			break;
		}

		showText(TX_SPEAKER_NAUIAN, TX_DEM4_043);

		const int choices2[] = { TX_SPEAKER_KIRK, TX_DEM4_008, TX_DEM4_007, TX_BLANK };
		choice = showText(choices2);

		switch (choice) {
		case 0:
			_vm->_awayMission.demon.field2d = true;
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_041);
			break;
		case 1:
			break;
		default:
			showText(TX_DIALOG_ERROR);
			break;
		}

		showText(TX_SPEAKER_NAUIAN, TX_DEM4_040);

		if (_roomVar.demon4.disabledSecurity) { // Impossible condition?
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_037);

			// BUGFIX: Originally all of these used no audio, despite the files existing
			// (being used elsewhere).
			showText(TX_SPEAKER_KIRK,   TX_DEM4_010);
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_035);
			showText(TX_SPEAKER_SPOCK,  TX_DEM4_030);

			_vm->_awayMission.disableInput = true;
			_vm->_awayMission.demon.missionScore += 5;
			_vm->_awayMission.timers[1] = 20;
		}
		else {
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_038);
		}
	}
	else {
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_031);
	}
}

void Room::demon4LookAtPattern() {
	showText(TX_DEM4N002);
}

void Room::demon4LookAtAlien() {
	showText(TX_DEM4N007);
}

void Room::demon4LookAnywhere() {
	showText(TX_DEM4N004);
}

void Room::demon4LookAtSecurityEquipment() {
	showText(TX_DEM4N008);
}

void Room::demon4LookAtFloor() {
	showText(TX_DEM4N009);
}

void Room::demon4LookAtKirk() {
	showText(TX_DEM4N003);
}

void Room::demon4LookAtMccoy() {
	showText(TX_DEM4N000);
}

void Room::demon4LookAtSpock() {
	showText(TX_DEM4N005);
}

void Room::demon4LookAtRedshirt() {
	showText(TX_DEM4N001);
}

void Room::demon4LookAtChamber() {
	showText(TX_DEM4N006);
}

void Room::demon4LookAtPanel() {
	showText(TX_DEM4N012);
}

void Room::demon4UseKirkOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_KIRK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseSpockOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_SPOCK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseMccoyOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_MCCOY;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseRedshirtOnPanel() {
	_roomVar.demon4.crewmanUsingPanel = OBJECT_REDSHIRT;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseCrewmanOnPanel() {
	if (_vm->_awayMission.demon.solvedSunPuzzle)
		return;
	walkCrewman(_roomVar.demon4.crewmanUsingPanel, 0xda, 0x83, 3);
	_vm->_awayMission.crewDirectionsAfterWalk[_roomVar.demon4.crewmanUsingPanel] = DIR_N;
	_vm->_awayMission.disableInput = true;
}

void Room::demon4CrewmanReachedPanel() {
	if (demon4ShowSunPuzzle()) {
		_vm->_awayMission.demon.solvedSunPuzzle = true;
		loadActorAnim(9, "ctrl", 0, 0, 0);
		_vm->_awayMission.demon.missionScore += 3;
		_vm->_awayMission.timers[0] = 10;
	}
	else
		showText(TX_DEM4N013);

	walkCrewman(_roomVar.demon4.crewmanUsingPanel, 0xae, 0x93, 0);
	_vm->_awayMission.disableInput = false;
}

bool Room::demon4ShowSunPuzzle() {
	bool solved = false;

	int sliderY = 17;
	int sliderR = -17;
	int sliderB = 17;

	_vm->_gfx->fadeoutScreen();
	_vm->_sound->stopAllVocSounds();

	_vm->_gfx->pushSprites();
	_vm->_gfx->setBackgroundImage(_vm->_gfx->loadBitmap("machineb"));
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->clearPri();

	Sprite sprites[21];

	for (int i = 0; i < 21; i++) {
		_vm->_gfx->addSprite(&sprites[i]);
	}

	sprites[0].setBitmap(_vm->_gfx->loadBitmap("levery"));
	sprites[1].setBitmap(_vm->_gfx->loadBitmap("leverr"));
	sprites[2].setBitmap(_vm->_gfx->loadBitmap("leverb"));

	SharedPtr<Bitmap> lightyBitmap = _vm->_gfx->loadBitmap("lighty");
	SharedPtr<Bitmap> lightrBitmap = _vm->_gfx->loadBitmap("lightr");
	SharedPtr<Bitmap> lightbBitmap = _vm->_gfx->loadBitmap("lightb");

	for (int i = 3; i < 9; i++)
		sprites[i].bitmap = lightyBitmap;
	for (int i = 9; i < 15; i++)
		sprites[i].bitmap = lightrBitmap;
	for (int i = 15; i < 21; i++)
		sprites[i].bitmap = lightbBitmap;

	Sprite doneButtonSprite;
	_vm->_gfx->addSprite(&doneButtonSprite);
	doneButtonSprite.setXYAndPriority(0x104, 0x64, 2);
	doneButtonSprite.setBitmap(_vm->_gfx->loadBitmap("donebutt"));
	// BUGFIX: use draw mode 2 so the entire button is clickable (not just non-transparent
	// pixels)
	doneButtonSprite.drawMode = 2;

	_vm->_gfx->fadeinScreen();

	bool continueLoop = true;
	bool sliderChanged = true;

	while (continueLoop) {
		if (sliderChanged) {
			sprites[0].setXYAndPriority(0xa0, sliderY + 0x75, 14);
			sprites[1].setXYAndPriority(0xa8, sliderR + 0x75, 14);
			sprites[2].setXYAndPriority(0xb0, sliderB + 0x75, 14);

			int var56 = 0x82;
			int var58 = 0x83;
			int var5a = 3;
			int numSprites = abs(sliderY) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
			}
			var56 -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
				var56 -= 6;
			}

			var56 = 0x82;
			var58 = 0x8b;
			var5a = 9;
			numSprites = abs(sliderR) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
			}
			var56 -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
				var56 -= 6;
			}

			var56 = 0x82;
			var58 = 0x93;
			var5a = 15;
			numSprites = abs(sliderB) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
			}
			var56 -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + var5a].setXYAndPriority(var58, var56, 14);
				var56 -= 6;
			}

			sliderChanged = false;
		}

		TrekEvent event;
		if (!_vm->popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_LBUTTONDOWN: {
lclick:
			Common::Point mousePos = _vm->_gfx->getMousePos();
			if (_vm->_gfx->getSpriteAt(mousePos) == &doneButtonSprite)
				goto done;
			else {
				if (mousePos.y >= 0x64 && mousePos.y <= 0x86) {
					if (mousePos.x >= 0xa0 && mousePos.x <= 0xa6)
						sliderY = mousePos.y - 0x75;
					else if (mousePos.x >= 0xa8 && mousePos.x <= 0xae)
						sliderR = mousePos.y - 0x75;
					else if (mousePos.x >= 0xb0 && mousePos.x <= 0xb6)
						sliderB = mousePos.y - 0x75;
					sliderChanged = true;
				}
			}
			break;
		}

		case TREKEVENT_RBUTTONDOWN:
done:
			if (abs(sliderY) <= 2 && abs(sliderR) <= 2 && abs(sliderB) <= 2)
				solved = true;
			else
				solved = false;
			continueLoop = false;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_F2:
				goto done;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				goto lclick;

			default:
				break;
			}
			break;

		default:
			break;
		}

		_vm->_gfx->drawAllSprites();
	}

	_vm->_gfx->fadeoutScreen();
	_vm->_gfx->popSprites();
	_vm->_gfx->loadPri(_vm->_screenName);
	_vm->_gfx->setBackgroundImage(_vm->_gfx->loadBitmap(_vm->_screenName));
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->forceDrawAllSprites();

	return solved;
}

void Room::demon4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM4_001);
}

void Room::demon4TalkToMccoy() {
	if (_roomVar.demon4.nauianEmerged)
		showText(TX_SPEAKER_MCCOY, TX_DEM4_019);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM4_015);
}

void Room::demon4TalkToSpock() {
	if (_roomVar.demon4.nauianEmerged)
		showText(TX_SPEAKER_SPOCK, TX_DEM4_022);
	else
		showText(TX_SPEAKER_SPOCK, TX_DEM4_024);
}

void Room::demon4TalkToRedshirt() {
	if (_roomVar.demon4.nauianEmerged)
		showText(TX_SPEAKER_EVERTS, TX_DEM4_047);
	else
		showText(TX_SPEAKER_EVERTS, TX_DEM4_046);
}

void Room::demon4UseSTricorderOnChamber() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_012);
}

void Room::demon4UseSTricorderOnPattern() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_014);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_025);
}

void Room::demon4UseMTricorderOnPattern() {
	showText(TX_SPEAKER_MCCOY, TX_DEM4_021);
}

void Room::demon4UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_027);
}

void Room::demon4UseMTricorderOnPanel() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_MCCOY, TX_DEM4_027); // BUG: Uses spock's voice (though speaker says "Mccoy")
}

void Room::demon4UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_023);
}

void Room::demon4UseMTricorderOnAnything() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_MCCOY, TX_DEM4_020);
}

void Room::demon4UseSTricorderOnNauian() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_029);
}

void Room::demon4UseMTricorderOnNauian() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_029); // BUG: Uses spock's voice (speaker says "Spock" too)
}

void Room::demon4UseSTricorderOnSecurityEquipment() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_026);
}

void Room::demon4UseMTricorderOnSecurityEquipment() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(4);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_026); // BUG: Uses spock's voice (speaker says "Spock" too)
}

}
