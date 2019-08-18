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
	playSoundEffectIndex(SND_07);
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
		TX_BLANK
	};

	int choice = showMultipleTexts(choices);

	switch (choice) {
	case 0:
		_awayMission->mudd.torpedoStatus = 0;

		// ENHANCEMENT: Original text was just "(Spock raises eyebrow)" without any audio.
		// This changes it to a narration to make it flow better.
		// TODO: This needs to be refactored
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
	spockScan(DIR_W, TX_MUD1_025, false);
	showText(TX_SPEAKER_BUCHERT, TX_MUD1_027);
	showText(TX_SPEAKER_SPOCK,   TX_MUD1_020);
	showText(TX_SPEAKER_BUCHERT, TX_MUD1_028);
	showText(TX_SPEAKER_SPOCK,   TX_MUD1_024);
}

void Room::mudd1UseSTricorderOnTorpedoLauncher() {
	spockScan(DIR_W, TX_MUD1_010, false);
}

void Room::mudd1UseSTricorderOnButton() {
	// ENHANCEMENT: Do the whole "spockScan" thing, don't just show the text
	spockScan(DIR_W, TX_MUD1_015, false);
}

void Room::mudd1UseSTricorderOnCrane() {
	// ENHANCEMENT: See above
	spockScan(DIR_N, TX_MUD1_014, false);
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
