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

	if (!_awayMission->love.wrenchTaken)
		loadActorAnim(OBJECT_WRENCH, "wrench", 0x10a, 0xb6, 0);

	if (_awayMission->love.grateRemoved)
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
	else
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);

	if (_awayMission->love.insulationOnGround)
		loadActorAnim(OBJECT_INSULATION, "s3r4p2", 0, 0, 0);

	if (!_awayMission->love.tookN2TankFromServicePanel)
		loadActorAnim(OBJECT_GAS_TANK, "r4n2", 0, 0, 0);

	if (_awayMission->love.servicePanelOpen)
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d1a", 1, 0, 0);
	else
		loadActorAnim(OBJECT_SERVICE_PANEL, "s3r4d2", 1, 0, 0);

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	_roomVar.love.cmnXPosToCureSpock = 0xb4;
	_roomVar.love.cmnYPosToCureSpock = 0xb7;
}

void Room::love3Tick80() {
	if (!_awayMission->love.visitedRoomWithRomulans && haveItem(OBJECT_IRLG)) {
		showText(TX_SPEAKER_MCCOY, 10, true);
		showText(TX_SPEAKER_SPOCK, 20, true);
	}
}

void Room::love3TouchedWarp1() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(SND_DOOR1);
	_awayMission->timers[3] = 10;
}

void Room::love3Timer3Expired() {
	loadRoomIndex(1, 0);
}

void Room::love3LookAtServicePanel() {
	showDescription(TX_LOV3NA20);
}

void Room::love3LookAtMonitor() {
	showDescription(0, true); // BUGFIX: original didn't play the audio
}

void Room::love3LookAtWrench() {
	showDescription(2, true);
}

void Room::love3LookAtKirk() {
	showDescription(3, true);
}

void Room::love3LookAtMccoy() {
	showDescription(5, true);
}

void Room::love3LookAtSpock() {
	showDescription(8, true);
}

void Room::love3LookAtRedshirt() {
	showDescription(4, true);
}

void Room::love3LookAnywhere() {
	showDescription(TX_LOV3NA23);
}

void Room::love3LookAtShaft() {
	showDescription(TX_LOV3NA21);
}

void Room::love3LookAtGrate() {
	showDescription(19, true);
}

void Room::love3LookAtPanel() {
	if (_awayMission->love.insulationOnGround)
		showDescription(17, true);
	else
		showDescription(TX_LOV3NA22);
}

void Room::love3LookAtGasTank() {
	showDescription(6, true);
}

void Room::love3LookAtTurbines() {
	showDescription(26, true);
	showText(TX_SPEAKER_MCCOY, 9, true);
	showText(TX_SPEAKER_SPOCK, 19, true);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, 26, true);
}

void Room::love3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::love3TalkToSpock() {
	if (_awayMission->love.visitedRoomWithRomulans)
		showText(TX_SPEAKER_SPOCK, 3, true);
	else if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		showText(TX_SPEAKER_SPOCK, 17, true);
	else if (haveItem(OBJECT_IRLG))
		showText(TX_SPEAKER_SPOCK, 4, true);
	else
		showText(TX_SPEAKER_SPOCK, 3, true);
}

void Room::love3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 8, true);
}

void Room::love3TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 24, true);
}

void Room::love3UseMTricorderAnywhere() {
	if (_awayMission->love.knowAboutVirus)
		mccoyScan(DIR_S, 5, false, true);
	else
		mccoyScan(DIR_S, 6, false, true);
}

void Room::love3UseSTricorderOnTurbines() {
	spockScan(DIR_N, 14, false, true);
}

void Room::love3UseSTricorderOnInsulation() {
	if (_awayMission->love.insulationOnGround)
		spockScan(DIR_W, 2, false, true);
}

void Room::love3UseSTricorderAnywhere() {
	spockScan(DIR_S, 13, false, true);
}

void Room::love3UseKirkOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_KIRK;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseSpockOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_SPOCK;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseMccoyOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_MCCOY;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3UseRedshirtOnServicePanel() {
	_roomVar.love.activeCrewman = OBJECT_REDSHIRT;
	walkCrewman(_roomVar.love.activeCrewman, 0x104, 0xae, 1);
}

void Room::love3CrewmanReachedServicePanel() {
	Common::String anim = getCrewmanAnimFilename(_roomVar.love.activeCrewman, "usehe");
	loadActorAnim2(_roomVar.love.activeCrewman, anim, -1, -1, 1);
}

void Room::love3OpenedOrClosedServicePanel() {
	if (_awayMission->love.servicePanelOpen)
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1b", 1, 0, 0);
	else
		loadActorAnim2(OBJECT_SERVICE_PANEL, "s3r4d1", 1, 0, 0);

	playVoc("SMADOOR1");
	_awayMission->love.servicePanelOpen = !_awayMission->love.servicePanelOpen;

	if (_awayMission->love.servicePanelOpen) {
		if (_awayMission->love.tookN2TankFromServicePanel)
			showDescription(23, true);
		else
			showDescription(22, true);
	} else
		showDescription(20, true);
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
	if (!_awayMission->love.gasTankUnscrewed) {
		showDescription(12, true);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_FERRIS, 22, true);
		_awayMission->love.gasTankUnscrewed = true;
	} else {
		showDescription(15, true);
		_awayMission->love.gasTankUnscrewed = false;
	}
}


void Room::love3UseWrenchOnO2OrH2Gas() {
	showText(TX_SPEAKER_SPOCK, 16, true);
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
	showDescription(13, true);
	_awayMission->love.tookN2TankFromServicePanel = false;
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
	if (!_awayMission->love.insulationOnGround) {
		playVoc("MADR4E4A");
		loadActorAnim(OBJECT_INSULATION, "s3r4p1", 0, 0, 0);
		_awayMission->love.insulationOnGround = true;
	}
	showDescription(21, true);
}


void Room::love3UseWrenchOnMonitor() {
	showText(TX_SPEAKER_SPOCK, 18, true);
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
	showText(TX_SPEAKER_FERRIS, 21, true);

	loseItem(OBJECT_IH2O);
}


void Room::love3UseCrewmanOnEngineeringPanelOrGrate() {
	showDescription(30, true);
}


void Room::love3UseWrenchOnGrate() {
	walkCrewmanC(OBJECT_KIRK, 0xf6, 0xaa, &Room::love3ReachedGateWithWrench);
	_awayMission->disableInput = true;
}

void Room::love3ReachedGateWithWrench() {
	if (_awayMission->love.grateRemoved)
		loadActorAnimC(OBJECT_KIRK, "s3r4g2a", -1, -1, &Room::love3OpenedOrClosedGrate);
	else
		loadActorAnimC(OBJECT_KIRK, "s3r4g2",  -1, -1, &Room::love3OpenedOrClosedGrate);
	loadActorStandAnim(OBJECT_GRATE);
	playVoc("MUR3E1S");
}

void Room::love3OpenedOrClosedGrate() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);

	if (!_awayMission->love.grateRemoved) {
		_awayMission->love.grateRemoved = true;
		loadActorAnim(OBJECT_GRATE, "s3r4g3", 0, 0, 0);
		showDescription(16, true);
		if (!_awayMission->love.gotPointsForOpeningGrate) {
			_awayMission->love.gotPointsForOpeningGrate = true;
			_awayMission->love.missionScore++;
		}
	} else {
		loadActorAnim(OBJECT_GRATE, "s3r4g1", 0x116, 0x8f, 0);
		_awayMission->love.grateRemoved = false;
		showDescription(32, true);
	}
}


void Room::love3UseCrewmanOnShaft() {
	showDescription(11, true);
}


void Room::love3UseWaterOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingWater);
}

void Room::love3ReachedShaftUsingWater() {
	if (!_awayMission->love.grateRemoved)
		showDescription(25, true);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredWaterDownShaft);
}

void Room::love3PouredWaterDownShaft() {
	showDescription(9, true);
	loseItem(OBJECT_IH2O);
}


void Room::love3UseNitrousOxideOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingNitrousOxide);
}

void Room::love3ReachedShaftUsingNitrousOxide() {
	if (!_awayMission->love.grateRemoved)
		showDescription(25, true);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredNitrousOxideDownShaft);
}

void Room::love3PouredNitrousOxideDownShaft() {
	showDescription(18, true);
	showText(TX_SPEAKER_SPOCK, 12, true);
	loseItem(OBJECT_IN2O);
}


void Room::love3UseAmmoniaOnShaft() {
	walkCrewmanC(OBJECT_KIRK, 0x100, 0xa6, &Room::love3ReachedShaftUsingAmmonia);
}

void Room::love3ReachedShaftUsingAmmonia() {
	if (!_awayMission->love.grateRemoved)
		showDescription(25, true);
	else
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredAmmoniaDownShaft);
}

void Room::love3PouredAmmoniaDownShaft() {
	if (!_awayMission->love.romulansUnconsciousFromVirus) {
		showDescription(01, true);
		if (!_awayMission->redshirtDead)
			showText(TX_SPEAKER_FERRIS, 23, true);
	}

	loseItem(OBJECT_INH3);
}


void Room::love3UseRomulanLaughingGasOnShaft() {
	if (_awayMission->love.gotPointsForGassingRomulans) {
		// BUG-ish: you could get the points for this by starting the action, then
		// canceling it before Kirk reaches the shaft.
		_awayMission->love.gotPointsForGassingRomulans = true;
		_awayMission->love.missionScore += 6;
	}
	if (_awayMission->love.romulansUnconsciousFromLaughingGas)
		showText(TX_SPEAKER_SPOCK, 11, true);
	else {
		walkCrewmanC(OBJECT_KIRK, 0xf6, 0xa4, &Room::love3ReachedShaftUsingRomulanLaughingGas);
	}
}

void Room::love3ReachedShaftUsingRomulanLaughingGas() {
	if (!_awayMission->love.grateRemoved)
		showDescription(25, true);
	else {
		loadActorAnimC(OBJECT_KIRK, "kuseme", -1, -1, &Room::love3PouredRomulanLaughingGasDownShaft);
		playVoc("EFX24");
	}
}

void Room::love3PouredRomulanLaughingGasDownShaft() {
	loseItem(OBJECT_IRLG);
	_awayMission->love.romulansUnconsciousFromLaughingGas = true;

	if (_awayMission->love.romulansUnconsciousFromVirus) {
		showDescription(TX_LOV3NA08);
		_awayMission->love.romulansUnconsciousFromLaughingGas = false;
	} else {
		showDescription(TX_LOV3NA09);
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
	_awayMission->love.wrenchTaken = true;
	showDescription(31, true);
}


// Get the N2 gas tank in the service panel (fails due to weight)
void Room::love3GetGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x121, 0xb7, &Room::love3ReachedGasTankToGet);
}

void Room::love3ReachedGasTankToGet() {
	if (_awayMission->love.gasTankUnscrewed)
		showDescription(07, true);
	else
		showDescription(14, true);
}


// Use antigrav unit N2 gas tank in the service panel (succeeds)
void Room::love3UseAntigravOnGasTank() {
	walkCrewmanC(OBJECT_KIRK, 0x10e, 0xae, &Room::love3ReachedGasTankUsingAntigrav);
}

void Room::love3ReachedGasTankUsingAntigrav() {
	if (_awayMission->love.gasTankUnscrewed) {
		loadActorAnimC(OBJECT_KIRK, "kusehe", -1, -1, &Room::love3PickedUpGasTank);
		playVoc("SE3PLBAT");
	} else {
		showDescription(10, true);
		showText(TX_SPEAKER_SPOCK, 15, true);
	}
}

void Room::love3PickedUpGasTank() {
	giveItem(OBJECT_IN2GAS);
	loadActorStandAnim(OBJECT_GAS_TANK);
	_awayMission->love.tookN2TankFromServicePanel = true;

	showDescription(TX_LOV3NJ32);
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_FERRIS, 25, true);
}

void Room::love3GetInsulation() {
	walkCrewmanC(OBJECT_KIRK, 0x32, 0xbe, &Room::love3ReachedInsulationToGet);
}

void Room::love3ReachedInsulationToGet() {
	if (!_awayMission->love.field3c)
		loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::love3PickedUpInsulation);
}

void Room::love3PickedUpInsulation() {
	loadActorStandAnim(OBJECT_KIRK);
	giveItem(OBJECT_IINSULAT);
	showDescription(29, true);
}

}
