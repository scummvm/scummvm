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

#define OBJECT_8 8
#define OBJECT_PREAX 9
#define OBJECT_MARCUS 10
#define OBJECT_CHEEVER 11

#define HOTSPOT_DEVICE 0x20
#define HOTSPOT_CONSOLE 0x21

namespace StarTrek {

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
	playSoundEffectIndex(SND_DOOR1);
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
		showDescription(TX_LOV5N015);

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
				TX_BLANK,
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

				// NOTE: This just says "(Spock raises an eyebrow)". No audio associated.
				showText(TX_SPEAKER_SPOCK, TX_LOV5C001);

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
	// NOTE: This just says "(Spock raises an eyebrow)". No audio associated.
	showText(TX_SPEAKER_SPOCK, TX_LOV5C001);
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
	spockScan(DIR_S, TX_LOV5_033, false);
}

void Room::love5UseSTricorderOnDevice() {
	spockScan(DIR_N, TX_LOV5_034, false);
	showText(TX_SPEAKER_MARCUS, TX_LOV5_049);
}

void Room::love5UseMTricorderOnMarcusOrCheever() {
	mccoyScan(DIR_W, TX_LOV5_025, false);
}

void Room::love5UseMTricorderOnPreax() {
	if (!_awayMission->love.preaxCured) {
		if (_awayMission->love.romulansUnconsciousFromLaughingGas)
			mccoyScan(DIR_E, TX_LOV5_024, false);
		else
			mccoyScan(DIR_E, TX_LOV5_016, false);
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
