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

extern const RoomAction tug0ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::tug0Tick1 },
	{ {ACTION_LOOK, 8, 0, 0, 0}, &Room::tug0LookAtEngineer },
	{ {ACTION_GET, 8, 0, 0, 0}, &Room::tug0GetEngineer },

	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::tug0LookAtControls },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::tug0LookAtControls },
	{ {ACTION_USE, OBJECT_SPOCK, 0x21, 0, 0}, &Room::tug0UseSpockOnControls },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0, 0}, &Room::tug0SpockReachedControlsToExamine },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0, 0}, &Room::tug0SpockExaminedControls },
	{ {ACTION_USE, OBJECT_IRT, 0x21, 0, 0}, &Room::tug0UseTransmogrifierWithoutBitOnControls },
	{ {ACTION_USE, OBJECT_IRTWB, 0x21, 0, 0}, &Room::tug0UseTransmogrifierWithBitOnControls },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::tug0SpockReachedControlsWithTransmogrifier },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0, 0}, &Room::tug0SpockFinishedUsingTransmogrifier },
	{ {ACTION_FINISHED_ANIMATION, 22, 0, 0, 0}, &Room::tug0TransporterScreenFullyLit },
	{ {ACTION_USE, OBJECT_IWIRSCRP, 0x21, 0, 0}, &Room::tug0UseWireScrapsOnControls },
	{ {ACTION_USE, OBJECT_IWIRING, 0x21, 0, 0}, &Room::tug0UseWireOnControls },
	{ {ACTION_FINISHED_WALKING, 8, 0, 0, 0}, &Room::tug0SpockReachedControlsWithWire },
	{ {ACTION_FINISHED_ANIMATION, 9, 0, 0, 0}, &Room::tug0SpockFinishedUsingWire },
	{ {ACTION_USE, OBJECT_IMEDKIT, 8, 0, 0}, &Room::tug0UseMedkitOnEngineer },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0}, &Room::tug0MccoyReachedEngineer },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0, 0}, &Room::tug0MccoyHealedEngineer },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0}, &Room::tug0EngineerGotUp },
	{ {ACTION_GET, 9, 0, 0, 0}, &Room::tug0GetTransmogrifier },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0}, &Room::tug0KirkReachedToolbox },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::tug0KirkGotTransmogrifier },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::tug0LookAtToolbox },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_IPWE, 0, 0}, &Room::tug0UsePhaserOnWelder },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_IPWE, 0, 0}, &Room::tug0UsePhaserOnWelder },
	{ {ACTION_USE, OBJECT_IPWF, OBJECT_IWIRSCRP, 0, 0}, &Room::tug0UseWelderOnWireScraps },
	{ {ACTION_USE, OBJECT_IPWF, OBJECT_IJNKMETL, 0, 0}, &Room::tug0UseWelderOnMetalScraps },
	{ {ACTION_USE, OBJECT_ICOMBBIT, OBJECT_IRT, 0, 0}, &Room::tug0UseCombBitOnTransmogrifier },

	{ {ACTION_USE, OBJECT_SPOCK, 11, 0, 0}, &Room::tug0UseTransporter },
	{ {ACTION_USE, OBJECT_KIRK, 0x22, 0, 0}, &Room::tug0UseTransporter },
	{ {ACTION_FINISHED_WALKING, 14, 0, 0, 0}, &Room::tug0SpockReachedControlsToTransport },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0, 0}, &Room::tug0SpockPreparedTransporter },
	{ {ACTION_FINISHED_WALKING, 20, 0, 0, 0}, &Room::tug0SpockReachedTransporter },
	{ {ACTION_FINISHED_ANIMATION, 21, 0, 0, 0}, &Room::tug0FinishedTransporting },

	{ {ACTION_USE, OBJECT_IBOMB, 0x22, 0, 0}, &Room::tug0UseBombOnTransporter },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::tug0KirkReachedTransporter },
	{ {ACTION_FINISHED_ANIMATION, 10, 0, 0, 0}, &Room::tug0KirkPlacedBomb },
	{ {ACTION_FINISHED_WALKING, 15, 0, 0, 0}, &Room::tug0SpockReachedControlsForBomb },
	{ {ACTION_FINISHED_ANIMATION, 19, 0, 0, 0}, &Room::tug0SpockBeginsBeamingBomb },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0, 0}, &Room::tug0SpockFinishesBeamingBomb },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::tug0BombExploded },

	{ {ACTION_USE, OBJECT_MCCOY, 0x21, 0, 0}, &Room::tug0UseMTricorderOnControls },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x21, 0, 0}, &Room::tug0UseMTricorderOnControls },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x21, 0, 0}, &Room::tug0UseSTricorderOnControls },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0, 0}, &Room::tug0UseMTricorderOnEngineer },
	{ {ACTION_FINISHED_WALKING, 24, 0, 0, 0}, &Room::tug0MccoyReachedEngineerToScan },
	{ {ACTION_FINISHED_ANIMATION, 25, 0, 0, 0}, &Room::tug0MccoyFinishedScanningEngineer },
	{ {ACTION_USE, OBJECT_SPOCK, 8, 0, 0}, &Room::tug0UseSTricorderOnEngineer },
	{ {ACTION_USE, OBJECT_ISTRICOR, 8, 0, 0}, &Room::tug0UseSTricorderOnEngineer },
	{ {ACTION_WALK, 0x23, 0, 0, 0}, &Room::tug0WalkToDoor },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::tug0LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::tug0LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::tug0LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::tug0LookAtRedshirt },
	{ {ACTION_LOOK, 0x22, 0, 0, 0}, &Room::tug0LookAtTransporter },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::tug0LookAtDoor },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::tug0TalkToKirk },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::tug0TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::tug0TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::tug0TalkToRedshirt },
	{ {ACTION_TALK, 8, 0, 0, 0}, &Room::tug0TalkToEngineer },
	{ {ACTION_USE, OBJECT_ICOMM, 0, 0, 0}, &Room::tug0UseCommunicator },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::tug0LookAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::tug0UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0xff, 0, 0}, &Room::tug0UseMTricorderAnywhere },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum tug0TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_CHRISTENSEN, TX_SPEAKER_SIMPSON,
	TX_SPEAKER_STARFLEET, TX_SPEAKER_SCOTT,
	TX_TUG0_001, TX_TUG0_002, TX_TUG0_003, TX_TUG0_004, TX_TUG0_005,
	TX_TUG0_006, TX_TUG0_007, TX_TUG0_008, TX_TUG0_009, TX_TUG0_010,
	TX_TUG0_011, TX_TUG0_012, TX_TUG0_013, TX_TUG0_014, TX_TUG0_015,
	TX_TUG0_016, TX_TUG0_017, TX_TUG0_018, TX_TUG0_019, TX_TUG0_020,
	TX_TUG0_021, TX_TUG0_022, TX_TUG0_023, TX_TUG0_024, TX_TUG0_025,
	TX_TUG0_026, TX_TUG0_027, TX_TUG0_028, TX_TUG0_029, TX_TUG0_030,
	TX_TUG0_031, TX_TUG0_032, TX_TUG0_033, TX_TUG0_034, TX_TUG0_035,
	TX_TUG0_036, TX_TUG0_037, TX_TUG0_038, TX_TUG0_039, TX_TUG0_040,
	TX_TUG0_041, TX_TUG0_042, TX_TUG0_043, TX_TUG0_F30, TX_TUG0_F31,
	TX_TUG0_S06, TX_TUG0_S11, TX_TUG0L006, TX_TUG0L008, TX_TUG0L009,
	TX_TUG0L010, TX_TUG0N000, TX_TUG0N001, TX_TUG0N002, TX_TUG0N003,
	TX_TUG0N004, TX_TUG0N005, TX_TUG0N006, TX_TUG0N007, TX_TUG0N008,
	TX_TUG0N009, TX_TUG0N010, TX_TUG0N011, TX_TUG0N012, TX_TUG0N013,
	TX_TUG0N014
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets tug0TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 9896, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 9907, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 9917, 0, 0, 0 },
	{ TX_SPEAKER_CHRISTENSEN, 9927, 0, 0, 0 },
	{ TX_SPEAKER_SIMPSON, 9943, 0, 0, 0 },
	{ TX_SPEAKER_STARFLEET, 9959, 0, 0, 0 },
	{ TX_SPEAKER_SCOTT, 9969, 0, 0, 0 },
	{ TX_TUG0_001, 5358, 0, 0, 0 },
	{ TX_TUG0_002, 2807, 0, 0, 0 },
	{ TX_TUG0_003, 6532, 0, 0, 0 },
	{ TX_TUG0_004, 6423, 0, 0, 0 },
	{ TX_TUG0_005, 1215, 0, 0, 0 },
	{ TX_TUG0_006, 8042, 0, 0, 0 },
	{ TX_TUG0_007, 8362, 0, 0, 0 },
	{ TX_TUG0_008, 1709, 0, 0, 0 },
	{ TX_TUG0_009, 8667, 0, 0, 0 },
	{ TX_TUG0_010, 2557, 0, 0, 0 },
	{ TX_TUG0_011, 8189, 0, 0, 0 },
	{ TX_TUG0_012, 9064, 0, 0, 0 },
	{ TX_TUG0_013, 1403, 0, 0, 0 },
	{ TX_TUG0_014, 1819, 0, 0, 0 },
	{ TX_TUG0_015, 6862, 0, 0, 0 },
	{ TX_TUG0_016, 6731, 0, 0, 0 },
	{ TX_TUG0_017, 7106, 0, 0, 0 },
	{ TX_TUG0_018, 7218, 0, 0, 0 },
	{ TX_TUG0_019,  737, 0, 0, 0 },
	{ TX_TUG0_020, 9705, 0, 0, 0 },
	{ TX_TUG0_021, 3364, 0, 0, 0 },
	{ TX_TUG0_022, 2612, 0, 0, 0 },
	{ TX_TUG0_023, 6104, 0, 0, 0 },
	{ TX_TUG0_024, 8445, 0, 0, 0 },
	{ TX_TUG0_025, 8270, 0, 0, 0 },
	{ TX_TUG0_026, 8961, 0, 0, 0 },
	{ TX_TUG0_027, 6365, 0, 0, 0 },
	{ TX_TUG0_028, 9470, 0, 0, 0 },
	{ TX_TUG0_029, 1998, 0, 0, 0 },
	{ TX_TUG0_030, 7391, 0, 0, 0 },
	{ TX_TUG0_031, 9154, 0, 0, 0 },
	{ TX_TUG0_032, 4852, 0, 0, 0 },
	{ TX_TUG0_033, 3574, 0, 0, 0 },
	{ TX_TUG0_034, 2228, 0, 0, 0 },
	{ TX_TUG0_035, 5025, 0, 0, 0 },
	{ TX_TUG0_036, 8121, 0, 0, 0 },
	{ TX_TUG0_037, 6227, 0, 0, 0 },
	{ TX_TUG0_038, 1564, 0, 0, 0 },
	{ TX_TUG0_039, 2460, 0, 0, 0 },
	{ TX_TUG0_040, 1136, 0, 0, 0 },
	{ TX_TUG0_041, 3485, 0, 0, 0 },
	{ TX_TUG0_042, 8510, 0, 0, 0 },
	{ TX_TUG0_043, 1270, 0, 0, 0 },
	{ TX_TUG0_F30, 8856, 0, 0, 0 },
	{ TX_TUG0_F31, 3874, 0, 0, 0 },
	{ TX_TUG0_S06, 6579, 0, 0, 0 },
	{ TX_TUG0_S11, 6483, 0, 0, 0 },
	{ TX_TUG0L006, 8741, 0, 0, 0 },
	{ TX_TUG0L008, 3031, 0, 0, 0 },
	{ TX_TUG0L009, 2860, 0, 0, 0 },
	{ TX_TUG0L010, 3180, 0, 0, 0 },
	{ TX_TUG0N000, 4358, 0, 0, 0 },
	{ TX_TUG0N001,  944, 0, 0, 0 },
	{ TX_TUG0N002,  581, 0, 0, 0 },
	{ TX_TUG0N003, 7518, 0, 0, 0 },
	{ TX_TUG0N004, 7787, 0, 0, 0 },
	{ TX_TUG0N005, 7694, 0, 0, 0 },
	{ TX_TUG0N006, 4643, 0, 0, 0 },
	{ TX_TUG0N007, 7594, 0, 0, 0 },
	{ TX_TUG0N008, 4556, 0, 0, 0 },
	{ TX_TUG0N009, 9325, 0, 0, 0 },
	{ TX_TUG0N010, 4706, 0, 0, 0 },
	{ TX_TUG0N011, 7938, 0, 0, 0 },
	{ TX_TUG0N012,  823, 0, 0, 0 },
	{ TX_TUG0N013, 7857, 0, 0, 0 },
	{ TX_TUG0N014,  625, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText tug0Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::tug0Tick1() {
	playVoc("TUG0LOOP");
	playMidiMusicTracks(MIDITRACK_0);

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
	playSoundEffectIndex(kSfxTricorder);
}

void Room::tug0SpockExaminedControls() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_040);
	showText(TX_SPEAKER_KIRK, TX_TUG0_005);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_043);
	showText(TX_SPEAKER_KIRK, TX_TUG0_013);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_038);
	showText(TX_SPEAKER_KIRK, TX_TUG0_008);
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

	showText(TX_SPEAKER_KIRK, TX_TUG0_010);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_022);

	// BUGFIX: check if the redshirt is dead.
	if (!_awayMission->redshirtDead) {
		showText(TX_SPEAKER_KIRK, TX_TUG0_002);

		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L009);
		showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L008);

		if (_awayMission->tug.haveBomb) {
			showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L010);
			showText(TX_SPEAKER_MCCOY, TX_TUG0_021);
			showText(TX_SPEAKER_SPOCK, TX_TUG0_041);
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
	playSoundEffectIndex(kSfxButton);
	playSoundEffectIndex(kSfxTransporterEnergize);
}

void Room::tug0SpockPreparedTransporter() {
	showText(TX_SPEAKER_KIRK, TX_TUG0_001);
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	walkCrewman(OBJECT_SPOCK, 0x9a, 0x7e, 20);
}

void Room::tug0SpockReachedTransporter() {
	playSoundEffectIndex(kSfxTransporterDematerialize);
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
	playSoundEffectIndex(kSfxButton);
	playSoundEffectIndex(kSfxTransporterEnergize);
}

void Room::tug0SpockBeginsBeamingBomb() {
	playSoundEffectIndex(kSfxTransporterDematerialize);
	loadActorAnim2(10, "bomb2", 0x5a, 0x7f, 12);
}

void Room::tug0SpockFinishesBeamingBomb() {
	_awayMission->tug.missionScore = 0;
	_awayMission->timers[0] = 64;
}

void Room::tug0BombExploded() {
	playMidiMusicTracks(MIDITRACK_2);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_023);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_037);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_027);
	showText(TX_SPEAKER_KIRK, TX_TUG0_004);
	showText(TX_SPEAKER_SCOTT, TX_TUG0_S11);
	showText(TX_SPEAKER_KIRK, TX_TUG0_003);
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
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_016);
}

void Room::tug0UseSTricorderOnControls() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 23);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_015);
}

void Room::tug0UseMTricorderOnEngineer() {
	_awayMission->disableInput = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	walkCrewman(OBJECT_MCCOY, 0x2d, 0xaf, 24);
}

void Room::tug0MccoyReachedEngineerToScan() {
	playSoundEffectIndex(kSfxTricorder);
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
	playSoundEffectIndex(kSfxTricorder);
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
	showText(TX_SPEAKER_KIRK, TX_TUG0_006);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_036);
	showText(TX_SPEAKER_KIRK, TX_TUG0_011);
}

void Room::tug0TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TUG0_025);
	showText(TX_SPEAKER_KIRK, TX_TUG0_007);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_024);
}

void Room::tug0TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TUG0_042);
	showText(TX_SPEAKER_KIRK, TX_TUG0_009);
}

void Room::tug0TalkToRedshirt() {
	showText(TX_SPEAKER_CHRISTENSEN, TX_TUG0L006);
}

void Room::tug0TalkToEngineer() {
	if (_awayMission->tug.engineerConscious) {
		showText(TX_SPEAKER_SIMPSON, TX_TUG0_F30);
		showText(TX_SPEAKER_MCCOY, TX_TUG0_026);
		showText(TX_SPEAKER_KIRK, TX_TUG0_012);
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
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_SPOCK, TX_TUG0_028);
}

void Room::tug0UseMTricorderAnywhere() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, TX_TUG0_020); // BUG: typo
}

}
