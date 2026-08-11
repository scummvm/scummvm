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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

extern const RoomAction demon4ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::demon4Tick1 },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::demon4FinishedAnimation1 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::demon4FinishedAnimation2 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::demon4FinishedAnimation3 },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::demon4Timer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::demon4Timer1Expired },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::demon4CrewmanReachedBeamoutPosition },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::demon4Timer2Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0}, &Room::demon4Timer3Expired },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0, 0}, &Room::demon4Timer4Expired },
	{ {ACTION_TIMER_EXPIRED, 5, 0, 0, 0}, &Room::demon4Timer5Expired },
	{ {ACTION_TIMER_EXPIRED, 6, 0, 0, 0}, &Room::demon4Timer6Expired },

	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0, 0}, &Room::demon4UsePhaserOnNauian },
	{ {ACTION_USE, OBJECT_IPHASERS, 8, 0, 0}, &Room::demon4UsePhaserOnNauian },
	{ {ACTION_USE, OBJECT_IMETAL, 0x22, 0, 0}, &Room::demon4UseMetalOnSecurityEquipment },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::demon4KirkReachedSecurityEquipment },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::demon4KirkFinishedUsingSecurityEquipment },
	{ {ACTION_USE, OBJECT_IMETAL, 8, 0, 0}, &Room::demon4UseMetalOnNauian },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::demon4KirkReachedNauian },
	{ {ACTION_USE, OBJECT_ISKULL, 8, 0, 0}, &Room::demon4UseSkullOnNauian },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::demon4KirkReachedNauianWithSkull },

	{ {ACTION_USE, OBJECT_IPHASERS, 0x21, 0, 0}, &Room::demon4UsePhaserOnPanel },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x20, 0, 0}, &Room::demon4UsePhaserOnPattern },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_MCCOY, 0, 0}, &Room::demon4UsePhaserOnMccoy },

	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::demon4TalkToNauian },
	{ {ACTION_TALK, 8, 0, 0, 0}, &Room::demon4TalkToNauian },

	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::demon4LookAtPattern },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::demon4LookAtAlien },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::demon4LookAnywhere },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::demon4LookAtSecurityEquipment },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::demon4LookAtFloor },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::demon4LookAtKirk },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon4LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon4LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon4LookAtRedshirt },

	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::demon4LookAtChamber },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::demon4LookAtPanel },

	{ {ACTION_USE, OBJECT_KIRK, 0x21, 0, 0}, &Room::demon4UseKirkOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0, 0}, &Room::demon4UseSpockOnPanel },
	{ {ACTION_USE, OBJECT_MCCOY, 0x21, 0, 0}, &Room::demon4UseMccoyOnPanel },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x21, 0, 0}, &Room::demon4UseRedshirtOnPanel },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::demon4CrewmanReachedPanel },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::demon4TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon4TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon4TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon4TalkToRedshirt },
	{ {ACTION_USE, OBJECT_ISTRICOR, 10, 0, 0}, &Room::demon4UseSTricorderOnChamber },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::demon4UseSTricorderOnPattern },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x20, 0, 0}, &Room::demon4UseMTricorderOnPattern },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::demon4UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0, 0}, &Room::demon4UseMTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::demon4UseSTricorderOnAnything },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::demon4UseSTricorderOnAnything },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x23, 0, 0}, &Room::demon4UseMTricorderOnAnything },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::demon4UseMTricorderOnAnything },
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0, 0}, &Room::demon4UseSTricorderOnNauian },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0, 0}, &Room::demon4UseMTricorderOnNauian },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0, 0}, &Room::demon4UseSTricorderOnSecurityEquipment },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x22, 0, 0}, &Room::demon4UseMTricorderOnSecurityEquipment },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum demon4TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_NAUIAN, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
	TX_DEM4_001, TX_DEM4_002, TX_DEM4_003, TX_DEM4_004, TX_DEM4_005,
	TX_DEM4_006, TX_DEM4_007, TX_DEM4_008, TX_DEM4_009, TX_DEM4_010,
	TX_DEM4_011, TX_DEM4_012, TX_DEM4_013, TX_DEM4_014, TX_DEM4_015,
	TX_DEM4_017, TX_DEM4_018, TX_DEM4_019, TX_DEM4_020, TX_DEM4_021,
	TX_DEM4_022, TX_DEM4_023, TX_DEM4_024, TX_DEM4_025, TX_DEM4_026,
	TX_DEM4_027, TX_DEM4_029, TX_DEM4_030, TX_DEM4_031, TX_DEM4_032,
	TX_DEM4_033, TX_DEM4_034, TX_DEM4_035, TX_DEM4_036, TX_DEM4_037,
	TX_DEM4_038, TX_DEM4_039, TX_DEM4_040, TX_DEM4_041, TX_DEM4_042,
	TX_DEM4_043, TX_DEM4_044, TX_DEM4_045, TX_DEM4_046, TX_DEM4_047,
	TX_DEM4N000, TX_DEM4N001, TX_DEM4N002, TX_DEM4N003, TX_DEM4N004,
	TX_DEM4N005, TX_DEM4N006, TX_DEM4N007, TX_DEM4N008, TX_DEM4N009,
	TX_DEM4N010, TX_DEM4N011, TX_DEM4N012, TX_DEM4N013, TX_DEM4N014,
	TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets demon4TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 3902, 0, 0, 0 },
	{ TX_SPEAKER_NAUIAN, 3913, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 3920, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 3930, 0, 0, 0 },
	{ TX_SPEAKER_EVERTS, 3940, 0, 0, 0 },
	{ TX_DEM4_001, 2980, 0, 0, 0 },
	{ TX_DEM4_002, 4032, 0, 0, 0 },
	{ TX_DEM4_003, 9925, 0, 0, 0 },
	{ TX_DEM4_004, 4097, 0, 0, 0 },
	{ TX_DEM4_005, 9991, 0, 0, 0 },
	{ TX_DEM4_006, 9774, 0, 0, 0 },
	{ TX_DEM4_007, 5073, 0, 0, 0 },
	{ TX_DEM4_008, 5030, 0, 0, 0 },
	{ TX_DEM4_009, 4210, 0, 0, 0 },
	{ TX_DEM4_010,  596, 0, 0, 0 },
	{ TX_DEM4_011, 1055, 0, 0, 0 },
	{ TX_DEM4_012, 3195, 0, 0, 0 },
	{ TX_DEM4_013, 8780, 0, 0, 0 },
	{ TX_DEM4_014, 7484, 0, 0, 0 },
	{ TX_DEM4_015, 6822, 0, 0, 0 },
	{ TX_DEM4_017, 9415, 0, 0, 0 },
	{ TX_DEM4_018, 8903, 0, 0, 0 },
	{ TX_DEM4_019, 7104, 0, 0, 0 },
	{ TX_DEM4_020, 7376, 0, 0, 0 },
	{ TX_DEM4_021, 7334, 0, 0, 0 },
	{ TX_DEM4_022, 7172, 0, 0, 0 },
	{ TX_DEM4_023, 7999, 0, 0, 0 },
	{ TX_DEM4_024, 6882, 0, 0, 0 },
	{ TX_DEM4_025, 7756, 0, 0, 0 },
	{ TX_DEM4_026, 8239, 0, 0, 0 },
	{ TX_DEM4_027, 8146, 0, 0, 0 },
	{ TX_DEM4_029, 8429, 0, 0, 0 },
	{ TX_DEM4_030,  914, 0, 0, 0 },
	{ TX_DEM4_031, 1971, 0, 0, 0 },
	{ TX_DEM4_032, 8718, 0, 0, 0 },
	{ TX_DEM4_033, 8637, 0, 0, 0 },
	{ TX_DEM4_034, 9051, 0, 0, 0 },
	{ TX_DEM4_035,  755, 0, 0, 0 },
	{ TX_DEM4_036, 9539, 0, 0, 0 },
	{ TX_DEM4_037, 6019, 0, 0, 0 },
	{ TX_DEM4_038, 5821, 0, 0, 0 },
	{ TX_DEM4_039, 4845, 0, 0, 0 },
	{ TX_DEM4_040, 5272, 0, 0, 0 },
	{ TX_DEM4_041, 5130, 0, 0, 0 },
	{ TX_DEM4_042, 4310, 0, 0, 0 },
	{ TX_DEM4_043, 4476, 0, 0, 0 },
	{ TX_DEM4_044, 3955, 0, 0, 0 },
	{ TX_DEM4_045, 8836, 0, 0, 0 },
	{ TX_DEM4_046, 7055, 0, 0, 0 },
	{ TX_DEM4_047, 7305, 0, 0, 0 },
	{ TX_DEM4N000, 6576, 0, 0, 0 },
	{ TX_DEM4N001, 6667, 0, 0, 0 },
	{ TX_DEM4N002, 6301, 0, 0, 0 },
	{ TX_DEM4N003, 6765, 0, 0, 0 },
	{ TX_DEM4N004, 6715, 0, 0, 0 },
	{ TX_DEM4N005, 6613, 0, 0, 0 },
	{ TX_DEM4N006, 10194, 0, 0, 0 },
	{ TX_DEM4N007, 6491, 0, 0, 0 },
	{ TX_DEM4N008, 6419, 0, 0, 0 },
	{ TX_DEM4N009, 10116, 0, 0, 0 },
	{ TX_DEM4N010, 6365, 0, 0, 0 },
	{ TX_DEM4N011, 8924, 0, 0, 0 },
	{ TX_DEM4N012, 2666, 0, 0, 0 },
	{ TX_DEM4N013, 2868, 0, 0, 0 },
	{ TX_DEM4N014, 1602, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 10252, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText demon4Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

// TODO: this room should have animations on computer terminals, but this isn't where
// they're implemented. Could be related to ".BAN" files which rooms can load?

void Room::demon4Tick1() {
	playVoc("DEM4LOOP");

	if (!_awayMission->demon.healedMiner)
		_awayMission->demon.minerDead = true;

	if (_awayMission->demon.solvedSunPuzzle) {
		loadActorAnim(10, "bxrise2", 0x122, 0x91, 0);
		loadActorAnim(8, "stpout2", 0x107, 0x92, 0);
		_roomVar.demon.nauianEmerged = true;

		if ((_awayMission->demon.itemsTakenFromCase & 0x10) && !_awayMission->demon.gaveSkullToNauian)
			_awayMission->timers[6] = 20;
	}

	if (!_awayMission->demon.foundAlienRoom) {
		_awayMission->demon.foundAlienRoom = true;
		playMidiMusicTracks(MIDITRACK_1);
	}
}

// Alien emerged
void Room::demon4FinishedAnimation1() {
	loadActorAnim(8, "stpout", 0x107, 0x92, 5);
	_roomVar.demon.nauianEmerged = true;
	if (_awayMission->demon.itemsTakenFromCase & 0x10)
		_awayMission->timers[6] = 45;
}

void Room::demon4FinishedAnimation2() {
	if (!_roomVar.demon.cd) {
		_roomVar.demon.cd = true;
		showDescription(TX_DEM4N011);
	}

	showGameOverMenu();

	// WORKAROUND: original game has the below line; however, it's never executed anyway
	// since the game over menu manipulates the stack. Here, the menu could delete this
	// room object, so it should be the last statement...
	//_awayMission->disableInput = false;
}

void Room::demon4FinishedAnimation3() {
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_034);
	showText(TX_SPEAKER_KIRK,   TX_DEM4_010);
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_035);
	showText(TX_SPEAKER_SPOCK,  TX_DEM4_030);

	_awayMission->disableInput = true;
	_awayMission->demon.missionScore += 5;
	_awayMission->timers[1] = 20;
}

// Just solved sun puzzle
void Room::demon4Timer0Expired() {
	loadActorAnim(10, "bxrise", 0x122, 0x91, 1);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	loadActorStandAnim(OBJECT_KIRK);
	playVoc("SE2POWER");
}

// Mission end
void Room::demon4Timer1Expired() {
	showText(TX_SPEAKER_KIRK, TX_DEM4_011);

	walkCrewman(OBJECT_KIRK, 0xbe, 0xa5, 4);
	walkCrewman(OBJECT_SPOCK, 0xb4, 0x9b, 4);
	walkCrewman(OBJECT_MCCOY, 0xc8, 0x9b, 4);

	if (!_awayMission->redshirtDead)
		walkCrewman(OBJECT_REDSHIRT, 0xc1, 0x91, 4);
	else
		_roomVar.demon.crewReadyToBeamOut++;
}

void Room::demon4CrewmanReachedBeamoutPosition() {
	_roomVar.demon.crewReadyToBeamOut++;
	if (_roomVar.demon.crewReadyToBeamOut != 4)
		return;

	if (!_awayMission->demon.insultedStephen)
		_awayMission->demon.missionScore += 3;
	if (!_awayMission->redshirtDead)
		_awayMission->demon.missionScore += 2;

	endMission(_awayMission->demon.missionScore, 0x24, 0);
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
	if (!_roomVar.demon.triedToShootNauian) { // Lenient the first time
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_033);
		_roomVar.demon.triedToShootNauian = true;
	} else { // Murdery the second time
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_032);
		loadActorAnim2(10, "bxfire", 0, 0, 2);
		playVoc("V0SPOCKT");
		_awayMission->disableInput = true;
		_awayMission->timers[2] = 7;
		_awayMission->timers[3] = 8;
		_awayMission->timers[4] = 7;
		_awayMission->timers[5] = 8;
	}
}

void Room::demon4UseMetalOnSecurityEquipment() {
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xf5, 0x90, 5);
}

void Room::demon4KirkReachedSecurityEquipment() {
	loadActorAnim2(OBJECT_KIRK, "kusemn", -1, -1, 4);
}

void Room::demon4KirkFinishedUsingSecurityEquipment() {
	_awayMission->disableInput = false;
	showDescription(TX_DEM4N014);
}

void Room::demon4UseMetalOnNauian() {
	walkCrewman(OBJECT_KIRK, 0xe9, 0x90, 1);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
}

void Room::demon4KirkReachedNauian() {
	loadActorAnim2(8, "usekey", 0x107, 0x8e, 3);
	loseItem(OBJECT_IMETAL);
	_awayMission->demon.missionScore += 2;
	_awayMission->demon.itemsTakenFromCase &= ~1;
}

void Room::demon4UseSkullOnNauian() {
	walkCrewman(OBJECT_KIRK, 0xe9, 0x90, 2);
}

void Room::demon4KirkReachedNauianWithSkull() {
	showText(TX_SPEAKER_NAUIAN, TX_DEM4_036);

	const TextRef choices[] = { TX_SPEAKER_KIRK, TX_DEM4_006, TX_DEM4_003, TX_DEM4_005, TX_END };
	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0:
		_awayMission->demon.missionScore++;
		loadActorAnim2(8, "takesk", 0x107, 0x8e, 0);
		loseItem(OBJECT_ISKULL);
		_awayMission->demon.itemsTakenFromCase &= ~16; // BUG: skull reappears in case? Can abuse for infinite ponits?
		_awayMission->demon.gaveSkullToNauian = true;
		break;

	case 1:
	case 2:
		break;

	default:
		showDescription(TX_DIALOG_ERROR);
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
	_awayMission->disableInput = false;

	if (!_awayMission->demon.metNauian) {
		_awayMission->demon.metNauian = true;
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_044);

		const TextRef choices[] = { TX_SPEAKER_KIRK, TX_DEM4_002, TX_DEM4_004, TX_DEM4_009, TX_END };
		int choice = showMultipleTexts(choices);

		switch (choice) {
		case 0:
			_awayMission->demon.field2d = true;
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_042);
			break;

		case 1:
			break;

		case 2:
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_039);
			break;

		default:
			showDescription(TX_DIALOG_ERROR);
			break;
		}

		showText(TX_SPEAKER_NAUIAN, TX_DEM4_043);

		const TextRef choices2[] = { TX_SPEAKER_KIRK, TX_DEM4_008, TX_DEM4_007, TX_END };
		choice = showMultipleTexts(choices2);

		switch (choice) {
		case 0:
			_awayMission->demon.field2d = true;
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_041);
			break;
		case 1:
			break;
		default:
			showDescription(TX_DIALOG_ERROR);
			break;
		}

		showText(TX_SPEAKER_NAUIAN, TX_DEM4_040);

		if (_roomVar.demon.disabledSecurity) { // Impossible condition?
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_037);

			// BUGFIX: Originally all of these used no audio, despite the files existing
			// (being used elsewhere).
			showText(TX_SPEAKER_KIRK,   TX_DEM4_010);
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_035);
			showText(TX_SPEAKER_SPOCK,  TX_DEM4_030);

			_awayMission->disableInput = true;
			_awayMission->demon.missionScore += 5;
			_awayMission->timers[1] = 20;
		} else {
			showText(TX_SPEAKER_NAUIAN, TX_DEM4_038);
		}
	} else {
		showText(TX_SPEAKER_NAUIAN, TX_DEM4_031);
	}
}

void Room::demon4LookAtPattern() {
	showDescription(TX_DEM4N002);
}

void Room::demon4LookAtAlien() {
	showDescription(TX_DEM4N007);
}

void Room::demon4LookAnywhere() {
	showDescription(TX_DEM4N004);
}

void Room::demon4LookAtSecurityEquipment() {
	showDescription(TX_DEM4N008);
}

void Room::demon4LookAtFloor() {
	showDescription(TX_DEM4N009);
}

void Room::demon4LookAtKirk() {
	showDescription(TX_DEM4N003);
}

void Room::demon4LookAtMccoy() {
	showDescription(TX_DEM4N000);
}

void Room::demon4LookAtSpock() {
	showDescription(TX_DEM4N005);
}

void Room::demon4LookAtRedshirt() {
	showDescription(TX_DEM4N001);
}

void Room::demon4LookAtChamber() {
	showDescription(TX_DEM4N006);
}

void Room::demon4LookAtPanel() {
	showDescription(TX_DEM4N012);
}

void Room::demon4UseKirkOnPanel() {
	_roomVar.demon.crewmanUsingPanel = OBJECT_KIRK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseSpockOnPanel() {
	_roomVar.demon.crewmanUsingPanel = OBJECT_SPOCK;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseMccoyOnPanel() {
	_roomVar.demon.crewmanUsingPanel = OBJECT_MCCOY;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseRedshirtOnPanel() {
	_roomVar.demon.crewmanUsingPanel = OBJECT_REDSHIRT;
	demon4UseCrewmanOnPanel();
}

void Room::demon4UseCrewmanOnPanel() {
	if (_awayMission->demon.solvedSunPuzzle)
		return;
	walkCrewman(_roomVar.demon.crewmanUsingPanel, 0xda, 0x83, 3);
	_awayMission->crewDirectionsAfterWalk[_roomVar.demon.crewmanUsingPanel] = DIR_N;
	_awayMission->disableInput = true;
}

void Room::demon4CrewmanReachedPanel() {
	if (demon4ShowSunPuzzle()) {
		_awayMission->demon.solvedSunPuzzle = true;
		loadActorAnim(9, "ctrl", 0, 0, 0);
		_awayMission->demon.missionScore += 3;
		_awayMission->timers[0] = 10;
	} else
		showDescription(TX_DEM4N013);

	walkCrewman(_roomVar.demon.crewmanUsingPanel, 0xae, 0x93, 0);
	_awayMission->disableInput = false;
}

bool Room::demon4ShowSunPuzzle() {
	bool solved = false;

	int sliderY = 17;
	int sliderR = -17;
	int sliderB = 17;

	_vm->_gfx->fadeoutScreen();
	_vm->_sound->stopAllVocSounds();

	_vm->_gfx->pushSprites();
	_vm->_gfx->setBackgroundImage("machineb");
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->clearPri();

	Sprite sprites[21];

	for (int i = 0; i < 21; i++) {
		_vm->_gfx->addSprite(&sprites[i]);
	}

	sprites[0].setBitmap(loadBitmapFile("levery"));
	sprites[1].setBitmap(loadBitmapFile("leverr"));
	sprites[2].setBitmap(loadBitmapFile("leverb"));

	Bitmap *lightyBitmap = new Bitmap(loadBitmapFile("lighty"));
	Bitmap *lightrBitmap = new Bitmap(loadBitmapFile("lightr"));
	Bitmap *lightbBitmap = new Bitmap(loadBitmapFile("lightb"));

	for (int i = 3; i < 9; i++)
		sprites[i].setBitmap(lightyBitmap);
	for (int i = 9; i < 15; i++)
		sprites[i].setBitmap(lightrBitmap);
	for (int i = 15; i < 21; i++)
		sprites[i].setBitmap(lightbBitmap);

	Sprite doneButtonSprite;
	_vm->_gfx->addSprite(&doneButtonSprite);
	doneButtonSprite.setXYAndPriority(0x104, 0x64, 2);
	doneButtonSprite.setBitmap(loadBitmapFile("donebutt"));
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

			int y = 0x82;
			int x = 0x83;
			int start = 3;
			int numSprites = abs(sliderY) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
			}
			y -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
				y -= 6;
			}

			y = 0x82;
			x = 0x8b;
			start = 9;
			numSprites = abs(sliderR) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
			}
			y -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
				y -= 6;
			}

			y = 0x82;
			x = 0x93;
			start = 15;
			numSprites = abs(sliderB) / 3;
			for (int i = 0; i < 6; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
			}
			y -= 6;
			for (int i = 0; i < numSprites; i++) {
				sprites[i + start].setXYAndPriority(x, y, 14);
				y -= 6;
			}

			sliderChanged = false;
		}

		TrekEvent event;
		if (!_vm->popNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_LBUTTONDOWN: {
			Common::Point mousePos = _vm->_gfx->getMousePos();
			if (_vm->_gfx->getSpriteAt(mousePos) == &doneButtonSprite) {
				solved = (abs(sliderY) <= 2 && abs(sliderR) <= 2 && abs(sliderB) <= 2);
				continueLoop = false;
			} else {
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
			solved = (abs(sliderY) <= 2 && abs(sliderR) <= 2 && abs(sliderB) <= 2);
			continueLoop = false;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_F2:
				solved = (abs(sliderY) <= 2 && abs(sliderR) <= 2 && abs(sliderB) <= 2);
				continueLoop = false;
				break;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1: {
				// Same as TREKEVENT_LBUTTONDOWN
				Common::Point mousePos = _vm->_gfx->getMousePos();
				if (_vm->_gfx->getSpriteAt(mousePos) == &doneButtonSprite) {
					solved = (abs(sliderY) <= 2 && abs(sliderR) <= 2 && abs(sliderB) <= 2);
					continueLoop = false;
				} else {
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
			}

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
	_vm->_gfx->loadPri(_vm->getScreenName());
	_vm->_gfx->setBackgroundImage(_vm->getScreenName());
	_vm->_gfx->copyBackgroundScreen();
	_vm->_gfx->forceDrawAllSprites();

	for (int i = 0; i < 21; i++) {
		sprites[i].bitmap = nullptr;
	}

	delete lightbBitmap;
	delete lightrBitmap;
	delete lightyBitmap;

	return solved;
}

void Room::demon4TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM4_001);
}

void Room::demon4TalkToMccoy() {
	if (_roomVar.demon.nauianEmerged)
		showText(TX_SPEAKER_MCCOY, TX_DEM4_019);
	else
		showText(TX_SPEAKER_MCCOY, TX_DEM4_015);
}

void Room::demon4TalkToSpock() {
	if (_roomVar.demon.nauianEmerged)
		showText(TX_SPEAKER_SPOCK, TX_DEM4_022);
	else
		showText(TX_SPEAKER_SPOCK, TX_DEM4_024);
}

void Room::demon4TalkToRedshirt() {
	if (_roomVar.demon.nauianEmerged)
		showText(TX_SPEAKER_EVERTS, TX_DEM4_047);
	else
		showText(TX_SPEAKER_EVERTS, TX_DEM4_046);
}

void Room::demon4UseSTricorderOnChamber() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_012);
}

void Room::demon4UseSTricorderOnPattern() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_014);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_025);
}

void Room::demon4UseMTricorderOnPattern() {
	showText(TX_SPEAKER_MCCOY, TX_DEM4_021);
}

void Room::demon4UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_027);
}

void Room::demon4UseMTricorderOnPanel() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM4_027); // BUG: Uses spock's voice (though speaker says "Mccoy")
}

void Room::demon4UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_023);
}

void Room::demon4UseMTricorderOnAnything() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM4_020);
}

void Room::demon4UseSTricorderOnNauian() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_029);
}

void Room::demon4UseMTricorderOnNauian() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_029); // BUG: Uses spock's voice (speaker says "Spock" too)
}

void Room::demon4UseSTricorderOnSecurityEquipment() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_026);
}

void Room::demon4UseMTricorderOnSecurityEquipment() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM4_026); // BUG: Uses spock's voice (speaker says "Spock" too)
}

} // namespace StarTrek
