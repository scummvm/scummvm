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

void Room::demon3Tick1() {
	playVoc("DEM3LOOP");
	loadActorAnim(8, "light", 0xab, 0x03, 0);

	_roomVar.demon3.boulder1Shot = true;
	if (!_vm->_awayMission.demon.boulder1Gone) {
		loadActorAnim(9, "bldr1", 0x26, 0, 0);
		_roomVar.demon3.boulder1Shot = false;
	}
	if (!_vm->_awayMission.demon.boulder2Gone) {
		loadActorAnim(10, "bldr2", 0x22, 0, 0);
	}
	if (!_vm->_awayMission.demon.boulder3Gone) {
		loadActorAnim(11, "bldr3", 0x22, 0, 0);
	}
	if (!_vm->_awayMission.demon.boulder4Gone) {
		loadActorAnim(12, "bldr4", 0x22, 0, 0);
	}

	if (_vm->_awayMission.demon.doorOpened) {
		loadActorAnim(14, "door2", 0x82, 0x0c, 0);
	}
	else {
		loadMapFile("demon3a");
	}

	if (!_vm->_awayMission.demon.healedMiner && _vm->_awayMission.demon.boulder4Gone)
		loadActorAnim(13, "miner", 0xda, 0x6c, 0);
	if (_vm->_awayMission.redshirtDead && !_vm->_awayMission.demon.field45)
		loadActorAnim(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);

	if (_vm->_awayMission.demon.field41 == 0)
		_vm->_awayMission.demon.field41++;

	if (!_vm->_awayMission.demon.field56) {
		_vm->_awayMission.demon.field56 = true;
		playMidiMusicTracks(0, -1);
	}
}

// Boulder falling over
void Room::demon3Timer0Expired() {
	_roomVar.demon3.boulder1Shot = true;
	loadActorAnim2(9, "drbldr", 0x26, 0, 0);
	_vm->_awayMission.timers[1] = 13;
	_vm->_awayMission.demon.boulder1Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_vm->_awayMission.transitioningIntoRoom = true;
	playMidiMusicTracks(2, -1);
	playVoc("BOULDERK");
}

// Redshirt pushing Kirk away
void Room::demon3Timer1Expired() {
	_roomVar.demon3.kirkPhaserOut = false;
	loadActorAnim2(OBJECT_REDSHIRT, "redkil", 0x46, 0xa0, 0);
	loadActorAnim2(OBJECT_KIRK, "redkil", 0x46, 0xa0, 1);
}

// Door just opened
void Room::demon3Timer3Expired() {
	if (_vm->_awayMission.demon.field3b) {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N008#The hand's circuitry triggers a connection, and the door opens.",
			""
		};
		showRoomSpecificText(text);
		loadActorAnim(14, "door", 0x82, 0xc, 0);
		loadMapFile("demon3");
		_vm->_awayMission.demon.doorOpened = true;
		_vm->_awayMission.missionScore += 2;
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N007#The fit is perfect, but something seems to be shorting out.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3FinishedAnimation1() {
	loadActorAnim2(OBJECT_KIRK, "kstnds", 0xee, 0xa6, 0);
	loadActorAnim2(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);
	loadActorStandAnim(OBJECT_SPOCK);
	walkCrewman(OBJECT_MCCOY, 0x9c, 0xb2, 5);
	_roomVar.demon3.inFiringPosition = false;
	_vm->_awayMission.redshirtDead = true;
}

// Just destroyed boulder 4 (revealing the miner)
void Room::demon3FinishedAnimation2() {
	loadActorAnim(13, "miner", 0xda, 0x6c, 0);
}

void Room::demon3FinishedWalking5() {
	loadActorAnim2(OBJECT_MCCOY, "mscanw", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_019#He's dead, Jim...",
		""
	};
	showRoomSpecificText(text);

	_vm->_awayMission.transitioningIntoRoom = false;
}

void Room::demon3McCoyInFiringPosition() {
	loadActorAnim2(OBJECT_MCCOY, "mwaitn", 0xd6, 0xb8, 0);
	_roomVar.demon3.mccoyInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3SpockInFiringPosition() {
	loadActorAnim2(OBJECT_SPOCK, "swaitn", 0xae, 0xb4, 0);
	_roomVar.demon3.spockInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3RedShirtInFiringPosition() {
	loadActorAnim2(OBJECT_REDSHIRT, "rwaitn", 0x46, 0xa0, 0);
	_roomVar.demon3.redshirtInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3KirkInFiringPosition() {
	loadActorAnim2(OBJECT_KIRK, "kwaitn", 0x79, 0xa0, 0);
	_roomVar.demon3.kirkInPosition = true;
	demon3CrewmanInFiringPosition();
}

void Room::demon3CrewmanInFiringPosition() {
	if (_roomVar.demon3.kirkInPosition && _roomVar.demon3.spockInPosition && _roomVar.demon3.mccoyInPosition && (_vm->_awayMission.redshirtDead || _roomVar.demon3.redshirtInPosition))
		demon3PullOutPhaserAndFireAtBoulder();
}

void Room::demon3PullOutPhaserAndFireAtBoulder() {
	if (_roomVar.demon3.kirkPhaserOut)
		demon3FireAtBoulder();
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
		_roomVar.demon3.kirkPhaserOut = true;
	}
}

void Room::demon3FireAtBoulder() {
	_roomVar.demon3.kirkInPosition = false;
	_roomVar.demon3.spockInPosition = false;
	_roomVar.demon3.mccoyInPosition = false;
	_roomVar.demon3.redshirtInPosition = false;
	_roomVar.demon3.shootingBoulder = false;

	switch (_roomVar.demon3.boulderBeingShot) {
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

	loadActorAnim(17, _roomVar.demon3.boulderAnim, 0, 0, 0);
	playSoundEffectIndex(0x06);
	if (!_roomVar.demon3.boulder1Shot)
		_vm->_awayMission.timers[0] = 1;
	_vm->_awayMission.transitioningIntoRoom = false;
}

void Room::demon3UsePhaserOnRedshirt() {
	if (!_vm->_awayMission.redshirtDead)
		return;
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_013#He's already dead, Jim. Isn't that good enough for you?",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseStunPhaserOnBoulder() {
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_007#Captain, the stun setting would be very ineffective on these boulders.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UsePhaserOnBoulder1() {
	if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder1Gone)
		return;
	_vm->_awayMission.demon.boulder1Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_roomVar.demon3.boulderBeingShot = 1;
	_roomVar.demon3.boulder1Shot = true;
	strcpy(_roomVar.demon3.boulderAnim, "s0r3s2");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder2() {
	if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder2Gone)
		return;
	_vm->_awayMission.demon.boulder2Gone = true;
	_vm->_awayMission.demon.numBouldersGone++;
	_roomVar.demon3.boulderBeingShot = 2;
	strcpy(_roomVar.demon3.boulderAnim, "s0r3s3");
	demon3BoulderCommon();
}

void Room::demon3UsePhaserOnBoulder3() {
	if (_vm->_awayMission.demon.boulder1Gone) {
		if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder3Gone)
			return;
		_vm->_awayMission.demon.boulder3Gone = true;
		_vm->_awayMission.demon.numBouldersGone++;
		_roomVar.demon3.boulderBeingShot = 3;
		strcpy(_roomVar.demon3.boulderAnim, "s0r3s1");
		demon3BoulderCommon();
	}
	else {
		const char *text[] = {
			SPEAKER_SPOCK,
			"#DEM3\\DEM3_006#Captain, the structure is extremely unstable. I would not recommend disturbing the lower section before the upper sections have been cleared.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3UsePhaserOnBoulder4() {
	if (_vm->_awayMission.demon.boulder2Gone) {
		if (_roomVar.demon3.shootingBoulder || _vm->_awayMission.demon.boulder4Gone)
			return;
		_vm->_awayMission.demon.boulder4Gone = true;
		_vm->_awayMission.demon.numBouldersGone++;
		_roomVar.demon3.boulderBeingShot = 4;
		strcpy(_roomVar.demon3.boulderAnim, "s0r3s4");
		_vm->_awayMission.demon.foundMiner = true;
		demon3BoulderCommon();
	}
	else {
		// BUGFIX: In the original, the audio didn't play, despite the file existing (and
		// despite it being used for the boulder on the left).
		const char *text[] = {
			SPEAKER_SPOCK,
			"#DEM3\\DEM3_006#Captain, the structure is extremely unstable. I would not recommend disturbing the lower section before the upper sections have been cleared.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3BoulderCommon() {
	_vm->_awayMission.transitioningIntoRoom = true;
	Common::Point pos = getActorPos(OBJECT_KIRK);
	if (!(pos.x == 0x79 && pos.y == 0xa0)) {
		_roomVar.demon3.inFiringPosition = false;
		_roomVar.demon3.kirkPhaserOut = false;
	}

	if (_roomVar.demon3.inFiringPosition) {
		demon3PullOutPhaserAndFireAtBoulder();
	}
	else {
		const char *text[] = {
			SPEAKER_KIRK,
			"#DEM3\\DEM3_001#Assume firing positions.",
			""
		};
		showRoomSpecificText(text);

		walkCrewman(OBJECT_KIRK, 0x79, 0xa0, 1);
		walkCrewman(OBJECT_SPOCK, 0xae, 0xb4, 2);
		walkCrewman(OBJECT_MCCOY, 0xd6, 0xb8, 3);
		if (!_vm->_awayMission.redshirtDead)
			walkCrewman(OBJECT_REDSHIRT, 0x46, 0xa0, 4);

		_roomVar.demon3.shootingBoulder = true;
		_roomVar.demon3.inFiringPosition = true;
	}
}

void Room::demon3UseSTricorderOnMiner() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_008#I think Doctor McCoy would be better suited to deal with this.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseSTricorderOnPanel() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_009#It appears to be a security lock designed to open the door when the correct hand print is registered.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseSTricorderOnBoulder() {
	if (_vm->_awayMission.demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_010#Captain, there are several weak points in the cave-in's structure. Careful use of our phasers, from the top down, should be able to clear it.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseMTricorderOnBoulder() {
	if (_vm->_awayMission.demon.foundMiner)
		return;
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_020#I'm picking up weak vital signs. If we don't dig him out soon, we're going to lose him!",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseCrewmanOnPanel() {
	if (_vm->_awayMission.demon.numBouldersGone != 4 || _vm->_awayMission.redshirtDead)
		return;
	const char *text[] = {
		SPEAKER_EVERTS,
		"#DEM3\\DEM3_031#Sir, it may be dangerous. Let me try it.",
		""
	};
	showRoomSpecificText(text);
	demon3UseRedshirtOnPanel();
}

void Room::demon3UseRedshirtOnPanel() {
	if (_vm->_awayMission.redshirtDead)
		return;
	if (_vm->_awayMission.demon.numBouldersGone != 4) {
		const char *text[] = {
			SPEAKER_EVERTS,
			"#DEM3\\DEM3_030#Sir, I think we should clear the rest of the rocks before we check out the device.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		walkCrewman(OBJECT_REDSHIRT, 0xd8, 0x70, 8);
	}
}

void Room::demon3RedshirtReachedPanel() {
	loadActorAnim2(OBJECT_REDSHIRT, "rusehn", -1, -1, 5);
}

void Room::demon3RedshirtUsedPanel() {
	if (!_vm->_awayMission.demon.foundMiner)
		return;

	_vm->_awayMission.demon.field41++;
	if (_vm->_awayMission.demon.field41 == 5) {
		playVoc("EFX3");
		const char *text[] = {
			SPEAKER_EVERTS,
			"#DEM3\\DEM3_A32#Aieeee!",
			""
		};
		showRoomSpecificText(text);

		loadActorAnim2(OBJECT_REDSHIRT, "rkille", -1, -1, 3);
		playSoundEffectIndex(0x06);
		_vm->_awayMission.redshirtDead = true;
		_vm->_awayMission.demon.field45 = true;
	}
	else {
		const char *textTable[] = {
			"#DEM3\\DEM3N005#Nothing happens.",
			"#DEM3\\DEM3_033#I think I was shocked, sir.",
			"#DEM3\\DEM3_035#That was definitely a mild shock.",
			"#DEM3\\DEM3_034#Ouch, that hurt.",
		};

		const char *text[] = {
			"SPEAKER_EVERTS",
			nullptr,
			""
		};
		text[1] = textTable[_vm->_awayMission.demon.field41 - 1];
		showRoomSpecificText(text);
		walkCrewman(OBJECT_REDSHIRT, 0xbe, 0x9b, 0);
	}
}

void Room::demon3RedshirtElectrocuted() {
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_018#He's dead Jim.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseSTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_012#Fascinating, Captain. This door is made of an unknown material. It is clearly built by an alien race we have no knowledge of.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseSTricorderOnAnything() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_027#Fascinating...  I'm registering low-intensity shielding unlike anything we've encountered before.  That kept this door -- and whatever is behind it -- hidden from the ship's sensors and earlier tricorder readings.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UseMTricorderOnDoor() {
	loadActorAnim2(OBJECT_SPOCK, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_016#This is incredible, Jim. I'm picking up faint lifesign readings behind this door!",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3UsePhaserOnDoor() {
	_roomVar.demon3.usedPhaserOnDoor++;
	if (_roomVar.demon3.usedPhaserOnDoor == 1) {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N009#There is no apparent effect.",
			""
		};
		showRoomSpecificText(text);
	}
	else if (_roomVar.demon3.usedPhaserOnDoor == 2) {
		const char *text[] = {
			SPEAKER_SPOCK,
			"#DEM3\\DEM3_011#Captain, this course of action is ineffectual.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3UseHandOnPanel() {
	if (_vm->_awayMission.demon.numBouldersGone == 4) {
		if (_vm->_awayMission.demon.doorOpened)
			return;
		walkCrewman(OBJECT_KIRK, 0xd8, 0x70, 7);
	}
	else {
		const char *text[] = {
			SPEAKER_SPOCK,
			"#DEM3\\DEM3_005#Captain, I would strongly recommend clearing all the debris first.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3KirkReachedHandPanel() {
	loadActorAnim2(OBJECT_KIRK, "kusehn", -1, -1, 6);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	playVoc("MUR4E1");
}

void Room::demon3KirkUsedHandPanel() {
	loadActorAnim(15, "panel", 0xd6, 0x3d, 0);
	_vm->_awayMission.timers[3] = 10;
	playVoc("SE0FORCE");
}

void Room::demon3UseMTricorderOnMiner() {
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.minerDead) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_022#The miner is dead.",
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_023#This man is badly hurt, and suffering from shock and exposure as well.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3UseMedkitOnMiner() {
	if (_vm->_awayMission.demon.healedMiner) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_015#I've done all I can. He just needs rest now.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		walkCrewman(OBJECT_MCCOY, 0xe6, 0x7b, 6);
		_roomVar.demon3.inFiringPosition = false;
		_roomVar.demon3.kirkPhaserOut = false;
	}
}

void Room::demon3McCoyReachedMiner() {
	loadActorAnim2(OBJECT_MCCOY, "museln", -1, -1, 7);
}

void Room::demon3McCoyFinishedHealingMiner() {
	if (_vm->_awayMission.demon.minerDead) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_022#The miner is dead.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_021#It was a near thing, but he'll live.",
			""
		};
		showRoomSpecificText(text);
		_vm->_awayMission.demon.healedMiner = true;
		loadActorAnim2(13, "drmine", 0xda, 0x6c, 0);
		_vm->_awayMission.missionScore += 2;
		walkCrewman(OBJECT_MCCOY, 0x104, 0x96, 0);
	}
}

void Room::demon3GetMiner() {
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_017#Dammit, Jim, I'm a doctor, not a bellhop. This man's too hurt to be moved.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3TalkToKirk() {
	const char *text1[] = {
		SPEAKER_KIRK,
		"#DEM3\\DEM3_002#A gateway to an alien race. The wonders of the galaxy are endless, aren't they, Mr. Spock?",
		""
	};
	const char *text2[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_029#Indeed, Captain.",
		""
	};
	const char *text3[] = {
		SPEAKER_MCCOY,
		"#DEM3\\DEM3_026#They can also be damned cold.",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
}

void Room::demon3TalkToSpock() {
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM3\\DEM3_028#I recommend as thorough an analysis of this area as possible.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3TalkToMccoy() {
	if (_vm->_awayMission.redshirtDead) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_014#I'm sorry, Jim. Ensign Everts is beyond my help.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text1[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_025#Jim, the next time you need medical help on a snowball...",
			""
		};
		const char *text2[] = {
			SPEAKER_KIRK,
			"#DEM3\\DEM3_003#Bones..",
			""
		};
		const char *text3[] = {
			SPEAKER_MCCOY,
			"#DEM3\\DEM3_024#I'll probably end up coming along.",
			""
		};
		showRoomSpecificText(text1);
		showRoomSpecificText(text2);
		showRoomSpecificText(text3);

	}
}

void Room::demon3TalkToRedshirt() {
	// FIXME: this shouldn't work if he's dead. Should it check higher up whether that's
	// the case?
	const char *text1[] = {
		SPEAKER_EVERTS,
		"#DEM3\\DEM3_036#No sign of demons, Klingons, or other hostiles, Captain. I promise I'll let you know the instant something appears.",
		""
	};
	const char *text2[] = {
		SPEAKER_KIRK,
		"#DEM3\\DEM3_004#Don't be too anxious, Ensign. We may want to talk with them.",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
}

void Room::demon3LookAtKirk() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N004#James T. Kirk. Captain of the Enterprise.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtSpock() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N001#Commander Spock; curious about what lies ahead.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtMccoy() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N002#Doctor Leonard McCoy, glad that the cave provides some shelter from the breeze.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtRedshirt() {
	if (_vm->_awayMission.redshirtDead) {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N017#Ensign Everts lies dead, crushed by the boulder.",
			""
		};
		showRoomSpecificText(text);
		// NOTE: there's an alternate string that isn't used?
		// "#DEM3\\DEM3N018#The crushed body of Ensign Everts reminds you of your poor command judgement.",
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N003#Ensign Everts, wary of more ambushes.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3LookAnywhere() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N010#A cavern, deep in Mount Idyll.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtMiner() {
	if (_vm->_awayMission.demon.healedMiner) {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N000#Brother Kandrey is barely conscious, and is lying still, trying to regain his strength.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N006#One of the Acolytes, who was trapped by the the rockfall.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3LookAtBoulder1() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N011#A huge boulder sits upon a large pile of rubble.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtBoulder2() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N013#A large pile of rubble blocks what appears to be a large metallic structure.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtBoulder3() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N013#A large pile of rubble blocks what appears to be a large metallic structure.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtBoulder4() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N020#You notice what appears to be a man's arm sticking out from beneath the rubble.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtStructure() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N016#A structure built into the surrounding rock. It must have been buried here for millennia.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtDoor() {
	if (_vm->_awayMission.demon.doorOpened) {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N014#A long tunnel descends into the mountain.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM3\\DEM3N012#A large metallic door is set in the structure.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3LookAtPanel() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N019#This looks like some of the hand security panels on the Enterprise.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon3LookAtLight() {
	const char *text[] = {
		nullptr,
		"#DEM3\\DEM3N015#A pulsing red light is set in the structure above the door.",
		""
	};
	showRoomSpecificText(text);
}

}
