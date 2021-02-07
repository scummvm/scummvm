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

#if 0
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
	{ {ACTION_TICK, 1, 0, 0}, &Room::sins1Tick1 },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::sins1LookAnywhere },
	{ {ACTION_LOOK, OBJECT_DOOR,    0, 0}, &Room::sins1LookAtLock },
	{ {ACTION_LOOK, HOTSPOT_DOOR,   0, 0}, &Room::sins1LookAtDoor },
	{ {ACTION_LOOK, HOTSPOT_PANEL,  0, 0}, &Room::sins1LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_KEYPAD, 0, 0}, &Room::sins1LookAtKeypad },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL,  0}, &Room::sins1UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_KEYPAD, 0}, &Room::sins1UseSTricorderOnKeypad },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_DOOR,    0}, &Room::sins1UseSTricorderOnLock },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DOOR,   0}, &Room::sins1UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,           0}, &Room::sins1UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LENS,   0}, &Room::sins1UseSTricorderOnLens },
	{ {ACTION_USE, OBJECT_IS8ROCKS, HOTSPOT_DOOR,   0}, &Room::sins1UseRockOnDoor },

	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_KEYPAD, 0}, &Room::sins1UseRedshirtOnKeypad },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_KEYPAD, 0}, &Room::sins1UseMccoyOnKeypad },
	{ {ACTION_USE, OBJECT_KIRK,     HOTSPOT_KEYPAD, 0}, &Room::sins1UseKirkOnKeypad },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_KEYPAD, 0}, &Room::sins1UseSpockOnKeypad },
	{ {ACTION_DONE_WALK, 6,                      0, 0}, &Room::sins1SpockReachedKeypad },
	{ {ACTION_DONE_ANIM, 8,                      0, 0}, &Room::sins1EnteredCorrectCode },
	{ {ACTION_DONE_ANIM, 1,                      0, 0}, &Room::sins1DoorDoneOpening },
	{ {ACTION_DONE_ANIM, 7,                      0, 0}, &Room::sins1EnteredIncorrectCode },
	{ {ACTION_DONE_ANIM, 9,                      0, 0}, &Room::sins1EnteredSacredSofNumber },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_DOOR,   0}, &Room::sins1UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_DOOR,   0}, &Room::sins1UsePhaserOnDoor },
	{ {ACTION_DONE_ANIM, 2,                      0, 0}, &Room::sins1DoorUsedLaser },

	{ {ACTION_TICK, 40,              0, 0}, &Room::sins1Tick40 },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::sins1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::sins1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::sins1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::sins1TalkToRedshirt },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::sins1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::sins1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::sins1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::sins1LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_LENS,    0, 0}, &Room::sins1LookAtLens },

	{ {ACTION_USE, OBJECT_ICOMM,    0xff,            0}, &Room::sins1UseCommunicator },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0}, &Room::sins1UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0}, &Room::sins1UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::sins1UseMTricorderOnCrewman },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::sins1WalkToDoor },
	{ {ACTION_WALK, OBJECT_DOOR,  0, 0}, &Room::sins1WalkToDoor },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

enum sins1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins1TextOffsets[] = {
	//{ TX_SPEAKER_KIRK, 2597, 0 },
	//{ TX_SPEAKER_MCCOY, 2622, 0 },
	//{ TX_SPEAKER_SPOCK, 2632, 0 },
	//{ TX_SPEAKER_EVERTS, 2642, 0 },
	{          -1, 0,    0 }
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

	playMidiMusicTracks(MIDITRACK_27, -3);

	if (!_awayMission->sins.enteredRoom1FirstTime) {
		playMidiMusicTracks(MIDITRACK_1, -2);
		_awayMission->sins.enteredRoom1FirstTime = true;
	}
}

void Room::sins1LookAnywhere() {
	showDescription(1);
}

void Room::sins1LookAtLock() {
	showDescription(9);
}

void Room::sins1LookAtDoor() {
	// NOTE: This function had two implementations (one unused).
	if (true)
		showDescription(10);
	else
		showDescription(8);
}

void Room::sins1LookAtPanel() {
	showDescription(3);
}

void Room::sins1LookAtKeypad() {
	showDescription(11);
}

void Room::sins1UseSTricorderOnPanel() {
	spockScan(DIR_N, 10, false);
}

void Room::sins1UseSTricorderOnKeypad() {
	spockScan(DIR_N, 21, false);
}

void Room::sins1UseSTricorderOnLock() {
	spockScan(DIR_N, 9, false);
}

void Room::sins1UseSTricorderOnDoor() {
	if (!_awayMission->sins.openedOuterDoor)
		spockScan(DIR_N, 19, false);
}

void Room::sins1UseSTricorderAnywhere() {
	spockScan(DIR_S, 24, false);
}

void Room::sins1UseSTricorderOnLens() {
	if (!_awayMission->sins.doorLaserFiredOnce)
		spockScan(DIR_N, 22, false);
	else // BUGFIX: Original didn't do the tricorder animation, etc. in this case
		spockScan(DIR_N, 23, false);
}

void Room::sins1UseRockOnDoor() {
	showDescription(7);
}

void Room::sins1UseRedshirtOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor)
		showText(TX_SPEAKER_MOSHER, 28);
}

void Room::sins1UseMccoyOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor)
		showText(TX_SPEAKER_MCCOY, 12);
}

void Room::sins1UseKirkOnKeypad() {
	if (!_awayMission->sins.openedOuterDoor) {
		showText(TX_SPEAKER_KIRK, 1);
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
	showText(TX_SPEAKER_SPOCK, 11);
}

void Room::sins1EnteredIncorrectCode() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	showText(TX_SPEAKER_SPOCK, 17);
}

void Room::sins1EnteredSacredSofNumber() {
	// Entered a sacred Sof number, which is wrong since this is a Lucr base
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0xf3, 0xad);
	loadActorAnimC(OBJECT_9, "s1ph1", 0, 0, &Room::sins1DoorUsedLaser);
	playSoundEffectIndex(kSfxPhaser);
}

void Room::sins1UsePhaserOnDoor() {
	showText(TX_SPEAKER_SPOCK, 16);
}

void Room::sins1DoorUsedLaser() {
	loadActorStandAnim(OBJECT_9);
	showText(TX_SPEAKER_SPOCK, 18);

	_awayMission->sins.field33 = true;
	_awayMission->sins.doorLaserFiredOnce = true;
}

void Room::sins1Tick40() {
	if (!_awayMission->sins.scottyInformedKirkAboutVirus) {
		showText(TX_SPEAKER_SCOTT_GLOBAL, 10 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  7);
		showText(TX_SPEAKER_SCOTT_GLOBAL, 18 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  5);
		showText(TX_SPEAKER_SCOTT_GLOBAL, 85 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  3);
		showText(TX_SPEAKER_SCOTT_GLOBAL, 17 + SCOTTY_MESSAGE_OFFSET);

		_awayMission->sins.scottyInformedKirkAboutVirus = true;
		_awayMission->disableInput = false;
	}
}

void Room::sins1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  6);
	showText(TX_SPEAKER_SPOCK, 27);
}

void Room::sins1TalkToSpock() {
	// NOTE: This function has two implementations. The first (used) one talks about where
	// the energy source is coming from. The second (unused) one says to "carefully
	// consider what we know about this culture".
	if (true)
		showText(TX_SPEAKER_SPOCK, 20);
	else
		showText(TX_SPEAKER_SPOCK, 25);
}

void Room::sins1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 15);
}

void Room::sins1TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, 30);
	showText(TX_SPEAKER_SPOCK,  26);
	showText(TX_SPEAKER_MOSHER, 29);
}

void Room::sins1LookAtKirk() {
	showDescription(5);
}

void Room::sins1LookAtSpock() {
	showDescription(4);
}

void Room::sins1LookAtMccoy() {
	showDescription(6);
}

void Room::sins1LookAtRedshirt() {
	showDescription(2);
}

void Room::sins1LookAtLens() {
	showDescription(0);
}

void Room::sins1UseCommunicator() {
	if (!_awayMission->sins.enteredRoom2FirstTime) {
		showText(TX_SPEAKER_KIRK,  8);
		showText(TX_SPEAKER_SCOTT_GLOBAL, 51 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  2);
		showText(TX_SPEAKER_SCOTT_GLOBAL, 20 + SCOTTY_MESSAGE_OFFSET);
		showText(TX_SPEAKER_KIRK,  4);
	} else
		showText(TX_SPEAKER_UHURA_GLOBAL, 70);
}

void Room::sins1UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, 14);
}

void Room::sins1UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, 13, false);
}

void Room::sins1WalkToDoor() {
	if (_awayMission->sins.openedOuterDoor)
		walkCrewman(OBJECT_KIRK, 0x98, 0x9e, 5); // NOTE: Callback 5 not defined
}

}
#endif
