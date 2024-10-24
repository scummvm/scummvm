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

#define OBJECT_ITEM 8 // The item being "operated" on by the drill
#define OBJECT_9 9
#define OBJECT_10 10
#define OBJECT_BOX 11

#define HOTSPOT_DRILL 0x20
#define HOTSPOT_PANEL 0x21
#define HOTSPOT_BOX 0x22
#define HOTSPOT_WALL 0x23
#define HOTSPOT_DOOR 0x24
#define HOTSPOT_LEFT_MONITOR 0x25
#define HOTSPOT_RIGHT_MONITOR 0x26

namespace StarTrek {

extern const RoomAction sins3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::sins3Tick1 },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_BOX,           0, 0}, &Room::sins3UseSTricorderOnBox },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LEFT_MONITOR,  0, 0}, &Room::sins3UseSTricorderOnMonitor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_MONITOR, 0, 0}, &Room::sins3UseSTricorderOnMonitor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,                  0, 0}, &Room::sins3UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DRILL,         0, 0}, &Room::sins3UseSTricorderOnDrill },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL,         0, 0}, &Room::sins3UseSTricorderOnPanel },

	{ {ACTION_LOOK, HOTSPOT_DRILL,         0, 0, 0}, &Room::sins3LookAtDrill },
	{ {ACTION_LOOK, HOTSPOT_PANEL,         0, 0, 0}, &Room::sins3LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_LEFT_MONITOR,  0, 0, 0}, &Room::sins3LookAtMonitor },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_MONITOR, 0, 0, 0}, &Room::sins3LookAtMonitor },
	{ {ACTION_LOOK, 0xff,                  0, 0, 0}, &Room::sins3LookAnywhere },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL, 0, 0}, &Room::sins3UseSTricorderOnWall },

	{ {ACTION_LOOK, OBJECT_BOX,  0, 0, 0}, &Room::sins3LookAtBox },
	{ {ACTION_LOOK, HOTSPOT_BOX, 0, 0, 0}, &Room::sins3LookAtBox },

	{ {ACTION_USE, OBJECT_KIRK, HOTSPOT_BOX,     0, 0}, &Room::sins3UseKirkOnBox },
	{ {ACTION_DONE_WALK, 4,                   0, 0, 0}, &Room::sins3KirkReachedBox },
	{ {ACTION_TIMER_EXPIRED, 0,               0, 0, 0}, &Room::sins3Timer0Expired },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_BOX,    0, 0}, &Room::sins3UseSpockOnBox },
	{ {ACTION_DONE_WALK, 5,                   0, 0, 0}, &Room::sins3SpockReachedBox },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_BOX,    0, 0}, &Room::sins3UseMccoyOnBox },
	{ {ACTION_DONE_WALK, 6,                   0, 0, 0}, &Room::sins3MccoyReachedBox },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_BOX, 0, 0}, &Room::sins3UseRedshirtOnBox },
	{ {ACTION_DONE_WALK, 7,                   0, 0, 0}, &Room::sins3RedshirtReachedBox },
	{ {ACTION_DONE_ANIM, 8,                   0, 0, 0}, &Room::sins3BoxOpened },

	{ {ACTION_GET, OBJECT_BOX,  0, 0, 0}, &Room::sins3GetBox },
	{ {ACTION_GET, HOTSPOT_BOX, 0, 0, 0}, &Room::sins3GetBox },
	{ {ACTION_DONE_WALK, 3,     0, 0, 0}, &Room::sins3KirkReachedBoxToGet },
	{ {ACTION_DONE_ANIM, 17,    0, 0, 0}, &Room::sins3KirkGotBoxContents },

	{ {ACTION_TICK, 30, 0, 0, 0}, &Room::sins3Tick30 },
	{ {ACTION_TICK, 60, 0, 0, 0}, &Room::sins3Tick60 },

	{ {ACTION_USE, OBJECT_KIRK,  HOTSPOT_PANEL,   0, 0}, &Room::sins3UseKirkOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_PANEL,   0, 0}, &Room::sins3UseSpockOnPanel },
	{ {ACTION_DONE_WALK, 10,                   0, 0, 0}, &Room::sins3SpockReachedPanel },
	{ {ACTION_USE, OBJECT_IS8ROCKS, OBJECT_ITEM,  0, 0}, &Room::sins3UseRockOnWall },
	{ {ACTION_USE, OBJECT_IS8ROCKS, HOTSPOT_WALL, 0, 0}, &Room::sins3UseRockOnWall },
	{ {ACTION_DONE_WALK, 1,                    0, 0, 0}, &Room::sins3KirkReachedWall },
	{ {ACTION_DONE_ANIM, 18,                   0, 0, 0}, &Room::sins3KirkPutRockOnWall },
	{ {ACTION_DONE_WALK, 2,                    0, 0, 0}, &Room::sins3KirkBackedAwayFromWall },
	{ {ACTION_DONE_ANIM, 11,                   0, 0, 0}, &Room::sins3UsedLowPowerLaserOnRock },
	{ {ACTION_DONE_ANIM, 12,                   0, 0, 0}, &Room::sins3MadeHoleInRock },
	{ {ACTION_DONE_ANIM, 13,                   0, 0, 0}, &Room::sins3CreatedTemplateInRock },
	{ {ACTION_DONE_ANIM, 14,                   0, 0, 0}, &Room::sins3RockTurnedIntoIDCard },
	{ {ACTION_DONE_ANIM, 19,                   0, 0, 0}, &Room::sins3RockVaporized },
	{ {ACTION_USE, HOTSPOT_DRILL, 0xff,           0, 0}, &Room::sins3UseDrillAnywhere },

	{ {ACTION_GET, OBJECT_ITEM,      0, 0, 0}, &Room::sins3GetIDCard },
	{ {ACTION_DONE_WALK, 15,         0, 0, 0}, &Room::sins3ReachedIDCard },
	{ {ACTION_DONE_ANIM, 16,         0, 0, 0}, &Room::sins3PickedUpIDCard },
	{ {ACTION_LOOK, OBJECT_ITEM,     0, 0, 0}, &Room::sins3LookAtItemBeingDrilled },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::sins3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins3LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::sins3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::sins3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::sins3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::sins3TalkToRedshirt },

	{ {ACTION_USE,  OBJECT_ICOMM,   0xff,          0, 0}, &Room::sins3UseCommunicator },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_DRILL, 0, 0}, &Room::sins3UseSpockOnDrill },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_DRILL, 0, 0}, &Room::sins3UseMccoyOnDrill },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_DRILL, 0, 0}, &Room::sins3UseRedshirtOnDrill },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0, 0}, &Room::sins3WalkToDoor },

	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0, 0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0, 0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0, 0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0, 0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0, 0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0, 0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0, 0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum sins3TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_MOSHER,
	TX_SPEAKER_UHURA,
	TX_SIN3_001, TX_SIN3_002, TX_SIN3_003, TX_SIN3_004, TX_SIN3_005,
	TX_SIN3_006, TX_SIN3_007, TX_SIN3_008, TX_SIN3_009, TX_SIN3_010,
	TX_SIN3_011, TX_SIN3_012, TX_SIN3_013, TX_SIN3_014, TX_SIN3_015,
	TX_SIN3_016, TX_SIN3_017, TX_SIN3_018, TX_SIN3_019, TX_SIN3_020,
	TX_SIN3_021, TX_SIN3_022, TX_SIN3_023, TX_SIN3_024, TX_SIN3_025,
	TX_SIN3_026, TX_SIN3_027, TX_SIN3N000, TX_SIN3N001, TX_SIN3N002,
	TX_SIN3N003, TX_SIN3N004, TX_SIN3N005, TX_SIN3N006, TX_SIN3N007,
	TX_SIN3N008, TX_SIN3N009, TX_SIN3N010, TX_SIN3N011, TX_SIN3N012,
	TX_SIN3N013, TX_SIN3N014, TX_SIN3U072,  TX_SIN3_LASERSETTING001,
	TX_SIN3_LASERSETTING010, TX_SIN3_LASERSETTING100, TX_SIN3_LASERCANCEL
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets sins3TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 289, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 300, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 310, 0, 0, 0 },
	{ TX_SPEAKER_MOSHER, 320, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 334, 0, 0, 0 },
	{ TX_SIN3_001, 3479, 0, 0, 0 },
	{ TX_SIN3_002, 5966, 0, 0, 0 },
	{ TX_SIN3_003, 870, 0, 0, 0 },
	{ TX_SIN3_004, 6675, 0, 0, 0 },
	{ TX_SIN3_005, 3812, 0, 0, 0 },
	{ TX_SIN3_006, 4217, 0, 0, 0 },
	{ TX_SIN3_007, 7257, 0, 0, 0 },
	{ TX_SIN3_008, 7154, 0, 0, 0 },
	{ TX_SIN3_009, 6769, 0, 0, 0 },
	{ TX_SIN3_010, 7021, 0, 0, 0 },
	{ TX_SIN3_011, 6146, 0, 0, 0 },
	{ TX_SIN3_012, 3244, 0, 0, 0 },
	{ TX_SIN3_013, 3392, 0, 0, 0 },
	{ TX_SIN3_014, 6325, 0, 0, 0 },
	{ TX_SIN3_015, 1632, 0, 0, 0 },
	{ TX_SIN3_016, 1346, 0, 0, 0 },
	{ TX_SIN3_017, 1158, 0, 0, 0 },
	{ TX_SIN3_018, 1019, 0, 0, 0 },
	{ TX_SIN3_019, 1518, 0, 0, 0 },
	{ TX_SIN3_020, 6029, 0, 0, 0 },
	{ TX_SIN3_021, 4924, 0, 0, 0 },
	{ TX_SIN3_022, 6477, 0, 0, 0 },
	{ TX_SIN3_023, 2190, 0, 0, 0 },
	{ TX_SIN3_024, 3298, 0, 0, 0 },
	{ TX_SIN3_025, 6885, 0, 0, 0 },
	{ TX_SIN3_026, 6257, 0, 0, 0 },
	{ TX_SIN3_027, 6389, 0, 0, 0 },
	{ TX_SIN3U072, 6611, 0, 0, 0 },
	{ TX_SIN3N000, 2293, 0, 0, 0 },
	{ TX_SIN3N001, 5755, 0, 0, 0 },
	{ TX_SIN3N002, 5836, 0, 0, 0 },
	{ TX_SIN3N003, 5572, 0, 0, 0 },
	{ TX_SIN3N004, 5651, 0, 0, 0 },
	{ TX_SIN3N005, 2434, 0, 0, 0 },
	{ TX_SIN3N006, 5307, 0, 0, 0 },
	{ TX_SIN3N007, 1959, 0, 0, 0 },
	{ TX_SIN3N008, 2358, 0, 0, 0 },
	{ TX_SIN3N009, 5022, 0, 0, 0 },
	{ TX_SIN3N010, 5480, 0, 0, 0 },
	{ TX_SIN3N011, 1881, 0, 0, 0 },
	{ TX_SIN3N012, 1816, 0, 0, 0 },
	{ TX_SIN3N013, 2038, 0, 0, 0 },
	{ TX_SIN3N014, 5404, 0, 0, 0 },
	{ TX_SIN3_LASERSETTING001, 3605, 0, 0, 0 },
	{ TX_SIN3_LASERSETTING100, 3675, 0, 0, 0 },
	{ TX_SIN3_LASERSETTING010, 3640, 0, 0, 0 },
	{ TX_SIN3_LASERCANCEL, 3710, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText sins3Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::sins3Tick1() {
	playVoc("SIN3LOOP");

	if (!_awayMission->sins.enteredRoom3FirstTime)
		_awayMission->disableInput = 2;

	if (_awayMission->sins.boxState == 1)
		loadActorAnim2(OBJECT_BOX, "s3bxo", 0xaf, 0xb8);
	else if (_awayMission->sins.boxState == 2)
		loadActorAnim2(OBJECT_BOX, "s3bxe", 0xaf, 0xb8);

	if (_awayMission->sins.moldState == 2)
		loadActorAnim2(OBJECT_ITEM, "s3mold", 0, 0);
	if (_awayMission->sins.moldState == 3)
		loadActorAnim2(OBJECT_ITEM, "s3rock", 0, 0);
	if (_awayMission->sins.moldState == 4)
		loadActorAnim2(OBJECT_ITEM, "s3card", 0, 0);

	_awayMission->sins.gatheredClues |= 2;
	playMidiMusicTracks(MIDITRACK_27, kLoopTypeRepeat);
}

void Room::sins3UseSTricorderOnBox() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN3_003);
}

void Room::sins3UseSTricorderOnMonitor() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_SIN3_018);
}

void Room::sins3UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_SIN3_017);
}

void Room::sins3UseSTricorderOnDrill() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_SIN3_016);
}

void Room::sins3UseSTricorderOnPanel() {
	if (_awayMission->sins.scannedKeycardLock) {
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_SIN3_015);
		_awayMission->sins.laserPattern = 1;
	} else
		spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_SIN3_019);
}

void Room::sins3LookAtDrill() {
	showDescription(TX_SIN3N012);
}

void Room::sins3LookAtPanel() {
	showDescription(TX_SIN3N011);
}

void Room::sins3LookAtMonitor() {
	showDescription(TX_SIN3N007);
}

void Room::sins3LookAnywhere() {
	showDescription(TX_SIN3N013);
}

void Room::sins3UseSTricorderOnWall() {
	// NOTE: this event has two implementations (index 23 and 22), one unused.
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_SIN3_023);
}

void Room::sins3LookAtBox() {
	if (_awayMission->sins.boxState == 0)
		showDescription(TX_SIN3N000);
	else if (_awayMission->sins.boxState == 1)
		showDescription(TX_SIN3N008);
	else if (_awayMission->sins.boxState == 2)
		showDescription(TX_SIN3N005);
}

void Room::sins3UseKirkOnBox() {
	if (_awayMission->sins.boxState == 0) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		walkCrewmanC(OBJECT_KIRK, 0x8f, 0xa9, &Room::sins3KirkReachedBox);
	}
}

void Room::sins3KirkReachedBox() {
	loadActorAnim2(OBJECT_KIRK, "kusele");
	loadActorAnimC(OBJECT_BOX, "s3bxop", 0xaf, 0xb8, &Room::sins3BoxOpened);
	_awayMission->timers[0] = 10; // play sound in 10 ticks
	_awayMission->sins.boxState = 1;
}

// Plays a sound effect at a specific time while opening the box
void Room::sins3Timer0Expired() {
}

void Room::sins3UseSpockOnBox() {
	if (_awayMission->sins.boxState == 0) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
		walkCrewmanC(OBJECT_SPOCK, 0x8f, 0xa9, &Room::sins3SpockReachedBox);
	}
}

void Room::sins3SpockReachedBox() {
	loadActorAnim2(OBJECT_SPOCK, "susele");
	loadActorAnimC(OBJECT_BOX, "s3bxop", 0xaf, 0xb8, &Room::sins3BoxOpened);
	_awayMission->timers[0] = 10; // ENHANCEMENT (play sound in 10 ticks)
	_awayMission->sins.boxState = 1;
}

void Room::sins3UseMccoyOnBox() {
	if (_awayMission->sins.boxState == 0) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
		walkCrewmanC(OBJECT_MCCOY, 0x8f, 0xa9, &Room::sins3MccoyReachedBox);
	}
}

void Room::sins3MccoyReachedBox() {
	loadActorAnim2(OBJECT_MCCOY, "musele");
	loadActorAnimC(OBJECT_BOX, "s3bxop", 0xaf, 0xb8, &Room::sins3BoxOpened);
	_awayMission->timers[0] = 10; // ENHANCEMENT (play sound in 10 ticks)
	_awayMission->sins.boxState = 1;
}

void Room::sins3UseRedshirtOnBox() {
	if (_awayMission->sins.boxState == 0) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_E;
		walkCrewmanC(OBJECT_REDSHIRT, 0x8f, 0xa9, &Room::sins3RedshirtReachedBox);
	}
}

void Room::sins3RedshirtReachedBox() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusele");
	loadActorAnimC(OBJECT_BOX, "s3bxop", 0xaf, 0xb8, &Room::sins3BoxOpened);
	_awayMission->timers[0] = 10; // ENHANCEMENT (play sound in 10 ticks)
	_awayMission->sins.boxState = 1;
}

void Room::sins3BoxOpened() {
	loadActorAnim2(OBJECT_BOX, "s3bxo", 0xaf, 0xb8);
	_awayMission->disableInput = false;
}

void Room::sins3GetBox() {
	if (_awayMission->sins.boxState == 1) { // Box is open, not empty
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
		walkCrewmanC(OBJECT_KIRK, 0x91, 0xab, &Room::sins3KirkReachedBoxToGet);
	}
}

void Room::sins3KirkReachedBoxToGet() {
	giveItem(OBJECT_ICONECT);
	loadActorAnimC(OBJECT_KIRK, "kusele", -1, -1, &Room::sins3KirkGotBoxContents);
}

void Room::sins3KirkGotBoxContents() {
	loadActorAnim2(OBJECT_BOX, "s3bxe", 0xaf, 0xb8);
	_awayMission->sins.boxState = 2;
	_awayMission->disableInput = false;
}

void Room::sins3Tick30() {
	if (!_awayMission->sins.enteredRoom3FirstTime) {
		showText(TX_SPEAKER_MCCOY, TX_SIN3_012);
		showText(TX_SPEAKER_SPOCK, TX_SIN3_024);
		showText(TX_SPEAKER_MCCOY, TX_SIN3_013);
		_awayMission->sins.enteredRoom3FirstTime = true;
		_awayMission->disableInput = false;
	}
}

void Room::sins3Tick60() {
	if (_awayMission->sins.gatheredClues == 7) {
		showText(TX_SPEAKER_SPOCK, TX_SIN3_007);
		_awayMission->sins.gatheredClues |= 8;
	}
}

void Room::sins3UseKirkOnPanel() {
	showText(TX_SPEAKER_KIRK, TX_SIN3_001);
	sins3UseSpockOnPanel();
}

void Room::sins3UseSpockOnPanel() {
	walkCrewmanC(OBJECT_SPOCK, 0x96, 0x78, &Room::sins3SpockReachedPanel);
}

void Room::sins3SpockReachedPanel() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_SIN3_LASERSETTING001, TX_SIN3_LASERSETTING010, TX_SIN3_LASERSETTING100, TX_SIN3_LASERCANCEL,
		TX_END
	};

	int choice = showMultipleTexts(choices);

	if (choice == 3) // cancel
		return;
	else {
		_awayMission->sins.laserSetting = choice + 1;

		if (_awayMission->sins.laserPattern == 0 && _awayMission->sins.laserSetting == 3)
			// High setting can't be used until the template pattern has been uploaded to
			// the machine
			showText(TX_SPEAKER_SPOCK, TX_SIN3_005);
		else if (_awayMission->sins.moldState != -1) {
			if (_awayMission->sins.moldState == 0) { // Mold not yet created
				loadActorAnim2(OBJECT_SPOCK, "susemn");
				playVoc("LAZERD2");

				if (_awayMission->sins.laserPattern == 0 && _awayMission->sins.laserSetting == 3) {
					// This code is unreachable, as the same condition was checked above
					loadActorAnimC(OBJECT_ITEM, "s3las2", 0, 0, &Room::sins3MadeHoleInRock);
					playSoundEffectIndex(kSfxPhaser);
					_awayMission->sins.moldState = -1;
				} else if (_awayMission->sins.laserPattern == 1 && _awayMission->sins.laserSetting == 3) {
					// Strong laser creates the needed mold
					loadActorAnimC(OBJECT_ITEM, "s3las3", 0, 0, &Room::sins3CreatedTemplateInRock);
					playSoundEffectIndex(kSfxPhaser);
					_awayMission->sins.moldState = 2;
				} else {
					// Weak laser does nothing to the rock
					loadActorAnimC(OBJECT_ITEM, "s3las1", 0, 0, &Room::sins3UsedLowPowerLaserOnRock);
					playSoundEffectIndex(kSfxPhaser);
				}
			} else if (_awayMission->sins.moldState == 3) { // There's a rock sitting on the mold
				loadActorAnim2(OBJECT_SPOCK, "susemn");
				playVoc("LAZERD2");

				if (_awayMission->sins.laserSetting == 1 || _awayMission->sins.laserSetting == 2) {
					// Weak-setting lasers create the keycard properly
					loadActorAnimC(OBJECT_ITEM, "s3las4", 0, 0, &Room::sins3RockTurnedIntoIDCard);
					playSoundEffectIndex(kSfxPhaser);
					_awayMission->sins.moldState = 4;
				} else if (_awayMission->sins.laserSetting == 3) {
					// High-setting laser vaporizes the rocks
					loadActorAnimC(OBJECT_ITEM, "s3las5", 0, 0, &Room::sins3RockVaporized);
					playSoundEffectIndex(kSfxPhaser);
					_awayMission->sins.moldState = 2;
					_awayMission->sins.laserPattern = 1;
				}
			} else { // moldState == 2 or 4 (mold created; either keycard is in it, or nothing)
				// "Template would be damaged from direct fire"
				showText(TX_SPEAKER_SPOCK, TX_SIN3_006);
			}
		}
	}
}

void Room::sins3UseRockOnWall() {
	if (_awayMission->sins.moldState == 2) {
		_awayMission->sins.moldState = 3;
		_awayMission->sins.laserPattern = 2;
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x10f, 0xa7, &Room::sins3KirkReachedWall);
	}
}

void Room::sins3KirkReachedWall() {
	loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::sins3KirkPutRockOnWall);
}

void Room::sins3KirkPutRockOnWall() {
	loseItem(OBJECT_IS8ROCKS);
	loadActorAnim2(OBJECT_ITEM, "s3rock", 0, 0);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_E;
	walkCrewmanC(OBJECT_KIRK, 0xfa, 0xaa, &Room::sins3KirkBackedAwayFromWall);
}

void Room::sins3KirkBackedAwayFromWall() {
	_awayMission->disableInput = false;
}

void Room::sins3UsedLowPowerLaserOnRock() {
	loadActorStandAnim(OBJECT_ITEM);
}

void Room::sins3MadeHoleInRock() {
	// Unused
	loadActorAnim2(OBJECT_ITEM, "s3hole", 0, 0);
}

void Room::sins3CreatedTemplateInRock() {
	loadActorAnim2(OBJECT_ITEM, "s3mold", 0, 0);
}

void Room::sins3RockTurnedIntoIDCard() {
	loadActorAnim2(OBJECT_ITEM, "s3card", 0, 0);
}

void Room::sins3RockVaporized() {
	showText(TX_SPEAKER_SPOCK, TX_SIN3_021);
}

void Room::sins3UseDrillAnywhere() {
	// It isn't possible to "use" a hotspot on something else, so this is never called?
	showDescription(TX_SIN3N009);
}

void Room::sins3GetIDCard() {
	if (_awayMission->sins.moldState == 4) {
		walkCrewmanC(OBJECT_KIRK, 0x10f, 0xa7, &Room::sins3ReachedIDCard);
		_awayMission->disableInput = true;
	}
}

void Room::sins3ReachedIDCard() {
	loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::sins3PickedUpIDCard);
}

void Room::sins3PickedUpIDCard() {
	giveItem(OBJECT_IIDCARD);
	loadActorAnim(OBJECT_ITEM, "s3mold", 0, 0);
	_awayMission->sins.moldState = 2;
	_awayMission->sins.laserPattern = 1;
	_awayMission->disableInput = false;
}

void Room::sins3LookAtItemBeingDrilled() {
	if (_awayMission->sins.moldState == 2)
		showDescription(TX_SIN3N006);
	else if (_awayMission->sins.moldState == 3)
		showDescription(TX_SIN3N014);
	else if (_awayMission->sins.moldState == 4)
		showDescription(TX_SIN3N010);
}

void Room::sins3LookAtKirk() {
	showDescription(TX_SIN3N003);
}

void Room::sins3LookAtSpock() {
	showDescription(TX_SIN3N004);
}

void Room::sins3LookAtMccoy() {
	showDescription(TX_SIN3N001); // BUGFIX: Speaker is "nobody", not Dr. McCoy
}

void Room::sins3LookAtRedshirt() {
	showDescription(TX_SIN3N002);
}

void Room::sins3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_SIN3_002);
}

void Room::sins3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_SIN3_020);
}

void Room::sins3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_SIN3_011);
}

void Room::sins3TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, TX_SIN3_026);
	showText(TX_SPEAKER_MCCOY,  TX_SIN3_014);
	showText(TX_SPEAKER_MOSHER, TX_SIN3_027);
}

void Room::sins3UseCommunicator() {
	showText(TX_SPEAKER_UHURA, TX_SIN3U072);
}

void Room::sins3UseSpockOnDrill() {
	showText(TX_SPEAKER_SPOCK, TX_SIN3_004);
}

void Room::sins3UseMccoyOnDrill() {
	showText(TX_SPEAKER_MCCOY, TX_SIN3_009);
}

void Room::sins3UseRedshirtOnDrill() {
	showText(TX_SPEAKER_MOSHER, TX_SIN3_025);
}

void Room::sins3WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x2b, 0x86);
}

void Room::sins3UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, TX_SIN3_010);
}

void Room::sins3UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, TX_SPEAKER_MCCOY, TX_SIN3_008);
}

}
