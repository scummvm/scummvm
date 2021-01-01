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

#define OBJECT_GLOB 8
#define OBJECT_SPLIT_GLOB_1 9
#define OBJECT_SPLIT_GLOB_2 10
#define OBJECT_11 11

#define HOTSPOT_INSIGNIA 0x20 // This doesn't seem to be visible anywhere
#define HOTSPOT_WALL 0x21     // Same here
#define HOTSPOT_DOOR 0x22


#define GLOB_X 0x46
#define GLOB_Y 0xaf

#define SPLIT_GLOB_1_X 0x20
#define SPLIT_GLOB_1_Y 0xaf

#define SPLIT_GLOB_2_X 0x69
#define SPLIT_GLOB_2_Y 0xaf

namespace StarTrek {

// This room has a good deal of unused code, relating to the "split" globs (automatons).
// Apparently, it would originally have been possible to interact with the split globs,
// instead of having them kill the crew right away. This doesn't seem to have quite been
// finished, though.

extern const RoomAction trial2ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0},  &Room::trial2Tick1 },
	{ {ACTION_TICK, 60, 0, 0}, &Room::trial2Tick60 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0},     &Room::trial2TouchedHotspot0 },
	{ {ACTION_LOOK, OBJECT_KIRK,      0, 0}, &Room::trial2LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,     0, 0}, &Room::trial2LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,     0, 0}, &Room::trial2LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT,  0, 0}, &Room::trial2LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_INSIGNIA, 0, 0}, &Room::trial2LookAtInsignia },
	{ {ACTION_LOOK, OBJECT_GLOB,         0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, OBJECT_SPLIT_GLOB_1, 0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, OBJECT_SPLIT_GLOB_2, 0, 0}, &Room::trial2LookAtGlob },
	{ {ACTION_LOOK, HOTSPOT_WALL, 0, 0}, &Room::trial2LookAtWall },
	{ {ACTION_LOOK, HOTSPOT_DOOR, 0, 0}, &Room::trial2LookAtDoor },

	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::trial2TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::trial2TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::trial2TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::trial2TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_GLOB,         0, 0}, &Room::trial2TalkToGlob },
	{ {ACTION_TALK, OBJECT_SPLIT_GLOB_1, 0, 0}, &Room::trial2TalkToGlob },
	{ {ACTION_TALK, OBJECT_SPLIT_GLOB_2, 0, 0}, &Room::trial2TalkToGlob },

	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_WALL, 0}, &Room::trial2UsePhaserOnWall },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_WALL, 0}, &Room::trial2UsePhaserOnWall },

	{ {ACTION_DONE_WALK, 7,  0, 0},                  &Room::trial2ReachedPositionToShootGlob },
	{ {ACTION_DONE_ANIM, 10, 0, 0},                  &Room::trial2DrewPhaserToShootGlob },
	{ {ACTION_DONE_ANIM, 4, 0, 0},                   &Room::trial2GlobDoneExploding },
	{ {ACTION_DONE_ANIM, 1, 0, 0},                   &Room::trial2GlobDoneSplitting },
	{ {ACTION_DONE_ANIM, 19, 0, 0},                  &Room::trial2KirkDied },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_GLOB, 0}, &Room::trial2UseStunPhaserOnGlob },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_GLOB, 0}, &Room::trial2UseKillPhaserOnGlob },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseStunPhaserOnSplitGlob1 },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseKillPhaserOnSplitGlob1 },
	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseStunPhaserOnSplitGlob2 },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseKillPhaserOnSplitGlob2 },

	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK,         0}, &Room::trial2UseMTricorderOnKirk },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK,        0}, &Room::trial2UseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY,        0}, &Room::trial2UseMTricorderOnMccoy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT,     0}, &Room::trial2UseMTricorderOnRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_GLOB,         0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseMTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL,        0}, &Room::trial2UseSTricorderOnWall },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff,                0}, &Room::trial2UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_GLOB,         0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseSTricorderOnGlob },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff,                   0}, &Room::trial2UseCommunicator },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SPLIT_GLOB_1,    0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_SPLIT_GLOB_2,    0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_WALL,           0}, &Room::trial2UseMccoyOnWall },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SPLIT_GLOB_1,    0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_SPLIT_GLOB_2,    0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_WALL,           0}, &Room::trial2UseSpockOnWall },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SPLIT_GLOB_1, 0}, &Room::trial2UseRedshirtOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_SPLIT_GLOB_2, 0}, &Room::trial2UseRedshirtOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_WALL,        0}, &Room::trial2UseRedshirtOnWall },

	{ {ACTION_WALK, HOTSPOT_DOOR, 0, 0}, &Room::trial2WalkToDoor },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0}, &Room::trial2UseMedkitAnywhere },

	// ENHANCEMENT: Define these actions for the main glob, not just the (unused) split
	// globs
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_GLOB, 0}, &Room::trial2UseMccoyOnGlob },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_GLOB, 0}, &Room::trial2UseSpockOnGlob },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_GLOB, 0}, &Room::trial2UseRedshirtOnGlob },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::trial2Tick1() {
	playVoc("NOOOLOOP");

	if (!_awayMission->trial.enteredGlobRoom)
		_awayMission->disableInput = 2;

	if (!_awayMission->trial.globDefeated) {
		playMidiMusicTracks(MIDITRACK_24, -1);
		loadMapFile("trial22");

		if (!_awayMission->trial.globSplitInTwo) {
			playVoc("TRI2LOOP");
			loadActorAnim2(OBJECT_GLOB, "sglob", 0x46, 0xaf);
			_awayMission->trial.globEnergyLevels[0] = 1;
		} else {
			playVoc("TRI2LOOP");
			loadActorAnim2(OBJECT_SPLIT_GLOB_1, "sglob", 0x20, 0xaf);
			loadActorAnim2(OBJECT_SPLIT_GLOB_2, "sglob", 0x69, 0xaf);
			_awayMission->trial.globEnergyLevels[1] = 1;
			_awayMission->trial.globEnergyLevels[2] = 1;
		}
	}
}

void Room::trial2Tick60() {
	if (!_awayMission->trial.enteredGlobRoom) {
		_awayMission->disableInput = false;
		showText(TX_SPEAKER_BENNIE, 34, true);
		showText(TX_SPEAKER_KIRK, 5, true);
		showText(TX_SPEAKER_SPOCK, 30, true);
		showText(TX_SPEAKER_KIRK, 8, true);
		_awayMission->trial.enteredGlobRoom = true;
	}
}

void Room::trial2TouchedHotspot0() { // This is unused
	if (_awayMission->trial.globEnergyLevels[1] != 0 || _awayMission->trial.globEnergyLevels[2] != 0)
		showText(TX_SPEAKER_SPOCK, 24, true);
}

void Room::trial2LookAtKirk() {
	showDescription(3, true);
}

void Room::trial2LookAtSpock() {
	showDescription(1, true);
}

void Room::trial2LookAtMccoy() {
	showDescription(6, true);
}

void Room::trial2LookAtRedshirt() {
	showDescription(2, true);
}

void Room::trial2LookAtInsignia() {
	showDescription(4, true);
}

void Room::trial2LookAtGlob() {
	showDescription(0, true);
}

void Room::trial2LookAtWall() {
	showDescription(7, true);
}

void Room::trial2LookAtDoor() {
	showDescription(5, true);
}

void Room::trial2TalkToKirk() {
	showText(TX_SPEAKER_KIRK, 1, true);
}

void Room::trial2TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 19, true);
}

void Room::trial2TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::trial2TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, 33, true);
}

void Room::trial2TalkToGlob() {
	showDescription(TX_QUIET);
}

void Room::trial2UsePhaserOnWall() {
	showDescription(8, true);
}


void Room::trial2UsePhaserOnGlob(int object, bool phaserOnKill) {
	_roomVar.trial.globBeingShot = object;
	_roomVar.trial.phaserOnKill = phaserOnKill;

	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0xcd, 0xc5, &Room::trial2ReachedPositionToShootGlob);
}

void Room::trial2ReachedPositionToShootGlob() {
	loadActorAnimC(OBJECT_KIRK, "kdraww", -1, -1, &Room::trial2DrewPhaserToShootGlob);
}

void Room::trial2DrewPhaserToShootGlob() {
	const char *stunPhaserAnims[] = {
		"t2kp00",
		"t2kp01",
		"t2kp02",
	};
	const char *killPhaserAnims[] = {
		"t2kp03",
		"t2kp04",
		"t2kp05",
	};

	int index = _roomVar.trial.globBeingShot - OBJECT_GLOB;

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	playSoundEffectIndex(kSfxPhaser);
	if (_roomVar.trial.phaserOnKill)
		showBitmapFor5Ticks(killPhaserAnims[index], 5);
	else
		showBitmapFor5Ticks(stunPhaserAnims[index], 5);
	loadActorStandAnim(OBJECT_KIRK);

	_awayMission->disableInput = false;

	if (_roomVar.trial.phaserOnKill)
		_awayMission->trial.globEnergyLevels[index] += 2;
	else
		_awayMission->trial.globEnergyLevels[index] += 1;

	if (!(_roomVar.trial.globBeingShot == OBJECT_GLOB && _roomVar.trial.phaserOnKill == true)) {
		if (_roomVar.trial.phaserOnKill)
			showText(TX_SPEAKER_SPOCK, 23, true);
		else
			showText(TX_SPEAKER_SPOCK, 25, true);
	}


	const Common::Point globPositions[] = {
		Common::Point(GLOB_X, GLOB_Y),
		Common::Point(SPLIT_GLOB_1_X, SPLIT_GLOB_1_Y),
		Common::Point(SPLIT_GLOB_2_X, SPLIT_GLOB_2_Y)
	};

	if (_awayMission->trial.globEnergyLevels[index] == 3) {
		playVoc("GLOBEDIV");
		loadActorAnimC(_roomVar.trial.globBeingShot, "globsp", globPositions[index].x, globPositions[index].y, &Room::trial2GlobDoneSplitting);
	} else if (_awayMission->trial.globEnergyLevels[index] >= 4) {
		playVoc("REDBALL");
		loadActorAnimC(_roomVar.trial.globBeingShot, "globex", globPositions[index].x, globPositions[index].y, &Room::trial2GlobDoneExploding);
	}
}

void Room::trial2GlobDoneExploding() {
	stopAllVocSounds();
	playVoc("Noooloop");
	showText(TX_SPEAKER_SPOCK, 20, true);
	_awayMission->trial.globDefeated = true;
	playMidiMusicTracks(MIDITRACK_28, -1);
	_awayMission->trial.missionScore += 1;
	loadMapFile("trial2");
}

void Room::trial2GlobDoneSplitting() {
	if (_roomVar.trial.globBeingShot == OBJECT_GLOB) {
		showText(TX_SPEAKER_SPOCK, 9, true);
	} else if (_roomVar.trial.globBeingShot == OBJECT_SPLIT_GLOB_1) {
		loadActorAnim2(OBJECT_11, "sglob", 0x43, 0xaf);
		loadActorAnim2(OBJECT_SPLIT_GLOB_1, "sglob", 0, 0xaf);
	} else if (_roomVar.trial.globBeingShot == OBJECT_SPLIT_GLOB_2) {
		loadActorAnim2(OBJECT_11, "sglob", 0x8c, 0xaf);
		loadActorAnim2(OBJECT_SPLIT_GLOB_2, "sglob", 0x46, 0xaf);
	}

	// Everyone gets vaporized
	playVoc("V7ALLGET");
	_awayMission->disableInput = true;
	playMidiMusicTracks(MIDITRACK_26, -1);
	loadActorAnimC(OBJECT_KIRK, "kkills", -1, -1, &Room::trial2KirkDied);
	loadActorAnim2(OBJECT_SPOCK, "skills");
	loadActorAnim2(OBJECT_MCCOY, "mkills");
	loadActorAnim2(OBJECT_REDSHIRT, "rkills");
}

void Room::trial2KirkDied() {
	_awayMission->disableInput = false;
	showGameOverMenu();
}

void Room::trial2UseStunPhaserOnGlob() {
	trial2UsePhaserOnGlob(OBJECT_GLOB, false);
}

void Room::trial2UseKillPhaserOnGlob() {
	trial2UsePhaserOnGlob(OBJECT_GLOB, true);
}

void Room::trial2UseStunPhaserOnSplitGlob1() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_1, false);
}

void Room::trial2UseKillPhaserOnSplitGlob1() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_1, true);
}

void Room::trial2UseStunPhaserOnSplitGlob2() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_2, false);
}

void Room::trial2UseKillPhaserOnSplitGlob2() {
	trial2UsePhaserOnGlob(OBJECT_SPLIT_GLOB_2, true);
}

void Room::trial2UseMTricorderOnKirk() {
	mccoyScan(DIR_S, 16, true, true);
}

void Room::trial2UseMTricorderOnSpock() {
	mccoyScan(DIR_S, 17, true, true);
}

void Room::trial2UseMTricorderOnMccoy() {
	mccoyScan(DIR_S, 14, true, true);
}

void Room::trial2UseMTricorderOnRedshirt() {
	mccoyScan(DIR_S, 15, true, true);
}

void Room::trial2UseMTricorderOnGlob() {
	mccoyScan(DIR_S, 11, true, true);

	if (!_awayMission->trial.gotPointsForScanningGlob) {
		_awayMission->trial.gotPointsForScanningGlob = true;
		_awayMission->trial.missionScore += 1;
	}
}

void Room::trial2UseSTricorderOnWall() {
	spockScan(DIR_S, 29, true, true);
}

void Room::trial2UseSTricorderAnywhere() {
	spockScan(DIR_S, 27, true, true);
}

void Room::trial2UseSTricorderOnGlob() {
	spockScan(DIR_S, 26, true, true);

	if (!_awayMission->trial.gotPointsForScanningGlob) {
		_awayMission->trial.gotPointsForScanningGlob = true;
		_awayMission->trial.missionScore += 1;
	}
}

void Room::trial2UseCommunicator() {
	if (_awayMission->trial.forceFieldDown) { // TODO: Refactor this between rooms?
		showText(TX_SPEAKER_UHURA, 91, true);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			3, 4, 7,
			TX_BLANK
		};
		int choice = showMultipleTexts(choices, true);

		if (choice == 0) { // "Beam us back to the enterprise"
			_awayMission->trial.missionEndMethod = 1;
			endMission(_awayMission->trial.missionScore, 1, 1); // FIXME: Inconsistent with TRIAL1
		} else if (choice == 1) { // "Beam us to Vlict's position"
			showText(TX_SPEAKER_UHURA, TX_TRI1U080); // NOTE: Original didn't show text here
			_awayMission->disableInput = true;
			loadRoomIndex(4, 4);
		} // Else don't transport anywhere
	} else { // Force field still up
		showText(TX_SPEAKER_UHURA, 87, true);
		showText(TX_SPEAKER_KIRK,  6, true);
		showText(TX_SPEAKER_UHURA, 104, true);
		if (!_awayMission->trial.globDefeated) {
			showText(TX_SPEAKER_UHURA, 81, true);
			showText(TX_SPEAKER_KIRK,  2, true);
		}
		showText(TX_SPEAKER_UHURA, 74, true);
	}
}

void Room::trial2UseMccoyOnGlob() {
	showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::trial2UseMccoyOnWall() {
	showText(TX_SPEAKER_MCCOY, 10, true);
}

void Room::trial2UseSpockOnGlob() {
	// NOTE: Two possible audio files to use, TRI2_028 and TRI2_F11
	showText(TX_SPEAKER_SPOCK, 28, true);
}

void Room::trial2UseSpockOnWall() {
	showText(TX_SPEAKER_SPOCK, 22, true);
}

void Room::trial2UseRedshirtOnGlob() {
	showText(TX_SPEAKER_BENNIE, 31, true);
}

void Room::trial2UseRedshirtOnWall() {
	showText(TX_SPEAKER_BENNIE, 32, true);
}

void Room::trial2WalkToDoor() {
	walkCrewman(OBJECT_KIRK, 0x117, 0xb5);
}

void Room::trial2UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 18, true);
}

}
