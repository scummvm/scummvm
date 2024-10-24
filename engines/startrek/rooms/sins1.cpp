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
#define OBJECT_9 9

#define HOTSPOT_DOOR 0x20
#define HOTSPOT_KEYPAD 0x21
#define HOTSPOT_PANEL 0x22
#define HOTSPOT_LENS 0x23
#define HOTSPOT_24 0x24 // Unused hotspot; just a sliver on the right side of the door?

namespace StarTrek {

extern const RoomAction sins1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::sins1Tick1 },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::sins1LookAnywhere },
	{ {ACTION_LOOK, OBJECT_DOOR,    0, 0, 0}, &Room::sins1LookAtLock },
	{ {ACTION_LOOK, HOTSPOT_DOOR,   0, 0, 0}, &Room::sins1LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_PANEL,  0, 0, 0}, &Room::sins1LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_KEYPAD, 0, 0, 0}, &Room::sins1LookAtKeypad },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL,  0, 0}, &Room::sins1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_KEYPAD, 0, 0}, &Room::sins1UseSTricorderOnKeypad },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DOOR,    0, 0}, &Room::sins1UseSTricorderOnLock },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,   0, 0}, &Room::sins1UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,           0, 0}, &Room::sins1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LENS,   0, 0}, &Room::sins1UseSTricorderOnLens },
	{ {ACTION_USE, OBJECT_IS8ROCKS, HOTSPOT_DOOR,   0, 0}, &Room::sins1UseRockOnDoor },

	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_KEYPAD, 0, 0}, &Room::sins1UseRedshirtOnKeypad },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_KEYPAD, 0, 0}, &Room::sins1UseMccoyOnKeypad },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_KEYPAD, 0, 0}, &Room::sins1UseKirkOnKeypad },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_KEYPAD, 0, 0}, &Room::sins1UseSpockOnKeypad },
	{ {ACTION_DONE_WALK, 6,                      0, 0, 0}, &Room::sins1SpockReachedKeypad },
	{ {ACTION_DONE_ANIM, 8,                      0, 0, 0}, &Room::sins1EnteredCorrectCode },
	{ {ACTION_DONE_ANIM, 1,                      0, 0, 0}, &Room::sins1DoorDoneOpening },
	{ {ACTION_DONE_ANIM, 7,                      0, 0, 0}, &Room::sins1EnteredIncorrectCode },
	{ {ACTION_DONE_ANIM, 9,                      0, 0, 0}, &Room::sins1EnteredSacredSofNumber },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR,   0, 0}, &Room::sins1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR,   0, 0}, &Room::sins1UsePhaserOnDoor },
	{ {ACTION_DONE_ANIM, 2,                      0, 0, 0}, &Room::sins1DoorUsedLaser },

	{ {ACTION_TICK, 40,              0, 0, 0}, &Room::sins1Tick40 },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::sins1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins1TalkToRedshirt },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::sins1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins1LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_LENS,    0, 0, 0}, &Room::sins1LookAtLens },

	{ {ACTION_USE, OBJECT_ICOMM,    0xff,            0, 0}, &Room::sins1UseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0, 0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0, 0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0, 0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0, 0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0, 0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0, 0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0, 0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::sins1UseMTricorderOnCrewman },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::sins1WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0, 0}, &Room::sins1WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum sins1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_MOSHER,
	TX_SPEAKER_SCOTT, TX_SPEAKER_UHURA,
	TX_SIN1_001, TX_SIN1_002, TX_SIN1_003, TX_SIN1_004, TX_SIN1_005,
	TX_SIN1_006, TX_SIN1_007, TX_SIN1_008, TX_SIN1_009, TX_SIN1_010,
	TX_SIN1_011, TX_SIN1_012, TX_SIN1_013, TX_SIN1_014, TX_SIN1_015,
	TX_SIN1_016, TX_SIN1_017, TX_SIN1_018, TX_SIN1_019, TX_SIN1_020,
	TX_SIN1_021, TX_SIN1_022, TX_SIN1_023, TX_SIN1_024, TX_SIN1_025,
	TX_SIN1_026, TX_SIN1_027, TX_SIN1_028, TX_SIN1_029, TX_SIN1_030,
	TX_SIN1_S10, TX_SIN1_S17, TX_SIN1_S18, TX_SIN1_S20, TX_SIN1_S51,
	TX_SIN1_S85, TX_SIN1U070, TX_SIN1N000, TX_SIN1N001, TX_SIN1N002,
	TX_SIN1N003, TX_SIN1N004, TX_SIN1N005, TX_SIN1N006, TX_SIN1N007,
	TX_SIN1N008, TX_SIN1N009, TX_SIN1N010, TX_SIN1N011
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 300, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 312, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 322, 0, 0, 0 },
	{ TX_SPEAKER_MOSHER, 332, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 356, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 346, 0, 0, 0 },
	{ TX_SIN1_001, 2352, 0, 0, 0 },
	{ TX_SIN1_002, 5887, 0, 0, 0 },
	{ TX_SIN1_003, 4101, 0, 0, 0 },
	{ TX_SIN1_004, 6100, 0, 0, 0 },
	{ TX_SIN1_005, 3891, 0, 0, 0 },
	{ TX_SIN1_006, 4283, 0, 0, 0 },
	{ TX_SIN1_007, 3659, 0, 0, 0 },
	{ TX_SIN1_008, 5648, 0, 0, 0 },
	{ TX_SIN1_009, 1571, 0, 0, 0 },
	{ TX_SIN1_010, 1341, 0, 0, 0 },
	{ TX_SIN1_011, 2860, 0, 0, 0 },
	{ TX_SIN1_012, 2268, 0, 0, 0 },
	{ TX_SIN1_013, 6436, 0, 0, 0 },
	{ TX_SIN1_014, 6257, 0, 0, 0 },
	{ TX_SIN1_015, 4577, 0, 0, 0 },
	{ TX_SIN1_016, 3301, 0, 0, 0 },
	{ TX_SIN1_017, 3034, 0, 0, 0 },
	{ TX_SIN1_018, 3428, 0, 0, 0 },
	{ TX_SIN1_019, 1682, 0, 0, 0 },
	{ TX_SIN1_020, 1162, 0, 0, 0 },
	{ TX_SIN1_021, 1454, 0, 0, 0 },
	{ TX_SIN1_022, 1900, 0, 0, 0 },
	{ TX_SIN1_023, 1980, 0, 0, 0 },
	{ TX_SIN1_024, 1801, 0, 0, 0 },
	{ TX_SIN1_025, 4455, 0, 0, 0 },
	{ TX_SIN1_026, 4813, 0, 0, 0 },
	{ TX_SIN1_027, 4365, 0, 0, 0 },
	{ TX_SIN1_028, 2195, 0, 0, 0 },
	{ TX_SIN1_029, 4970, 0, 0, 0 },
	{ TX_SIN1_030, 4713, 0, 0, 0 },
	{ TX_SIN1_S10, 3583, 0, 0, 0 },
	{ TX_SIN1_S17, 4180, 0, 0, 0 },
	{ TX_SIN1_S18, 3706, 0, 0, 0 },
	{ TX_SIN1_S20, 6016, 0, 0, 0 },
	{ TX_SIN1_S51, 5702, 0, 0, 0 },
	{ TX_SIN1_S85, 3974, 0, 0, 0 },
	{ TX_SIN1U070, 6169, 0, 0, 0 },
	{ TX_SIN1N000, 5548, 0, 0, 0 },
	{ TX_SIN1N001, 738, 0, 0, 0 },
	{ TX_SIN1N002, 5366, 0, 0, 0 },
	{ TX_SIN1N003, 1014, 0, 0, 0 },
	{ TX_SIN1N004, 5131, 0, 0, 0 },
	{ TX_SIN1N005, 5023, 0, 0, 0 },
	{ TX_SIN1N006, 5266, 0, 0, 0 },
	{ TX_SIN1N007, 2065, 0, 0, 0 },
	{ TX_SIN1N008, 5477, 0, 0, 0 },
	{ TX_SIN1N009, 813, 0, 0, 0 },
	{ TX_SIN1N010, 893, 0, 0, 0 },
	{ TX_SIN1N011, 1089, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText sins1Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::sins1Tick1() {
	playVoc("SIN1LOOP");

	if (!_awayMission->sins.scottyInformedKirkAboutVirus)
		// BUGFIX: Set this to 2, not 1 (disable input even after walking animation is done)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.openedOuterDoor) {
		loadActorAnim2(OBJECT_DOOR, "s1dro", 0, 0);
		loadMapFile("sins12");
	} else
		loadMapFile("sins1");

	if (_awayMission->sins.openedInnerDoor)
		loadActorAnim2(OBJECT_DOOR, "s1ndro", 0, 0);

	_awayMission->sins.field32 = 1;
	_awayMission->sins.field31 = 1;

	playMidiMusicTracks(MIDITRACK_27, kLoopTypeRepeat);

	if (!_awayMission->sins.enteredRoom1FirstTime) {
		playMidiMusicTracks(MIDITRACK_1, kLoopTypeLast);
		_awayMission->sins.enteredRoom1FirstTime = true;
	}
}

void Room::sins1LookAnywhere() {
	showDescription(TX_SIN1N001);
}

void Room::sins1LookAtLock() {
	showDescription(TX_SIN1N009);
}

void Room::sins1LookAtDoor() {
	// NOTE: This function had two implementations (one unused).
	showDescription(TX_SIN1N010);
}

void Room::sins1LookAtPanel() {
	showDescription(TX_SIN1N003);
}

void Room::sins1LookAtKeypad() {
	showDescription(TX_SIN1N011);
}

void Room::sins1UseSTricorderOnPanel() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_010);
}

void Room::sins1UseSTricorderOnKeypad() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_021);
}

void Room::sins1UseSTricorderOnLock() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_009);
}

void Room::sins1UseSTricorderOnDoor() {
	if (!_awayMission->sins.openedOuterDoor)
		spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_019);
}

void Room::sins1UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN1_024);
}

void Room::sins1UseSTricorderOnLens() {
	if (!_awayMission->sins.doorLaserFiredOnce)
		spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_022);
	else // BUGFIX: Original didn't do the tricorder animation, etc. in this case
		spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN1_023);
}

void Room::sins1UseRockOnDoor() {
	showDescription(TX_SIN1N007);
}

void Room::sins1UseRedshirtOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor)
		showText(TX_SPEAKER_MOSHER, TX_SIN1_028);
}

void Room::sins1UseMccoyOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor)
		showText(TX_SPEAKER_MCCOY, TX_SIN1_012);
}

void Room::sins1UseKirkOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor) {
		showText(TX_SPEAKER_KIRK, TX_SIN1_001);
		sins1UseSpockOnKeypad();
	}
}

void Room::sins1UseSpockOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor) {
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		walkCrewmanC(OBJECT_SPOCK, 0xfd, 0xad, &Room::sins1SpockReachedKeypad);
	}
}

void Room::sins1SpockReachedKeypad() {
	const char * const codes[] = {
		"01210", "1210", "10200", nullptr
	};
	int ans = showCodeInputBox(codes);

	if (ans == -1) {
		// ENHANCEMENT: Do nothing if no code was entered.
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
		walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	} else if (ans == 1 || ans == 2) {
		playVoc("EFX14S");
		loadActorAnimC(OBJECT_SPOCK, "susehn", -1, -1, &Room::sins1EnteredSacredSofNumber);
	} else if (ans == 3) {
		playVoc("EFX14S");
		loadActorAnimC(OBJECT_SPOCK, "susehn", -1, -1, &Room::sins1EnteredCorrectCode);
	} else {
		playVoc("EFX14S");
		loadActorAnimC(OBJECT_SPOCK, "susehn", -1, -1, &Room::sins1EnteredIncorrectCode);
	}
}

void Room::sins1EnteredCorrectCode() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	_awayMission->sins.openedOuterDoor = true;
	loadMapFile("sins12");
	loadActorAnimC(OBJECT_DOOR, "s1door", 0, 0, &Room::sins1DoorDoneOpening);
	playVoc("HUGEDOO2");
}

void Room::sins1DoorDoneOpening() {
	showText(TX_SPEAKER_SPOCK, TX_SIN1_011);
}

void Room::sins1EnteredIncorrectCode() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	showText(TX_SPEAKER_SPOCK, TX_SIN1_017);
}

void Room::sins1EnteredSacredSofNumber() {
	// Entered a sacred Sof number, which is wrong since this is a Lucr base
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	loadActorAnimC(OBJECT_9, "s1ph1", 0, 0, &Room::sins1DoorUsedLaser);
	playSoundEffectIndex(kSfxPhaser);
}

void Room::sins1UsePhaserOnDoor() {
	showText(TX_SPEAKER_SPOCK, TX_SIN1_016);
}

void Room::sins1DoorUsedLaser() {
	loadActorStandAnim(OBJECT_9);
	showText(TX_SPEAKER_SPOCK, TX_SIN1_018);

	_awayMission->sins.field33 = true;
	_awayMission->sins.doorLaserFiredOnce = true;
}

void Room::sins1Tick40() {
	if (!_awayMission->sins.scottyInformedKirkAboutVirus) {
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S10);
		showText(TX_SPEAKER_KIRK,  TX_SIN1_007);
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S18);
		showText(TX_SPEAKER_KIRK,  TX_SIN1_005);
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S85);
		showText(TX_SPEAKER_KIRK,  TX_SIN1_003);
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S17);

		_awayMission->sins.scottyInformedKirkAboutVirus = true;
		_awayMission->disableInput = false;
	}
}

void Room::sins1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_SIN1_006);
	showText(TX_SPEAKER_SPOCK, TX_SIN1_027);
}

void Room::sins1TalkToSpock() {
	// NOTE: This function has two implementations. The first (used) one talks about where
	// the energy source is coming from. The second (unused) one says to "carefully
	// consider what we know about this culture".
	showText(TX_SPEAKER_SPOCK, TX_SIN1_020);
}

void Room::sins1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_SIN1_015);
}

void Room::sins1TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, TX_SIN1_030);
	showText(TX_SPEAKER_SPOCK,  TX_SIN1_026);
	showText(TX_SPEAKER_MOSHER, TX_SIN1_029);
}

void Room::sins1LookAtKirk() {
	showDescription(TX_SIN1N005);
}

void Room::sins1LookAtSpock() {
	showDescription(TX_SIN1N004);
}

void Room::sins1LookAtMccoy() {
	showDescription(TX_SIN1N006);
}

void Room::sins1LookAtRedshirt() {
	showDescription(TX_SIN1N002);
}

void Room::sins1LookAtLens() {
	showDescription(TX_SIN1N000);
}

void Room::sins1UseCommunicator() {
	if (!_awayMission->sins.enteredRoom2FirstTime) {
		showText(TX_SPEAKER_KIRK,  TX_SIN1_008);
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S51);
		showText(TX_SPEAKER_KIRK,  TX_SIN1_002);
		showText(TX_SPEAKER_SCOTT, TX_SIN1_S20);
		showText(TX_SPEAKER_KIRK,  TX_SIN1_004);
	} else
		showText(TX_SPEAKER_UHURA, TX_SIN1U070);
}

void Room::sins1UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, TX_SIN1_014);
}

void Room::sins1UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN1_013);
}

void Room::sins1WalkToDoor() {
	if (_awayMission->sins.openedOuterDoor)
		walkCrewman(OBJECT_KIRK, 0x98, 0x9e, 5); // NOTE: Callback 5 not defined
}

}
