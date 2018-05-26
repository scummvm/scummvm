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

	if (_vm->_awayMission.demon.doorRevealed) {
		loadActorAnim(14, "door2", 0x82, 0x0c, 0);
	}
	else {
		loadMapFile("demon3a");
	}

	if (!_vm->_awayMission.demon.field3c && _vm->_awayMission.demon.boulder4Gone)
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

void Room::demon3Timer3Expired() {
	// TODO
}

void Room::demon3FinishedAnimation1() {
	loadActorAnim2(OBJECT_KIRK, "kstnds", 0xee, 0xa6, 0);
	loadActorAnim2(OBJECT_REDSHIRT, "deadre", 0x46, 0xa0, 0);
	loadActorStandAnim(OBJECT_SPOCK);
	walkCrewman(OBJECT_MCCOY, 0x9c, 0xb2, 5);
	_roomVar.demon3.inFiringPosition = false;
	_vm->_awayMission.redshirtDead = true;
}

void Room::demon3FinishedAnimation2() {
	// TODO
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
	case 1:
		loadActorAnim2(9, "sbldr1", 0x22, 0, 0);
		break;
	case 2:
		loadActorAnim2(10, "sbldr2", 0x22, 0, 0);
		break;
	case 3:
		loadActorAnim2(11, "sbldr3", 0x22, 0, 0);
		break;
	case 4:
		loadActorAnim2(12, "sbldr4", 0x22, 0, 0);
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
		// BUG: Spock doesn't speak, even though an audio file exists
		const char *text[] = {
			SPEAKER_SPOCK,
			"Captain, the structure is extremely unstable. I would not recommend disturbing the lower section before the upper sections have been cleared.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon3BoulderCommon() {
	_vm->_awayMission.transitioningIntoRoom = true;
	Common::Point pos = getActorPos(13);
	/* FIXME
	if (!(pos.x == 0x79 && pos.y == 0xa0)) {
		_roomVar.demon3.inFiringPosition = false;
		_roomVar.demon3.kirkPhaserOut = false;
	}
	*/

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

}
