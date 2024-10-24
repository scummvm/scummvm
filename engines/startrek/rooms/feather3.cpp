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

#define OBJECT_TLAOXAC 8
#define OBJECT_KNIFE 9
#define OBJECT_ROCK 10

#define HOTSPOT_LEFT_EXIT 0x20
#define HOTSPOT_LIGHT_1 0x21
#define HOTSPOT_LIGHT_2 0x22

#define KNIFE_X 0x64
#define KNIFE_Y 0xb4

namespace StarTrek {

extern const RoomAction feather3ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0},                 &Room::feather3Tick1 },
	{ {ACTION_TICK, 40, 0, 0, 0},                &Room::feather3Tick40 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0},      &Room::feather3TouchedHotspot0 },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_TLAOXAC, 0, 0}, &Room::feather3UseSpockOnTlaoxac },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_TLAOXAC, 0, 0}, &Room::feather3UseMccoyOnTlaoxac },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_TLAOXAC, 0, 0}, &Room::feather3UseRedshirtOnTlaoxac },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_LEFT_EXIT, 0, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_LEFT_EXIT, 0, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_LEFT_EXIT, 0, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_ICOMM, 0, 0, 0}, &Room::feather3UseCommunicator },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0, 0},     &Room::feather3UseSnakeOnKirk },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0, 0},    &Room::feather3UseSnakeOnSpock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0, 0}, &Room::feather3UseSnakeOnRedshirt },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0, 0},              &Room::feather3KirkReachedRedshirtWithSnake },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0, 0},    &Room::feather3UseSnakeOnMccoy },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_TLAOXAC, 0, 0},  &Room::feather3UseSnakeOnTlaoxac },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_SPOCK, 0, 0},    &Room::feather3UseKnifeOnSpock },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_MCCOY, 0, 0},    &Room::feather3UseKnifeOnMccoy },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_REDSHIRT, 0, 0}, &Room::feather3UseKnifeOnRedshirt },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_TLAOXAC, 0, 0},   &Room::feather3UseRockOnTlaoxac },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0, 0},              &Room::feather3KirkReachedPositionToThrowRock },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0, 0},                 &Room::feather3Timer4Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0},                 &Room::feather3Timer2Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0, 0},                 &Room::feather3Timer3Expired },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0},            &Room::feather3TlaoxacKnockedOutFromRockThrow },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0},          &Room::feather3UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0},          &Room::feather3UsePhaser },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0},               &Room::feather3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0},               &Room::feather3TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0},            &Room::feather3TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_TLAOXAC, 0, 0, 0},             &Room::feather3TalkToTlaoxac },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0},            &Room::feather3TlaoxacKilledRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_TLAOXAC, 0, 0}, &Room::feather3UseMTricorderOnTlaoxac },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_TLAOXAC, 0, 0}, &Room::feather3UseMedkitOnTlaoxac },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_REDSHIRT, 0, 0}, &Room::feather3UseMedkitOnRedshirt },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},           &Room::feather3UseMedkitAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_1, 0, 0}, &Room::feather3UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_2, 0, 0}, &Room::feather3UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_1, 0, 0}, &Room::feather3UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_2, 0, 0}, &Room::feather3UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0},            &Room::feather3UseSTricorderAnywhere },
	{ {ACTION_GET, OBJECT_KNIFE, 0, 0, 0},                  &Room::feather3GetKnife },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0, 0},                &Room::feather3ReachedKnife },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0},                   &Room::feather3Timer1Expired },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0, 0},              &Room::feather3PickedUpKnife },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0},                 &Room::feather3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0},              &Room::feather3LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_TLAOXAC, 0, 0, 0},               &Room::feather3LookAtTlaoxac },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_1, 0, 0, 0},              &Room::feather3LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_2, 0, 0, 0},              &Room::feather3LookAtLight },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0},                 &Room::feather3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_KNIFE, 0, 0, 0},                 &Room::feather3LookAtKnife },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather3TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY, TX_SPEAKER_TLAOXAC,
	TX_FEA3_001, TX_FEA3_002, TX_FEA3_003, TX_FEA3_004, TX_FEA3_005,
	TX_FEA3_006, TX_FEA3_007, TX_FEA3_008, TX_FEA3_009, TX_FEA3_010,
	TX_FEA3_011, TX_FEA3_012, TX_FEA3_013, TX_FEA3_014, TX_FEA3_015,
	TX_FEA3_016, TX_FEA3_017, TX_FEA3_018, TX_FEA3_019, TX_FEA3_020,
	TX_FEA3_021, TX_FEA3_022, TX_FEA3_023, TX_FEA3_024, TX_FEA3_025,
	TX_FEA3_026, TX_FEA3_027, TX_FEA3_028, TX_FEA3_029, TX_FEA3_030,
	TX_FEA3_031, TX_FEA3_032, TX_FEA3_033, TX_FEA3_034, TX_FEA3_035,
	TX_FEA3_036, TX_FEA3_037, TX_FEA3_038, TX_FEA3_039, TX_FEA3_040,
	TX_FEA3_041, TX_FEA3_042, TX_FEA3_043, TX_FEA3_044,	TX_FEA3N000,
	TX_FEA3N001, TX_FEA3N002, TX_FEA3N003, TX_FEA3N004, TX_FEA3N005,
	TX_FEA3N006, TX_FEA3N007, TX_FEA3N010, TX_FEA3N011, TX_FEA3N012,
	TX_LOVA_100, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather3TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 2681, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 2692, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 2702, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 2712, 0, 0, 0 },
	{ TX_SPEAKER_TLAOXAC, 2724, 0, 0, 0 },
	{ TX_FEA3_001, 5973, 0, 0, 0 },
	{ TX_FEA3_002, 6028, 0, 0, 0 },
	{ TX_FEA3_003, 6157, 0, 0, 0 },
	{ TX_FEA3_004, 4518, 0, 0, 0 },
	{ TX_FEA3_005, 5799, 0, 0, 0 },
	{ TX_FEA3_006, 1987, 0, 0, 0 },
	{ TX_FEA3_007, 4639, 0, 0, 0 },
	{ TX_FEA3_008, 2843, 0, 0, 0 },
	{ TX_FEA3_009, 5281, 0, 0, 0 },
	{ TX_FEA3_010, 6762, 0, 0, 0 },
	{ TX_FEA3_011, 5418, 0, 0, 0 },
	{ TX_FEA3_012, 1792, 0, 0, 0 },
	{ TX_FEA3_013, 5619, 0, 0, 0 },
	{ TX_FEA3_014, 6616, 0, 0, 0 },
	{ TX_FEA3_015, 3671, 0, 0, 0 },
	{ TX_FEA3_016, 3704, 0, 0, 0 },
	{ TX_FEA3_017, 3849, 0, 0, 0 },
	{ TX_FEA3_018, 3551, 0, 0, 0 },
	{ TX_FEA3_019, 5496, 0, 0, 0 },
	{ TX_FEA3_020, 4360, 0, 0, 0 },
	{ TX_FEA3_021, 6832, 0, 0, 0 },
	{ TX_FEA3_022, 5695, 0, 0, 0 },
	{ TX_FEA3_023, 6936, 0, 0, 0 },
	{ TX_FEA3_024, 6979, 0, 0, 0 },
	{ TX_FEA3_025, 5367, 0, 0, 0 },
	{ TX_FEA3_026, 3470, 0, 0, 0 },
	{ TX_FEA3_027, 3780, 0, 0, 0 },
	{ TX_FEA3_028, 3602, 0, 0, 0 },
	{ TX_FEA3_029, 3890, 0, 0, 0 },
	{ TX_FEA3_030, 6800, 0, 0, 0 },
	{ TX_FEA3_031, 4683, 0, 0, 0 },
	{ TX_FEA3_032, 5871, 0, 0, 0 },
	{ TX_FEA3_033, 5470, 0, 0, 0 },
	{ TX_FEA3_034, 4757, 0, 0, 0 },
	{ TX_FEA3_035, 4032, 0, 0, 0 },
	{ TX_FEA3_036, 2733, 0, 0, 0 },
	{ TX_FEA3_037, 6278, 0, 0, 0 },
	{ TX_FEA3_038, 5916, 0, 0, 0 },
	{ TX_FEA3_039, 5108, 0, 0, 0 },
	{ TX_FEA3_040, 6493, 0, 0, 0 },
	{ TX_FEA3_041, 4107, 0, 0, 0 },
	{ TX_FEA3_042, 6387, 0, 0, 0 },
	{ TX_FEA3_043, 4207, 0, 0, 0 },
	{ TX_FEA3_044, 4882, 0, 0, 0 },
	{ TX_FEA3N000, 3092, 0, 0, 0 },
	{ TX_FEA3N001, 3030, 0, 0, 0 },
	{ TX_FEA3N002, 3135, 0, 0, 0 },
	{ TX_FEA3N003, 3344, 0, 0, 0 },
	{ TX_FEA3N004, 4158, 0, 0, 0 },
	{ TX_FEA3N005, 2971, 0, 0, 0 },
	{ TX_FEA3N006, 5225, 0, 0, 0 },
	{ TX_FEA3N007, 3199, 0, 0, 0 },
	{ TX_FEA3N010, 6878, 0, 0, 0 },
	{ TX_FEA3N011, 3408, 0, 0, 0 },
	{ TX_FEA3N012, 2883, 0, 0, 0 },
	{ TX_LOVA_100, 8620, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 7022, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather3Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather3Tick1() {
	playVoc("FEA3LOOP");
	playMidiMusicTracks(MIDITRACK_27);

	if (!_awayMission->feather.tlaoxacTestPassed) {
		loadActorAnim(OBJECT_TLAOXAC, "s5r3as", 0x2e, 0xab);
		loadMapFile("feath3b");
	} else if (!_awayMission->feather.tookKnife)
		loadActorAnim(OBJECT_KNIFE, "s5r3nf", 0x64, 0xb4);
}

void Room::feather3Tick40() {
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, TX_FEA3_036);
}

void Room::feather3TouchedHotspot0() { // Tlaoxac prevents you from passing this point
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, TX_FEA3_035);
}

void Room::feather3UseSpockOnTlaoxac() {
	showText(TX_SPEAKER_SPOCK, TX_FEA3_026);
	showText(TX_SPEAKER_MCCOY, TX_FEA3_018);
	showText(TX_SPEAKER_SPOCK, TX_FEA3_028);
	showText(TX_SPEAKER_MCCOY, TX_FEA3_015);
}

void Room::feather3UseMccoyOnTlaoxac() {
	showText(TX_SPEAKER_MCCOY, TX_FEA3_016);
	showText(TX_SPEAKER_SPOCK, TX_FEA3_027);
	showText(TX_SPEAKER_MCCOY, TX_FEA3_017);
}

void Room::feather3UseRedshirtOnTlaoxac() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, TX_FEA3_029);
}

void Room::feather3UseCrewmanOnLeftExit() {
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, TX_FEA3_041);
}

void Room::feather3UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, TX_FEA3_024);
}

void Room::feather3UseSnakeOnKirk() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead) {
		if (_roomVar.feather.showedSnakeToTlaoxac && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
			showText(TX_SPEAKER_TLAOXAC, TX_FEA3_043);
			_awayMission->feather.tlaoxacTestPassed = true;
			_awayMission->feather.missionScore += 2;
			loadMapFile("feather3");
			loadActorAnim(OBJECT_KNIFE, "s5r3nf", KNIFE_X, KNIFE_Y);
			_awayMission->feather.field2e = true;
		} else
			showDescription(TX_FEA3N004);
	}
}

void Room::feather3UseSnakeOnSpock() {
	if (_roomVar.feather.showedSnakeToTlaoxac)
		showText(TX_SPEAKER_SPOCK, TX_FEA3_004);
	else
		showText(TX_SPEAKER_SPOCK, TX_FEA3_020);
}

void Room::feather3UseSnakeOnRedshirt() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead) {
		if (_roomVar.feather.showedSnakeToTlaoxac && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
			walkCrewmanC(OBJECT_KIRK, 0x7c, 0xbc, &Room::feather3KirkReachedRedshirtWithSnake);
			_awayMission->disableInput = true;
		} else
			showText(TX_SPEAKER_STRAGEY, TX_FEA3_031);
	}
}

void Room::feather3KirkReachedRedshirtWithSnake() {
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_STRAGEY, TX_FEA3_034);
	showText(TX_SPEAKER_TLAOXAC, TX_FEA3_044);

	_awayMission->feather.tlaoxacTestPassed = true;
	_awayMission->feather.missionScore += 1;
	loadMapFile("feather3");
	loadActorAnim(OBJECT_KNIFE, "s5r3nf", KNIFE_X, KNIFE_Y);
	_awayMission->feather.field2e = true;
}

void Room::feather3UseSnakeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA3_007);
}

void Room::feather3UseSnakeOnTlaoxac() {
	if (!_awayMission->feather.tlaoxacTestPassed && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
		showText(TX_SPEAKER_TLAOXAC, TX_FEA3_039);
		_roomVar.feather.showedSnakeToTlaoxac = true;
		if (!_awayMission->feather.showedSnakeToTlaoxac) {
			_awayMission->feather.showedSnakeToTlaoxac = true;
			_awayMission->feather.missionScore += 1;
		}
	}
}

void Room::feather3UseKnifeOnSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA3_025);
}

void Room::feather3UseKnifeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA3_011); // BUGFIX: Speaker is Mccoy, not Spock.
}

void Room::feather3UseKnifeOnRedshirt() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, TX_FEA3_033); // BUGFIX: Speaker is Stragey, not Spock.
}

void Room::feather3UseRockOnTlaoxac() {
	if (_roomVar.feather.tlaoxacUnconscious)
		showText(TX_SPEAKER_MCCOY, TX_FEA3_009);
	else {
		_awayMission->feather.numRocksThrownAtTlaoxac++;
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0xac, 0xb4, &Room::feather3KirkReachedPositionToThrowRock);
	}
}

void Room::feather3KirkReachedPositionToThrowRock() {
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_W;
	loadActorAnim2(OBJECT_KIRK, "s5r3kt");
	loadActorAnim(OBJECT_ROCK, "s5r3rt", 0x80, 0x7e);
	_awayMission->timers[4] = 21;

	if (_awayMission->feather.numRocksThrownAtTlaoxac == 2) {
		_awayMission->feather.knockedOutTlaoxac = true;
		_awayMission->feather.missionScore -= 2;
		loadActorAnimC(OBJECT_TLAOXAC, "s5r3ad", -1, -1, &Room::feather3TlaoxacKnockedOutFromRockThrow);
		playVoc("ROCKFACE");
		_awayMission->timers[2] = 24;
	} else {
		loadActorAnim2(OBJECT_TLAOXAC, "s5r3ah");
		playVoc("ROCKFACE");
		_awayMission->timers[3] = 97;
	}
}

void Room::feather3Timer4Expired() {
}

void Room::feather3Timer2Expired() {
	if (!_awayMission->feather.tookKnife)
		loadActorAnim(OBJECT_KNIFE, "s5r3nf", KNIFE_X, KNIFE_Y);
}

void Room::feather3Timer3Expired() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
}

void Room::feather3TlaoxacKnockedOutFromRockThrow() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(TX_FEA3N006);

	if (!_awayMission->feather.tookKnife && !_awayMission->feather.field2e)
		showText(TX_SPEAKER_MCCOY, TX_FEA3_008);

	_awayMission->feather.tlaoxacTestPassed = true;
	_roomVar.feather.tlaoxacUnconscious = true;
	loadMapFile("feather3");
}

void Room::feather3UsePhaser() {
	showText(TX_SPEAKER_SPOCK, TX_FEA3_023);
}

void Room::feather3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA3_005);
}

void Room::feather3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, TX_FEA3_022);
}

void Room::feather3TalkToRedshirt() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, TX_FEA3_032);
}

void Room::feather3TalkToTlaoxac() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead && !_roomVar.feather.tlaoxacUnconscious) {
		if (!_roomVar.feather.showedSnakeToTlaoxac)
			showText(TX_SPEAKER_TLAOXAC, TX_FEA3_038);
		else if (_awayMission->feather.numRocksThrownAtTlaoxac != 0)
			showText(TX_SPEAKER_TLAOXAC, TX_FEA3_038);
		else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_FEA3_001, TX_FEA3_002, TX_FEA3_003,
				TX_END
			};
			int choice = showMultipleTexts(choices);

			if (choice == 0)
				showText(TX_SPEAKER_TLAOXAC, TX_FEA3_037);
			else if (choice == 1)
				showText(TX_SPEAKER_TLAOXAC, TX_FEA3_042);
			else if (choice == 2) { // Accidentally insulted Quetzecoatl
				showText(TX_SPEAKER_TLAOXAC, TX_FEA3_040);
				_awayMission->disableInput = true;
				loadActorAnimC(OBJECT_TLAOXAC, "s5r3aj", -1, -1, &Room::feather3TlaoxacKilledRedshirt);
				playMidiMusicTracks(MIDITRACK_2);
				loadActorAnim(OBJECT_REDSHIRT, "s5r3rd");
				_awayMission->redshirtDead = true;
			} else
				showDescription(TX_DIALOG_ERROR);
		}
	}
}

void Room::feather3TlaoxacKilledRedshirt() {
	_awayMission->disableInput = false;
}

void Room::feather3UseMTricorderOnTlaoxac() {
	if (_roomVar.feather.tlaoxacUnconscious)
		mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA3_010);
	else
		mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA3_014);
}

void Room::feather3UseMedkitOnTlaoxac() {
	if (_awayMission->feather.knockedOutTlaoxac)
		showText(TX_SPEAKER_MCCOY, TX_FEA3_012);
}

void Room::feather3UseMedkitOnRedshirt() {
	if (_awayMission->redshirtDead) {
		mccoyScan(DIR_N, TX_SPEAKER_MCCOY, TX_FEA3_030);
	}
}

void Room::feather3UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_FEA3_006);
}

void Room::feather3UseMTricorderOnLight() {
	mccoyScan(DIR_W, TX_SPEAKER_MCCOY, TX_FEA3_013);
}

void Room::feather3UseSTricorderOnLight() {
	spockScan(DIR_W, TX_SPEAKER_SPOCK, TX_FEA3_019);
}

void Room::feather3UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA3_021);
}

void Room::feather3GetKnife() {
	_awayMission->feather.missionScore += 1;
	walkCrewmanC(OBJECT_KIRK, 0x57, 0xb4, &Room::feather3ReachedKnife);
	_awayMission->disableInput = true;
}

void Room::feather3ReachedKnife() {
	loadActorAnimC(OBJECT_KIRK, "s5r1kg", -1, -1, &Room::feather3PickedUpKnife);
	_awayMission->timers[1] = 20;
}

void Room::feather3Timer1Expired() {
	giveItem(OBJECT_IKNIFE);
	loadActorStandAnim(OBJECT_KNIFE);
}

void Room::feather3PickedUpKnife() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	showDescription(TX_FEA3N010);
}

void Room::feather3LookAtSpock() {
	if (!_awayMission->feather.knockedOutTlaoxac && !_awayMission->feather.tlaoxacTestPassed)
		showDescription(TX_FEA3N002);
	else // ENHANCEMENT: Originally did nothing here. Fall back to default behaviour in the engine.
		_awayMission->rdfStillDoDefaultAction = true;
}

void Room::feather3LookAtRedshirt() {
	if (_awayMission->redshirtDead)
		showDescription(TX_FEA3N005);
	else
		showDescription(TX_FEA3N012);
}

void Room::feather3LookAtTlaoxac() {
	if (_awayMission->feather.knockedOutTlaoxac)
		showDescription(TX_FEA3N003);
	else
		showDescription(TX_FEA3N007);
}

void Room::feather3LookAtLight() {
	showDescription(TX_FEA3N001);
}

void Room::feather3LookAtMccoy() {
	showDescription(TX_FEA3N000);
}

void Room::feather3LookAtKnife() {
	showDescription(TX_FEA3N011);
}

}
