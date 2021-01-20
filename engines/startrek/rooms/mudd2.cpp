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

extern const RoomAction mudd2ActionList[] = {
	{ {ACTION_WALK, 0x21, 0, 0}, &Room::mudd2WalkToNorthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0}, &Room::mudd2TouchedHotspot0 },
	{ {ACTION_WALK, 0x22, 0, 0}, &Room::mudd2WalkToSouthDoor },
	{ {ACTION_TOUCHED_HOTSPOT, 1, 0, 0}, &Room::mudd2TouchedHotspot1 },
	{ {ACTION_TICK, 1, 0, 0}, &Room::mudd2Tick1 },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::mudd2Timer1Expired },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,   0}, &Room::mudd2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_SPOCK,    0x20, 0}, &Room::mudd2UseSpockOnCapsules },
	{ {ACTION_GET, 0x20, 0, 0}, &Room::mudd2GetCapsules },
	{ {ACTION_FINISHED_WALKING,   12, 0, 0}, &Room::mudd2MccoyReachedCapsules },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0}, &Room::mudd2MccoyPickedUpCapsules },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::mudd2UseCommunicator },

	// Common code (next 4 lines)
	{ {ACTION_USE, OBJECT_IDEGRIME, 0xff, 0}, &Room::muddaUseDegrimer },
	{ {ACTION_USE, OBJECT_ILENSES, OBJECT_IDEGRIME, 0}, &Room::muddaUseLenseOnDegrimer },
	{ {ACTION_USE, OBJECT_IALIENDV, 0xff, 0}, &Room::muddaUseAlienDevice },
	{ {ACTION_FINISHED_ANIMATION, 18, 0, 0}, &Room::muddaFiredAlienDevice },

	{ {ACTION_LOOK, 0x20, 0, 0}, &Room::mudd2LookAtCapsules },
	{ {ACTION_USE, OBJECT_IMTRICOR, 0x20, 0}, &Room::mudd2UseMTricorderOnCapsules },
	{ {ACTION_USE, OBJECT_ICAPSULE, 0x23, 0}, &Room::mudd2UseCapsuleOnControlPanel },
	{ {ACTION_FINISHED_WALKING,   15, 0, 0},  &Room::mudd2MccoyReachedControlPanel },
	{ {ACTION_FINISHED_ANIMATION, 15, 0, 0},  &Room::mudd2MccoyPutCapsuleInControlPanel },
	{ {ACTION_USE, OBJECT_KIRK, 0x24, 0},     &Room::mudd2UseKirkOnBed },
	{ {ACTION_USE, OBJECT_KIRK, 0x25, 0},     &Room::mudd2UseKirkOnBed },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0},     &Room::mudd2KirkReachedBed },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0},   &Room::mudd2MuddNoticedKirk },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0},   &Room::mudd2MuddDroppedCapsule },
	{ {ACTION_USE, OBJECT_IPHASERS, 8, 0},    &Room::mudd2UsePhaserOnMudd },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0},    &Room::mudd2UsePhaserOnMudd },

	{ {ACTION_USE, OBJECT_SPOCK, 8, 0},       &Room::mudd2UseSpockOnMudd },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0},     &Room::mudd2SpockReachedMudd },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0},   &Room::mudd2SpockPinchedMudd },
	{ {ACTION_USE, OBJECT_KIRK, 8, 0},        &Room::mudd2UseKirkOnMudd },

	{ {ACTION_USE, OBJECT_REDSHIRT, 8, 0},    &Room::mudd2UseRedshirtOnMudd },
	{ {ACTION_FINISHED_WALKING, 16, 0, 0},    &Room::mudd2RedshirtReachedMudd },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0},        &Room::mudd2Timer2Expired },
	{ {ACTION_FINISHED_ANIMATION, 17, 0, 0},  &Room::mudd2MuddFinishedPushingRedshirt },
	{ {ACTION_FINISHED_ANIMATION, 16, 0, 0},  &Room::mudd2RedshirtPushedAway },
	{ {ACTION_USE, OBJECT_IMTRICOR, 8, 0},    &Room::mudd2UseMTricorderOnMudd },
	{ {ACTION_USE, OBJECT_IMEDKIT, 8, 0},     &Room::mudd2UseMedkitOnMudd },
	{ {ACTION_USE, OBJECT_MCCOY, 8, 0},       &Room::mudd2UseMedkitOnMudd },
	{ {ACTION_FINISHED_WALKING, 10, 0, 0},    &Room::mudd2MccoyReachedMudd },
	{ {ACTION_FINISHED_ANIMATION, 11, 0, 0},  &Room::mudd2MccoyCuredMudd },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::mudd2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::mudd2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::mudd2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd2LookAtRedshirt },
	{ {ACTION_LOOK, 8,    0, 0}, &Room::mudd2LookAtMudd },
	{ {ACTION_LOOK, 0x23, 0, 0}, &Room::mudd2LookAtControlPanel },
	{ {ACTION_LOOK, 0x25, 0, 0}, &Room::mudd2LookAtBed },
	{ {ACTION_LOOK, 0x24, 0, 0}, &Room::mudd2LookAtBed },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::mudd2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::mudd2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::mudd2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::mudd2TalkToRedshirt },
	{ {ACTION_TALK, 8,               0, 0}, &Room::mudd2TalkToMudd },

	// Common code (countdown for losing atmosphere when life support malfunctioning)
	{ {ACTION_TICK, 0xff, 0xff, 0xff},           &Room::muddaTick },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
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
		playMidiMusicTracks(3);
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
	spockScan(DIR_S, 37, false, true);
}

void Room::mudd2UseSpockOnCapsules() {
	showText(TX_SPEAKER_MCCOY, 18, true);
}

void Room::mudd2GetCapsules() {
	if (!_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		if (!_awayMission->mudd.translatedAlienLanguage)
			showText(TX_SPEAKER_MCCOY, 24, true);
		else
			walkCrewmanC(OBJECT_MCCOY, 0x9f, 0xbf, &Room::mudd2MccoyReachedCapsules);
	} else {
		showText(TX_SPEAKER_MCCOY, 23, true);
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
	showText(TX_SPEAKER_KIRK,  TX_MUD4_018);	// originally TX_MUD4_019, this matches the audio
	showText(TX_SPEAKER_UHURA, TX_STATICU1);
}

void Room::mudd2LookAtCapsules() {
	showDescription(8, true);
}

void Room::mudd2UseMTricorderOnCapsules() {
	mccoyScan(DIR_W, 30, false, true);
	showText(TX_SPEAKER_KIRK,  4, true);
	showText(TX_SPEAKER_MCCOY, 19, true);
}

void Room::mudd2UseCapsuleOnControlPanel() {
	if (!_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		walkCrewmanC(OBJECT_MCCOY, 0x9f, 0xbf, &Room::mudd2MccoyReachedControlPanel);
	else
		showText(TX_SPEAKER_MCCOY, 23, true);
}

void Room::mudd2MccoyReachedControlPanel() {
	if (_awayMission->mudd.translatedAlienLanguage)
		loadActorAnimC(OBJECT_MCCOY, "musehn", -1, -1, &Room::mudd2MccoyPutCapsuleInControlPanel);
	else // NOTE: Unused, since you can't get capsules without translating the language first
		showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::mudd2MccoyPutCapsuleInControlPanel() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
	loadActorStandAnim(OBJECT_MCCOY);
	showText(TX_SPEAKER_MCCOY, 20, true);
	showText(TX_SPEAKER_MCCOY, 29, true);
	showText(TX_SPEAKER_SPOCK, 39, true);
	if (_awayMission->mudd.muddUnconscious)
		showText(TX_SPEAKER_MCCOY, 34, true);

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
	showText(TX_SPEAKER_MCCOY, 22, true);
}


void Room::mudd2MuddNoticedKirk() {
	showText(TX_SPEAKER_MUDD, 44, true);
	playVoc("BATTYGAS");
	loadActorAnimC(OBJECT_MUDD, "s4sbhb", 0x9f, 0xbf, &Room::mudd2MuddDroppedCapsule); // Drops the capsule
	playMidiMusicTracks(0);
}

void Room::mudd2MuddDroppedCapsule() {
	loadActorAnim2(OBJECT_MUDD, "s4sbhn", 0x9f, 0xbf, 3); // NOTE: no callback from this
	loadActorAnim2(OBJECT_CAPSULE, "s4sbvp", 0x93, 0xc3);
	_awayMission->mudd.muddCurrentlyInsane = true;

	showText(TX_SPEAKER_MCCOY, 32, true);
	showText(TX_SPEAKER_MUDD,  49, true);
	showText(TX_SPEAKER_MCCOY, 26, true);
	showText(TX_SPEAKER_MUDD,  51, true);
	showText(TX_SPEAKER_MCCOY, 31, true);
	showText(TX_SPEAKER_MUDD,  50, true);

	_awayMission->disableInput = false;
}


void Room::mudd2UsePhaserOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, 53, true);
		showText(TX_SPEAKER_MUDD,    42, true);
		showText(TX_SPEAKER_MUDD,    43, true);
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

	showText(TX_SPEAKER_SPOCK, 12, true);

	_awayMission->disableInput = false;
	_awayMission->mudd.muddUnconscious = true;
	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_W;
	walkCrewman(OBJECT_SPOCK, 0x118, 0xc4);
}

void Room::mudd2UseKirkOnMudd() {
	if (_awayMission->mudd.muddInhaledGas && !_awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_KIRK, 1, true);
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
	showText(TX_SPEAKER_BUCHERT, 52, true);
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
		mccoyScan(DIR_W, 13, false, true);
}


void Room::mudd2UseMedkitOnMudd() {
	if (!_awayMission->mudd.muddInhaledGas)
		return;
	else if (_awayMission->mudd.muddUnconscious) {
		if (!_awayMission->mudd.translatedAlienLanguage)
			showText(TX_SPEAKER_MCCOY, 15, true);
		else if (!_awayMission->mudd.putCapsuleInMedicalMachine)
			showText(TX_SPEAKER_MCCOY, 21, true);
		else
			walkCrewmanC(OBJECT_MCCOY, 0xde, 0xaf, &Room::mudd2MccoyReachedMudd);
	} else // Can't get to him since he's busy being crazy
		showText(TX_SPEAKER_MCCOY, 16, true);
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

	showText(TX_SPEAKER_MCCOY, 33, true);
	showText(TX_SPEAKER_MUDD,  45, true);

	_awayMission->mudd.muddUnconscious = false;
}


void Room::mudd2LookAtKirk() {
	showDescription(3, true);
}

void Room::mudd2LookAtSpock() {
	showDescription(6, true);
}

void Room::mudd2LookAtMccoy() {
	showDescription(0, true);
}

void Room::mudd2LookAtRedshirt() {
	showDescription(4, true);
}

void Room::mudd2LookAtMudd() {
	showDescription(2, true);
}

void Room::mudd2LookAtControlPanel() {
	showDescription(11, true);
}

void Room::mudd2LookAtBed() {
	showDescription(7, true);
}

void Room::mudd2TalkToKirk() {
	// BUGFIX: second condition in if statement changed to "must be false" instead of
	// "must be true". (Same applies to below talk functions.)
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		showText(TX_SPEAKER_KIRK, 10, true);
	else {
		showText(TX_SPEAKER_KIRK,   5, true);
		showText(TX_SPEAKER_MUDD,  46, true);
		showText(TX_SPEAKER_MCCOY, 35, true);
	}
}

void Room::mudd2TalkToSpock() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_SPOCK, 40, true);
		showText(TX_SPEAKER_KIRK,  11, true);
	} else {
		showText(TX_SPEAKER_SPOCK, 38, true);
	}
}

void Room::mudd2TalkToMccoy() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_MCCOY, 25, true);
		showText(TX_SPEAKER_KIRK,   7, true);
	} else {
		showText(TX_SPEAKER_MCCOY, 27, true);
	}
}

void Room::mudd2TalkToRedshirt() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious) {
		showText(TX_SPEAKER_BUCHERT, 54, true);
		showText(TX_SPEAKER_KIRK,     8, true);
	} else {
		showText(TX_SPEAKER_BUCHERT, 55, true);
		showText(TX_SPEAKER_KIRK,     3, true);
		showText(TX_SPEAKER_MCCOY,   36, true);
	}
}

void Room::mudd2TalkToMudd() {
	if (!_awayMission->mudd.muddCurrentlyInsane || !_awayMission->mudd.muddInhaledGas || _awayMission->mudd.muddUnconscious)
		return;
	else if (_awayMission->mudd.muddInhaledGas) {
		showText(TX_SPEAKER_MUDD, 48, true);
		showText(TX_SPEAKER_MCCOY, 28, true);
	} else { // NOTE: Unused (assumes harry is in a normal state, which doesn't happen here)
		showText(TX_SPEAKER_MUDD, 47, true);
		showText(TX_SPEAKER_KIRK,  6, true);
	}
}

}
