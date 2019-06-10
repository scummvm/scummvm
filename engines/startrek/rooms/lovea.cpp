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

// Common code for all rooms in LOVE mission.
// TODO: from a cursory glance, this seems to be different in the floppy version?

#include "startrek/room.h"

namespace StarTrek {

// Mccoy or Ferris say something under effects of laughing gas
void Room::loveaTimer0Expired() {
	const TextRef ferrisText[] = {
		TX_LOV0_039,
		TX_LOV0_048,
		TX_LOV0_045,
		TX_LOV0_040,
		TX_LOV0_042,
		TX_LOV0_050,
		TX_LOV0_043,
		TX_LOV0_041,
		TX_LOV0_047,
		TX_LOV0_046,
		TX_LOV0_039
	};

	const TextRef mccoyText[] = {
		TX_LOV0_011,
		TX_LOV0_020,
		TX_LOV0_016,
		TX_LOV0_012,
		TX_LOV0_014,
		TX_LOV0_022,
		TX_LOV0_015,
		TX_LOV0_021,
		TX_LOV0_013,
		TX_LOV0_019,
		TX_LOV0_017
	};

	// BUGFIX: should range from 0-1, not 0-2. Original had "get name errors" when it
	// rolled a 2.
	byte randomVal = getRandomWordInRange(0, 1);
	TextRef speaker;

	const TextRef *textTable;

	if (randomVal == 0 || _awayMission->redshirtDead) {
		speaker = TX_SPEAKER_MCCOY;
		textTable = mccoyText;
	} else {
		speaker = TX_SPEAKER_FERRIS;
		textTable = ferrisText;
	}

	randomVal = getRandomWordInRange(0, 10);

	showText(speaker, textTable[randomVal]);

	if (!_awayMission->love.releasedRomulanLaughingGas) {
		const int spockText[] = {
			TX_LOV0_029,
			TX_LOV0_033,
			TX_LOV0_026,
			TX_LOV0_124
		};
		showText(TX_SPEAKER_SPOCK, spockText[getRandomWordInRange(0, 3)]);

		// BUG(?): This is in an if statement, meaning the human crewmen stop talking from
		// laughing gas if Spock is under laughing gas effects. Might be intentional, to
		// reduce "spamming" of text?
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	}
}

// Spock says something under effects of laughing gas
void Room::loveaTimer1Expired() {
	const int spockText[] = {
		TX_LOV0_025,
		TX_LOV0_101,
		TX_LOV0_102,
		TX_LOV0_103,
		TX_LOV0_104,
		TX_LOV0_105,
		TX_LOV0_106,
		TX_LOV0_107
	};

	showText(TX_SPEAKER_SPOCK, spockText[getRandomWordInRange(0, 7)]);

	_awayMission->timers[1] = getRandomWordInRange(200, 400);
}

void Room::loveaUseMedkitOnSpock() {
	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV5_015);
	else
		showText(TX_SPEAKER_MCCOY, TX_LOV5_019);
}

void Room::loveaUseCureSampleOnSpock() {
	if (_awayMission->love.spockCured) {
		walkCrewman(OBJECT_SPOCK, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock, 99);
		walkCrewman(OBJECT_MCCOY, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock + 10, 99);
	} else
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F55);
}


void Room::loveaUseCureOnSpock() {
	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV5_015);
	else {
		walkCrewman(OBJECT_SPOCK, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock, 99);
		walkCrewman(OBJECT_MCCOY, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock + 10, 99);
	}
}

void Room::loveaSpockOrMccoyInPositionToUseCure() {
	_roomVar.love.spockAndMccoyReadyToUseCure++;
	if (_roomVar.love.spockAndMccoyReadyToUseCure == 2)
		loadActorAnim2(OBJECT_MCCOY, "musemn", -1, -1, 99);
}

void Room::loveaFinishedCuringSpock() {
	showText(TX_SPEAKER_MCCOY, TX_LOV5_030);
	showText(TX_SPEAKER_SPOCK, TX_LOV5_038);
	showText(TX_SPEAKER_MCCOY, TX_LOV5_027);
	showText(TX_SPEAKER_SPOCK, TX_LOV5C001);
	_awayMission->love.spockCured = true;
}


// Timer 2 counts down the time until Spock and the Romulans succumb to the virus.
// BUG(-ish): once Spock is cured, the romulans will never succumb to the virus.
void Room::loveaTimer2Expired() {
	if (!_awayMission->love.spockCured) {
		_awayMission->love.spockInfectionCounter++;
		if (_awayMission->love.spockInfectionCounter >= 100) { // Spock succumbs, game over
			loadActorAnim(OBJECT_SPOCK, "sgetdn", -1, -1, 0);
			playMidiMusicTracks(2, -1); // FIXME: assembly had no second parameter...?
			showDescription(TX_GENER004);
			showGameOverMenu();
		} else if (_awayMission->love.spockInfectionCounter == 15) {
			showText(TX_SPEAKER_SPOCK, TX_SPOKCOFF);
			_awayMission->love.spockInfectionCounter++;
		} else if (_awayMission->love.spockInfectionCounter == 30) {
			showText(TX_SPEAKER_MCCOY, TX_LOVA_F08);
			_awayMission->love.spockInfectionCounter++;
		} else if (_awayMission->love.spockInfectionCounter == 45) {
			showText(TX_SPEAKER_SPOCK, TX_SPOKCOFF);
			_awayMission->love.spockInfectionCounter++;
		} else if (_awayMission->love.spockInfectionCounter == 60) {
			showText(TX_SPEAKER_SPOCK, TX_LOVA_F54);
			_awayMission->love.romulansUnconsciousFromVirus = true;
			_awayMission->love.spockInfectionCounter++;
		}
	}

	_awayMission->timers[2] = 200;
}

void Room::loveaUseMTricorderOnSpock() {
	// Mccoy doesn't do his animation, I guess because he doesn't know which way to face?
	// (possible future enhancement?)
	playSoundEffectIndex(SND_TRICORDER);

	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F01);
	else if (!_awayMission->love.knowAboutVirus)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F02);
	else if (_awayMission->love.spockInfectionCounter < 10)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F04);
	else if (_awayMission->love.spockInfectionCounter < 30)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F03);
	else if (_awayMission->love.spockInfectionCounter < 50)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F08);
	else if (_awayMission->love.spockInfectionCounter < 70) // BUGFIX: < 70 instead of == 70
		showText(TX_SPEAKER_MCCOY, TX_TUG2_010);
	else if (_awayMission->love.spockInfectionCounter < 100)
		showText(TX_SPEAKER_MCCOY, TX_LOVA_F10);
	else
		showText(TX_SPEAKER_MCCOY, TX_LOVA_100);
}

void Room::loveaUseMTricorderOnHuman() {
	playSoundEffectIndex(SND_TRICORDER);
	showText(TX_SPEAKER_MCCOY, TX_LOVA_F07);
}

void Room::loveaUseRomulanLaughingGas() {
	showDescription(TX_LOV2N005);
	_awayMission->love.releasedRomulanLaughingGas = true;
	loseItem(OBJECT_IRLG);

	// BUGFIX: start the effects immediately
	_awayMission->timers[1] = getRandomWordInRange(200, 400);
}

void Room::loveaUseHumanLaughingGas() {
	showDescription(TX_LOV2N005);
	showText(TX_SPEAKER_SPOCK, TX_MUD2_040);
	_awayMission->love.releasedHumanLaughingGas = true;
	loseItem(OBJECT_IN2O);
	playMidiMusicTracks(3, -1); // FIXME: assembly had no second parameter...?

	// BUGFIX: start the effects immediately
	_awayMission->timers[0] = getRandomWordInRange(200, 400);
}

void Room::loveaUseAmmonia() {
	showDescription(TX_LOV2N005);
	// TODO: redshirt says something in floppy edition only
	loseItem(OBJECT_INH3);
}

void Room::loveaUseCommunicator() {
	// There were originally number of branches here, based on whether we've got the cure
	// and whether we've contacted the enterprise already; but all of them branch to the
	// exact same code.
	// TODO: perhaps the floppy edition had different text for each case.

	showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
	showText(TX_SPEAKER_UHURA, TX_BRIDU146);
	showText(TX_SPEAKER_KIRK,  TX_VENA_F41);
	_awayMission->love.contactedEnterpriseBeforeCure = true;
}

}
