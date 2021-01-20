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
	{ {ACTION_TICK, 1, 0, 0},                 &Room::feather3Tick1 },
	{ {ACTION_TICK, 40, 0, 0},                &Room::feather3Tick40 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0},      &Room::feather3TouchedHotspot0 },
	{ {ACTION_USE, OBJECT_SPOCK,    OBJECT_TLAOXAC, 0}, &Room::feather3UseSpockOnTlaoxac },
	{ {ACTION_USE, OBJECT_MCCOY,    OBJECT_TLAOXAC, 0}, &Room::feather3UseMccoyOnTlaoxac },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_TLAOXAC, 0}, &Room::feather3UseRedshirtOnTlaoxac },
	{ {ACTION_USE, OBJECT_SPOCK,    HOTSPOT_LEFT_EXIT, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_MCCOY,    HOTSPOT_LEFT_EXIT, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_REDSHIRT, HOTSPOT_LEFT_EXIT, 0}, &Room::feather3UseCrewmanOnLeftExit },
	{ {ACTION_USE, OBJECT_ICOMM, 0, 0}, &Room::feather3UseCommunicator },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_KIRK, 0},     &Room::feather3UseSnakeOnKirk },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_SPOCK, 0},    &Room::feather3UseSnakeOnSpock },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_REDSHIRT, 0}, &Room::feather3UseSnakeOnRedshirt },
	{ {ACTION_FINISHED_WALKING, 2, 0, 0},              &Room::feather3KirkReachedRedshirtWithSnake },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_MCCOY, 0},    &Room::feather3UseSnakeOnMccoy },
	{ {ACTION_USE, OBJECT_ISNAKE, OBJECT_TLAOXAC, 0},  &Room::feather3UseSnakeOnTlaoxac },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_SPOCK, 0},    &Room::feather3UseKnifeOnSpock },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_MCCOY, 0},    &Room::feather3UseKnifeOnMccoy },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_REDSHIRT, 0}, &Room::feather3UseKnifeOnRedshirt },
	{ {ACTION_USE, OBJECT_IROCK, OBJECT_TLAOXAC, 0},   &Room::feather3UseRockOnTlaoxac },
	{ {ACTION_FINISHED_WALKING, 3, 0, 0},              &Room::feather3KirkReachedPositionToThrowRock },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0},                 &Room::feather3Timer4Expired },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0},                 &Room::feather3Timer2Expired },
	{ {ACTION_TIMER_EXPIRED, 3, 0, 0},                 &Room::feather3Timer3Expired },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0},            &Room::feather3TlaoxacKnockedOutFromRockThrow },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0},          &Room::feather3UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0},          &Room::feather3UsePhaser },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0},               &Room::feather3TalkToMccoy },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0},               &Room::feather3TalkToSpock },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0},            &Room::feather3TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_TLAOXAC, 0, 0},             &Room::feather3TalkToTlaoxac },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0},            &Room::feather3TlaoxacKilledRedshirt },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_TLAOXAC, 0}, &Room::feather3UseMTricorderOnTlaoxac },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_TLAOXAC, 0}, &Room::feather3UseMedkitOnTlaoxac },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_REDSHIRT, 0}, &Room::feather3UseMedkitOnRedshirt },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0},           &Room::feather3UseMedkitAnywhere },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_1, 0}, &Room::feather3UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_LIGHT_2, 0}, &Room::feather3UseMTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_1, 0}, &Room::feather3UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_LIGHT_2, 0}, &Room::feather3UseSTricorderOnLight },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0},            &Room::feather3UseSTricorderAnywhere },
	{ {ACTION_GET, OBJECT_KNIFE, 0, 0},                  &Room::feather3GetKnife },
	{ {ACTION_FINISHED_WALKING, 1, 0, 0},                &Room::feather3ReachedKnife },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0},                   &Room::feather3Timer1Expired },
	{ {ACTION_FINISHED_ANIMATION, 1, 0, 0},              &Room::feather3PickedUpKnife },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0},                 &Room::feather3LookAtSpock },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0},              &Room::feather3LookAtRedshirt },
	{ {ACTION_LOOK, OBJECT_TLAOXAC, 0, 0},               &Room::feather3LookAtTlaoxac },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_1, 0, 0},              &Room::feather3LookAtLight },
	{ {ACTION_LOOK, HOTSPOT_LIGHT_2, 0, 0},              &Room::feather3LookAtLight },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0},                 &Room::feather3LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_KNIFE, 0, 0},                 &Room::feather3LookAtKnife },
	{ {ACTION_LIST_END, 0, 0, 0}, nullptr }
};

void Room::feather3Tick1() {
	playVoc("FEA3LOOP");
	playMidiMusicTracks(27);

	if (!_awayMission->feather.tlaoxacTestPassed) {
		loadActorAnim(OBJECT_TLAOXAC, "s5r3as", 0x2e, 0xab);
		loadMapFile("feath3b");
	} else if (!_awayMission->feather.tookKnife)
		loadActorAnim(OBJECT_KNIFE, "s5r3nf", 0x64, 0xb4);
}

void Room::feather3Tick40() {
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, 36, true);
}

void Room::feather3TouchedHotspot0() { // Tlaoxac prevents you from passing this point
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, 35, true);
}

void Room::feather3UseSpockOnTlaoxac() {
	showText(TX_SPEAKER_SPOCK, 26, true);
	showText(TX_SPEAKER_MCCOY, 18, true);
	showText(TX_SPEAKER_SPOCK, 28, true);
	showText(TX_SPEAKER_MCCOY, 15, true);
}

void Room::feather3UseMccoyOnTlaoxac() {
	showText(TX_SPEAKER_MCCOY, 16, true);
	showText(TX_SPEAKER_SPOCK, 27, true);
	showText(TX_SPEAKER_MCCOY, 17, true);
}

void Room::feather3UseRedshirtOnTlaoxac() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, 29, true);
}

void Room::feather3UseCrewmanOnLeftExit() {
	if (!_awayMission->feather.tlaoxacTestPassed)
		showText(TX_SPEAKER_TLAOXAC, 41, true);
}

void Room::feather3UseCommunicator() {
	showText(TX_SPEAKER_SPOCK, 24, true);
}

void Room::feather3UseSnakeOnKirk() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead) {
		if (_roomVar.feather.showedSnakeToTlaoxac && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
			showText(TX_SPEAKER_TLAOXAC, 43, true);
			_awayMission->feather.tlaoxacTestPassed = true;
			_awayMission->feather.missionScore += 2;
			loadMapFile("feather3");
			loadActorAnim(OBJECT_KNIFE, "s5r3nf", KNIFE_X, KNIFE_Y);
			_awayMission->feather.field2e = true;
		} else
			showDescription(4, true);
	}
}

void Room::feather3UseSnakeOnSpock() {
	if (_roomVar.feather.showedSnakeToTlaoxac)
		showText(TX_SPEAKER_SPOCK, 4, true);
	else
		showText(TX_SPEAKER_SPOCK, 20, true);
}

void Room::feather3UseSnakeOnRedshirt() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead) {
		if (_roomVar.feather.showedSnakeToTlaoxac && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
			walkCrewmanC(OBJECT_KIRK, 0x7c, 0xbc, &Room::feather3KirkReachedRedshirtWithSnake);
			_awayMission->disableInput = true;
		} else
			showText(TX_SPEAKER_STRAGEY, 31, true);
	}
}

void Room::feather3KirkReachedRedshirtWithSnake() {
	_awayMission->disableInput = false;

	showText(TX_SPEAKER_STRAGEY, 34, true);
	showText(TX_SPEAKER_TLAOXAC, 44, true);

	_awayMission->feather.tlaoxacTestPassed = true;
	_awayMission->feather.missionScore += 1;
	loadMapFile("feather3");
	loadActorAnim(OBJECT_KNIFE, "s5r3nf", KNIFE_X, KNIFE_Y);
	_awayMission->feather.field2e = true;
}

void Room::feather3UseSnakeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 7, true);
}

void Room::feather3UseSnakeOnTlaoxac() {
	if (!_awayMission->feather.tlaoxacTestPassed && _awayMission->feather.numRocksThrownAtTlaoxac == 0) {
		showText(TX_SPEAKER_TLAOXAC, 39, true);
		_roomVar.feather.showedSnakeToTlaoxac = true;
		if (!_awayMission->feather.showedSnakeToTlaoxac) {
			_awayMission->feather.showedSnakeToTlaoxac = true;
			_awayMission->feather.missionScore += 1;
		}
	}
}

void Room::feather3UseKnifeOnSpock() {
	showText(TX_SPEAKER_SPOCK, 25, true);
}

void Room::feather3UseKnifeOnMccoy() {
	showText(TX_SPEAKER_MCCOY, 11, true); // BUGFIX: Speaker is Mccoy, not Spock.
}

void Room::feather3UseKnifeOnRedshirt() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, 33, true); // BUGFIX: Speaker is Stragey, not Spock.
}

void Room::feather3UseRockOnTlaoxac() {
	if (_roomVar.feather.tlaoxacUnconscious)
		showText(TX_SPEAKER_MCCOY, 9, true);
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
	showDescription(6, true);

	if (!_awayMission->feather.tookKnife && !_awayMission->feather.field2e)
		showText(TX_SPEAKER_MCCOY, 8, true);

	_awayMission->feather.tlaoxacTestPassed = true;
	_roomVar.feather.tlaoxacUnconscious = true;
	loadMapFile("feather3");
}

void Room::feather3UsePhaser() {
	showText(TX_SPEAKER_SPOCK, 23, true);
}

void Room::feather3TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, 5, true);
}

void Room::feather3TalkToSpock() {
	showText(TX_SPEAKER_SPOCK, 22, true);
}

void Room::feather3TalkToRedshirt() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_STRAGEY, 32, true);
}

void Room::feather3TalkToTlaoxac() {
	if (!_awayMission->feather.tlaoxacTestPassed && !_awayMission->redshirtDead && !_roomVar.feather.tlaoxacUnconscious) {
		if (!_roomVar.feather.showedSnakeToTlaoxac)
			showText(TX_SPEAKER_TLAOXAC, 38, true);
		else if (_awayMission->feather.numRocksThrownAtTlaoxac != 0)
			showText(TX_SPEAKER_TLAOXAC, 38, true);
		else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				1, 2, 3,
				TX_BLANK
			};
			int choice = showMultipleTexts(choices, true);

			if (choice == 0)
				showText(TX_SPEAKER_TLAOXAC, 37, true);
			else if (choice == 1)
				showText(TX_SPEAKER_TLAOXAC, 42, true);
			else if (choice == 2) { // Accidentally insulted Quetzecoatl
				showText(TX_SPEAKER_TLAOXAC, 40, true);
				_awayMission->disableInput = true;
				loadActorAnimC(OBJECT_TLAOXAC, "s5r3aj", -1, -1, &Room::feather3TlaoxacKilledRedshirt);
				playMidiMusicTracks(2);
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
		mccoyScan(DIR_W, 10, false, true);
	else
		mccoyScan(DIR_W, 14, false, true);
}

void Room::feather3UseMedkitOnTlaoxac() {
	if (_awayMission->feather.knockedOutTlaoxac)
		showText(TX_SPEAKER_MCCOY, 12, true);
}

void Room::feather3UseMedkitOnRedshirt() {
	if (_awayMission->redshirtDead)
		mccoyScan(DIR_N, 30, true);
}

void Room::feather3UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, 6, true);
}

void Room::feather3UseMTricorderOnLight() {
	mccoyScan(DIR_W, 13, false, true);
}

void Room::feather3UseSTricorderOnLight() {
	spockScan(DIR_W, 19, false, true);
}

void Room::feather3UseSTricorderAnywhere() {
	spockScan(DIR_S, 21, false, true);
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
	showDescription(10, true);
}

void Room::feather3LookAtSpock() {
	if (!_awayMission->feather.knockedOutTlaoxac && !_awayMission->feather.tlaoxacTestPassed)
		showDescription(2, true);
	else // ENHANCEMENT: Originally did nothing here. Fall back to default behaviour in the engine.
		_awayMission->rdfStillDoDefaultAction = true;
}

void Room::feather3LookAtRedshirt() {
	if (_awayMission->redshirtDead)
		showDescription(5, true);
	else
		showDescription(12, true);
}

void Room::feather3LookAtTlaoxac() {
	if (_awayMission->feather.knockedOutTlaoxac)
		showDescription(3, true);
	else
		showDescription(7, true);
}

void Room::feather3LookAtLight() {
	showDescription(1, true);
}

void Room::feather3LookAtMccoy() {
	showDescription(0, true);
}

void Room::feather3LookAtKnife() {
	showDescription(11, true);
}

}
