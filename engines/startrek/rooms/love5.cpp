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

#define OBJECT_8 8
#define OBJECT_PREAX 9
#define OBJECT_MARCUS 10
#define OBJECT_CHEEVER 11

#define HOTSPOT_DEVICE 0x20
#define HOTSPOT_CONSOLE 0x21

namespace StarTrek {

extern const RoomAction love5ActionList[] = {
	{ {ACTION_TICK, 1, 0, 0, 0}, &Room::love5Tick1 },
	{ {ACTION_TOUCHED_WARP, 1, 0, 0, 0}, &Room::love5TouchedWarp1 },
	{ {ACTION_TIMER_EXPIRED, 4, 0, 0, 0}, &Room::love5Timer4Expired },
	{ {ACTION_USE, OBJECT_IPHASERS, 9, 0, 0}, &Room::love5UseStunPhaserOnPreax },
	// NOTE: nothing for OBJECT_IPHASERK. There's an unused audio file which might fit.
	{ {ACTION_LOOK, OBJECT_KIRK, 0, 0, 0}, &Room::love5LookAtKirk },
	{ {ACTION_LOOK, OBJECT_SPOCK, 0, 0, 0}, &Room::love5LookAtSpock },
	{ {ACTION_LOOK, OBJECT_MCCOY, 0, 0, 0}, &Room::love5LookAtMccoy },
	{ {ACTION_LOOK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love5LookAtRedshirt },
	{ {ACTION_LOOK, 0xff, 0, 0, 0}, &Room::love5LookAnywhere },
	{ {ACTION_LOOK, 0x20, 0, 0, 0}, &Room::love5LookAtDevice },
	{ {ACTION_LOOK, 0x21, 0, 0, 0}, &Room::love5LookAtConsole },
	{ {ACTION_LOOK, 10, 0, 0, 0}, &Room::love5LookAtDrMarcus },
	{ {ACTION_LOOK, 11, 0, 0, 0}, &Room::love5LookAtDrCheever },
	{ {ACTION_LOOK, 9, 0, 0, 0}, &Room::love5LookAtPreax },
	{ {ACTION_TALK, 9, 0, 0, 0}, &Room::love5TalkToPreax },
	{ {ACTION_FINISHED_WALKING, 5, 0, 0, 0}, &Room::love5MccoyReachedSpockToCure },
	{ {ACTION_FINISHED_ANIMATION, 6, 0, 0, 0}, &Room::love5MccoyCuredSpock },
	{ {ACTION_FINISHED_WALKING, 6, 0, 0, 0}, &Room::love5CrewmanReachedBeamoutPosition },
	{ {ACTION_TALK, 10, 0, 0, 0}, &Room::love5TalkToDrMarcus },
	{ {ACTION_TALK, 11, 0, 0, 0}, &Room::love5TalkToDrCheever },
	{ {ACTION_TALK, OBJECT_KIRK, 0, 0, 0}, &Room::love5TalkToKirk },
	{ {ACTION_TALK, OBJECT_SPOCK, 0, 0, 0}, &Room::love5TalkToSpock },
	{ {ACTION_TALK, OBJECT_MCCOY, 0, 0, 0}, &Room::love5TalkToMccoy },
	{ {ACTION_TALK, OBJECT_REDSHIRT, 0, 0, 0}, &Room::love5TalkToRedshirt },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0xff, 0, 0}, &Room::love5UseSTricorderAnywhere },
	{ {ACTION_USE, OBJECT_ISTRICOR, 0x20, 0, 0}, &Room::love5UseSTricorderOnDevice },
	{ {ACTION_USE, OBJECT_IMTRICOR, 10, 0, 0}, &Room::love5UseMTricorderOnMarcusOrCheever },
	{ {ACTION_USE, OBJECT_IMTRICOR, 11, 0, 0}, &Room::love5UseMTricorderOnMarcusOrCheever },
	{ {ACTION_USE, OBJECT_IMTRICOR, 9, 0, 0}, &Room::love5UseMTricorderOnPreax },
	{ {ACTION_USE, OBJECT_IMEDKIT, 9, 0, 0}, &Room::love5UseMedkitOnPreax },
	{ {ACTION_USE, OBJECT_IH2O, 9, 0, 0}, &Room::love5UseWaterOnPreax },
	{ {ACTION_USE, OBJECT_ISAMPLE, 9, 0, 0}, &Room::love5UseCureSampleOnPreax },
	{ {ACTION_USE, OBJECT_ICURE, 9, 0, 0}, &Room::love5UseCureOnPreax },
	{ {ACTION_FINISHED_WALKING, 4, 0, 0, 0}, &Room::love5ReachedPreaxUsingCure },
	{ {ACTION_FINISHED_ANIMATION, 5, 0, 0, 0}, &Room::love5CuredPreax },
	{ {ACTION_USE, 0xff, 9, 0, 0}, &Room::love5UseAnythingOnPreax },
	{ {ACTION_USE, OBJECT_KIRK, 10, 0, 0}, &Room::love5UseKirkOnMarcusOrCheever },
	{ {ACTION_USE, OBJECT_KIRK, 11, 0, 0}, &Room::love5UseKirkOnMarcusOrCheever },
	{ {ACTION_FINISHED_WALKING,   1, 0, 0, 0}, &Room::love5KirkReachedCheever },
	{ {ACTION_FINISHED_ANIMATION, 2, 0, 0, 0}, &Room::love5KirkUntiedCheever },
	{ {ACTION_FINISHED_WALKING,   2, 0, 0, 0}, &Room::love5KirkReachedMarcus },
	{ {ACTION_FINISHED_ANIMATION, 3, 0, 0, 0}, &Room::love5KirkUntiedMarcus },
	{ {ACTION_FINISHED_ANIMATION, 4, 0, 0, 0}, &Room::love5MarcusStoodUp },

	// Common code
	{ {ACTION_TIMER_EXPIRED, 0, 0, 0, 0}, &Room::loveaTimer0Expired },
	{ {ACTION_TIMER_EXPIRED, 1, 0, 0, 0}, &Room::loveaTimer1Expired },
	{ {ACTION_USE, OBJECT_IMEDKIT, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMedkitOnSpock },
	{ {ACTION_USE, OBJECT_ISAMPLE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureSampleOnSpock },
	{ {ACTION_USE, OBJECT_ICURE, OBJECT_SPOCK, 0, 0}, &Room::loveaUseCureOnSpock },
	{ {ACTION_FINISHED_WALKING,   99, 0, 0, 0}, &Room::loveaSpockOrMccoyInPositionToUseCure },
	{ {ACTION_FINISHED_ANIMATION, 99, 0, 0, 0}, &Room::loveaFinishedCuringSpock },
	{ {ACTION_TIMER_EXPIRED, 2, 0, 0, 0}, &Room::loveaTimer2Expired },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_SPOCK, 0, 0}, &Room::loveaUseMTricorderOnSpock },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_KIRK, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_MCCOY, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IMTRICOR, OBJECT_REDSHIRT, 0, 0}, &Room::loveaUseMTricorderOnHuman },
	{ {ACTION_USE, OBJECT_IRLG, 0xff, 0, 0}, &Room::loveaUseRomulanLaughingGas },
	{ {ACTION_USE, OBJECT_IN2O, 0xff, 0, 0}, &Room::loveaUseHumanLaughingGas },
	{ {ACTION_USE, OBJECT_INH3, 0xff, 0, 0}, &Room::loveaUseAmmonia },
	{ {ACTION_USE, OBJECT_ICOMM, 0xff, 0, 0}, &Room::loveaUseCommunicator },
	{ {ACTION_LIST_END, 0, 0, 0, 0}, nullptr }
};

enum love5TextIds {
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_FERRIS, TX_SPEAKER_MARCUS,
	TX_SPEAKER_CHEEVER, TX_SPEAKER_PREAX,
	TX_LOV0_011, TX_LOV0_012, TX_LOV0_013, TX_LOV0_014, TX_LOV0_015,
	TX_LOV0_016, TX_LOV0_017, TX_LOV0_019, TX_LOV0_020, TX_LOV0_021,
	TX_LOV0_022, TX_LOV0_025, TX_LOV0_026, TX_LOV0_029, TX_LOV0_033,
	TX_LOV0_039, TX_LOV0_040, TX_LOV0_041, TX_LOV0_042, TX_LOV0_043,
	TX_LOV0_045, TX_LOV0_046, TX_LOV0_047, TX_LOV0_048, TX_LOV0_049,
	TX_LOV0_050, TX_LOV0_101, TX_LOV0_102, TX_LOV0_103, TX_LOV0_104,
	TX_LOV0_105, TX_LOV0_106, TX_LOV0_107, TX_LOV0_124, TX_LOV5_001,
	TX_LOV5_002, TX_LOV5_003, TX_LOV5_004, TX_LOV5_005, TX_LOV5_006,
	TX_LOV5_007, TX_LOV5_008, TX_LOV5_009, TX_LOV5_010, TX_LOV5_011,
	TX_LOV5_013, TX_LOV5_014, TX_LOV5_015, TX_LOV5_016, TX_LOV5_017,
	TX_LOV5_018, TX_LOV5_019, TX_LOV5_020, TX_LOV5_021, TX_LOV5_023,
	TX_LOV5_024, TX_LOV5_025, TX_LOV5_026, TX_LOV5_027, TX_LOV5_028,
	TX_LOV5_029, TX_LOV5_030, TX_LOV5_031, TX_LOV5_032, TX_LOV5_033,
	TX_LOV5_034, TX_LOV5_036, TX_LOV5_038, TX_LOV5_039, TX_LOV5_040,
	TX_LOV5_041, TX_LOV5_042, TX_LOV5_043, TX_LOV5_044, TX_LOV5_045,
	TX_LOV5_046, TX_LOV5_047, TX_LOV5_048, TX_LOV5_049, TX_LOV5_050,
	TX_LOV5_051, TX_LOV5_052, TX_LOV5_053, TX_LOV5_054, TX_LOV5_055,
	TX_LOV5_056, TX_LOV5_057, TX_LOV5_058, TX_LOV5_059, TX_LOV5_060,
	TX_LOV5_061, TX_FEA5_008, TX_LOV5N000, TX_LOV5N001, TX_LOV5N002,
	TX_LOV5N003, TX_LOV5N004, TX_LOV5N005, TX_LOV5N006, TX_LOV5N007,
	TX_LOV5N008, TX_LOV5N009, TX_LOV5N010, TX_LOV5N011, TX_LOV5N012,
	TX_LOV5N013, TX_LOV5N014, TX_LOV5N016, TX_LOV5N017, TX_LOV5N018,
	/*TX_LOV5C001,*/ TX_DEM0N009, TX_G_024, TX_DIALOG_ERROR
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets love5TextOffsets[] = {
	{ TX_SPEAKER_KIRK, 8968, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 8979, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 8989, 0, 0, 0 },
	{ TX_SPEAKER_FERRIS, 8999, 0, 0, 0 },
	{ TX_SPEAKER_CHEEVER, 9027, 0, 0, 0 },
	{ TX_SPEAKER_MARCUS, 9016, 0, 0, 0 },
	{ TX_SPEAKER_PREAX, 9010, 0, 0, 0 },
	{ TX_LOV0_011, 13010, 0, 0, 0 },
	{ TX_LOV0_012, 13137, 0, 0, 0 },
	{ TX_LOV0_013, 13433, 0, 0, 0 },
	{ TX_LOV0_014, 13197, 0, 0, 0 },
	{ TX_LOV0_015, 13301, 0, 0, 0 },
	{ TX_LOV0_016, 13104, 0, 0, 0 },
	{ TX_LOV0_017, 13567, 0, 0, 0 },
	{ TX_LOV0_019, 13487, 0, 0, 0 },
	{ TX_LOV0_020, 13044, 0, 0, 0 },
	{ TX_LOV0_021, 13355, 0, 0, 0 },
	{ TX_LOV0_022, 13245, 0, 0, 0 },
	{ TX_LOV0_025, 14493, 0, 0, 0 },
	{ TX_LOV0_026, 14366, 0, 0, 0 },
	{ TX_LOV0_029, 14246, 0, 0, 0 },
	{ TX_LOV0_033, 14306, 0, 0, 0 },
	{ TX_LOV0_039, 13628, 0, 0, 0 },
	{ TX_LOV0_040, 13755, 0, 0, 0 },
	{ TX_LOV0_041, 14051, 0, 0, 0 },
	{ TX_LOV0_042, 13815, 0, 0, 0 },
	{ TX_LOV0_043, 13919, 0, 0, 0 },
	{ TX_LOV0_045, 13722, 0, 0, 0 },
	{ TX_LOV0_046, 14185, 0, 0, 0 },
	{ TX_LOV0_047, 14105, 0, 0, 0 },
	{ TX_LOV0_048, 13662, 0, 0, 0 },
	{ TX_LOV0_049, 13973, 0, 0, 0 },
	{ TX_LOV0_050, 13863, 0, 0, 0 },
	{ TX_LOV0_101, 14574, 0, 0, 0 },
	{ TX_LOV0_102, 14638, 0, 0, 0 },
	{ TX_LOV0_103, 14708, 0, 0, 0 },
	{ TX_LOV0_104, 14758, 0, 0, 0 },
	{ TX_LOV0_105, 14815, 0, 0, 0 },
	{ TX_LOV0_106, 14927, 0, 0, 0 },
	{ TX_LOV0_107, 14995, 0, 0, 0 },
	{ TX_LOV0_124, 14443, 0, 0, 0 },
	{ TX_LOV5_001, 2119, 0, 0, 0 },
	{ TX_LOV5_002, 1715, 0, 0, 0 },
	{ TX_LOV5_003, 10797, 0, 0, 0 },
	{ TX_LOV5_004, 12334, 0, 0, 0 },
	{ TX_LOV5_005, 4485, 0, 0, 0 },
	{ TX_LOV5_006, 12464, 0, 0, 0 },
	{ TX_LOV5_007, 4020, 0, 0, 0 },
	{ TX_LOV5_008, 12178, 0, 0, 0 },
	{ TX_LOV5_009, 10983, 0, 0, 0 },
	{ TX_LOV5_010, 1502, 0, 0, 0 },
	{ TX_LOV5_011,  614, 0, 0, 0 },
	{ TX_LOV5_013, 4130, 0, 0, 0 },
	{ TX_LOV5_014, 2897, 0, 0, 0 },
	{ TX_LOV5_015, 3732, 0, 0, 0 },
	{ TX_LOV5_016, 3576, 0, 0, 0 },
	{ TX_LOV5_017, 4915, 0, 0, 0 },
	{ TX_LOV5_018, 1263, 0, 0, 0 },
	{ TX_LOV5_019, 3654, 0, 0, 0 },
	{ TX_LOV5_020, 4640, 0, 0, 0 },
	{ TX_LOV5_021, 1821, 0, 0, 0 },
	{ TX_LOV5_024, 3407, 0, 0, 0 },
	{ TX_LOV5_025, 11753, 0, 0, 0 },
	{ TX_LOV5_026, 4543, 0, 0, 0 },
	{ TX_LOV5_027, 2035, 0, 0, 0 },
	{ TX_LOV5_028, 9148, 0, 0, 0 },
	{ TX_LOV5_029, 3893, 0, 0, 0 },
	{ TX_LOV5_030, 1939, 0, 0, 0 },
	{ TX_LOV5_031, 9380, 0, 0, 0 },
	{ TX_LOV5_032, 1629, 0, 0, 0 },
	{ TX_LOV5_033, 3159, 0, 0, 0 },
	{ TX_LOV5_034, 11526, 0, 0, 0 },
	{ TX_LOV5_036, 11189, 0, 0, 0 },
	{ TX_LOV5_038, 1990, 0, 0, 0 },
	{ TX_LOV5_039, 2414, 0, 0, 0 },
	{ TX_LOV5_040, 2334, 0, 0, 0 },
	{ TX_LOV5_042, 2607, 0, 0, 0 },
	{ TX_LOV5_043, 12668, 0, 0, 0 },
	{ TX_LOV5_044, 9199, 0, 0, 0 },
	{ TX_LOV5_045, 1756, 0, 0, 0 },
	{ TX_LOV5_046, 9839, 0, 0, 0 },
	{ TX_LOV5_047, 11049, 0, 0, 0 },
	{ TX_LOV5_048, 10854, 0, 0, 0 },
	{ TX_LOV5_049, 11642, 0, 0, 0 },
	{ TX_LOV5_050, 9442, 0, 0, 0 },
	{ TX_LOV5_051, 11290, 0, 0, 0 },
	{ TX_LOV5_052,  3039, 0, 0, 0 },
	{ TX_LOV5_053,  4426, 0, 0, 0 },
	{ TX_LOV5_054, 10123, 0, 0, 0 },
	{ TX_LOV5_055, 2685, 0, 0, 0 },
	{ TX_LOV5_056, 2744, 0, 0, 0 },
	{ TX_LOV5_057, 10052, 0, 0, 0 },
	{ TX_LOV5_058, 10193, 0, 0, 0 },
	{ TX_LOV5_059, 11980, 0, 0, 0 },
	{ TX_LOV5_060, 12789, 0, 0, 0 },
	{ TX_LOV5_061, 12604, 0, 0, 0 },
	{ TX_LOV5_023,  5017, 0, 0, 0 },
	{ TX_LOV5_041,  5296, 0, 0, 0 },
	{ TX_FEA5_008,   706, 0, 0, 0 },
	{ TX_G_024,    11501, 0, 0, 0 },
	{ TX_LOV5N000, 10294, 0, 0, 0 },
	{ TX_LOV5N001, 9732, 0, 0, 0 },
	{ TX_LOV5N001, 9890, 0, 0, 0 },
	{ TX_LOV5N002, 10571, 0, 0, 0 },
	{ TX_LOV5N003, 10691, 0, 0, 0 },
	{ TX_LOV5N004, 10481, 0, 0, 0 },
	{ TX_LOV5N005, 10633, 0, 0, 0 },
	{ TX_LOV5N006, 1352, 0, 0, 0 },
	{ TX_LOV5N007, 4813, 0, 0, 0 },
	{ TX_LOV5N008,  863, 0, 0, 0 },
	{ TX_LOV5N009, 1115, 0, 0, 0 },
	{ TX_LOV5N010, 9660, 0, 0, 0 },
	{ TX_LOV5N011, 10413, 0, 0, 0 },
	{ TX_LOV5N012, 10359, 0, 0, 0 },
	{ TX_LOV5N013, 9040, 0, 0, 0 },
	{ TX_LOV5N014, 9961, 0, 0, 0 },
	{ TX_LOV5N016, 9311, 0, 0, 0 },
	{ TX_LOV5N017, 4320, 0, 0, 0 },
	{ TX_LOV5N018, 3805, 0, 0, 0 },
	//{ TX_LOV5C001, 1598, 0, 0, 0 },
	{ TX_DIALOG_ERROR, 11927, 0, 0, 0 },
	{          -1, 0,    0, 0, 0 }
};

extern const RoomText love5Texts[] = {
	{ TX_DEM0N009, Common::EN_ANY, "#DEM0\\DEM0N009#Spock raises an eyebrow" },
	{ -1, Common::UNK_LANG, "" }
};

void Room::love5Tick1() {
	playVoc("LOV5LOOP");

	loadActorAnim(OBJECT_8, "s3r6c1", 0x27, 0xa9);

	if (_awayMission->love.freedMarcusAndCheever) {
		loadActorAnim(OBJECT_MARCUS, "marcusst", 0x28, 0xa6);
		loadActorAnim(OBJECT_CHEEVER, "s3gtupst", 0x49, 0xac);
	} else {
		loadActorAnim(OBJECT_MARCUS, "marcus2", 0x28, 0xa6);
		loadActorAnim(OBJECT_CHEEVER, "s3gtup2", 0x49, 0xac);
	}

	if (_awayMission->love.preaxCured)
		loadActorAnim(OBJECT_PREAX, "s3r6r3", 0x116, 0xba);
	else
		loadActorAnim(OBJECT_PREAX, "s3r6r2", 0x116, 0xba);

	if (_awayMission->love.releasedHumanLaughingGas)
		_awayMission->timers[0] = getRandomWordInRange(200, 400);
	if (_awayMission->love.releasedRomulanLaughingGas)
		_awayMission->timers[1] = getRandomWordInRange(200, 400);
	_awayMission->timers[2] = 200;

	_roomVar.love.cmnXPosToCureSpock = 0x89;
	_roomVar.love.cmnXPosToCureSpock = 0xb9;
}

void Room::love5TouchedWarp1() {
	_awayMission->disableInput = true;
	playSoundEffectIndex(kSfxDoor);
	_awayMission->timers[4] = 10;
}

void Room::love5Timer4Expired() {
	loadRoomIndex(4, 0);
}

void Room::love5UseStunPhaserOnPreax() {
	showText(TX_SPEAKER_MCCOY, TX_LOV5_011);

	// BUG: this function has two implementations. The second one (which is shadowed) is
	// below.
	// Perhaps one was supposed to be for IPHASERK, or for Dr. Marcus?
	//showText(TX_SPEAKER_MCCOY, TX_FEA5_008);
}

void Room::love5LookAtKirk() {
	showDescription(TX_LOV5N004);
}

void Room::love5LookAtSpock() {
	showDescription(TX_LOV5N005);
}

void Room::love5LookAtMccoy() {
	showDescription(TX_LOV5N002);
}

void Room::love5LookAtRedshirt() {
	showDescription(TX_LOV5N003);
}

void Room::love5LookAnywhere() {
	showDescription(TX_LOV5N008);

	// There is an unused version of this function, which states "This is Dr. Marcus's
	// quarters, and there are hostages and 4 romulans here". That description doesn't
	// match any actual room.
	// There are LOVE6.MAP and LOVE6.IW files, but none of the other necessary files to
	// load room LOVE6. This may be a leftover from a removed room that somehow ended up
	// here.

	//showText(TX_LOV5N009);
}

void Room::love5LookAtDevice() {
	showDescription(TX_LOV5N013);
	showText(TX_SPEAKER_MCCOY, TX_LOV5_028);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_044);
}

void Room::love5LookAtConsole() {
	showDescription(TX_LOV5N016);
	showText(TX_SPEAKER_MCCOY, TX_LOV5_031);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_050);
}

void Room::love5LookAtDrMarcus() {
	if (_awayMission->love.freedMarcusAndCheever)
		showDescription(TX_LOV5N001);
	else {
		// BUGFIX: originally played audio "LOV5N001", which is only the first sentence of
		// what should be spoken.
		//showDescription(TX_LOV5N015);	// TODO

		showText(TX_SPEAKER_MARCUS, TX_LOV5_046);
	}
}

void Room::love5LookAtDrCheever() {
	if (_awayMission->love.freedMarcusAndCheever)
		showDescription(TX_LOV5N000);
	else {
		showDescription(TX_LOV5N014);
		showText(TX_SPEAKER_CHEEVER, TX_LOV5_057);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_FERRIS, TX_LOV5_054);
			showText(TX_SPEAKER_CHEEVER, TX_LOV5_058);
		}
	}
}

void Room::love5LookAtPreax() {
	if (_awayMission->love.preaxCured)
		showDescription(TX_LOV5N011);
	else
		showDescription(TX_LOV5N012);
}

void Room::love5TalkToPreax() {
	if (_awayMission->love.preaxCured) {
		if (!_awayMission->love.freedMarcusAndCheever)
			showDescription(TX_LOV5N006);
		else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				TX_LOV5_008,
				TX_LOV5_004,
				TX_LOV5_006,
				TX_END,
			};

			showText(TX_SPEAKER_PREAX, TX_LOV5_059);
			int choice = showMultipleTexts(choices);

			switch (choice) {
			case 1:
				showText(TX_SPEAKER_MARCUS, TX_LOV5_043);
			// fall through
			case 0:
				showText(TX_SPEAKER_PREAX, TX_LOV5_061);
				showText(TX_SPEAKER_KIRK, TX_LOV5_010);

				// ENHANCEMENT: Original text was just "(Spock raises eyebrow)" without any audio.
				// This changes it to a narration to make it flow better.
				showText(TX_SPEAKER_SPOCK, TX_DEM0N009);

				showText(TX_SPEAKER_MCCOY, TX_LOV5_032);

				break;
			case 2:
				showText(TX_SPEAKER_PREAX, TX_LOV5_060);
				break;
			default:
				showDescription(TX_DIALOG_ERROR);
				break;
			}

			showText(TX_SPEAKER_KIRK,   TX_LOV5_002);
			showText(TX_SPEAKER_MARCUS, TX_LOV5_045);

			_awayMission->disableInput = true;

			if (!_awayMission->love.spockCured) {
				showText(TX_SPEAKER_MCCOY, TX_LOV5_021);
				walkCrewmanC(OBJECT_MCCOY, 0x87, 0xc3, &Room::love5MccoyReachedSpockToCure);
			} else
				love5EndMission();
		}
	} else
		showText(TX_SPEAKER_MARCUS, TX_LOV5_018);
}

void Room::love5MccoyReachedSpockToCure() {
	loadActorAnimC(OBJECT_MCCOY, "musemn", -1, -1, &Room::love5MccoyCuredSpock);
}

void Room::love5MccoyCuredSpock() {
	showText(TX_SPEAKER_MCCOY, TX_LOV5_030);
	showText(TX_SPEAKER_SPOCK, TX_LOV5_038);
	showText(TX_SPEAKER_MCCOY, TX_LOV5_027);
	// ENHANCEMENT: Original text was just "(Spock raises eyebrow)" without any audio.
	// This changes it to a narration to make it flow better.
	showText(TX_SPEAKER_SPOCK, TX_DEM0N009);
	love5EndMission();
}

void Room::love5EndMission() {
	showText(TX_SPEAKER_KIRK, TX_LOV5_001);

	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
	walkCrewmanC(OBJECT_KIRK, 0x64, 0xc2, &Room::love5CrewmanReachedBeamoutPosition);

	_awayMission->crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
	walkCrewmanC(OBJECT_SPOCK, 0x5a, 0xb8, &Room::love5CrewmanReachedBeamoutPosition);

	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
	walkCrewmanC(OBJECT_MCCOY, 0x6e, 0xb8, &Room::love5CrewmanReachedBeamoutPosition);

	if (!_awayMission->redshirtDead) {
		_awayMission->crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
		walkCrewmanC(OBJECT_REDSHIRT, 0x69, 0xae, &Room::love5CrewmanReachedBeamoutPosition);
		_roomVar.love.numCrewmenReadyToBeamOut--;
	}
}

void Room::love5CrewmanReachedBeamoutPosition() {
	_roomVar.love.numCrewmenReadyToBeamOut++;
	if (_roomVar.love.numCrewmenReadyToBeamOut == 3) {
		_awayMission->love.missionScore += 17;
		endMission(_awayMission->love.missionScore, 0x2c, 0);
	}
}

void Room::love5TalkToDrMarcus() {
	if (!_awayMission->love.freedMarcusAndCheever)
		showText(TX_SPEAKER_MARCUS, TX_LOV5_040);
	else {
		if (!haveItem(OBJECT_ICURE))
			showText(TX_SPEAKER_MARCUS, TX_LOV5_039);
		else
			showText(TX_SPEAKER_MARCUS, TX_LOV5_042);
	}
}

void Room::love5TalkToDrCheever() {
	if (!_awayMission->love.freedMarcusAndCheever)
		showText(TX_SPEAKER_CHEEVER, TX_LOV5_055);
	else
		showText(TX_SPEAKER_CHEEVER, TX_LOV5_056);
}

void Room::love5TalkToKirk() {
	showText(TX_SPEAKER_KIRK,   TX_LOV5_003);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_048);
	showText(TX_SPEAKER_KIRK,   TX_LOV5_009);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_047);
}

void Room::love5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK,  TX_LOV5_036);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_051);
	showText(TX_SPEAKER_SPOCK,  TX_G_024);
}

void Room::love5TalkToMccoy() {
	if (!_awayMission->love.preaxCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV5_014);
}

void Room::love5TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, TX_LOV5_052);
}

void Room::love5UseSTricorderAnywhere() {
	spockScan(DIR_S, TX_SPEAKER_SPOCK, TX_LOV5_033, false);
}

void Room::love5UseSTricorderOnDevice() {
	spockScan(DIR_N, TX_SPEAKER_SPOCK, TX_LOV5_034, false);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_049);
}

void Room::love5UseMTricorderOnMarcusOrCheever() {
	mccoyScan(DIR_W, TX_LOV5_025, false);
}

void Room::love5UseMTricorderOnPreax() {
	if (!_awayMission->love.preaxCured) {
		if (_awayMission->love.romulansUnconsciousFromLaughingGas)
			mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_LOV5_024, false);
		else
			mccoyScan(DIR_E, TX_SPEAKER_MCCOY, TX_LOV5_016, false);
	}
}

void Room::love5UseMedkitOnPreax() {
	if (_awayMission->love.preaxCured)
		showText(TX_SPEAKER_MCCOY, TX_LOV5_015);
	else
		showText(TX_SPEAKER_MCCOY, TX_LOV5_019);
}

void Room::love5UseWaterOnPreax() {
	if (_awayMission->love.preaxCured) {
		showDescription(TX_LOV5N018);
		showText(TX_SPEAKER_MCCOY, TX_LOV5_029);
		showText(TX_SPEAKER_KIRK,  TX_LOV5_007);
		if (!_awayMission->love.gotPointsForHydratingPreax) {
			_awayMission->love.gotPointsForHydratingPreax = true;
			_awayMission->love.missionScore++;
		}
		loseItem(OBJECT_IH2O);
	} else {
		if (_awayMission->love.romulansUnconsciousFromVirus) {
			showText(TX_SPEAKER_MCCOY, TX_LOV5_013);
			// BUGFIX: original didn't have correct speaker. Also, you shouldn't lose your
			// water since it's not actually used here, so a "loseItem" line was removed.
		} else {
			showDescription(TX_LOV5N017);
			if (!_awayMission->redshirtDead) {
				showText(TX_SPEAKER_FERRIS, TX_LOV5_053);
				showText(TX_SPEAKER_KIRK,   TX_LOV5_005);
			}
			showText(TX_SPEAKER_MCCOY,  TX_LOV5_026);
			loseItem(OBJECT_IH2O);

			// BUGFIX: give a point for hydrating the Preax. It's inconsistent to only
			// give points if he's already cured.
			if (!_awayMission->love.gotPointsForHydratingPreax) {
				_awayMission->love.gotPointsForHydratingPreax = true;
				_awayMission->love.missionScore++;
			}
		}
	}
}

void Room::love5UseCureSampleOnPreax() {
	showText(TX_SPEAKER_MCCOY, TX_LOV5_020); // BUGFIX: original didn't have correct speaker
}


void Room::love5UseCureOnPreax() {
	if (!_awayMission->love.preaxCured)
		walkCrewmanC(OBJECT_MCCOY, 0x116, 0xbf, &Room::love5ReachedPreaxUsingCure);
}

void Room::love5ReachedPreaxUsingCure() {
	loadActorAnimC(OBJECT_MCCOY, "musemn", -1, -1, &Room::love5CuredPreax);
}

void Room::love5CuredPreax() {
	walkCrewman(OBJECT_MCCOY, 0xdc, 0xc3);
	_awayMission->crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_E;

	showDescription(TX_LOV5N007);

	// BUG: says he's dehydrated, but doesn't check whether you've given water to him
	// already (like it does in LOVE4).
	// TODO: make the whole "giving water to romulans" thing consistent between LOVE4 and
	// LOVE5?
	showText(TX_SPEAKER_MCCOY, TX_LOV5_017);

	loadActorAnim2(OBJECT_PREAX, "s3r6r1", 0x116, 0xba);
	_awayMission->love.preaxCured = true;
}


void Room::love5UseAnythingOnPreax() {
	showText(TX_SPEAKER_MCCOY, TX_LOV5_023);
}


// Kirk walks to Cheever, unties him, then walks to Marcus, unties her.
void Room::love5UseKirkOnMarcusOrCheever() {
	if (!_awayMission->love.freedMarcusAndCheever) {
		_awayMission->disableInput = true;
		walkCrewmanC(OBJECT_KIRK, 0x49, 0xb1, &Room::love5KirkReachedCheever);
		_awayMission->love.freedMarcusAndCheever = true;
	}
}

void Room::love5KirkReachedCheever() {
	loadActorAnimC(OBJECT_KIRK, "kuntie", -1, -1, &Room::love5KirkUntiedCheever);
}

void Room::love5KirkUntiedCheever() {
	loadActorAnim2(OBJECT_CHEEVER, "s3gtup", 0x49, 0xac);
	walkCrewmanC(OBJECT_KIRK, 0x26, 0xb1, &Room::love5KirkReachedMarcus);
}

void Room::love5KirkReachedMarcus() {
	loadActorAnimC(OBJECT_KIRK, "kuntie", -1, -1, &Room::love5KirkUntiedMarcus);
}

void Room::love5KirkUntiedMarcus() {
	loadActorStandAnim(OBJECT_KIRK);
	loadActorAnimC(OBJECT_MARCUS, "marcus", 0x28, 0xa6, &Room::love5MarcusStoodUp);
	walkCrewman(OBJECT_KIRK, 0x35, 0xb9);
	_awayMission->crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
}

void Room::love5MarcusStoodUp() {
	_awayMission->disableInput = false;
	showText(TX_SPEAKER_MARCUS, TX_LOV5_041);
}

}
