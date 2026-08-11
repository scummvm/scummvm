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

#define OBJECT_CRYSTALS 8
#define OBJECT_STALACTITES 9
#define OBJECT_THROWN_STONE 10

#define HOTSPOT_EAST_EXIT 0x20
#define HOTSPOT_ROCKS 0x21
#define HOTSPOT_STALACTITES 0x22
#define HOTSPOT_STALAGMITES 0x23

namespace StarTrek {

extern const RoomAction feather6ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0},                            &Room::feather6Tick1 },
	{ {ACTION_TOUCHED_HOTSPOT, 0, 0, 0, 0},                 &Room::feather6TouchedHotspot0 },
	{ {ACTION_USE, OBJECT_SPOCK, OBJECT_CRYSTALS, 0, 0},    &Room::feather6UseSpockOnCrystals },
	{ {ACTION_USE, OBJECT_MCCOY, OBJECT_CRYSTALS, 0, 0},    &Room::feather6UseMccoyOnCrystals },
	{ {ACTION_USE, OBJECT_REDSHIRT, OBJECT_CRYSTALS, 0, 0}, &Room::feather6UseRedshirtOnCrystals },

	{ {ACTION_USE, OBJECT_IROCK, OBJECT_STALACTITES, 0, 0},  &Room::feather6UseRockOnStalactites },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_STALACTITES, 0, 0}, &Room::feather6UseRockOnStalactites },
	{ {ACTION_DONE_WALK, 1, 0, 0, 0},                        &Room::feather6ReachedPositionToThrowRock },
	{ {ACTION_DONE_ANIM, 7, 0, 0, 0},                        &Room::feather6DoneThrowingRock },
	{ {ACTION_DONE_ANIM, 2, 0, 0, 0},                        &Room::feather6KirkDiedFromStalactites },

	{ {ACTION_USE, OBJECT_IROCK, OBJECT_CRYSTALS, 0, 0},     &Room::feather6UseRockOnCrystals },
	{ {ACTION_DONE_WALK, 4, 0, 0, 0},                        &Room::feather6ReachedCrystalsWithRock },
	{ {ACTION_TICK, 0xff, 0xff, 0xff, 0},                    &Room::feather6Tick },
	{ {ACTION_DONE_ANIM, 10, 0, 0, 0},                       &Room::feather6HitCrystalsWithRockFirstTime },
	{ {ACTION_DONE_ANIM, 9, 0, 0, 0},                        &Room::feather6HitCrystalsWithRockSecondTime },

	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_STALAGMITES, 0, 0}, &Room::feather6UseRockOnStalagmites },
	{ {ACTION_USE, OBJECT_IROCK, HOTSPOT_EAST_EXIT, 0, 0},   &Room::feather6UseRockOnStalagmites },
	{ {ACTION_USE, OBJECT_IROCK, 0xff, 0, 0},                &Room::feather6UseRockAnywhere },
	{ {ACTION_USE, OBJECT_IKNIFE, HOTSPOT_STALAGMITES, 0, 0}, &Room::feather6UseKnifeOnStalagmites },
	{ {ACTION_USE, OBJECT_IKNIFE, 0xff, 0, 0},               &Room::feather6UseKnifeAnywhere },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_MCCOY, 0, 0},       &Room::feather6UseKnifeOnMccoy },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_SPOCK, 0, 0},       &Room::feather6UseKnifeOnSpock },
	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_REDSHIRT, 0, 0},    &Room::feather6UseKnifeOnRedshirt },
	{ {ACTION_USE, OBJECT_ICRYSTAL, 0xff, 0, 0},             &Room::feather6UseCrystalAnywhere },
	{ {ACTION_USE, OBJECT_ISNAKE, 0xff, 0, 0},               &Room::feather6UseSnakeAnywhere },
	{ {ACTION_USE, OBJECT_IMEDKIT, 0xff, 0, 0},              &Room::feather6UseMedkitAnywhere },
	{ {ACTION_USE, OBJECT_IPHASERS, 0xff, 0, 0},             &Room::feather6UsePhaser },
	{ {ACTION_USE, OBJECT_IPHASERK, 0xff, 0, 0},             &Room::feather6UsePhaser },
	{ {ACTION_LOOK, HOTSPOT_EAST_EXIT, 0, 0, 0},             &Room::feather6LookAtEastExit },
	{ {ACTION_LOOK, 0xff, 0, 0, 0},                          &Room::feather6LookAnywhere },
	{ {ACTION_LOOK, HOTSPOT_ROCKS, 0, 0, 0},                 &Room::feather6LookAtRocks },
	{ {ACTION_LOOK, HOTSPOT_STALAGMITES, 0, 0, 0},           &Room::feather6LookAtStalagmites },
	{ {ACTION_LOOK, OBJECT_CRYSTALS, 0, 0, 0},               &Room::feather6LookAtCrystals },
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0},                   &Room::feather6LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0},                  &Room::feather6LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0},                  &Room::feather6LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0},               &Room::feather6LookAtRedshirt },
	{ {ACTION_LOOK, HOTSPOT_STALACTITES, 0, 0, 0},           &Room::feather6LookAtStalactites },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0},                  &Room::feather6TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0},               &Room::feather6TalkToRedshirt },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0},                  &Room::feather6TalkToSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_EAST_EXIT, 0, 0}, &Room::feather6UseMTricorderOnEastExit },
	{ {ACTION_USE, OBJECT_IMTRICOR, HOTSPOT_ROCKS, 0, 0},    &Room::feather6UseMTricorderOnRocks },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_EAST_EXIT, 0, 0}, &Room::feather6UseSTricorderOnEastExit },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0},             &Room::feather6UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_ROCKS, 0, 0},    &Room::feather6UseSTricorderOnRocks },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_CRYSTALS, 0, 0},  &Room::feather6UseSTricorderOnCrystals },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_STALACTITES, 0, 0}, &Room::feather6UseSTricorderOnStalactites },
	{ {ACTION_USE, OBJECT_ISTRICOR, HOTSPOT_STALAGMITES, 0, 0}, &Room::feather6UseSTricorderOnStalagmites },
	{ {ACTION_GET, OBJECT_CRYSTALS, 0, 0, 0},                &Room::feather6GetCrystals },

	{ {ACTION_USE, OBJECT_IKNIFE, OBJECT_CRYSTALS, 0, 0},    &Room::feather6UseKnifeOnCrystals },
	{ {ACTION_DONE_WALK, 2, 0, 0, 0},                        &Room::feather6ReachedCrystalsWithKnife },
	{ {ACTION_DONE_ANIM, 3, 0, 0, 0},                        &Room::feather6DoneCuttingCrystals },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0, 0},                    &Room::feather6Timer4Expired },

	// ENHANCEMENTs: Also add entries for OBJECT_STALACTITES, not just HOTSPOT_STALACTITES
	{ {ACTION_LOOK, OBJECT_STALACTITES, 0, 0, 0},              &Room::feather6LookAtStalactites },
	{ {ACTION_USE, OBJECT_ISTRICOR, OBJECT_STALACTITES, 0, 0}, &Room::feather6UseSTricorderOnStalactites },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum feather6TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_STRAGEY,
	TX_FEA6_001, TX_FEA6_002, TX_FEA6_003, TX_FEA6_004, TX_FEA6_005,
	TX_FEA6_006, TX_FEA6_007, TX_FEA6_008, TX_FEA6_009, TX_FEA6_011,
	TX_FEA6_012, TX_FEA6_013, TX_FEA6_014, TX_FEA6_015, TX_FEA6_016,
	TX_FEA6_017, TX_FEA6_018, TX_FEA6_019, TX_FEA6_020, TX_FEA6_021,
	TX_FEA6_022, TX_FEA6_023, TX_FEA6_024, TX_FEA6_025, TX_FEA6_026,
	TX_FEA6_027, TX_FEA6_028, TX_FEA6_029, TX_FEA6_030, TX_FEA6_031,
	TX_FEA6_032, TX_FEA6_033, TX_FEA6_034, TX_FEA6_035, TX_FEA6_036,
	TX_FEA6_037, TX_FEA6_038, TX_FEA6_039, TX_FEA6_040, TX_FEA6_041,
	TX_FEA6_042, TX_FEA6_043, TX_FEA6N000, TX_FEA6N001, TX_FEA6N002,
	TX_FEA6N003, TX_FEA6N004, TX_FEA6N005, TX_FEA6N006, TX_FEA6N007,
	TX_FEA6N008, TX_FEA6N009, TX_FEA6N010, TX_FEA6N011, TX_FEA6N012,
	TX_FEA6N013, TX_FEA6N014, TX_FEA6N015, TX_FEA6N016, TX_FEA6N017,
	TX_FEA6N020, TX_FEA6N028, TX_FEA6N029
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets feather6TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 2326, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 2337, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 2347, 0, 0, 0 },
	{ TX_SPEAKER_STRAGEY, 2357, 0, 0, 0 },
	{ TX_FEA6_001, 2687, 0, 0, 0 },
	{ TX_FEA6_002, 5067, 0, 0, 0 },
	{ TX_FEA6_003, 5632, 0, 0, 0 },
	{ TX_FEA6_004, 4306, 0, 0, 0 },
	{ TX_FEA6_005, 5755, 0, 0, 0 },
	{ TX_FEA6_006, 4238, 0, 0, 0 },
	{ TX_FEA6_007, 5863, 0, 0, 0 },
	{ TX_FEA6_008, 4179, 0, 0, 0 },
	{ TX_FEA6_009, 2445, 0, 0, 0 },
	{ TX_FEA6_011, 1104, 0, 0, 0 },
	{ TX_FEA6_012, 6315, 0, 0, 0 },
	{ TX_FEA6_013, 6247, 0, 0, 0 },
	{ TX_FEA6_014, 7241, 0, 0, 0 },
	{ TX_FEA6_015, 6445, 0, 0, 0 },
	{ TX_FEA6_016, 5227, 0, 0, 0 },
	{ TX_FEA6_017, 4043, 0, 0, 0 },
	{ TX_FEA6_018, 6874, 0, 0, 0 },
	{ TX_FEA6_019, 3201, 0, 0, 0 },
	{ TX_FEA6_020, 2370, 0, 0, 0 },
	{ TX_FEA6_021, 2778, 0, 0, 0 },
	{ TX_FEA6_022, 6590, 0, 0, 0 },
	{ TX_FEA6_023, 7370, 0, 0, 0 },
	{ TX_FEA6_024, 6537, 0, 0, 0 },
	{ TX_FEA6_025, 6680, 0, 0, 0 },
	{ TX_FEA6_026, 6777, 0, 0, 0 },
	{ TX_FEA6_027, 7043, 0, 0, 0 },
	{ TX_FEA6_028, 3303, 0, 0, 0 },
	{ TX_FEA6_029, 6014, 0, 0, 0 },
	{ TX_FEA6_030, 6046, 0, 0, 0 },
	{ TX_FEA6_031, 6172, 0, 0, 0 },
	{ TX_FEA6_032, 4380, 0, 0, 0 },
	{ TX_FEA6_033, 4086, 0, 0, 0 },
	{ TX_FEA6_034, 3883, 0, 0, 0 },
	{ TX_FEA6_035, 5377, 0, 0, 0 },
	{ TX_FEA6_036, 4492, 0, 0, 0 },
	{ TX_FEA6_037, 4592, 0, 0, 0 },
	{ TX_FEA6_038, 5498, 0, 0, 0 },
	{ TX_FEA6_039, 2867, 0, 0, 0 },
	{ TX_FEA6_040, 2504, 0, 0, 0 },
	{ TX_FEA6_041, 5142, 0, 0, 0 },
	{ TX_FEA6_042, 4972, 0, 0, 0 },
	{ TX_FEA6_043, 5328, 0, 0, 0 },
	{ TX_FEA6N000, 2601, 0, 0, 0 },
	{ TX_FEA6N001, 3147, 0, 0, 0 },
	{ TX_FEA6N002, 4804, 0, 0, 0 },
	{ TX_FEA6N003, 3631, 0, 0, 0 },
	{ TX_FEA6N004, 7307, 0, 0, 0 },
	{ TX_FEA6N005, 3404, 0, 0, 0 },
	{ TX_FEA6N006, 3367, 0, 0, 0 },
	{ TX_FEA6N007, 4643, 0, 0, 0 },
	{ TX_FEA6N008, 4870, 0, 0, 0 },
	{ TX_FEA6N009, 2746, 0, 0, 0 },
	{ TX_FEA6N010, 2835, 0, 0, 0 },
	{ TX_FEA6N011, 4758, 0, 0, 0 },
	{ TX_FEA6N012, 3770, 0, 0, 0 },
	{ TX_FEA6N013, 3823, 0, 0, 0 },
	{ TX_FEA6N014, 3682, 0, 0, 0 },
	{ TX_FEA6N015, 3059, 0, 0, 0 },
	{ TX_FEA6N016, 3454, 0, 0, 0 },
	{ TX_FEA6N017, 7125, 0, 0, 0 },
	{ TX_FEA6N020, 7185, 0, 0, 0 },
	{ TX_FEA6N028, 2997, 0, 0, 0 },
	{ TX_FEA6N029, 2921, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText feather6Texts[] = {
	{ -1, Common::UNK_LANG, "" }
};

void Room::feather6Tick1() {
	playVoc("FEA6LOOP");
	playMidiMusicTracks(MIDITRACK_27);
	loadActorAnim(OBJECT_CRYSTALS, "s5r6cx", 0x8a, 0x91);
	loadActorAnim(OBJECT_STALACTITES, "s5r6ts", 0x90, 0x25);
}

void Room::feather6TouchedHotspot0() { // McCoy warns you not to go back to the river
	showText(TX_SPEAKER_MCCOY, TX_FEA6_014);
}

void Room::feather6UseSpockOnCrystals() {
	showText(TX_SPEAKER_SPOCK, TX_FEA6_020);
}

void Room::feather6UseMccoyOnCrystals() {
	showText(TX_SPEAKER_MCCOY, TX_FEA6_009);
}

void Room::feather6UseRedshirtOnCrystals() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_040);
}

void Room::feather6UseRockOnStalactites() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x94, 0x97, &Room::feather6ReachedPositionToThrowRock);
}

void Room::feather6ReachedPositionToThrowRock() {
	loadActorAnim2(OBJECT_KIRK, "s5r1kt");
	loadActorAnimC(OBJECT_THROWN_STONE, "s5r1ru", 0x96, 0x4e, &Room::feather6DoneThrowingRock);
}

void Room::feather6DoneThrowingRock() {
	loadActorAnim2(OBJECT_STALACTITES, "s5r6tf");
	loadActorAnimC(OBJECT_KIRK, "s5r6kd", -1, -1, &Room::feather6KirkDiedFromStalactites);
	playMidiMusicTracks(MIDITRACK_1);
}

void Room::feather6KirkDiedFromStalactites() {
	_awayMission->feather.missionScore -= 3;
	_awayMission->feather.diedFromStalactites = true;
	_awayMission->disableInput = false;
	showDescription(TX_FEA6N000);
	showDescription(TX_FEA6N004);
	loadRoomIndex(7, 5);
}

void Room::feather6UseRockOnCrystals() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x9a, 0x97, &Room::feather6ReachedCrystalsWithRock);
}

void Room::feather6ReachedCrystalsWithRock() {
	_awayMission->timers[2] = 174;
	loadActorAnim2(OBJECT_STALACTITES, "s5r6tm");
	if (!_roomVar.feather.usedRockOnCrystalsOnce) {
		_roomVar.feather.usedRockOnCrystalsOnce = true;
		loadActorAnimC(OBJECT_KIRK, "s5r6kr", -1, -1, &Room::feather6HitCrystalsWithRockFirstTime);
	} else
		loadActorAnimC(OBJECT_KIRK, "s5r6kr", -1, -1, &Room::feather6HitCrystalsWithRockSecondTime);
}

void Room::feather6Tick() {
}

void Room::feather6HitCrystalsWithRockFirstTime() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	showText(TX_SPEAKER_KIRK, TX_FEA6_001);
}

void Room::feather6HitCrystalsWithRockSecondTime() {
	loadActorAnimC(OBJECT_KIRK, "s5r6kd", -1, -1, &Room::feather6KirkDiedFromStalactites);
	loadActorAnim2(OBJECT_STALACTITES, "s5r6tf");
	playMidiMusicTracks(MIDITRACK_0);
}

void Room::feather6UseRockOnStalagmites() {
	showText(TX_SPEAKER_SPOCK, TX_FEA6_019);
}

void Room::feather6UseRockAnywhere() {
	// ENHANCEMENT: There were two implementations of this function. The first (which took
	// precedence) showed TX_FEA6N009, a generic "nothing happens" text. The second is
	// Spock advising you not to use rocks, which is more interesting.
	showText(TX_SPEAKER_SPOCK, TX_FEA6_021);
}

void Room::feather6UseKnifeOnStalagmites() {
	showText(TX_SPEAKER_SPOCK, TX_FEA6_028);
}

void Room::feather6UseKnifeAnywhere() {
	showDescription(TX_FEA6N010);
}

void Room::feather6UseKnifeOnMccoy() {
	// BUG: Stragey could be dead...
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_039); // BUGFIX: Speaker is Stragey (not Spock)
}

void Room::feather6UseKnifeOnSpock() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_039); // BUGFIX: Speaker is Stragey (not Spock)
}

void Room::feather6UseKnifeOnRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_039); // BUGFIX: Speaker is Stragey (not Spock)
}

void Room::feather6UseCrystalAnywhere() {
	showDescription(TX_FEA6N015);
}

void Room::feather6UseSnakeAnywhere() {
	showDescription(TX_FEA6N001);
}

void Room::feather6UseMedkitAnywhere() {
	showText(TX_SPEAKER_MCCOY, TX_FEA6_011);
}

void Room::feather6UsePhaser() {
	showText(TX_SPEAKER_SPOCK, TX_FEA6_023);
}

void Room::feather6LookAtEastExit() {
	showDescription(TX_FEA6N016);
}

void Room::feather6LookAnywhere() {
	showDescription(TX_FEA6N003);
}

void Room::feather6LookAtRocks() {
	showDescription(TX_FEA6N014);
}

void Room::feather6LookAtStalagmites() {
	showDescription(TX_FEA6N013);
}

void Room::feather6LookAtCrystals() {
	showDescription(TX_FEA6N005);
}

void Room::feather6LookAtKirk() {
	showDescription(TX_FEA6N007);
}

void Room::feather6LookAtSpock() {
	showDescription(TX_FEA6N011);
}

void Room::feather6LookAtMccoy() {
	showDescription(TX_FEA6N002);
}

void Room::feather6LookAtRedshirt() {
	showDescription(TX_FEA6N008);
}

void Room::feather6LookAtStalactites() {
	showDescription(TX_FEA6N012);
	showText(TX_SPEAKER_SPOCK, TX_FEA6_034);
	showText(TX_SPEAKER_MCCOY, TX_FEA6_017);
	showText(TX_SPEAKER_SPOCK, TX_FEA6_033);

	// BUG: Choice 2 (TX_FEA6_004) doesn't match with audio at all.
	const TextRef choices[] = {
		TX_SPEAKER_KIRK,
		TX_FEA6_008, TX_FEA6_006, TX_FEA6_004,
		TX_END
	};
	int choice = showMultipleTexts(choices);

	if (choice == 0)
		showText(TX_SPEAKER_SPOCK, TX_FEA6_032);
	else if (choice == 1)
		showText(TX_SPEAKER_SPOCK, TX_FEA6_036);
	else
		showText(TX_SPEAKER_SPOCK, TX_FEA6_037);
}

void Room::feather6TalkToMccoy() {
	showText(TX_SPEAKER_MCCOY, TX_FEA6_013);
}

void Room::feather6TalkToRedshirt() {
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_042);
	showText(TX_SPEAKER_KIRK,    TX_FEA6_002);
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_041);
	showText(TX_SPEAKER_MCCOY,   TX_FEA6_016);
	showText(TX_SPEAKER_STRAGEY, TX_FEA6_043);
}

void Room::feather6TalkToSpock() {
	if (!_awayMission->redshirtDead)
		showText(TX_SPEAKER_SPOCK, TX_FEA6_035);
	else {
		showText(TX_SPEAKER_SPOCK, TX_FEA6_038);

		const TextRef choices[] = {
			TX_SPEAKER_KIRK,
			TX_FEA6_003, TX_FEA6_005, TX_FEA6_007,
			TX_END
		};
		int choice = showMultipleTexts(choices);

		if (choice == 0)
			showText(TX_SPEAKER_SPOCK, TX_FEA6_029);
		else if (choice == 1)
			showText(TX_SPEAKER_SPOCK, TX_FEA6_030);
		else
			showText(TX_SPEAKER_SPOCK, TX_FEA6_031);
	}
}

void Room::feather6UseMTricorderOnEastExit() {
	mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA6_012);
}

void Room::feather6UseMTricorderOnRocks() {
	// ENHANCEMENT: Original didn't play tricorder noise, etc
	mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_FEA6_015);
}

void Room::feather6UseSTricorderOnEastExit() {
	spockScan(DIR_E, TX_SPEAKER_SPOCK, TX_FEA6_024);
}

void Room::feather6UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_FEA6_022);
}

void Room::feather6UseSTricorderOnRocks() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_FEA6_018);
}

void Room::feather6UseSTricorderOnCrystals() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_FEA6_027);
}

void Room::feather6UseSTricorderOnStalactites() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_FEA6_025);
}

void Room::feather6UseSTricorderOnStalagmites() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_FEA6_026);
}

void Room::feather6GetCrystals() {
	showDescription(TX_FEA6N020);
}

void Room::feather6UseKnifeOnCrystals() {
	_awayMission->disableInput = true;
	walkCrewmanC(OBJECT_KIRK, 0x9a, 0x97, &Room::feather6ReachedCrystalsWithKnife);
}

void Room::feather6ReachedCrystalsWithKnife() {
	loadActorAnimC(OBJECT_KIRK, "s5r6kp", -1, -1, &Room::feather6DoneCuttingCrystals);
	_awayMission->timers[4] = 122;
}

void Room::feather6DoneCuttingCrystals() {
	_awayMission->disableInput = false;
	loadActorStandAnim(OBJECT_KIRK);
	_awayMission->feather.missionScore += 1;
	giveItem(OBJECT_ICRYSTAL);
	showDescription(TX_FEA6N017);
}

void Room::feather6Timer4Expired() { // Crystal disappears when timer 4 expires
	loadActorStandAnim(OBJECT_CRYSTALS);
}

}
