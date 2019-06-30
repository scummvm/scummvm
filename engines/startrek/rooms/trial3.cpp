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

#define OBJECT_KLINGON_1 8
#define OBJECT_KLINGON_2 9  // Unused
#define OBJECT_KLINGON_3 10 // Unused
#define OBJECT_EXPLOSION 11

#define HOTSPOT_EXIT 0x20
#define HOTSPOT_WALL 0x21

namespace StarTrek {

extern const RoomAction trial3ActionList[] = {
	{ {ACTION_TICK, 1,  0, 0}, &Room::trial3Tick1 },
	{ {ACTION_TICK, 30, 0, 0}, &Room::trial3Tick30 },
	{ {ACTION_DONE_ANIM,  1, 0, 0}, &Room::trial3Klingon1BeamedIn },
	{ {ACTION_DONE_ANIM,  2, 0, 0}, &Room::trial3Klingon2BeamedIn },
	{ {ACTION_DONE_ANIM,  3, 0, 0}, &Room::trial3Klingon3BeamedIn },
	{ {ACTION_DONE_ANIM,  4, 0, 0}, &Room::trial3Klingon1DoneShooting },
	{ {ACTION_DONE_ANIM,  5, 0, 0}, &Room::trial3Klingon2DoneShooting },
	{ {ACTION_DONE_ANIM,  6, 0, 0}, &Room::trial3Klingon3DoneShooting },
	{ {ACTION_DONE_ANIM, 15, 0, 0}, &Room::trial3RedshirtDoneDying },
	{ {ACTION_DONE_ANIM, 16, 0, 0}, &Room::trial3KirkDoneDying },
	{ {ACTION_DONE_ANIM, 11, 0, 0}, &Room::trial3Klingon1Shot },
	{ {ACTION_DONE_ANIM, 12, 0, 0}, &Room::trial3Klingon2Shot },
	{ {ACTION_DONE_ANIM, 13, 0, 0}, &Room::trial3Klingon3Shot },
	{ {ACTION_DONE_ANIM, 19, 0, 0}, &Room::trial3CrewmanBeamedOut },
	{ {ACTION_TICK, 90,      0, 0}, &Room::trial3Tick90 },
	{ {ACTION_TOUCHED_HOTSPOT, 3,    0, 0}, &Room::trial3TouchedHotspot3 },
	{ {ACTION_DONE_ANIM, 14,         0, 0}, &Room::trial3KirkExploded },
	{ {ACTION_LOOK, OBJECT_KIRK,     0, 0}, &Room::trial3LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK,    0, 0}, &Room::trial3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY,    0, 0}, &Room::trial3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::trial3LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_EXIT,    0, 0}, &Room::trial3LookAtExit },
	{ {ACTION_LOOK, HOTSPOT_WALL,    0, 0}, &Room::trial3LookAtWall },
	{ {ACTION_TALK, OBJECT_KIRK,     0, 0}, &Room::trial3TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK,    0, 0}, &Room::trial3TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY,    0, 0}, &Room::trial3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::trial3TalkToRedshirt },
	{ {ACTION_USE, OBJECT_IPHASERS, HOTSPOT_WALL, 0}, &Room::trial3UsePhaserOnWall },
	{ {ACTION_USE, OBJECT_IPHASERK, HOTSPOT_WALL, 0}, &Room::trial3UsePhaserOnWall },

	{ {ACTION_USE, OBJECT_IPHASERS, OBJECT_KLINGON_1, 0}, &Room::trial3UseStunPhaserOnKlingon1 },
	{ {ACTION_DONE_ANIM, 17, 0, 0},                       &Room::trial3ReadyToShootKlingon1OnStun },
	{ {ACTION_USE, OBJECT_IPHASERK, OBJECT_KLINGON_1, 0}, &Room::trial3UseKillPhaserOnKlingon1 },
	{ {ACTION_DONE_ANIM, 18, 0, 0},                       &Room::trial3ReadyToShootKlingon1OnKill },
	// OMITTED: Similar code for unused klingons 2 and 3

	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::trial3UsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::trial3UsePhaserAnywhere },

	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0},      &Room::trial3UseMTricorderOnKirk },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0},     &Room::trial3UseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0},     &Room::trial3UseMTricorderOnMccoy },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0},  &Room::trial3UseMTricorderOnRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_EXIT, 0},     &Room::trial3UseMTricorderOnExit },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_WALL, 0},     &Room::trial3UseSTricorderOnWall },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_EXIT, 0},     &Room::trial3UseSTricorderOnExit },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KLINGON_1, 0}, &Room::trial3UseMTricorderOnKlingon },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0},                &Room::trial3UseCommunicator },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_WALL, 0},        &Room::trial3UseMccoyOnWall },
	{ {ACTION_USE, OBJECT_MCCOY, HOTSPOT_EXIT, 0},        &Room::trial3UseMccoyOnExit },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_WALL, 0},        &Room::trial3UseSpockOnWall },
	{ {ACTION_USE, OBJECT_SPOCK, HOTSPOT_EXIT, 0},        &Room::trial3UseSpockOnExit },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_EXIT, 0},     &Room::trial3UseRedshirtOnExit },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_WALL, 0},     &Room::trial3UseRedshirtOnWall },
	{ {ACTION_WALK, HOTSPOT_EXIT, 0, 0},                  &Room::trial3WalkToExit },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},              &Room::trial3UseMedkitAnywhere },
};

extern const int trial3NumActions = ARRAYSIZE(trial3ActionList);


void Room::trial3Tick1() {
	playVoc("TRI3LOOP");

	if (!_awayMission->trial.enteredTrial3FirstTime) {
		_awayMission->disableWalking = true;
		_awayMission->disableInput = 2;
	}
	playMidiMusicTracks(MIDITRACK_33, -1);
}

void Room::trial3Tick30() {
	if (!_awayMission->trial.enteredTrial3FirstTime) {
		_awayMission->disableInput = false;
		_awayMission->trial.enteredTrial3FirstTime = true;

		showText(TX_SPEAKER_BENNIE, TX_TRI3_030);
		showText(TX_SPEAKER_KIRK,   TX_TRI3_005);
		showText(TX_SPEAKER_MCCOY,  TX_TRI3_019);
		showText(TX_SPEAKER_SPOCK,  TX_TRI3_025);
		showText(TX_SPEAKER_MCCOY,  TX_TRI3_020);
		showText(TX_SPEAKER_KIRK,   TX_TRI3_004);
	}
}

void Room::trial3Klingon1BeamedIn() {
	loadActorAnimC(OBJECT_KLINGON_1, "t3kfir", -1, -1, &Room::trial3Klingon1DoneShooting);
	trial3KlingonShootsSomeone1();
}

void Room::trial3Klingon2BeamedIn() {
	loadActorAnimC(OBJECT_KLINGON_2, "t3kfir", -1, -1, &Room::trial3Klingon2DoneShooting);
	trial3KlingonShootsSomeone1();
}

void Room::trial3Klingon3BeamedIn() {
	loadActorAnimC(OBJECT_KLINGON_3, "t3kfir", -1, -1, &Room::trial3Klingon3DoneShooting);
	trial3KlingonShootsSomeone1();
}

void Room::trial3KlingonShootsSomeone1() {
	_awayMission->trial.klingonShootIndex++;
	if (_awayMission->trial.klingonShootIndex == 1) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas04", 5);
		loadActorAnimC(OBJECT_REDSHIRT, "rkillw", -1, -1, &Room::trial3RedshirtDoneDying);
		_awayMission->redshirtDead = true;
	} else if (_awayMission->trial.klingonShootIndex == 2) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas05", 5);
		playMidiMusicTracks(MIDITRACK_2, -1);
		loadActorAnimC(OBJECT_KIRK, "kkillw", -1, -1, &Room::trial3KirkDoneDying);
	}
}

void Room::trial3Klingon1DoneShooting() {
	loadActorAnimC(OBJECT_KLINGON_1, "t3kfir", -1, -1, &Room::trial3Klingon1DoneShooting);
	trial3KlingonShootsSomeone2();
}

void Room::trial3Klingon2DoneShooting() {
	loadActorAnimC(OBJECT_KLINGON_2, "t3kfir", -1, -1, &Room::trial3Klingon2DoneShooting);
	trial3KlingonShootsSomeone2();
}

void Room::trial3Klingon3DoneShooting() {
	loadActorAnimC(OBJECT_KLINGON_3, "t3kfir", -1, -1, &Room::trial3Klingon3DoneShooting);
	trial3KlingonShootsSomeone2();
}

void Room::trial3KlingonShootsSomeone2() {
	// This function is almost exactly identical to "trial3KlingonShootsSomeone1(), just
	// one line differs...
	_awayMission->trial.klingonShootIndex++;
	if (_awayMission->trial.klingonShootIndex == 1) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas04", 5);
		loadActorAnimC(OBJECT_REDSHIRT, "rkillw", -1, -1, &Room::trial3RedshirtDoneDying);
		_awayMission->redshirtDead = true;
	} else if (_awayMission->trial.klingonShootIndex == 2) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas05", 5);
		// NOTE: Only difference to "trial3KlingonShootsSomeone1" is this doesn't play a midi track?
		loadActorAnimC(OBJECT_KIRK, "kkillw", -1, -1, &Room::trial3KirkDoneDying);
	}
}

void Room::trial3RedshirtDoneDying() {
	_awayMission->redshirtDead = true;
}

void Room::trial3KirkDoneDying() {
	showGameOverMenu();
}

void Room::trial3Klingon1Shot() {
	_awayMission->trial.shotKlingons |= 1;
	trial3CheckShowUhuraText();
}

void Room::trial3Klingon2Shot() {
	_awayMission->trial.shotKlingons |= 2;
	trial3CheckShowUhuraText();
}

void Room::trial3Klingon3Shot() {
	_awayMission->trial.shotKlingons |= 4;
	trial3CheckShowUhuraText();
}

void Room::trial3CheckShowUhuraText() {
	if (_awayMission->trial.shotKlingons == 1) {
		_awayMission->trial.shotKlingons |= 8;
		_awayMission->disableWalking = false;
		loadActorStandAnim(OBJECT_KIRK);

		showText(TX_SPEAKER_UHURA, TX_TRI3U084);
		showText(TX_SPEAKER_KIRK,  TX_TRI3_007);
		showText(TX_SPEAKER_UHURA, TX_TRI3U099);

		_awayMission->trial.forceFieldDown = true;

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_TRI3_006, TX_TRI3_002, TX_TRI3_003,
			TX_BLANK
		};
		int choice = showMultipleTexts(choices);

		if (choice == 0) { // Don't beam out
		} else if (choice == 1) { // Beam to enterprise
			endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 1); // FIXME: inconsistent
		} else if (choice == 2) { // Beam to Vlict
			trial3BeamToVlict();
		}
	}
}

void Room::trial3CrewmanBeamedOut() {
	if (!_awayMission->trial.gotPointsForBeamingOut) {
		_awayMission->trial.gotPointsForBeamingOut = true;
		_awayMission->trial.missionScore += 2; // BUG: Doesn't happen when done in other rooms
	}
	loadRoomIndex(4, 4);
}

void Room::trial3Tick90() {
	if ((!(_awayMission->trial.shotKlingons & 8) && _awayMission->trial.shotKlingonState != 20)) {
		playSoundEffectIndex(SND_TRANSMAT);
		playMidiMusicTracks(MIDITRACK_32, -1);
		loadActorAnimC(OBJECT_KLINGON_1, "t3ktel", 0x57, 0xb1, &Room::trial3Klingon1BeamedIn);
		_awayMission->trial.shotKlingonState = 21;
	}
}

void Room::trial3TouchedHotspot3() { // Activated the explosive
	playSoundEffectIndex(SND_BLANK_14);
	playMidiMusicTracks(MIDITRACK_2, -1);
	playVoc("BITOKIRK");
	loadActorAnimC(OBJECT_EXPLOSION, "t3expl", 0, 0xc7, &Room::trial3KirkExploded);
}

void Room::trial3KirkExploded() {
	showGameOverMenu();
}

void Room::trial3LookAtKirk() {
	showDescription(TX_TRI3N000);
}

void Room::trial3LookAtSpock() {
	showDescription(TX_TRI3N004);
}

void Room::trial3LookAtMccoy() {
	showDescription(TX_TRI3N001);
}

void Room::trial3LookAtRedshirt() {
	showDescription(TX_TRI3N002);
}

void Room::trial3LookAtExit() {
	showDescription(TX_TRI3N005);
}

void Room::trial3LookAtWall() {
	showDescription(TX_TRI3N007);
}

void Room::trial3TalkToKirk() {
	showText(TX_SPEAKER_KIRK, TX_TRI3_001);
}

void Room::trial3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_TRI3_022);
}

void Room::trial3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_TRI3_018);
}

void Room::trial3TalkToRedshirt() {
	showText(TX_SPEAKER_BENNIE, TX_TRI3_029);
}

void Room::trial3UsePhaserOnWall() {
	showDescription(TX_TRI3N006);
}

void Room::trial3UseStunPhaserOnKlingon1() {
	// BUGFIX: Instead of checking that the klingon isn't unconscious, (22), check that
	// he's conscious (21).
	// There's also the "dead" state (23) to consider. This prevents a softlock if
	// a phaser is used on him just as he's being vaporized.
	if (_awayMission->trial.shotKlingonState == 21) {
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "kdraww", -1, -1, &Room::trial3ReadyToShootKlingon1OnStun);
	}
}

void Room::trial3ReadyToShootKlingon1OnStun() {
	if (_awayMission->trial.shotKlingonState == 21) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas00", 5);
		loadActorAnimC(OBJECT_KLINGON_1, "t3kstn", -1, -1, &Room::trial3Klingon1Shot);
		_awayMission->disableInput = false;
		_awayMission->trial.shotKlingonState = 22;
	}
}

void Room::trial3UseKillPhaserOnKlingon1() {
	// BUGFIX: Prevent softlock by checking that he's conscious (shotKlingonState == 21)
	// In addition to preventing the softlock mentioned above, this also prevents
	// a softlock where a kill phaser is used on the unconscious klingon.
	if (_awayMission->trial.shotKlingonState == 21) {
		_awayMission->disableInput = true;
		loadActorAnimC(OBJECT_KIRK, "kdraww", -1, -1, &Room::trial3ReadyToShootKlingon1OnKill);
	}
}

void Room::trial3ReadyToShootKlingon1OnKill() {
	if (_awayMission->trial.shotKlingonState == 21) {
		playSoundEffectIndex(SND_PHASSHOT);
		showBitmapFor5Ticks("t3phas02", 5);
		loadActorAnimC(OBJECT_KLINGON_1, "t3kdie", -1, -1, &Room::trial3Klingon1Shot);
		_awayMission->disableInput = false;
		_awayMission->trial.shotKlingonState = 23;
		_awayMission->trial.missionScore -= 3; // Penalty for killing klingon
	}
}

void Room::trial3UsePhaserAnywhere() {
	showDescription(TX_TRI3N003);
}

void Room::trial3UseMTricorderOnKirk() {
	// BUGFIX: Original animated Spock instead of Mccoy (same for below mccoy-scan functions)
	mccoyScan(DIR_S, TX_TRI3_015, true);
}

void Room::trial3UseMTricorderOnSpock() {
	mccoyScan(DIR_S, TX_TRI3_016, true);
}

void Room::trial3UseMTricorderOnMccoy() {
	mccoyScan(DIR_S, TX_TRI3_013, true);
}

void Room::trial3UseMTricorderOnRedshirt() {
	mccoyScan(DIR_S, TX_TRI3_014, true);
}

void Room::trial3UseMTricorderOnExit() {
	mccoyScan(DIR_S, TX_TRI3_009, true);
}

void Room::trial3UseSTricorderOnWall() {
	spockScan(DIR_S, TX_TRI3_024, true);
}

void Room::trial3UseSTricorderOnExit() {
	spockScan(DIR_S, TX_TRI3_023, true);
}

void Room::trial3UseMTricorderOnKlingon() {
	if (_awayMission->trial.shotKlingonState == 22) { // Unconscious
		mccoyScan(DIR_S, TX_TRI3_011);
		if (!_awayMission->redshirtDead) // BUGFIX: Check if redshirt is dead
			showText(TX_SPEAKER_BENNIE, TX_TRI3_028);
	}
}

void Room::trial3UseCommunicator() {
	if (_awayMission->trial.forceFieldDown) {
		showText(TX_SPEAKER_UHURA, TX_TRI3U089);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_TRI3_103, TX_TRI3_104, TX_TRI3_107,
			TX_BLANK
		};
		int choice = showMultipleTexts(choices);

		if (choice == 0) { // "Beam us back to the enterprise"
			_awayMission->trial.missionEndMethod = 1;
			endMission(_awayMission->trial.missionScore, _awayMission->trial.field2b, 1); // FIXME: inconsistent
		} else if (choice == 1) { // "Beam us to Vlict's position"
			trial3BeamToVlict();
		} // Else don't transport anywhere
	} else { // Force field still up
		showText(TX_SPEAKER_UHURA, TX_TRI3U067);
	}
}

void Room::trial3BeamToVlict() {
	// ENHANCEMENT: The audio that should play here (TX_TRI3U080) doesn't seem to have the
	// normal "filter" applied over it, making it sound jarring. So, use the equivalent
	// text from TRIAL1 instead.
	showText(TX_SPEAKER_UHURA, TX_TRI1U080);

	_awayMission->disableInput = true;
	playSoundEffectIndex(SND_TRANSDEM);

	loadActorAnimC(OBJECT_KIRK,  "kteled", -1, -1, &Room::trial3CrewmanBeamedOut);
	loadActorAnimC(OBJECT_SPOCK, "steled", -1, -1, &Room::trial3CrewmanBeamedOut);
	loadActorAnimC(OBJECT_MCCOY, "mteled", -1, -1, &Room::trial3CrewmanBeamedOut);
	if (!_awayMission->redshirtDead)
		loadActorAnimC(OBJECT_REDSHIRT, "rteled", -1, -1, &Room::trial3CrewmanBeamedOut);
}

void Room::trial3UseMccoyOnWall() {
	showText(TX_SPEAKER_MCCOY, TX_TRI3_010);
}

void Room::trial3UseMccoyOnExit() {
	showText(TX_SPEAKER_MCCOY, TX_TRI3_012);
}

void Room::trial3UseSpockOnWall() {
	showText(TX_SPEAKER_SPOCK, TX_TRI3_021);
}

void Room::trial3UseSpockOnExit() {
	showText(TX_SPEAKER_SPOCK, TX_TRI3_008);
}

void Room::trial3UseRedshirtOnExit() {
	showText(TX_SPEAKER_BENNIE, TX_TRI3_027);
}

void Room::trial3UseRedshirtOnWall() {
	showText(TX_SPEAKER_BENNIE, TX_TRI3_026);
}

void Room::trial3WalkToExit() {
	walkCrewman(OBJECT_KIRK, 0x26, 0x9d);
}

void Room::trial3UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_TRI3_017);
}

}
