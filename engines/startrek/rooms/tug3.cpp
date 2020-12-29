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

#define OBJECT_ELASI_1 8
#define OBJECT_ELASI_2 9
#define OBJECT_ELASI_3 10
#define OBJECT_ELASI_4 11
#define OBJECT_12 12
#define OBJECT_13 13
#define OBJECT_14 14
#define OBJECT_15 15

#define GUARDSTAT_UP 0
#define GUARDSTAT_STUNNED 1
#define GUARDSTAT_DEAD 2
#define GUARDSTAT_SURRENDERED 4
#define GUARDSTAT_TIED 8

extern const RoomAction tug3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0}, &Room::tug3Tick1 },
	{ {ACTION_TICK, 40, 0, 0}, &Room::tug3Tick40 },
	{ {ACTION_LOOK, 0xff, 0, 0}, &Room::tug3LookAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0}, &Room::tug3UsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0}, &Room::tug3UsePhaserAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERS, 8, 0}, &Room::tug3UseStunPhaserOnElasi1 },
	{ {ACTION_USE, OBJECT_IPHASERS, 9, 0}, &Room::tug3UseStunPhaserOnElasi2 },
	{ {ACTION_USE, OBJECT_IPHASERS, 10, 0}, &Room::tug3UseStunPhaserOnElasi3 },
	{ {ACTION_USE, OBJECT_IPHASERS, 11, 0}, &Room::tug3UseStunPhaserOnElasi4 },
	{ {ACTION_USE, OBJECT_IPHASERK, 8, 0}, &Room::tug3UseKillPhaserOnElasi1 },
	{ {ACTION_USE, OBJECT_IPHASERK, 9, 0}, &Room::tug3UseKillPhaserOnElasi2 },
	{ {ACTION_USE, OBJECT_IPHASERK, 10, 0}, &Room::tug3UseKillPhaserOnElasi3 },
	{ {ACTION_USE, OBJECT_IPHASERK, 11, 0}, &Room::tug3UseKillPhaserOnElasi4 },
	{ {ACTION_FINISHED_ANIMATION, 12, 0, 0}, &Room::tug3ElasiStunnedOrKilled },
	{ {ACTION_TALK, 8, 0, 0}, &Room::tug3TalkToElasi1 },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0}, &Room::tug3Elasi1DrewPhaser },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0}, &Room::tug3Elasi1ShotConsole },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0}, &Room::tug3Elasi1DrewPhaser2 },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0}, &Room::tug3Elasi1ShotConsoleAndSurrenders },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0}, &Room::tug3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0}, &Room::tug3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0}, &Room::tug3LookAtRedshirt },
	{ {ACTION_LOOK, 8, 0, 0}, &Room::tug3LookAtElasi1 },
	{ {ACTION_LOOK, 9, 0, 0}, &Room::tug3LookAtElasi2 },
	{ {ACTION_LOOK, 10, 0, 0}, &Room::tug3LookAtElasi3 },
	{ {ACTION_LOOK, 11, 0, 0}, &Room::tug3LookAtElasi4 },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0}, &Room::tug3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0}, &Room::tug3TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0}, &Room::tug3TalkToRedshirt },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0}, &Room::tug3UseCommunicator },
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0}, &Room::tug3Timer0Expired },
	{ {ACTION_FINISHED_ANIMATION, 13, 0, 0}, &Room::tug3AllCrewmenDead },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0}, &Room::tug3Timer1Expired },
	{ {ACTION_FINISHED_ANIMATION, 7, 0, 0}, &Room::tug3SecurityTeamBeamedIn },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::tug3Tick1() {
	playVoc("TUG3LOOP");

	_awayMission->disableWalking = true;
	loadActorAnim2(OBJECT_ELASI_1, "p1turn", 0xa4, 0x98, 0);
	loadActorAnim2(OBJECT_ELASI_2, "p2turn", 0xd1, 0x88, 0);
	loadActorAnim2(OBJECT_ELASI_3, "p3turn", 0xfb, 0xc4, 0);
	loadActorAnim2(OBJECT_ELASI_4, "p4turn", 0x5a, 0x9e, 0);
}

void Room::tug3Tick40() {
	loadActorAnim2(OBJECT_KIRK, "kdraws", -1, -1, 0); // Draw phaser
}

void Room::tug3LookAnywhere() {
	showDescription(7, true);
}

void Room::tug3ElasiSurrendered() {
	// BUGFIX: the original game had the condition below. However, this would cause
	// problems if you start shooting, then get them to surrender; if timed correctly,
	// they could shoot Kirk as he's beaming out and the mission is "successful". To
	// prevent that, the below condition is removed in ScummVM.
	/*
	if (_awayMission->tug.bridgeElasiDrewPhasers)
		return;
	*/

	// Also part of the bugfix (stop shooting)
	_awayMission->timers[0] = 0;

	_awayMission->tug.elasiSurrendered = true;
	loadActorAnim2(OBJECT_ELASI_1, "p1surr", -1, -1, 0);
	_awayMission->tug.bridgeElasi1Status = GUARDSTAT_SURRENDERED;

	if (_awayMission->tug.bridgeElasi2Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_ELASI_2, "p2surr", -1, -1, 0);
		_awayMission->tug.bridgeElasi2Status = GUARDSTAT_SURRENDERED;
	}
	if (_awayMission->tug.bridgeElasi3Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_ELASI_3, "p3surr", -1, -1, 0);
		_awayMission->tug.bridgeElasi3Status = GUARDSTAT_SURRENDERED;
	}
	if (_awayMission->tug.bridgeElasi4Status == GUARDSTAT_UP) {
		loadActorAnim2(OBJECT_ELASI_4, "p4surr", -1, -1, 0);
		_awayMission->tug.bridgeElasi4Status = GUARDSTAT_SURRENDERED;
	}

	_awayMission->tug.missionScore += 4;
}

void Room::tug3UsePhaserAnywhere() {
	// Stub function to suppress "you can't do that" messages
}

void Room::tug3ElasiDrawPhasers() {
	if (_awayMission->tug.bridgeElasiDrewPhasers)
		return;

	_awayMission->tug.bridgeElasiDrewPhasers = true;

	// If brig guards are alive & untied, kill the hostages
	if (_awayMission->tug.guard1Status == 0 || _awayMission->tug.guard2Status == 0) {
		showText(TX_SPEAKER_ELASI_CLANSMAN, 81, true, true);
		_awayMission->tug.missionScore = 0;
	}

	if (_awayMission->timers[0] == 0)
		_awayMission->timers[0] = 10;

	// Make all elasi draw their phasers
	if (_awayMission->tug.bridgeElasi1Status == 0)
		loadActorAnim2(OBJECT_ELASI_1, "p1draw", -1, -1, 0);
	if (_awayMission->tug.bridgeElasi2Status == 0)
		loadActorAnim2(OBJECT_ELASI_2, "p2draw", -1, -1, 0);
	if (_awayMission->tug.bridgeElasi3Status == 0)
		loadActorAnim2(OBJECT_ELASI_3, "p3draw", -1, -1, 0);
	if (_awayMission->tug.bridgeElasi4Status == 0)
		loadActorAnim2(OBJECT_ELASI_4, "p4draw", -1, -1, 0);
}

void Room::tug3UseStunPhaserOnElasi1() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi1Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_1, "p1stun", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem05", 5);
	_awayMission->tug.bridgeElasi1Status = GUARDSTAT_STUNNED;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseStunPhaserOnElasi2() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi2Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_2, "p2stun", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem06", 5);
	_awayMission->tug.bridgeElasi2Status = GUARDSTAT_STUNNED;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseStunPhaserOnElasi3() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi3Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_3, "p3stun", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem07", 5);
	_awayMission->tug.bridgeElasi3Status = GUARDSTAT_STUNNED;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseStunPhaserOnElasi4() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi4Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_4, "p4stun", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem04", 5);
	_awayMission->tug.bridgeElasi4Status = GUARDSTAT_STUNNED;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseKillPhaserOnElasi1() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi1Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_1, "p1Kill", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem25", 5);
	_awayMission->tug.bridgeElasi1Status = GUARDSTAT_DEAD;
	_awayMission->tug.missionScore -= 2;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseKillPhaserOnElasi2() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi2Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_2, "p2Kill", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem27", 5);
	_awayMission->tug.bridgeElasi2Status = GUARDSTAT_DEAD;
	_awayMission->tug.missionScore -= 2;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseKillPhaserOnElasi3() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi3Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_3, "p3Kill", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem26", 5);
	_awayMission->tug.bridgeElasi3Status = GUARDSTAT_DEAD;
	_awayMission->tug.missionScore -= 2;
	tug3ElasiDrawPhasers();
}

void Room::tug3UseKillPhaserOnElasi4() {
	if (_awayMission->tug.crewmanKilled[OBJECT_KIRK] || _awayMission->tug.bridgeElasi4Status != GUARDSTAT_UP)
		return;
	loadActorAnim2(OBJECT_ELASI_4, "p4Kill", -1, -1, 12);
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem24", 5);
	_awayMission->tug.bridgeElasi4Status = GUARDSTAT_DEAD;
	_awayMission->tug.missionScore -= 2;
	tug3ElasiDrawPhasers();
}

void Room::tug3ElasiStunnedOrKilled() {
	if (_awayMission->tug.bridgeWinMethod == 1)
		return;
	if (_awayMission->tug.bridgeElasi1Status == GUARDSTAT_UP || _awayMission->tug.bridgeElasi2Status == GUARDSTAT_UP
	        || _awayMission->tug.bridgeElasi3Status == GUARDSTAT_UP || _awayMission->tug.bridgeElasi4Status == GUARDSTAT_UP)
		return;

	// BUGFIX: if the ship is deorbiting, the mission isn't won yet.
	if (_awayMission->tug.orbitalDecayCounter != 0)
		return;

	_awayMission->tug.bridgeWinMethod = 1;
	tug3EndMission();
}

void Room::tug3TalkToElasi1() {
	if (_awayMission->tug.bridgeElasi1Status != GUARDSTAT_UP || _awayMission->tug.talkedToCereth)
		return;
	_awayMission->tug.talkedToCereth = true;

	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		4,
		3,
		2,
		TX_BLANK
	};
	int choice = showMultipleTexts(choices, true);

	switch (choice) {
	case 0: // They surrender
		showText(TX_SPEAKER_ELASI_CERETH, 84, true, true);
		_awayMission->tug.bridgeElasi1Status = GUARDSTAT_SURRENDERED;
		loadActorAnim2(OBJECT_ELASI_1, "p1surr", -1, -1, 0);
		tug3ElasiSurrendered();
		_awayMission->tug.bridgeWinMethod = 2;
		_awayMission->tug.missionScore += 8;
		tug3EndMission();
		break;

	case 1: // Cereth shoots console, doesn't surrender
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_ELASI_CERETH, 27 + FOLLOWUP_MESSAGE_OFFSET, true);
		loadActorAnim2(OBJECT_ELASI_1, "p1draw", -1, -1, 3);
		break;

	case 2: // Cereth shoots console and surrenders
		_awayMission->disableInput = true;
		showText(TX_SPEAKER_ELASI_CERETH, 83, true, true);
		loadActorAnim2(OBJECT_ELASI_1, "p1draw", -1, -1, 5);

		// BUGFIX: they're going to surrender, so stop the firefight.
		_awayMission->timers[0] = 0;
		break;

	default:
		break;
	}
}

void Room::tug3Elasi1DrewPhaser() {
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem41", 13);
	loadActorAnim2(OBJECT_12, "sparks", 0xa0, 0xad, 4);
}

void Room::tug3Elasi1ShotConsole() {
	tug3ElasiDrawPhasers();
	_awayMission->disableInput = false;
	_awayMission->tug.bridgeWinMethod = 3;
	_awayMission->timers[1] = 10;
}

void Room::tug3Elasi1DrewPhaser2() {
	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks("t3beem41", 13);
	loadActorAnim2(OBJECT_12, "sparks", 0xa0, 0xad, 6);
}

void Room::tug3Elasi1ShotConsoleAndSurrenders() {
	showText(TX_SPEAKER_ELASI_CERETH, 80, true, true);
	tug3ElasiSurrendered();
	_awayMission->disableInput = false;
	_awayMission->tug.bridgeWinMethod = 3;
	_awayMission->timers[1] = 10;
}

void Room::tug3LookAtMccoy() {
	showDescription(0, true);
}

void Room::tug3LookAtSpock() {
	showDescription(2, true);
}

void Room::tug3LookAtRedshirt() {
	showDescription(1, true);
}

void Room::tug3LookAtElasi1() {
	// BUGFIX: there were two implementations of this function; the first was the same as
	// the other 3 elasi, the second was specific to the captain. The second was never
	// called in the original game, but it's used here instead for more variety.
	// The function itself is changed by checking both if he's stunned or if he's dead,
	// instead of just checking if he's stunned.

	if (_awayMission->tug.bridgeElasi1Status == GUARDSTAT_STUNNED || _awayMission->tug.bridgeElasi1Status == GUARDSTAT_DEAD)
		showDescription(3, true);
	else
		showDescription(8, true);
}

void Room::tug3LookAtElasi2() {
	// BUGFIX: also check if stunned. They can't "glare at the crewmembers" if they're
	// unconscious. (applies to below functions too.)
	if (_awayMission->tug.bridgeElasi2Status == GUARDSTAT_DEAD || _awayMission->tug.bridgeElasi2Status == GUARDSTAT_STUNNED)
		showDescription(4, true);
	else
		showDescription(5, true);
}

void Room::tug3LookAtElasi3() {
	if (_awayMission->tug.bridgeElasi3Status == GUARDSTAT_DEAD || _awayMission->tug.bridgeElasi2Status == GUARDSTAT_STUNNED)
		showDescription(4, true);
	else
		showDescription(5, true);
}

void Room::tug3LookAtElasi4() {
	if (_awayMission->tug.bridgeElasi4Status == GUARDSTAT_DEAD || _awayMission->tug.bridgeElasi2Status == GUARDSTAT_STUNNED)
		showDescription(4, true);
	else
		showDescription(5, true);
}

void Room::tug3TalkToMccoy() {
	if (_awayMission->tug.orbitalDecayCounter != 0 || _awayMission->tug.bridgeElasi1Status != GUARDSTAT_UP) {
		if (_awayMission->tug.orbitalDecayCounter >= 10) {
			if (_awayMission->tug.orbitalDecayCounter < 16)
				showText(TX_SPEAKER_MCCOY, 11, true);
		} else if (_awayMission->tug.orbitalDecayCounter != 0) {
			// BUGFIX: original game displays a blank textbox. An appropriate audio file
			// exists, but the corresponding text was written from scratch for ScummVM.
			// TODO: check if original floppy version has text for this.
			showText(TX_SPEAKER_MCCOY, 12, true);
		}
	} else
		showText(TX_SPEAKER_MCCOY, 13, true);
}

void Room::tug3TalkToSpock() {
	if (_awayMission->tug.orbitalDecayCounter != 0) {
		if (_awayMission->tug.orbitalDecayCounter >= 10) {
			if (_awayMission->tug.orbitalDecayCounter < 16)
				showText(TX_SPEAKER_SPOCK, 8, true);
		} else
			showText(TX_SPEAKER_SPOCK, 9, true);
	}
}

void Room::tug3TalkToRedshirt() {
	showText(TX_SPEAKER_CHRISTENSEN, 3, true, true);
}

void Room::tug3UseCommunicator() {
	if (_awayMission->tug.orbitalDecayCounter == 0)
		return;

	if (_awayMission->tug.orbitalDecayCounter < 10) {
		showText(TX_SPEAKER_KIRK, 7, true);
		showText(TX_SPEAKER_SULU, 15, true);
		showText(TX_SPEAKER_SHIPS_COMPUTER, TX_COMPA180);
		_awayMission->timers[1] = 0;

		// BUGFIX: if still fighting the elasi, the mission isn't done yet.
		_awayMission->tug.orbitalDecayCounter = 0;
		if (!_awayMission->tug.elasiSurrendered &&
		        (_awayMission->tug.bridgeElasi1Status == GUARDSTAT_UP
		         || _awayMission->tug.bridgeElasi2Status == GUARDSTAT_UP
		         || _awayMission->tug.bridgeElasi3Status == GUARDSTAT_UP
		         || _awayMission->tug.bridgeElasi4Status == GUARDSTAT_UP))
			return;

		tug3EndMission();
	} else {
		if (_awayMission->tug.orbitalDecayCounter < 16) {
			showText(TX_SPEAKER_KIRK, 6, true);
			showText(TX_SPEAKER_SCOTT, 7 + SCOTTY_MESSAGE_OFFSET, true);

			playMidiMusicTracks(-1, -1);

			_awayMission->disableInput = true;
			if (_awayMission->tug.missionScore < 0)
				_awayMission->tug.missionScore = 0;
			endMission(_awayMission->tug.missionScore, _awayMission->tug.field2b, _awayMission->tug.field2d);
		}
	}
}

// One of the elasi shoots one of the crewmen
void Room::tug3Timer0Expired() {
	const char *beamAnims[][4] = {
		{ "t3beem33", "t3beem35", "t3beem34", "t3beem32" },
		{ "t3beem37", "t3beem39", "t3beem38", "t3beem36" },
		{ "t3beem29", "t3beem31", "t3beem30", "t3beem28" },
	};

	int elasiShooter, elasiTarget;

	if (_awayMission->tug.bridgeElasi2Status == GUARDSTAT_UP)
		elasiShooter = 0;
	else if (_awayMission->tug.bridgeElasi3Status == GUARDSTAT_UP)
		elasiShooter = 1;
	else if (_awayMission->tug.bridgeElasi4Status == GUARDSTAT_UP)
		elasiShooter = 2;
	else
		return;

	if (!_awayMission->tug.crewmanKilled[OBJECT_REDSHIRT]) {
		_awayMission->tug.crewmanKilled[OBJECT_REDSHIRT] = 2;
		_awayMission->redshirtDead = true;
		loadActorAnim2(OBJECT_REDSHIRT, "rkills", -1, -1, 0);
		elasiTarget = OBJECT_REDSHIRT;
	} else if (!_awayMission->tug.crewmanKilled[OBJECT_KIRK]) {
		_awayMission->tug.crewmanKilled[OBJECT_KIRK] = 2;
		_awayMission->disableInput = true;
		loadActorAnim2(OBJECT_KIRK, "kkills", -1, -1, 0);
		elasiTarget = OBJECT_KIRK;
	} else if (!_awayMission->tug.crewmanKilled[OBJECT_SPOCK]) {
		_awayMission->tug.crewmanKilled[OBJECT_SPOCK] = 2;
		loadActorAnim2(OBJECT_SPOCK, "skills", -1, -1, 0);
		elasiTarget = OBJECT_SPOCK;
	} else if (!_awayMission->tug.crewmanKilled[OBJECT_MCCOY]) {
		_awayMission->tug.crewmanKilled[OBJECT_MCCOY] = 2;
		loadActorAnim2(OBJECT_MCCOY, "mkills", -1, -1, 13);
		elasiTarget = OBJECT_MCCOY;
	} else
		return;

	playSoundEffectIndex(kSfxPhaser);
	showBitmapFor5Ticks(beamAnims[elasiShooter][elasiTarget], 5);
	_awayMission->timers[0] = 50;
}

void Room::tug3AllCrewmenDead() {
	playMidiMusicTracks(2, -1);
	showGameOverMenu();
}

// "Orbital decay" countdown timer
void Room::tug3Timer1Expired() {
	if (_awayMission->tug.orbitalDecayCounter < 10) { // Decay still preventable
		showText(TX_SPEAKER_SHIPS_COMPUTER, TX_COMPU182);
		_awayMission->timers[1] = 100;
		_awayMission->tug.orbitalDecayCounter++;
	} else if (_awayMission->tug.orbitalDecayCounter < 16) { // Decay now unavoidable
		showText(TX_SPEAKER_SHIPS_COMPUTER, TX_COMPU181);
		_awayMission->timers[1] = 100;
		_awayMission->tug.orbitalDecayCounter++;
	} else { // Game over
		showDescription(6, true);
		showGameOverMenu();
	}
}

void Room::tug3EndMission() {
	playMidiMusicTracks(28, -1);
	showText(TX_SPEAKER_KIRK, 1, true);
	playSoundEffectIndex(kSfxTransporterMaterialize);
	loadActorAnim2(OBJECT_13, "rteleb", 0x14, 0xa0, 7);
	loadActorAnim2(OBJECT_14, "rteleb", 0x118, 0xa0, 0);
	loadActorAnim2(OBJECT_15, "rteleb", 0x96, 0xbe, 0);
}

void Room::tug3SecurityTeamBeamedIn() {
	loadActorAnim2(OBJECT_13, "rdrawe", -1, -1, 8);
	loadActorAnim2(OBJECT_14, "rdraws", -1, -1, 9);
	loadActorAnim2(OBJECT_15, "rfiren", -1, -1, 10);

	showText(TX_SPEAKER_KIRK, 5, true);
	showText(TX_SPEAKER_SCOTT, 8 + SCOTTY_MESSAGE_OFFSET, true);

	playMidiMusicTracks(-1, -1);
	_awayMission->disableInput = true;
	if (_awayMission->tug.missionScore < 0)
		_awayMission->tug.missionScore = 0;
	endMission(_awayMission->tug.missionScore, _awayMission->tug.field2b, _awayMission->tug.field2d);
}

}
