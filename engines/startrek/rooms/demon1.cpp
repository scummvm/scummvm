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

// BUG: under certain circumstances, the klingons just stop firing?

void Room::demon1Tick1() {
	playVoc("DEM1LOOP");

	if (!_vm->_awayMission.demon.beatKlingons)
		playMidiMusicTracks(1, -1);

	if (_vm->_awayMission.demon.enteredFrom == 1) { // Entered from south
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_N;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_N;
	}
	else if (_vm->_awayMission.demon.enteredFrom == 2) { // Entered from north?
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_KIRK] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_SPOCK] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_MCCOY] = DIR_S;
		_vm->_awayMission.crewDirectionsAfterWalk[OBJECT_REDSHIRT] = DIR_S;
	}

	if (!_vm->_awayMission.demon.beatKlingons) {
		_vm->_awayMission.disableWalking = true;
		_vm->_awayMission.timers[2] = 50;
	}
	else {
		loadActorAnim(8, "klg1d2", 0x120, 0x82, 0);
		if (!_vm->_awayMission.demon.tookKlingonHand)
			loadActorAnim(13, "klghnd", 0x10b, 0x8e, 0);
		loadActorAnim(9, "klg2d2", 0xaa, 0x7c, 0);
		loadActorAnim(10, "klg3d2", 0, 0, 0);

		_rdfData[0xca] = 1; // FIXME
		_rdfData[0xcb] = 1;
		_rdfData[0xcc] = 1;
		_roomVar.demon1.numKlingonsKilled = 3;
	}
}

void Room::demon1WalkToCave() {
	if (_roomVar.demon1.numKlingonsKilled != 3)
		return;
	walkCrewman(OBJECT_KIRK, 0xa3, 0x56, 2);
}

void Room::demon1TouchedTopWarp() {
	if (_vm->_awayMission.demon.warpsDisabled)
		return;
	loadRoomIndex(2, 1);
}

void Room::demon1TouchedBottomWarp() {
	if (_vm->_awayMission.demon.warpsDisabled)
		return;
	loadRoomIndex(0, 0);
}

void Room::demon1Timer2Expired() {
	if (_vm->_awayMission.demon.beatKlingons)
		return;
	_vm->_awayMission.demon.field37 = 1;
	_vm->_awayMission.demon.beatKlingons = true;
	_vm->_awayMission.demon.warpsDisabled = true;
	_vm->_awayMission.timers[0] = 5;
	_vm->_awayMission.timers[3] = 2;
	_vm->_awayMission.timers[1] = 100;
	loadActorAnim(8, "klg1u", 0x120, 0x82, 0);
}

void Room::demon1Timer0Expired() {
	loadActorAnim(9, "klg2u", 0xaa, 0x7c, 0);
}

void Room::demon1Timer3Expired() {
	loadActorAnim(10, "klg3u", 0xdb, 0x6d, 0);
}

void Room::demon1Timer1Expired() {
	int shooter;
	Common::String anim;

	if (_rdfData[0xca] != 1) {
		anim = "klg1f";
		shooter = 8;

		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg1fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg1fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg1fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg1fk");
			break;
		default:
			// TODO
			return;
		}
	}
	else if (_rdfData[0xcb] != 1) {
		anim = "klg2f";
		shooter = 9;
		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg2fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg2fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg2fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg2fk");
			break;
		default:
			// TODO
			return;
		}
	}
	else {
		anim = "klg3f";
		shooter = 10;
		switch (_roomVar.demon1.attackIndex) {
		case 0:
			strcpy(_roomVar.demon1.d6, "klg3fr");
			break;
		case 1:
			strcpy(_roomVar.demon1.d6, "klg3fm");
			break;
		case 2:
			strcpy(_roomVar.demon1.d6, "klg3fs");
			break;
		case 3:
			strcpy(_roomVar.demon1.d6, "klg3fk");
			break;
		default:
			// TODO
			return;
		}
	}

	loadActorAnim2(shooter, anim, -1, -1, 2);
}

void Room::demon1KlingonFinishedAimingWeapon() {
	loadActorAnim(11, _roomVar.demon1.d6, 0, 0, 0);
	playSoundEffectIndex(0x06);

	_roomVar.demon1.attackIndex++;
	int crewman;
	if (_roomVar.demon1.attackIndex == 1) {
		loadActorAnim2(OBJECT_REDSHIRT, "rstunn", -1, -1, 0);
		crewman = OBJECT_REDSHIRT;
	}
	else if (_roomVar.demon1.attackIndex == 2) {
		loadActorAnim2(OBJECT_SPOCK, "sstunn", -1, -1, 0);
		crewman = OBJECT_SPOCK;
	}
	else if (_roomVar.demon1.attackIndex == 3) {
		loadActorAnim2(OBJECT_MCCOY, "mstune", -1, -1, 0);
		crewman = OBJECT_MCCOY;
	}
	else if (_roomVar.demon1.attackIndex == 4) {
		loadActorAnim2(OBJECT_KIRK, "kstunn", -1, -1, 1);
		crewman = OBJECT_KIRK;
	}

	_vm->_awayMission.crewDirectionsAfterWalk[crewman] = DIR_N;
	_vm->_awayMission.crewGetupTimers[crewman] = 650;
	_vm->_awayMission.crewDownBitset |= (1 << crewman);

	_vm->_awayMission.timers[1] = 100;
}

void Room::demon1KirkShot() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N000#Captain Kirk is fatally shot by a Klingon, game over.",
		""
	};

	showRoomSpecificText(text);
	showGameOverMenu();
}

void Room::demon1UsePhaserOnAnything() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM1\\DEM1_011#The fire fight is over, Jim. I think you can put that away now.",
		""
	};

	if (_roomVar.demon1.numKlingonsKilled == 3)
		showRoomSpecificText(text);
}

void Room::demon1UsePhaserOnKlingon1() {
	if (_rdfData[0xca] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xca] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon1();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 3);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon1() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks1", 0, 0, 0);
	playSoundEffectIndex(0x06);
	playVoc("EFX19");
	loadActorAnim2(8, "klg1d", 0x120, 0x84, 7);
}

void Room::demon1KlingonDropsHand() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N020#You see a small explosion, and the Klingon's hand falls to the ground with a dull thud.",
		""
	};
	loadActorAnim(13, "klghnd", 0x10b, 0x8e, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
	_vm->_awayMission.timers[1] = 0;
	showRoomSpecificText(text);

	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		return;

	const char *rshirtText[] = {
		"Ensign Everts",
		"#DEM1\\DEM1_025#I guess they don't make Klingons like they used to, Sir.",
		""
	};
	showRoomSpecificText(rshirtText);

	if (_roomVar.demon1.numKlingonsKilled == 3)
		return;

	const char *klingonText[] = {
		"Klingon",
		"#DEM1\\DEM1_F23#Federation Scum!",
		""
	};
	showRoomSpecificText(klingonText);

	_vm->_awayMission.timers[1] = 1;
}

void Room::demon1UsePhaserOnKlingon2() {
	if (_rdfData[0xcb] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcb] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon2();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 4);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon2() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks2", 0, 0, 0);
	playSoundEffectIndex(0x06);
	loadActorAnim2(9, "klg2d", 0xaa, 0x7c, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
}

void Room::demon1UsePhaserOnKlingon3() {
	if (_rdfData[0xcc] != 0)
		return;
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcc] = 1;

	if (_roomVar.demon1.kirkShooting) {
		demon1ShootKlingon3();
	}
	else {
		loadActorAnim2(OBJECT_KIRK, "kfiren", -1, -1, 5);
		_roomVar.demon1.kirkShooting = true;
	}
}

void Room::demon1ShootKlingon3() {
	_roomVar.demon1.numKlingonsKilled++;
	if (_roomVar.demon1.numKlingonsKilled == 3) {
		_vm->_awayMission.timers[1] = 0;
		_vm->_awayMission.timers[5] = 180;
		_vm->_awayMission.timers[6] = 1;
		_vm->_awayMission.demon.warpsDisabled = false;
		_vm->_awayMission.disableWalking = false;
	}
	loadActorAnim(12, "s0ks3", 0, 0, 0);
	playSoundEffectIndex(0x06);
	loadActorAnim2(10, "klg3d", 0, 0, 0);
	_vm->_awayMission.transitioningIntoRoom = 0;
}

// Timer 6 expired
void Room::demon1AllKlingonsDead() {
	_vm->_awayMission.crewGetupTimers[OBJECT_KIRK] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_SPOCK] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_MCCOY] = 45;
	_vm->_awayMission.crewGetupTimers[OBJECT_REDSHIRT] = 45;
}

void Room::demon1Timer5Expired() {
	if (_vm->_awayMission.crewDownBitset != 0)
		return;

	const char *text1[] = {
		"Lt. Uhura",
		"#DEM1\\DEM1U077#Captain we registered Phaser fire and an unknown energy beam. Is everyone OK?",
		""
	};
	const char *text2[] = {
		"Capt. Kirk",
		"#DEM1\\DEM1_003#We're fine. Did you register any Disruptor fire?",
		""
	};
	const char *text3[] = {
		"Lt. Uhura",
		"#DEM1\\DEM1U078#No, Captain. Why, are Klingons down there?",
		""
	};
	const char *text4[] = {
		"Capt. Kirk",
		"#DEM1\\DEM1_002#No just an idea, Kirk out.",
		""
	};
	const char *text5[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_020#Fascinating. I begin to suspect that we have stumbled upon something that the colonists would never have uncovered.",
		""
	};
	const char *text6[] = {
		"Capt. Kirk",
		"#DEM1\\DEM1_004#What is it, Spock?",
		""
	};
	const char *text7[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_021#I wish to gather further data before making a definite conclusion, Captain.",
		""
	};

	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
	showRoomSpecificText(text4);
	showRoomSpecificText(text5);
	showRoomSpecificText(text6);
	showRoomSpecificText(text7);
}


void Room::demon1UseMTricorderOnKlingon() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM1\\DEM1_012#This is definitely not a real klingon Jim.",
		""
	};
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnTulips() {
	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_007#Khytellian tulips are a perennial flower that survive in almost any climate.",
		""
	};
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnPods() {
	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_008#The Brandzite pod is similar to terran milkweed except that the silk pods are in bright iridecent colors.",
		""
	};
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnCattails() {
	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_005#Doctis Cattails are similar to their terran name sake except that they are known to cause hives if in contact with flesh for any amount of time.",
		""
	};
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnFerns() {
	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_006#Gindorian ferns are regarded as an intergalactic weed, Captain.",
		""
	};
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnHand() {
	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_017#This is a detached hand with some kind of circuitry in the palm, Captain.",
		""
	};
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1UseSTricorderOnKlingon1() {
	// NOTE: it sets up the stack, but doesn't call the function...
	//loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_roomVar.demon1.numKlingonsKilled == 3 && !_vm->_awayMission.demon.tookKlingonHand && _rdfData[0xcf] != 1) {
		const char *text[] = {
			"Mr. Spock",
			"#DEM1\\DEM1_018#This is not a Klingon, Captain, not a real one. It is an organic construct -- an android-like robot. It looks like a Klingon, but the appearance is entirely superficial. There is something different about this particular construct. Come here, Captain, look at the hand.It seems to have been separated from the body. There is a wiring circuit in the middle of the palm.",
			""
		};
		showRoomSpecificText(text);
		_rdfData[0xcf] = 1;
	}
	else {
		const char *text[] = {
			"Mr. Spock",
			"#DEM1\\DEM1_019#This is not a Klingon, Captain, not a real one. It is an organic construct -- an android-like robot. It looks like a Klingon, but the appearance is entirely superficial.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon1UseSTricorderOnKlingon2Or3() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		"Mr. Spock",
		"#DEM1\\DEM1_019#This is not a Klingon, Captain, not a real one. It is an organic construct -- an android-like robot. It looks like a Klingon, but the appearance is entirely superficial.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1UseMTricorderOnKirk() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		demon1UseMTricorderOnCrewman();
}

void Room::demon1UseMTricorderOnCrewman() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM1\\DEM1_009#He's only stunned. He'll be back up in a few seconds.",
		""
	};
	loadActorAnim2(OBJECT_MCCOY, "mscann", -1, -1, 0);
	playSoundEffectIndex(0x04);
	showRoomSpecificText(text);
}

void Room::demon1GetHand() {
	if (_roomVar.demon1.numKlingonsKilled != 3)
		return;
	walkCrewman(OBJECT_KIRK, 0x104, 0x8e, 1);
}

void Room::demon1ReachedHand() {
	loadActorAnim2(OBJECT_KIRK, "s5r1kg", -1, -1, 6);
}

void Room::demon1PickedUpHand() {
	loadActorStandAnim(13);
	_vm->_awayMission.missionScore += 3;
	loadActorStandAnim(OBJECT_KIRK);
	_vm->_awayMission.timers[4] = 2;
}

// Timer 4 expired
void Room::demon1FinishedGettingHand() {
	const char *text1[] = {
		nullptr,
		"#DEM1\\DEM1N005#You already took the hand from the Klingon.",
		""
	};
	const char *text2[] = {
		nullptr,
		"#DEM1\\DEM1N007#You take the Klingon's detached hand.",
		""
	};

	if (_vm->_awayMission.demon.tookKlingonHand)
		showRoomSpecificText(text1);
	else {
		_vm->_awayMission.demon.tookKlingonHand = true;
		giveItem(OBJECT_IHAND);
		showRoomSpecificText(text2);
	}
}

void Room::demon1LookAtKlingon() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N004#They look like Klingons.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtCattails() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N018#These are very beautiful Doctis Cattails.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtTulips() {
	const char *text[] = {
		nullptr,
		"M1\\DEM1N010#A large patch of Khytellian Tulips.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtPods() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N019#These Brandzite Pods add a nice touch to the local flora.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtFerns() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N009#A Gindorian Fern.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtStream() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N011#A small stream flows down towards the forest.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtMine() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N021#You see the mine entrance ahead.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtMountain() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N016#Mt. Idyll rises above you.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtHand() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N023#It's the Klingon's detached hand!",
		""
	};
	showRoomSpecificText(text);
}
void Room::demon1LookAnywhere() {
	const char *text[] = {
		nullptr,
		"#DEM1\\DEM1N017#The path is surrounded by some beautiful shrubbery.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon1LookAtKirk() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK)) {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N012#Captain Kirk is unconscious.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N003#James Kirk, filled with a premonition of more dangers yet to come.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon1LookAtSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK)) {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N015#Mr. Spock is unconscious.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N008#Your Vulcan science officer seems to be lost in thought, but remains alert.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon1LookAtMcCoy() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_MCCOY)) {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N013#Dr. McCoy is unconscious.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N001#Dr. McCoy, still hoping the cold winds that whip around Mount Idyll will soon die down.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon1LookAtRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT)) {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N014#Ensign Everts is unconscious.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			nullptr,
			"#DEM1\\DEM1N002#Ensign Everts seems to be rattled by the attack of the Klingons.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon1TalkToKirk() {
	const char *text[] = {
		"Capt. Kirk",
		"#DEM1\\DEM1_001#We were caught flat footed there. I don't want any more surprises to catch us off guard.",
		""
	};
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_KIRK))
		demon1TalkToUnconsciousCrewman();
	else if (_roomVar.demon1.numKlingonsKilled == 3)
		showRoomSpecificText(text);
}

void Room::demon1TalkToSpock() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_SPOCK))
		demon1TalkToUnconsciousCrewman();
	else {
		const char *text1[] = {
			"Mr. Spock",
			"#DEM1\\DEM1_022#Captain, I detect a recent avalanche, approximately 6.2 kilometers away, that occurred within the last three days. The mountain may be quite dangerous.",
			""
		};
		const char *text2[] = {
			"Dr. McCoy",
			"#DEM1\\DEM1_013#Demons, Klingons, avalanches... What's next, the Wicked Witch of the West?",
			""
		};
		const char *text3[] = {
			"Mr. Spock",
			"#DEM1\\DEM1_024#That is not logical, doctor.",
			""
		};
		const char *text4[] = {
			"Dr. McCoy",
			"#DEM1\\DEM1_014#It wasn't supposed to be logical, you green blooded Vulcan! Why does everything have to be so damned logical?",
			""
		};
		showRoomSpecificText(text1);
		showRoomSpecificText(text2);
		showRoomSpecificText(text3);
		showRoomSpecificText(text4);
	}
}

void Room::demon1TalkToMcCoy() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_MCCOY))
		demon1TalkToUnconsciousCrewman();
	else {
		const char *text1[] = {
			"Dr. McCoy",
			"#DEM1\\DEM1_015#Well, we've seen Klingons. Now all we need is a few Romulans...",
			""
		};
		const char *text2[] = {
			"Mr. Spock",
			"#DEM1\\DEM1_023#Control your thoughts, Doctor. There is a high probability that something here is using our own memories against us.",
			""
		};
		showRoomSpecificText(text1);
		showRoomSpecificText(text2);
	}
}

void Room::demon1TalkToRedshirt() {
	if (_vm->_awayMission.crewDownBitset & (1 << OBJECT_REDSHIRT))
		demon1TalkToUnconsciousCrewman();
	else {
		const char *text[] = {
			"Ensign Everts",
			"#DEM1\\DEM1_026#I guess this isn't such a great planet after all.",
			""
		};
		showRoomSpecificText(text);
	}
}

// FIXME: this doesn't happen in actual game? (does the event get blocked from higher up?)
void Room::demon1TalkToUnconsciousCrewman() {
	const char *text[] = {
		nullptr,
		"Zzzzz....",
		""
	};
	showRoomSpecificText(text);
}

}
