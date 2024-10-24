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

#define OBJECT_DOOR 8

#define HOTSPOT_TERMINAL 0x20
#define HOTSPOT_DOOR 0x21
#define HOTSPOT_KEYPAD 0x22

namespace StarTrek {

extern const RoomAction sins2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::sins2Tick1 },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR, 0, 0}, &Room::sins2UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR, 0, 0}, &Room::sins2UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,         0, 0}, &Room::sins2UseSTricorderAnywhere },

	{ {ACTION_USE, OBJECT_KIRK,  HOTSPOT_TERMINAL,    0, 0}, &Room::sins2UseKirkOnTerminal },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_TERMINAL,    0, 0}, &Room::sins2UseSpockOnTerminal },
	{ {ACTION_DONE_WALK, 5,                        0, 0, 0}, &Room::sins2SpockReachedTerminal },
	{ {ACTION_DONE_ANIM, 6,                        0, 0, 0}, &Room::sins2SpockUsedTerminal },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_TERMINAL, 0, 0}, &Room::sins2UseMccoyOnTerminal },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_TERMINAL, 0, 0}, &Room::sins2UseRedshirtOnTerminal },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_TERMINAL, 0, 0}, &Room::sins2UseSTricorderOnTerminal },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_KEYPAD,   0, 0}, &Room::sins2UseSTricorderOnKeypad },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,     0, 0}, &Room::sins2UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_KEYPAD,   0, 0}, &Room::sins2UseKirkOnKeypad },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_KEYPAD,   0, 0}, &Room::sins2UseSpockOnKeypad },
	{ {ACTION_DONE_WALK, 2,                        0, 0, 0}, &Room::sins2SpockReachedKeypad },
	{ {ACTION_DONE_ANIM, 3,                        0, 0, 0}, &Room::sins2EnteredCorrectCode },
	{ {ACTION_DONE_ANIM, 4,                        0, 0, 0}, &Room::sins2EnteredIncorrectCode },
	{ {ACTION_DONE_ANIM, 1,                        0, 0, 0}, &Room::sins2DoorFinishedOpening },

	{ {ACTION_TICK, 40,               0, 0, 0}, &Room::sins2Tick40 },
	{ {ACTION_LOOK, 0xff,             0, 0, 0}, &Room::sins2LookAnywhere },
	{ {ACTION_LOOK, OBJECT_DOOR,      0, 0, 0}, &Room::sins2LookAtOpenDoor },
	{ {ACTION_LOOK, HOTSPOT_TERMINAL, 0, 0, 0}, &Room::sins2LookAtTerminal },
	{ {ACTION_LOOK, HOTSPOT_KEYPAD,   0, 0, 0}, &Room::sins2LookAtKeypad },
	{ {ACTION_LOOK, OBJECT_KIRK,      0, 0, 0}, &Room::sins2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,     0, 0, 0}, &Room::sins2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,     0, 0, 0}, &Room::sins2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,  0, 0, 0}, &Room::sins2LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_KIRK,      0, 0, 0}, &Room::sins2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,     0, 0, 0}, &Room::sins2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,     0, 0, 0}, &Room::sins2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT,  0, 0, 0}, &Room::sins2TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_DOOR,     0, 0, 0}, &Room::sins2LookAtDoor },

	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_DOOR,    0, 0}, &Room::sins2UseSpockOnDoor },
	{ {ACTION_USE, OBJECT_ICOMM,    0xff,            0, 0}, &Room::sins2UseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0, 0}, &Room::sins2UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0, 0}, &Room::sins2UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0, 0}, &Room::sins2UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0, 0}, &Room::sins2UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0, 0}, &Room::sins2UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0, 0}, &Room::sins2UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0, 0}, &Room::sins2UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::sins2UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_KEYPAD,  0, 0}, &Room::sins2UseMccoyOnKeypad },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_KEYPAD,  0, 0}, &Room::sins2UseRedshirtOnKeypad },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::sins2WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::sins2WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum sins2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_MOSHER,
	TX_SPEAKER_SCOTT, TX_SPEAKER_UHURA,
	TX_SIN2_001, TX_SIN2_002, TX_SIN2_003, TX_SIN2_004, TX_SIN2_005,
	TX_SIN2_006, TX_SIN2_007, TX_SIN2_008, TX_SIN2_009, TX_SIN2_010,
	TX_SIN2_012, TX_SIN2_013, TX_SIN2_014, TX_SIN2_015, TX_SIN2_016,
	TX_SIN2_017, TX_SIN2_019, TX_SIN2_020, TX_SIN2_021, TX_SIN2_022,
	TX_SIN2_023, TX_SIN2_024, TX_SIN2_025, TX_SIN2_026, TX_SIN2_027,
	TX_SIN2_028, TX_SIN2_029, TX_SIN2_030, TX_SIN2_031, TX_SIN2_032,
	TX_SIN2_033, TX_SIN2_034, TX_SIN2_035, TX_SIN2_037, TX_SIN2_038,
	TX_SIN2_039, TX_SIN2_040, TX_SIN2_041, TX_SIN2_042, TX_SIN2_043,
	TX_SIN2_044, TX_SIN2_045, TX_SIN2_046, TX_SIN2_047, TX_SIN2_048,
	TX_SIN2_049, TX_SIN2_050, TX_SIN2_F12, TX_SIN2_F26, TX_SIN2_S05,
	TX_SIN2_S16, TX_SIN3_008, TX_SIN1U070, TX_SIN2N000, TX_SIN2N001,
	TX_SIN2N002, TX_SIN2N003, TX_SIN2N004, TX_SIN2N005, TX_SIN2N006,
	TX_SIN2N007, TX_SIN2N008, TX_SIN2U085, TX_SIN2U096, TX_SIN2U097
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins2TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 301, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 312, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 322, 0, 0, 0 },
	{ TX_SPEAKER_MOSHER, 332, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 356, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 346, 0, 0, 0 },
	{ TX_SIN2_001, 4324, 0, 0, 0 },
	{ TX_SIN2_002, 892, 0, 0, 0 },
	{ TX_SIN2_003, 6723, 0, 0, 0 },
	{ TX_SIN2_004, 5869, 0, 0, 0 },
	{ TX_SIN2_005, 5052, 0, 0, 0 },
	{ TX_SIN2_006, 8116, 0, 0, 0 },
	{ TX_SIN2_007, 3070, 0, 0, 0 },
	{ TX_SIN2_008, 5400, 0, 0, 0 },
	{ TX_SIN2_009, 3770, 0, 0, 0 },
	{ TX_SIN2_010, 4811, 0, 0, 0 },
	{ TX_SIN2_012, 3514, 0, 0, 0 },
	{ TX_SIN2_013, 8717, 0, 0, 0 },
	{ TX_SIN2_014, 8458, 0, 0, 0 },
	{ TX_SIN2_015, 1353, 0, 0, 0 },
	{ TX_SIN2_016, 6980, 0, 0, 0 },
	{ TX_SIN2_017, 8296, 0, 0, 0 },
	{ TX_SIN2_019, 5806, 0, 0, 0 },
	{ TX_SIN2_020, 2835, 0, 0, 0 },
	{ TX_SIN2_021, 2303, 0, 0, 0 },
	{ TX_SIN2_022, 3268, 0, 0, 0 },
	{ TX_SIN2_023, 7064, 0, 0, 0 },
	{ TX_SIN2_024, 4683, 0, 0, 0 },
	{ TX_SIN2_025, 670, 0, 0, 0 },
	{ TX_SIN2_026, 789, 0, 0, 0 },
	{ TX_SIN2_027, 4101, 0, 0, 0 },
	{ TX_SIN2_028, 3933, 0, 0, 0 },
	{ TX_SIN2_029, 3302, 0, 0, 0 },
	{ TX_SIN2_030, 7914, 0, 0, 0 },
	{ TX_SIN2_031, 2586, 0, 0, 0 },
	{ TX_SIN2_032, 1436, 0, 0, 0 },
	{ TX_SIN2_033, 2936, 0, 0, 0 },
	{ TX_SIN2_034, 2372, 0, 0, 0 },
	{ TX_SIN2_035, 8953, 0, 0, 0 },
	{ TX_SIN2_037, 3219, 0, 0, 0 },
	{ TX_SIN2_038, 2687, 0, 0, 0 },
	{ TX_SIN2_039, 6813, 0, 0, 0 },
	{ TX_SIN2_040, 2170, 0, 0, 0 },
	{ TX_SIN2_041, 1974, 0, 0, 0 },
	{ TX_SIN2_042, 1064, 0, 0, 0 },
	{ TX_SIN2_043, 1770, 0, 0, 0 },
	{ TX_SIN2_044, 1127, 0, 0, 0 },
	{ TX_SIN2_045, 3615, 0, 0, 0 },
	{ TX_SIN2_046, 8820, 0, 0, 0 },
	{ TX_SIN2_047, 7328, 0, 0, 0 },
	{ TX_SIN2_048, 7795, 0, 0, 0 },
	{ TX_SIN2_049, 7155, 0, 0, 0 },
	{ TX_SIN2_050, 1639, 0, 0, 0 },
	{ TX_SIN2_F12, 7608, 0, 0, 0 },
	{ TX_SIN2_F26, 5591, 0, 0, 0 },
	{ TX_SIN2_S05, 5116, 0, 0, 0 },
	{ TX_SIN2_S16, 8179, 0, 0, 0 },
	{ TX_SIN3_008, 8591, 0, 0, 0 },
	{ TX_SIN1U070, 8370, 0, 0, 0 },
	{ TX_SIN2N000, 6119, 0, 0, 0 },
	{ TX_SIN2N001, 6301, 0, 0, 0 },
	{ TX_SIN2N002, 6620, 0, 0, 0 },
	{ TX_SIN2N003, 6359, 0, 0, 0 },
	{ TX_SIN2N004, 6536, 0, 0, 0 },
	{ TX_SIN2N005, 6451, 0, 0, 0 },
	{ TX_SIN2N006, 5948, 0, 0, 0 },
	{ TX_SIN2N007, 6060, 0, 0, 0 },
	{ TX_SIN2N008, 7524, 0, 0, 0 },
	{ TX_SIN2U085, 5252, 0, 0, 0 },
	{ TX_SIN2U096, 5671, 0, 0, 0 },
	{ TX_SIN2U097, 5445, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText sins2Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::sins2Tick1() {
	playVoc("SIN2LOOP");
	playMidiMusicTracks(MIDITRACK_27, kLoopTypeRepeat);

	if (!_awayMission->sins.enteredRoom2FirstTime)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.openedInnerDoor) {
		// NOTE: callback is never called, since animation never finishes?
		loadActorAnimC(OBJECT_DOOR, "s2dro", 0, 0, &Room::sins2DoorFinishedOpening);
		loadMapFile("sins22");
	} else {
		loadMapFile("sins2");
	}
}

void Room::sins2UsePhaserOnDoor() {
	showText(TX_SPEAKER_SPOCK, TX_SIN2_025);
}

void Room::sins2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN2_026);
}

void Room::sins2UseKirkOnTerminal() {
	showText(TX_SPEAKER_KIRK, TX_SIN2_002);
	sins2UseSpockOnTerminal();
}

void Room::sins2UseSpockOnTerminal() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewmanC(OBJECT_SPOCK, 0x5f, 0xa7, &Room::sins2SpockReachedTerminal);
}

void Room::sins2SpockReachedTerminal() {
	playVoc("EFX14S");
	loadActorAnimC(OBJECT_SPOCK, "susehw", -1, -1, &Room::sins2SpockUsedTerminal);
}

void Room::sins2SpockUsedTerminal() {
	showText(TX_SPEAKER_SPOCK, TX_SIN2_042);
	showText(TX_SPEAKER_SPOCK, TX_SIN2_044);

	if (!_awayMission->sins.gotPointsForAccessingTerminal) {
		_awayMission->sins.missionScore += 2;
		_awayMission->sins.gotPointsForAccessingTerminal = true;
	}

	showText(TX_SPEAKER_MCCOY,  TX_SIN2_015);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_032);
	showText(TX_SPEAKER_MOSHER, TX_SIN2_050);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_035);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_043);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_041);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_040);
	showText(TX_SPEAKER_MCCOY,  TX_SIN2_021);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_034);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_031);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_038);
	showText(TX_SPEAKER_MCCOY,  TX_SIN2_020);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_033);
	showText(TX_SPEAKER_KIRK,   TX_SIN2_007);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_037);
	showText(TX_SPEAKER_MCCOY,  TX_SIN2_022);
	showText(TX_SPEAKER_SPOCK,  TX_SIN2_029);
}

void Room::sins2UseMccoyOnTerminal() {
	showText(TX_SPEAKER_MCCOY, TX_SIN2_012);
}

void Room::sins2UseRedshirtOnTerminal() {
	showText(TX_SPEAKER_MOSHER, TX_SIN2_045);
}

void Room::sins2UseSTricorderOnTerminal() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_SIN2_009);
}

void Room::sins2UseSTricorderOnKeypad() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN2_028);
}

void Room::sins2UseSTricorderOnDoor() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN2_027);
}

void Room::sins2UseKirkOnKeypad() {
	if (!_awayMission->sins.openedInnerDoor) {
		showText(TX_SPEAKER_KIRK, TX_SIN2_001);
		sins2UseSpockOnKeypad();
	}
}

void Room::sins2UseSpockOnKeypad() {
	if (!_awayMission->sins.openedInnerDoor) {
		walkCrewmanC(OBJECT_SPOCK, 0xc3, 0x9b, &Room::sins2SpockReachedKeypad);
	}
}

void Room::sins2SpockReachedKeypad() {
	const char *codes[] = {
		"122", nullptr
	};

	int ans = showCodeInputBox(codes);

	if (ans == -1) { // ENHANCEMENT: if nothing was entered, do nothing.
	} else if (ans == 1) {
		playVoc("EFX28S");
		loadActorAnimC(OBJECT_SPOCK, "susehe", -1, -1, &Room::sins2EnteredCorrectCode);
	} else { // Incorrect answer
		playVoc("EFX28S");
		loadActorAnimC(OBJECT_SPOCK, "susehe", -1, -1, &Room::sins2EnteredIncorrectCode);
	}
}

void Room::sins2EnteredCorrectCode() {
	_awayMission->sins.openedInnerDoor = true;
	loadMapFile("sins22");
	playVoc("BIGDOOR3");
	loadActorAnimC(OBJECT_DOOR, "s2door", 0, 0, &Room::sins2DoorFinishedOpening);
}

void Room::sins2EnteredIncorrectCode() {
	showText(TX_SPEAKER_SPOCK, TX_SIN2_024); // BUGFIX: Speaker is spock, not "none"
}

void Room::sins2DoorFinishedOpening() {
	showText(TX_SPEAKER_SPOCK, TX_SIN2_010);
}

void Room::sins2Tick40() {
	if (!_awayMission->sins.enteredRoom2FirstTime) {
		_awayMission->disableInput = false;

		showText(TX_SPEAKER_KIRK,  TX_SIN2_005);
		showText(TX_SPEAKER_SCOTT, TX_SIN2_S05);
		showText(TX_SPEAKER_UHURA, TX_SIN2U085);
		showText(TX_SPEAKER_KIRK,  TX_SIN2_008);
		showText(TX_SPEAKER_UHURA, TX_SIN2U097);
		showText(TX_SPEAKER_MCCOY, TX_SIN2_F26);
		showText(TX_SPEAKER_UHURA, TX_SIN2U096);
		showText(TX_SPEAKER_MCCOY, TX_SIN2_019);
		showText(TX_SPEAKER_KIRK,  TX_SIN2_004);

		_awayMission->sins.enteredRoom2FirstTime = true;
	}
}

void Room::sins2LookAnywhere() {
	showDescription(TX_SIN2N006);
}

void Room::sins2LookAtOpenDoor() {
	showDescription(TX_SIN2N007);
}

void Room::sins2LookAtTerminal() {
	showDescription(TX_SIN2N000);
}

void Room::sins2LookAtKeypad() {
	showDescription(TX_SIN2N001);
}

void Room::sins2LookAtKirk() {
	showDescription(TX_SIN2N003);
}

void Room::sins2LookAtSpock() {
	showDescription(TX_SIN2N005);
}

void Room::sins2LookAtMccoy() {
	showDescription(TX_SIN2N004);
}

void Room::sins2LookAtRedshirt() {
	showDescription(TX_SIN2N002);
}

void Room::sins2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_SIN2_003);
}

void Room::sins2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_SIN2_039);
	showText(TX_SPEAKER_MCCOY, TX_SIN2_016);
}

void Room::sins2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY,  TX_SIN2_023);
	showText(TX_SPEAKER_MOSHER, TX_SIN2_049);
}

void Room::sins2TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, TX_SIN2_047);
}

void Room::sins2LookAtDoor() {
	if (!_awayMission->sins.openedInnerDoor)
		showDescription(TX_SIN2N008);
}

void Room::sins2UseSpockOnDoor() {
	if (!_awayMission->sins.openedInnerDoor) {
		showText(TX_SPEAKER_SPOCK,  TX_SIN2_F12);
		showText(TX_SPEAKER_MOSHER, TX_SIN2_048);
		showText(TX_SPEAKER_SPOCK,  TX_SIN2_030);
	}
}

void Room::sins2UseCommunicator() {
	if (!_awayMission->sins.enteredRoom4FirstTime) {
		showText(TX_SPEAKER_KIRK,  TX_SIN2_006);
		showText(TX_SPEAKER_SCOTT, TX_SIN2_S16);
		showText(TX_SPEAKER_MCCOY, TX_SIN2_017);
	} else
		showText(TX_SPEAKER_UHURA, TX_SIN1U070);
}

void Room::sins2UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, TX_SIN2_014);
}

void Room::sins2UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_SIN3_008); // BUGFIX: original game had wrong audio file path
}

void Room::sins2UseMccoyOnKeypad() {
	if (!_awayMission->sins.openedInnerDoor)
		showText(TX_SPEAKER_MCCOY, TX_SIN2_013);
}

void Room::sins2UseRedshirtOnKeypad() {
	if (!_awayMission->sins.openedInnerDoor)
		showText(TX_SPEAKER_MOSHER, TX_SIN2_046);
}

void Room::sins2WalkToDoor() {
	if (_awayMission->sins.openedInnerDoor)
		walkCrewman(OBJECT_KIRK, 0x9c, 0x8b);
}
}
