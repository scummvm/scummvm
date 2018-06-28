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

#define OBJECT_MUDD 8
#define OBJECT_CAPSULE 9
#define OBJECT_NORTH_DOOR 10
#define OBJECT_ALIENDV 11

#define HOTSPOT_CAPSULES 0x20
#define HOTSPOT_NORTH_DOOR 0x21
#define HOTSPOT_WEST_DOOR 0x22
#define HOTSPOT_CONTROL_PANEL 0x23
#define HOTSPOT_LEFT_BED 0x24
#define HOTSPOT_RIGHT_BED 0x25

namespace StarTrek {

void Room::mudd2WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_KIRK, 0xbc, 0x6a);
}

void Room::mudd2TouchedHotspot0() { // Trigger north door
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
		loadActorAnim(OBJECT_NORTH_DOOR, "s4sbdo", 0xbe, 0x6b);
	}
}

void Room::mudd2WalkToSouthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_vm->_awayMission.disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x46, 0x6f);
}

void Room::mudd2TouchedHotspot1() {
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
	}
}

void Room::mudd2Tick1() {
	playVoc("MUD2LOOP");
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	_vm->_awayMission.mudd.muddDroppedCapsule = false;

	if (_vm->_awayMission.mudd.muddState == 0) {
		_vm->_awayMission.mudd.muddState = 0;
	} else if (_vm->_awayMission.mudd.muddState == 2) {
		_vm->_awayMission.mudd.muddDroppedCapsule = true;
		if (!_vm->_awayMission.mudd.muddUnconscious) {
			_vm->_awayMission.mudd.muddUnconscious = false;
			loadActorAnim2(OBJECT_MUDD, "s4sbhn", 0x9f, 0xbf);
			loadActorAnim2(OBJECT_CAPSULE, "s4sbvp", 0x93, 0xc3);
		} else {
			loadActorAnim2(OBJECT_MUDD, "s4sbob", 0x9f, 0xba);
		}
	} else if (_vm->_awayMission.mudd.muddCurrentlyInsane) {
		_vm->_awayMission.mudd.muddState = 0;
	} else if (_vm->_awayMission.mudd.muddState == 1) {
		playMidiMusicTracks(3);
		loadActorAnim2(OBJECT_MUDD, "s4sbhw", 0x99, 0xbf);
		_vm->_awayMission.disableInput = 2;
		_vm->_awayMission.mudd.muddInhaledGas = true;
		_vm->_awayMission.timers[1] = 70;
		_vm->_awayMission.mudd.muddState = 2;
		_vm->_awayMission.mudd.muddCurrentlyInsane = true;
	}
}

void Room::mudd2Timer1Expired() {
	loadActorAnimC(OBJECT_MUDD, "s4sbmt", 0xa0, 0xbf, &Room::mudd2MuddNoticedKirk);
}

void Room::mudd2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_MUD2_037, false);
}

void Room::mudd2UseSpockOnCapsules() {
	showText(TX_SPEAKER_MCCOY, TX_MUD2_018);
}

void Room::mudd2GetCapsules() {
	if (!_vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious) {
		if (!_vm->_awayMission.mudd.translatedAlienLanguage)
			showText(TX_SPEAKER_MCCOY, TX_MUD2_024);
		else
			walkCrewmanC(OBJECT_MCCOY, 0x9f, 0xbf, &Room::mudd2MccoyReachedCapsules);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_MUD2_023);
	}
}

void Room::mudd2MccoyReachedCapsules() {
	loadActorAnimC(OBJECT_MCCOY, "musehn", -1, -1, &Room::mudd2MccoyPickedUpCapsules);
	giveItem(OBJECT_ICAPSULE);
}

void Room::mudd2MccoyPickedUpCapsules() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xfe, 0xb2);
}

void Room::mudd2UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD2_009);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd2LookAtCapsules() {
	showText(TX_MUD2N008);
}

void Room::mudd2UseMTricorderOnCapsules() {
	mccoyScan(DIR_W, TX_MUD2_030, false);
	showText(TX_SPEAKER_KIRK,  TX_MUD2_004);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_019);
}

void Room::mudd2UseCapsuleOnControlPanel() {
	if (!_vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious)
		walkCrewmanC(OBJECT_MCCOY, 0x9f, 0xbf, &Room::mudd2MccoyReachedControlPanel);
	else
		showText(TX_SPEAKER_MCCOY, TX_MUD2_023);
}

void Room::mudd2MccoyReachedControlPanel() {
	if (_vm->_awayMission.mudd.translatedAlienLanguage)
		showText(TX_SPEAKER_MCCOY, TX_MUD2_014);
	else
		loadActorAnimC(OBJECT_MCCOY, "musehn", -1, -1, &Room::mudd2MccoyPutCapsuleInControlPanel);
}

void Room::mudd2MccoyPutCapsuleInControlPanel() {
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorStandAnim(OBJECT_MCCOY);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_020);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_029);
	showText(TX_SPEAKER_SPOCK, TX_MUD2_039);
	if (_vm->_awayMission.mudd.muddUnconscious)
		showText(TX_SPEAKER_MCCOY, TX_MUD2_034);

	_vm->_awayMission.mudd.putCapsuleInMedicalMachine = true;

	loseItem(OBJECT_ICAPSULE);
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xfe, 0xb2);
}

void Room::mudd2UseKirkOnBed() {
	if (!_vm->_awayMission.mudd.putCapsuleInMedicalMachine && !_vm->_awayMission.mudd.translatedAlienLanguage) {
		walkCrewmanC(OBJECT_KIRK, 0xd7, 0xbd, &Room::mudd2KirkReachedBed);
	} else if (_vm->_awayMission.mudd.translatedAlienLanguage) {
		walkCrewman(OBJECT_KIRK, 0xd7, 0xbd, 7); // BUG(?): no continuation?
	}
}

void Room::mudd2KirkReachedBed() {
	// BUGFIX: Speaker is mccoy, not none
	showText(TX_SPEAKER_MCCOY, TX_MUD2_022);
}


void Room::mudd2MuddNoticedKirk() {
	showText(TX_SPEAKER_MUDD, TX_MUD2_044);
	playVoc("BATTYGAS");
	loadActorAnimC(OBJECT_MUDD, "s4sbhb", 0x9f, 0xbf, &Room::mudd2MuddDroppedCapsule); // Drops the capsule
	playMidiMusicTracks(0);
}

void Room::mudd2MuddDroppedCapsule() {
	loadActorAnim2(OBJECT_MUDD, "s4sbhn", 0x9f, 0xbf, 3); // NOTE: no callback from this
	loadActorAnim2(OBJECT_CAPSULE, "s4sbvp", 0x93, 0xc3);
	_vm->_awayMission.mudd.muddDroppedCapsule = true;

	showText(TX_SPEAKER_MCCOY, TX_MUD2_032);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_049);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_026);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_051);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_031);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_050);

	_vm->_awayMission.disableInput = false;
}


void Room::mudd2UsePhaserOnMudd() {
	if (_vm->_awayMission.mudd.muddInhaledGas && !_vm->_awayMission.mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_053);
		showText(TX_SPEAKER_MUDD,    TX_MUD2_042);
		showText(TX_SPEAKER_MUDD,    TX_MUD2_043);
	}
}


// Spock neck-pinches Mudd
void Room::mudd2UseSpockOnMudd() {
	if (_vm->_awayMission.mudd.muddInhaledGas && !_vm->_awayMission.mudd.muddUnconscious) {
		_vm->_awayMission.disableInput = true;
		walkCrewmanC(OBJECT_SPOCK, 0x8a, 0xbf, &Room::mudd2SpockReachedMudd);
	}
}

void Room::mudd2SpockReachedMudd() {
	loadActorAnimC(OBJECT_SPOCK, "s4sbsp", 0x9f, 0xbf, &Room::mudd2SpockPinchedMudd);
	loadActorStandAnim(OBJECT_MUDD);
}

void Room::mudd2SpockPinchedMudd() {
	loadActorAnim2(OBJECT_MUDD, "s4sbob", 0x9f, 0xba);
	loadActorAnim2(OBJECT_SPOCK, "sstnde", 0xd0, 0xbd);

	showText(TX_SPEAKER_SPOCK, TX_MUD2_012);

	_vm->_awayMission.disableInput = false;
	_vm->_awayMission.mudd.muddUnconscious = true;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0x118, 0xc4);
}

void Room::mudd2UseKirkOnMudd() {
	if (_vm->_awayMission.mudd.muddInhaledGas && !_vm->_awayMission.mudd.muddUnconscious) {
		showText(TX_SPEAKER_KIRK, TX_MUD2_001);
		mudd2UseSpockOnMudd();
	}
}


void Room::mudd2UseRedshirtOnMudd() {
	if (_vm->_awayMission.mudd.muddInhaledGas && !_vm->_awayMission.mudd.muddUnconscious) {
		_vm->_awayMission.disableInput = true;
		walkCrewmanC(OBJECT_REDSHIRT, 0xc3, 0xbe, &Room::mudd2RedshirtReachedMudd);
	}
}

void Room::mudd2RedshirtReachedMudd() {
	_vm->_awayMission.timers[2] = 8;
	loadActorAnimC(OBJECT_REDSHIRT, "s4sbrh", -1, -1, &Room::mudd2RedshirtPushedAway);
	playVoc("ROCKFACE");
	loadActorAnimC(OBJECT_MUDD, "s4sbhh", 0x9f, 0xbf, &Room::mudd2MuddFinishedPushingRedshirt);
}

void Room::mudd2Timer2Expired() {
	playSoundEffectIndex(SND_BLANK_0b);
}

void Room::mudd2MuddFinishedPushingRedshirt() {
	loadActorAnim2(OBJECT_MUDD, "s4sbhn");
}

void Room::mudd2RedshirtPushedAway() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD2_052);
	loadActorAnim(OBJECT_REDSHIRT, "rstnds", 0xd8, 0xc3);
	_vm->_awayMission.disableInput = false;
	_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	walkCrewman(OBJECT_REDSHIRT, 0x117, 0xae);
}


// BUGFIX: This was originally "Action 0x45 on Mudd"; as far as I know, action 0x45
// doesn't exist. It's far more likely that 0x45 is supposed to correspond to
// OBJECT_IMTRICOR in a USE action.
// The function itself was also modified (ie. condition for showing text was inverted).
void Room::mudd2UseMTricorderOnMudd() {
	if (_vm->_awayMission.mudd.muddInhaledGas && !_vm->_awayMission.mudd.muddUnconscious)
		mccoyScan(DIR_W, TX_MUD2_013, false);
}


void Room::mudd2UseMedkitOnMudd() {
	if (!_vm->_awayMission.mudd.muddInhaledGas)
		return;
	else if (_vm->_awayMission.mudd.muddUnconscious) {
		if (!_vm->_awayMission.mudd.translatedAlienLanguage)
			showText(TX_SPEAKER_MCCOY, TX_MUD2_015);
		else if (!_vm->_awayMission.mudd.putCapsuleInMedicalMachine)
			showText(TX_SPEAKER_MCCOY, TX_MUD2_021);
		else
			walkCrewmanC(OBJECT_MCCOY, 0xde, 0xaf, &Room::mudd2MccoyReachedMudd);
	}
	else // Can't get to him since he's busy being crazy
		showText(TX_SPEAKER_MCCOY, TX_MUD2_016);
}

void Room::mudd2MccoyReachedMudd() {
	_vm->_awayMission.mudd.missionScore += 2;
	loadActorAnimC(OBJECT_MCCOY, "s4sbms", -1, -1, &Room::mudd2MccoyCuredMudd);
}

void Room::mudd2MccoyCuredMudd() {
	_vm->_awayMission.mudd.muddCurrentlyInsane = false;
	_vm->_awayMission.mudd.muddState = 3;
	_vm->_awayMission.mudd.muddDroppedCapsule = false;
	_vm->_awayMission.mudd.muddInhaledGas = false;

	showText(TX_SPEAKER_MCCOY, TX_MUD2_033);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_045);

	_vm->_awayMission.mudd.muddUnconscious = false;
}


void Room::mudd2LookAtKirk() {
	showText(TX_MUD2N003);
}

void Room::mudd2LookAtSpock() {
	showText(TX_MUD2N006);
}

void Room::mudd2LookAtMccoy() {
	showText(TX_MUD2N000);
}

void Room::mudd2LookAtRedshirt() {
	showText(TX_MUD2N004);
}

void Room::mudd2LookAtMudd() {
	showText(TX_MUD2N002);
}

void Room::mudd2LookAtControlPanel() {
	showText(TX_MUD2N011);
}

void Room::mudd2LookAtBed() {
	showText(TX_MUD2N007);
}

// FIXME: The conditions in the below functions seem wrong.
void Room::mudd2TalkToKirk() {
	if (!_vm->_awayMission.mudd.muddDroppedCapsule || _vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious)
		showText(TX_SPEAKER_KIRK, TX_MUD2_010);
	else {
		showText(TX_SPEAKER_KIRK,  TX_MUD2_005);
		showText(TX_SPEAKER_MUDD,  TX_MUD2_046);
		showText(TX_SPEAKER_MCCOY, TX_MUD2_035);
	}
}

void Room::mudd2TalkToSpock() {
	if (!_vm->_awayMission.mudd.muddDroppedCapsule || _vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious) {
		showText(TX_SPEAKER_SPOCK, TX_MUD2_040);
		showText(TX_SPEAKER_KIRK,  TX_MUD2_011);
	} else {
		showText(TX_SPEAKER_SPOCK, TX_MUD2_038);
	}
}

void Room::mudd2TalkToMccoy() {
	if (!_vm->_awayMission.mudd.muddDroppedCapsule || _vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious) {
		showText(TX_SPEAKER_MCCOY, TX_MUD2_025);
		showText(TX_SPEAKER_KIRK,  TX_MUD2_007);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_MUD2_027);
	}
}

void Room::mudd2TalkToRedshirt() {
	if (!_vm->_awayMission.mudd.muddDroppedCapsule || _vm->_awayMission.mudd.muddInhaledGas || _vm->_awayMission.mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_054);
		showText(TX_SPEAKER_KIRK,    TX_MUD2_008);
	} else {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_055);
		showText(TX_SPEAKER_KIRK,    TX_MUD2_003);
		showText(TX_SPEAKER_MCCOY,   TX_MUD2_048);
	}
}

void Room::mudd2TalkToMudd() {
	if (!_vm->_awayMission.mudd.muddDroppedCapsule || _vm->_awayMission.mudd.muddUnconscious)
		return;
	else if (_vm->_awayMission.mudd.muddInhaledGas) {
		showText(TX_SPEAKER_MUDD, TX_MUD2_048);
		showText(TX_SPEAKER_MCCOY, TX_MUD2_028);
	} else {
		showText(TX_SPEAKER_MUDD, TX_MUD2_047);
		showText(TX_SPEAKER_KIRK, TX_MUD2_006);
	}
}

}
