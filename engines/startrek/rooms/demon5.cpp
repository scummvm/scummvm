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

extern const RoomAction demon5ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::demon5Tick1 },
	{ {ACTION_WALK, 0x22, 0, 0}, &Room::demon5WalkToDoor },
	{ {ACTION_WALK, 12, 0, 0}, &Room::demon5WalkToDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::demon5TouchedDoorOpenTrigger },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::demon5DoorOpenedOrReachedDoor },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::demon5DoorOpenedOrReachedDoor },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::demon5UseSTricorderOnCrate },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::demon5UsePhaserOnAnything },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::demon5UsePhaserOnAnything },

	{ {ACTION_USE, OBJECT_IHAND, 11, 0}, &Room::demon5UseHandOnStephen },
	{ {ACTION_USE, OBJECT_IBERRY, 11, 0}, &Room::demon5UseBerryOnStephen },
	{ {ACTION_USE, OBJECT_IDETOXIN, 9, 0}, &Room::demon5UseHypoDytoxinOnChub },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::demon5MccoyReachedChub },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::demon5MccoyHealedChub },

	{ {ACTION_USE, OBJECT_IDETOXIN, 0xff, 0}, &Room::demon5UseHypoDytoxinOnAnything },
	{ {ACTION_USE, OBJECT_IBERRY, 9, 0}, &Room::demon5UseBerryOnChub },

	{ {ACTION_LOOK, 8, 0, 0}, &Room::demon5LookAtRoberts },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::demon5LookAtGrisnash },
	{ {ACTION_LOOK, 11, 0, 0}, &Room::demon5LookAtStephen },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::demon5LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::demon5LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::demon5LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::demon5LookAtRedshirt },

	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::demon5LookAtMountain },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::demon5LookAtCrate },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::demon5LookAnywhere },
	{ {ACTION_LOOK, 9, 0, 0}, &Room::demon5LookAtChub },

	{ {ACTION_TALK, 8, 0, 0}, &Room::demon5TalkToRoberts },
	{ {ACTION_TALK, 9, 0, 0}, &Room::demon5TalkToChub },
	{ {ACTION_TALK, 10, 0, 0}, &Room::demon5TalkToGrisnash },
	{ {ACTION_TALK, 11, 0, 0}, &Room::demon5TalkToStephen },

	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::demon5TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::demon5TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::demon5TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::demon5TalkToMccoy },

	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0}, &Room::demon5UseMTricorderOnRoberts },
	{ {ACTION_USE, OBJECT_IMEDKIT,  9, 0}, &Room::demon5UseMTricorderOnChub },
	{ {ACTION_USE, OBJECT_MCCOY,    9, 0}, &Room::demon5UseMTricorderOnChub },
	{ {ACTION_USE, OBJECT_IMTRICOR, 9, 0}, &Room::demon5UseMTricorderOnChub },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0}, &Room::demon5UseMTricorderOnGrisnash },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0}, &Room::demon5UseMTricorderOnStephen },

	{ {ACTION_GET, 0x21, 0, 0}, &Room::demon5GetCrate },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::demon5Tick1() {
	playVoc("DEM5LOOP");

	if (!_awayMission->demon.curedChub || _awayMission->demon.field37 != 1)
		loadActorAnim(11, "oldman", 0x7a, 0xb0, 0);
	loadActorAnim(8, "brorob", 0xc9, 0xa8, 0);
	loadActorAnim(9, "crip", 0xe3, 0xa8, 0);
	loadActorAnim(10, "brotel", 0xf9, 0xbc, 0);
	loadActorAnim(12, "s0r6d2", 0xa0, 0x92, 0);
}

void Room::demon5WalkToDoor() {
	_awayMission->disableInput = true;
	_roomVar.demon.movingToDoor = true;
	walkCrewman(OBJECT_KIRK, 0xa0, 0x94, 1);
}

void Room::demon5TouchedDoorOpenTrigger() {
	if (!_roomVar.demon.movingToDoor)
		return;
	loadActorAnim(12, "s0r6d1", 0xa0, 0x92, 1);
	playSoundEffectIndex(0x05);
}

void Room::demon5DoorOpenedOrReachedDoor() {
	// This is invoked when the door opens and when Kirk reaches the door.
	// Must wait for both to occur.
	_roomVar.demon.doorCounter++;
	if (_roomVar.demon.doorCounter == 2)
		loadRoomIndex(0, 1);
}

void Room::demon5UseSTricorderOnCrate() {
	loadActorAnim(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 6, true);
}

// FIXME: should work when used on people as well, but doesn't
void Room::demon5UsePhaserOnAnything() {
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::demon5UseHandOnStephen() {
	if (_awayMission->demon.repairedHand)
		showText(TX_SPEAKER_STEPHEN, 36, true);
	else
		showText(TX_SPEAKER_STEPHEN, 39, true);
}

void Room::demon5UseBerryOnStephen() {
	if (_awayMission->demon.knowAboutHypoDytoxin)
		showText(TX_SPEAKER_STEPHEN, 33, true);
	else
		showText(TX_SPEAKER_STEPHEN, 37, true);
}

void Room::demon5UseHypoDytoxinOnChub() {
	walkCrewman(OBJECT_MCCOY, 0xe1, 0xb0, 2);
}

void Room::demon5MccoyReachedChub() {
	loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 2);
}

void Room::demon5MccoyHealedChub() {
	walkCrewman(OBJECT_MCCOY, 0xc8, 0xba, 0);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	showText(TX_SPEAKER_CHUB, 27, true);
	loseItem(OBJECT_IDETOXIN);
	_awayMission->demon.curedChub = true;
	_awayMission->demon.missionScore += 2;
}

void Room::demon5UseHypoDytoxinOnAnything() {
	showText(TX_SPEAKER_MCCOY, 8, true);
}

void Room::demon5UseBerryOnChub() {
	showText(TX_SPEAKER_MCCOY, 9, true);
}

void Room::demon5LookAtRoberts() {
	showDescription(1, true);
}

void Room::demon5LookAtGrisnash() {
	showDescription(9, true);
}

void Room::demon5LookAtStephen() {
	showDescription(3, true);
}

void Room::demon5LookAtKirk() {
	showDescription(6, true);
}

void Room::demon5LookAtSpock() {
	showDescription(8, true);
}

void Room::demon5LookAtMccoy() {
	showDescription(7, true);
}

void Room::demon5LookAtRedshirt() {
	showDescription(5, true);
}

void Room::demon5LookAtMountain() {
	showDescription(2, true);
}

void Room::demon5LookAtCrate() {
	showDescription(4, true);
}

void Room::demon5LookAnywhere() {
	showDescription(0, true);
}

void Room::demon5LookAtChub() {
	showText(TX_SPEAKER_CHUB, 28, true, true);
}

void Room::demon5TalkToRoberts() {
	if (_awayMission->demon.curedChub) {
		showText(TX_SPEAKER_ROBERTS, 30, true);
		if (!_roomVar.demon.talkedToRoberts) {
			_roomVar.demon.talkedToRoberts = true;
			_roomVar.demon.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	} else {
		showDescription(10, true);
	}
}

void Room::demon5TalkToChub() {
	if (_awayMission->demon.curedChub) {
		showText(TX_SPEAKER_CHUB, 29, true, true);
		showText(TX_SPEAKER_KIRK, 2, true);
		showText(TX_SPEAKER_CHUB, 30, true, true);
		showText(TX_SPEAKER_KIRK, 5, true);
		showText(TX_SPEAKER_CHUB, 31, true, true);

		if (!_roomVar.demon.talkedToChub) {
			_roomVar.demon.talkedToChub = true;
			_roomVar.demon.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	} else {
		showDescription(13, true);
	}
}

void Room::demon5TalkToGrisnash() {
	if (_awayMission->demon.curedChub) {
		showText(TX_SPEAKER_GRISNASH, 28, true);
		showText(TX_SPEAKER_SPOCK, 24, true);
		showText(TX_SPEAKER_GRISNASH, 29, true);
		showText(TX_SPEAKER_SPOCK, 25, true);

		if (!_roomVar.demon.talkedToGrisnash) {
			_roomVar.demon.talkedToGrisnash = true;
			_roomVar.demon.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	} else {
		showDescription(11, true);
	}
}

void Room::demon5TalkToStephen() {
	if (_awayMission->demon.curedChub) {
		showText(TX_SPEAKER_STEPHEN, 41, true);
		showText(TX_SPEAKER_MCCOY, 22, true);
		showText(TX_SPEAKER_STEPHEN, 43, true);
		showText(TX_SPEAKER_ROBERTS, 31, true);
		showText(TX_SPEAKER_STEPHEN, 42, true);

		if (!_roomVar.demon.talkedToStephen) {
			_roomVar.demon.talkedToStephen = true;
			_roomVar.demon.numTalkedTo++;
			demon5CheckCompletedStudy();
		}
	} else if (!_awayMission->demon.knowAboutHypoDytoxin) {
		showText(TX_SPEAKER_ROBERTS, 34, true);
	} else if (_awayMission->demon.madeHypoDytoxin) {
		showText(TX_SPEAKER_ROBERTS, 35, true);
	} else if (_awayMission->demon.gotBerries) {
		showText(TX_SPEAKER_ROBERTS, 32, true);
		_awayMission->demon.field3e = false;
	} else if (_awayMission->demon.field3e) {
		showText(TX_SPEAKER_ROBERTS, 38, true);
	} else {
		showText(TX_SPEAKER_ROBERTS, 34, true);
	}
}

void Room::demon5TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::demon5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 27, true);
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::demon5TalkToRedshirt() {
	showText(TX_SPEAKER_EVERTS, 45, true);
	showText(TX_SPEAKER_MCCOY,  21, true);
	showText(TX_SPEAKER_KIRK,   3, true);
}

void Room::demon5TalkToMccoy() {
	if (_awayMission->demon.curedChub) {
		showText(TX_SPEAKER_MCCOY, 23, true);
		showText(TX_SPEAKER_KIRK,  4, true);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_EVERTS, 44, true);
			showText(TX_SPEAKER_MCCOY, 20, true);
			showText(TX_SPEAKER_SPOCK, 26, true);
			showText(TX_SPEAKER_MCCOY, 19, true);
		}
	} else {
		showText(TX_SPEAKER_MCCOY, 17, true);
	}
}

void Room::demon5UseMTricorderOnRoberts() {
	if (_roomVar.demon.scannedRoberts)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 15, true);

	_roomVar.demon.scannedRoberts = true;
	_roomVar.demon.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5UseMTricorderOnChub() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_awayMission->demon.curedChub) {
		if (_roomVar.demon.scannedChub)
			return;
		showText(TX_SPEAKER_MCCOY, 16, true);
		_roomVar.demon.scannedChub = true;
		_roomVar.demon.numScanned++;
		demon5CheckCompletedStudy();
	} else {
		if (_awayMission->demon.field3e)
			showText(TX_SPEAKER_MCCOY, 10, true);
		else {
			showText(TX_SPEAKER_MCCOY, 12, true);
			showText(TX_SPEAKER_STEPHEN, 40, true);
			_awayMission->demon.knowAboutHypoDytoxin = true;
			_awayMission->demon.field3e = true;
		}
	}
}


void Room::demon5UseMTricorderOnGrisnash() {
	if (_roomVar.demon.scannedGrisnash)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 13, true);

	_roomVar.demon.scannedGrisnash = true;
	_roomVar.demon.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5UseMTricorderOnStephen() {
	if (_roomVar.demon.scannedStephen)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscanw", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 14, true);

	_roomVar.demon.scannedStephen = true;
	_roomVar.demon.numScanned++;
	demon5CheckCompletedStudy();
}

void Room::demon5CheckCompletedStudy() {
	if (_roomVar.demon.numScanned == 4 && _roomVar.demon.numTalkedTo == 4) {
		showText(TX_SPEAKER_MCCOY, 11, true);
		_roomVar.demon.numTalkedTo = 5;
	}
}

void Room::demon5GetCrate() {
	showDescription(12, true);
}

}
