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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#include "teenagent/scene.h"
#include "teenagent/teenagent.h"
#include "teenagent/resources.h"
#include "teenagent/dialog.h"

namespace TeenAgent {

#define CHECK_FLAG(addr, v) (res->dseg.get_byte(addr) == (v))
#define SET_FLAG(addr, v) (res->dseg.set_byte((addr), (v)))
#define GET_FLAG(addr) (res->dseg.get_byte(addr))
#define INC_FLAG(addr) (++*res->dseg.ptr(addr))

void TeenAgentEngine::rejectMessage() {
	Resources * res = Resources::instance();
	//random reject message:
	uint i = random.getRandomNumber(3);
	//debug(0, "reject message: %s", (const char *)res->dseg.ptr(res->dseg.get_word(0x339e + 2 * i)));
	displayMessage(res->dseg.get_word(0x339e + 2 * i));
}


bool TeenAgentEngine::processCallback(uint16 addr) {
	if (addr == 0)
		return false;

	Resources * res = Resources::instance();
	debug(0, "processCallback(%04x)", addr);
	byte * code = res->cseg.ptr(addr);

	//try trivial callbacks first
	if (code[0] == 0xbb && code[3] == 0xe8 && code[6] == 0xc3) {
		//call display_message, r
		uint16 msg = READ_LE_UINT16(code + 1);
		uint16 func = 6 + addr + READ_LE_UINT16(code + 4);
		debug(0, "call %04x", func);
		//debug(0, "trivial callback, showing message %s", (const char *)res->dseg.ptr(addr));
		switch (func) {
		case 0x11c5:
			Dialog::show(scene, msg);
			return true;
		case 0xa055:
			displayMessage(msg);
			return true;
		}
	}

	if (code[0] == 0xe8 && code[3] == 0xc3) {
		uint func = 3 + addr + READ_LE_UINT16(code + 1);
		debug(0, "call %04x and return", func);
		if (func == 0xa4d6) {
			rejectMessage();
			return true;
		}
	}

	if (code[0] == 0xc7 && code[1] == 0x06 && code[2] == 0xf3 && code[3] == 0xb4 &&
	        code[6] == 0xb8 && code[9] == 0xbb && code[12] == 0xbf &&
	        code[22] == 0xe8 && code[25] == 0xc3) {
		loadScene(code[4], Common::Point(
		              (READ_LE_UINT16(code + 7) + READ_LE_UINT16(code + 13) + 1) / 2 ,
		              READ_LE_UINT16(code + 10)));
		scene->setOrientation(code[21]);
		return true;
	}

	switch (addr) {

	case 0x024c: //intro
		hideActor();

		loadScene(41, 139, 156, 3);
		playSound(41, 12);
		playAnimation(912, 1);
		setOns(0, 108);
		playSound(62, 8);
		playSound(58, 40);
		playAnimation(913, 1);
		setOns(1, 109);
		setLan(2, 1);
		Dialog::show(scene, 0x748e, 914, 915, 0xe7, 0xd7, 2, 1);
		displayCredits(0xe3c2);
		loadScene(42, 139, 156, 3);
		playSound(15, 20);
		playAnimation(916, 1);
		playSound(40, 18);
		playSound(40, 22);
		for (byte i = 27; i < 37; i += 2)
			playSound(40, i);
		playSound(29, 44);
		playAnimation(918, 0, true);
		playAnimation(917, 1, true);
		waitAnimation();
		displayCredits(0xe3e6);

		loadScene(40, 139, 156, 3);
		playMusic(3);
		Dialog::show(scene, 0x750d, 920, 924, 0xe7, 0xeb); //as i told you, our organization...
		playSound(26, 50);
		playAnimation(925, 0, true);
		playAnimation(926, 1, true);
		waitAnimation();
		Dialog::show(scene, 0x78a6, 920, 927, 0xeb, 0xeb);
		displayCredits(0xe3ff);

		loadScene(39, 139, 156, 3);
		playMusic(11);
		playSound(81, 2);
		playSound(81, 8);
		playSound(81, 11);
		playSound(81, 14);
		playSound(81, 16);
		playSound(81, 18);
		playSound(81, 20);
		playSound(81, 21);
		playAnimation(928, 1);
		setOns(0, 112);
		Dialog::show(scene, 0x78e1, 929, 0, 0xd1); //he's coming
		showActor();
		moveTo(319, 150, 1, true);
		moveTo(63, 150, 1);
		displayAsyncMessage(0x5da8, 19844, 18, 36);
		playAnimation(851, 0);
		playSound(24, 11);
		playActorAnimation(931);
		displayCredits(0xe42f);

		playMusic(3);
		loadScene(40, 50, 186, 1);
		setOns(0, 113);
		Dialog::show(scene, 0x78f1, 919, 0, 0xe7);
		moveTo(196, 186, 1);
		Dialog::show(scene, 0x7958, 0, 920, 0xd1, 0xe7);
		playActorAnimation(932);
		Dialog::show(scene, 0x7e07, 920, 0, 0xe7);
		playActorAnimation(932);
		Dialog::show(scene, 0x7e1a, 920, 0, 0xe7);
		playActorAnimation(932);
		Dialog::show(scene, 0x7e2c, 922, 0, 0xe7);
		playActorAnimation(933);
		Dialog::show(scene, 0x7e70, 920, 0, 0xe7);
		moveTo(174, 186, 1);
		playAnimation(851, 0, true);
		playActorAnimation(934, true);
		waitAnimation();
		loadScene(10, 136, 153);

		return true;

	case 0x4021:
		//pulling out mysterious object
		if (CHECK_FLAG(0xdbe1, 1)) {
			playActorAnimation(844);
			playActorAnimation(846);
			playActorAnimation(845);
			displayMessage(0x5696);
		} else {
			displayMessage(0x570f);
		}
		return true;

	case 0x4094: //climbing to the pole near mudpool
		if (CHECK_FLAG(0xDBE4, 1)) {
			displayMessage(0x57b2);
			return true;
		} else {
			for (byte i = 11; i <= 27; i += 4)
				playSound(76, i);
			
			playSound(56, 35);
			playSound(19, 59);
			playActorAnimation(864);
			playAnimation(865, 1);
			playActorAnimation(866);
			//InventoryObject *obj = inventory->selectedObject();
			//if (obj != NULL && obj->id == 0x55) {

			//implement pause and using real object:
			if (inventory->has(0x55)) {
				playSound(5, 4);
				playSound(5, 19);
				playSound(64, 11);
				playActorAnimation(867);
				inventory->remove(0x55);
				inventory->add(0x56);
				moveTo(86, 195, 1, true);
				playActorAnimation(868);
				SET_FLAG(0xDBE4, 1);
			} else {
				processCallback(0x4173);
				Dialog::pop(scene, 0xDB72);
			}
			return true;
		}
	case 0x4173:
		//fail!
		moveTo(86, 195, 1, true);
		playActorAnimation(868);
		return true;

	case 0x419c: //getting the bird
		setOns(0, 0);
		playSound(56, 10);
		playActorAnimation(875);
		disableObject(6);
		inventory->add(0x5c);
		return true;


	case 0x41ce:
		moveTo(197, 159, 4);
		setOns(0, 0);
		playSound(71, 8);
		playActorAnimation(833);
		moveTo(225, 159, 4);
		inventory->add(0x4e);
		disableObject(3);
		return true;

	case 0x4267:
		playSound(23, 8);
		playSound(24, 13);
		setOns(1, 0);
		playActorAnimation(841);
		setOns(1, 0x61);
		setOns(2, 0);
		playSound(63, 12);
		playSound(5, 26);
		playActorAnimation(842);
		hideActor();
		//shown in different positions
		displayMessage(0x5656);
		displayMessage(0x567a);
		displayMessage(0x5682);
		playActorAnimation(843);
		showActor();
		moveTo(223, 149, 0, true);
		disableObject(7);
		disableObject(1);
		inventory->add(0x51);
		displayMessage(0x5646);
		return true;

	case 0x4388:
		playSound(80, 4);
		playActorAnimation(961);
		loadScene(8, 155, 199, 1);
		return true;

	case 0x43b5: //HQ, first trial - prison
		playSound(70, 6);
		playActorAnimation(962);
		loadScene(7, 30, 184, 2);
		if (res->dseg.get_byte(0xDBDF) < 2) {
			moveTo(134, 167, 2);
			displayMessage(0x54f7);
			setLan(1, 0);
			playAnimation(812, 0, true);
			playActorAnimation(811);
			Dialog::show(scene, 0x6117, 0, 813, 0xd1, 0xec, 0, 1);
			loadScene(6, Common::Point(230, 184));
			Dialog::show(scene, 0x626a, 0, 814, 0xd1, 0xec, 0, 1);
			playSound(4, 14);
			playAnimation(815, 0);
			setOns(1, 0);

			Dialog::show(scene, 0x62dc);

			SET_FLAG(0xDBDF, 1);
			playMusic(5);
		}
		return true;

	case 0x4482:
		if (CHECK_FLAG(0xDBDF, 0)) {
			playActorAnimation(968);
			displayMessage(0x5511);
		} else {
			playSound(80, 3);
			playSound(79, 4);
			playActorAnimation(968);
			loadScene(6, 280, 186, 4);
		}
		return true;

	case 0x44fc:  //pull out spring from bed
		playSound(53, 25);
		playSound(24, 27);
		playSound(5, 36);
		playActorAnimation(839);
		moveTo(278, scene->getPosition().y, 0, true);
		inventory->add(0x50);
		disableObject(1);
		return true;

	case 0x44cb:
		if (CHECK_FLAG(0xDBE5, 1)) {
			displayMessage(0x57c0);
		} else {
			playSound(49, 14);
			playSound(5, 21);
			playActorAnimation(869);
			inventory->add(0x58);
			SET_FLAG(0xDBE5, 1);
		}
		return true;

	case 0x4539: //prison cell: use crates
		if (CHECK_FLAG(0xdbdd, 2)) {
			//finished the meal - trap
			displayMessage(0x55c0);
			moveTo(306, 196, 2);
			//playAnimation(825, 1); //very long empty animation. what for?
			setLan(1, 0);
			playSound(71, 4);
			playActorAnimation(823);

			loadScene(5, scene->getPosition());
			playSound(74, 1);
			playSound(74, 3);
			playSound(74, 6);
			playActorAnimation(826);
			loadScene(6, scene->getPosition());
			setOns(3, 0x5b);
			displayMessage(0x55db);
			SET_FLAG(0xdbdd, 3);
			scene->getObject(4)->setName("body");
		} else {
			if (Dialog::pop(scene, 0xdb5c) != 0x636b) //not 'im getting hungry'
				return true;

			playSound(52, 8);
			playAnimation(820, 1);
			setOns(3, 0x59);
			//some moving animation is missing here
			displayMessage(0x551f);
			enableObject(4);
			SET_FLAG(0xdbdc, 1);
		}
		return true;

	case 0x4662:
		if (CHECK_FLAG(0xDBDD, 3)) {
			if (CHECK_FLAG(0xDBDE, 1)) {
				displayMessage(0x5608);
			} else {
				moveTo(280, 179, 2);
				playSound(49, 7);
				playSound(5, 17);
				playActorAnimation(827);
				inventory->add(0x4d);
				SET_FLAG(0xDBDE, 1);
			}
		} else
			displayMessage(0x5905);
		return true;

	case 0x46af: //prison cell: use live cable
		if (CHECK_FLAG(0xdbdc, 1)) {
			displayMessage(0x555d);
			setOns(2, 0);
			playActorAnimation(821);
			setOns(2, 0x5a);
			setOns(3, 0);
			playSound(22, 2);
			playActorAnimation(822);
			displayMessage(0x5577);
			disableObject(5);
			SET_FLAG(0xdbdd, 1);
		} else
			displayMessage(0x5528);
		return true;

	case 0x4705:  //prison: getting lamp bulb
		moveTo(144, 185, 4);
		playSound(56, 15);
		setOns(0, 86); //hiding lamp
		playActorAnimation(816, true);
		playAnimation(817, 1, true);
		waitAnimation();
		setOns(0, 87);

		playSound(34, 1);
		playSound(5, 15);
		playActorAnimation(818, true);
		playAnimation(819, 1, true);
		waitAnimation();

		moveTo(160, 188, 1, true);
		setOns(2, 88);

		disableObject(6);
		enableObject(5);
		inventory->add(0x4c);
		return true;

	case 0x4794: //prison cell door
		if (res->dseg.get_byte(0xDBDF) >= 2) {
			loadScene(5, 287, 143);
		} else {
			displayMessage(0x592f);
		}
		return true;

	case 0x47bc: //prison: examining trash can
		playSound(79, 5);
		playSound(1, 14);
		playActorAnimation(966);
		displayMessage(0x5955);
		return true;

	case 0x47db: //prison: use switch
		if (CHECK_FLAG(0xDBDF, 1)) {
			playSound(71, 4);
			playActorAnimation(823);
			if (CHECK_FLAG(0xDBDD, 0)) {
				displayMessage(0x4d80);
			} else {
				playSound(74, 1);
				playAnimation(824, 1);
				if (CHECK_FLAG(0xDBDD, 1)) {
					displayMessage(0x559a);
					SET_FLAG(0xDBDD, 2);
				}
			}
		} else {
			displayMessage(0x52f6);
		}
		return true;

	case 0x4871:
		playActorAnimation(965);
		displayMessage(0x5511);
		return true;

	case 0x4893: //taking pills
		if (CHECK_FLAG(0xDBE6, 1)) {
			SET_FLAG(0xDBE6, 2);
			setOns(1, 0x67);
			playSound(5, 9);
			playActorAnimation(872);
			inventory->add(0x5a);
			disableObject(7);
		} else {
			playActorAnimation(964);
			displayMessage(0x5511);
		}
		return true;

	case 0x4918: //talking with barmen
		if (CHECK_FLAG(0xDBE7, 1)) {
			moveTo(140, 152, 1);
			if (CHECK_FLAG(0xDBE8, 1)) {
				Dialog::show(scene, 0x6f20);
				displayMessage(0x5883, 0xef);
				//reloadLan();
				setLan(1, 0);
				playAnimation(882, 0);
				playSound(75, 10);
				setOns(2, 0);
				playSound(75, 10);
				playSound(24, 15);
				playAnimation(883, 0);
				disableObject(1);
				disableObject(2);
				SET_FLAG(0xDBE9, 1);
			} else
				displayMessage(0x5855);
		} else {
			if (CHECK_FLAG(0xDBDF, 3)) {
				if (CHECK_FLAG(0xDBE3, 1)) {
					Dialog::show(scene, 0x6BD6, 0, 857, 0xd1, 0xef, 0, 1);
				} else {
					Dialog::show(scene, 0x69B5, 0, 857, 0xd1, 0xef, 0, 1); //taking mug
					playActorAnimation(859, true);
					playAnimation(858, 0, true);
					waitAnimation();
					playSound(75, 6);
					playActorAnimation(860);
					Dialog::show(scene, 0x69C2, 0, 857, 0xd1, 0xef, 0, 1);
					inventory->add(0x55);
					SET_FLAG(0xDBE3, 1);
					SET_FLAG(0xDBF0, 0);
				}
			} else {
				Dialog::pop(scene, 0xDB68, 0, 857, 0xd1, 0xef, 0, 1);
			}
		}
		return true;

	case 0x4f14: //use the hollow
		displayMessage(CHECK_FLAG(0xDBA1, 1) ? 0x370f : 0x36c2);
		return true;

	case 0x4a64:
		if (CHECK_FLAG(0xDBF0, 1)) {
			displayMessage(0x5e25);
		} else {
			loadScene(5, 35, 162);
		}
		return true;

	case 0x4bf5:
		playActorAnimation(959);
		loadScene(8, 40, 152, 3);
		return true;

	case 0x483a:
		Dialog::pop(scene, 0xdb82);
		return true;

	case 0x4844:
		playSound(80, 4);
		playActorAnimation(963);
		loadScene(5, 166, 158);
		return true;

	case 0x48ea:
		setOns(0, 0);
		playSound(5, 9);
		playActorAnimation(836);
		inventory->add(0x4f);
		disableObject(12);
		return true;

	case 0x4a8c:
		if (CHECK_FLAG(0xDBE9, 1)) {
			playSound(89, 5);
			playActorAnimation(958);
			loadScene(9, 240, 182, 4);
		} else if (CHECK_FLAG(0xDBE9, 1)) {
			displayMessage(0x5894);
		} else {
			Dialog::pop(scene, 0xDB8A, 0, 857, 0xd1, 0xef, 0, 1);
		}
		return true;

	case 0x4af4: //taking the crumbs
		setOns(0, 0);
		playSound(49, 6);
		playSound(5, 13);
		playActorAnimation(861);
		inventory->add(0x57);
		disableObject(6);
		return true;

	case 0x4b35:
		playSound(15, 7);
		playActorAnimation(884);
		playSound(55, 1);
		playSound(24, 12);
		playAnimation(885, 0);
		Dialog::show(scene, 0x67e5, 886, 0, 0xd0, 0xd1, 1, 0);
		playMusic(3);
		loadScene(40, 198, 186, 1);
		Dialog::show(scene, 0x7f20, 0, 920, 0xd1, 0xe7);
		inventory->clear();
		inventory->add(0x1d);
		displayCredits(0xe45c);
		loadScene(1, 198, 186);
		hideActor();
		playActorAnimation(956, true);
		Dialog::show(scene, 0x8bc4);
		waitAnimation();
		loadScene(15, 157, 199, 1);
		playMusic(6);
		return true;

	case 0x4c3e: //get the grenade
		playSound(32, 24);
		playActorAnimation(862);
		reloadLan();
		playAnimation(863, 1);
		inventory->add(0x54);
		disableObject(1);
		SET_FLAG(0xDBE2, 2);
		return true;

	case 0x4c70:
		if (CHECK_FLAG(0xDBE2, 0)) {
			if (CHECK_FLAG(0xDBDA, 1)) { //papers are shown
				Dialog::pop(scene, 0xDB4C, 0, 809, 0xd1, 0xd0, 0, 1);
			} else {
				Dialog::pop(scene, 0xDB40, 0, 809, 0xd1, 0xd0, 0, 1);
			}
		} else {
			displayMessage(0x5722);
			displayMessage(0x572a);
		}
		return true;

	case 0x4c1c:
		playActorAnimation(960);
		displayMessage(0x5511);
		return true;

	case 0x4cac:
		if (CHECK_FLAG(0xdbda, 1)) { //papers are shown
			loadScene(5, 124, 199);
		} else {
			Dialog::show(scene, 0x5FE9, 0, 809, 0xd1, 0xd0, 0, 1);
			moveTo(269, 175, 4);
			Dialog::pop(scene, 0xDB56, 0, 809, 0xd1, 0xd0, 0, 1);
		}
		return true;

	case 0x4cf1: { //talking with mansion guard
		SET_FLAG(0xda96, 1);
		if (Dialog::pop(scene, 0xdaa6, 0, 529, 0xd1, 0xd9, 0, 1) != 0x1b4)
			return true;

		Common::Point p = scene->getPosition();
		moveTo(189, 159, 0);
		//waitLanAnimationFrame(1, 1);

		playSound(5, 2);
		playSound(5, 19);
		playActorAnimation(550, true);
		playAnimation(551, 0, true);
		waitAnimation();

		moveTo(p, 2);
		inventory->add(0x13);
		playAnimation(529, 1);
		Dialog::pop(scene, 0xdaa6, 0, 529, 0xd1, 0xd9, 0, 1);
	}
	return true;

	case 0x4d94: //talking with fatso
		Dialog::show(scene, 0x33bd, 0, 666, 0xd1, 0xd0, 0, 2);
		displayMessage(0x49ae, 0xd0, 25060);
		playSound(5, 3);
		playAnimation(667, 1);
		playAnimation(668, 1);
		setOns(2, 50);
		Dialog::show(scene, 0x36c7, 0, 666, 0xd1, 0xd0, 0, 2);
		setOns(3, 0);
		SET_FLAG(0xDBEC, 0);
		reloadLan();
		playSound(82, 19);
		playAnimation(669, 1);
		Dialog::show(scene, 0x3779, 0, 0, 0xd1, 0xd1, 0, 0);
		enableObject(15);
		disableObject(8);
		return true;

	case 0x4e61:
		loadScene(14, 280, 198);
		return true;

	case 0x4ee5:
		setOns(2, 0);
		playSound(5, 12);
		playActorAnimation(676);
		displayMessage(0x4ab0);
		disableObject(15);
		inventory->add(51);
		return true;

	case 0x4d56:
		inventory->add(16);
		disableObject(2);
		setOns(0, 0);
		playSound(5, 12);
		playActorAnimation(547);
		return true;


	case 0x4eb9://Pick up wrapper
		playSound(5, 12);
		playSound(5, 18);
		inventory->add(0x12);
		setOns(1, 0);
		playActorAnimation(549);
		disableObject(13);
		return true;

	case 0x4f25:
		playActorAnimation(967);
		displayMessage(0x3542);
		return true;

	case 0x4f32: //use tree near the mansion
		if (CHECK_FLAG(0xDBA1, 1)) {
			if (CHECK_FLAG(0xDBA2, 1)) {
				displayMessage(0x3766);
			} else {
				playSound(26, 13);
				playSound(26, 15);
				playSound(26, 23);
				playSound(26, 25);
				playSound(26, 32);
				playSound(26, 34);
				playSound(26, 36);
				playActorAnimation(590);
				moveTo(204, 178, 3, true);
				playSound(59, 1);
				playSound(60, 16);
				playActorAnimation(591);
				displayMessage(0x372e);
				SET_FLAG(0xDBA2, 1);
				processCallback(0x9d45);
			}
		} else {
			playActorAnimation(49);
			playSound(56, 8);
			playSound(56, 12);
			playSound(49, 10);
			displayAsyncMessage(0x4652, 31579, 16, 24);
			playActorAnimation(587);
			displayMessage(0x3668);
		}
		return true;

	case 0x500d: //picking up wild plant
		if (CHECK_FLAG(0xDB9E, 1)) {
			displayMessage(0x35E8); //there are no more
		} else {
			SET_FLAG(0xDB9E, 1);
			setOns(2, 0);
			playSound(21, 9);
			playSound(34, 21);
			playSound(26, 30);
			playActorAnimation(552);
			setOns(2, 0x12);
			inventory->add(0x14);
		}
		return true;

	case 0x5104:
		loadScene(11, 319, 198, 4);
		if (CHECK_FLAG(0xDB9C, 1))
			return true;

		//guard's drinking
		SET_FLAG(0, 3);
		setTimerCallback(0x516d, 40);
		playAnimation(544, 0, true, true); //ignore busy flag for this animation
		return true;
		
	case 0x516d: //too late to scare guard, resetting
		SET_FLAG(0, 0);
		return true;

	case 0x5189: //guard's drinking, boo!
		SET_FLAG(0, 0);
		setTimerCallback(0, 0);
		scene->getAnimation(0)->free();
		SET_FLAG(0xDB9C, 1); 
		
		displayAsyncMessage(0x3563, 320 * 130 + 300, 1, 5);
		setOns(0, 16);
		enableObject(2);

		playSound(17, 5);
		playAnimation(545, 0);

		Dialog::show(scene, 0x0917, 0, 546, 0xd1, 0xd9, 0, 1);
		SET_FLAG(0xDA96, 1);
		SET_FLAG(0xDA97, 0);
		return true;

	case 0x51f0:
		setOns(0, 0);
		playSound(5, 11);
		playActorAnimation(637);
		disableObject(7);
		inventory->add(49);
		return true;

	case 0x5217:
		displayMessage(CHECK_FLAG(0xDB9F, 1) ? 0x402e : 0x34e1);
		return true;

	case 0x5237:
		if (!CHECK_FLAG(0xDB9F, 1)) {
			displayMessage(0x34e1);
		} else if (CHECK_FLAG(0xDBA0, 1))
			displayMessage(0x3E31);
		else {
			moveTo(173, 138, 2);
			playSound(28, 5);
			playActorAnimation(583);
			playActorAnimation(584);

			loadScene(0, 0, 0, 0); //clear background

			playSound(72, 18);
			playSound(73, 39);
			playActorAnimation(585);

			loadScene(11, 194, 160, 2);
			playSound(28, 2);
			playActorAnimation(586);
			moveTo(138, 163, 3);
			displayMessage(0x3650);
			SET_FLAG(0xDBA0, 1);
			processCallback(0x9d45); //another mansion try
		}
		return true;

	case 0x55a8: {
		uint16 d = Dialog::pop(scene, 0xdb08);
		if (d == 0x2c5d) {
			waitLanAnimationFrame(1, 0x23);
			setOns(0, 0);
			playSound(52, 9);
			playSound(52, 11);
			playSound(52, 13);
			playSound(53, 32);
			playAnimation(570, 0);
			displayMessage(0x551f);
			disableObject(5);
			SET_FLAG(0xDBB0, 1);
		} else if (d != 0x2c9b) {
			waitLanAnimationFrame(1, 0x23);
			playSound(52, 9);
			playSound(52, 11);
			playSound(52, 13);
			playAnimation(569, 0);
		}
	}
	return true;

	case 0x5663:
		displayMessage(CHECK_FLAG(0xDBB0, 1) ? 0x41b1 : 0x417e);
		return true;

	case 0x569c:
		playSound(67, 5);
		playActorAnimation(983);
		displayMessage(0x5955);
		return true;

	case 0x56b7:
		playSound(66, 5);
		playSound(67, 11);
		playActorAnimation(984);
		displayMessage(0x5955);
		return true;

	case 0x5728:
		inventory->add(0x0d);
		disableObject(14);
		setOns(0, 0);
		playSound(5, 10);
		playActorAnimation(566);
		return true;

	case 0x5793:
		if (!CHECK_FLAG(0xDB94, 1)) {
			displayMessage(0x3e63);
		} else if (CHECK_FLAG(0xDB95, 1)) {
			displayMessage(0x3e75);
		} else {
			SET_FLAG(0xDB95, 1);
			moveTo(188, 179, 0);
			playSound(7, 16);
			playActorAnimation(519);
			moveTo(168, 179, 2);
			inventory->add(3);
		}
		return true;

	case 0x5d88:
		if (CHECK_FLAG(0xDBA5, 1)) { //dry laundry
			SET_FLAG(0xDBA5, 2);
			Dialog::show(scene, 0x1F4F, 0, 523, 0xd1, 0xe5, 0, 1);
			//waitLanAnimationFrame(1, 1); //another long waiting
			playAnimation(604, 0);

			loadScene(21, scene->getPosition());
			setOns(0, 0);
			disableObject(4);
			enableObject(12);
			playSound(46, 5);
			playAnimation(606, 1);
			setOns(0, 33);
			loadScene(23, scene->getPosition());
			playAnimation(605, 0);
			Dialog::show(scene, 0x2002, 0, 523, 0xd1, 0xe5, 0, 1);
		} else {
			uint16 d = Dialog::pop(scene, 0xdada, 0, 523, 0xd1, 0xe5, 0, 1);
			if (d == 0x1913)
				displayMessage(0x34d5); //+orientation = 3
		}
		return true;

	case 0x5ff3: //get duster
		if (CHECK_FLAG(0xDB9A, 0)) {
			Dialog::pop(scene, 0xdaf6, 0, 523, 0xd1, 0xe5, 0, 1);
		} else {
			Dialog::show(scene, 0x1e1e, 0, 523, 0xd1, 0xe5, 0, 1);
			inventory->add(12);
			disableObject(12);
			setOns(0, 0);
			playSound(5, 6);
			playActorAnimation(541);
		}
		return true;
		
	case 0x603e:
		if (CHECK_FLAG(0xDBB3, 1)) {
			displayMessage(0x44a7);
		} else {
			displayMessage(0x4412);
			displayMessage(0x444f);
			displayMessage(0x446b);
			displayMessage(0x4492);
			SET_FLAG(0xDBB3, 1);
		}
		return true;

	case 0x6205:
		if (CHECK_FLAG(0xDBA4, 1))
			displayMessage(0x450e);
		else
			processCallback(0x61fe);
		return true;

	case 0x6217:
		if (CHECK_FLAG(0xDBA4, 1))
			displayMessage(0x44d6);
		else
			processCallback(0x61fe);
		return true;

	case 0x62c1:
		if (CHECK_FLAG(0xDBA4, 1))
			return false;

		processCallback(0x61fe);
		return true;

	case 0x63bc:
		playMusic(6);
		loadScene(25, 151, 156, 2);
		return true;

	case 0x646e:
	case 0x6475:
		Dialog::show(scene, 0x32C1);
		return true;

	case 0x6507:
		if (CHECK_FLAG(0xDB96, 1)) {
			rejectMessage();
		} else
			displayMessage(0x47e7);
		return true;

	case 0x65c3:
		if (CHECK_FLAG(0xDBA9, 1)) {
			playActorAnimation(635);
			setOns(5, 0);
			playSound(63, 11);
			playSound(15, 20);
			playSound(32, 31);
			playActorAnimation(636);
			inventory->add(47);
			inventory->add(48);
			moveTo(scene->getPosition().x - 1, 139, 1, true);
			displayMessage(0x3b83);
			SET_FLAG(0xDBA9, 2);
			SET_FLAG(0xDBA8, 0);
		} else
			displayMessage(0x4808);
		return true;

	case 0x7866:
		if (CHECK_FLAG(0xdbdd, 3)) {
			displayMessage(0x55ff);
			return true;
		} else
			return false;

	case 0x7878: {
		byte v = res->dseg.get_byte(0xDBDB) + 1;
		if (v <= 6)
			SET_FLAG(0xDBDB, v);

		switch (v) {
		case 1:
			displayMessage(0x5411);
			return true;
		case 2:
			displayMessage(0x5463);
			return true;
		case 3:
			displayMessage(0x5475);
			return true;
		case 4:
			displayMessage(0x5484);
			return true;
		case 5:
			displayMessage(0x54c4);
			return true;
		default:
			displayMessage(0x54d5);
			return true;
		}
	}

	case 0x78a9:
		if (CHECK_FLAG(0xDBE6, 1)) {
			displayMessage(0x5827);
			return true;
		} else
			return false;

	case 0x78bb:
		if (CHECK_FLAG(0xDBE8, 1)) {
			displayMessage(0x58b0);
			return true;
		} else
			return false;

	case 0x78ce:
		if (!CHECK_FLAG(0xDBA1, 1)) {
			displayMessage(0x3694);
			return true;
		} else
			return false;

	case 0x792b: //left click on ann
		moveTo(245, 198, 1);
		if (CHECK_FLAG(0xDBAF, 1))
			return false;

		Dialog::show(scene, 0x2193);
		SET_FLAG(0xDBAF, 1);
		return true;

	case 0x79c3:
		if (CHECK_FLAG(0xDBA4, 1))
			return false;
		processCallback(0x61fe);
		return true;

	case 0x7b26: //cutting the fence
		setOns(0, 0);
		playSound(5, 2);
		playSound(51, 11);
		playSound(51, 23);
		playActorAnimation(837);
		playSound(51, 3);
		playSound(51, 19);
		playSound(23, 26);
		playActorAnimation(838);
		setOns(0, 0x60);
		moveTo(281, scene->getPosition().y, 0, true);
		disableObject(4);
		SET_FLAG(0xDBE1, 1);
		return true;

	case 0x7b89: //digging mysterious object
		if (CHECK_FLAG(0xDBE1, 1)) {
			playActorAnimation(844);
			setOns(1, 0);
			playSound(5, 5);
			playSound(26, 19);
			playSound(24, 25);
			playActorAnimation(847);
			playSound(5, 11);
			playActorAnimation(848);
			setOns(1, 0x64);
			playActorAnimation(845);
			disableObject(3);
			inventory->add(0x52);
			inventory->remove(0x51);
		} else
			displayMessage(0x56da);
		return true;

	case 0x7bfd:
		playSound(76, 18);
		playSound(76, 22);
		playSound(76, 26);
		playSound(76, 30);
		playSound(76, 34);
		playSound(76, 47);
		playSound(76, 51);
		playSound(76, 55);
		playSound(76, 59);
		playSound(76, 63);
		playActorAnimation(873);
		moveTo(240, 163, 4);
		displayMessage(0x5837);
		waitLanAnimationFrame(1, 0x22);
		playSound(77, 2);
		playSound(77, 12);
		playSound(77, 16);
		playSound(77, 20);
		playSound(77, 34);
		playSound(78, 41);
		playSound(78, 51);
		playSound(56, 63);
		playSound(24, 67);
		playSound(23, 76);
		setLan(1, 0);
		playAnimation(874, 1);
		setOns(0, 0x68);
		inventory->remove(0x5b);
		enableObject(6);
		disableObject(1);
		return true;

	case 0x7ce5: //put spring on the solid ground
		playSound(5, 2);
		playSound(19, 11);
		playActorAnimation(840);
		setOns(1, 0x61);
		inventory->remove(0x50);
		disableObject(2);
		enableObject(7);
		return true;

	case 0x7d1a: //captain's key + door
		if (res->dseg.get_byte(0xDBDF) <= 1) {
			playSound(5, 2);
			playSound(57, 12);
			playSound(70, 19);
			playActorAnimation(828);
			moveTo(262, 160, 1, true);
			disableObject(4);
			disableObject(3);
			setOns(0, 0);
			setOns(1, 85);
			setOns(2, 0);
			setOns(3, 0);
			loadScene(5, scene->getPosition());
			setOns(0, 92);
			playAnimation(829, 1);
			setOns(0, 0);
			Dialog::show(scene, 0x63a5, 830, 0, 0xec, 0xd1, 1, 0);
			loadScene(7, 130, 195);
			playMusic(4);
			setLan(1, 1);

			Dialog::show(scene, 0x6406, 0, 832, 0xd1, 0xec, 0, 1);

			//playAnimation(831, 1);

			SET_FLAG(0xDBDF, 2);

		} else
			displayMessage(0x52f6);
		return true;

	case 0x7e02: //tickling the captain
		if (CHECK_FLAG(0xdbe0, 1)) {
			displayMessage(0x5632);
		} else {
			playSound(5, 6);
			playSound(27, 49);
			playActorAnimation(834, true);
			playAnimation(835, 1, true);
			waitAnimation();

			setOns(0, 94);
			Dialog::show(scene, 0x65e9, 0, 832, 0xd1, 0xec, 0, 1);
			enableObject(12);
			SET_FLAG(0xdbe0, 1);
		}
		return true;

	case 0x7e4f: //giving magazine to captain
		Dialog::show(scene, 0x66c0, 0, 856, 0xd1, 0xec, 0, 1);
		playSound(5, 3);
		playActorAnimation(852, true);
		playActorAnimation(853, true);
		displayMessage(0x5742);
		displayMessage(0x5757);
		displayMessage(0x5770);
		displayMessage(0x5782);
		displayMessage(0x5799);
		playAnimation(856, 1);
		playSound(5, 3);
		//playActorAnimation(854);
		Dialog::show(scene, 0x66fe, 0, 856, 0xd1, 0xec, 0, 1);
		playAnimation(855, 1);
		moveTo(30, 181, 0);
		disableObject(1);
		setLan(1, 0);
		SET_FLAG(0xDBDF, 3);
		SET_FLAG(0xDBF0, 1);
		loadScene(8, 155, 199);
		return true;

	case 0x7fbd: //using bird & bartender
		playSound(5, 3);
		playActorAnimation(876);
		setOns(1, 0);
		playSound(26, 7);
		playSound(79, 15);
		playAnimation(877, 1);
		playAnimation(880, 1, true);

		Dialog::show(scene, 0x6f0e, 857);
		setOns(2, 0x6a);
		reloadLan();
		playAnimation(878, 0);
		//playAnimation(879, 0); //background bartender animation
		inventory->remove(0x5c);
		enableObject(1);
		SET_FLAG(0xDBE7, 1);
		return true;

	case 0x8047:
		playSound(32, 5);
		playSound(5, 17);
		playSound(52, 23);
		playActorAnimation(881);
		setOns(2, 0x6b);
		inventory->remove(0x56);
		inventory->add(0x55);
		SET_FLAG(0xDBE8, 1);
		return true;

	case 0x808b:
		if (CHECK_FLAG(0xDBDA, 1)) {
			//alredy shown
			displayMessage(0x53F2);
		} else {
			displayMessage(0x53DD);
			playSound(5, 2);
			playSound(5, 18);
			playActorAnimation(810);
			Dialog::show(scene, 0x60BF, 0, 809, 0xd1, 0xd0);
			SET_FLAG(0xDBDA, 1);
		}
		return true;

	case 0x80c3: //show kaleydoscope to the guard
		Dialog::show(scene, 0x6811, 0, 809, 0xd1, 0xd0, 0, 1);
		playSound(5, 3);
		playSound(5, 30);
		playSound(26, 14);
		hideActor();
		playAnimation(849, 0);
		showActor();
		playAnimation(851, 0);
		playAnimation(850, 0);
		reloadLan();
		inventory->add(0x53);
		inventory->remove(0x52);
		enableObject(1);
		SET_FLAG(0xDBE2, 1);
		return true;

		//Shore

	case 0x5348:
		if (CHECK_FLAG(0xdb99, 1)) { //got broken paddle from boat
			displayMessage(0x351f);
		} else {
			SET_FLAG(0xDB99, 1);
			playSound(57, 6);
			playActorAnimation(536);
			Dialog::show(scene, 0x30c3);
			inventory->add(0x8);
		}
		return true;

	case 0x53a1:
		if (CHECK_FLAG(0xdbb2, 1)) { //spoken to man in well
			displayMessage(0x411d);
		} else {
			SET_FLAG(0xDBB2, 1);
			displayMessage(0x408a);
			displayMessage(0x4091);
			displayMessage(0x4098);
			displayMessage(0x40a7);
			displayMessage(0x40b6);
			displayMessage(0x40ce);
			displayMessage(0x40e8);
			displayMessage(0x410f);
		}
		return true;


	case 0x5458: {
		setOns(2, 0);
		playSound(34, 7);
		playActorAnimation(535);
		inventory->add(11);
		disableObject(1);

		byte * scene_15_ons = scene->getOns(15); //patch ons for the scene 15
		scene_15_ons[0] = 0;

		byte f = GET_FLAG(0xDB98) + 1;
		SET_FLAG(0xDB98, f);
		if (f >= 2) {
			//disable object boat for scene 15!!
			disableObject(1, 15);
		}
	}
	return true;

	case 0x54b3: {
		setOns(1, 0);
		setOns(3, 0);
		playSound(33, 6);
		playActorAnimation(534);
		inventory->add(10);
		disableObject(2);
		setOns(1, 10);
		setOns(1, 0, 15);
		byte f = GET_FLAG(0xDB98) + 1;
		SET_FLAG(0xDB98, f);
		if (f >= 2) {
			//disable object boat for scene 15!!
			disableObject(1, 15);
		}
	}
	return true;

	case 0x5502:
		setOns(0, 0);
		loadScene(15, 115, 180, 1);
		playActorAnimation(568);
		playMusic(6);
		return true;

	case 0x5561://Enter lakeside house
		processCallback(0x557e);
		loadScene(19, 223, 199, 1);
		return true;

	case 0x557e:
		//scaled moveTo
		if (scene->getPosition().y <= 149)
			moveTo(94, 115, 4);
		else
			moveTo(51, 149, 4);
		return true;

	case 0x563b:
		playSound(5, 10);
		setOns(1, 0);
		playActorAnimation(561);
		inventory->add(26);
		disableObject(6);
		return true;

	case 0x56f6:
		playSound(32, 7);
		setOns(1, 0);
		playActorAnimation(626);
		disableObject(12);
		inventory->add(45);
		displayMessage(0x3b04);
		return true;

	case 0x5756://Open car door
		playSound(11, 4);
		playActorAnimation(514);
		setOns(4, 8);
		setOns(2, 5);
		enableObject(14);
		enableObject(15);
		enableObject(16);
		disableObject(1);
		return true;

	case 0x5805://Enter basketball house
		playSound(70, 6);
		playActorAnimation(513);
		loadScene(22, 51, 180, 2);
		return true;

	case 0x5832://Ring doorbell
		playActorAnimation(509);
		displayMessage(0x5dce);
		return true;

	case 0x58a2:
		Dialog::pop(scene, 0xdaba, 0, 502, 0xd1, 0xe5, 0, 1);
		scene->getObject(13)->setName((const char *)res->dseg.ptr(0x92e5));
		return true;

	case 0x58b7://Get comb from car
		disableObject(14);
		setOns(4, 0);
		playSound(5, 7);
		playActorAnimation(521);
		setOns(4, 0);
		inventory->add(0x6);
		return true;

	case 0x58df://Pull trunk lever in car
		SET_FLAG(0xDB94, 1);
		playSound(6, 1);
		setOns(3, 6);
		playActorAnimation(515);
		return true;

	case 0x593e://Enter annes house
		playSound(89, 4);
		playActorAnimation(980);
		loadScene(23, 76, 199, 1);
		if (CHECK_FLAG(0xDBEE, 1))
			playMusic(7);
		return true;

	case 0x5994:
		processCallback(0x599b);
		processCallback(0x5a21);
		return true;

	case 0x599b:
		return true;

	case 0x5a21:
		loadScene(24, 230, 170, 1);
		playSound(52, 3);
		playSound(52, 7);
		playSound(52, 11);
		playSound(52, 14);
		playSound(52, 18);
		playSound(52, 21);
		playSound(52, 25);
		playActorAnimation(601);
		moveTo(230, 179, 3);
		if (!CHECK_FLAG(0xDBA4, 1))
			displayMessage(0x37ea); //it's kinda dark here
		return true;

	case 0x5a8b:
		if (!CHECK_FLAG(0xDBAD, 1)) {
			playSound(43, 4); //grrrrrr
			playSound(42, 15); 
			playSound(42, 17); 
			playSound(42, 19); 
			playAnimation(656, 0);
			displayMessage(0x3c16);
		} else if (!CHECK_FLAG(0xDBA3, 1)) {//Dog has bone
			playSound(28, 3);
			playActorAnimation(596);
			setOns(1, 30);
			SET_FLAG(0xDBA3, 1);
			enableObject(8);
		} else {
			setOns(1, 0);
			playSound(4, 4);
			playActorAnimation(597);
			SET_FLAG(0xDBA3, 0);
			disableObject(8);
			displayMessage(0x37b8);
			setOns(1, 32, 24);
			enableObject(4, 24);
		}
		return true;

	case 0x5b3a://Click on dog
		Dialog::pop(scene, 0xDB14);
		return true;

	case 0x5b59: //picking up the rope
		Dialog::show(scene, 0x2cbd);
		Dialog::show(scene, 0x2dc2);
		moveRel(0, -12, 0);
		playSound(34, 5);
		playActorAnimation(607);
		setOns(0, 0);
		playActorAnimation(608);
		playActorAnimation(609);
		playActorAnimation(610);
		playSound(5, 25);
		playActorAnimation(611);
		moveTo(16, scene->getPosition().y, 4, true);
		inventory->add(38);
		disableObject(12);
		return true;

	case 0x5be1://Talk to grandpa
		Dialog::pop(scene, 0xDAC4, 0, 522, 0xd1, 0xd8, 0, 1);
		return true;

	case 0x5bee:
		playSound(89, 5);
		playSound(67, 11);
		playActorAnimation(982);
		displayMessage(0x5955);
		return true;

	case 0x5c0d: //grandpa - drawers
		if (CHECK_FLAG(0xDBA7, 1)) {
			displayMessage(0x3bac);
		} else {
			if (!CHECK_FLAG(0xDB92, 1))
				Dialog::show(scene, 0x15a0, 0, 522, 0xd1, 0xd8, 0, 1); //can I search your drawers?

			playSound(66, 5);
			playSound(67, 20);
			playSound(5, 23);
			playActorAnimation(631);
			inventory->add(47);
			SET_FLAG(0xDBA7, 1);
		}
		return true;

	case 0x5c84:
		if (CHECK_FLAG(0xDB92, 1)) {
			inventory->add(2);
			disableObject(7);
			playSound(32, 7);
			setOns(0, 0);
			playActorAnimation(520);
		} else {
			Dialog::pop(scene, 0xDACE, 0, 522, 0xd1, 0xd8, 0, 1);
		}
		return true;

	case 0x5cf0://Exit basketball house
		playSound(88, 5);
		playActorAnimation(981);
		loadScene(20, 161, 165);
		return true;

	case 0x5d24: //getting the fan
		if (CHECK_FLAG(0xDB92, 1)) {
			setLan(2, 0);
			playSound(32, 7);
			playActorAnimation(508);
			disableObject(13);
			inventory->add(7);
		} else {
			Dialog::pop(scene, 0xDAD4, 0, 522, 0xd1, 0xd8, 0, 1);
		}
		return true;

	case 0x5e4d: //right click on ann
		if (!CHECK_FLAG(0xDB97, 0)) {
			displayMessage(0x3d59);
		} else {
			moveTo(245, 198, 1);
			Dialog::show(scene, 0x21d7, 0, 524, 0xd1, 0xe5, 0, 2);
			//waitLanAnimationFrame(2, 1); //too long, about 200 frames! seems to be present in original game (sic)
			SET_FLAG(0xDB97, 1);
			for (byte i = 10; i <= 20; i += 2)
				playSound(13, i);
			playAnimation(528, 1);
			playMusic(7);
			SET_FLAG(0xDBEE, 1);
			for (byte i = 3; i <= 17; i += 2)
				playSound(56, i);
			playActorAnimation(525);
			for (byte i = 1; i <= 13; i += 2)
				playSound(56, i);
			playSound(40, 15);
			playSound(40, 18);
			playSound(40, 22);
			playActorAnimation(526);
			playSound(54, 1);
			playSound(55, 5);
			playActorAnimation(527);
			Dialog::show(scene, 0x2219, 0, 524, 0xd1, 0xe5, 0, 2);
			scene->getObject(2)->setName((const char *)res->dseg.ptr(0x9820));
		}
		return true;

	case 0x5f73: //exiting ann's house
		if (CHECK_FLAG(0xDBEE, 1))
			playMusic(6);
		loadScene(21, 99, 180, 3);
		return true;

	case 0x5fba:
		if (CHECK_FLAG(0xDBB1, 1)) {
			displayMessage(0x4380);
		} else {
			Dialog::pop(scene, 0xDAFC, 0, 523, 0xd1, 0xe5, 0, 1);
		}
		return true;

	case 0x607f:
		return processCallback(0x60b5);

	case 0x6083:
		if (CHECK_FLAG(0xDBA4, 1)) {
			setOns(0, 0);
			playSound(56, 10);
			playActorAnimation(599);
			inventory->add(37);
			disableObject(2);
		} else
			processCallback(0x60b5);
		return true;

	case 0x60b5:
		if (CHECK_FLAG(0xDBAE, 1)) {
			processCallback(0x60d9);
			Dialog::show(scene, 0x2fdd);
		} else {
			Dialog::show(scene, 0x2e41);
			processCallback(0x60d9);
			Dialog::show(scene, 0x2e6d);
		}
		return true;

	case 0x60d9: {
		Object *obj = scene->getObject(3);
		moveTo(obj);
		processCallback(0x612b);
		moveTo(48, 190, 3);
	}
	return true;

	case 0x612b:
		playSound(52, 10);
		playSound(52, 14);
		playSound(52, 18);
		playSound(52, 21);
		playSound(52, 25);
		playSound(52, 28);
		playSound(52, 32);
		playActorAnimation(600);
		loadScene(21, 297, 178, 3);
		return true;

	case 0x6176:
		if (CHECK_FLAG(0xDBA4, 1)) {
			displayMessage(0x3801);
			return true;
		}
		playSound(71, 6);
		playActorAnimation(598);
		loadScene(24, scene->getPosition());
		setOns(2, 0);
		setLan(1, 0);
		playAnimation(660, 1);
		disableObject(1);
		SET_FLAG(0xDBA4, 1);
		loadScene(24, scene->getPosition());

		return true;

	case 0x61e9:
		if (CHECK_FLAG(0xDBA4, 1)) {
			Dialog::pop(scene, 0xdb1e);
		} else
			processCallback(0x61fe);

		return true;

	case 0x6229: //shelves in cellar
		if (CHECK_FLAG(0xDBA4, 1)) {
			Common::Point p = scene->getPosition();
			byte v = GET_FLAG(0xDBB4);
			switch (v) {
			case 0:
				displayMessage(0x4532);
				moveRel(-34, 0, 1);
				displayMessage(0x4555);
				moveRel(20, 0, 1);
				displayMessage(0x4568);
				moveRel(20, 0, 1);
				displayMessage(0x457b);
				moveRel(20, 0, 1);
				displayMessage(0x458e);
				moveTo(p, 3);
				SET_FLAG(0xDBB4, 1);
				break;
			case 1:
				displayMessage(0x45b8);
				displayMessage(0x45da);
				SET_FLAG(0xDBB4, 2);
				break;
			default:
				displayMessage(0x4603);
			}
		} else
			processCallback(0x61fe);

		return true;

	case 0x6480: //dive mask
		if (CHECK_FLAG(0xDB96, 1)) {
			playSound(56, 7);
			playSound(5, 15);
			playActorAnimation(613);
			setOns(3, 36);
			inventory->add(39);
			disableObject(5);
			displayMessage(0x387c);
		} else
			displayMessage(0x3eb2);
		return true;

	case 0x64c4: //flippers
		if (CHECK_FLAG(0xDB96, 1)) {
			setOns(2, 35);
			playSound(63, 8);
			playSound(24, 10);
			playActorAnimation(612);
			inventory->add(40);
			disableObject(6);
		} else
			displayMessage(0x3eb2);
		return true;

	case 0x7907://Describe car lever
		if (CHECK_FLAG(0xdb94, 1)) {//Already pulled lever?
			displayMessage(0x3e4f);
			return true;
		} else
			return false;

	case 0x62d0://Get bone from under rock
		displayAsyncMessage(0x463c, 30938, 16, 24);
		playSound(26, 6);
		playSound(26, 10);
		playSound(24, 13);
		playSound(46, 37);
		setOns(0, 0);
		playActorAnimation(594);
		setOns(0, 29);
		disableObject(1);
		inventory->add(36);
		playSound(5, 2);
		playActorAnimation(595);
		displayMessage(0x3790);
		return true;

	case 0x6351:
		if (CHECK_FLAG(0xdaca, 1)) { //cave bush is cut down
			playMusic(8);
			loadScene(26, 319, 169, 4);
		} else
			displayMessage(0x3bd2);
		return true;

	case 0x63ea:
		playSound(5, 10);
		setOns(0, 0);
		playActorAnimation(640);
		inventory->add(50);
		disableObject(6);
		return true;

	case 0x6411://Kick hen
		if (CHECK_FLAG(0xdb93, 1)) { //already kicked hen
			displayMessage(0x3e08);
			return true;
		} else {
			SET_FLAG(0xdb93, 1);
			displayMessage(0x3dc6);
			waitLanAnimationFrame(1, 87);
			playSound(30, 26);
			playSound(29, 49);
			playActorAnimation(500, true);
			playAnimation(501, 0, true);
			waitAnimation();
			setOns(0, 1);
			enableObject(14);
			displayMessage(0x3df4);
			return true;
		}

	case 0x6592: //Rake
		setOns(1, 0);
		playSound(18, 10);
		playActorAnimation(553);
		inventory->add(0x15);
		displayMessage(0x3605);
		disableObject(11);
		return true;

	case 0x66b5:
		playSound(89, 5);
		playActorAnimation(969);
		loadScene(33, 319, 181, 4);
		return true;

	case 0x6519://Sickle
		setOns(4, 0);
		playSound(5, 11);
		playActorAnimation(625);
		inventory->add(0x2c);
		disableObject(8);
		return true;

	case 0x655b://Get needle from haystack
		if (CHECK_FLAG(0xdabb, 1)) { //already have needle
			displayMessage(0x356a);
			return true;
		} else {
			SET_FLAG(0xdabb, 1);
			playSound(49, 3);
			playActorAnimation(548);
			inventory->add(0x11);
			displayMessage(0x35b2);
			return true;
		}

	case 0x663c://Feather
		setOns(0, 0);
		playSound(5, 9);
		playActorAnimation(511);
		inventory->add(1);
		disableObject(15);
		return true;

	case 0x667c:
		playSound(70, 4);
		playActorAnimation(972);
		loadScene(29, 160, 199, 1);
		return true;

	case 0x66a9:
		displayMessage(0x4a7e);
		disableObject(4);
		return true;

	case 0x66e2:
		playSound(88, 4);
		playActorAnimation(970);
		loadScene(35, 160, 199, 1);
		return true;

	case 0x70bb:
		Dialog::pop(scene, 0xdb24, 0, 709, 0xd1, 0xef, 0, 1);
		return true;

	case 0x71ae:
		if (CHECK_FLAG(0xDBCD, 1)) {
			if (CHECK_FLAG(0xDBCE, 1)) {
				displayMessage(0x4f9b);
			} else {
				displayMessage(0x4fb1);
				playSound(32, 6);
				playActorAnimation(717);
				inventory->add(66);
				SET_FLAG(0xDBCE, 1);
			}
		} else
			Dialog::show(scene, 0x3c9d);
		return true;

	case 0x70c8:
		if (!processCallback(0x70e0))
			return true;
		moveTo(81, 160, 4);
		displayMessage(0x5cac);
		return true;

	case 0x70e0:
		if (!CHECK_FLAG(0xDBCC, 1)) {
			displayMessage(0x4ece);
			return false;
		}
		return true;

	case 0x70ef:
		if (!processCallback(0x70e0))
			return true;
		displayMessage(0x5046);
		return true;

	case 0x70f9:
		if (inventory->has(68)) {
			inventory->remove(68);
			loadScene(29, 40, 176, 2);
			displayMessage(0x500a);
		} else
			loadScene(29, 40, 176, 2);
		return true;

	case 0x712c:
		if (!processCallback(0x70e0))
			return true;

		if (CHECK_FLAG(0xDBCF, 1)) {
			playSound(89, 4);
			playActorAnimation(719);
			setOns(4, 67);
			++ *res->dseg.ptr(READ_LE_UINT16(res->dseg.ptr(0x6746 + (scene->getId() - 1) * 2)));
			disableObject(5);
			enableObject(12);
		} else {
			playSound(89, 4);
			playSound(89, 4);
			playSound(87, 45);
			displayAsyncMessage(0x4fcb, 34672, 11, 35, 0xe5);
			playActorAnimation(718);
			displayMessage(0x4fe2);
			SET_FLAG(0xDBCF, 1);
		}
		return true;

	case 0x71eb:
		setOns(2, 0);
		playSound(32, 7);
		playActorAnimation(710);
		inventory->add(62);
		disableObject(7);
		enableObject(8);
		return true;

	case 0x7244:
		if (!processCallback(0x70e0))
			return true;
		displayMessage(0x5c60);
		return true;

	case 0x7255:
		if (CHECK_FLAG(0xDBD0, 1)) {
			setOns(4, 69);
			playSound(32, 5);
			playActorAnimation(725);
			disableObject(12);
			inventory->add(69);
		} else {
			playActorAnimation(721);
			displayMessage(0x505e);
		}
		return true;

	case 0x721c:
		setOns(3, 0);
		playSound(32, 7);
		playActorAnimation(715);
		inventory->add(63);
		disableObject(9);
		return true;

	case 0x7336:
		setOns(1, 0);
		playSound(5, 42);
		displayAsyncMessage(0x4d02, 32642, 20, 38);
		playActorAnimation(697);
		inventory->add(56);
		disableObject(1);
		return true;

	case 0x73a3:
		if (CHECK_FLAG(0xdbc5, 1)) {
			SET_FLAG(0xdbc5, 0);

			//call 73e6
			playSound(71, 3);
			playActorAnimation(700);
			playAnimation(0, 0, true);

		} else {
			SET_FLAG(0xdbc5, 1);

			//call 73e6
			playSound(71, 3);
			playActorAnimation(700);

			playAnimation(CHECK_FLAG(0xDBC6, 0) ? 701 : 702, 0, true, true, true);

			if (CHECK_FLAG(0xDBC6, 1)) {
				displayMessage(0x4da6);
			}
		}
		return true;

	case 0x7381:
		playSound(5, 12);
		playActorAnimation(704);
		disableObject(2);
		inventory->add(58);
		return true;

	case 0x7408:
		if (CHECK_FLAG(0xDBC4, 1)) {
			displayMessage(0x4d2a);
		} else {
			setOns(0, 0);
			playSound(26, 17);
			playSound(26, 23);
			playSound(26, 30);
			playSound(26, 37);
			playSound(26, 43);
			playSound(52, 34);
			playActorAnimation(698);
			setOns(0, 52);
			setOns(2, 61);
			Dialog::show(scene, 0x38b6);
			enableObject(11);
			SET_FLAG(0xDBC4, 1);
		}
		return true;

	case 0x7476:
		if (CHECK_FLAG(0xDBC9, 1)) {
			displayMessage(0x4dbb);
		} else {
			SET_FLAG(0xDBC9, 1);
			Dialog::show(scene, 0x3aca);
			playSound(61, 5);
			playSound(5, 14);
			playActorAnimation(705);
			displayMessage(0x4dd3);
			inventory->add(59);
		}
		return true;

	case 0x74d1:
		setOns(2, 0);
		playSound(5, 12);
		playActorAnimation(699);
		inventory->add(57);
		disableObject(11);
		return true;

	case 0x7513: //fatso + doctor: pre-final
		if (CHECK_FLAG(0xDBD7, 1)) {
			if (CHECK_FLAG(0xDBD8, 1)) {
				playSound(88, 4);
				playActorAnimation(979);
				loadScene(37, 51, 183);

				Dialog::show(scene, 0x54ea, 768, 769, 0xd9, 0xe5, 1, 2);
				playAnimation(770, 0, true, true, true);
				playAnimation(771, 1, true, true, true);
				Dialog::show(scene, 0x5523, 0, 0, 0xd1, 0xd1, 0, 0);
				playAnimation(770, 0, true, true, true);
				playAnimation(771, 1, true, true, true);
				playSound(5, 3);
				playSound(56, 12);
				playSound(23, 20);
				playSound(75, 25);
				playActorAnimation(772);

				playActorAnimation(773, true);
				playAnimation(774, 0, true);
				waitAnimation();
				setOns(0, 74);
				hideActor();
				Dialog::show(scene, 0x5556, 775, 775, 0xd0, 0xd0, 1, 1);
				playAnimation(771, 1, true, true, true);
				playAnimation(776, 0);
				
				Dialog::show(scene, 0x55f7, 777, 778, 0xd0, 0xe5, 1, 2); //i have to kill you anyway
				
				playAnimation(779, 0, true, true, true);
				playAnimation(780, 1, true, true, true);
				
				for (byte i = 1; i <= 6; ++i)
					playSound(58, i);
				playSound(58, 10);
				playSound(2, 7);
				playSound(55, 11);
				playSound(54, 15);
				playAnimation(781, 2, true);
				playAnimation(782, 3, true);
				waitAnimation();
				setOns(1, 75);
				setOns(2, 76);
				
				for (byte i = 1; i <= 6; ++i)
					playSound(58, i);
				playSound(58, 9);
				playSound(2, 7);
				playSound(2, 15);
				playSound(55, 10);

				playAnimation(783, 2, true);
				playAnimation(784, 3, true);
				waitAnimation();
				setOns(1, 77);
				setOns(2, 78);

				playAnimation(785, 2, true);
				playAnimation(786, 3, true);
				waitAnimation();

				moveTo(112, 183, 2, true);

				setOns(3, 79);
				setOns(0, 0);
				
				showActor();
				playAnimation(0, 0);
				playAnimation(787, 2, true);
				playAnimation(788, 3, true);
				waitAnimation();

				playAnimation(0, 1);
				
				playSound(32, 2);
				playSound(24, 7);

				playAnimation(790, 3, true);
				playAnimation(789, 0, true);
				waitAnimation();

				setOns(0, 80);

				playAnimation(792, 3, true, true, true);
				Dialog::show(scene, 0x5665, 0, 791, 0xd1, 0xd0, 0, 4);
				playAnimation(792, 3, true, true, true);

				moveTo(40, 171, 4);
				
				setOns(3, 81, 35);
				enableObject(12, 35);
				playAnimation(0, 3);

				loadScene(31, 298, 177, 4);
				SET_FLAG(0xDBD9, 1);
			} else {
				displayMessage(0x52fe);
			}
		} else
			displayMessage(0x52cb);
		return true;

	case 0x783d:
		Dialog::pop(scene, 0xdb36, 0, 797, 0xd1, 0xd0, 0, 1);
		return true;
		
	case 0x7966:
		if (CHECK_FLAG(0xDBA4, 1))
			return false;
		return processCallback(0x60b5);

	case 0x7ad0:
	case 0x7ad7:
		return !processCallback(0x70e0);
		
	case 0x7ab9:
		if (CHECK_FLAG(0xDBB6, 1))
			return false;
		Dialog::show(scene, 0x37d0, 0, 0, 0xd1, 0xd1, 0, 0);
		SET_FLAG(0xDBB6, 1);
		return true;

	case 0x7ade:
		if (CHECK_FLAG(0xdbcd, 1)) {
			displayMessage(0x4f69);
			return true;
		} else
			return false;

	case 0x7f23://Use grenade on captains drawer
		if (CHECK_FLAG(0xDBDF, 3)) {
			playSound(5, 3);
			playSound(58, 11);
			playSound(46, 56);
			playSound(46, 85);
			playSound(46, 117);
			playActorAnimation(870);
			playSound(54, 15);
			playActorAnimation(871);
			SET_FLAG(0xDBE6, 1);
			setOns(1, 0x66);
			moveTo(224, 194, 0, true);
			displayCutsceneMessage(0x57df, 30423);
			inventory->remove(0x59);
		} else {
			displayMessage(0x5de2);
		}
		return true;

	case 0x505c: {
		//suspicious stuff
		Common::Point p = scene->getPosition();
		if (p.x != 203 && p.y != 171)
			moveTo(203, 169, 2);
		else
			moveTo(203, 169, 1);
	}
	return true;

	case 0x509a:
		processCallback(0x505c);
		setOns(1, 0);
		playSound(5, 10);
		playActorAnimation(543);
		inventory->add(15);
		disableObject(9);
		return true;

	case 0x7802:
		if (CHECK_FLAG(0xDBD7, 1)) {
			if (CHECK_FLAG(0xDBD8, 1))
				displayMessage(0x52f6);
			else {
				playSound(71, 4);
				playActorAnimation(796);
				setLan(1, 0);
				SET_FLAG(0xDBD8, 1);
			}
		} else
			displayMessage(0x52cb);
		return true;

	case 0x78e0:
		processCallback(0x50c5);
		return false;

	case 0x78e7:
		processCallback(0x557e);
		return false;

	case 0x78ee:
		processCallback(0x557e);
		return false;

	case 0x78f5:
		if (CHECK_FLAG(0xDB95, 1)) {
			displayMessage(0x3575);
			return true;
		} else
			return false;

	case 0x7919:
		if (!CHECK_FLAG(0xDBA5, 1))
			return false;
		displayMessage(0x3E98);
		return true;

	case 0x7950:
		if (!CHECK_FLAG(0xDBB1, 1))
			return false;

		displayMessage(0x3DAF);
		return true;

	case 0x7975:
		if (CHECK_FLAG(0xDBA4, 1))
			return false;
		displayMessage(0x3832);
		return true;

	case 0x7987:
	case 0x7996:
	case 0x79a5:
	case 0x79b4:
		if (CHECK_FLAG(0xDBA4, 1))
			return false;
		return processCallback(0x61fe);

	case 0x7af0:
		if (!processCallback(0x70e0))
			return true;
		return false;

	case 0x8117:
		Dialog::show(scene, 0x0a41, 0, 529, 0xd1, 0xd9, 0, 1);
		playSound(5, 2);
		playSound(5, 44);
		playAnimation(642, 0, true);
		playActorAnimation(641, true);
		waitAnimation();
		Dialog::show(scene, 0x0aff, 0, 529, 0xd1, 0xd9, 0, 1);
		Dialog::show(scene, 0x0ba0, 0, 529, 0xd1, 0xd9, 0, 1);
		moveRel(0, 1, 0);
		Dialog::show(scene, 0x0c10, 0, 529, 0xd1, 0xd9, 0, 1);
		inventory->remove(50);
		processCallback(0x9d45);
		return true;

	case 0x8174:
		setOns(0, 0);
		playSound(5, 2);
		playSound(5, 5);
		playSound(5, 9);
		playSound(14, 19);
		playSound(5, 50);
		playActorAnimation(542);
		setOns(1, 15);
		disableObject(3);
		enableObject(9);
		return true;

	case 0x81c2:
		playSound(56, 11);
		playSound(36, 13);
		playSound(48, 22);
		playSound(56, 57);
		playSound(36, 59);
		playSound(48, 68);
		playSound(54, 120);
		playSound(56, 141);
		playSound(56, 144);
		playSound(56, 147);
		playAnimation(589, 1, true);
		playActorAnimation(588, true);
		waitAnimation();
		displayMessage(0x367f);
		inventory->remove(34);
		SET_FLAG(0xDBA1, 1);
		return true;

	case 0x823d: //grappling hook on the wall
		playSound(5, 3);
		for (byte i = 16; i <= 28; i += 2)
			playSound(65, i);
		playSound(47, 33);
		playActorAnimation(620);
		for (byte i = 3; i <= 18; i += 3)
			playSound(56, i);

		displayAsyncMessage(0x3ace, 3878, 20, 37, 0xd9);
		playActorAnimation(621, true);
		playAnimation(623, 1, true);
		waitAnimation();

		displayAsyncMessage(0x3ae6, 3870, 1, 9, 0xd9);
		playSound(35, 1);
		playActorAnimation(622, true);
		playAnimation(624, 0, true);
		waitAnimation();
		
		displayMessage(0x3afd);

		inventory->remove(43);
		processCallback(0x9d45);
		return true;


	case 0x8312: //hedgehog + plastic apple
		Dialog::show(scene, 0x3000);
		setLan(1, 0);
		playSound(5, 24);
		playSound(26, 32);
		playSound(5, 42);
		playSound(15, 77);
		playSound(15, 79);
		playSound(15, 82);
		playSound(22, 91);
		playSound(22, 102);
		playSound(26, 114);
		playSound(24, 124);
		waitLanAnimationFrame(1, 0x1a);
		playActorAnimation(562, true);
		playAnimation(563, 1, true);
		waitAnimation();

		disableObject(6);
		displayMessage(0x363f);
		inventory->remove(27);
		inventory->add(28);
		return true;

	case 0x839f:
		inventory->remove(32);
		playSound(37, 14);
		playSound(16, 17);
		playActorAnimation(564, true);
		playAnimation(565, 2, true);
		waitAnimation();
		setOns(0, 24);
		playSound(39, 5);
		playActorAnimation(582);
		moveTo(63, 195, 1);
		playAnimation(571, 1);
		playAnimation(572, 1);
		playAnimation(573, 1);
		for (byte i = 1; i <= 7; i += 2)
			playSound(40, i);
		playAnimation(574, 1);
		setLan(1, 0);
		playAnimation(575, 1);
		playAnimation(576, 1);
		playAnimation(577, 1);
		playAnimation(578, 1);
		playAnimation(579, 1);
		playAnimation(580, 1);
		playSound(55, 18);
		playAnimation(581, 1);
		disableObject(2);
		SET_FLAG(0xDB9F, 1);
		return true;

	case 0x84c7:
		playSound(20, 9);
		playActorAnimation(530);
		loadScene(16, 236, 95, 1);
		playActorAnimation(531);
		playSound(36, 4);
		playActorAnimation(532);
		playActorAnimation(533);
		setOns(0, 9);
		moveTo(236, 95, 1, true);
		playMusic(9);
		return true;

	case 0x8538://Sharpen sickle on well
		moveTo(236, 190, 0);
		setOns(2, 0);
		//TODO: Remove handle sprite
		playSound(5, 4);
		playSound(14, 14);
		playSound(14, 33);
		playSound(5, 43);
		playActorAnimation(643);
		setOns(2, 43);
		moveTo(236, 179, 3);
		inventory->remove(0x2c);
		inventory->add(0x2e);
		return true;

	case 0x85eb:
		if (CHECK_FLAG(0xDBB0, 1)) {
			enableObject(6);
			playSound(25, 10);
			playSound(25, 14);
			playSound(25, 18);
			playActorAnimation(559);
			setOns(1, 23);
			SET_FLAG(0xDBB0, 2);
		} else
			displayMessage(0x3d86);

		return true;

	case 0x863d:
		playSound(12, 4);
		playSound(50, 20);
		playSound(50, 29);
		playActorAnimation(554);
		inventory->remove(19);
		inventory->add(22);
		return true;

	case 0x8665:
		playSound(5, 3);
		for (byte i = 12; i <= 24; i += 2)
			playSound(56, i);
		playActorAnimation(567);
		inventory->remove(12);
		inventory->add(33);
		return true;

	case 0x86a9: //correcting height of the pole with spanner
		if (CHECK_FLAG(0xDB92, 1)) {
			displayMessage(0x3d40);
		} else {
			SET_FLAG(0xDB92, 1);
			Dialog::show(scene, 0x0fcd, 0, 502, 0xd0, 0xe5, 0, 1);
			waitLanAnimationFrame(1, 7);
			playSound(5, 16);
			playSound(1, 25);
			playSound(1, 29);
			playSound(1, 34);
			playAnimation(506, 0, true);
			playActorAnimation(504, true);
			waitAnimation();
			
			setOns(0, 0);
			playSound(24, 2);
			playSound(22, 24);
			playSound(1, 28);
			playSound(1, 32);
			playSound(1, 37);
			playSound(5, 43);
			playSound(61, 70);
			playSound(61, 91);
			displayAsyncMessage(0x3cfb, 28877, 6, 17);
			playActorAnimation(505, true);
			playAnimation(507, 0, true);
			waitAnimation();
			
			setOns(0, 4);
			{
				Object *obj = scene->getObject(3);
				obj->rect.top += 20;
				obj->rect.bottom += 20;
				obj->rect.save();
			}
			playSound(10, 3);
			playAnimation(503, 0);
			setLan(1, 0, 22);
			disableObject(1, 22);
			disableObject(13, 20);
			setLan(1, 0);
			disableObject(1);
			disableObject(2);
			disableObject(14);
			disableObject(15);
			disableObject(16);
			moveTo(162, 164, 2);
			displayMessage(0x3d01, 0xe5, 24390);
			displayMessage(0x3d20, 0xd8, 24410);
			moveTo(162, 191, 2);
			setOns(1, 0);
			setOns(2, 0);
			setOns(3, 0);
			setOns(4, 0);

			{
				Walkbox * w = scene->getWalkbox(0);
				w->rect.clear();
				w->save();
			}

			playSound(62, 1);
			playSound(9, 8);
			setLan(1, 0);
			playAnimation(512, 0);
			
			warning("FIXME: fadeout not implemented");
			displayMessage(0x3d3a);
			{
				Object *obj = scene->getObject(7);
				obj->actor_rect.left = obj->actor_rect.right = 228;
				obj->actor_rect.top = obj->actor_rect.bottom = 171;
				obj->actor_rect.save();
			}
			{
				Object *obj = scene->getObject(8);
				obj->actor_rect.left = obj->actor_rect.right = 290;
				obj->actor_rect.top = obj->actor_rect.bottom = 171;
				obj->actor_rect.save();
			}
		}
		return true;

	case 0x88c9: //give flower to old lady
		if (CHECK_FLAG(0xDB9A, 1))
			return processCallback(0x890b);

		inventory->remove(10);
		SET_FLAG(0xDB9A, 1);
		processCallback(0x88DE);
		return true;

	case 0x88de:
		playSound(5, 2);
		Dialog::show(scene, 0x1B5F, 0, 523, 0xd1, 0xe5, 0, 1);
		playActorAnimation(537, true);
		playAnimation(538, 0, true);
		waitAnimation();
		Dialog::show(scene, 0x1BE0, 0, 523, 0xd1, 0xe5, 0, 1);
		return true;

	case 0x890b:
		Dialog::pop(scene, 0xDAF0);
		return true;

	case 0x8918://give flower to old lady
		if (CHECK_FLAG(0xDB9A, 1))
			return processCallback(0x890B);

		inventory->remove(11);
		SET_FLAG(0xDB9A, 1);
		processCallback(0x88DE);
		return true;

	case 0x892d:
		if (CHECK_FLAG(0xDB9B, 1))
			return processCallback(0x89aa);

		processCallback(0x8942);
		inventory->remove(10);
		SET_FLAG(0xDB9B, 1);
		return true;

	case 0x8942:
		Dialog::show(scene, 0x2293, 0, 524, 0xd1, 0xe5, 0, 2);
		playSound(5, 10);
		playActorAnimation(540, true);
		playAnimation(539, 1, true);
		waitAnimation();
		Dialog::show(scene, 0x24b1, 0, 524, 0xd1, 0xe5, 0, 2);
		Dialog::show(scene, 0x24d7, 0, 524, 0xd1, 0xe5, 0, 2);
		Dialog::show(scene, 0x2514, 0, 524, 0xd1, 0xe5, 0, 2);
		moveTo(scene->getPosition().x, scene->getPosition().y + 1, 0);
		Dialog::show(scene, 0x2570, 0, 524, 0xd1, 0xe5, 0, 2);
		return true;

	case 0x89aa:
		Dialog::pop(scene, 0xdb02);
		return true;

	case 0x89b7:
		if (CHECK_FLAG(0xDB9B, 1))
			return processCallback(0x89aa);

		processCallback(0x8942);
		inventory->remove(11);
		SET_FLAG(0xDB9B, 1);
		return true;

	case 0x89cc:
		inventory->remove(23);
		playSound(5, 6);
		Dialog::show(scene, 0x2634, 0, 524, 0xd1, 0xe5, 0, 2);
		playActorAnimation(555, true);
		playAnimation(556, 1, true);
		waitAnimation();
		playActorAnimation(557, true);
		playAnimation(558, 1, true);
		waitAnimation();
		Dialog::show(scene, 0x2971, 0, 524, 0xd1, 0xe5, 0, 2);
		inventory->add(24);
		return true;

	case 0x8a22:
		playSound(45, 16);
		playActorAnimation(560);
		inventory->remove(26);
		inventory->add(27);
		Dialog::show(scene, 0x1ecd, 0, 523, 0xd1, 0xe5, 0, 1);
		Dialog::show(scene, 0x1f09, 0, 523, 0xd1, 0xe5, 0, 1);
		SET_FLAG(0xDBB1, 1);
		return true;

	case 0x8a6f: //banknote + ann
		if (CHECK_FLAG(0xDBB5, 1)) {
			Dialog::show(scene, 0x2992, 0, 524, 0xd1, 0xe5, 0, 2);
			playSound(5, 3);
			playSound(5, 20);
			playActorAnimation(671, true);
			playAnimation(670, 1, true);
			waitAnimation();
			playAnimation(672, 1);
			Dialog::show(scene, 0x2a00, 0, 672, 0xd1, 0xe5, 0, 2);
			playAnimation(672, 1);

			playSound(83, 12);
			displayAsyncMessage(0x4a5b, 36684, 23, 38, 0xe5);
			playActorAnimation(673);
			loadScene(11, scene->getPosition());
			playSound(24, 31);
			playSound(24, 48);
			playSound(79, 50);
			playActorAnimation(674, true);
			playAnimation(675, 0, true);
			waitAnimation();
			loadScene(28, 0, 167, 2);
			moveTo(66, 167, 2);
			displayMessage(0x4a6f);
			inventory->clear();
			inventory->add(29);
			playMusic(10);
		} else
			displayMessage(0x4a29);
		return true;

	case 0x8b82: //use fan on laundry
		setOns(0, 0);
		playSound(5, 3);
		playSound(5, 6);
		playSound(5, 10);
		playSound(92, 20);
		playSound(92, 38);
		playSound(92, 58);
		displayAsyncMessage(0x464a, 36510, 58, 67);
		playActorAnimation(602);
		playSound(5, 3);
		playActorAnimation(603);
		setOns(0, 27);
		SET_FLAG(0xDBA5, 1);
		return true;

	case 0x8bfc://Give bone to dog
		displayMessage(0x3c31);
		playSound(5, 3);
		playSound(26, 13);
		playActorAnimation(657, true);
		playAnimation(658, 0, true);
		waitAnimation();

		playAnimation(659, 0);

		inventory->remove(36);
		SET_FLAG(0xDBAD, 1);
		{
			Object * o = scene->getObject(7);
			o->actor_rect.left = o->actor_rect.right = 297;
			o->actor_rect.top = o->actor_rect.bottom = 181;
			o->actor_orientation = 1;
			o->save();
		}
		{
			Object * o = scene->getObject(9);
			o->actor_rect.left = o->actor_rect.right = 297;
			o->actor_rect.top = o->actor_rect.bottom = 181;
			o->actor_orientation = 1;
			o->save();
		}
		{
			Walkbox * w = scene->getWalkbox(0);
			w->rect.right = 266;
			w->rect.bottom = 193;
			w->save();
		}
		displayMessage(0x3c3d);
		return true;

	case 0x8c6e://Use car jack on rock
		playSound(5, 3);
		playSound(26, 13);
		playSound(24, 22);
		playActorAnimation(592);
		playSound(1, 5);
		playSound(1, 9);
		playSound(1, 13);
		setOns(0, 0);
		playActorAnimation(593);
		setOns(0, 28);
		enableObject(1);
		inventory->remove(35);
		return true;

	case 0x8cc8://Cut bush with sickle
		playSound(5, 3);
		playActorAnimation(644);
		setOns(1, 45);
		playSound(56, 2);
		playSound(26, 4);
		playActorAnimation(645);
		playSound(56, 1);
		playSound(56, 6);
		playSound(26, 3);
		playSound(26, 8);
		playActorAnimation(646);
		playSound(5, 21);
		playActorAnimation(647);
		SET_FLAG(0xdaca, 1);
		inventory->remove(0x2e);
		disableObject(2);
		scene->getObject(3)->actor_rect.right = 156;
		scene->getObject(3)->save();
		return true;

	case 0x8d79: //mouse falls back from the hole (cave)
		if (CHECK_FLAG(0, 1)) {
			inventory->add(48);
			playSound(24, 26);
			playActorAnimation(650, true);
			playAnimation(651, 2, true);
			waitAnimation();
		} else {
			playSound(27, 5);
			setOns(1, 0);
			playActorAnimation(652);
			playSound(24, 4);
			playSound(5, 12);
			moveTo(186, 179, 1, true);
			playActorAnimation(653);
			playActorAnimation(654, true);
			playAnimation(655, 2, true);
			waitAnimation();
			displayMessage(0x3bf6);
			inventory->add(49);
			setLan(2, 4, 27);
			enableObject(4, 27);
			SET_FLAG(0xdba9, 0);
		} 
		SET_FLAG(0, 0);
		return true;

	case 0x8d57:
		if (CHECK_FLAG(0, 0)) {
			playSound(5, 2);
			playSound(15, 12);
			playActorAnimation(638);
			inventory->remove(48);
			setTimerCallback(0x8d79, 100);
			SET_FLAG(0, 1);
		} else if (CHECK_FLAG(0, 1)) {
			playSound(5, 2);
			playSound(52, 13);
			playActorAnimation(648);
			setOns(1, 46);
			inventory->remove(49);
			setTimerCallback(0x8d79, 100);
			SET_FLAG(0, 2);
		} else if (CHECK_FLAG(0, 2)) {
			playActorAnimation(649);
			setOns(1, 47);
			for (byte i = 1; i <= 37; i += 4)
				playSound(68, i);
			playAnimation(639, 2);
			setOns(0, 42);
			enableObject(6);
			disableObject(5);
			SET_FLAG(0xDBAB, 1);
			SET_FLAG(0, 0);
			setTimerCallback(0, 0);
		}
		return true;

	case 0x8f1d:
		Dialog::show(scene, 0x2dd6);
		for (uint i = 16; i <= 30; i += 2)
			playSound(56, i);
		playSound(2, 64);
		playSound(3, 74);
		displayAsyncMessage(0x34c7, 25812, 35, 50);
		playActorAnimation(516, true);
		playAnimation(517, 2, true);
		playAnimation(518, 3, true);
		waitAnimation();
		setLan(3, 0);
		setLan(4, 0);
		disableObject(2);
		disableObject(3);
		inventory->remove(2);
		SET_FLAG(0xDB96, 1);
		return true;

	case 0x8fc8:
		displayMessage(0x3b2f);
		waitLanAnimationFrame(2, 4);
		playSound(5, 3);
		playActorAnimation(627, true);
		playAnimation(629, 1, true);
		waitAnimation();
		playSound(41, 10);
		playSound(41, 47);
		playSound(55, 52);
		if (CHECK_FLAG(0xDBA8, 1)) {
			setLan(2, 0);
			playActorAnimation(628, true);
			playAnimation(634, 1, true);
			waitAnimation();
			disableObject(4);
			displayMessage(0x3b6c);
			SET_FLAG(0xDBA9, 1);
		} else {
			playActorAnimation(628, true);
			playAnimation(630, 1, true);
			waitAnimation();
			displayMessage(0x3b59);
		}
		return true;

	case 0x9054: //mouse hole
		if (CHECK_FLAG(0xDBAB, 1)) {
			displayMessage(0x3c0b);
		} else {
			playSound(5, 11);
			playSound(49, 21);
			playActorAnimation(632);
			setOns(5, 40);
			moveTo(239, 139, 0, true);
			playActorAnimation(633);
			SET_FLAG(0xDBA8, 1);
			inventory->remove(47);
			if (!CHECK_FLAG(0xDBAA, 1)) {
				SET_FLAG(0xDBAA, 1);
				displayMessage(0x3b8b);
			}
		}
		return true;

	case 0x933d:
		if (!processCallback(0x70e0))
			return true;

		if (CHECK_FLAG(0xdbcd, 1)) {
			displayMessage(0x4f3d);
			return true;
		}

		setOns(1, 0);
		playSound(5, 3);
		playSound(5, 33);
		playSound(24, 13);
		playSound(24, 19);
		playSound(24, 23);
		playSound(24, 26);
		playSound(24, 29);
		playSound(23, 21);
		playSound(74, 25);
		playActorAnimation(716);
		setOns(1, 66);
		SET_FLAG(0xDBCD, 1);
		return true;

	case 0x93af: //sheet + hot plate
		if (!processCallback(0x70e0))
			return true;
		playSound(5, 3);
		playSound(86, 11);
		playActorAnimation(720);
		inventory->add(68);
		inventory->remove(55);
		return true;

	case 0x93d5: //burning sheet + plate
		setOns(4, 0);
		playSound(87, 7);
		playActorAnimation(722);
		playSound(5, 3);
		playSound(88, 12);
		playSound(87, 24);
		playActorAnimation(723);
		displayMessage(0x502b);
		playSound(89, 4);
		playActorAnimation(724);
		setOns(4, 68);
		displayMessage(0x503e);
		inventory->remove(68);
		SET_FLAG(0xDBD0, 1);
		return true;

	case 0x98fa://Right click to open toolbox
		inventory->remove(3);
		inventory->add(4);
		inventory->add(35);
		inventory->activate(false);
		inventory->resetSelectedObject();
		displayMessage(0x3468);
		return true;

	case 0x9910:
		inventory->remove(4);
		inventory->add(5);
		inventory->activate(false);
		inventory->resetSelectedObject();
		displayMessage(0x3490);
		return true;


		//very last part of the game:
	case 0x671d:
		moveTo(153, 163, 4);
		playActorAnimation(973);
		if (CHECK_FLAG(0xDBC1, 0)) {
			SET_FLAG(0xDBC1, random.getRandomNumber(5) + 1);
		}
		loadScene(30, 18, 159, 2);
		return true;

	case 0x67a6:
		loadScene(29, 149, 163, 1);
		playActorAnimation(974);
		moveTo(160, 188, 0);
		return true;

	case 0x6805:
		processCallback(0x6849);
		playSound(32, 12);
		playActorAnimation(694);
		playSound(15, 8);
		playAnimation(693, 0);
		setOns(6, 0);
		displayMessage(0x4cc7);
		inventory->add(54);
		disableObject(4);
		return true;

	case 0x6849: {
		Common::Point p = scene->getPosition();
		if (p.x == 208 && p.y == 151) {
			moveRel(0, 0, 2);
		} else
			moveTo(208, 151, 1);
	}
	return true;

	case 0x687a: //using the book
		if (CHECK_FLAG(0xDBC2, 1)) {
			displayMessage(0x4ca0);
		} else {
			playSound(49, 5);
			playSound(49, 17);
			playActorAnimation(691);
			if (!processCallback(0x68e6)) {
				if (!CHECK_FLAG(0xDBC0, 1)) {
					displayMessage(0x4c61);
					SET_FLAG(0xDBC0, 1);
				}
			} else {
				playSound(15, 8); //secret compartment
				playAnimation(692, 0);
				setOns(6, 59);
				enableObject(4);
				displayMessage(0x4c84);
				SET_FLAG(0xDBC2, 1);
			}
		}
		return true;

	case 0x68e6: { //checking drawers
		uint16 v = GET_FLAG(0xDBC1) - 1;
		uint bx = 0xDBB7;
		if (GET_FLAG(bx + v) != 1)
			return false;

		uint16 sum = 0;
		for (uint i = 0; i < 6; ++i) {
			sum += GET_FLAG(bx + i);
		}
		return sum == 1;
	}

	case 0x6918:
		if (inventory->has(55)) {
			displayMessage(0x4cd9);
			return true;
		}
		if (!CHECK_FLAG(0xDBC3, 1)) {
			playActorAnimation(695);
			Dialog::show(scene, 0x386a);
			SET_FLAG(0xDBC3, 1);
		}

		playSound(5, 11);
		playActorAnimation(696);
		inventory->add(55);
		return true;

	case 0x6962:
		if (CHECK_FLAG(0xDBB7, 1)) {
			setOns(0, 0);
			playSound(67, 4);
			playActorAnimation(678);
			SET_FLAG(0xDBB7, 0);
		} else if (CHECK_FLAG(0xDBB8, 1)) {
			processCallback(0x6b86);
		} else {
			playSound(66, 4);
			playActorAnimation(677);
			setOns(0, 53);
			SET_FLAG(0xDBB7, 1);
		}
		return true;

	case 0x69b8:
		if (CHECK_FLAG(0xDBB8, 1)) {
			setOns(1, 0);
			playSound(67, 4);
			playActorAnimation(680);
			SET_FLAG(0xDBB8, 0);
		} else if (CHECK_FLAG(0xDBB8, 1)) {
			processCallback(0x6b86);
		} else if (CHECK_FLAG(0xDBB9, 1)) {
			processCallback(0x6b86);
		} else {
			playSound(66, 5);
			playActorAnimation(679);
			setOns(1, 54);
			SET_FLAG(0xDBB8, 1);
		}
		return true;

	case 0x6a1b:
		if (CHECK_FLAG(0xDBB9, 1)) {
			setOns(2, 0);
			playSound(67, 5);
			playActorAnimation(682);
			SET_FLAG(0xDBB9, 0);
		} else if (CHECK_FLAG(0xDBB8, 1)) {
			processCallback(0x6b86);
		} else {
			playSound(67, 5);
			playActorAnimation(681);
			setOns(2, 55);
			SET_FLAG(0xDBB9, 1);
		}
		return true;

	case 0x6a73:
		if (CHECK_FLAG(0xDBBA, 1)) {
			setOns(3, 0);
			playSound(67, 4);
			playActorAnimation(684);
			SET_FLAG(0xDBBA, 0);
		} else if (!CHECK_FLAG(0xDBBB, 1)) {
			playSound(66, 4);
			playActorAnimation(683);
			setOns(3, 56);
			SET_FLAG(0xDBBA, 1);
		} else
			processCallback(0x6b86);
		return true;

	case 0x6acb:
		if (CHECK_FLAG(0xDBBB, 1)) {
			setOns(4, 0);
			playSound(67, 4);
			playActorAnimation(686);
			SET_FLAG(0xDBBB, 0);
		} else if (CHECK_FLAG(0xDBBA, 1)) {
			processCallback(0x6b86);
		} else if (CHECK_FLAG(0xDBBC, 1)) {
			processCallback(0x6b86);
		} else {
			playSound(66, 5);
			playActorAnimation(685);
			setOns(4, 57);
			SET_FLAG(0xDBBB, 1);
		}
		return true;

	case 0x6b2e:
		if (CHECK_FLAG(0xdbbc, 1)) {
			setOns(5, 0);
			playSound(67, 5);
			playActorAnimation(688);
			SET_FLAG(0xdbbc, 0);
		} else if (CHECK_FLAG(0xdbbc, 1)) {
			processCallback(0x6b86);
		} else {
			playSound(66, 6);
			playActorAnimation(687);
			setOns(5, 58);
			SET_FLAG(0xDBBC, 1);
		}
		return true;


	case 0x6b86:
		if (CHECK_FLAG(0xDBBD, 1)) {
			displayMessage(0x4b39);
		} else {
			displayMessage(0x4acd);
			displayMessage(0x4b0d);
			SET_FLAG(0xDBBD, 1);
		}
		return true;

	case 0x6be1: //handle to the bathroom
		if (CHECK_FLAG(0xDBD9, 1)) {
			displayMessage(0x5326); //i'd better catch johnny
		} else {
			playSound(88, 4);
			playActorAnimation(808);
			loadScene(36, 41, 195, 2);
		}
		return true;

	case 0x6bad:
		playSound(80, 4);
		playActorAnimation(971);
		loadScene(32, 139, 199, 1);
		return true;

	case 0x6c45:
		playSound(89, 6);
		playActorAnimation(CHECK_FLAG(0xDBEF, 1) ? 985 : 806);
		loadScene(34, 40, 133, 2);
		return true;

	case 0x6c83:
		Dialog::pop(scene, 0xdb2e, 0, 727, 0xd1, 0xef, 0, 1);
		scene->getObject(1)->setName((const char *)res->dseg.ptr(0xaa94));
		SET_FLAG(0xDBD1, 1);
		return true;

	case 0x6c9d: //getting jar
		setOns(0, 71);
		playSound(32, 5);
		playActorAnimation(732);
		disableObject(2);
		inventory->add(72);
		return true;

	case 0x6cc4: //secret diary
		playActorAnimation(754);
		hideActor();
		
		displayCutsceneMessage(0x517b, 30430);

		playMusic(3);
		loadScene(11, scene->getPosition());

		playAnimation(750, 2);
		Dialog::show(scene, 0x4f50, 751, 529, 0xe5, 0xd9, 2, 1);

		playAnimation(752, 0, true);
		playAnimation(753, 1, true);
		waitAnimation();
		Dialog::show(scene, 0x5168, 529, 751, 0xd9, 0xe5, 1, 2);
		
		loadScene(30, scene->getPosition());
		Dialog::show(scene, 0x449e, 733, 734, 0xe5, 0xd0, 2, 3);

		playSound(75, 13);
		playSound(32, 22);
		playAnimation(735, 1, true);
		playAnimation(736, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x46cf, 737, 738, 0xd0, 0xe5, 3, 2);


		playSound(32, 1);
		playAnimation(739, 1, true);
		playAnimation(740, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x4772, 733, 734, 0xe5, 0xd0, 2, 3);

		playAnimation(742, 1, true);
		playAnimation(741, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x481c, 743, 733, 0xd0, 0xe5, 3, 2); //where's my wallet??

		playAnimation(744, 1, true);
		playAnimation(745, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x4873, 734, 733, 0xd0, 0xe5, 3, 2);

		playAnimation(746, 1, true);
		playAnimation(747, 2, true);
		waitAnimation();

		
		Dialog::show(scene, 0x4da5, 734, 734, 0xd0, 0xd0, 3, 3);
		Dialog::show(scene, 0x4eb9, 748, 748, 0xd0, 0xd0, 3, 3);
		Dialog::show(scene, 0x4f15, 749, 749, 0xd0, 0xd0, 3, 3);
		Dialog::show(scene, 0x4f2f, 748, 748, 0xd0, 0xd0, 3, 3);

		playMusic(10);
		loadScene(32, scene->getPosition());
		showActor();
		playSound(26, 10);
		playActorAnimation(755);
		moveRel(0, 0, 3);

		Dialog::show(scene, 0x51bf, 0, 0, 0xd1, 0xd1, 0, 0);

		hideActor();
		loadScene(31, scene->getPosition());
		Dialog::show(scene, 0x539f, 763, 764, 0xd9, 0xd0, 1, 2);

		loadScene(32, scene->getPosition());
		showActor();
		Dialog::show(scene, 0x52c3, 0, 0, 0xd1, 0xd1, 0, 0); //i have to hide somewhere
		disableObject(3);
		enableObject(7);

		SET_FLAG(0xDBD5, 1);
		return true;

	case 0x6f20:
		if (CHECK_FLAG(0xDBD5, 1)) {
			displayMessage(0x51a7);
		} else {
			rejectMessage();
		}
		return true;

	case 0x6f75: //hiding in left corner
		moveRel(0, 0, 3);
		playActorAnimation(756);
		hideActor();
		playAnimation(758, 1);
		Dialog::show(scene, 0x52e6, 759, 759, 0xd0, 0xd0, 2, 2); //I have to buy...

		playSound(40, 5);
		playSound(52, 13);
		playSound(52, 17);
		playSound(52, 21);
		playAnimation(760, 1);
		setOns(1, 72);
		setOns(2, 73);
		loadScene(31, scene->getPosition());
		playSound(58, 5);
		playSound(58, 8);
		playSound(58, 10);
		playSound(58, 12);
		playSound(58, 14);
		playAnimation(765, 1);
		Dialog::show(scene, 0x5443, 766, 766, 0xd9, 0xd9, 1, 1);
		loadScene(32, scene->getPosition());
		Dialog::show(scene, 0x5358, 761, 761, 0xd0, 0xd0, 2, 2);
		playAnimation(762, 1);
		setOns(2, 0);
		showActor();
		playActorAnimation(757);
		moveRel(0, 0, 1);
		displayMessage(0x51e7);
		enableObject(8);
		disableObject(7);

		SET_FLAG(0xDBD5, 0);
		return true;

	case 0x6f4d:
		if (CHECK_FLAG(0xDBD5, 1)) {
			displayMessage(0x51bb);
		} else {
			loadScene(31, 139, 172, 3);
		}
		return true;

	case 0x6f32:
		if (CHECK_FLAG(0xDBD5, 1)) {
			displayMessage(0x51a7);
		} else {
			playActorAnimation(977);
			displayMessage(0x5511);
		}
		return true;

	case 0x7096:
		playSound(32, 5);
		playActorAnimation(767);
		setOns(1, 0);
		inventory->add(73);
		disableObject(8);
		return true;

	case 0x7291:
		playSound(89, 3);
		playActorAnimation(975);
		loadScene(31, 298, 177, 4);
		return true;

	case 0x72c2:
		if (CHECK_FLAG(0xDBD6, 2)) {
			displayMessage(0x522c);
		} else {
			playSound(79, 6);
			playSound(84, 9);
			playActorAnimation(801);
			if (CHECK_FLAG(0xDBD6, 1)) {
				displayMessage(0x538d);
				SET_FLAG(0xDBD6, 2);
			} else
				displayMessage(0x5372);
		}
		return true;

	case 0x7309:
		playSound(66, 5);
		playSound(67, 11);
		playActorAnimation(976);
		displayMessage(0x5955);
		return true;
		
	case 0x77d5:
		playSound(89, 6);
		playActorAnimation(978);
		loadScene(31, 298, 177, 4);
		return true;

	case 0x79e4:
		processCallback(0x6849);
		return false;

	case 0x79eb: //color of the book
		displayMessage(res->dseg.get_word(0x5f3c + GET_FLAG(0xDBC1) * 2 - 2));
		return true;

	case 0x79fd:
		if (CHECK_FLAG(0xDBB7, 1)) {
			displayMessage(0x4b6c);
			return true;
		} else
			return false;

	case 0x7a0f:
		if (CHECK_FLAG(0xDBB8, 1)) {
			if (!CHECK_FLAG(0xDBBF, 1)) {
				displayMessage(0x4c32);
				playSound(5, 11);
				playActorAnimation(690);
				inventory->add(53);
				SET_FLAG(0xDBBF, 1);
			}
			displayMessage(0x4b87);
			return true;
		} else
			return false;

	case 0x7a49:
		if (CHECK_FLAG(0xDBB9, 1)) {
			displayMessage(0x4ba1);
			return true;
		} else
			return false;

	case 0x7a5b:
		if (CHECK_FLAG(0xDBBA, 1)) {
			displayMessage(0x4bbc);
			return true;
		} else
			return false;

	case 0x7a6d:
		if (CHECK_FLAG(0xDBBB, 1)) {
			displayMessage(0x4bd8);
			return true;
		} else
			return false;

	case 0x7a7f:
		if (CHECK_FLAG(0xDBBC, 1)) {
			if (!CHECK_FLAG(0xDBBE, 1)) {
				displayMessage(0x4c0f); //there's dictaphone inside!
				playSound(5, 12);
				playActorAnimation(689);
				inventory->add(52);
				SET_FLAG(0xDBBE, 1);
			}
			displayMessage(0x4bf4);
			return true;
		} else
			return false;
			
	case 0x7af7:
		if (CHECK_FLAG(0xDBD0, 1)) {
			displayMessage(0x5082);
			return true;
		} else 
			return false;

	case 0x7b09: {
		byte v = GET_FLAG(0xDBD6);
		switch (v) {
		case 1:
			displayMessage(0x51f8);
			return true;
		case 2:
			displayMessage(0x538d);
			return true;
		default:
			return false;
		}
	}

	case 0x9166:
		if (CHECK_FLAG(0xDBD1, 1)) {
			return true;
		} else {
			displayMessage(0x50a6);
			return false;
		}

	case 0x9175:
		if (CHECK_FLAG(0xDBD2, 0) || CHECK_FLAG(0xDBD3, 0) || CHECK_FLAG(0xDBD4, 0))
			return true;

		waitLanAnimationFrame(1, 1);
		playSound(89, 2);
		playActorAnimation(731);
		setOns(0, 70);
		setLan(1, 0);
		disableObject(1);
		enableObject(2);
		enableObject(3);
		return true;

	case 0x90bc: //handle on the hole
		playSound(5, 3);
		playSound(6, 9);
		playActorAnimation(807);
		setOns(0, 83);
		inventory->remove(73);
		disableObject(2);
		enableObject(3);
		SET_FLAG(0xDBEF, 1);
		return true;

	case 0x90fc: //dictaphone on robot
		if (!processCallback(0x9166))
			return true;

		if (CHECK_FLAG(0xDBD2, 1)) {
			displayMessage(0x50c3);
			return true;
		}

		if (!CHECK_FLAG(0xDBCB, 1)) {
			displayMessage(0x5101);
			return true;
		}

		displayMessage(0x50e1);
		waitLanAnimationFrame(1, 1);

		playSound(5, 3);
		playSound(5, 39);
		displayAsyncMessage(0x5124, 40388, 9, 35, 0xd0);
		playActorAnimation(728);
		//fixme: add 727 animation
		Dialog::show(scene, 0x3d17, 0, 0, 0xd1, 0xef, 0, 1);
		SET_FLAG(0xDBD2, 1);
		processCallback(0x9175);
		return true;

	case 0x91cb: //use socks on robot
		if (!processCallback(0x9166))
			return true;

		if (CHECK_FLAG(0xDBD3, 1)) {
			displayMessage(0x50c3);
			return true;
		}
		displayMessage(0x5138);
		waitLanAnimationFrame(1, 1);

		playSound(5, 3);
		playSound(5, 23);
		playActorAnimation(729);
		//fixme: add 727 animation
		Dialog::show(scene, 0x3d70, 0, 0, 0xd1, 0xef, 0, 1);
		SET_FLAG(0xDBD3, 1);
		processCallback(0x9175);
		return true;

	case 0x9209: //photo on robot
		if (!processCallback(0x9166))
			return true;

		if (CHECK_FLAG(0xDBD4, 1)) {
			displayMessage(0x50c3);
			return true;
		}
		displayMessage(0x5161);
		waitLanAnimationFrame(1, 1);

		playSound(5, 3);
		playSound(5, 25);
		playActorAnimation(730);
		//fixme: add 727 animation
		Dialog::show(scene, 0x3dd6, 0, 0, 0xd1, 0xef, 0, 1);
		SET_FLAG(0xDBD4, 1);
		processCallback(0x9175);
		return true;

	case 0x924e:
		setOns(2, 64);
		playSound(5, 3);
		playSound(52, 10);
		playActorAnimation(711);
		moveRel(0, 0, 4);
		Dialog::show(scene, 0x3b21, 0, 709, 0xd1, 0xef, 0, 1);
		moveTo(300, 190, 4);
		inventory->remove(64);
		disableObject(8);
		playAnimation(712, 0);
		setOns(2, 0);
		playSound(15, 26);
		playSound(15, 28);
		playSound(16, 37);
		playAnimation(713, 0);
		Dialog::show(scene, 0x3c0d, 0, 709, 0xd1, 0xef, 0, 1);
		playSound(85, 2);
		playAnimation(714, 0);
		setLan(1, 0);
		disableObject(1);
		{
			Object *obj = scene->getObject(2);
			obj->actor_rect.left = obj->actor_rect.right = 81;
			obj->actor_rect.top = obj->actor_rect.bottom = 160;
			obj->actor_orientation = 4;
			obj->save();
		}
		{
			Object *obj = scene->getObject(3);
			obj->actor_rect.left = obj->actor_rect.right = 64;
			obj->actor_rect.top = obj->actor_rect.bottom = 168;
			obj->actor_orientation = 4;
			obj->save();
		}
		{
			Object *obj = scene->getObject(10);
			obj->actor_rect.left = obj->actor_rect.right = 105;
			obj->actor_rect.top = obj->actor_rect.bottom = 160;
			obj->actor_orientation = 1;
			obj->save();
		}
		SET_FLAG(0xDBCC, 1);
		return true;

	case 0x9472:
		playSound(5, 4);
		playSound(19, 14);
		playActorAnimation(793);
		displayMessage(0x5218);
		inventory->remove(60);
		SET_FLAG(0xDBD6, 1);
		return true;

	case 0x9449: //meat + stew
		playSound(5, 4);
		playSound(63, 12);
		playActorAnimation(726);
		displayMessage(0x508a);
		inventory->remove(69);
		inventory->add(70);
		return true;

	case 0x949b:
		if (CHECK_FLAG(0xDBD6, 2)) {
			playSound(5, 4);
			playSound(5, 25);
			playActorAnimation(802);
			displayMessage(0x5272);
			inventory->remove(62);
			inventory->add(74);
			inventory->add(65);
		} else
			displayMessage(0x524f);
		return true;

	case 0x94d4:
		if (inventory->has(70)) {
			setOns(0, 0);
			playSound(5, 3);
			playSound(5, 18);
			playSound(13, 12);
			playActorAnimation(803);
			disableObject(7);
			inventory->remove(70);
			inventory->add(71);
		} else
			displayMessage(0x53ad);
		return true;

	case 0x951b:
		playSound(5, 4);
		playSound(5, 22);
		playActorAnimation(804);
		displayMessage(0x528b);
		return true;

	case 0x9537: //using remote on VCR
		playSound(5, 3);
		playSound(5, 16);
		playActorAnimation(703);
		if (CHECK_FLAG(0xDBC8, 1)) {
			if (CHECK_FLAG(0xDBC6, 0)) {
				if (CHECK_FLAG(0xDBC5, 1)) { //tv on
					if (!CHECK_FLAG(0xDBC7, 1))
						displayMessage(0x4d93); //the tape started
					playAnimation(702, 0, true, true, true); //fixme: we need some overlay animation support
					SET_FLAG(0xDBC6, 1);
					if (!CHECK_FLAG(0xDBC7, 1)) {
						Dialog::show(scene, 0x392c, 0, 0, 0xd1, 0xd0, 0, 1);
						SET_FLAG(0xDBC7, 1);
					}
					reloadLan();
				} else
					displayMessage(0x4d5b);
			} else {
				SET_FLAG(0xDBC6, 0);
				if (CHECK_FLAG(0xDBC5, 1)) { //tv on
					playAnimation(701, 1);
					displayMessage(0x4da6); //much better!
				}
			}
		} else
			displayMessage(0x4D80); //nothing happened
		return true;

	case 0x95eb: //polaroid + tv
		if (CHECK_FLAG(0xDBC6, 1)) {
			if (CHECK_FLAG(0xDBCA, 1)) {
				displayMessage(0x4de6);
			} else {
				playSound(5, 3);
				playSound(5, 24);
				playSound(90, 18);
				playActorAnimation(707);
				inventory->add(61);
				SET_FLAG(0xDBCA, 1);
			}
		} else
			displayMessage(0x4ea5);
		return true;

	case 0x962f: //polaroid + tv
		if (CHECK_FLAG(0xDBC6, 1)) {
			if (CHECK_FLAG(0xDBCB, 1)) {
				displayMessage(0x4e32);
			} else {
				displayMessage(0x4e05);
				playSound(5, 3);
				playSound(5, 27);
				playActorAnimation(708);
				SET_FLAG(0xDBCB, 1);
			}
		} else
			displayMessage(0x4ea5);
		return true;


	case 0x95c8:
		playSound(5, 3);
		playSound(91, 12);
		playActorAnimation(706);
		inventory->remove(54);
		SET_FLAG(0xDBC8, 1);
		return true;

	case 0x9673:
		playSound(5, 3);
		playSound(24, 10);
		playActorAnimation(798);
		playSound(63, 11);
		playSound(19, 20);
		playAnimation(799, 0);
		moveTo(50, 170, 1);
		playAnimation(800, 0, true, true, true);
		playActorAnimation(805);
		moveTo(50, 170, 3);
		displayMessage(0x5349);
		//moveTo(105, 157, 0, true);
		playMusic(3);
		loadScene(11, 105, 157, 0);
		Dialog::show(scene, 0x8409, 0, 938, 0xd1, 0xec, 0, 1);

		playAnimation(939, 0, true);
		playActorAnimation(942, true);
		waitAnimation();

		playAnimation(939, 0, true);
		playAnimation(935, 1, true);
		playActorAnimation(943, true);
		waitAnimation();

		playAnimation(940, 0, true);
		playAnimation(936, 1, true);
		playActorAnimation(944, true);
		waitAnimation();

		playAnimation(941, 0, true);
		playAnimation(937, 1, true);
		playActorAnimation(945, true);
		waitAnimation();

		playAnimation(945, 0);
		Dialog::show(scene, 0x844f, 0, 938, 0xd1, 0xec, 0, 2);
		playAnimation(946, 1);
		Dialog::show(scene, 0x87c7, 0, 938, 0xd1, 0xec, 0, 2);

		playSound(24, 7);
		playAnimation(948, 0, true);
		playActorAnimation(947, true);
		waitAnimation();

		loadScene(40, 198, 186, 1);
		Dialog::show(scene, 0x8890);
		Dialog::show(scene, 0x8a2f);
		playAnimation(923, 0);
		Dialog::show(scene, 0x8aa7);

		moveTo(237, 186, 0);
		moveTo(237, 177, 0);
		moveTo(192, 177, 0);
		playAnimation(949, 1);
		Dialog::show(scene, 0x8af6, 950, 950, 0xe7, 0xe7, 1, 1);

		playSound(32, 5);
		playSound(40, 14);

		playAnimation(951, 0, true);
		playActorAnimation(952, true);
		waitAnimation();

		playMusic(11);
		loadScene(39, 192, 177, 0);
		hideActor();
		Dialog::show(scene, 0x8b4d, 953);
		playSound(5, 15);
		playAnimation(954, 0);
		Dialog::show(scene, 0x8b7a, 955);
		playMusic(2);

		displayCredits(0xe47c);
		scene->push(SceneEvent(SceneEvent::kQuit));

		return true;

	case 0x9921: {
		int id = scene->getId();
		if (id != 15) {
			displayMessage(id == 16 ? 0x38ce : 0x38a7);
		} else {
			playSound(5, 3);
			playSound(38, 16);
			playSound(38, 22);
			playActorAnimation(614);
			playSound(5, 3);
			playSound(44, 10);
			playSound(20, 26);
			playActorAnimation(615);
			loadScene(17, 156, 180, 3);
			SET_FLAG(0, 4);
			playSound(64, 7);
			playSound(64, 21);
			playSound(64, 42);
			playSound(64, 63);
			setTimerCallback(0x9a1d, 40);
			playActorAnimation(617, false, true);
			//another time challenge!
		}
	}
	return true;
	
	case 0x9a1d: //no anchor, timeout
		SET_FLAG(0, 0);
		processCallback(0x9a7a);
		INC_FLAG(0xDBA6);
		switch(GET_FLAG(0xDBA6)) {
		case 1:
			displayMessage(0x39ae);
			break;
		case 2:
			displayMessage(0x39f6);
			break;
		case 3:
			displayMessage(0x3a28);
			break;
		case 4:
			displayMessage(0x3a85);
			break;
		case 5:
			displayMessage(0x39ae);
			break;
		default: 
			displayMessage(0x39b7);
		}
		return true;
	
	case 0x99e0: //success getting an anchor
		SET_FLAG(0, 0);
		setTimerCallback(0, 0);
		scene->getActorAnimation()->free();
		playSound(64, 7);
		playActorAnimation(618);
		disableObject(5);
		setOns(0, 0);
		playSound(31, 1);
		playActorAnimation(619);
		processCallback(0x9a7a);
		inventory->add(42);
		displayMessage(0x3989);
		return true;
		
	case 0x9a7a:
		loadScene(15, 156, 180, 3);
		playSound(5, 5);
		playSound(38, 14);
		playSound(38, 20);
		playSound(5, 25);
		playActorAnimation(616);
		return true;
		
	case 0x9aca:
		if (scene->getId() == 13) {
			moveTo(172, 181, 1);
			playSound(26, 19);
			for (uint i = 0; i < 8; ++i)
				playSound(26, 30 + i * 11);
			playActorAnimation(661);
			displayCutsceneMessage(0x3c80, 30484);
			playSound(56, 10);
			playSound(56, 21);

			playSound(8, 48);
			for (uint i = 0; i < 7; ++i)
				playSound(26, 117 + i * 11);

			moveRel(-20, 0, 0, true);
			playActorAnimation(662, true);
			playAnimation(663, 2, true);
			waitAnimation();
			setOns(1, 49);

			displayCutsceneMessage(0x3c9a, 30453);
			moveTo(162, 184, 0, true);
			playSound(26, 6);
			playSound(26, 17);
			playSound(56, 10);
			playSound(56, 21);
			playSound(19, 27);
			playSound(24, 38);
			playSound(23, 44);
			playActorAnimation(664);
			playAnimation(665, 1);
			displayMessage(0x3cbc);
			displayMessage(0x3cea);
			inventory->remove(37);
			processCallback(0x9d45); //another mansion try
		} else
			displayMessage(0x3c58);
		return true;

	case 0x9c6d:
		displayMessage(0x49d1);
		SET_FLAG(0xDBB5, 1);
		return false;

	case 0x9c79: //use pills
		if (scene->getId() != 36) {
			displayMessage(0x52a9);
		} else if (CHECK_FLAG(0xDBF1, 1)) {
			displayMessage(0x52F6);
		} else {
			SET_FLAG(0xDBF1, 1);
			moveTo(102, 195, 2);
			playSound(5, 3);
			playSound(75, 12);
			playActorAnimation(794);
			setLan(1, 0);
			//scene->getWalkbox(0)->rect.left = 0;
			//scene->getWalkbox(0)->rect.top = 0;
			moveTo(151, 197, 2);
			playActorAnimation(795);
			moveTo(186, 198, 2, true);
			moveTo(220, 198, 4);
			//scene->getWalkbox(0)->rect.top = 200;
			setLan(1, 0xff);

			Dialog::show(scene, 0x58a9);

			Object *obj = scene->getObject(1);
			obj->actor_rect.left = obj->actor_rect.right = 270;
			obj->actor_rect.top = obj->actor_rect.bottom = 193;
			obj->actor_orientation = 2;
			obj->save();

			obj = scene->getObject(3);
			obj->actor_rect.left = obj->actor_rect.right = 254;
			obj->actor_rect.top = obj->actor_rect.bottom = 193;
			obj->actor_orientation = 1;
			obj->save();
			
			SET_FLAG(0xDBD7, 1);
		}
		return true;

	case 0x9d45: {
		byte tries = ++ *(res->dseg.ptr(0xDBEA));
		debug(0, "another mansion try: %u", tries);
		if (tries >= 7)
			return false;

		uint16 ptr = res->dseg.get_word((tries - 2) * 2 + 0x6035);
		byte id = scene->getId();

		playMusic(11);
		displayCutsceneMessage(0x580a, 30484);
		processCallback(ptr);
		playMusic(6);
		if (scene->getId() == 11 && CHECK_FLAG(0xDBEC, 1))
			return true;
		//some effect
		loadScene(id, scene->getPosition());
	}
	return true;

	case 0x9d90:
		hideActor();
		loadScene(34, scene->getPosition());
		Dialog::show(scene, 0x6f60, 986, 987, 0xd9, 0xd0, 1, 2);
		playAnimation(990, 0, true);
		playAnimation(991, 1, true);
		waitAnimation();
		showActor();
		return true;

	case 0x9de5:
		hideActor();
		loadScene(30, scene->getPosition());
		playAnimation(887, 1, true);
		playAnimation(888, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x6fb8, 889, 890, 0xd9, 0xd0, 2, 3);
		playSound(26, 3);
		playAnimation(891, 1, true);
		playAnimation(892, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x6ff0, 890, 889, 0xd0, 0xd9, 3, 2);
		showActor();
		return true;

	case 0x9e54:
		hideActor();
		loadScene(32, scene->getPosition());
		playAnimation(894, 1, true);
		playAnimation(893, 2, true);
		waitAnimation();
		Dialog::show(scene, 0x706e, 894, 893, 0xd9, 0xd1, 3, 2);
		playSound(75, 9);
		playAnimation(898, 1, true);
		playAnimation(897, 2, true);
		Dialog::show(scene, 0x7096, 896, 895, 0xd0, 0xd9, 2, 3);
		showActor();
		return true;

	case 0x9ec3:
		hideActor();
		loadScene(29, scene->getPosition());
		playActorAnimation(901, true);
		playAnimation(900, 1, true);
		waitAnimation();
		Dialog::show(scene, 0x7161, 903, 902, 0xd0, 0xd9, 2, 3);
		for (byte i = 3; i <= 9; i += 2)
			playSound(56, i);

		playActorAnimation(905, true);
		playAnimation(904, 1, true);
		Dialog::show(scene, 0x71c6, 903, 902, 0xd0, 0xd9, 2, 3);
		showActor();
		return true;

	case 0x9f3e:
		hideActor();
		loadScene(35, scene->getPosition());
		playAnimation(907, 2, true);
		playAnimation(906, 3, true);
		waitAnimation();
		Dialog::show(scene, 0x7243, 908, 909, 0xd9, 0xd0, 2, 3);
		Dialog::show(scene, 0x7318, 910, 908, 0xd0, 0xd9, 3, 2);
		loadScene(11, scene->getPosition());
		setOns(3, 51);
		playAnimation(911, 1);
		playAnimation(899, 1);
		enableObject(8);
		setLan(2, 8);
		SET_FLAG(0xDBEC, 1);
		return true;
	}

	//error("invalid callback %04x called", addr);
	warning("invalid callback %04x called", addr);
	return true;
}

} // End of namespace TeenAgent
