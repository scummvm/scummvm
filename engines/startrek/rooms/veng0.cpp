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

#define OBJECT_BEAM 8
#define OBJECT_9 9
#define OBJECT_10 10

#define HOTSPOT_CREWMAN_1 0x20 // This one lives for a bit
#define HOTSPOT_CHAIR 0x21
#define HOTSPOT_CREWMAN_2 0x22
#define HOTSPOT_COMPUTER_BANK 0x23
#define HOTSPOT_PATTERSON 0x24
#define HOTSPOT_NAV_CONSOLE 0x25
#define HOTSPOT_HELM_CONSOLE 0x26
#define HOTSPOT_VIEWSCREEN 0x27

namespace StarTrek {

extern const RoomAction veng0ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::veng0Tick1 },
	{ {ACTION_TICK, 10, 0, 0}, &Room::veng0Tick10 },
	{ {ACTION_TICK, 50, 0, 0}, &Room::veng0Tick50 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0}, &Room::veng0TouchedDoor },

	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_COMPUTER_BANK, 0}, &Room::veng0UseSTricorderOnComputerBank },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_CHAIR,         0}, &Room::veng0UseSTricorderOnChair },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_VIEWSCREEN,    0}, &Room::veng0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,                  0}, &Room::veng0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_BEAM,           0}, &Room::veng0UseMccoyOnBeam },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_BEAM,           0}, &Room::veng0UseSTricorderOnBeam },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_BEAM,           0}, &Room::veng0UseSTricorderOnBeam },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_BEAM,           0}, &Room::veng0UseRedshirtOnBeam },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_HELM_CONSOLE,  0}, &Room::veng0UseSpockOnConsole },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_NAV_CONSOLE,   0}, &Room::veng0UseSpockOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_HELM_CONSOLE,  0}, &Room::veng0UseSpockOnConsole },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_NAV_CONSOLE,   0}, &Room::veng0UseSpockOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_HELM_CONSOLE,  0}, &Room::veng0UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_NAV_CONSOLE,   0}, &Room::veng0UseRedshirtOnConsole },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_BEAM,           0}, &Room::veng0UsePhaserOnBeam },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_BEAM,           0}, &Room::veng0UsePhaserOnBeam },

	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_PATTERSON, 0}, &Room::veng0UseMccoyOnDeadCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_PATTERSON, 0}, &Room::veng0UseMccoyOnDeadCrewman },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_CREWMAN_2, 0}, &Room::veng0UseMccoyOnDeadCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  HOTSPOT_CREWMAN_2, 0}, &Room::veng0UseMccoyOnDeadCrewman },
	{ {ACTION_USE, OBJECT_IMEDKIT,  HOTSPOT_PATTERSON, 0}, &Room::veng0UseMccoyOnDeadCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_CREWMAN_2, 0}, &Room::veng0UseMccoyOnDeadCrewman },

	{ {ACTION_USE, OBJECT_ILMD, OBJECT_BEAM, 0}, &Room::veng0UseSawOnBeam },
	{ {ACTION_DONE_WALK, 1,               0, 0}, &Room::veng0KirkReachedSaw },
	{ {ACTION_TIMER_EXPIRED, 0,           0, 0}, &Room::veng0Timer0Expired },
	{ {ACTION_DONE_ANIM, 2,               0, 0}, &Room::veng0DoneCuttingBeam },

	{ {ACTION_GET,  OBJECT_BEAM,           0, 0}, &Room::veng0GetBeam },
	{ {ACTION_TALK, OBJECT_KIRK,           0, 0}, &Room::veng0TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY,          0, 0}, &Room::veng0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK,          0, 0}, &Room::veng0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT,       0, 0}, &Room::veng0TalkToRedshirt },
	{ {ACTION_LOOK, HOTSPOT_PATTERSON,     0, 0}, &Room::veng0LookAtPatterson },
	{ {ACTION_LOOK, HOTSPOT_CREWMAN_2,     0, 0}, &Room::veng0LookAtDeadCrewman2 },
	{ {ACTION_LOOK, HOTSPOT_COMPUTER_BANK, 0, 0}, &Room::veng0LookAtComputerBank },
	{ {ACTION_LOOK, HOTSPOT_CHAIR,         0, 0}, &Room::veng0LookAtChair },
	{ {ACTION_LOOK, OBJECT_KIRK,           0, 0}, &Room::veng0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,          0, 0}, &Room::veng0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,          0, 0}, &Room::veng0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,       0, 0}, &Room::veng0LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_VIEWSCREEN,    0, 0}, &Room::veng0LookAtViewscreen },
	{ {ACTION_LOOK, HOTSPOT_HELM_CONSOLE,  0, 0}, &Room::veng0LookAtHelmConsole },
	{ {ACTION_LOOK, HOTSPOT_NAV_CONSOLE,   0, 0}, &Room::veng0LookAtNavConsole },
	{ {ACTION_LOOK, OBJECT_BEAM,           0, 0}, &Room::veng0LookAtBeam },
	{ {ACTION_LOOK, 0xff,                  0, 0}, &Room::veng0LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_CREWMAN_1,     0, 0}, &Room::veng0LookAtDeadCrewman1 },

	{ {ACTION_USE, OBJECT_IMEDKIT,  HOTSPOT_CREWMAN_1, 0}, &Room::veng0UseMccoyOnLivingCrewman },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_CREWMAN_1, 0}, &Room::veng0UseMccoyOnLivingCrewman },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_CREWMAN_1, 0}, &Room::veng0UseMccoyOnLivingCrewman },
	{ {ACTION_DONE_WALK, 3,                         0, 0}, &Room::veng0MccoyReachedCrewman },
	{ {ACTION_DONE_ANIM, 1,                         0, 0}, &Room::veng0MccoyScannedCrewman },

	// Common code
	{ {ACTION_TICK, 0xff, 0xff, 0xff}, &Room::vengaTick },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff,     0}, &Room::vengaUsePhaserAnywhere },
	{ {ACTION_LOOK, OBJECT_IHYPO,          0, 0}, &Room::vengaLookAtHypo },
	{ {ACTION_USE, OBJECT_ICOMM, OBJECT_KIRK, 0}, &Room::vengaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::veng0Tick1() {
	playVoc("VEN0LOOP");

	if (_awayMission->veng.beamState != 2)
		loadActorAnim2(OBJECT_BEAM, "s7r0bo", 0x28, 0xc6);
	else
		loadActorAnim2(OBJECT_BEAM, "s7r0bf", 0x0e, 0x9a);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
}

void Room::veng0Tick10() {
	// stub
}

void Room::veng0Tick50() {
	if (!_awayMission->veng.enteredRoom0FirstTime) {
		_awayMission->veng.enteredRoom0FirstTime = true;
		showText(TX_SPEAKER_MCCOY, 18, true);
		showText(TX_SPEAKER_KIRK,   6, true);
		showText(TX_SPEAKER_SCOTT, 12 + SCOTTY_MESSAGE_OFFSET, true);
		showText(TX_SPEAKER_KIRK,   8, true);
		showText(TX_SPEAKER_SCOTT, 52 + SCOTTY_MESSAGE_OFFSET, true);
		showText(TX_SPEAKER_KIRK,   7, true);
		showText(TX_SPEAKER_SPOCK, 30, true);
		showText(TX_SPEAKER_SCOTT, 15 + SCOTTY_MESSAGE_OFFSET, true);
		showText(TX_SPEAKER_KIRK,   5, true);
		showText(TX_SPEAKER_CHEKOV, 39, true);
		showText(TX_SPEAKER_KIRK,   9, true);
		showText(TX_SPEAKER_SCOTT, 54 + SCOTTY_MESSAGE_OFFSET, true);
		showText(TX_SPEAKER_KIRK,  11, true);
		showText(TX_SPEAKER_KIRK,  10, true);
		showText(TX_SPEAKER_MCCOY, 17, true);
		_awayMission->veng.enterpriseLeftForDistressCall = true;
	}
}

void Room::veng0TouchedDoor() {
	playSoundEffectIndex(kSfxDoor);
	showRepublicMap(0, 0);
}

void Room::veng0UseSTricorderOnComputerBank() {
	if (!_awayMission->veng.scannedComputerBank) {
		_awayMission->veng.scannedComputerBank = true;
		_awayMission->veng.field6c = true;

		spockScan(DIR_S, 28, false, true);
		showText(TX_SPEAKER_KIRK,   2, true);
		showText(TX_SPEAKER_SPOCK, 29, true);
		showText(TX_SPEAKER_MCCOY, 20, true);
		showText(TX_SPEAKER_SPOCK, 31, true);
	} else
		showText(TX_SPEAKER_SPOCK, 25, true);
}

void Room::veng0UseSTricorderOnChair() {
	if (!_awayMission->veng.scannedCaptainsChair) {
		_awayMission->veng.scannedCaptainsChair = true;
		_awayMission->veng.field6b = true;

		spockScan(DIR_S, 32, false, true);
		showText(TX_SPEAKER_PATTERSON, 35, true);
		showText(TX_SPEAKER_SPOCK, 33, true);
		showText(TX_SPEAKER_KIRK,   4, true);
	} else
		showText(TX_SPEAKER_SPOCK, 26, true);
}

void Room::veng0UseSTricorderAnywhere() {
	if (!_awayMission->veng.impulseEnginesOn)
		spockScan(DIR_S, 27, false, true);
}

void Room::veng0UseMccoyOnBeam() {
	showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::veng0UseSTricorderOnBeam() {
	spockScan(DIR_W, 34, false, true);
	showText(TX_SPEAKER_MCCOY, 21, true);
}

void Room::veng0UseRedshirtOnBeam() {
	showText(TX_SPEAKER_KIJE, 37, true);
}

void Room::veng0UseSpockOnConsole() {
	showText(TX_SPEAKER_SPOCK, 24, true);
}

void Room::veng0UseRedshirtOnConsole() {
	showText(TX_SPEAKER_KIJE, 38, true);
}

void Room::veng0UsePhaserOnBeam() {
	showText(TX_SPEAKER_SPOCK, 13, true);
}

void Room::veng0UseMccoyOnDeadCrewman() {
	mccoyScan(DIR_S, 16, true);
}

void Room::veng0UseSawOnBeam() {
	if (_awayMission->veng.beamState == 1 || _awayMission->veng.beamState == 2)
		showDescription(8, true);
	else {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x24, 0xc2, &Room::veng0KirkReachedSaw);
	}
}

void Room::veng0KirkReachedSaw() {
	loadActorAnimC(OBJECT_KIRK, "kcutw", -1, -1, &Room::veng0DoneCuttingBeam);
	_awayMission->timers[0] = 8;
}

void Room::veng0Timer0Expired() {
	playVoc("MUR3E2");
	loadActorAnim2(OBJECT_9, "s7r0s1", 0, 0xc7);
	loadActorAnim2(OBJECT_10, "s7r0b1", 0, 0xc7);
	playSoundEffectIndex(kSfxPhaser);
}

void Room::veng0DoneCuttingBeam() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_9);
	loadActorStandAnim(OBJECT_10);

	showDescription(7, true);

	_awayMission->veng.beamState = 1;
	veng0GetBeam();
}

void Room::veng0GetBeam() {
	if (_awayMission->veng.beamState == 1) {
		showDescription(19, true);
		_awayMission->veng.beamState = 2;
		giveItem(OBJECT_IBEAM);
		loadActorAnim2(OBJECT_BEAM, "s7r0bf", 0x0e, 0x9a);
	} else
		showDescription(9, true);
}

void Room::veng0TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::veng0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::veng0TalkToSpock() {
	if (_awayMission->veng.scannedComputerBank && _awayMission->veng.scannedCaptainsChair)
		showText(TX_SPEAKER_SPOCK, 12, true);
	else
		showText(TX_SPEAKER_SPOCK, 23, true);
}

void Room::veng0TalkToRedshirt() {
	showText(TX_SPEAKER_KIJE, 36, true);
}

void Room::veng0LookAtPatterson() {
	showDescription(4, true);
}

void Room::veng0LookAtDeadCrewman2() {
	showDescription(17, true);
}

void Room::veng0LookAtComputerBank() {
	showDescription(5, true);
}

void Room::veng0LookAtChair() {
	showDescription(15, true);
}

void Room::veng0LookAtKirk() {
	showDescription(1, true);
}

void Room::veng0LookAtSpock() {
	showDescription(6, true);
}

void Room::veng0LookAtMccoy() {
	showDescription(2, true);
}

void Room::veng0LookAtRedshirt() {
	showDescription(3, true);
}

void Room::veng0LookAtViewscreen() {
	showDescription(10, true);
}

void Room::veng0LookAtHelmConsole() {
	showDescription(11, true);
}

void Room::veng0LookAtNavConsole() {
	showDescription(12, true);
}

void Room::veng0LookAtBeam() {
	if (_awayMission->veng.beamState == 1)
		showDescription(13, true);
	else
		showDescription(0, true);
}

void Room::veng0LookAnywhere() {
	showDescription(18, true);
}

void Room::veng0LookAtDeadCrewman1() {
	showDescription(16, true);
}

void Room::veng0UseMccoyOnLivingCrewman() {
	if (!_awayMission->veng.bridgeCrewmanDead) {
		_awayMission->disableInput = true;
		_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;
		walkCrewmanC(OBJECT_MCCOY, 0xdc, 0xa0, &Room::veng0MccoyReachedCrewman);
	} else
		showText(TX_SPEAKER_MCCOY, 16, true);
}

void Room::veng0MccoyReachedCrewman() {
	_awayMission->veng.bridgeCrewmanDead = true;
	_awayMission->veng.field68 = true;

	mccoyScan(DIR_E, 22, false, true);
	showText(TX_SPEAKER_KIRK, 3, true);
	loadActorAnimC(OBJECT_MCCOY, "mscane", -1, -1, &Room::veng0MccoyScannedCrewman);
}

void Room::veng0MccoyScannedCrewman() {
	showText(TX_SPEAKER_MCCOY, 19, true);
	_awayMission->disableInput = false;
}

}
