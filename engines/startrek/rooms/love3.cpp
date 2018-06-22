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

#define HOTSPOT_ENGINEERING_PANEL 0x20
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

	if (_vm->_awayMission.love.insulationOnGround)
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
	if (!_vm->_awayMission.love.visitedRoomWithRomulans && haveItem(OBJECT_IRLG)) {
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
	if (_vm->_awayMission.love.insulationOnGround)
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
	if (_vm->_awayMission.love.visitedRoomWithRomulans)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_003);
	else if (_vm->_awayMission.love.romulansUnconsciousFromLaughingGas)
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
	if (_vm->_awayMission.love.insulationOnGround)
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


// Use wrench on engineering panel, insulation comes out
void Room::love3UseWrenchOnEngineeringPanel() {
	walkCrewman(OBJECT_KIRK, 0x37, 0xb4, 3);
}

void Room::love3ReachedEngineeringPanel() {
	loadActorAnim2(OBJECT_KIRK, "kuselw", -1, -1, 4);
	playVoc("MUR3E1S");
}

void Room::love3OpenedEngineeringPanel() {
	if (!_vm->_awayMission.love.insulationOnGround) {
		playVoc("MADR4E4A");
		loadActorAnim(OBJECT_INSULATION, "s3r4p1", 0, 0, 0);
		_vm->_awayMission.love.insulationOnGround = true;
	}
	showText(TX_LOV3N021);
}


void Room::love3UseWrenchOnMonitor() {
	showText(TX_SPEAKER_SPOCK, TX_LOV3_018);
}


void Room::love3UseWaterOnMonitor() {
	walkCrewmanC(OBJECT_KIRK, 0xc8, 0xb2, &Room::love3ReachedMonitorToUseWater);
}

void Room::love3ReachedMonitorToUseWater() {
	loadActorAnimC(OBJECT_KIRK, "kusemn", -1, -1, &Room::love3PouredWaterOnMonitor);
}

void Room::love3PouredWaterOnMonitor() {
	// BUGFIX: the speaker should be Lt. Ferris. In the original, the speaker wasn't
	// specified.
	// However, there is no substitute text for when Lt. Ferris is dead.
	showText(TX_SPEAKER_FERRIS, TX_LOV3_021);

	loseItem(OBJECT_IH2O);
}


void Room::love3UseCrewmanOnEngineeringPanelOrGrate() {
	showText(TX_LOV3N030);
}


void Room::love3UseWrenchOnGrate() {
	walkCrewmanC(OBJECT_KIRK, 0xf6, 0xaa, &Room::love3ReachedGateWithWrench);
	_vm->_awayMission.disableInput = true;
}

void Room::love3ReachedGateWithWrench() {
	if (_vm->_awayMission.love.grateRemoved)
		loadActorAnimC(OBJECT_KIRK, "s3r4g2a", -1, -1, &Room::love3OpenedOrClosedGrate);
	else
		loadActorAnimC(OBJECT_KIRK, "s3r4g2",  -1, -1, &Room::love3OpenedOrClosedGrate);
	loadActorStandAnim(OBJECT_GRATE);
	playVoc("MUR3E1S");
}

void Room::love3OpenedOrClosedGrate() {
	_vm->_awayMission.disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);

	if (!_vm->_awayMission.love.grateRemoved) {
		_vm->_awayMission.love.grateRemoved = true;
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
		showText(TX_LOV3N016);
		if (!_vm->_awayMission.love.gotPointsForOpeningGrate) {
			_vm->_awayMission.love.gotPointsForOpeningGrate = true;
			_vm->_awayMission.love.missionScore++;
		}
	}
	else {
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);
		_vm->_awayMission.love.grateRemoved = false;
		showText(TX_LOV3N032);
	}
}


void Room::love3UseCrewmanOnShaft() {
	showText(TX_LOV3N011);
}


void Room::love3UseWaterOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingWater);
}

void Room::love3ReachedShaftUsingWater() {
	if (!_vm->_awayMission.love.grateRemoved)
		showText(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredWaterDownShaft);
}

void Room::love3PouredWaterDownShaft() {
	showText(TX_LOV3N009);
	loseItem(OBJECT_IH2O);
}


void Room::love3UseNitrousOxideOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingNitrousOxide);
}

void Room::love3ReachedShaftUsingNitrousOxide() {
	if (!_vm->_awayMission.love.grateRemoved)
		showText(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredNitrousOxideDownShaft);
}

void Room::love3PouredNitrousOxideDownShaft() {
	showText(TX_LOV3N018);
	showText(TX_SPEAKER_SPOCK, TX_LOV3_012);
	loseItem(OBJECT_IN2O);
}


void Room::love3UseAmmoniaOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingAmmonia);
}

void Room::love3ReachedShaftUsingAmmonia() {
	if (!_vm->_awayMission.love.grateRemoved)
		showText(TX_LOV3N025);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredAmmoniaDownShaft);
}

void Room::love3PouredAmmoniaDownShaft() {
	if (!_vm->_awayMission.love.romulansUnconsciousFromVirus) {
		showText(TX_LOV3N001);
		if (!_vm->_awayMission.redshirtDead)
			showText(TX_SPEAKER_FERRIS, TX_LOV3_023);
	}

	loseItem(OBJECT_INH3);
}


void Room::love3UseRomulanLaughingGasOnShaft() {
	if (_vm->_awayMission.love.gotPointsForGassingRomulans) {
		// BUG-ish: you could get the points for this by starting the action, then
		// canceling it before Kirk reaches the shaft.
		_vm->_awayMission.love.gotPointsForGassingRomulans = true;
		_vm->_awayMission.love.missionScore += 6;
	}
	if (_vm->_awayMission.love.romulansUnconsciousFromLaughingGas)
		showText(TX_SPEAKER_SPOCK, TX_LOV3_011);
	else {
		walkCrewmanC(OBJECT_KIRK, 0xf6, 0xa4, &Room::love3ReachedShaftUsingRomulanLaughingGas);
	}
}

void Room::love3ReachedShaftUsingRomulanLaughingGas() {
	if (!_vm->_awayMission.love.grateRemoved)
		showText(TX_LOV3N025);
	else {
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredRomulanLaughingGasDownShaft);
		playVoc("EFX24");
	}
}

void Room::love3PouredRomulanLaughingGasDownShaft() {
	loseItem(OBJECT_IRLG);
	_vm->_awayMission.love.romulansUnconsciousFromLaughingGas = true;

	if (_vm->_awayMission.love.romulansUnconsciousFromVirus) {
		showText(TX_LOV3NA08);
		_vm->_awayMission.love.romulansUnconsciousFromLaughingGas = false;
	}
	else {
		showText(TX_LOV3NA09);
		playVoc("ROMULANL");
	}
}


// Get the wrench on the ground
void Room::love3GetWrench() {
	walkCrewmanC(OBJECT_KIRK, 0x104, 0xb6, &Room::love3ReachedWrenchToGet);
}

void Room::love3ReachedWrenchToGet() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::love3PickedUpWrench);
}

void Room::love3PickedUpWrench() {
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IWRENCH);
	loadActorStandAnim(OBJECT_WRENCH);
	_vm->_awayMission.love.wrenchTaken = true;
	showText(TX_LOV3N031);
}


// Get the N2 gas tank in the service panel (fails due to weight)
void Room::love3GetGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x121, 0xb7, &Room::love3ReachedGasTankToGet);
}

void Room::love3ReachedGasTankToGet() {
	if (_vm->_awayMission.love.gasTankUnscrewed)
		showText(TX_LOV3N007);
	else
		showText(TX_LOV3N014);
}


// Use antigrav unit N2 gas tank in the service panel (succeeds)
void Room::love3UseAntigravOnGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x10e, 0xae, &Room::love3ReachedGasTankUsingAntigrav);
}

void Room::love3ReachedGasTankUsingAntigrav() {
	if (_vm->_awayMission.love.gasTankUnscrewed) {
		loadActorAnimC(OBJECT_KIRK, "kusehe", -1, -1, &Room::love3PickedUpGasTank);
		playVoc("SE3PLBAT");
	}
	else {
		showText(TX_LOV3N010);
		showText(TX_SPEAKER_SPOCK, TX_LOV3_015);
	}
}

void Room::love3PickedUpGasTank() {
	giveItem(OBJECT_IN2GAS);
	loadActorStandAnim(OBJECT_GAS_TANK);
	_vm->_awayMission.love.tookN2TankFromServicePanel = true;

	showText(TX_LOV3NJ32);
	if (!_vm->_awayMission.redshirtDead)
		showText(TX_SPEAKER_FERRIS, TX_LOV3_025);
}

void Room::love3GetInsulation() {
	walkCrewmanC(OBJECT_KIRK, 0x32, 0xbe, &Room::love3ReachedInsulationToGet);
}

void Room::love3ReachedInsulationToGet() {
	if (!_vm->_awayMission.love.field3c)
		loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::love3PickedUpInsulation);
}

void Room::love3PickedUpInsulation() {
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IINSULAT);
	showText(TX_LOV3N029);
}

}
