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

void Room::tug0Tick1() {
	playVoc("TUG0LOOP");
	playMidiMusicTracks(0, -1);

	_awayMission->tug.field2b = 0x23;
	if (_awayMission->tug.transporterRepaired)
		loadActorAnim2(11, "t0con2", 0x105, 0xc2, 0);

	if (!_awayMission->tug.engineerConscious)
		loadActorAnim2(8, "tcdown", 0x27, 0xa8, 0);
	else {
		loadActorAnim2(8, "tcupd", 0x27, 0xa8, 0);

		if (_awayMission->tug.gotTransmogrifier)
			loadActorAnim2(9, "pempty", 0x27, 0xa9, 0);
		else {
			loadActorAnim2(9, "popend", 0x27, 0xa9, 0);
			playVoc("MUR4E6S");
		}
	}
}

void Room::tug0LookAtEngineer() {
	if (_awayMission->tug.engineerConscious)
		showDescription(TX_TUG0N002);
	else
		showDescription(TX_TUG0N014);
}

void Room::tug0GetEngineer() {
	showText(TX_SPEAKER_MCCOY, TX_TUG0_019);
}

void Room::tug0LookAtControls() {
	if (_awayMission->tug.transporterRepaired)
		showDescription(TX_TUG0N012);
	else
		showDescription(TX_TUG0N001);
}

void Room::tug0UseSpockOnControls() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xdb, 0xc5, 16);
}

void Room::tug0SpockReachedControlsToExamine() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 17);
	playSoundEffectIndex(SND_TRICORDER);
}

void Room::tug0SpockExaminedControls() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_040);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_005);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_043);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_013);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_038);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_008);
	_awayMission->tug.spockExaminedTransporter = true;
	_awayMission->disableInput = false;
}

void Room::tug0UseTransmogrifierWithoutBitOnControls() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_014);
}

void Room::tug0UseTransmogrifierWithBitOnControls() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
	walkCrewman(OBJECT_SPOCK, 0xdb, 0xc5, 6);
}

void Room::tug0SpockReachedControlsWithTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_029);
	loadActorAnim2(OBJECT_SPOCK, "susehn", -1, -1, 7);
	playSoundEffectIndex(SND_BLANK_0b); // FIXME: blank sound?
}

void Room::tug0SpockFinishedUsingTransmogrifier() {
	loseItem(OBJECT_IRTWB);
	_awayMission->tug.usedTransmogrifierOnTransporter = true;
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore++;
}

void Room::tug0TransporterScreenFullyLit() {
	loadActorAnim2(11, "t0con2", 0x105, 0xc2, 0);
}

void Room::tug0UseWireScrapsOnControls() {
	if (_awayMission->tug.usedTransmogrifierOnTransporter)
		showText(TX_SPEAKER_SPOCK, TX_TUG0_034);
}

void Room::tug0UseWireOnControls() {
	if (_awayMission->tug.usedTransmogrifierOnTransporter) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_E;
		walkCrewman(OBJECT_SPOCK, 0xdb, 0xc5, 8);
	}
}

void Room::tug0SpockReachedControlsWithWire() {
	loadActorAnim2(OBJECT_SPOCK, "susehn", -1, -1, 9);
}

void Room::tug0SpockFinishedUsingWire() {
	_awayMission->disableInput = false;
	loseItem(OBJECT_IWIRING);
	_awayMission->tug.missionScore++;
	loadActorAnim2(11, "t0con1", 0x105, 0xc2, 22);

	showText(TX_SPEAKER_SPOCK, TX_TUG0_039);

	_awayMission->tug.transporterRepaired = true;

	showText(TX_SPEAKER_KIRK,  TX_TUG0_010);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_022);

	// BUGFIX: check if the redshirt is dead.
	if (!_awayMission->redshirtDead) {
		showText(TX_SPEAKER_KIRK,  TX_TUG0_002);

		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L009);
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L008);

		if (_awayMission->tug.haveBomb) {
			showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L010);
			showText(TX_SPEAKER_MCCOY,       TX_TUG0_021);
			showText(TX_SPEAKER_SPOCK,       TX_TUG0_041);
		}
	}
}

void Room::tug0UseMedkitOnEngineer() {
	if (_awayMission->tug.engineerConscious)
		return;
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x2d, 0xaf, 2);
}

void Room::tug0MccoyReachedEngineer() {
	loadActorAnim2(OBJECT_MCCOY, "museln", -1, -1, 13);
}

void Room::tug0MccoyHealedEngineer() {
	loadActorAnim2(8, "tcup", 0x27, 0xa8, 1);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
	walkCrewman(OBJECT_MCCOY, 0x41, 0xaf, 0);
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore++;
}

void Room::tug0EngineerGotUp() {
	showText(TX_SPEAKER_SIMPSON, TX_TUG0_F31);
	_awayMission->tug.engineerConscious = true;
	loadActorAnim2(8, "tcshow", 0x27, 0xa8, 0);
	loadActorAnim2(9, "popen", 0x27, 0xa9, 0);
	playVoc("MUR4E6S");
}

void Room::tug0GetTransmogrifier() {
	if (_awayMission->tug.gotTransmogrifier)
		return;
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x1e, 0xb1, 3);
}

void Room::tug0KirkReachedToolbox() {
	loadActorAnim2(OBJECT_KIRK, "kuselw", -1, -1, 4);
}

void Room::tug0KirkGotTransmogrifier() {
	_awayMission->tug.gotTransmogrifier = true;
	giveItem(OBJECT_IRT);
	loadActorAnim2(9, "pempty", 0x27, 0xa9, 0);
	loadActorStandAnim(OBJECT_KIRK);
	_awayMission->disableInput = false;
	_awayMission->tug.missionScore++;
}

void Room::tug0LookAtToolbox() {
	if (_awayMission->tug.gotTransmogrifier)
		showDescription(TX_TUG0N008);
	else
		showDescription(TX_TUG0N000);
}

void Room::tug0UsePhaserOnWelder() {
	loseItem(OBJECT_IPWE);
	giveItem(OBJECT_IPWF);
	showDescription(TX_TUG0N006);

	// BUGFIX: this following line didn't exist, despite it existing in TUG1; meaning this
	// was supposed to give points, but it only did in a specific room.
	_awayMission->tug.missionScore += 3;
}

void Room::tug0UseWelderOnWireScraps() {
	showDescription(TX_TUG0N010);
	loseItem(OBJECT_IWIRSCRP);
}

void Room::tug0UseWelderOnMetalScraps() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_032);
	loseItem(OBJECT_IJNKMETL);
	giveItem(OBJECT_ICOMBBIT);
}

void Room::tug0UseCombBitOnTransmogrifier() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_035);
	loseItem(OBJECT_ICOMBBIT);
	loseItem(OBJECT_IRT);
	giveItem(OBJECT_IRTWB);
}

void Room::tug0UseTransporter() {
	if (!_awayMission->tug.transporterRepaired)
		return;
	_awayMission->disableInput = true;

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	walkCrewman(OBJECT_KIRK, 0x62, 0x7f, 0);
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0x11e, 0xc7, 14);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	walkCrewman(OBJECT_MCCOY, 0x7b, 0x7a, 0);
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	walkCrewman(OBJECT_REDSHIRT, 0x4a, 0x7b, 0);
}

void Room::tug0SpockReachedControlsToTransport() {
	loadActorAnim2(OBJECT_SPOCK, "susehw", -1, -1, 18);
	playSoundEffectIndex(SND_07);
	playSoundEffectIndex(SND_TRANSENE);
}

void Room::tug0SpockPreparedTransporter() {
	showText(TX_SPEAKER_KIRK, TX_TUG0_001);
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	walkCrewman(OBJECT_SPOCK, 0x9a, 0x7e, 20);
}

void Room::tug0SpockReachedTransporter() {
	playSoundEffectIndex(SND_TRANSDEM);
	loadActorAnim2(OBJECT_KIRK,     "kteled", -1, -1, 21);
	loadActorAnim2(OBJECT_SPOCK,    "steled", -1, -1, 0);
	loadActorAnim2(OBJECT_MCCOY,    "mteled", -1, -1, 0);
	loadActorAnim2(OBJECT_REDSHIRT, "rteled", -1, -1, 0);
}

void Room::tug0FinishedTransporting() {
	_awayMission->tug.missionScore += 3;
	_awayMission->disableInput = true;
	loadRoomIndex(3, 4);
}

void Room::tug0UseBombOnTransporter() {
	_awayMission->disableInput = true;

	walkCrewman(OBJECT_KIRK, 0x62, 0x7f, 5);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x46, 0xbe, 0);
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	walkCrewman(OBJECT_REDSHIRT, 0x78, 0xbe, 0);
}

void Room::tug0KirkReachedTransporter() {
	loadActorAnim2(OBJECT_KIRK, "kpickw", -1, -1, 10);
}

void Room::tug0KirkPlacedBomb() {
	loseItem(OBJECT_IBOMB);
	loadActorAnim2(10, "bomb1", 0x5a, 0x7f, 0);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
	walkCrewman(OBJECT_KIRK, 0xc8, 0xbe, 11);
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0x11e, 0xc7, 15);
}

void Room::tug0SpockReachedControlsForBomb() {
	loadActorAnim2(OBJECT_SPOCK, "susehw", -1, -1, 19);
	playSoundEffectIndex(SND_07);
	playSoundEffectIndex(SND_TRANSENE);
}

void Room::tug0SpockBeginsBeamingBomb() {
	playSoundEffectIndex(SND_TRANSDEM);
	loadActorAnim2(10, "bomb2", 0x5a, 0x7f, 12);
}

void Room::tug0SpockFinishesBeamingBomb() {
	playSoundEffectIndex(SND_BLANK_14);
	_awayMission->tug.missionScore = 0;
	_awayMission->timers[0] = 64;
}

void Room::tug0BombExploded() {
	playMidiMusicTracks(2, -1);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_023);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_037);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_027);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_004);
	showText(TX_SPEAKER_SCOTT, TX_TUG0_S11);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_003);
	showText(TX_SPEAKER_SCOTT, TX_TUG0_S06);

	_awayMission->tug.field2d = 1;
	_awayMission->disableInput = true;
	if (_awayMission->tug.missionScore < 0)
		_awayMission->tug.missionScore = 0;
	endMission(_awayMission->tug.missionScore, _awayMission->tug.field2b, _awayMission->tug.field2d);
}

void Room::tug0UseMTricorderOnControls() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_016);
}

void Room::tug0UseSTricorderOnControls() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 23);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_015);
}

void Room::tug0UseMTricorderOnEngineer() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x2d, 0xaf, 24);
}

void Room::tug0MccoyReachedEngineerToScan() {
	playSoundEffectIndex(SND_TRICORDER);
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 25);
}

void Room::tug0MccoyFinishedScanningEngineer() {
	if (_awayMission->tug.engineerConscious)
		showText(TX_SPEAKER_MCCOY, TX_TUG0_017);
	else
		showText(TX_SPEAKER_MCCOY, TX_TUG0_018);

	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0x41, 0xaf, 26);
	_awayMission->disableInput = false;
}

void Room::tug0UseSTricorderOnEngineer() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_030);
}

void Room::tug0WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x138, 0xaa, 0);
}

void Room::tug0LookAtKirk() {
	showDescription(TX_TUG0N003);
}

void Room::tug0LookAtSpock() {
	showDescription(TX_TUG0N007);
}

void Room::tug0LookAtMccoy() {
	showDescription(TX_TUG0N005);
}

void Room::tug0LookAtRedshirt() {
	showDescription(TX_TUG0N004);
}

void Room::tug0LookAtTransporter() {
	showDescription(TX_TUG0N013);
}

void Room::tug0LookAtDoor() {
	showDescription(TX_TUG0N011);
}

void Room::tug0TalkToKirk() {
	showText(TX_SPEAKER_KIRK,  TX_TUG0_006);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_036);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_011);
}

void Room::tug0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TUG0_025);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_007);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_024);
}

void Room::tug0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_042);
	showText(TX_SPEAKER_KIRK,  TX_TUG0_009);
}

void Room::tug0TalkToRedshirt() {
	showText(TX_SPEAKER_CHRISTENSEN,  TX_TUG0L006);
}

void Room::tug0TalkToEngineer() {
	if (_awayMission->tug.engineerConscious) {
		showText(TX_SPEAKER_SIMPSON, TX_TUG0_F30);
		showText(TX_SPEAKER_MCCOY,  TX_TUG0_026);
		showText(TX_SPEAKER_KIRK,   TX_TUG0_012);
	}
}

void Room::tug0UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_031);
}

void Room::tug0LookAnywhere() {
	showDescription(TX_TUG0N009);
}

void Room::tug0UseSTricorderAnywhere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_028);
}

void Room::tug0UseMTricorderAnywhere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_020); // BUG: typo
}

}
