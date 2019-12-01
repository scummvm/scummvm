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

namespace StarTrek {

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
			showDescription(40 + COMMON_MESSAGE_OFFSET, true);
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
	showDescription(32 + COMMON_MESSAGE_OFFSET, true);
	showGameOverMenu();
}

void Room::vengaUsePhaserAnywhere() {
	showText(TX_SPEAKER_SPOCK, 34 + COMMON_MESSAGE_OFFSET, true);
}

void Room::vengaLookAtHypo() {
	if (_awayMission->veng.oilInHypo)
		showDescription(TX_VEN4N007);
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
}

}
