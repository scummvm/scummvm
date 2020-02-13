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
	showText(TX_SPEAKER_MCCOY, 11, true);

	// BUG: this function has two implementations. The second one (which is shadowed) is
	// below.
	// Perhaps one was supposed to be for IPHASERK, or for Dr. Marcus?
	//showText(TX_SPEAKER_MCCOY, TX_FEA5_008);
}

void Room::love5LookAtKirk() {
	showDescription(04, true);
}

void Room::love5LookAtSpock() {
	showDescription(05, true);
}

void Room::love5LookAtMccoy() {
	showDescription(02, true);
}

void Room::love5LookAtRedshirt() {
	showDescription(3, true);
}

void Room::love5LookAnywhere() {
	showDescription(8, true);

	// There is an unused version of this function, which states "This is Dr. Marcus's
	// quarters, and there are hostages and 4 romulans here". That description doesn't
	// match any actual room.
	// There are LOVE6.MAP and LOVE6.IW files, but none of the other necessary files to
	// load room LOVE6. This may be a leftover from a removed room that somehow ended up
	// here.

	//showText(09, true);
}

void Room::love5LookAtDevice() {
	showDescription(13, true);
	showText(TX_SPEAKER_MCCOY, 28, true);
	showText(TX_SPEAKER_MARCUS, 44, true);
}

void Room::love5LookAtConsole() {
	showDescription(16, true);
	showText(TX_SPEAKER_MCCOY, 31, true);
	showText(TX_SPEAKER_MARCUS, 50, true);
}

void Room::love5LookAtDrMarcus() {
	if (_awayMission->love.freedMarcusAndCheever)
		showDescription(01, true);
	else {
		// BUGFIX: originally played audio "LOV5N001", which is only the first sentence of
		// what should be spoken.
		showDescription(15, true);

		showText(TX_SPEAKER_MARCUS, 46, true);
	}
}

void Room::love5LookAtDrCheever() {
	if (_awayMission->love.freedMarcusAndCheever)
		showDescription(00, true);
	else {
		showDescription(14, true);
		showText(TX_SPEAKER_CHEEVER, 57, true);
		if (!_awayMission->redshirtDead) {
			showText(TX_SPEAKER_FERRIS, 54, true);
			showText(TX_SPEAKER_CHEEVER, 58, true);
		}
	}
}

void Room::love5LookAtPreax() {
	if (_awayMission->love.preaxCured)
		showDescription(11, true);
	else
		showDescription(12, true);
}

void Room::love5TalkToPreax() {
	if (_awayMission->love.preaxCured) {
		if (!_awayMission->love.freedMarcusAndCheever)
			showDescription(06, true);
		else {
			const TextRef choices[] = {
				TX_SPEAKER_KIRK,
				8,
				4,
				6,
				TX_BLANK,
			};

			showText(TX_SPEAKER_PREAX, 59, true);
			int choice = showMultipleTexts(choices, true);

			switch (choice) {
			case 1:
				showText(TX_SPEAKER_MARCUS, 43, true);
			// fall through
			case 0:
				showText(TX_SPEAKER_PREAX, 61, true);
				showText(TX_SPEAKER_KIRK, 10, true);

				// NOTE: This just says "(Spock raises an eyebrow)". No audio associated.
				showText(TX_SPEAKER_SPOCK, TX_LOV5C001);

				showText(TX_SPEAKER_MCCOY, 32, true);

				break;
			case 2:
				showText(TX_SPEAKER_PREAX, 60, true);
				break;
			default:
				showDescription(TX_DIALOG_ERROR);
				break;
			}

			showText(TX_SPEAKER_KIRK,   02, true);
			showText(TX_SPEAKER_MARCUS, 45, true);

			_awayMission->disableInput = true;

			if (!_awayMission->love.spockCured) {
				showText(TX_SPEAKER_MCCOY, 21, true);
				walkCrewmanC(OBJECT_MCCOY, 0x87, 0xc3, &Room::love5MccoyReachedSpockToCure);
			} else
				love5EndMission();
		}
	} else
		showText(TX_SPEAKER_MARCUS, 18, true);
}

void Room::love5MccoyReachedSpockToCure() {
	loadActorAnimC(OBJECT_MCCOY, "musemn", -1, -1, &Room::love5MccoyCuredSpock);
}

void Room::love5MccoyCuredSpock() {
	showText(TX_SPEAKER_MCCOY, 30, true);
	showText(TX_SPEAKER_SPOCK, 38, true);
	showText(TX_SPEAKER_MCCOY, 27, true);
	// NOTE: This just says "(Spock raises an eyebrow)". No audio associated.
	showText(TX_SPEAKER_SPOCK, TX_LOV5C001);
	love5EndMission();
}

void Room::love5EndMission() {
	showText(TX_SPEAKER_KIRK, 01, true);

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
		showText(TX_SPEAKER_MARCUS, 40, true);
	else {
		if (!haveItem(OBJECT_ICURE))
			showText(TX_SPEAKER_MARCUS, 39, true);
		else
			showText(TX_SPEAKER_MARCUS, 42, true);
	}
}

void Room::love5TalkToDrCheever() {
	if (!_awayMission->love.freedMarcusAndCheever)
		showText(TX_SPEAKER_CHEEVER, 55, true);
	else
		showText(TX_SPEAKER_CHEEVER, 56, true);
}

void Room::love5TalkToKirk() {
	showText(TX_SPEAKER_KIRK,    3, true);
	showText(TX_SPEAKER_MARCUS, 48, true);
	showText(TX_SPEAKER_KIRK,    9, true);
	showText(TX_SPEAKER_MARCUS, 47, true);
}

void Room::love5TalkToSpock() {
	showText(TX_SPEAKER_SPOCK,  36, true);
	showText(TX_SPEAKER_MARCUS, 51, true);
	showText(TX_SPEAKER_SPOCK,  TX_G_024);
}

void Room::love5TalkToMccoy() {
	if (!_awayMission->love.preaxCured)
		showText(TX_SPEAKER_MCCOY, 14, true);
}

void Room::love5TalkToRedshirt() {
	showText(TX_SPEAKER_FERRIS, 52, true);
}

void Room::love5UseSTricorderAnywhere() {
	spockScan(DIR_S, 33, false, true);
}

void Room::love5UseSTricorderOnDevice() {
	spockScan(DIR_N, 34, false, true);
	showText(TX_SPEAKER_MARCUS, 49, true);
}

void Room::love5UseMTricorderOnMarcusOrCheever() {
	mccoyScan(DIR_W, 25, false, true);
}

void Room::love5UseMTricorderOnPreax() {
	if (!_awayMission->love.preaxCured) {
		if (_awayMission->love.romulansUnconsciousFromLaughingGas)
			mccoyScan(DIR_E, 24, false, true);
		else
			mccoyScan(DIR_E, 16, false, true);
	}
}

void Room::love5UseMedkitOnPreax() {
	if (_awayMission->love.preaxCured)
		showText(TX_SPEAKER_MCCOY, 15, true);
	else
		showText(TX_SPEAKER_MCCOY, 19, true);
}

void Room::love5UseWaterOnPreax() {
	if (_awayMission->love.preaxCured) {
		showDescription(18, true);
		showText(TX_SPEAKER_MCCOY, 29, true);
		showText(TX_SPEAKER_KIRK,  07, true);
		if (!_awayMission->love.gotPointsForHydratingPreax) {
			_awayMission->love.gotPointsForHydratingPreax = true;
			_awayMission->love.missionScore++;
		}
		loseItem(OBJECT_IH2O);
	} else {
		if (_awayMission->love.romulansUnconsciousFromVirus) {
			showText(TX_SPEAKER_MCCOY, 13, true);
			// BUGFIX: original didn't have correct speaker. Also, you shouldn't lose your
			// water since it's not actually used here, so a "loseItem" line was removed.
		} else {
			showDescription(17, true);
			if (!_awayMission->redshirtDead) {
				showText(TX_SPEAKER_FERRIS, 53, true);
				showText(TX_SPEAKER_KIRK,   05, true);
			}
			showText(TX_SPEAKER_MCCOY,  26, true);
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
	showText(TX_SPEAKER_MCCOY, 20, true); // BUGFIX: original didn't have correct speaker
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

	showDescription(07, true);

	// BUG: says he's dehydrated, but doesn't check whether you've given water to him
	// already (like it does in LOVE4).
	// TODO: make the whole "giving water to romulans" thing consistent between LOVE4 and
	// LOVE5?
	showText(TX_SPEAKER_MCCOY, 17, true);

	loadActorAnim2(OBJECT_PREAX, "s3r6r1", 0x116, 0xba);
	_awayMission->love.preaxCured = true;
}


void Room::love5UseAnythingOnPreax() {
	showText(TX_SPEAKER_MCCOY, 23, true);
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
	showText(TX_SPEAKER_MARCUS, 41, true);
}

}
