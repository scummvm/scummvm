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
	{ {ACTION_TICK, 1, 0, 0}, &Room::sins3Tick1 },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_BOX,           0}, &Room::sins3UseSTricorderOnBox },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LEFT_MONITOR,  0}, &Room::sins3UseSTricorderOnMonitor },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_RIGHT_MONITOR, 0}, &Room::sins3UseSTricorderOnMonitor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,                  0}, &Room::sins3UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_DRILL,         0}, &Room::sins3UseSTricorderOnDrill },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_PANEL,         0}, &Room::sins3UseSTricorderOnPanel },

	{ {ACTION_LOOK, HOTSPOT_DRILL,         0, 0}, &Room::sins3LookAtDrill },
	{ {ACTION_LOOK, HOTSPOT_PANEL,         0, 0}, &Room::sins3LookAtPanel },
	{ {ACTION_LOOK, HOTSPOT_LEFT_MONITOR,  0, 0}, &Room::sins3LookAtMonitor },
	{ {ACTION_LOOK, HOTSPOT_RIGHT_MONITOR, 0, 0}, &Room::sins3LookAtMonitor },
	{ {ACTION_LOOK, 0xff,                  0, 0}, &Room::sins3LookAnywhere },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL, 0}, &Room::sins3UseSTricorderOnWall },

	{ {ACTION_LOOK, OBJECT_BOX,  0, 0}, &Room::sins3LookAtBox },
	{ {ACTION_LOOK, HOTSPOT_BOX, 0, 0}, &Room::sins3LookAtBox },

	{ {ACTION_USE, OBJECT_KIRK, HOTSPOT_BOX,     0}, &Room::sins3UseKirkOnBox },
	{ {ACTION_DONE_WALK, 4,                   0, 0}, &Room::sins3KirkReachedBox },
	{ {ACTION_TIMER_EXPIRED, 0,               0, 0}, &Room::sins3Timer0Expired },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_BOX,    0}, &Room::sins3UseSpockOnBox },
	{ {ACTION_DONE_WALK, 5,                   0, 0}, &Room::sins3SpockReachedBox },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_BOX,    0}, &Room::sins3UseMccoyOnBox },
	{ {ACTION_DONE_WALK, 6,                   0, 0}, &Room::sins3MccoyReachedBox },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_BOX, 0}, &Room::sins3UseRedshirtOnBox },
	{ {ACTION_DONE_WALK, 7,                   0, 0}, &Room::sins3RedshirtReachedBox },
	{ {ACTION_DONE_ANIM, 8,                   0, 0}, &Room::sins3BoxOpened },

	{ {ACTION_GET, OBJECT_BOX,  0, 0}, &Room::sins3GetBox },
	{ {ACTION_GET, HOTSPOT_BOX, 0, 0}, &Room::sins3GetBox },
	{ {ACTION_DONE_WALK, 3,     0, 0}, &Room::sins3KirkReachedBoxToGet },
	{ {ACTION_DONE_ANIM, 17,    0, 0}, &Room::sins3KirkGotBoxContents },

	{ {ACTION_TICK, 30, 0, 0}, &Room::sins3Tick30 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::sins3Tick60 },

	{ {ACTION_USE, OBJECT_KIRK,  HOTSPOT_PANEL,   0}, &Room::sins3UseKirkOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_PANEL,   0}, &Room::sins3UseSpockOnPanel },
	{ {ACTION_DONE_WALK, 10,                   0, 0}, &Room::sins3SpockReachedPanel },
	{ {ACTION_USE, OBJECT_IS8ROCKS, OBJECT_ITEM,  0}, &Room::sins3UseRockOnWall },
	{ {ACTION_USE, OBJECT_IS8ROCKS, HOTSPOT_WALL, 0}, &Room::sins3UseRockOnWall },
	{ {ACTION_DONE_WALK, 1,                    0, 0}, &Room::sins3KirkReachedWall },
	{ {ACTION_DONE_ANIM, 18,                   0, 0}, &Room::sins3KirkPutRockOnWall },
	{ {ACTION_DONE_WALK, 2,                    0, 0}, &Room::sins3KirkBackedAwayFromWall },
	{ {ACTION_DONE_ANIM, 11,                   0, 0}, &Room::sins3UsedLowPowerLaserOnRock },
	{ {ACTION_DONE_ANIM, 12,                   0, 0}, &Room::sins3MadeHoleInRock },
	{ {ACTION_DONE_ANIM, 13,                   0, 0}, &Room::sins3CreatedTemplateInRock },
	{ {ACTION_DONE_ANIM, 14,                   0, 0}, &Room::sins3RockTurnedIntoIDCard },
	{ {ACTION_DONE_ANIM, 19,                   0, 0}, &Room::sins3RockVaporized },
	{ {ACTION_USE, HOTSPOT_DRILL, 0xff,           0}, &Room::sins3UseDrillAnywhere },

	{ {ACTION_GET, OBJECT_ITEM,      0, 0}, &Room::sins3GetIDCard },
	{ {ACTION_DONE_WALK, 15,         0, 0}, &Room::sins3ReachedIDCard },
	{ {ACTION_DONE_ANIM, 16,         0, 0}, &Room::sins3PickedUpIDCard },
	{ {ACTION_LOOK, OBJECT_ITEM,     0, 0}, &Room::sins3LookAtItemBeingDrilled },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::sins3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::sins3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::sins3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::sins3LookAtRedshirt },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::sins3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::sins3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::sins3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::sins3TalkToRedshirt },

	{ {ACTION_USE,  OBJECT_ICOMM,   0xff,          0}, &Room::sins3UseCommunicator },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_DRILL, 0}, &Room::sins3UseSpockOnDrill },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_DRILL, 0}, &Room::sins3UseMccoyOnDrill },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_DRILL, 0}, &Room::sins3UseRedshirtOnDrill },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::sins3WalkToDoor },

	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_KIRK,     0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_SPOCK,    0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_MCCOY,    0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  OBJECT_REDSHIRT, 0}, &Room::sins3UseMedkitOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,     0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,    0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,    0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0}, &Room::sins3UseMTricorderOnCrewman },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
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
	playMidiMusicTracks(MIDITRACK_27, -3);
}

void Room::sins3UseSTricorderOnBox() {
	spockScan(DIR_S, 3, false, true);
}

void Room::sins3UseSTricorderOnMonitor() {
	spockScan(DIR_N, 18, false, true);
}

void Room::sins3UseSTricorderAnywhere() {
	spockScan(DIR_S, 17, false, true);
}

void Room::sins3UseSTricorderOnDrill() {
	spockScan(DIR_E, 16, false, true);
}

void Room::sins3UseSTricorderOnPanel() {
	if (_awayMission->sins.scannedKeycardLock) {
		spockScan(DIR_E, 15, false, true);
		_awayMission->sins.laserPattern = 1;
	} else
		spockScan(DIR_E, 19, false, true);
}

void Room::sins3LookAtDrill() {
	showDescription(12, true);
}

void Room::sins3LookAtPanel() {
	showDescription(11, true);
}

void Room::sins3LookAtMonitor() {
	showDescription(7, true);
}

void Room::sins3LookAnywhere() {
	showDescription(13, true);
}

void Room::sins3UseSTricorderOnWall() {
	// NOTE: this event has two implementations (index 23 and 22), one unused.
	spockScan(DIR_E, 23, false, true);
}

void Room::sins3LookAtBox() {
	if (_awayMission->sins.boxState == 0)
		showDescription(0, true);
	else if (_awayMission->sins.boxState == 1)
		showDescription(8, true);
	else if (_awayMission->sins.boxState == 2)
		showDescription(5, true);
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
		showText(TX_SPEAKER_MCCOY, 12, true);
		showText(TX_SPEAKER_SPOCK, 24, true);
		showText(TX_SPEAKER_MCCOY, 13, true);
		_awayMission->sins.enteredRoom3FirstTime = true;
		_awayMission->disableInput = false;
	}
}

void Room::sins3Tick60() {
	if (_awayMission->sins.gatheredClues == 7) {
		showText(TX_SPEAKER_SPOCK, 7, true);
		_awayMission->sins.gatheredClues |= 8;
	}
}

void Room::sins3UseKirkOnPanel() {
	showText(TX_SPEAKER_KIRK, 1, true);
	sins3UseSpockOnPanel();
}

void Room::sins3UseSpockOnPanel() {
	walkCrewmanC(OBJECT_SPOCK, 0x96, 0x78, &Room::sins3SpockReachedPanel);
}

void Room::sins3SpockReachedPanel() {
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_SIN3_LASERSETTING001, TX_SIN3_LASERSETTING010, TX_SIN3_LASERSETTING100, TX_SIN3_LASERCANCEL,
		TX_BLANK
	};

	int choice = showMultipleTexts(choices);

	if (choice == 3) // cancel
		return;
	else {
		_awayMission->sins.laserSetting = choice + 1;

		if (_awayMission->sins.laserPattern == 0 && _awayMission->sins.laserSetting == 3)
			// High setting can't be used until the template pattern has been uploaded to
			// the machine
			showText(TX_SPEAKER_SPOCK, 5, true);
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
				showText(TX_SPEAKER_SPOCK, 6, true);
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
	showText(TX_SPEAKER_SPOCK, 21, true);
}

void Room::sins3UseDrillAnywhere() {
	// It isn't possible to "use" a hotspot on something else, so this is never called?
	showDescription(9, true);
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
		showDescription(6, true);
	else if (_awayMission->sins.moldState == 3)
		showDescription(14, true);
	else if (_awayMission->sins.moldState == 4)
		showDescription(10, true);
}

void Room::sins3LookAtKirk() {
	showDescription(3, true);
}

void Room::sins3LookAtSpock() {
	showDescription(4, true);
}

void Room::sins3LookAtMccoy() {
	showDescription(1, true); // BUGFIX: Speaker is "nobody", not Dr. McCoy
}

void Room::sins3LookAtRedshirt() {
	showDescription(2, true);
}

void Room::sins3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 2, true);
}

void Room::sins3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 20, true);
}

void Room::sins3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 11, true);
}

void Room::sins3TalkToRedshirt() {
	showText(TX_SPEAKER_MOSHER, 26, true);
	showText(TX_SPEAKER_MCCOY,  14, true);
	showText(TX_SPEAKER_MOSHER, 27, true);
}

void Room::sins3UseCommunicator() {
	showText(TX_SPEAKER_UHURA, 72, true);
}

void Room::sins3UseSpockOnDrill() {
	showText(TX_SPEAKER_SPOCK, 4, true);
}

void Room::sins3UseMccoyOnDrill() {
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::sins3UseRedshirtOnDrill() {
	showText(TX_SPEAKER_MOSHER, 25, true);
}

void Room::sins3WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x2b, 0x86);
}

void Room::sins3UseMedkitOnCrewman() {
	showText(TX_SPEAKER_MCCOY, 10, true);
}

void Room::sins3UseMTricorderOnCrewman() {
	mccoyScan(DIR_S, 8, false, true);
}

}
