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

void Room::demon0Tick1() {
	playVoc("DEM0LOOP");
	loadActorAnim(9, "s0r0d3", 252, 153, 0);
	loadActorAnim(10, "s0r0d4", 158, 130, 0);
}

void Room::demon0Tick2() {
	if (_vm->_awayMission.field33 == 1)
		return;
	loadActorAnim(8, "prel", 120, 190, 0);
}

void Room::demon0Tick60() {
	if (_vm->_awayMission.field33 == 1)
		return;
	loadActorAnim2(8, "prelclap", 120, 190, 0);
}

void Room::demon0Tick100() {
	const char *text[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_036#This is so much better, gentlefolk. We are honored at your presence and hope you will find peace here in our haven.",
		""
	};

	showRoomSpecificText(text);
}

void Room::demon0Tick140() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_016#Captain, the flora on this planet is very interesting. I wonder how useful it may be for medicinal purposes.",
		""
	};

	if (_vm->_awayMission.field38 == 1)
		return;

	showRoomSpecificText(text);

	_vm->_awayMission.field38 = 1;
}

void Room::demon0TouchedWarp0() {
	_vm->_awayMission.field3a = 1;
	_vm->_awayMission.rdfStillDoDefaultAction = true;
}

void Room::demon0WalkToBottomDoor() {
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcd] = 1; // FIXME
	_vm->_awayMission.field25[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 243, 158, 1);
}

void Room::demon0TouchedHotspot1() {
	if (_rdfData[0xcd] != 1) // FIXME: not portable to mac/amiga?
		return;
	loadActorAnim2(9, "s0r0d1", 254, 153, 1);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedBottomDoor() {
	if (++_rdfData[0xca] != 2) // FIXME
		return;
	loadRoomIndex(5, 0);
}

void Room::demon0WalkToTopDoor() {
	_vm->_awayMission.transitioningIntoRoom = 1;
	_rdfData[0xcc] = 1; // FIXME
	_vm->_awayMission.field25[OBJECT_KIRK] = DIR_E;
	walkCrewman(OBJECT_KIRK, 157, 134, 2);
}

void Room::demon0TouchedHotspot0() {
	if (_rdfData[0xcc] != 1) // FIXME: not portable to mac/amiga?
		return;
	loadActorAnim2(10, "s0r0d2", 158, 130, 2);
	playSoundEffectIndex(0x05);
}

void Room::demon0ReachedTopDoor() {
	if (++_rdfData[0xcb] != 2) // FIXME
		return;
	loadRoomIndex(6, 3);
}

void Room::demon0TalkToPrelate() {
	const char *options1[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_006#I'm Captain James T. Kirk of the U.S.S. Enterprise. We have received word that alien lifeforms are creating problems at your mining facilities at Idyll Mountain. Tell me more.",
		"#DEM0\\DEM0_008#Most High Prelate Angiven, I am honored to meet you. I consider it my divine duty to assist you in any possible way with the spawn of the devil.",
		"#DEM0\\DEM0_003#Been seeing ghosts and bogeymen eh? I find that a little hard to believe.",
		""
	};
	const char *firstResponse0[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_038#Certainly, Captain Kirk. Not aliens, per se -- we have encountered what we believe are demons at Idyll Mountain, creatures surely emerging from the very gates of Hell. Our God would not test us thus without reason, so we believe your might and insight are our God's method to help us discover what is going on.",
		""
	};
	const char *firstResponse1[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_032#Captain Kirk. I had no idea we were blessed with one of our order in the ranks of Starfleet. We believe we have located the very Gates of Hell below Idyll Mountain.",
		""
	};
	const char *options2[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_002#Aside from seeing demons, has any hard data been collected? Any evidence I could see?",
		"#DEM0\\DEM0_005#Demons? Gates of Hell? This is the 23rd Century!",
		""
	};
	const char *secondResponse[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_031#A skeptic would consider everything merely anecdotal or unproven. My people will gladly tell you their own stories, so you need not hear it secondhand through me.",
		""
	};
	const char *options3[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_010#What can you tell me about the mine itself?",
		"#DEM0\\DEM0_012#You're wasting the time of a starship capable of destroying this planet with campfire stories? No wonder you were dumped out here in the middle of nowhere.",
		""
	};
	const char *thirdResponse[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_035#The area is exceptionally stable tectonically, and easy for our machinery to work in, praise God. We've mined for hafnium and a variety of useful trace elements. The deeper we dig, however, the more anomalous the variety of minerals seems to be. Our Ignaciate, Brother Stephen, has his own theories about why this might be. Either way, the anomalies inspired Brother Kandrey to conduct studies inside the mine. Yesterday, he reported discovering a strange door -- a gate to Hell, surely, for the demons caused a cave-in immediately. Kandrey was trapped, unconscious, and the demons prevent us from rescuing him. We can only hope he is still alive.",
		""
	};

	const char *badConclusion[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_037#We need your help, Kirk. You may have no respect for our beliefs, but I hope you will look beyond that. Godspeed.",
		""
	};
	const char *goodConclusion[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_034#Thank you for your courtesy, Kirk. May you receive the guidance and protection of our God as you complete this divine mission.",
		""
	};

	if (_vm->_awayMission.field33 != 0)
		return;

	_vm->_awayMission.missionScore += 3;
	_vm->_awayMission.field33 = 1;

	const char **response = nullptr;

	switch (showRoomSpecificText(options1)) {
	case 0:
		response = firstResponse0;
		break;
	case 1:
		response = firstResponse1;
		break;
	case 2:
		demon0BadResponse();
		break;
	}

	if (response != nullptr)
		showRoomSpecificText(response);

	if (showRoomSpecificText(options2) == 1)
		demon0BadResponse();

	showRoomSpecificText(secondResponse);

	if (showRoomSpecificText(options3) == 1)
		demon0BadResponse();

	showRoomSpecificText(thirdResponse);

	if (_vm->_awayMission.field29 != 0)
		showRoomSpecificText(badConclusion);
	else
		showRoomSpecificText(goodConclusion);
}

void Room::demon0LookAtPrelate() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N004#High Prelate Angiven waits patiently for you to decide what you will do next.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0UsePhaserOnSnow() {
	const char *text[] = {
		"Ensign Everts",
		"#DEM0\\DEM0_039#Aw, Captain, please don't melt the snow. I've never seen it before.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0UsePhaserOnSign() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_017#Quite the vandal aren't we, Jim?",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0UsePhaserOnShelter() {
	const char *text1[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_015#Burning down their house is not the best way to gain their confidence Jim!",
		""
	};
	const char *text2[] = {
		"Mr. Spock",
		"#DEM0\\DEM0_013#Captain, Doctor McCoy is correct.",
		""
	};
	const char *text3[] = {
		"Ensign Everts",
		"#DEM0\\DEM0_040#Is he always this trigger happy on ground missions?",
		""
	};
	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	if (!_vm->_awayMission.redshirtDead)
		showRoomSpecificText(text3);
}

void Room::demon0UsePhaserOnPrelate() {
	const char *text[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_020#Jim! Are you out of your mind?",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtSign() {
	const char *text[] = {
		"Sign",
		"All mining equipment use this road.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtTrees() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N006#On the other side of the trees is Idyll Mountain. A tall, forbidding place. You have a vague feeling of danger.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtSnow() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N007#Remnants of a recent snowfall cover the ground.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAnywhere() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N000#A beautiful, snow covered scene with a path leading off to Mount Idyll.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtBushes() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N010#Various bushes and shrubs grow along the edge of the forest.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtKirk() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N005#James Tiberius Kirk, Captain of the Enterprise. He's always happy to run an errand of mercy.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtMcCoy() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N002#Dr. Leonard McCoy, the finest doctor in Starfleet, wishes that he were on a warmer planet.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtRedShirt() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N003#Ensign Everts, who has never been this close to snow before in his life, gazes with child-like fascination at the ground.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtSpock() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N009#Spock raises an eyebrow.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0LookAtShelter() {
	const char *text[] = {
		nullptr,
		"#DEM0\\DEM0N001#A quickly constructed spartan shelter, primarily used by fledgling colonies.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0TalkToKirk() {
	const char *text1[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_009#This planet's as beautiful as everyone says it is.",
		""
	};
	const char *text2[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_027#The trees. The fresh air. The freezing cold...",
		""
	};
	const char *text3[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_004#C'mon Bones, the cold will improve your circulation.",
		""
	};
	const char *text4[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_024#Some people get too much circulation!",
		""
	};

	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
	showRoomSpecificText(text4);
}

void Room::demon0TalkToRedshirt() {
	const char *text1[] = {
		"Ensign Everts",
		"#DEM0\\DEM0_043#I've never seen snow like this before. This is great!",
		""
	};
	const char *text2[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_026#You mean you've never built a snowman, ensign?",
		""
	};
	const char *text3[] = {
		"Ensign Everts",
		"#DEM0\\DEM0_042#I've never even thrown a snowball. Do you think anyone would mind?",
		""
	};
	const char *text4[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_025#Well...",
		""
	};
	const char *text5[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_007#Later Ensign. We have work to do.",
		""
	};
	const char *text6[] = {
		"Ensign Everts",
		"#DEM0\\DEM0_044#Of course, sir.",
		""
	};

	showRoomSpecificText(text1);
	showRoomSpecificText(text2);
	showRoomSpecificText(text3);
	showRoomSpecificText(text4);
	showRoomSpecificText(text5);
	showRoomSpecificText(text6);
}

void Room::demon0TalkToMcCoy() {
	if (_vm->_awayMission.field33 == 1) {
		const char *text1[] = {
			"Capt. Kirk",
			"#DEM0\\DEM0_011#You look rather cold, Bones.",
			""
		};
		const char *text2[] = {
			"Dr. McCoy",
			"#DEM0\\DEM0_023#I'm not cold, I'm freezing! And that damn transporter just had to set me down in the middle of a snow drift!",
			""
		};
		const char *text3[] = {
			"Mr. Spock",
			"#DEM0\\DEM0_029#A centimeter of snow does not technically constitute a drift, Doctor.",
			""
		};
		const char *text4[] = {
			"Ensign Everts",
			"#DEM0\\DEM0_041#And doctors say that patients complain too much!",
			""
		};

		showRoomSpecificText(text1);
		showRoomSpecificText(text2);
		showRoomSpecificText(text3);
		if (!_vm->_awayMission.redshirtDead)
			showRoomSpecificText(text4);
	}
	else {
		const char *text1[] = {
			"Dr. McCoy",
			"#DEM0\\DEM0_019#I don't know if the problem is real, the result of a new illness, or mass hysteria. But at the very least, there's an injured miner who needs my help.",
			""
		};

		showRoomSpecificText(text1);
		if (_vm->_awayMission.field36 != 1) {
			demon0AskPrelateAboutSightings();
		}
	}
}

void Room::demon0TalkToSpock() {
	const char *text1[] = {
		"Mr. Spock",
		"#DEM0\\DEM0_014#Captain, demons and supernatural creatures are, almost by definition, illogical. Yet it is evident these people believe what they have seen. Barring illness or mass hysteria, I agree that a real problem seems to exist.",
		""
	};

	if (_vm->_awayMission.field33 == 1) {
		showRoomSpecificText(text1);
	}
	else {
		showRoomSpecificText(text1);
		if (_vm->_awayMission.field36 != 1)
			demon0AskPrelateAboutSightings();
	}
}

void Room::demon0AskPrelateAboutSightings() {
	const char *text2[] = {
		"Capt. Kirk",
		"#DEM0\\DEM0_001#Doctor, you need to investigate the possibility of disease, mental or physical, among these people, before we go chasing up the mountains. Prelate Angiven, may we see those who have encountered the demons?",
		""
	};
	const char *text3[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_030#They are already gathered in the chapel, and will cooperate in any way with you. First door on my right.",
		""
	};

	showRoomSpecificText(text2);
	showRoomSpecificText(text3);

	_vm->_awayMission.field36 = 1;
}

void Room::demon0UseSTricorderAnywhere() {
	loadActorAnim2(OBJECT_SPOCK, "sscans", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		"Mr. Spock",
		"#DEM0\\DEM0_028#Captain, reading the rocks in this area, I find that this locality may have been disturbed in the distant past. Recent disturbances created by the colonists' construction and mining prevent me from discovering anything further.",
		""
	};

	showRoomSpecificText(text);
}

void Room::demon0UseMTricorderAnywhere() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);

	const char *text[] = {
		"Dr. McCoy",
		"#DEM0\\DEM0_021#Jim, I am not picking up any unusual life here, just native lifeforms and the settlers. The colonists are all human, except for the one Tellarite. If there are demons here, they don't register on my tricorder.",
		""
	};
	showRoomSpecificText(text);
}

void Room::demon0UseMTricorderOnPrelate() {
	loadActorAnim2(OBJECT_MCCOY, "mscans", -1, -1, 0);
	playSoundEffectIndex(0x04);

	if (_vm->_awayMission.field33 == 1) {
		const char *text[] = {
			"Dr. McCoy",
			"#DEM0\\DEM0_018#His blood pressure's up a bit, but he believes he's telling the truth.",
			""
		};
		showRoomSpecificText(text);
	}
	else {
		const char *text[] = {
			"Dr. McCoy",
			"#DEM0\\DEM0_022#The man's in perfect health.",
			""
		};
		showRoomSpecificText(text);
	}
}

// Helper functions
void Room::demon0BadResponse() {
	const char *text[] = {
		"Prel. Angiven",
		"#DEM0\\DEM0_033#Starfleet recognizes our freedom to worship and believe as we see fit, Captain. I am surprised that you do not share that feeling. Rest assured that Starfleet Command will be informed of your rudeness.",
		""
	};

	if (_vm->_awayMission.field29 != 0)
		return;

	_vm->_awayMission.missionScore -= 3;
	_vm->_awayMission.field29 = 1;

	showRoomSpecificText(text);
}
}

