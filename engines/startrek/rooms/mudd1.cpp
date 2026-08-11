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

#define OBJECT_CRANE 8
#define OBJECT_TORPEDO 9
#define OBJECT_CRANE_ANIMATION 10
#define OBJECT_NORTH_DOOR 11
#define OBJECT_WEST_DOOR 12
#define OBJECT_ALIENDV 13

#define HOTSPOT_TORPEDO 0x20
#define HOTSPOT_FALLEN_TORPEDO 0x21
#define HOTSPOT_TORPEDO_LAUNCHER 0x22
#define HOTSPOT_BLUE_BUTTON 0x23
#define HOTSPOT_YELLOW_BUTTON 0x24
#define HOTSPOT_PURPLE_BUTTON 0x25
#define HOTSPOT_WEST_DOOR  0x26
#define HOTSPOT_NORTH_DOOR 0x27
#define HOTSPOT_SOUTH_DOOR 0x28

// BUG: The action menu appears sort of behind a torpedo. ScummVM-exclusive bug. Also,
// during the loading animation, it can appear on top of Kirk if he's standing in front
// (not scummvm-exclusive).

namespace StarTrek {

extern const RoomAction mudd1ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::mudd1Tick1 },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::mudd1Timer1Expired },

	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0},    &Room::mudd1UseCommunicator },
	{ {ACTION_USE, OBJECT_SPOCK, 0x23, 0, 0},  &Room::mudd1UseSpockOnBlueButton },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0},   &Room::mudd1SpockReachedBlueButton },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::mudd1SpockPressedBlueButton },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::mudd1CraneFinishedMoving },
	{ {ACTION_USE, OBJECT_SPOCK, 0x24, 0, 0},  &Room::mudd1UseSpockOnYellowButton },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0},   &Room::mudd1SpockReachedYellowButton },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::mudd1SpockPressedYellowButton },
	{ {ACTION_USE, OBJECT_SPOCK, 0x25, 0, 0},  &Room::mudd1UseSpockOnRedButton },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0},   &Room::mudd1SpockReachedRedButton },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::mudd1SpockPressedRedButton },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0, 0}, &Room::muddaUseDegrimer },
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::muddaFiredAlienDevice },

	{ {ACTION_GET, 9,    0, 0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_GET, 0x21, 0, 0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_GET, 0x20, 0, 0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 9,    0, 0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0, 0}, &Room::mudd1UseSTricorderOnTorpedoLauncher },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x24, 0, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0, 0},    &Room::mudd1UseSTricorderOnCrane },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},  &Room::mudd1UseMedkitAnywhere },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::mudd1LookAnywhere },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::mudd1LookAtTorpedo },
	{ {ACTION_LOOK, 9,    0, 0, 0}, &Room::mudd1LookAtTorpedo },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::mudd1LookAtFallenTorpedo },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::mudd1LookAtTorpedoLauncher },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::mudd1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::mudd1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::mudd1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd1LookAtRedshirt },
	{ {ACTION_LOOK, 8,    0, 0, 0}, &Room::mudd1LookAtCrane },
	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::mudd1LookAtRedButton },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::mudd1LookAtBlueButton },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::mudd1LookAtYellowButton },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::mudd1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::mudd1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::mudd1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd1TalkToRedshirt },
	{ {ACTION_WALK, 0x28, 0, 0, 0}, &Room::mudd1WalkToSouthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 2, 0, 0, 0}, &Room::mudd1TouchedHotspot2 },
	{ {ACTION_WALK, 0x27, 0, 0, 0}, &Room::mudd1WalkToNorthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0, 0}, &Room::mudd1TouchedHotspot1 },
	{ {ACTION_WALK, 0x26, 0, 0, 0}, &Room::mudd1WalkToWestDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::mudd1TouchedHotspot0 },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum mudd1TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_UHURA, TX_SPEAKER_BUCHERT,
	TX_MUD1_001, TX_MUD1_003, TX_MUD1_004, TX_MUD1_005, TX_MUD1_006,
	TX_MUD1_007, TX_MUD1_008, TX_MUD1_009, TX_MUD1_010, TX_MUD1_011,
	TX_MUD1_012, TX_MUD1_013, TX_MUD1_014, TX_MUD1_015, TX_MUD1_016,
	TX_MUD1_017, TX_MUD1_019, TX_MUD1_020, TX_MUD1_021, TX_MUD1_022,
	TX_MUD1_023, TX_MUD1_024, TX_MUD1_025, TX_MUD1_026, TX_MUD1_027,
	TX_MUD1_028, TX_MUD1N000, TX_MUD1N001, TX_MUD1N002, TX_MUD1N003,
	TX_MUD1N005, TX_MUD1N006, TX_MUD1N007, TX_MUD1N008, TX_MUD1N009,
	TX_MUD1N010, TX_MUD1N011, TX_MUD1N012, TX_MUD1N014, TX_MUD1N015,
	TX_MUD1N016, TX_STATICU1, TX_DEM0N009
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets mudd1TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 3119, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 3130, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 3140, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 3162, 0, 0, 0 },
	{ TX_SPEAKER_BUCHERT, 3150, 0, 0, 0 },
	{ TX_MUD1_001, 471, 0, 0, 0 },
	{ TX_MUD1_003, 5160, 0, 0, 0 },
	{ TX_MUD1_004, 5023, 0, 0, 0 },
	{ TX_MUD1_005, 5885, 0, 0, 0 },
	{ TX_MUD1_006, 5744, 0, 0, 0 },
	{ TX_MUD1_007, 5231, 0, 0, 0 },
	{ TX_MUD1_008, 5971, 0, 0, 0 },
	{ TX_MUD1_009, 3561, 0, 0, 0 },
	{ TX_MUD1_010, 3751, 0, 0, 0 },
	{ TX_MUD1_011, 1546, 0, 0, 0 },
	{ TX_MUD1_012, 6082, 0, 0, 0 },
	{ TX_MUD1_013, 5815, 0, 0, 0 },
	{ TX_MUD1_014, 6545, 0, 0, 0 },
	{ TX_MUD1_015, 6647, 0, 0, 0 },
	{ TX_MUD1_016, 6293, 0, 0, 0 },
	{ TX_MUD1_017, 6236, 0, 0, 0 },
	{ TX_MUD1_019, 5916, 0, 0, 0 },
	{ TX_MUD1_020, 4092, 0, 0, 0 },
	{ TX_MUD1_021, 4686, 0, 0, 0 },
	{ TX_MUD1_022, 4582, 0, 0, 0 },
	{ TX_MUD1_023, 4860, 0, 0, 0 },
	{ TX_MUD1_024, 4478, 0, 0, 0 },
	{ TX_MUD1_025, 3906, 0, 0, 0 },
	{ TX_MUD1_026, 3473, 0, 0, 0 },
	{ TX_MUD1_027, 4005, 0, 0, 0 },
	{ TX_MUD1_028, 4309, 0, 0, 0 },
	{ TX_MUD1N000, 6347, 0, 0, 0 },
	{ TX_MUD1N001, 6413, 0, 0, 0 },
	{ TX_MUD1N002, 3291, 0, 0, 0 },
	{ TX_MUD1N003, 3411, 0, 0, 0 },
	{ TX_MUD1N005, 5357, 0, 0, 0 },
	{ TX_MUD1N006, 5666, 0, 0, 0 },
	{ TX_MUD1N007, 2457, 0, 0, 0 },
	{ TX_MUD1N008, 5577, 0, 0, 0 },
	{ TX_MUD1N009, 5434, 0, 0, 0 },
	{ TX_MUD1N010, 2649, 0, 0, 0 },
	{ TX_MUD1N011, 3173, 0, 0, 0 },
	{ TX_MUD1N012, 3348, 0, 0, 0 },
	{ TX_MUD1N014, 3704, 0, 0, 0 },
	{ TX_MUD1N015, 6499, 0, 0, 0 },
	{ TX_MUD1N016, 6453, 0, 0, 0 },
	{ TX_STATICU1, 5318, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText mudd1Texts[] = {
	{ TX_DEM0N009, Common::EN_ANY, "#DEM0\\DEM0N009#Spock raises an eyebrow" },
	{ -1, Common::UNK_LANG, "" }
};

void Room::mudd1Tick1() {
	playVoc("MUD1LOOP"); // BUGFIX: moved this out of below if statement

	if (!_awayMission->mudd.enteredRoom1ForFirstTime) {
		playMidiMusicTracks(MIDITRACK_0);
		_awayMission->mudd.enteredRoom1ForFirstTime = true;
	}

	loadActorAnim2(OBJECT_CRANE, "s4wbhs", 0x9b, 0x31);

	if (!_awayMission->mudd.torpedoLoaded)
		loadActorAnim(OBJECT_TORPEDO, "s4wbcs", 0x32, 0x65);
}

void Room::mudd1Timer1Expired() {
	playSoundEffectIndex(kSfxButton);
}

void Room::mudd1UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD1_001);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}


void Room::mudd1UseSpockOnBlueButton() { // Loads up the torpedo
	if (!_awayMission->mudd.torpedoLoaded) {
		walkCrewmanC(OBJECT_SPOCK, 0x2f, 0x9f, &Room::mudd1SpockReachedBlueButton);
		_awayMission->disableInput = true;
		if (!_awayMission->mudd.gotPointsForLoadingTorpedo) {
			_awayMission->mudd.gotPointsForLoadingTorpedo = true;
			_awayMission->mudd.missionScore += 2;
		}
	}
}

void Room::mudd1SpockReachedBlueButton() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::mudd1SpockPressedBlueButton);
	_awayMission->timers[1] = 5;
}

void Room::mudd1SpockPressedBlueButton() {
	playVoc("crn2can");
	loadActorAnim(OBJECT_CRANE, "s4wbcr");
	loadActorAnim(OBJECT_TORPEDO, "s4wbcn");
	loadActorAnimC(OBJECT_CRANE_ANIMATION, "s4wbsl", 0x77, 0x83, &Room::mudd1CraneFinishedMoving);
	_awayMission->mudd.torpedoLoaded = true;
}

void Room::mudd1CraneFinishedMoving() {
	_awayMission->disableInput = false;
	walkCrewman(OBJECT_SPOCK, 0x43, 0x9f);

	if (_awayMission->mudd.torpedoLoaded)
		showText(TX_SPEAKER_SPOCK, TX_MUD1_017);
	else
		showText(TX_SPEAKER_SPOCK, TX_MUD1_016);
}


void Room::mudd1UseSpockOnYellowButton() { // Unloads the torpedo
	if (_awayMission->mudd.torpedoLoaded) {
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
		walkCrewmanC(OBJECT_SPOCK, 0x2f, 0x9f, &Room::mudd1SpockReachedYellowButton);
		_awayMission->disableInput = true;
	}
}

void Room::mudd1SpockReachedYellowButton() {
	loadActorAnim2(OBJECT_SPOCK, "susemn", -1, -1, 3);
	_awayMission->timers[1] = 5;
}

void Room::mudd1SpockPressedYellowButton() {
	playVoc("crn2rack");
	loadActorAnim2(OBJECT_CRANE, "s4wbhb");
	loadActorAnim2(OBJECT_TORPEDO, "s4wbcb", 0x32, 0x65, 0);
	loadActorAnimC(OBJECT_CRANE_ANIMATION, "s4wbsb", -1, -1, &Room::mudd1CraneFinishedMoving);
	_awayMission->mudd.torpedoLoaded = false;
}


void Room::mudd1UseSpockOnRedButton() {
	if (_awayMission->mudd.torpedoLoaded) {
		walkCrewmanC(OBJECT_SPOCK, 0x2f, 0x9f, &Room::mudd1SpockReachedRedButton);
		_awayMission->disableInput = true;
		if (!_awayMission->mudd.gotPointsForPressingRedButton) {
			_awayMission->mudd.gotPointsForPressingRedButton = true;
			_awayMission->mudd.missionScore += 3;
		}
	}
}

void Room::mudd1SpockReachedRedButton() {
	loadActorAnimC(OBJECT_SPOCK, "susemn", -1, -1, &Room::mudd1SpockPressedRedButton);
	_awayMission->timers[1] = 5;
}

void Room::mudd1SpockPressedRedButton() {
	walkCrewman(OBJECT_SPOCK, 0x43, 0x9f);

	showText(TX_SPEAKER_SPOCK, TX_MUD1_022);
	showText(TX_SPEAKER_SPOCK, TX_MUD1_021);
	showText(TX_SPEAKER_SPOCK, TX_MUD1_023);

	_awayMission->disableInput = false;
	_awayMission->mudd.knowAboutTorpedo = true;

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_MUD1_004,
		TX_MUD1_003,
		TX_MUD1_007,
		TX_END
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0:
		_awayMission->mudd.torpedoStatus = 0;

		// ENHANCEMENT: Original text was just "(Spock raises eyebrow)" without any audio.
		// This changes it to a narration to make it flow better.
		showDescription(TX_DEM0N009);
		break;

	case 1:
		showText(TX_SPEAKER_KIRK, TX_MUD1_007);
	// fall through

	case 2:
		_awayMission->mudd.torpedoStatus = 1;
		showText(TX_SPEAKER_UHURA, TX_STATICU1);
		break;
	}
}


void Room::mudd1GetTorpedo() {
	showDescription(TX_MUD1N014);
}

void Room::mudd1UseSTricorderOnTorpedo() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_MUD1_025, false);
	showText(TX_SPEAKER_BUCHERT, TX_MUD1_027);
	showText(TX_SPEAKER_SPOCK,   TX_MUD1_020);
	showText(TX_SPEAKER_BUCHERT, TX_MUD1_028);
	showText(TX_SPEAKER_SPOCK,   TX_MUD1_024);
}

void Room::mudd1UseSTricorderOnTorpedoLauncher() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_MUD1_010, false);
}

void Room::mudd1UseSTricorderOnButton() {
	// ENHANCEMENT: Do the whole "spockScan" thing, don't just show the text
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_MUD1_015, false);
}

void Room::mudd1UseSTricorderOnCrane() {
	// ENHANCEMENT: See above
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_MUD1_014, false);
}

void Room::mudd1UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_MUD1_011);
}

void Room::mudd1LookAnywhere() {
	showDescription(TX_MUD1N011);
}

void Room::mudd1LookAtTorpedo() {
	showDescription(TX_MUD1N002);
}

void Room::mudd1LookAtFallenTorpedo() {
	showDescription(TX_MUD1N012);
}

void Room::mudd1LookAtTorpedoLauncher() {
	showDescription(TX_MUD1N003);
}

void Room::mudd1LookAtKirk() {
	showDescription(TX_MUD1N005);
}

void Room::mudd1LookAtSpock() {
	showDescription(TX_MUD1N009);
}

void Room::mudd1LookAtMccoy() {
	showDescription(TX_MUD1N008);
}

void Room::mudd1LookAtRedshirt() {
	showDescription(TX_MUD1N006);
}

void Room::mudd1LookAtCrane() {
	showDescription(TX_MUD1N000);
}

void Room::mudd1LookAtRedButton() {
	showDescription(TX_MUD1N001);
}

void Room::mudd1LookAtBlueButton() {
	showDescription(TX_MUD1N015);
}

void Room::mudd1LookAtYellowButton() {
	showDescription(TX_MUD1N016);
}

void Room::mudd1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_MUD1_006);
	showText(TX_SPEAKER_MCCOY, TX_MUD1_013);
	showText(TX_SPEAKER_KIRK,  TX_MUD1_005);
}

void Room::mudd1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_MUD1_019);
	showText(TX_SPEAKER_KIRK,  TX_MUD1_008);
}

void Room::mudd1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_MUD1_012);
}

void Room::mudd1TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD1_026);
	showText(TX_SPEAKER_KIRK,    TX_MUD1_009);
}

void Room::mudd1WalkToSouthDoor() {
	_roomVar.mudd.walkingToDoor = 1;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0xc7);
}

void Room::mudd1TouchedHotspot2() { // Trigger door at bottom of room
	if (_roomVar.mudd.walkingToDoor == 1) {
		playVoc("SMADOOR3");
	}
}

void Room::mudd1WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = 1;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xab, 0x71);
}

void Room::mudd1TouchedHotspot1() { // Trigger door at top of room
	if (_roomVar.mudd.walkingToDoor == 1) {
		playVoc("SMADOOR3");
		loadActorAnim(OBJECT_NORTH_DOOR, "s4wbd1", 0xab, 0x73);
	}
}

void Room::mudd1WalkToWestDoor() {
	_roomVar.mudd.walkingToDoor = 2;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x35, 0x71);
}

void Room::mudd1TouchedHotspot0() { // Trigger door at west of room
	if (_roomVar.mudd.walkingToDoor == 2) {
		playVoc("SMADOOR3");
		loadActorAnim(OBJECT_WEST_DOOR, "s4wbd2", 0x37, 0x73);
	}
}

}
