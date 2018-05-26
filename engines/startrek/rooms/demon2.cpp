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

void Room::demon2Tick1() {
	playVoc("DEM2LOOP");
}

void Room::demon2WalkToCave() {
	walkCrewman(OBJECT_KIRK, 0x83, 0x5c, 2);
}

void Room::demon2ReachedCave() {
	loadRoomIndex(3, 1);
}

void Room::demon2TouchedWarp1() {
	_vm->_awayMission.demon.enteredFrom = 2;
	_vm->_awayMission.rdfStillDoDefaultAction = true;
}

void Room::demon2LookAtCave() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N009#Mine entrance.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtMountain() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N007#Idyll Mountain.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtBerries() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N012#Various types of berries grow amongst the bushes.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtFern() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N005#A Gindorian fern.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtMoss() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N010#Phequedian moss grows on the cave wall.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtLights() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N006#Feeble lights illuminate the mine tunnel.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtAnything() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N011#The mine entrance goes deep into Mount Idyll.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtKirk() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N002#James T. Kirk. Always wondering what the next surprise will be.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtSpock() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N004#Spock. Perhaps the most brilliant mind in Starfleet, pondering a most peculiar mystery.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtMcCoy() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N003#Leonard McCoy. Hoping that the cave will be warmer than outside.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2LookAtRedshirt() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N000#Ensign Everts. Keeping a sharp eye out for anything dangerous.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2TalkToKirk() {
	const char *text[] = {
		SPEAKER_KIRK,
		"#DEM2\\DEM2_001#The answer to this mystery lies ahead of us, gentlemen.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2TalkToSpock() {
	const char *text1[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_018#Whoever was trying to stop us may not stop with those Klingons, Captain. I recommend extreme caution.",
		""
	};
	const char *text2[] = {
		SPEAKER_KIRK,
		"#DEM2\\DEM2_003#The thought had occurred to me, Mr. Spock, but thank you for mentioning it.",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
}

void Room::demon2TalkToMcCoy() {
	const char *text1[] = {
		SPEAKER_MCCOY,
		"#DEM2\\DEM2_015#Does your tricorder say the cave is warmer, Spock...",
		""
	};
	const char *text2[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_017#It is not logical for me to use my tricorder to determine the cave's temperature, doctor. I do not see what purpose it would serve.",
		""
	};
	const char *text3[] = {
		SPEAKER_MCCOY,
		"#DEM2\\DEM2_016#Spock, everybody talks about the weather!",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
}

void Room::demon2TalkToRedshirt() {
	const char *text1[] = {
		SPEAKER_EVERTS,
		"#DEM2\\DEM2_019#I'm sorry I let you down with those Klingons back there. I should have been paying more attention.",
		""
	};
	const char *text2[] = {
		SPEAKER_KIRK,
		"#DEM2\\DEM2_002#Just don't make that mistake again, Ensign.",
		""
	};
	const char *text3[] = {
		SPEAKER_EVERTS,
		"#DEM2\\DEM2_020#Those Klingons give me the willies. They always have. My sister was wounded by them in the Chozon ambush.",
		""
	};
	const char *text4[] = {
		SPEAKER_KIRK,
		"#DEM2\\DEM2_004#We've all had our share of conflict with Klingons, Ensign. The Organians told me that one day Humans and Klingons will become good friends. I wonder if I'll ever live to see that day.",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
	showRoomSpecificText(text4);
}

void Room::demon2UsePhaserOnBerries() {
	const char *text[] = {
		SPEAKER_MCCOY,
		"#DEM2\\DEM2_011#Jim, you usually pick food and then cook it.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2UseSTricorderOnBerries() {
	loadActorAnim2(OBJECT_SPOCK, "sscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_008#These seem to be Laraxian berries, Captain. They have several medicinal uses, but Dr. McCoy would know more.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2UseSTricorderOnMoss() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_007#The Phequedine moss extracts nourishment from hafnium, Captain.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2UseSTricorderOnFern() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_006#Gindorian ferns are regarded as an intergalactic weed, Captain.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2UseSTricorderOnCave() {
	loadActorAnim2(OBJECT_SPOCK, "sscann", -1, -1, 0);
	playSoundEffectIndex(0x04);

	// NOTE: audio file for this is empty.
	const char *text[] = {
		SPEAKER_SPOCK,
		"#DEM2\\DEM2_005#Captain, I'm having trouble getting reliable readings from inside the mountain. Unusual crystalline structures within the ore may be at fault, though I cannot say for certain.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon2UseMTricorderOnBerries() {
	loadActorAnim2(OBJECT_MCCOY, "mscane", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.demon.curedBrother) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM2\\DEM2_013#These are the Laraxian Berries we used to cure Brother Chub.",
			""
		};
		showRoomSpecificText(text);
	}
	else if (_vm->_awayMission.demon.madeHypoDytoxin) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM2\\DEM2_009#Jim, these are Laraxian Berries that we used to make the Hypo-Dytoxin.",
			""
		};
		showRoomSpecificText(text);
	}
	else if (_vm->_awayMission.demon.knowAboutHypoDytoxin) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM2\\DEM2_010#Jim, these are the berries we need to synthesize the Hypo-Dytoxin. We must get these to Brother Stephen quickly.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM2\\DEM2_012#These are Laraxian Berries. From what I remember, certain chemical compounds in the berry can be used to treat Nugaireyn infections.",
			""
		};
		showRoomSpecificText(text);
	}
}

void Room::demon2GetBerries() {
	if (_vm->_awayMission.demon.gotBerries) {
		const char *text[] = {
			SPEAKER_MCCOY,
			"#DEM2\\DEM2_014#We don't need any more berries, Jim.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		walkCrewman(OBJECT_KIRK, 0xe8, 0x97, 1);
	}
}

void Room::demon2ReachedBerries() {
	loadActorAnim2(OBJECT_KIRK, "kusehe", -1, -1, 1);
	playVoc("PIKBERRY");
}

void Room::demon2PickedUpBerries() {
	const char *text[] = {
		nullptr,
		"#DEM2\\DEM2N013#You have retrieved a sample of berries.",
		""
	};
	showRoomSpecificText(text);
	giveItem(OBJECT_IBERRY);
	_vm->_awayMission.demon.gotBerries = true;
	_vm->_awayMission.missionScore += 1;
}

}
