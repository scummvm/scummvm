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

namespace StarTrek {

extern const RoomAction demon3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::demon3Tick1 },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::demon3Timer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::demon3Timer1Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0}, &Room::demon3Timer3Expired },

	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::demon3FinishedAnimation1 },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::demon3FinishedAnimation2 },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::demon3FinishedWalking5 },

	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::demon3McCoyInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::demon3SpockInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::demon3RedShirtInFiringPosition },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0}, &Room::demon3KirkInFiringPosition },

	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::demon3FireAtBoulder },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_REDSHIRT, 0, 0}, &Room::demon3UsePhaserOnRedshirt },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_REDSHIRT, 0, 0}, &Room::demon3UsePhaserOnRedshirt },

	{ {ACTION_USE, OBJECT_IPHASERS,  9, 0, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 10, 0, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 11, 0, 0}, &Room::demon3UseStunPhaserOnBoulder },
	{ {ACTION_USE, OBJECT_IPHASERS, 12, 0, 0}, &Room::demon3UseStunPhaserOnBoulder },

	{ {ACTION_USE, OBJECT_IPHASERK,  9, 0, 0}, &Room::demon3UsePhaserOnBoulder1 },
	{ {ACTION_USE, OBJECT_IPHASERK, 10, 0, 0}, &Room::demon3UsePhaserOnBoulder2 },
	{ {ACTION_USE, OBJECT_IPHASERK, 11, 0, 0}, &Room::demon3UsePhaserOnBoulder3 },
	{ {ACTION_USE, OBJECT_IPHASERK, 12, 0, 0}, &Room::demon3UsePhaserOnBoulder4 },

	{ {ACTION_USE, OBJECT_ISTRICOR, 13, 0, 0}, &Room::demon3UseSTricorderOnMiner },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::demon3UseSTricorderOnPanel },
	{ {ACTION_USE, OBJECT_ISTRICOR,  9, 0, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 10, 0, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 11, 0, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_ISTRICOR, 12, 0, 0}, &Room::demon3UseSTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR,  9, 0, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0, 0}, &Room::demon3UseMTricorderOnBoulder },
	{ {ACTION_USE, OBJECT_IMTRICOR, 12, 0, 0}, &Room::demon3UseMTricorderOnBoulder },

	{ {ACTION_USE, OBJECT_KIRK, 0x20, 0, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_SPOCK, 0x20, 0, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_MCCOY, 0x20, 0, 0}, &Room::demon3UseCrewmanOnPanel },
	{ {ACTION_USE, OBJECT_REDSHIRT, 0x20, 0, 0}, &Room::demon3UseRedshirtOnPanel },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::demon3RedshirtReachedPanel },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::demon3RedshirtUsedPanel },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::demon3RedshirtElectrocuted },

	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::demon3UseSTricorderOnDoor },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::demon3UseSTricorderOnAnything },
	{ {ACTION_USE, OBJECT_IMTRICOR, 14, 0, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x22, 0, 0}, &Room::demon3UseMTricorderOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, 14, 0, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 14, 0, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERS, 0x21, 0, 0}, &Room::demon3UsePhaserOnDoor },
	{ {ACTION_USE, OBJECT_IPHASERK, 0x21, 0, 0}, &Room::demon3UsePhaserOnDoor },

	{ {ACTION_USE, OBJECT_IHAND, 0x20, 0, 0}, &Room::demon3UseHandOnPanel },
	{ {ACTION_FINISHED_WALKING, 7, 0, 0, 0}, &Room::demon3KirkReachedHandPanel },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::demon3KirkUsedHandPanel },

	{ {ACTION_USE, OBJECT_IMTRICOR, 13, 0, 0}, &Room::demon3UseMTricorderOnMiner },
	{ {ACTION_USE, OBJECT_MCCOY, 13, 0, 0}, &Room::demon3UseMedkitOnMiner },
	{ {ACTION_USE, OBJECT_IMEDKIT, 13, 0, 0}, &Room::demon3UseMedkitOnMiner },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::demon3McCoyReachedMiner },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0}, &Room::demon3McCoyFinishedHealingMiner },

	{ {ACTION_GET, 13, 0, 0, 0}, &Room::demon3GetMiner },
	{ {ACTION_TALK, 13, 0, 0, 0}, &Room::demon3TalkToMiner },

	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::demon3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon3TalkToRedshirt },

	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::demon3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::demon3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::demon3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::demon3LookAtRedshirt },

	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::demon3LookAnywhere },
	{ {ACTION_LOOK, 13, 0, 0, 0}, &Room::demon3LookAtMiner },
	{ {ACTION_LOOK,  9, 0, 0, 0}, &Room::demon3LookAtBoulder1 },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::demon3LookAtBoulder2 },
	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::demon3LookAtBoulder3 },
	{ {ACTION_LOOK, 12, 0, 0, 0}, &Room::demon3LookAtBoulder4 },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::demon3LookAtStructure },
	{ {ACTION_LOOK, 14, 0, 0, 0}, &Room::demon3LookAtDoor },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::demon3LookAtDoor },
	{ {ACTION_LOOK, 15, 0, 0, 0}, &Room::demon3LookAtPanel },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::demon3LookAtPanel },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::demon3LookAtLight },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum demon3TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_EVERTS,
	TX_SPEAKER_KANDREY,
	TX_DEM3_001, TX_DEM3_002, TX_DEM3_003, TX_DEM3_004, TX_DEM3_005,
	TX_DEM3_006, TX_DEM3_007, TX_DEM3_008, TX_DEM3_009, TX_DEM3_010,
	TX_DEM3_011, TX_DEM3_012, TX_DEM3_013, TX_DEM3_014, TX_DEM3_015,
	TX_DEM3_016, TX_DEM3_017, TX_DEM3_018, TX_DEM3_019, TX_DEM3_020,
	TX_DEM3_021, TX_DEM3_022, TX_DEM3_023, TX_DEM3_024, TX_DEM3_025,
	TX_DEM3_026, TX_DEM3_027, TX_DEM3_028, TX_DEM3_029, TX_DEM3_030,
	TX_DEM3_031, TX_DEM3_033, TX_DEM3_034, TX_DEM3_035, TX_DEM3_036,
	TX_DEM3_A32, TX_DEM3_B32, TX_DEM3_F21, TX_DEM3N000, TX_DEM3N001,
	TX_DEM3N002, TX_DEM3N003, TX_DEM3N004, TX_DEM3N005, TX_DEM3N006,
	TX_DEM3N007, TX_DEM3N008, TX_DEM3N009, TX_DEM3N010, TX_DEM3N011,
	TX_DEM3N012, TX_DEM3N013, TX_DEM3N014, TX_DEM3N015, TX_DEM3N016,
	TX_DEM3N017, TX_DEM3N018, TX_DEM3N019, TX_DEM3N020
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets demon3TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 6011, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 6022, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 6032, 0, 0, 0 },
	{ TX_SPEAKER_EVERTS, 6042, 0, 0, 0 },
	{ TX_SPEAKER_KANDREY, 6056, 0, 0, 0 },
	{ TX_DEM3_001, 7134, 0, 0, 0 },
	{ TX_DEM3_002, 6521, 0, 0, 0 },
	{ TX_DEM3_003, 6854, 0, 0, 0 },
	{ TX_DEM3_004, 7058, 0, 0, 0 },
	{ TX_DEM3_005, 3794, 0, 0, 0 },
	{ TX_DEM3_006, 1957, 0, 0, 0 },
	{ TX_DEM3_007, 1747, 0, 0, 0 },
	{ TX_DEM3_008, 2585, 0, 0, 0 },
	{ TX_DEM3_009, 2714, 0, 0, 0 },
	{ TX_DEM3_010, 7277, 0, 0, 0 },
	{ TX_DEM3_011, 8332, 0, 0, 0 },
	{ TX_DEM3_012, 7918, 0, 0, 0 },
	{ TX_DEM3_013, 1634, 0, 0, 0 },
	{ TX_DEM3_014, 4494, 0, 0, 0 },
	{ TX_DEM3_015, 4174, 0, 0, 0 },
	{ TX_DEM3_016, 3603, 0, 0, 0 },
	{ TX_DEM3_017, 7606, 0, 0, 0 },
	{ TX_DEM3_018, 7876, 0, 0, 0 },
	{ TX_DEM3_019, 8548, 0, 0, 0 },
	{ TX_DEM3_020, 7174, 0, 0, 0 },
	{ TX_DEM3_021, 7520, 0, 0, 0 },
	{ TX_DEM3_022, 7572, 0, 0, 0 },
	{ TX_DEM3_023, 7434, 0, 0, 0 },
	{ TX_DEM3_024, 6877, 0, 0, 0 },
	{ TX_DEM3_025, 6781, 0, 0, 0 },
	{ TX_DEM3_026, 6659, 0, 0, 0 },
	{ TX_DEM3_027, 8059, 0, 0, 0 },
	{ TX_DEM3_028, 6704, 0, 0, 0 },
	{ TX_DEM3_029, 6627, 0, 0, 0 },
	{ TX_DEM3_030, 3120, 0, 0, 0 },
	{ TX_DEM3_031, 3031, 0, 0, 0 },
	{ TX_DEM3_033, 7728, 0, 0, 0 },
	{ TX_DEM3_034, 7820, 0, 0, 0 },
	{ TX_DEM3_035, 7771, 0, 0, 0 },
	{ TX_DEM3_036, 6927, 0, 0, 0 },
	{ TX_DEM3_A32, 3326, 0, 0, 0 },
	{ TX_DEM3_B32, 7852, 0, 0, 0 },
	{ TX_DEM3_F21, 8642, 0, 0, 0 },
	{ TX_DEM3N000, 6418, 0, 0, 0 },
	{ TX_DEM3N001, 6134, 0, 0, 0 },
	{ TX_DEM3N002, 6197, 0, 0, 0 },
	{ TX_DEM3N003, 6292, 0, 0, 0 },
	{ TX_DEM3N004, 6077, 0, 0, 0 },
	{ TX_DEM3N005, 7696, 0, 0, 0 },
	{ TX_DEM3N006, 6345, 0, 0, 0 },
	{ TX_DEM3N007, 8394, 0, 0, 0 },
	{ TX_DEM3N008, 8469, 0, 0, 0 },
	{ TX_DEM3N009, 8288, 0, 0, 0 },
	{ TX_DEM3N010, 4772, 0, 0, 0 },
	{ TX_DEM3N011, 4995, 0, 0, 0 },
	{ TX_DEM3N012, 5547, 0, 0, 0 },
	{ TX_DEM3N013, 5077, 0, 0, 0 },
	{ TX_DEM3N013, 5187, 0, 0, 0 },
	{ TX_DEM3N014, 5621, 0, 0, 0 },
	{ TX_DEM3N015, 5805, 0, 0, 0 },
	{ TX_DEM3N016, 5410, 0, 0, 0 },
	{ TX_DEM3N017, 4680, 0, 0, 0 },
	{ TX_DEM3N018, 4852, 0, 0, 0 },
	{ TX_DEM3N019, 5704, 0, 0, 0 },
	{ TX_DEM3N020, 5297, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText demon3Texts[] = {
	{ -1, Common::UNK_LANG, "" }
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
		playMidiMusicTracks(MIDITRACK_0);
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
	playMidiMusicTracks(MIDITRACK_2);
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
		showDescription(TX_DEM3N008);
		loadActorAnim(14, "door", 0x82, 0xc, 0);
		loadMapFile("demon3");
		_awayMission->demon.doorOpened = true;
		_awayMission->demon.missionScore += 2;
	} else {
		showDescription(TX_DEM3N007);
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
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_019);

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
	playSoundEffectIndex(kSfxPhaser);
	if (!_roomVar.demon.boulder1Shot)
		_awayMission->timers[0] = 1;
	_awayMission->disableInput = false;
}

void Room::demon3UsePhaserOnRedshirt() {
	if (!_awayMission->redshirtDead)
		return;
	showText(TX_SPEAKER_MCCOY, TX_DEM3_013);
}

void Room::demon3UseStunPhaserOnBoulder() {
	showText(TX_SPEAKER_SPOCK, TX_DEM3_007);
}

void Room::demon3UsePhaserOnBoulder1() {
	if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder1Gone)
		return;
	_awayMission->demon.boulder1Gone = true;
	_awayMission->demon.numBouldersGone++;
	_roomVar.demon.boulderBeingShot = 1;
	_roomVar.demon.boulder1Shot = true;
	Common::strcpy_s(_roomVar.demon.boulderAnim, "s0r3s2");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder2() {
	if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder2Gone)
		return;
	_awayMission->demon.boulder2Gone = true;
	_awayMission->demon.numBouldersGone++;
	_roomVar.demon.boulderBeingShot = 2;
	Common::strcpy_s(_roomVar.demon.boulderAnim, "s0r3s3");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder3() {
	if (_awayMission->demon.boulder1Gone) {
		if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder3Gone)
			return;
		_awayMission->demon.boulder3Gone = true;
		_awayMission->demon.numBouldersGone++;
		_roomVar.demon.boulderBeingShot = 3;
		Common::strcpy_s(_roomVar.demon.boulderAnim, "s0r3s1");
		demon3BoulderCommon();
	} else {
		showText(TX_SPEAKER_SPOCK, TX_DEM3_006);
	}
}

void Room::demon3UsePhaserOnBoulder4() {
	if (_awayMission->demon.boulder2Gone) {
		if (_roomVar.demon.shootingBoulder || _awayMission->demon.boulder4Gone)
			return;
		_awayMission->demon.boulder4Gone = true;
		_awayMission->demon.numBouldersGone++;
		_roomVar.demon.boulderBeingShot = 4;
		Common::strcpy_s(_roomVar.demon.boulderAnim, "s0r3s4");
		_awayMission->demon.foundMiner = true;
		demon3BoulderCommon();
	} else {
		// BUGFIX: In the original, the audio didn't play, despite the file existing (and
		// despite it being used for the boulder on the left).
		showText(TX_SPEAKER_SPOCK, TX_DEM3_006);
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
		showText(TX_SPEAKER_KIRK, TX_DEM3_001);

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
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_008);
}

void Room::demon3UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_009);
}

void Room::demon3UseSTricorderOnBoulder() {
	if (_awayMission->demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_010);
}

void Room::demon3UseMTricorderOnBoulder() {
	if (_awayMission->demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_020);
}

void Room::demon3UseCrewmanOnPanel() {
	if (_awayMission->demon.numBouldersGone != 4 || _awayMission->redshirtDead)
		return;
	showText(TX_SPEAKER_EVERTS, TX_DEM3_031);
	demon3UseRedshirtOnPanel();
}

void Room::demon3UseRedshirtOnPanel() {
	if (_awayMission->redshirtDead)
		return;

	if (_awayMission->demon.numBouldersGone != 4)
		showText(TX_SPEAKER_EVERTS, TX_DEM3_030);
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
		playSoundEffectIndex(kSfxPhaser);
		_awayMission->redshirtDead = true;
		_awayMission->demon.field45 = true;
	} else {
		const TextRef textTable[] = {
		    TX_DEM3N005,
		    TX_DEM3_033,
		    TX_DEM3_035,
		    TX_DEM3_034,
		};

		TextRef text[] = {
		    TX_SPEAKER_EVERTS,
		    TX_EMPTY,
		    TX_END
		};
		text[1] = textTable[_awayMission->demon.field41 - 1];
		showMultipleTexts(text);
		walkCrewman(OBJECT_REDSHIRT, 0xbe, 0x9b, 0);
	}
}

void Room::demon3RedshirtElectrocuted() {
	showText(TX_SPEAKER_MCCOY, TX_DEM3_018);
}

void Room::demon3UseSTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_012);
}

void Room::demon3UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_027);
}

void Room::demon3UseMTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "mscann", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_016);
}

void Room::demon3UsePhaserOnDoor() {
	_roomVar.demon.usedPhaserOnDoor++;

	if (_roomVar.demon.usedPhaserOnDoor == 1)
		showDescription(TX_DEM3N009);
	else if (_roomVar.demon.usedPhaserOnDoor == 2)
		showText(TX_SPEAKER_SPOCK, TX_DEM3_011);
}

void Room::demon3UseHandOnPanel() {
	if (_awayMission->demon.numBouldersGone == 4) {
		if (_awayMission->demon.doorOpened)
			return;
		walkCrewman(OBJECT_KIRK, 0xd8, 0x70, 7);
	} else {
		showText(TX_SPEAKER_SPOCK, TX_DEM3_005);
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
	playSoundEffectIndex(kSfxTricorder);

	if (_awayMission->demon.minerDead) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_022);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_023);
	}
}

void Room::demon3UseMedkitOnMiner() {
	if (_awayMission->demon.healedMiner) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_015);
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
		showText(TX_SPEAKER_MCCOY, TX_DEM3_022);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_021);
		_awayMission->demon.healedMiner = true;
		loadActorAnim2(13, "drmine", 0xda, 0x6c, 0);
		_awayMission->demon.missionScore += 2;
		walkCrewman(OBJECT_MCCOY, 0x104, 0x96, 0);
	}
}

void Room::demon3GetMiner() {
	showText(TX_SPEAKER_MCCOY, TX_DEM3_017);
}

void Room::demon3TalkToMiner() {
	if (!_awayMission->demon.healedMiner)
		return;
	showText(TX_SPEAKER_KANDREY, TX_DEM3_F21);
}

void Room::demon3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_DEM3_002);
	showText(TX_SPEAKER_SPOCK, TX_DEM3_029);
	showText(TX_SPEAKER_MCCOY, TX_DEM3_026);
}

void Room::demon3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_DEM3_028);
}

void Room::demon3TalkToMccoy() {
	if (_awayMission->redshirtDead) {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_014);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_DEM3_025);
		showText(TX_SPEAKER_KIRK, TX_DEM3_003);
		showText(TX_SPEAKER_MCCOY, TX_DEM3_024);
	}
}

void Room::demon3TalkToRedshirt() {
	// FIXME: this shouldn't work if he's dead. Should it check higher up whether that's
	// the case?
	showText(TX_SPEAKER_EVERTS, TX_DEM3_036);
	showText(TX_SPEAKER_KIRK, TX_DEM3_004);
}

void Room::demon3LookAtKirk() {
	showDescription(TX_DEM3N004);
}

void Room::demon3LookAtSpock() {
	showDescription(TX_DEM3N001);
}

void Room::demon3LookAtMccoy() {
	showDescription(TX_DEM3N002);
}

void Room::demon3LookAtRedshirt() {
	if (_awayMission->redshirtDead) {
		showDescription(TX_DEM3N017);
		// NOTE: there's an alternate string that isn't used? (TX_DEM3N018)
	} else {
		showDescription(TX_DEM3N003);
	}
}

void Room::demon3LookAnywhere() {
	showDescription(TX_DEM3N010);
}

void Room::demon3LookAtMiner() {
	if (_awayMission->demon.healedMiner) {
		showDescription(TX_DEM3N000);
	} else {
		showDescription(TX_DEM3N006);
	}
}

void Room::demon3LookAtBoulder1() {
	showDescription(TX_DEM3N011);
}

void Room::demon3LookAtBoulder2() {
	showDescription(TX_DEM3N013);
}

void Room::demon3LookAtBoulder3() {
	showDescription(TX_DEM3N013);
}

void Room::demon3LookAtBoulder4() {
	showDescription(TX_DEM3N020);
}

void Room::demon3LookAtStructure() {
	showDescription(TX_DEM3N016);
}

void Room::demon3LookAtDoor() {
	if (_awayMission->demon.doorOpened) {
		showDescription(TX_DEM3N014);
	} else {
		showDescription(TX_DEM3N012);
	}
}

void Room::demon3LookAtPanel() {
	showDescription(TX_DEM3N019);
}

void Room::demon3LookAtLight() {
	showDescription(TX_DEM3N015);
}


}
