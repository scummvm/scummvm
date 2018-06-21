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

#define OBJECT_SERVICE_PANEL 8
#define OBJECT_INSULATION 9
#define OBJECT_WRENCH 10
#define OBJECT_GRATE 11
#define OBJECT_GAS_TANK 12

#define HOTSPOT_INSULATION 0x20
#define HOTSPOT_MONITOR 0x21
#define HOTSPOT_SHAFT 0x22
#define HOTSPOT_TURBINES 0x23

namespace StarTrek {

void Room::love3Tick1() {
	playVoc("LOV3LOOP");

	if (!_vm->_awayMission.love.wrenchTaken)
		loadActorAnim(OBJECT_WRENCH, "wrench", 0x10a, 0xb6, 0);

	if (_vm->_awayMission.love.grateRemoved)
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
	else
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);

	if (_vm->_awayMission.love.field3e)
		loadActorAnim(OBJECT_INSULATION, "s3r4p2", 0, 0, 0);

	if (!_vm->_awayMission.love.tookN2TankFromServicePanel)
		loadActorAnim(OBJECT_GAS_TANK, "r4n2", 0, 0, 0);

	if (_vm->_awayMission.love.servicePanelOpen)
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d1a", 1, 0, 0);
	else
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d2", 1, 0, 0);

	if (_vm->_awayMission.love.field2c)
		_vm->_awayMission.timers[0] = getRandomWordInRange(200, 400);
	if (_vm->_awayMission.love.field2d)
		_vm->_awayMission.timers[1] = getRandomWordInRange(200, 400);

	_vm->_awayMission.timers[2] = 200;

	_roomVar.love3._188e = 0xb4;
	_roomVar.love3._188f = 0xb7;
}

void Room::love3Tick80() {
	if (!_vm->_awayMission.love.field3f && haveItem(OBJECT_IRLG)) {
		showText(TX_SPEAKER_MCCOY, TX_LOV3_010);
		showText(TX_SPEAKER_SPOCK, TX_LOV3_020);
	}
}

void Room::love3TouchedWarp1() {
	_vm->_awayMission.disableInput = true;
	playSoundEffectIndex(SND_DOOR1);
	_vm->_awayMission.timers[3] = 10;
}

void Room::love3Timer3Expired() {
	loadRoomIndex(1, 0);
}

void Room::love3LookAtServicePanel() {
	showText(TX_LOV3NA20);
}

void Room::love3LookAtMonitor() {
	showText(TX_LOV3N000); // BUGFIX: original didn't play the audio
}

void Room::love3LookAtWrench() {
	showText(TX_LOV3N002);
}

void Room::love3LookAtKirk() {
	showText(TX_LOV3N003);
}

void Room::love3LookAtMccoy() {
	showText(TX_LOV3N005);
}

void Room::love3LookAtSpock() {
	showText(TX_LOV3N008);
}

void Room::love3LookAtRedshirt() {
	showText(TX_LOV3N004);
}

void Room::love3LookAnywhere() {
	showText(TX_LOV3NA23);
}

void Room::love3LookAtShaft() {
	showText(TX_LOV3N019);
}

void Room::love3LookAtPanel() {
	if (_vm->_awayMission.love.field3e)
		showText(TX_LOV3N017);
	else
		showText(TX_LOV3NA22);
}

void Room::love3LookAtGasTank() {
	showText(TX_LOV3N006);
}

void Room::love3LookAtTurbines() {
	showText(TX_LOV3N026);
	showText(TX_SPEAKER_MCCOY, TX_LOV3_009);
	showText(TX_SPEAKER_SPOCK, TX_LOV3_019);
	if (!_vm->_awayMission.redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV3_026);
}

void Room::love3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_LOV3_001);
}

void Room::love3TalkToSpock() {
	if (_vm->_awayMission.love.field3f)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_003);
	if (_vm->_awayMission.love.putLaughingGasInVent)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_017);
	else if (haveItem(OBJECT_IRLG))
		showText(TX_SPEAKER_SPOCK, TX_LOV3_004);
	else
		showText(TX_SPEAKER_SPOCK, TX_LOV3_003);
}

void Room::love3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_LOV3_008);
}

void Room::love3TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV3_024);
}

void Room::love3UseMTricorderAnywhere() {
	if (_vm->_awayMission.love.knowAboutVirus)
		mccoyScan(DIR_S, TX_LOV3_005, false);
	else
		mccoyScan(DIR_S, TX_LOV3_006, false);
}

void Room::love3UseSTricorderOnTurbines() {
	spockScan(DIR_N, TX_LOV3_014, false);
}

void Room::love3UseSTricorderOnInsulation() {
	if (_vm->_awayMission.love.field3e)
		spockScan(DIR_W, TX_LOV3_002, false);
}

void Room::love3UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_LOV3_013, false);
}

void Room::love3UseKirkOnServicePanel() {
	_roomVar.love3.activeCrewman = OBJECT_KIRK;
	walkCrewman(_roomVar.love3.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseSpockOnServicePanel() {
	_roomVar.love3.activeCrewman = OBJECT_SPOCK;
	walkCrewman(_roomVar.love3.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseMccoyOnServicePanel() {
	_roomVar.love3.activeCrewman = OBJECT_MCCOY;
	walkCrewman(_roomVar.love3.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseRedshirtOnServicePanel() {
	_roomVar.love3.activeCrewman = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love3.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3CrewmanReachedServicePanel() {
	Common::String anim = _vm->getCrewmanAnimFilename(_roomVar.love3.activeCrewman, "usehe");
	loadActorAnim2(_roomVar.love3.activeCrewman, anim, -1, -1, 1);
}

void Room::love3OpenedOrClosedServicePanel() {
	if (_vm->_awayMission.love.servicePanelOpen)
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1b", 1, 0, 0);
	else
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1", 1, 0, 0);

	playVoc("SMADOOR1");
	_vm->_awayMission.love.servicePanelOpen = !_vm->_awayMission.love.servicePanelOpen;

	if (_vm->_awayMission.love.servicePanelOpen) {
		if (_vm->_awayMission.love.tookN2TankFromServicePanel)
			showText(TX_LOV3N023);
		else
			showText(TX_LOV3N022);
	}
	else
		showText(TX_LOV3N020);
}

// Use wrench on gas tank, to screw or unscrew it in position
void Room::love3UseWrenchOnGasTank() {
	walkCrewman(OBJECT_KIRK, 0x10e, 0xae, 2);
}

void Room::love3ReachedGasTankToUnscrew() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 2);
	playVoc("MUR3E1S");
}

void Room::love3ScrewedOrUnscrewedGasTank() {
	if (!_vm->_awayMission.love.gasTankUnscrewed) {
		showText(TX_LOV3N012);
		if (!_vm->_awayMission.redshirtDead)
			showText(TX_SPEAKER_FERRIS, TX_LOV3_022);
		_vm->_awayMission.love.gasTankUnscrewed = true;
	}
	else {
		showText(TX_LOV3N015);
		_vm->_awayMission.love.gasTankUnscrewed = false;
	}
}


void Room::love3UseWrenchOnO2OrH2Gas() {
	showText(TX_SPEAKER_SPOCK, TX_LOV3_016);
}


// Use wrench on N2 gas = putting it from the inventory back to the service panel
void Room::love3UseWrenchOnN2Gas() {
	walkCrewman(OBJECT_KIRK, 0x10e, 0xae, 15);
}

void Room::love3ReachedGasTankToPutDown() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 3);
	playVoc("MUR3E1S");
}

void Room::love3PutN2TankBack() {
	showText(TX_LOV3N013);
	_vm->_awayMission.love.tookN2TankFromServicePanel = false;
	loseItem(OBJECT_IN2GAS);
	loadActorAnim(OBJECT_GAS_TANK, "r4n2", 0, 0, 0);
}

}
