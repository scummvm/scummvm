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

#include "scene.h"
#include "teenagent.h"
#include "resources.h"
#include "dialog.h"

using namespace TeenAgent;

#define CHECK_FLAG(addr, v) (res->dseg.get_byte(addr) == (v))
#define SET_FLAG(addr, v) (res->dseg.set_byte((addr), (v)))
#define GET_FLAG(addr) (res->dseg.get_byte(addr))

void TeenAgentEngine::rejectMessage() {
	Resources * res = Resources::instance();
	//random reject message:
	uint i = random.getRandomNumber(3);
	//debug(0, "reject message: %s", (const char *)res->dseg.ptr(res->dseg.get_word(0x339e + 2 * i)));
	scene->displayMessage((const char *)res->dseg.ptr(res->dseg.get_word(0x339e + 2 * i)));
}

void TeenAgentEngine::anotherMansionTry() {
	Resources * res = Resources::instance();
	byte tries = ++ *(res->dseg.ptr(0xDBEA));
	debug(0, "another mansion try: %u", tries);
	if (tries >= 7)
		return;
	uint16 ptr = res->dseg.get_word(res->dseg.get_word((tries - 2) * 2 + 0x6035));
	
	playMusic(11);
	debug(0, "FIXME: cutscene: meanwhile in a mansion #%u, %04x", tries, ptr);
	processCallback(ptr);
	playMusic(6);
	if (scene->getId() == 11 && CHECK_FLAG(0xDBEC, 1))
		return;
	//call 0xac27
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
		switch(func) {
		case 0x11c5: 
			Dialog::show(scene, msg);
			return true;
		case 0xa055:
			displayMessage((const char *)res->dseg.ptr(msg));
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
	
	switch(addr) {
	
	case 0x4021:
		//pulling out mysterious object
		if (CHECK_FLAG(0xdbe1, 1)) {
			playAnimation(844);
			playAnimation(846);
			playAnimation(845);
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
			playSound(76);
			playAnimation(864);
			playAnimation(866);
			//InventoryObject *obj = inventory->selectedObject();
			//if (obj != NULL && obj->id == 0x55) {

			//implement pause and using real object: 
			if (inventory->has(0x55)) {
				playSound(5, 4);
				playAnimation(867);
				inventory->remove(0x55);
				inventory->add(0x56);
				moveTo(86, 195, true); //warp and orientation 1
				playAnimation(868);
				SET_FLAG(0xDBE4, 1);
			} else {
				//fail! 
				moveTo(86, 195, true); //warp and orientation 1
				playAnimation(868);
				Dialog::pop(scene, 0xDB72);
			}
			return true;
		}
		
	case 0x419c: //getting the bird
		setOns(0, 0);
		playSound(56, 10);
		playAnimation(875);
		disableObject(6);
		inventory->add(0x5c);
		return true;
		
		
	case 0x41ce: 
		moveTo(Common::Point(197, 159));
		setOns(0, 0);
		playSound(71);
		playAnimation(833);
		moveTo(Common::Point(225, 159));
		inventory->add(0x4e);
		disableObject(3);
		return true;
		
	case 0x4267:
		playSound(23, 8);
		setOns(1, 0);
		playAnimation(841);
		setOns(1, 0x61);
		setOns(2, 0);
		playSound(63, 12);
		playAnimation(842);
		//shown in different positions
		displayMessage(0x5656);
		displayMessage(0x567a);
		displayMessage(0x5682);
		playAnimation(843);
		moveTo(223, 149, true);
		disableObject(7);
		disableObject(1);
		inventory->add(0x51);
		displayMessage(0x5646);
		return true;
	
	case 0x4388:
		playSound(80);
		loadScene(8, 155, 199);
		scene->setOrientation(1);
		return true;

	case 0x43b5: //HQ, first trial - prison
		playSound(70);
		playAnimation(962);
		loadScene(7, 30, 184, 2);
		if (res->dseg.get_byte(0xDBDF) < 2) {
			moveTo(Common::Point(134, 167));
			displayMessage(0x54f7);
			setLan(1, 0);
			playAnimation(812, 1, true);
			playAnimation(811);
			Dialog::show(scene, 0x6117, 813);
			loadScene(6, Common::Point(230, 184));
			Dialog::show(scene, 0x626a, 814);
			playAnimation(815, 1);
			setOns(1, 0);

			Dialog::show(scene, 0x62dc);

			SET_FLAG(0xDBDF, 1);
			SET_FLAG(0xDB90, 5);
		}
		return true;

	case 0x4482:
		if (CHECK_FLAG(0xDBDF, 0)) {
			playAnimation(968);
			displayMessage(0x5511);
		} else {
			playSound(80);
			playAnimation(968);
			loadScene(6, Common::Point(280, 186));
		}
		return true;

	case 0x44fc:  //pull out spring from bed
		playSound(53, 25);
		playAnimation(839);
		moveTo(278, scene->getPosition().y, true);
		inventory->add(0x50);
		disableObject(1);
		return true;

	case 0x44cb: 
		if (CHECK_FLAG(0xDBE5, 1)) {
			scene->displayMessage((const char *)res->dseg.ptr(0x57c0));
		} else {
			playSound(49);
			playAnimation(869);
			inventory->add(0x58);
			SET_FLAG(0xDBE5, 1);
		}
		return true;
		
	case 0x4539: //prison cell: use crates
		if (CHECK_FLAG(0xdbdd, 2)) {
			//finished the meal - trap
			displayMessage(0x55c0);
			moveTo(306, 196);
			//playAnimation(825, 1); //very long empty animation. what for?
			setLan(1, 0);
			playSound(71, 4);
			playAnimation(823);

			//skipped one 826 animation for this scene!
			playSound(74, 4); //delay 10 in code
			loadScene(5, scene->getPosition());
			playAnimation(826);
			loadScene(6, scene->getPosition());
			setOns(3, 0x5b);
			displayMessage(0x55db);
			SET_FLAG(0xdbdd, 3);
			strcpy(scene->getObject(4)->name, "body");
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
			moveTo(280, 179);
			playSound(49, 7);
			playAnimation(827);
			inventory->add(0x4d);
			SET_FLAG(0xDBDE, 1);
		} else 
			displayMessage(0x5905);
		return true;
		
	case 0x46af: //prison cell: use live cable 
		if (CHECK_FLAG(0xdbdc, 1)) {
			displayMessage(0x555d);
			setOns(2, 0);
			playAnimation(821);
			setOns(2, 0x5a);
			setOns(3, 0);
			playSound(22, 2);
			playAnimation(822);
			displayMessage(0x5577);
			disableObject(5);
			SET_FLAG(0xdbdd, 1);
		} else 
			displayMessage(0x5528);
		return true;
	
	case 0x4705: { //prison: getting lamp bulb
		moveTo(144, 185);
		playSound(56, 15);
		setOns(0, 86); //hiding lamp
		playAnimation(816, 0, true);
		playAnimation(817, 1, true);
		waitAnimation();
		setOns(0, 87);
		
		playSound(34, 1);
		playAnimation(818);
		playAnimation(819, 1, true);
		waitAnimation();
		
		moveTo(160, 188, true);
		setOns(2, 88);

		disableObject(6);
		enableObject(5);
		inventory->add(0x4c);
		}
		return true;
	
	case 0x4794: //prison cell door
		if (res->dseg.get_byte(0xDBDF) >= 2) {
			loadScene(5, 287, 143);
		} else {
			displayMessage(0x592f);
		}
		return true;
	
	case 0x47bc: //prison: examining trash can
		playSound(49, 5);
		playAnimation(966);
		displayMessage(0x5955);
		return true;
		
	case 0x47db: //prison: use switch
		if (CHECK_FLAG(0xDBDF, 1)) {
			playSound(71, 4);
			playAnimation(823);
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
		playAnimation(965);
		displayMessage(0x5511);
		return true;
		
	case 0x4893: //taking pills
		if (CHECK_FLAG(0xDBE6, 1)) {
			SET_FLAG(0xDBE6, 2);
			setOns(1, 0x67);
			playSound(5, 9);
			playAnimation(872);
			inventory->add(0x5a);
			disableObject(7);
		} else {
			playAnimation(964);
			displayMessage(0x5511);
		}
		return true;
	
	case 0x4918: //talking with barmen
		if (CHECK_FLAG(0xDBE7, 1)) {
			moveTo(140, 152);
			if (CHECK_FLAG(0xDBE8, 1)) {
				Dialog::show(scene, 0x6f20);
				displayMessage(0x5883, 0xef);
				//reloadLan();
				setLan(1, 0);
				playAnimation(882, 1);
				playSound(75, 10);
				setOns(2, 0);
				playAnimation(883, 1);
				disableObject(1);
				disableObject(2);
				SET_FLAG(0xDBE9, 1);
			} else 
				displayMessage(0x5855);
		} else {
			if (CHECK_FLAG(0xDBDF, 3)) {
				if (CHECK_FLAG(0xDBE3, 1)) {
					Dialog::show(scene, 0x6BD6, 857);
				} else {
					Dialog::show(scene, 0x69B5, 857); //taking mug
					playAnimation(859, 0, true);
					playAnimation(858, 1, true);
					waitAnimation();
					playSound(75, 6);
					playAnimation(860);
					Dialog::show(scene, 0x69C2, 857);
					inventory->add(0x55);
					SET_FLAG(0xDBE3, 1);
					SET_FLAG(0xDBF0, 0);
				}
			} else {
				Dialog::pop(scene, 0xDB68, 857);
			}
		}
		return true;
		
	case 0x4f14: //use the hollow
		displayMessage(CHECK_FLAG(0xDBA1, 1)? 0x370f: 0x36c2);
		return true;
		
	case 0x4a64: 
		if (CHECK_FLAG(0xDBF0, 1)) {
			displayMessage(0x5e25);
		} else {
			loadScene(5, 35, 162);
		}
		return true;
		
	case 0x4bf5: 
		playAnimation(959);
		loadScene(8, 40, 152, 3);
		return true;
	
	case 0x483a:
		Dialog::pop(scene, 0xdb82);
		return true;
		
	case 0x4844:
		playSound(80, 4);
		playAnimation(963);
		loadScene(5, 166, 158);
		return true;
		
	case 0x48ea: 
		setOns(0, 0);
		playSound(5, 9);
		playAnimation(836);
		inventory->add(0x4f);
		disableObject(12);
		return true;
		
	case 0x4a8c: 
		if (CHECK_FLAG(0xDBE9, 1)) {
			playSound(89, 5);
			playAnimation(958);
			loadScene(9, 240, 182, 4);
		} else if (CHECK_FLAG(0xDBE9, 1)) {
			displayMessage(0x5894);
		} else {
			Dialog::pop(scene, 0xDB8A, 857);
		}
		return true;
		
	case 0x4af4: //taking the crumbs
		setOns(0, 0);
		playAnimation(861);
		playSound(49);
		inventory->add(0x57);
		disableObject(6);
		return true;
		
	case 0x4b35:
		playSound(15, 7);
		playAnimation(884);
		playSound(55, 1);
		playAnimation(885, 1);
		Dialog::show(scene, 0x67e5, 886);
		playMusic(3);
		loadScene(40, 198, 186, 1);
		Dialog::show(scene, 0x7f20);
		inventory->clear();
		inventory->add(0x1d);
		//showFullscreenMessage(0xe45c);
		loadScene(1, 198, 186);
		playAnimation(956, 0, true);
		Dialog::show(scene, 0x8bc4);
		waitAnimation();
		loadScene(15, 157, 199, 1);
		playMusic(6);
		return true;

	case 0x4c3e: //get the grenade
		playSound(32, 24);
		playAnimation(862);
		reloadLan();
		playAnimation(863, 1);
		inventory->add(0x54);
		disableObject(1);
		SET_FLAG(0xDBE2, 2);
		return true;

	case 0x4c70:
		if (CHECK_FLAG(0xDBE2, 0)) {
			if (CHECK_FLAG(0xDBDA, 1)) { //papers are shown
				Dialog::pop(scene, 0xDB4C);
			} else {
				Dialog::pop(scene, 0xDB40);
			}
			return true;
		} else {
			displayMessage(0x5722);
	
			scene->displayMessage("He's totally addicted.");
		}
		return true;
		
	case 0x4c1c:
		playAnimation(960);
		displayMessage(0x5511);
		return true;
		
	case 0x4cac:
		if (CHECK_FLAG(0xdbda, 1)) { //papers are shown
			loadScene(5, 124, 199);
		} else {
			playAnimation(809, 1, true);
			Dialog::show(scene, 0x5FE9);
			moveTo(Common::Point(269, 175));
			Dialog::pop(scene, 0xDB56);
		}
		return true;

	case 0x4cf1: { //talking with mansion guard 
		playAnimation(529, 1);
		SET_FLAG(0xda96, 1);
		if (Dialog::pop(scene, 0xdaa6) != 0x1b4)
			return true;
		Common::Point p = scene->getPosition();
		moveTo(159, 189);
		
		playAnimation(550);
		playAnimation(551, 1);

		moveTo(p);
		inventory->add(0x13);
		playAnimation(529, 1);
		Dialog::pop(scene, 0xdaa6);
		}
		return true;
	
	case 0x4e61:
		loadScene(14, 280, 198);
		return true;

	case 0x4f25:
		playAnimation(967);
		displayMessage(0x3542);
		return true;
		
	case 0x4f32:
		if (CHECK_FLAG(0xDBA1, 1)) {
			break;
		} else {
			playAnimation(49);
			playSound(56);
			//there's some black magic here! investigate! 
			playAnimation(587);
			displayMessage(0x4652);
			displayMessage(0x3668);
			return true;
		}
		
	case 0x500d: //picking up wild plant
		if (CHECK_FLAG(0xDB9E, 1)) {
			displayMessage(0x35E8); //there are no more
		} else {
			SET_FLAG(0xDB9E, 1);
			setOns(2, 0);
			playAnimation(552);
			setOns(2, 0x12);
			inventory->add(0x14);
		}
		return true;
		
	case 0x5104:
		loadScene(11, 319, 198, 4); //orientation: left
		if (CHECK_FLAG(0xDB9C, 1)) 
			return true;

		SET_FLAG(0xDB9C, 1); //guard's drinking, boo! 
		playAnimation(544, 1);
		displayMessage(0x3563);
		playSound(17);
		playAnimation(545, 1);
		setOns(0, 16);
		enableObject(2);
		Dialog::show(scene, 0x0917);
		
		return true;
		
	case 0x5217:
		displayMessage(CHECK_FLAG(0xDB9F, 1)? 0x402e: 0x34e1);
		return true;
		
	case 0x5237:
		if (!CHECK_FLAG(0xDB9F, 1)) {
			displayMessage(0x34e1);
		} else if (CHECK_FLAG(0xDBA0, 1)) 
			displayMessage(0x3E31);
		else {
			moveTo(173, 138);
			playAnimation(583);
			playAnimation(584);
			playSound(72);
			playAnimation(585);
			loadScene(11, 194, 160);
			playSound(28);
			moveTo(138, 163);
			displayMessage(0x3650);
			SET_FLAG(0xDBA0, 1);
		}
		return true;
		
	case 0x55a8: {
			uint16 d = Dialog::pop(scene, 0xdb08);
			if (d == 0x2c5d) {
				setOns(0, 0);
				playAnimation(570, 1);
				displayMessage(0x551f);
				disableObject(5);
				SET_FLAG(0xDBB0, 1);
			} else if (d != 0x2c9b) {
				playAnimation(569, 1);
			}
		} 
		return true;
		
	case 0x5663:
		displayMessage(CHECK_FLAG(0xDBB0, 1)? 0x41b1: 0x417e);
		return true;

	case 0x569c:
		playAnimation(983);
		displayMessage(0x5955);
		return true;
		
	case 0x56b7:
		playAnimation(984);
		displayMessage(0x5955);
		return true;
		
	case 0x5728:
		inventory->add(0x0d);
		disableObject(14);
		setOns(0, 0);
		playAnimation(566);
		return true;
		
	case 0x5793:
		if (!CHECK_FLAG(0xDB94, 1)) {
			displayMessage(0x3e63);
		} else if (CHECK_FLAG(0xDB95, 1)) {
			displayMessage(0x3e75);
		} else {
			SET_FLAG(0xDB95, 1);
			moveTo(188, 179);
			playSound(7, 16);
			playAnimation(519);
			moveTo(168, 179);
			inventory->add(3);
		}
		return true;
	
	case 0x5d88:
		if (CHECK_FLAG(0xDBA5, 1)) { //dry laundry
			SET_FLAG(0xDBA5, 2);
			Dialog::show(scene, 0x1F4F);
			playAnimation(604, 1);

			loadScene(21, scene->getPosition());
			setOns(0, 0);
			disableObject(4);
			enableObject(12);
			playSound(46);
			playAnimation(606, 1);
			loadScene(23, scene->getPosition());
			playAnimation(605, 1);
			Dialog::show(scene, 0x2002);
		} else {
			uint16 d = Dialog::pop(scene, 0xdada);
			if (d == 0x1913)
				displayMessage(0x34d5); //+orientation = 3
		}
		return true;
		
	case 0x5ff3: //get duster
		if (CHECK_FLAG(0xDB9A, 0)) {
			Dialog::pop(scene, 0xdaf6);
		} else {
			Dialog::show(scene, 0x1e1e);
			inventory->add(12);
			disableObject(12);
			setOns(0, 0);
			playSound(5);
			playAnimation(541);
		}
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
			playAnimation(635);
			setOns(5, 0);
			playSound(63);
			playAnimation(636);
			inventory->add(47);
			inventory->add(48);
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
		
		switch(v) {
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
		
	//case 0x78f5: //trunk description
	//	if (CHECK_FLAG(0xDB95, 1)) {
	//		displayMessage(0x3e75);
	//	}
	//	return;
	
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

	case 0x7b26: //cutting the fence
		setOns(0, 0);
		playSound(5, 2);
		playAnimation(837);
		playSound(51, 3);
		playAnimation(838);
		setOns(0, 0x60);
		moveTo(281, scene->getPosition().y, true);
		disableObject(4);
		SET_FLAG(0xDBE1, 1);
		return true;
		
	case 0x7b89: //digging mysterious object
		if (CHECK_FLAG(0xDBE1, 1)) {
			playAnimation(844);
			setOns(1, 0);
			playSound(5, 5);
			playAnimation(847);
			playSound(5, 11);
			playAnimation(848);
			setOns(1, 0x64);
			playAnimation(845);
			disableObject(3);
			inventory->add(0x52);
			inventory->remove(0x51);
		} else 
			displayMessage(0x56da);
		return true;
		
	case 0x7bfd:
		playSound(76, 18);
		playAnimation(873);
		moveTo(240, 163);
		displayMessage(0x5837);
		playSound(77, 2);
		setLan(1, 0);
		playAnimation(874, 1);
		setOns(0, 0x68);
		inventory->remove(0x5b);
		enableObject(6);
		disableObject(1);
		return true;
	
	case 0x7ce5: //put spring on the solid ground
		playSound(5, 2);
		playAnimation(840);
		setOns(1, 0x61);
		inventory->remove(0x50);
		disableObject(2);
		enableObject(7);
		return true;

	case 0x7d1a: //captain's key + door
		if (res->dseg.get_byte(0xDBDF) <= 1) {
			playSound(5, 2);
			playAnimation(828);
			moveTo(262, 160, true);
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
			Dialog::show(scene, 0x63a5, 830);
			loadScene(7, 130, 195);
			playMusic(4);
			setLan(1, 1);

			Dialog::show(scene, 0x6406, 832);
			
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
			playAnimation(834, 0, true);
			playAnimation(835, 1, true);
			waitAnimation();
			
			setOns(0, 94);
			Dialog::show(scene, 0x65e9, 832);
			enableObject(12);
			SET_FLAG(0xdbe0, 1);
		}
		return true;
		
	case 0x7e4f: //giving magazine to captain
		playSound(5, 3);
		Dialog::show(scene, 0x66c0);
		playAnimation(852, 0, true);
		playAnimation(853, 0, true);
		displayMessage(0x5742);
		displayMessage(0x5757);
		displayMessage(0x5770);
		displayMessage(0x5782);
		displayMessage(0x5799);
		playAnimation(856, 1);
		playSound(5, 3);
		//playAnimation(854);
		Dialog::show(scene, 0x66fe);
		playAnimation(855, 1);
		moveTo(30, 181);
		disableObject(1);
		setLan(1, 0);
		SET_FLAG(0xDBDF, 3);
		SET_FLAG(0xDBF0, 1);
		loadScene(8, 155, 199);
		return true;
		
	case 0x7fbd: //using bird & bartender
		playSound(5, 3);
		playAnimation(876);
		setOns(1, 0);
		playAnimation(877, 2);
		playAnimation(880, 2, true);
		
		Dialog::show(scene, 0x6f0e, 857);
		setOns(2, 0x6a);
		reloadLan();
		playAnimation(878, 1);
		playAnimation(879, 1);
		inventory->remove(0x5c);
		enableObject(1);
		SET_FLAG(0xDBE7, 1);
		return true;
		
	case 0x8047:
		playSound(32, 5);
		playAnimation(881);
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
			playSound(5);
			displayMessage(0x53DD);
			playAnimation(810, false);
			Dialog::show(scene, 0x60BF);
			SET_FLAG(0xDBDA, 1);
		}
		return true;
		
	case 0x80c3: //show kaleydoscope to the guard
		Dialog::show(scene, 0x6811, 809);
		playSound(5, 3);
		playAnimation(849, 0, true);
		playAnimation(851, 1, true);
		waitAnimation();
		
		playAnimation(850, 1);
		reloadLan();
		inventory->add(0x53);
		inventory->remove(0x52);
		enableObject(1);
		SET_FLAG(0xDBE2, 1);
		return true;

	//Shore
	
	case 0x4d56:
		inventory->add(16);
		disableObject(2);
		setOns(0, 0);
		playSound(5);
		playAnimation(547);
		return true;
	
	
	case 0x4eb9://Pick up wrapper
		inventory->add(0x12);
		setOns(1, 0);
		playAnimation(549);
		disableObject(13);
		return true;

	case 0x5348:
		if (CHECK_FLAG(0xdb99, 1)) { //got broken paddle from boat
			displayMessage(0x351f);
		} else {
			SET_FLAG(0xDB99, 1);
			playSound(57, 6);
			playAnimation(536);
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
		playSound(34);
		playAnimation(535);
		inventory->add(11);
		disableObject(1);
		
		byte * scene_15_ons = scene->getOns(15); //patch ons for the scene 15
		scene_15_ons[0] = 0;
		
		byte f = GET_FLAG(0xDB98) + 1;
		SET_FLAG(0xDB98, f);
		if (f >= 2) {
			//disable object boat for scene 15!!
			scene->getObject(1, 15)->enabled = 0;
		}
		}
		return true;

	case 0x54b3: {
		setOns(1, 0);
		setOns(3, 0);
		playSound(33);
		playAnimation(534);
		inventory->add(10);
		disableObject(2);
		setOns(1, 10);

		byte * scene_15_ons = scene->getOns(15); //patch ons for the scene 15
		scene_15_ons[1] = 0;
		byte f = GET_FLAG(0xDB98) + 1;
		SET_FLAG(0xDB98, f);
		if (f >= 2) {
			//disable object boat for scene 15!!
			scene->getObject(1, 15)->enabled = 0;
		}
		}
		return true;
		
	case 0x5502:
		setOns(0, 0);
		loadScene(15, 115, 180, 1);
		playAnimation(568);
		playMusic(6);
		return true;

	case 0x5561://Enter lakeside house
		moveTo(81, 101);
		loadScene(19, 223, 193);
		return true;
		
	case 0x563b:
		playSound(5);
		playAnimation(561);
		setOns(1, 0);
		inventory->add(26);
		disableObject(6);
		return true;

	case 0x5756://Open car door
		playSound(11, 4);
		playAnimation(514);
		setOns(4, 8);
		setOns(2, 5);
		enableObject(14);
		enableObject(15);
		enableObject(16);
		disableObject(1);
		return true;

	case 0x5805://Enter basketball house
		playSound(70, 6);
		moveTo(161, 165);
		playAnimation(513);
		loadScene(22, 51, 180);
		return true;

	case 0x5832://Ring doorbell
		playAnimation(509);
		displayMessage(0x5dce);
		return true;
		
	case 0x58a2:
		Dialog::pop(scene, 0xdaba);
		strcpy(scene->getObject(13)->name, (const char *)res->dseg.ptr(0x92e5));
		return true;
	
	case 0x58b7://Get comb from car
		disableObject(14);
		setOns(4,0);
		playSound(5, 7);
		playAnimation(521);
		setOns(4, 0);
		inventory->add(0x6);
		return true;

	case 0x58df://Pull trunk lever in car
		SET_FLAG(0xDB94, 1);
		playSound(6, 1);
		setOns(3, 6);
		playAnimation(515);
		return true;
	
	case 0x593e://Enter annes house
		moveTo(100, 176);
		//Need to add support for the choice of music
		//After speaking with anne the default room song changes
		loadScene(23, 94, 190);
		if (CHECK_FLAG(0xdbee, 1)) {//spoken with anne
			//playMusic(6);//Play lovers music
			//i've added music changing right into scene.
		}
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
		playAnimation(601);
		moveTo(230, 179, 3);
		if (!CHECK_FLAG(0xDBA4, 1))
			displayMessage(0x37ea); //it's kinda dark here
		return true;

	case 0x5a8b:
		if (!CHECK_FLAG(0xDBAD, 1)) {
			playSound(43, 4); //grrrrrr
			setLan(1, 0);
			playAnimation(656, 1);
			setLan(1, 0xff);
			displayMessage(0x3c16);
		} else if (!CHECK_FLAG(0xDBA3, 1)) {//Dog has bone
			playSound(28, 3);
			playAnimation(596);
			setOns(1, 30);
			//loadScene(24, 100, 100);//TODO: Extend and fix the coords
			SET_FLAG(0xDBA3, 1);
			enableObject(8);
		} else {
			setOns(1, 0);
			playSound(4, 4);
			playAnimation(597);
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

	case 0x5be1://Talk to grandpa
		Dialog::pop(scene, 0xDAC4);
		return true;
	
	case 0x5c0d: //grandpa - drawers
		if (CHECK_FLAG(0xDBA7, 1)) {
			displayMessage(0x3bac);
		} else {
			if (!CHECK_FLAG(0xDB92, 1))
				Dialog::show(scene, 0x15a0); //can I search your drawers?
			
			playSound(66);
			playAnimation(631);
			inventory->add(47);
			SET_FLAG(0xDBA7, 1);
		}
		return true;
		
	case 0x5c84:
		if (CHECK_FLAG(0xDB92, 1)) {
			inventory->add(2);
			disableObject(7);
			playSound(32);
			setOns(0, 0);
			playAnimation(520);
		} else {
			Dialog::pop(scene, 0xDACE);
		}
		return true;
	
	case 0x5cf0://Exit basketball house
		playSound(88, 5);
		playAnimation(981);
		loadScene(20, 161, 165);
		return true;
		
	case 0x5d24: //getting the fan
		if (CHECK_FLAG(0xDB92, 1)) {
			setLan(2, 0);
			playSound(32);
			playAnimation(508);
			disableObject(13);
			inventory->add(7);
		} else {
			Dialog::pop(scene, 0xDAD4);
		}
		return true;
		
	case 0x5e4d: //right click on ann
		if (!CHECK_FLAG(0xDB97, 0)) {
			displayMessage(0x3d59);
		} else {
			moveTo(245, 198, 1);
			Dialog::show(scene, 0x21d7);
			SET_FLAG(0xDB97, 1);
			playSound(13);
			playAnimation(528, 1);
			playMusic(7);
			SET_FLAG(0xDBEE, 1);
			playSound(56);
			playAnimation(525);
			playSound(56);
			playAnimation(526);
			playSound(54);
			playAnimation(527);
			Dialog::show(scene, 0x2219);
			strcpy(scene->getObject(2)->name, (const char *)res->dseg.ptr(0x9820));
		}
		return true;

	case 0x5f73://Exit annes house
		//Need to fully understand what this does, there is a compare in the asm
		//I assume it is probably to do with the music
		loadScene(21, 161, 165);

		return true;
		
	case 0x5fba:
		if (CHECK_FLAG(0xDBB1, 1)) {
			displayMessage(0x4380);
		} else {
			Dialog::pop(scene, 0xDAFC);
		}
		return true;
		
	case 0x6480: //flips
		if (CHECK_FLAG(0xDB96, 1)) {
			setOns(3, 36);
			playSound(56);
			playAnimation(613);
			inventory->add(39);
			disableObject(5);
			displayMessage(0x387c);
		} else 
			displayMessage(0x3eb2);
		return true;
	
	case 0x64c4: //mask (maybe flips) :)
		if (CHECK_FLAG(0xDB96, 1)) {
			setOns(2, 35);
			playSound(63);
			playAnimation(612);
			inventory->add(40);
			disableObject(6);
		} else 
			displayMessage(0x3eb2);
		return true;

	case 0x78f5://Describe trunk
		if (CHECK_FLAG(0xdb95, 1)) {//trunk closed
			displayMessage(0x3e75);
			return true;
		}
		else {
			displayMessage(0x9080);
			return true;
		}
		
	case 0x7907://Describe car lever
		if (CHECK_FLAG(0xdb94, 1)) {//Already pulled lever?
			displayMessage(0x3e4f);
			return true;
		} else 
			return false;

	case 0x612b://climb basement ladder(exit)
		playSound(52, 10);
		playAnimation(600);
		loadScene(21, 297, 178, 3);
		return true;
	
	case 0x6351:
		if (CHECK_FLAG(0xdaca, 1)) { //cave bush is cut down
			playMusic(8);
			loadScene(26, 319, 169);
			return true;
		} else {
			displayMessage(0x3bd2);			
			return true;
		}
	
	case 0x6592: //Rake
		setOns(1, 0);
		playSound(18, 10);
		playAnimation(553);
		inventory->add(0x15);
		displayMessage(0x3605);	
		disableObject(11);
		return true;

	case 0x62d0://Get bone from under rock
		playSound(26, 6);
		setOns(0, 0);
		playAnimation(594);
		setOns(0, 29);
		displayMessage(0x463c);
		disableObject(1);
		inventory->add(36);
		playSound(5, 3);
		playAnimation(595);
		displayMessage(0x3790);
		return true;

	case 0x6411://Kick hen
		if (CHECK_FLAG(0xdb93, 1)) { //already kicked hen
			displayMessage(0x3e08);
			return true;
		} else {
			SET_FLAG(0xdb93, 1);
			playSound(30, 26);
			displayMessage(0x3dc6);
			playAnimation(500, 0, true);
			playAnimation(501, 1, true);
			waitAnimation();
			setOns(0, 1);
			enableObject(14);
			displayMessage(0x3df4);
			return true;
		}
	
	case 0x6519://Sickle
		setOns(4, 0);
		playSound(5, 11);
		playAnimation(625);
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
			playAnimation(548);
			inventory->add(0x11);
			displayMessage(0x35b2);
			return true;
		}

	case 0x663c://Feather
		setOns(0, 0);
		playSound(5, 9);
		playAnimation(511);
		inventory->add(1);
		disableObject(15);
		return true;	

	case 0x7f23://Use grenade on captains drawer
		if (CHECK_FLAG(0xDBDF, 3)) {
			playSound(5, 3);
			playAnimation(870);
			playSound(54, 15);
			playAnimation(871);
			SET_FLAG(0xDBE6, 1);
			setOns(1, 0x66);
			moveTo(224, 194, true);
			debug(0, "FIXME: add cut message: 57DF");
			inventory->remove(0x59);
		} else {
			displayMessage(0x5de2);
		}
		return true;
		
	case 0x509a:
		moveTo(203, 169);
		setOns(1, 0);
		playSound(5);
		playAnimation(543);
		inventory->add(15);
		disableObject(9);
		return true;
		
	case 0x78e0:
		moveTo(203, 169);
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
		
	case 0x8174:
		setOns(0, 0);
		playSound(5);
		playAnimation(542);
		setOns(1, 15);
		disableObject(3);
		enableObject(9);
		return true;
		
	case 0x8312: //hedgehog + plastic apple
		playSound(5);
		Dialog::show(scene, 0x3000);
		setLan(1, 0);
		playAnimation(562);
		playAnimation(563, 1);
		disableObject(6);
		displayMessage(0x363f);
		inventory->remove(27);
		inventory->add(28);
		return true;
		
	case 0x84c7:
		playSound(20);
		playAnimation(530);
		loadScene(16, 236, 95, 1);
		setOns(0, 9);
		playAnimation(531);
		playSound(36);
		playAnimation(532);
		playAnimation(533);
		playMusic(9);
		return true;
	
	case 0x8538://Sharpen sickle on well
		setOns(2, 0);
		//TODO: Remove handle sprite
		playAnimation(643);
		inventory->remove(0x2c);
		inventory->add(0x2e);
		return true;
		
	case 0x85eb:
		if (CHECK_FLAG(0xDBB0, 1)) {
			enableObject(6);
			playSound(25);
			playAnimation(559);
			setOns(1, 23);
			SET_FLAG(0xDBB0, 2);
		} else 
			displayMessage(0x3d86);
		
		return true;

	case 0x863d:
		playSound(12);
		playAnimation(554);
		inventory->remove(19);
		inventory->add(22);
		return true;

	case 0x8665:
		playSound(5);
		playAnimation(567);
		inventory->remove(12);
		inventory->add(33);
		return true;
		
	case 0x86a9: //correcting height of the pole with spanner
		if (CHECK_FLAG(0xDB92, 1)) {
			displayMessage(0x3d40);
		} else {
			SET_FLAG(0xDB92, 1);
			Dialog::show(scene, 0x0fcd);
			playSound(5);
			playAnimation(506, 1);
			playAnimation(504);
			setOns(0, 0);
			playSound(24);
			playAnimation(505);
			displayMessage(0x3cfb);
			playAnimation(507, 1);
			setOns(0, 4);
			Object * obj = scene->getObject(3);
			obj->rect.top += 20;
			obj->rect.bottom += 20;
			playSound(10);
			playAnimation(503, 1);
			setLan(1, 0, 22);
			scene->getObject(1, 22)->enabled = 0;
			scene->getObject(13, 20)->enabled = 0;
			setLan(1, 0);
			disableObject(1);
			disableObject(2);
			disableObject(14);
			disableObject(15);
			disableObject(16);
			moveTo(162, 164);
			displayMessage(0x3d01, 0xe5);
			displayMessage(0x3d20, 0xd8);
			moveTo(162, 191);
			setOns(1, 0);
			setOns(2, 0);
			setOns(3, 0);
			setOns(4, 0);
			scene->getWalkbox(0)->rect.clear();
			playSound(62);
			//patch lan, 1
			displayMessage(0x3d3a);
			scene->getObject(7)->actor_rect.left = 228;
			scene->getObject(7)->actor_rect.top = 171;
			scene->getObject(8)->actor_rect.left = 290;
			scene->getObject(8)->actor_rect.top = 171;
		}
		return true;
		
	case 0x88c9: //give flower to old lady
		if (CHECK_FLAG(0xDB9A, 1))
			return processCallback(0x890b);

		playAnimation(523, 1);
		inventory->remove(10);
		SET_FLAG(0xDB9A, 1);
		processCallback(0x88DE);
		return true;
	
	case 0x890b:
		Dialog::pop(scene, 0xDAF0);
		return true;

	case 0x8918://give flower to old lady
		if (CHECK_FLAG(0xDB9A, 1))
			return processCallback(0x890B);
		
		playAnimation(523, 1);
		inventory->remove(11);
		SET_FLAG(0xDB9A, 1);
		processCallback(0x88DE);
		return true;
		
	case 0x88de:
		playSound(5);
		Dialog::show(scene, 0x1B5F);
		playAnimation(537);
		playAnimation(538, 1);
		Dialog::show(scene, 0x1BE0);
		return true;
		
	case 0x892d:
		if (CHECK_FLAG(0xDB9B, 1))
			return processCallback(0x89aa);

		processCallback(0x8942);
		inventory->remove(10);
		SET_FLAG(0xDB9B, 1);
		return true;
	
	case 0x8942:
		playSound(5);
		Dialog::show(scene, 0x2293);
		playAnimation(540);
		playAnimation(539, 1);
		Dialog::show(scene, 0x24b1);
		Dialog::show(scene, 0x24d7);
		Dialog::show(scene, 0x2514);
		moveTo(scene->getPosition().x, scene->getPosition().y + 1);
		Dialog::show(scene, 0x2570);
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
		playSound(5);
		Dialog::show(scene, 0x2634);
		playAnimation(555);
		playAnimation(556, 1);
		playAnimation(557);
		playAnimation(558, 1);
		Dialog::show(scene, 0x2971);
		inventory->add(24);
		return true;
		
	case 0x8a22:
		playAnimation(560);
		playSound(45);
		inventory->remove(26);
		inventory->add(27);
		Dialog::show(scene, 0x1ecd);
		Dialog::show(scene, 0x1f09);
		SET_FLAG(0xDBB1, 1);
		return true;
		
	case 0x8b82: //use fan on laundry
		setOns(0, 0);
		playSound(5);
		playAnimation(602);
		displayMessage(0x464a);
		playAnimation(603);
		setOns(0, 27);
		SET_FLAG(0xDBA5, 1);
		return true;

	case 0x8bfc://Give bone to dog
		displayMessage(0x3c31);
		playSound(5, 3);
		playAnimation(657, 0, true);
		playAnimation(658, 1, true);
		waitAnimation();
		
		playAnimation(659, 1);
		
		displayMessage(0x3c3d);
		inventory->remove(36);
		SET_FLAG(0xDBAD, 1);
		//TODO:Adjust Walkboxes
		return true;
	
	case 0x8c6e://Use car jack on rock
		playSound(5, 3);
		playAnimation(592);
		playSound(1, 5);
		playAnimation(593);
		setOns(0, 28);
		disableObject(35);
		enableObject(1);
		inventory->remove(35);
		return true;

	case 0x8cc8://Cut bush with sickle
		moveTo(Common::Point(234, 152));
		setOns(1, 45);
		playAnimation(645);
		playAnimation(646);
		playAnimation(647);
		SET_FLAG(0xdaca, 1);
		inventory->remove(0x2e);
		disableObject(2);
		return true;
		
	case 0x8f1d:
		Dialog::show(scene, 0x2dd6);
		displayMessage(0x34c7);
		setLan(3, 0);
		setLan(4, 0);
		playSound(56);
		playAnimation(516);
		playAnimation(517, 2);
		playAnimation(518, 3);
		disableObject(2);
		disableObject(3);
		inventory->remove(2);
		SET_FLAG(0xDB96, 1);
		return true;
		
	case 0x9054: //mouse hole
		if (CHECK_FLAG(0xDBAB, 1)) {
			displayMessage(0x3c0b);
		} else {
			playSound(5);
			moveTo(99, scene->getPosition().y);
			playAnimation(632);
			setOns(5, 40);
			moveTo(239, 139, true);
			playAnimation(633);
			SET_FLAG(0xDBA8, 1);
			inventory->remove(47);
			if (!CHECK_FLAG(0xDBAA, 1)) {
				SET_FLAG(0xDBAA, 1);
				displayMessage(0x3b8b);
			}
		}
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


	case 0x6be1: //handle to the bathroom
		if (!CHECK_FLAG(0xDBD9, 1)) {
			displayMessage(0x5326); //i'd better catch johnny
		} else {
			playSound(88);
			playAnimation(808);
			loadScene(36, 41, 195);
		}
		return true;

	case 0x6bad:
		playAnimation(971);
		loadScene(32, 139, 199);
		return true;

	case 0x9c79:
		if (scene->getId() != 36) {
			displayMessage(0x52a9);
		} else if (CHECK_FLAG(0xDBF1, 1)) {
			displayMessage(0x52F6);
		} else {
			SET_FLAG(0xDBF1, 1);
			moveTo(102, 195);
			playAnimation(794);
			//scene->getWalkbox(0)->rect.left = 0;
			//scene->getWalkbox(0)->rect.top = 0;
			moveTo(151, 197);
			playAnimation(795);
			moveTo(186, 198, true);
			moveTo(220, 198);
			scene->getWalkbox(0)->rect.top = 200;

			Object * obj = scene->getObject(1);
			obj->actor_rect.left = obj->actor_rect.right = 270;
			obj->actor_rect.top = obj->actor_rect.bottom = 193;
			obj->actor_orientation = 2;

			obj = scene->getObject(3);
			obj->actor_rect.left = obj->actor_rect.right = 254;
			obj->actor_rect.top = obj->actor_rect.bottom = 193;
			obj->actor_orientation = 1;
			SET_FLAG(0xDBD7, 1);
		}
		return true;
		
	case 0x66b5:
		playAnimation(969);
		playSound(89);
		loadScene(33, 319, 181);
		return true;
	}

	//unimplemented callback  :(
	for(uint i = 0; i < 32; ++i) {
		debug(0, "code[%u] = %02x ", i, code[i]);
	}
	//error("invalid callback %04x called", addr);
	debug(0, "invalid callback %04x called", addr);
	return true;
}
