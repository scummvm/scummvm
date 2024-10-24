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

extern const RoomAction mudd2ActionList[] = {
	{ {ACTION_WALK, 0x21, 0, 0, 0}, &Room::mudd2WalkToNorthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0}, &Room::mudd2TouchedHotspot0 },
	{ {ACTION_WALK, 0x22, 0, 0, 0}, &Room::mudd2WalkToSouthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0, 0}, &Room::mudd2TouchedHotspot1 },
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::mudd2Tick1 },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::mudd2Timer1Expired },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,   0, 0}, &Room::mudd2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_SPOCK,    0x20, 0, 0}, &Room::mudd2UseSpockOnCapsules },
	{ {ACTION_GET, 0x20, 0, 0, 0}, &Room::mudd2GetCapsules },
	{ {ACTION_FINISHED_WALKING,   12, 0, 0, 0}, &Room::mudd2MccoyReachedCapsules },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0, 0}, &Room::mudd2MccoyPickedUpCapsules },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::mudd2UseCommunicator },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0, 0}, &Room::muddaUseDegrimer },
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0, 0}, &Room::muddaFiredAlienDevice },

	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::mudd2LookAtCapsules },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x20, 0, 0}, &Room::mudd2UseMTricorderOnCapsules },
	{ {ACTION_USE, OBJECT_ICAPSULE, 0x23, 0, 0}, &Room::mudd2UseCapsuleOnControlPanel },
	{ {ACTION_FINISHED_WALKING,   15, 0, 0, 0},  &Room::mudd2MccoyReachedControlPanel },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0, 0},  &Room::mudd2MccoyPutCapsuleInControlPanel },
	{ {ACTION_USE, OBJECT_KIRK, 0x24, 0, 0},     &Room::mudd2UseKirkOnBed },
	{ {ACTION_USE, OBJECT_KIRK, 0x25, 0, 0},     &Room::mudd2UseKirkOnBed },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0},     &Room::mudd2KirkReachedBed },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0},   &Room::mudd2MuddNoticedKirk },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0},   &Room::mudd2MuddDroppedCapsule },
	{ {ACTION_USE, OBJECT_IPHASERS, 8, 0, 0},    &Room::mudd2UsePhaserOnMudd },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0, 0},    &Room::mudd2UsePhaserOnMudd },

	{ {ACTION_USE, OBJECT_SPOCK, 8, 0, 0},       &Room::mudd2UseSpockOnMudd },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0},     &Room::mudd2SpockReachedMudd },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0},   &Room::mudd2SpockPinchedMudd },
	{ {ACTION_USE, OBJECT_KIRK, 8, 0, 0},        &Room::mudd2UseKirkOnMudd },

	{ {ACTION_USE, OBJECT_REDSHIRT, 8, 0, 0},    &Room::mudd2UseRedshirtOnMudd },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0, 0},    &Room::mudd2RedshirtReachedMudd },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0},        &Room::mudd2Timer2Expired },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0, 0},  &Room::mudd2MuddFinishedPushingRedshirt },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0, 0},  &Room::mudd2RedshirtPushedAway },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0, 0},    &Room::mudd2UseMTricorderOnMudd },
	{ {ACTION_USE, OBJECT_IMEDKIT, 8, 0, 0},     &Room::mudd2UseMedkitOnMudd },
	{ {ACTION_USE, OBJECT_MCCOY, 8, 0, 0},       &Room::mudd2UseMedkitOnMudd },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0, 0},    &Room::mudd2MccoyReachedMudd },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0, 0},  &Room::mudd2MccoyCuredMudd },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0, 0}, &Room::mudd2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0, 0}, &Room::mudd2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0, 0}, &Room::mudd2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd2LookAtRedshirt },
	{ {ACTION_LOOK, 8,    0, 0, 0}, &Room::mudd2LookAtMudd },
	{ {ACTION_LOOK, 0x23, 0, 0, 0}, &Room::mudd2LookAtControlPanel },
	{ {ACTION_LOOK, 0x25, 0, 0, 0}, &Room::mudd2LookAtBed },
	{ {ACTION_LOOK, 0x24, 0, 0, 0}, &Room::mudd2LookAtBed },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0, 0}, &Room::mudd2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0, 0}, &Room::mudd2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0, 0}, &Room::mudd2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::mudd2TalkToRedshirt },
	{ {ACTION_TALK, 8,               0, 0, 0}, &Room::mudd2TalkToMudd },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum mudd2TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_UHURA, TX_SPEAKER_BUCHERT,
	TX_SPEAKER_MUDD,
	TX_MUD2_001, TX_MUD2_002, TX_MUD2_003, TX_MUD2_004, TX_MUD2_005,
	TX_MUD2_006, TX_MUD2_007, TX_MUD2_008, TX_MUD2_009, TX_MUD2_010,
	TX_MUD2_011, TX_MUD2_012, TX_MUD2_013, TX_MUD2_014, TX_MUD2_015,
	TX_MUD2_016, TX_MUD2_018, TX_MUD2_019, TX_MUD2_020, TX_MUD2_021,
	TX_MUD2_022, TX_MUD2_023, TX_MUD2_024, TX_MUD2_025, TX_MUD2_026,
	TX_MUD2_027, TX_MUD2_028, TX_MUD2_029, TX_MUD2_030, TX_MUD2_031,
	TX_MUD2_032, TX_MUD2_033, TX_MUD2_034, TX_MUD2_035, TX_MUD2_036,
	TX_MUD2_037, TX_MUD2_038, TX_MUD2_039, TX_MUD2_040, TX_MUD2_042,
	TX_MUD2_043, TX_MUD2_044, TX_MUD2_045, TX_MUD2_046, TX_MUD2_047,
	TX_MUD2_048, TX_MUD2_049, TX_MUD2_050, TX_MUD2_051, TX_MUD2_052,
	TX_MUD2_053, TX_MUD2_054, TX_MUD2_055, TX_MUD2N000, TX_MUD2N002,
	TX_MUD2N003, TX_MUD2N004, TX_MUD2N005, TX_MUD2N006, TX_MUD2N007,
	TX_MUD2N008, TX_MUD2N011, TX_STATICU1
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets mudd2TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 333, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 344, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 354, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 376, 0, 0, 0 },
	{ TX_SPEAKER_BUCHERT, 364, 0, 0, 0 },
	{ TX_SPEAKER_MUDD, 386, 0, 0, 0 },
	{ TX_MUD2_001, 5240, 0, 0, 0 },
	{ TX_MUD2_002, 1977, 0, 0, 0 },
	{ TX_MUD2_003, 8694, 0, 0, 0 },
	{ TX_MUD2_004, 2392, 0, 0, 0 },
	{ TX_MUD2_005, 8288, 0, 0, 0 },
	{ TX_MUD2_006, 10058, 0, 0, 0 },
	{ TX_MUD2_007, 9467, 0, 0, 0 },
	{ TX_MUD2_008, 9763, 0, 0, 0 },
	{ TX_MUD2_009, 1719, 0, 0, 0 },
	{ TX_MUD2_010, 9121, 0, 0, 0 },
	{ TX_MUD2_011, 9266, 0, 0, 0 },
	{ TX_MUD2_012, 5126, 0, 0, 0 },
	{ TX_MUD2_013, 5690, 0, 0, 0 },
	{ TX_MUD2_014, 2729, 0, 0, 0 },
	{ TX_MUD2_015, 5973, 0, 0, 0 },
	{ TX_MUD2_016, 5875, 0, 0, 0 },
	{ TX_MUD2_018, 1307, 0, 0, 0 },
	{ TX_MUD2_019, 2458, 0, 0, 0 },
	{ TX_MUD2_020, 2937, 0, 0, 0 },
	{ TX_MUD2_021, 6114, 0, 0, 0 },
	{ TX_MUD2_022, 3515, 0, 0, 0 },
	{ TX_MUD2_023, 10226, 0, 0, 0 },
	{ TX_MUD2_024, 10113, 0, 0, 0 },
	{ TX_MUD2_025, 9366, 0, 0, 0 },
	{ TX_MUD2_026, 4089, 0, 0, 0 },
	{ TX_MUD2_027, 8547, 0, 0, 0 },
	{ TX_MUD2_028, 9050, 0, 0, 0 },
	{ TX_MUD2_029, 3045, 0, 0, 0 },
	{ TX_MUD2_030, 2232, 0, 0, 0 },
	{ TX_MUD2_031, 4278, 0, 0, 0 },
	{ TX_MUD2_032, 3764, 0, 0, 0 },
	{ TX_MUD2_033, 6423, 0, 0, 0 },
	{ TX_MUD2_034, 3292, 0, 0, 0 },
	{ TX_MUD2_035, 8400, 0, 0, 0 },
	{ TX_MUD2_036, 8737, 0, 0, 0 },
	{ TX_MUD2_037, 1165, 0, 0, 0 },
	{ TX_MUD2_038, 8446, 0, 0, 0 },
	{ TX_MUD2_039, 3202, 0, 0, 0 },
	{ TX_MUD2_040, 9223, 0, 0, 0 },
	{ TX_MUD2_042, 4656, 0, 0, 0 },
	{ TX_MUD2_043, 4831, 0, 0, 0 },
	{ TX_MUD2_044, 3575, 0, 0, 0 },
	{ TX_MUD2_045, 6564, 0, 0, 0 },
	{ TX_MUD2_046, 8342, 0, 0, 0 },
	{ TX_MUD2_047, 9841, 0, 0, 0 },
	{ TX_MUD2_048, 8883, 0, 0, 0 },
	{ TX_MUD2_049, 3917, 0, 0, 0 },
	{ TX_MUD2_050, 4442, 0, 0, 0 },
	{ TX_MUD2_051, 4190, 0, 0, 0 },
	{ TX_MUD2_052, 5521, 0, 0, 0 },
	{ TX_MUD2_053, 4525, 0, 0, 0 },
	{ TX_MUD2_054, 9535, 0, 0, 0 },
	{ TX_MUD2_055, 8611, 0, 0, 0 },
	{ TX_MUD2N000, 7954, 0, 0, 0 },
	{ TX_MUD2N002, 8128, 0, 0, 0 },
	{ TX_MUD2N003, 7795, 0, 0, 0 },
	{ TX_MUD2N004, 8028, 0, 0, 0 },
	{ TX_MUD2N005, 7449, 0, 0, 0 },
	{ TX_MUD2N006, 7899, 0, 0, 0 },
	{ TX_MUD2N007, 10590, 0, 0, 0 },
	{ TX_MUD2N008, 2122, 0, 0, 0 },
	{ TX_MUD2N011, 10479, 0, 0, 0 },
	{ TX_STATICU1, 1756, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText mudd2Texts[] = {
	  { -1, Common::UNK_LANG, "" }
};

void Room::mudd2WalkToNorthDoor() {
	_roomVar.mudd.walkingToDoor = true;
	_awayMission->disableInput = true;
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
	_awayMission->disableInput = true;
	walkCrewman(OBJECT_KIRK, 0x46, 0x6f);
}

void Room::mudd2TouchedHotspot1() {
	if (_roomVar.mudd.walkingToDoor) {
		playVoc("SMADOOR3");
	}
}

void Room::mudd2Tick1() {
	playVoc("MUD2LOOP");
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	_awayMission->mudd.muddCurrentlyInsane = false;

	if (_awayMission->mudd.muddInsanityState == 0) { // First time entering room
		_awayMission->mudd.muddInsanityState = 1;
	} else if (_awayMission->mudd.muddInsanityState == 2) { // Currently insane
		_awayMission->mudd.muddCurrentlyInsane = true;
		if (!_awayMission->mudd.muddUnconscious) {
			_awayMission->mudd.muddUnconscious = false;
			loadActorAnim2(OBJECT_MUDD, "s4sbhn", 0x9f, 0xbf);
			loadActorAnim2(OBJECT_CAPSULE, "s4sbvp", 0x93, 0xc3);
		} else {
			loadActorAnim2(OBJECT_MUDD, "s4sbob", 0x9f, 0xba);
		}
	} else if (_awayMission->mudd.muddUnavailable) {
		_awayMission->mudd.muddInsanityState = 1;
	} else if (_awayMission->mudd.muddInsanityState == 1) { // Second time entering room, start cutscene
		playMidiMusicTracks(MIDITRACK_3);
		loadActorAnim2(OBJECT_MUDD, "s4sbhw", 0x99, 0xbf);
		_awayMission->disableInput = 2;
		_awayMission->mudd.muddInhaledGas = true;
		_awayMission->timers[1] = 70;
		_awayMission->mudd.muddInsanityState = 2;
		_awayMission->mudd.muddUnavailable = true;
	}
}

void Room::mudd2Timer1Expired() {
	loadActorAnimC(OBJECT_MUDD, "s4sbmt", 0xa0, 0xbf, &Room::mudd2MuddNoticedKirk);
}

void Room::mudd2UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_MUD2_037, false);
}

void Room::mudd2UseSpockOnCapsules() {
	showText(TX_SPEAKER_MCCOY, TX_MUD2_018);
}

void Room::mudd2GetCapsules() {
	if (!_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		if (!_awayMission->mudd.translatedAlienLanguage)
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
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xfe, 0xb2);
}

void Room::mudd2UseCommunicator() {
	showText(TX_SPEAKER_KIRK,  TX_MUD2_009);
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd2LookAtCapsules() {
	showDescription(TX_MUD2N008);
}

void Room::mudd2UseMTricorderOnCapsules() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_MUD2_030, false);
	showText(TX_SPEAKER_KIRK,  TX_MUD2_004);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_019);
}

void Room::mudd2UseCapsuleOnControlPanel() {
	if (!_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		walkCrewmanC(OBJECT_MCCOY, 0x9f, 0xbf, &Room::mudd2MccoyReachedControlPanel);
	else
		showText(TX_SPEAKER_MCCOY, TX_MUD2_023);
}

void Room::mudd2MccoyReachedControlPanel() {
	if (_awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_MCCOY, "musehn", -1, -1, &Room::mudd2MccoyPutCapsuleInControlPanel);
	else // NOTE: Unused, since you can't get capsules without translating the language first
		showText(TX_SPEAKER_MCCOY, TX_MUD2_014);
}

void Room::mudd2MccoyPutCapsuleInControlPanel() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorStandAnim(OBJECT_MCCOY);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_020);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_029);
	showText(TX_SPEAKER_SPOCK, TX_MUD2_039);
	if (_awayMission->mudd.muddUnconscious)
		showText(TX_SPEAKER_MCCOY, TX_MUD2_034);

	_awayMission->mudd.putCapsuleInMedicalMachine = true;

	loseItem(OBJECT_ICAPSULE);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_W;
	walkCrewman(OBJECT_MCCOY, 0xfe, 0xb2);
}

void Room::mudd2UseKirkOnBed() {
	if (!_awayMission->mudd.putCapsuleInMedicalMachine && !_awayMission->mudd.translatedAlienLanguage) {
		walkCrewmanC(OBJECT_KIRK, 0xd7, 0xbd, &Room::mudd2KirkReachedBed);
	} else if (_awayMission->mudd.translatedAlienLanguage) {
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
	playMidiMusicTracks(MIDITRACK_0);
}

void Room::mudd2MuddDroppedCapsule() {
	loadActorAnim2(OBJECT_MUDD, "s4sbhn", 0x9f, 0xbf, 3); // NOTE: no callback from this
	loadActorAnim2(OBJECT_CAPSULE, "s4sbvp", 0x93, 0xc3);
	_awayMission->mudd.muddCurrentlyInsane = true;

	showText(TX_SPEAKER_MCCOY, TX_MUD2_032);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_049);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_026);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_051);
	showText(TX_SPEAKER_MCCOY, TX_MUD2_031);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_050);

	_awayMission->disableInput = false;
}


void Room::mudd2UsePhaserOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_053);
		showText(TX_SPEAKER_MUDD,    TX_MUD2_042);
		showText(TX_SPEAKER_MUDD,    TX_MUD2_043);
	}
}


// Spock neck-pinches Mudd
void Room::mudd2UseSpockOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		_awayMission->disableInput = true;
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

	_awayMission->disableInput = false;
	_awayMission->mudd.muddUnconscious = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0x118, 0xc4);
}

void Room::mudd2UseKirkOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_KIRK, TX_MUD2_001);
		mudd2UseSpockOnMudd();
	}
}

void Room::mudd2UseRedshirtOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_REDSHIRT, 0xc3, 0xbe, &Room::mudd2RedshirtReachedMudd);
	}
}

void Room::mudd2RedshirtReachedMudd() {
	_awayMission->timers[2] = 8;
	loadActorAnimC(OBJECT_REDSHIRT, "s4sbrh", -1, -1, &Room::mudd2RedshirtPushedAway);
	playVoc("ROCKFACE");
	loadActorAnimC(OBJECT_MUDD, "s4sbhh", 0x9f, 0xbf, &Room::mudd2MuddFinishedPushingRedshirt);
}

void Room::mudd2Timer2Expired() {
}

void Room::mudd2MuddFinishedPushingRedshirt() {
	loadActorAnim2(OBJECT_MUDD, "s4sbhn");
}

void Room::mudd2RedshirtPushedAway() {
	showText(TX_SPEAKER_BUCHERT, TX_MUD2_052);
	loadActorAnim(OBJECT_REDSHIRT, "rstnds", 0xd8, 0xc3);
	_awayMission->disableInput = false;
	_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_W;
	walkCrewman(OBJECT_REDSHIRT, 0x117, 0xae);
}


// BUGFIX: This was originally "Action 0x45 on Mudd"; as far as I know, action 0x45
// doesn't exist. It's far more likely that 0x45 is supposed to correspond to
// OBJECT_IMTRICOR in a USE action.
// The function itself was also modified (ie. condition for showing text was inverted).
void Room::mudd2UseMTricorderOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious)
		mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_MUD2_013, false);
}


void Room::mudd2UseMedkitOnMudd() {
	if (!_awayMission->mudd.muddInhaledGas)
		return;
	else if (_awayMission->mudd.muddUnconscious) {
		if (!_awayMission->mudd.translatedAlienLanguage)
			showText(TX_SPEAKER_MCCOY, TX_MUD2_015);
		else if (!_awayMission->mudd.putCapsuleInMedicalMachine)
			showText(TX_SPEAKER_MCCOY, TX_MUD2_021);
		else
			walkCrewmanC(OBJECT_MCCOY, 0xde, 0xaf, &Room::mudd2MccoyReachedMudd);
	} else // Can't get to him since he's busy being crazy
		showText(TX_SPEAKER_MCCOY, TX_MUD2_016);
}

void Room::mudd2MccoyReachedMudd() {
	_awayMission->mudd.missionScore += 2;
	loadActorAnimC(OBJECT_MCCOY, "s4sbms", -1, -1, &Room::mudd2MccoyCuredMudd);
}

void Room::mudd2MccoyCuredMudd() {
	_awayMission->mudd.muddUnavailable = false;
	_awayMission->mudd.muddInsanityState = 3;
	_awayMission->mudd.muddCurrentlyInsane = false;
	_awayMission->mudd.muddInhaledGas = false;

	showText(TX_SPEAKER_MCCOY, TX_MUD2_033);
	showText(TX_SPEAKER_MUDD,  TX_MUD2_045);

	_awayMission->mudd.muddUnconscious = false;
}


void Room::mudd2LookAtKirk() {
	showDescription(TX_MUD2N003);
}

void Room::mudd2LookAtSpock() {
	showDescription(TX_MUD2N006);
}

void Room::mudd2LookAtMccoy() {
	showDescription(TX_MUD2N000);
}

void Room::mudd2LookAtRedshirt() {
	showDescription(TX_MUD2N004);
}

void Room::mudd2LookAtMudd() {
	showDescription(TX_MUD2N002);
}

void Room::mudd2LookAtControlPanel() {
	showDescription(TX_MUD2N011);
}

void Room::mudd2LookAtBed() {
	showDescription(TX_MUD2N007);
}

void Room::mudd2TalkToKirk() {
	// BUGFIX: second condition in if statement changed to "must be false" instead of
	// "must be true". (Same applies to below talk functions.)
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		showText(TX_SPEAKER_KIRK, TX_MUD2_010);
	else {
		showText(TX_SPEAKER_KIRK,  TX_MUD2_005);
		showText(TX_SPEAKER_MUDD,  TX_MUD2_046);
		showText(TX_SPEAKER_MCCOY, TX_MUD2_035);
	}
}

void Room::mudd2TalkToSpock() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_SPOCK, TX_MUD2_040);
		showText(TX_SPEAKER_KIRK,  TX_MUD2_011);
	} else {
		showText(TX_SPEAKER_SPOCK, TX_MUD2_038);
	}
}

void Room::mudd2TalkToMccoy() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_MCCOY, TX_MUD2_025);
		showText(TX_SPEAKER_KIRK,  TX_MUD2_007);
	} else {
		showText(TX_SPEAKER_MCCOY, TX_MUD2_027);
	}
}

void Room::mudd2TalkToRedshirt() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_054);
		showText(TX_SPEAKER_KIRK,    TX_MUD2_008);
	} else {
		showText(TX_SPEAKER_BUCHERT, TX_MUD2_055);
		showText(TX_SPEAKER_KIRK,    TX_MUD2_003);
		showText(TX_SPEAKER_MCCOY,   TX_MUD2_036);
	}
}

void Room::mudd2TalkToMudd() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		return;
	else if (_awayMission->mudd.muddInhaledGas) {
		showText(TX_SPEAKER_MUDD, TX_MUD2_048);
		showText(TX_SPEAKER_MCCOY, TX_MUD2_028);
	} else { // NOTE: Unused (assumes harry is in a normal state, which doesn't happen here)
		showText(TX_SPEAKER_MUDD, TX_MUD2_047);
		showText(TX_SPEAKER_KIRK, TX_MUD2_006);
	}
}

}
