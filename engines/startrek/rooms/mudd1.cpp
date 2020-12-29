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
	{ {ACTION_TICK, 1, 0, 0}, &Room::mudd1Tick1 },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::mudd1Timer1Expired },

	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},    &Room::mudd1UseCommunicator },
	{ {ACTION_USE, OBJECT_SPOCK, 0x23, 0},  &Room::mudd1UseSpockOnBlueButton },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0},   &Room::mudd1SpockReachedBlueButton },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::mudd1SpockPressedBlueButton },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::mudd1CraneFinishedMoving },
	{ {ACTION_USE, OBJECT_SPOCK, 0x24, 0},  &Room::mudd1UseSpockOnYellowButton },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0},   &Room::mudd1SpockReachedYellowButton },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::mudd1SpockPressedYellowButton },
	{ {ACTION_USE, OBJECT_SPOCK, 0x25, 0},  &Room::mudd1UseSpockOnRedButton },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0},   &Room::mudd1SpockReachedRedButton },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::mudd1SpockPressedRedButton },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0}, &Room::muddaUseDegrimer },
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0}, &Room::muddaFiredAlienDevice },

	{ {ACTION_GET, 9,    0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_GET, 0x21, 0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_GET, 0x20, 0, 0}, &Room::mudd1GetTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 9,    0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0}, &Room::mudd1UseSTricorderOnTorpedo },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x22, 0}, &Room::mudd1UseSTricorderOnTorpedoLauncher },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x25, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x23, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x24, 0}, &Room::mudd1UseSTricorderOnButton },
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0},    &Room::mudd1UseSTricorderOnCrane },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},  &Room::mudd1UseMedkitAnywhere },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::mudd1LookAnywhere },
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::mudd1LookAtTorpedo },
	{ {ACTION_LOOK, 9,    0, 0}, &Room::mudd1LookAtTorpedo },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::mudd1LookAtFallenTorpedo },
	{ {ACTION_LOOK, 0x22, 0, 0}, &Room::mudd1LookAtTorpedoLauncher },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::mudd1LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::mudd1LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::mudd1LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd1LookAtRedshirt },
	{ {ACTION_LOOK, 8,    0, 0}, &Room::mudd1LookAtCrane },
	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::mudd1LookAtRedButton },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::mudd1LookAtBlueButton },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::mudd1LookAtYellowButton },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::mudd1TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::mudd1TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::mudd1TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd1TalkToRedshirt },
	{ {ACTION_WALK, 0x28, 0, 0}, &Room::mudd1WalkToSouthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 2, 0, 0}, &Room::mudd1TouchedHotspot2 },
	{ {ACTION_WALK, 0x27, 0, 0}, &Room::mudd1WalkToNorthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0}, &Room::mudd1TouchedHotspot1 },
	{ {ACTION_WALK, 0x26, 0, 0}, &Room::mudd1WalkToWestDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::mudd1TouchedHotspot0 },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::mudd1Tick1() {
	playVoc("MUD1LOOP"); // BUGFIX: moved this out of below if statement

	if (!_awayMission->mudd.enteredRoom1ForFirstTime) {
		playMidiMusicTracks(0);
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
	showText(TX_SPEAKER_KIRK,  1, true);
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
		showText(TX_SPEAKER_SPOCK, 17, true);
	else
		showText(TX_SPEAKER_SPOCK, 16, true);
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

	showText(TX_SPEAKER_SPOCK, 22, true);
	showText(TX_SPEAKER_SPOCK, 21, true);
	showText(TX_SPEAKER_SPOCK, 23, true);

	_awayMission->disableInput = false;
	_awayMission->mudd.knowAboutTorpedo = true;

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		4,
		3,
		7,
		TX_BLANK
	};

	int choice = showMultipleTexts(choices, true);

	switch (choice) {
	case 0:
		_awayMission->mudd.torpedoStatus = 0;

		// ENHANCEMENT: Original text was just "(Spock raises eyebrow)" without any audio.
		// This changes it to a narration to make it flow better.
		// TODO: This needs to be refactored
		showDescription(TX_DEM0N009);
		break;

	case 1:
		showText(TX_SPEAKER_KIRK, 7, true);
	// fall through

	case 2:
		_awayMission->mudd.torpedoStatus = 1;
		showText(TX_SPEAKER_UHURA, TX_STATICU1);
		break;

	default:
		break;
	}
}

void Room::mudd1GetTorpedo() {
	showDescription(14, true);
}

void Room::mudd1UseSTricorderOnTorpedo() {
	spockScan(DIR_W, 25, false, true);
	showText(TX_SPEAKER_BUCHERT, 27, true);
	showText(TX_SPEAKER_SPOCK,   20, true);
	showText(TX_SPEAKER_BUCHERT, 28, true);
	showText(TX_SPEAKER_SPOCK,   24, true);
}

void Room::mudd1UseSTricorderOnTorpedoLauncher() {
	spockScan(DIR_W, 10, false, true);
}

void Room::mudd1UseSTricorderOnButton() {
	// ENHANCEMENT: Do the whole "spockScan" thing, don't just show the text
	spockScan(DIR_W, 15, false, true);
}

void Room::mudd1UseSTricorderOnCrane() {
	// ENHANCEMENT: See above
	spockScan(DIR_N, 14, false, true);
}

void Room::mudd1UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::mudd1LookAnywhere() {
	showDescription(11, true);
}

void Room::mudd1LookAtTorpedo() {
	showDescription(2, true);
}

void Room::mudd1LookAtFallenTorpedo() {
	showDescription(12, true);
}

void Room::mudd1LookAtTorpedoLauncher() {
	showDescription(3, true);
}

void Room::mudd1LookAtKirk() {
	showDescription(5, true);
}

void Room::mudd1LookAtSpock() {
	showDescription(9, true);
}

void Room::mudd1LookAtMccoy() {
	showDescription(8, true);
}

void Room::mudd1LookAtRedshirt() {
	showDescription(6, true);
}

void Room::mudd1LookAtCrane() {
	showDescription(0, true);
}

void Room::mudd1LookAtRedButton() {
	showDescription(1, true);
}

void Room::mudd1LookAtBlueButton() {
	showDescription(15, true);
}

void Room::mudd1LookAtYellowButton() {
	showDescription(16, true);
}

void Room::mudd1TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  6, true);
	showText(TX_SPEAKER_MCCOY, 13, true);
	showText(TX_SPEAKER_KIRK,  5, true);
}

void Room::mudd1TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 19, true);
	showText(TX_SPEAKER_KIRK,  8, true);
}

void Room::mudd1TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::mudd1TalkToRedshirt() {
	showText(TX_SPEAKER_BUCHERT, 26, true);
	showText(TX_SPEAKER_KIRK,    9, true);
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
