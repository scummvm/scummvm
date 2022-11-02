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

namespace StarTrek {

enum vengTextIds {
	TX_COMMON_IDS_OFFSET_START = 5000, // needed to distinguish common IDs
	TX_SPEAKER_KIRK, TX_SPEAKER_MCCOY, TX_SPEAKER_SPOCK, TX_SPEAKER_ELASI_CAPTAIN, TX_SPEAKER_UHURA,
	TX_VEN2_098, TX_VEN2_028, TX_VEN2_066, TX_VENA_F32, TX_VENA_F34,
	TX_VENA_F40, TX_VEN6N007, TX_VEN4N010, TX_VEN2U093, TX_MUD4_018,
	TX_BRIDU146, /*TX_DEM3_019, TX_VEN4_016, TX_G_014,*/ TX_VEN0_016
};

// TODO: Finish floppy offsets
extern const RoomTextOffsets vengTextOffsets[] = {
	{ TX_SPEAKER_KIRK, 6590, 0, 0, 0 },
	{ TX_SPEAKER_MCCOY, 6601, 0, 0, 0 },
	{ TX_SPEAKER_SPOCK, 6611, 0, 0, 0 },
	{ TX_SPEAKER_UHURA, 6672, 0, 0, 0 },
	{ TX_SPEAKER_ELASI_CAPTAIN, 6682, 0, 0, 0 },
	{ TX_VEN2_098, 5016, 0, 0, 0 },
	{ TX_VEN2_028, 6128, 0, 0, 0 },
	{ TX_VEN2_066, 6206, 0, 0, 0 },
	{ TX_VENA_F32, 5246, 0, 0, 0 },
	{ TX_VENA_F34, 5516, 0, 0, 0 },
	{ TX_VENA_F40, 5111, 0, 0, 0 },
	{ TX_VEN6N007, 5735, 0, 0, 0 },
	{ TX_VEN4N010, 5670, 0, 0, 0 },
	{ TX_VEN2U093, 6021, 0, 0, 0 },
	{ TX_MUD4_018, 5820, 0, 0, 0 },
	{ TX_BRIDU146, 5889, 0, 0, 0 },
	//{ TX_DEM3_019, 0, 0, 0, 0 },	// Examining dead guy 1
	//{ TX_VEN4_016, 0, 0, 0, 0 },	// Examining dead guy 2
	//{ TX_G_014, 0, 0, 0, 0 },	// Examining dead guy 3
	{ TX_VEN0_016, 8838, 0, 0, 0 },	// Examining dead guy 4
	{          -1, 0,    0, 0, 0 }
};

void Room::vengaTick() {
	if (_awayMission->veng.counterUntilElasiBoardWithShieldsDown != 0) {
		_awayMission->veng.counterUntilElasiBoardWithShieldsDown--;
		if (_awayMission->veng.counterUntilElasiBoardWithShieldsDown == 0 && _awayMission->veng.poweredSystem != 2) {
			// BUG: Should check you're now lowering the shields in the next phase (this
			// is only meant for when they initially appear with your shields down)
			vengaElasiBeamOver();
			return;
		}
	}

	if (_awayMission->veng.counterUntilElasiAttack != 0) {
		_awayMission->veng.counterUntilElasiAttack--;
		if (_awayMission->veng.counterUntilElasiAttack == 0 && !_awayMission->veng.toldElasiToBeamOver) {
			// BUG: Message should say that they destroyed the ship, since shields are
			// probably up right now...
			vengaElasiBeamOver();
			return;
		}
	}

	if (_awayMission->veng.counterUntilElasiNagToDisableShields != 0) {
		_awayMission->veng.counterUntilElasiNagToDisableShields--;
		if (_awayMission->veng.counterUntilElasiNagToDisableShields == 0 && !_awayMission->veng.firedTorpedo) {
			// Elasi tells you to lower your shields already.
			// BUG: Should add a check to above condition that shields aren't down
			// already?
			showText(TX_SPEAKER_ELASI_CAPTAIN, TX_VEN2_098);
			_awayMission->veng.counterUntilElasiDestroyShip = 2700;
		}
	}

	if (_awayMission->veng.counterUntilElasiDestroyShip != 0) {
		_awayMission->veng.counterUntilElasiDestroyShip--;
		if (_awayMission->veng.counterUntilElasiDestroyShip == 0 && !_awayMission->veng.firedTorpedo) {
			// Elasi fire torpedoes at you since you wouldn't lower your shields.
			showDescription(TX_VENA_F40);
			showGameOverMenu();
			return;
		}
	}

	if (_awayMission->veng.counterUntilElasiBoardWithInvitation != 0) {
		_awayMission->veng.counterUntilElasiBoardWithInvitation--;
		if (_awayMission->veng.counterUntilElasiBoardWithInvitation == 0 && !_awayMission->veng.firedTorpedo) {
			vengaElasiBeamOver();
			return;
		}
	}
}

void Room::vengaElasiBeamOver() {
	showDescription(TX_VENA_F32);
	showGameOverMenu();
}

void Room::vengaUsePhaserAnywhere() {
	showText(TX_SPEAKER_SPOCK, TX_VENA_F34);
}

void Room::vengaLookAtHypo() {
	if (_awayMission->veng.oilInHypo)
		showDescription(TX_VEN6N007);
	else
		showDescription(TX_VEN4N010);
}

void Room::vengaUseCommunicator() {
	// Use communicator in any room except VENG2
	if (!_awayMission->veng.enterpriseLeftForDistressCall) {
		showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
		showText(TX_SPEAKER_UHURA, TX_BRIDU146);
	} else if (!_awayMission->veng.elasiShipDecloaked) {
		showText(TX_SPEAKER_KIRK,  TX_MUD4_018);
		showText(TX_SPEAKER_UHURA, TX_VEN2U093);
		showText(TX_SPEAKER_KIRK,  TX_VEN2_028);
	} else
		showText(TX_SPEAKER_SPOCK, TX_VEN2_066);
}

void Room::vengaUseMccoyOnDeadGuy() {
	showText(TX_SPEAKER_MCCOY, TX_VEN0_016);

	// TODO: Implement the random description again?
#if 0
	int val = getRandomWordInRange(0, 5);

	switch (val) {
	case 0:
		// NOTE: The exact text differs from the text used in DEM3. But, in order for this
		// text indexing scheme to work, two rooms can't use the same audio with different
		// text. Original was "He's dead, jim." instead of "He's dead, Jim...".
		showText(TX_SPEAKER_MCCOY, TX_DEM3_019);
		break;

	case 1:
		showText(TX_SPEAKER_MCCOY, TX_VEN4_016);
		break;

	case 2:
	case 3:
		showText(TX_SPEAKER_MCCOY, TX_G_014);
		break;

	case 4:
	case 5:
		showText(TX_SPEAKER_MCCOY, TX_VEN0_016);
		break;

	default:
		break;
	}
#endif
}

}
