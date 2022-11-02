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

// Common code for all rooms in LOVE mission.
// TODO: from a cursory glance, this seems to be different in the floppy version?

#include "startrek/room.h"

namespace StarTrek {

enum loveTextIds {
	TX_COMMON_IDS_OFFSET_START = 5000,	// needed to distinguish common IDs
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_UHURA, TX_SPEAKER_FERRIS,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_050,
	TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104, TX_LOV0_105,
	TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV5_015, TX_LOV5_019,
	TX_LOV5_027, TX_LOV5_030, TX_LOV5_038, TX_LOVA_100, TX_LOVA_F01,
	TX_LOVA_F02, TX_LOVA_F03, TX_LOVA_F04, TX_LOVA_F07, TX_LOVA_F08,
	TX_LOVA_F10, TX_LOVA_F54, TX_LOVA_F55, TX_TUG2_010, TX_MUD2_040,
	TX_LOV2N005, TX_MUD4_018, TX_BRIDU146, TX_VENA_F41, TX_GENER004,
	TX_LOV5C001, TX_SPOKCOFF
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets loveTextOffsets[] = {
	{ TX_SPEAKER_KIRK, 5915, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 5926, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 5936, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 4990, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 5946, 0, 0, 0 },
	{ TX_LOV0_011, 10498, 0, 0, 0 },
	{ TX_LOV0_012, 10625, 0, 0, 0 },
	{ TX_LOV0_013, 10921, 0, 0, 0 },
	{ TX_LOV0_014, 10685, 0, 0, 0 },
	{ TX_LOV0_015, 10789, 0, 0, 0 },
	{ TX_LOV0_016, 10592, 0, 0, 0 },
	{ TX_LOV0_017, 11055, 0, 0, 0 },
	{ TX_LOV0_019, 10975, 0, 0, 0 },
	{ TX_LOV0_020, 10532, 0, 0, 0 },
	{ TX_LOV0_021, 10843, 0, 0, 0 },
	{ TX_LOV0_022, 10733, 0, 0, 0 },
	{ TX_LOV0_025, 11981, 0, 0, 0 },
	{ TX_LOV0_026, 11854, 0, 0, 0 },
	{ TX_LOV0_029, 11734, 0, 0, 0 },
	{ TX_LOV0_033, 11794, 0, 0, 0 },
	{ TX_LOV0_039, 11116, 0, 0, 0 },
	{ TX_LOV0_040, 11243, 0, 0, 0 },
	{ TX_LOV0_041, 11539, 0, 0, 0 },
	{ TX_LOV0_042, 11303, 0, 0, 0 },
	{ TX_LOV0_043, 11407, 0, 0, 0 },
	{ TX_LOV0_045, 11210, 0, 0, 0 },
	{ TX_LOV0_046, 11673, 0, 0, 0 },
	{ TX_LOV0_047, 11593, 0, 0, 0 },
	{ TX_LOV0_048, 11150, 0, 0, 0 },
	{ TX_LOV0_050, 11351, 0, 0, 0 },
	{ TX_LOV0_101, 12062, 0, 0, 0 },
	{ TX_LOV0_102, 12126, 0, 0, 0 },
	{ TX_LOV0_103, 12196, 0, 0, 0 },
	{ TX_LOV0_104, 12246, 0, 0, 0 },
	{ TX_LOV0_105, 12303, 0, 0, 0 },
	{ TX_LOV0_106, 12415, 0, 0, 0 },
	{ TX_LOV0_107, 12483, 0, 0, 0 },
	{ TX_LOV0_124, 11931, 0, 0, 0 },
	{ TX_LOV5_015,  3009, 0, 0, 0 },
	{ TX_LOV5_019,  2931, 0, 0, 0 },
	{ TX_LOV5_027,  3444, 0, 0, 0 },
	{ TX_LOV5_030,  3348, 0, 0, 0 },
	{ TX_LOV5_038,   444, 0, 0, 0 },
	{ TX_LOVA_100,  4553, 0, 0, 0 },
	{ TX_LOVA_F01,  3995, 0, 0, 0 },
	{ TX_LOVA_F02,  4067, 0, 0, 0 },
	{ TX_LOVA_F03,  4266, 0, 0, 0 },
	{ TX_LOVA_F04,  4181, 0, 0, 0 },
	{ TX_LOVA_F07,  4630, 0, 0, 0 },
	{ TX_LOVA_F08,  3619, 0, 0, 0 },
	{ TX_LOVA_F10,  4501, 0, 0, 0 },
	{ TX_LOVA_F54,  3775, 0, 0, 0 },
	{ TX_LOVA_F55,  3076, 0, 0, 0 },
	{ TX_TUG2_010,  4431, 0, 0, 0 },
	{ TX_MUD2_040,  4826, 0, 0, 0 },
	{ TX_LOV2N005,  4689, 0, 0, 0 },
	{ TX_MUD4_018,  5035, 0, 0, 0 },
	{ TX_BRIDU146,  5104, 0, 0, 0 },
	{ TX_VENA_F41,  5158, 0, 0, 0 },
	{ TX_LOV5C001,  3497, 0, 0, 0 },
	{ TX_SPOKCOFF,  3564, 0, 0, 0 },
	{ TX_GENER004,  3924, 0, 0, 0 },	// Game over
	{          -1,  0,    0, 0, 0 }
};

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
	playSoundEffectIndex(kSfxTricorder);

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
	playSoundEffectIndex(kSfxTricorder);
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
