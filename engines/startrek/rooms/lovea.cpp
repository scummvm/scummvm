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
	const TextRef ferrisText[] = { 39, 48, 45, 40, 42, 50, 43, 41, 47, 46, 39 };
	const TextRef mccoyText[] = { 11, 20, 16, 12, 14, 22, 15, 21, 13, 19, 17 };

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

	showText(speaker, COMMON_MESSAGE_OFFSET + textTable[randomVal], true);

	if (!_awayMission->love.releasedRomulanLaughingGas) {
		const int spockText[] = { 29, 33, 26, 124 };
		showText(TX_SPEAKER_SPOCK, COMMON_MESSAGE_OFFSET + spockText[getRandomWordInRange(0, 3)], true);

		// BUG(?): This is in an if statement, meaning the human crewmen stop talking from
		// laughing gas if Spock is under laughing gas effects. Might be intentional, to
		// reduce "spamming" of text?
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	}
}

// Spock says something under effects of laughing gas
void Room::loveaTimer1Expired() {
	const int spockText[] = { 25, 101, 102, 103, 104, 105, 106, 107 };
	showText(TX_SPEAKER_SPOCK, COMMON_MESSAGE_OFFSET + spockText[getRandomWordInRange(0, 7)], true);
	_awayMission->timers[1] = getRandomWordInRange(200, 400);
}

void Room::loveaUseMedkitOnSpock() {
	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 15, true);
	else
		showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 19, true);
}

void Room::loveaUseCureSampleOnSpock() {
	if (_awayMission->love.spockCured) {
		walkCrewman(OBJECT_SPOCK, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock, 99);
		walkCrewman(OBJECT_MCCOY, _roomVar.love.cmnXPosToCureSpock, _roomVar.love.cmnYPosToCureSpock + 10, 99);
	} else
		showText(TX_SPEAKER_MCCOY, 55 + COMMON_MESSAGE_OFFSET, true);
}


void Room::loveaUseCureOnSpock() {
	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 15, true);
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
	showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 30, true);
	showText(TX_SPEAKER_SPOCK, COMMON_MESSAGE_OFFSET + 38, true);
	showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 27, true);
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
			showText(TX_SPEAKER_MCCOY, 8 + COMMON_MESSAGE_OFFSET, true);
			_awayMission->love.spockInfectionCounter++;
		} else if (_awayMission->love.spockInfectionCounter == 45) {
			showText(TX_SPEAKER_SPOCK, TX_SPOKCOFF);
			_awayMission->love.spockInfectionCounter++;
		} else if (_awayMission->love.spockInfectionCounter == 60) {
			showText(TX_SPEAKER_SPOCK, 54 + COMMON_MESSAGE_OFFSET, true);
			_awayMission->love.romulansUnconsciousFromVirus = true;
			_awayMission->love.spockInfectionCounter++;
		}
	}

	_awayMission->timers[2] = 200;
}

void Room::loveaUseMTricorderOnSpock() {
	// Mccoy doesn't do his animation, I guess because he doesn't know which way to face?
	// (possible future enhancement?)
	playSoundEffectIndex(kSfxTricorder);

	if (_awayMission->love.spockCured)
		showText(TX_SPEAKER_MCCOY, 1 + COMMON_MESSAGE_OFFSET, true);
	else if (!_awayMission->love.knowAboutVirus)
		showText(TX_SPEAKER_MCCOY, 2 + COMMON_MESSAGE_OFFSET, true);
	else if (_awayMission->love.spockInfectionCounter < 10)
		showText(TX_SPEAKER_MCCOY, 4 + COMMON_MESSAGE_OFFSET, true);
	else if (_awayMission->love.spockInfectionCounter < 30)
		showText(TX_SPEAKER_MCCOY, 3 + COMMON_MESSAGE_OFFSET, true);
	else if (_awayMission->love.spockInfectionCounter < 50)
		showText(TX_SPEAKER_MCCOY, 8 + COMMON_MESSAGE_OFFSET, true);
	else if (_awayMission->love.spockInfectionCounter < 70) // BUGFIX: < 70 instead of == 70
		showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 10, true);	// TX_TUG2_010
	else if (_awayMission->love.spockInfectionCounter < 100)
		showText(TX_SPEAKER_MCCOY, 10 + COMMON_MESSAGE_OFFSET, true);
	else
		showText(TX_SPEAKER_MCCOY, COMMON_MESSAGE_OFFSET + 100);
}

void Room::loveaUseMTricorderOnHuman() {
	playSoundEffectIndex(kSfxTricorder);
	showText(TX_SPEAKER_MCCOY, 7 + COMMON_MESSAGE_OFFSET, true);
}

void Room::loveaUseRomulanLaughingGas() {
	showDescription(COMMON_MESSAGE_OFFSET + 2, true);
	_awayMission->love.releasedRomulanLaughingGas = true;
	loseItem(OBJECT_IRLG);

	// BUGFIX: start the effects immediately
	_awayMission->timers[1] = getRandomWordInRange(200, 400);
}

void Room::loveaUseHumanLaughingGas() {
	showDescription(COMMON_MESSAGE_OFFSET + 5, true);
	showText(TX_SPEAKER_SPOCK, TX_MUD2_040);
	_awayMission->love.releasedHumanLaughingGas = true;
	loseItem(OBJECT_IN2O);
	playMidiMusicTracks(3, -1); // FIXME: assembly had no second parameter...?

	// BUGFIX: start the effects immediately
	_awayMission->timers[0] = getRandomWordInRange(200, 400);
}

void Room::loveaUseAmmonia() {
	showDescription(COMMON_MESSAGE_OFFSET + 5, true);
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
	showText(TX_SPEAKER_KIRK,  41 + COMMON_MESSAGE_OFFSET, true);
	_awayMission->love.contactedEnterpriseBeforeCure = true;
}

}
