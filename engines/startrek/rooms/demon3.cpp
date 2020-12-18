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

extern const RoomAction demon3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::demon3Tick1 },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::demon3Timer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::demon3Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0}, &Room::demon3Timer3Expired },

	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0}, &Room::demon3FinishedAnimation1 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0}, &Room::demon3FinishedAnimation2 },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0}, &Room::demon3FinishedWalking5 },

	{ {ACTION_FINISHED_WALKING, 3, 0, 0}, &Room::demon3McCoyInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0}, &Room::demon3SpockInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0}, &Room::demon3RedShirtInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0}, &Room::demon3KirkInFiringPosition },

	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::demon3FireAtBoulder },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_REDSHIRT, 0}, &Room::demon3UsePhaserOnRedshirt },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_REDSHIRT, 0}, &Room::demon3UsePhaserOnRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERS,  9, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 10, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 11, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 12, 0}, &Room::demon3UseStunPhaserOnBoulder },

	{ {ACTION_USE, OBJECT_IPHASERK,  9, 0}, &Room::demon3UsePhaserOnBoulder1 },
	{ {ACTION_USE, OBJECT_IPHASERK, 10, 0}, &Room::demon3UsePhaserOnBoulder2 },
	{ {ACTION_USE, OBJECT_IPHASERK, 11, 0}, &Room::demon3UsePhaserOnBoulder3 },
	{ {ACTION_USE, OBJECT_IPHASERK, 12, 0}, &Room::demon3UsePhaserOnBoulder4 },

	{ {ACTION_USE, OBJECT_ISTRICOR, 13, 0}, &Room::demon3UseSTricorderOnMiner },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0}, &Room::demon3UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR,  9, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 10, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 11, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 12, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR,  9, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 12, 0}, &Room::demon3UseMTricorderOnBoulder },

	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0}, &Room::demon3UseRedshirtOnPanel },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0}, &Room::demon3RedshirtReachedPanel },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0}, &Room::demon3RedshirtUsedPanel },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::demon3RedshirtElectrocuted },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0}, &Room::demon3UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0}, &Room::demon3UseSTricorderOnAnything },
	{ {ACTION_USE, OBJECT_IMTRICOR, 14, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x22, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, 14, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 14, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x21, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x21, 0}, &Room::demon3UsePhaserOnDoor },

	{ {ACTION_USE, OBJECT_IHAND, 0x20, 0}, &Room::demon3UseHandOnPanel },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0}, &Room::demon3KirkReachedHandPanel },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0}, &Room::demon3KirkUsedHandPanel },

	{ {ACTION_USE, OBJECT_IMTRICOR, 13, 0}, &Room::demon3UseMTricorderOnMiner },
	{ {ACTION_USE, OBJECT_MCCOY, 13, 0}, &Room::demon3UseMedkitOnMiner },
	{ {ACTION_USE, OBJECT_IMEDKIT, 13, 0}, &Room::demon3UseMedkitOnMiner },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0}, &Room::demon3McCoyReachedMiner },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0}, &Room::demon3McCoyFinishedHealingMiner },

	{ {ACTION_GET, 13, 0, 0}, &Room::demon3GetMiner },
	{ {ACTION_TALK, 13, 0, 0}, &Room::demon3TalkToMiner },

	{ {ACTION_TALK, OBJECT_KIRK, 0, 0}, &Room::demon3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::demon3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::demon3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::demon3TalkToRedshirt },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0}, &Room::demon3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::demon3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::demon3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::demon3LookAtRedshirt },

	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::demon3LookAnywhere },
	{ {ACTION_LOOK, 13, 0, 0}, &Room::demon3LookAtMiner },
	{ {ACTION_LOOK,  9, 0, 0}, &Room::demon3LookAtBoulder1 },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::demon3LookAtBoulder2 },
	{ {ACTION_LOOK, 11, 0, 0}, &Room::demon3LookAtBoulder3 },
	{ {ACTION_LOOK, 12, 0, 0}, &Room::demon3LookAtBoulder4 },
	{ {ACTION_LOOK, 0x22, 0, 0}, &Room::demon3LookAtStructure },
	{ {ACTION_LOOK, 14, 0, 0}, &Room::demon3LookAtDoor },
	{ {ACTION_LOOK, 0x21, 0, 0}, &Room::demon3LookAtDoor },
	{ {ACTION_LOOK, 15, 0, 0}, &Room::demon3LookAtPanel },
	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::demon3LookAtPanel },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::demon3LookAtLight },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }

};

void Room::demon3Tick1() {
	playVoc("DEM3LOOP");
	loadActorAnim(8, "light", 0xab, 0x03, 0);

	_roomVar.demon.boulder1Shot = true;
	if (!_awayMission->demon.boulder1Gone) {
		loadActorAnim(9, "bldr1", 0x26, 0, 0);
		_roomVar.demon.boulder1Shot = false;
	}
	if (!_awayMission->demon.boulder2Gone) {
		loadActorAnim(10, "bldr2", 0x22, 0, 0);
	}
	if (!_awayMission->demon.boulder3Gone) {
		loadActorAnim(11, "bldr3", 0x22, 0, 0);
	}
	if (!_awayMission->demon.boulder4Gone) {
		loadActorAnim(12, "bldr4", 0x22, 0, 0);
	}

	if (_awayMission->demon.doorOpened) {
		loadActorAnim(14, "door2", 0x82, 0x0c, 0);
	} else {
		loadMapFile("demon3a");
	}

	if (!_awayMission->demon.healedMiner && _awayMission->demon.boulder4Gone)
		loadActorAnim(13, "miner", 0xda, 0x6c, 0);
	if (_awayMission->redshirtDead && !_awayMission->demon.field45)
		loadActorAnim(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);

	if (_awayMission->demon.field41 == 0)
		_awayMission->demon.field41++;

	if (!_awayMission->demon.field56) {
		_awayMission->demon.field56 = true;
		playMidiMusicTracks(0, -1);
	}
}

// Boulder falling over
void Room::demon3Timer0Expired() {
	_roomVar.demon.boulder1Shot = true;
	loadActorAnim2(9, "drbldr", 0x26, 0, 0);
	_awayMission->timers[1] = 13;
	_awayMission->demon.boulder1Gone = true;
	_awayMission->demon.numBouldersGone++;
	_awayMission->disableInput = true;
	playMidiMusicTracks(2, -1);
	playVoc("BOULDERK");
}

// Redshirt pushing Kirk away
void Room::demon3Timer1Expired() {
	_roomVar.demon.kirkPhaserOut = false;
	loadActorAnim2(OBJECT_REDSHIRT, "redkil", 0x46, 0xa0, 0);
	loadActorAnim2(OBJECT_KIRK, "redkil", 0x46, 0xa0, 1);
}

// Door just opened
void Room::demon3Timer3Expired() {
	if (_awayMission->demon.repairedHand) {
		showDescription(8, true);
		loadActorAnim(14, "door", 0x82, 0xc, 0);
		loadMapFile("demon3");
		_awayMission->demon.doorOpened = true;
		_awayMission->demon.missionScore += 2;
	} else {
		showDescription(7, true);
	}
}

void Room::demon3FinishedAnimation1() {
	loadActorAnim2(OBJECT_KIRK, "kstnds", 0xee, 0xa6, 0);
	loadActorAnim2(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);
	loadActorStandAnim(OBJECT_SPOCK);
	walkCrewman(OBJECT_MCCOY, 0x9c, 0xb2, 5);
	_roomVar.demon.inFiringPosition = false;
	_awayMission->redshirtDead = true;
}

// Just destroyed boulder 4 (revealing the miner)
void Room::demon3FinishedAnimation2() {
	loadActorAnim(13, "miner", 0xda, 0x6c, 0);
}

void Room::demon3FinishedWalking5() {
	loadActorAnim2(OBJECT_MCCOY, "mscanw", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 19, true);

	_awayMission->disableInput = false;
}

void Room::demon3McCoyInFiringPosition() {
	loadActorAnim2(OBJECT_MCCOY, "mwaitn", 0xd6, 0xb8, 0);
	_roomVar.demon.mccoyInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3SpockInFiringPosition() {
	loadActorAnim2(OBJECT_SPOCK, "swaitn", 0xae, 0xb4, 0);
	_roomVar.demon.spockInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3RedShirtInFiringPosition() {
	loadActorAnim2(OBJECT_REDSHIRT, "rwaitn", 0x46, 0xa0, 0);
	_roomVar.demon.redshirtInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3KirkInFiringPosition() {
	loadActorAnim2(OBJECT_KIRK, "kwaitn", 0x79, 0xa0, 0);
	_roomVar.demon.kirkInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3CrewmanInFiringPosition() {
	if (_roomVar.demon.kirkInPosition && _roomVar.demon.spockInPosition && _roomVar.demon.mccoyInPosition && (_awayMission->redshirtDead || _roomVar.demon.redshirtInPosition))
		demon3PullOutPhaserAndFireAtBoulder();
}

void Room::demon3PullOutPhaserAndFireAtBoulder() {
	if (_roomVar.demon.kirkPhaserOut)
		demon3FireAtBoulder();
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
		_roomVar.demon.kirkPhaserOut = true;
	}
}

void Room::demon3FireAtBoulder() {
	_roomVar.demon.kirkInPosition = false;
	_roomVar.demon.spockInPosition = false;
	_roomVar.demon.mccoyInPosition = false;
	_roomVar.demon.redshirtInPosition = false;
	_roomVar.demon.shootingBoulder = false;

	switch (_roomVar.demon.boulderBeingShot) {
	case 1: // FIXME: animation overlaps on Kirk, probably due to sprite sorting inaccuracy
		loadActorAnim2(9, "sbldr1", 0x22, 0, 0);
		break;
	case 2:
		loadActorAnim2(10, "sbldr2", 0x22, 0, 0);
		break;
	case 3:
		loadActorAnim2(11, "sbldr3", 0x22, 0, 0);
		break;
	case 4:
		loadActorAnim2(12, "sbldr4", 0x22, 0, 2);
		break;
	default: {
		// This should never happen (in the original game, this would cause a stack
		// misalignment?
		const char *text[] = {
			"wes",
			"There is a boulder check error.",
			""
		};
		showRoomSpecificText(text);
		break;
	}
	}

	loadActorAnim(17, _roomVar.demon.boulderAnim, 0, 0, 0);
	playSoundEffectIndex(0x06);
	if (!_roomVar.demon.boulder1Shot)
		_awayMission->timers[0] = 1;
	_awayMission->disableInput = false;
}

void Room::demon3UsePhaserOnRedshirt() {
	if (!_awayMission->redshirtDead)
		return;
	showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::demon3UseStunPhaserOnBoulder() {
	showText(TX_SPEAKER_SPOCK, 7, true);
}

void Room::demon3UsePhaserOnBoulder1() {
	if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder1Gone)
		return;
	_awayMission->demon.boulder1Gone = true;
	_awayMission->demon.numBouldersGone++;
	_roomVar.demon.boulderBeingShot = 1;
	_roomVar.demon.boulder1Shot = true;
	strcpy(_roomVar.demon.boulderAnim, "s0r3s2");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder2() {
	if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder2Gone)
		return;
	_awayMission->demon.boulder2Gone = true;
	_awayMission->demon.numBouldersGone++;
	_roomVar.demon.boulderBeingShot = 2;
	strcpy(_roomVar.demon.boulderAnim, "s0r3s3");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder3() {
	if (_awayMission->demon.boulder1Gone) {
		if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder3Gone)
			return;
		_awayMission->demon.boulder3Gone = true;
		_awayMission->demon.numBouldersGone++;
		_roomVar.demon.boulderBeingShot = 3;
		strcpy(_roomVar.demon.boulderAnim, "s0r3s1");
		demon3BoulderCommon();
	} else {
		showText(TX_SPEAKER_SPOCK, 6, true);
	}
}

void Room::demon3UsePhaserOnBoulder4() {
	if (_awayMission->demon.boulder2Gone) {
		if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder4Gone)
			return;
		_awayMission->demon.boulder4Gone = true;
		_awayMission->demon.numBouldersGone++;
		_roomVar.demon.boulderBeingShot = 4;
		strcpy(_roomVar.demon.boulderAnim, "s0r3s4");
		_awayMission->demon.foundMiner = true;
		demon3BoulderCommon();
	} else {
		// BUGFIX: In the original, the audio didn't play, despite the file existing (and
		// despite it being used for the boulder on the left).
		showText(TX_SPEAKER_SPOCK, 6, true);
	}
}

void Room::demon3BoulderCommon() {
	_awayMission->disableInput = true;
	Common::Point pos = getActorPos(OBJECT_KIRK);
	if (!(pos.x == 0x79 && pos.y == 0xa0)) {
		_roomVar.demon.inFiringPosition = false;
		_roomVar.demon.kirkPhaserOut = false;
	}

	if (_roomVar.demon.inFiringPosition) {
		demon3PullOutPhaserAndFireAtBoulder();
	} else {
		showText(TX_SPEAKER_KIRK, 1, true);

		walkCrewman(OBJECT_KIRK, 0x79, 0xa0, 1);
		walkCrewman(OBJECT_SPOCK, 0xae, 0xb4, 2);
		walkCrewman(OBJECT_MCCOY, 0xd6, 0xb8, 3);
		if (!_awayMission->redshirtDead)
			walkCrewman(OBJECT_REDSHIRT, 0x46, 0xa0, 4);

		_roomVar.demon.shootingBoulder = true;
		_roomVar.demon.inFiringPosition = true;
	}
}

void Room::demon3UseSTricorderOnMiner() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 8, true);
}

void Room::demon3UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 9, true);
}

void Room::demon3UseSTricorderOnBoulder() {
	if (_awayMission->demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 10, true);
}

void Room::demon3UseMTricorderOnBoulder() {
	if (_awayMission->demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 20, true);
}

void Room::demon3UseCrewmanOnPanel() {
	if (_awayMission->demon.numBouldersGone != 4 || _awayMission->redshirtDead)
		return;
	showText(TX_SPEAKER_EVERTS, 31, true);
	demon3UseRedshirtOnPanel();
}

void Room::demon3UseRedshirtOnPanel() {
	if (_awayMission->redshirtDead)
		return;

	if (_awayMission->demon.numBouldersGone != 4)
		showText(TX_SPEAKER_EVERTS, 30, true);
	else
		walkCrewman(OBJECT_REDSHIRT, 0xd8, 0x70, 8);
}

void Room::demon3RedshirtReachedPanel() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusehn", -1, -1, 5);
}

void Room::demon3RedshirtUsedPanel() {
	if (!_awayMission->demon.foundMiner)
		return;

	_awayMission->demon.field41++;
	if (_awayMission->demon.field41 == 5) {
		playVoc("EFX3");
		showText(TX_SPEAKER_EVERTS, TX_DEM3_A32);

		loadActorAnim2(OBJECT_REDSHIRT, "rkille", -1, -1, 3);
		playSoundEffectIndex(0x06);
		_awayMission->redshirtDead = true;
		_awayMission->demon.field45 = true;
	} else {
		const TextRef textTable[] = {
			5,
			33,
			35,
			34,
		};

		TextRef text[] = {
			TX_SPEAKER_EVERTS,
			TX_NULL,
			TX_BLANK
		};
		text[1] = textTable[_awayMission->demon.field41 - 1];
		showMultipleTexts(text, true);
		walkCrewman(OBJECT_REDSHIRT, 0xbe, 0x9b, 0);
	}
}

void Room::demon3RedshirtElectrocuted() {
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::demon3UseSTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 12, true);
}

void Room::demon3UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_SPOCK, 27, true);
}

void Room::demon3UseMTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::demon3UsePhaserOnDoor() {
	_roomVar.demon.usedPhaserOnDoor++;

	if (_roomVar.demon.usedPhaserOnDoor == 1)
		showDescription(9, true);
	else if (_roomVar.demon.usedPhaserOnDoor == 2)
		showText(TX_SPEAKER_SPOCK, 11, true);
}

void Room::demon3UseHandOnPanel() {
	if (_awayMission->demon.numBouldersGone == 4) {
		if (_awayMission->demon.doorOpened)
			return;
		walkCrewman(OBJECT_KIRK, 0xd8, 0x70, 7);
	} else {
		showText(TX_SPEAKER_SPOCK, 5, true);
	}
}

void Room::demon3KirkReachedHandPanel() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 6);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	playVoc("MUR4E1");
}

void Room::demon3KirkUsedHandPanel() {
	loadActorAnim(15, "panel", 0xd6, 0x3d, 0);
	_awayMission->timers[3] = 10;
	playVoc("SE0FORCE");
}

void Room::demon3UseMTricorderOnMiner() {
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_awayMission->demon.minerDead) {
		showText(TX_SPEAKER_MCCOY, 22, true);
	} else {
		showText(TX_SPEAKER_MCCOY, 23, true);
	}
}

void Room::demon3UseMedkitOnMiner() {
	if (_awayMission->demon.healedMiner) {
		showText(TX_SPEAKER_MCCOY, 15, true);
	} else {
		walkCrewman(OBJECT_MCCOY, 0xe6, 0x7b, 6);
		_roomVar.demon.inFiringPosition = false;
		_roomVar.demon.kirkPhaserOut = false;
	}
}

void Room::demon3McCoyReachedMiner() {
	loadActorAnim2(OBJECT_MCCOY, "museln", -1, -1, 7);
}

void Room::demon3McCoyFinishedHealingMiner() {
	if (_awayMission->demon.minerDead) {
		showText(TX_SPEAKER_MCCOY, 22, true);
	} else {
		showText(TX_SPEAKER_MCCOY, 21, true);
		_awayMission->demon.healedMiner = true;
		loadActorAnim2(13, "drmine", 0xda, 0x6c, 0);
		_awayMission->demon.missionScore += 2;
		walkCrewman(OBJECT_MCCOY, 0x104, 0x96, 0);
	}
}

void Room::demon3GetMiner() {
	showText(TX_SPEAKER_MCCOY, 17, true);
}

void Room::demon3TalkToMiner() {
	if (!_awayMission->demon.healedMiner)
		return;
	showText(TX_SPEAKER_KANDREY, 21 + FOLLOWUP_MESSAGE_OFFSET, true);
}

void Room::demon3TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  2, true);
	showText(TX_SPEAKER_SPOCK, 29, true);
	showText(TX_SPEAKER_MCCOY, 26, true);
}

void Room::demon3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 28, true);
}

void Room::demon3TalkToMccoy() {
	if (_awayMission->redshirtDead) {
		showText(TX_SPEAKER_MCCOY, 14, true);
	} else {
		showText(TX_SPEAKER_MCCOY, 25, true);
		showText(TX_SPEAKER_KIRK,  3, true);
		showText(TX_SPEAKER_MCCOY, 24, true);

	}
}

void Room::demon3TalkToRedshirt() {
	// FIXME: this shouldn't work if he's dead. Should it check higher up whether that's
	// the case?
	showText(TX_SPEAKER_EVERTS, 36, true);
	showText(TX_SPEAKER_KIRK,   4, true);
}

void Room::demon3LookAtKirk() {
	showDescription(4, true);
}

void Room::demon3LookAtSpock() {
	showDescription(1, true);
}

void Room::demon3LookAtMccoy() {
	showDescription(2, true);
}

void Room::demon3LookAtRedshirt() {
	if (_awayMission->redshirtDead) {
		showDescription(17, true);
		// NOTE: there's an alternate string that isn't used? (TX_DEM3N018)
	} else {
		showDescription(3, true);
	}
}

void Room::demon3LookAnywhere() {
	showDescription(10, true);
}

void Room::demon3LookAtMiner() {
	if (_awayMission->demon.healedMiner) {
		showDescription(0, true);
	} else {
		showDescription(6, true);
	}
}

void Room::demon3LookAtBoulder1() {
	showDescription(11, true);
}

void Room::demon3LookAtBoulder2() {
	showDescription(13, true);
}

void Room::demon3LookAtBoulder3() {
	showDescription(13, true);
}

void Room::demon3LookAtBoulder4() {
	showDescription(20, true);
}

void Room::demon3LookAtStructure() {
	showDescription(16, true);
}

void Room::demon3LookAtDoor() {
	if (_awayMission->demon.doorOpened) {
		showDescription(14, true);
	} else {
		showDescription(12, true);
	}
}

void Room::demon3LookAtPanel() {
	showDescription(19, true);
}

void Room::demon3LookAtLight() {
	showDescription(15, true);
}

}
