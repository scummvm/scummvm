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
 */

#include "bladerunner/script/vk.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

bool ScriptVK::SCRIPT_VK_DLL_Initialize(int a1) {
	VK_Add_Question(0, 7400, -1);
	VK_Add_Question(0, 7405, -1);
	VK_Add_Question(0, 7410, -1);
	VK_Add_Question(0, 7415, -1);
	VK_Add_Question(0, 7420, -1);
	VK_Add_Question(0, 7425, -1);
	if (a1 == 6 || a1 == 3) {
		VK_Add_Question(0, 7430, -1);
	}
	VK_Add_Question(0, 7435, -1);
	VK_Add_Question(0, 7440, -1);
	VK_Add_Question(0, 7445, -1);
	VK_Add_Question(0, 7450, -1);
	VK_Add_Question(0, 7455, -1);
	VK_Add_Question(0, 7460, -1);
	VK_Add_Question(0, 7465, -1);
	VK_Add_Question(0, 7470, -1);
	VK_Add_Question(1, 7475, -1);
	VK_Add_Question(1, 7480, -1);
	VK_Add_Question(1, 7485, -1);
	VK_Add_Question(1, 7490, -1);
	VK_Add_Question(1, 7495, -1);
	VK_Add_Question(1, 7515, -1);
	VK_Add_Question(1, 7525, -1);
	VK_Add_Question(1, 7535, -1);
	VK_Add_Question(1, 7540, -1);
	VK_Add_Question(1, 7550, -1);
	VK_Add_Question(1, 7565, -1);
	VK_Add_Question(1, 7580, -1);
	VK_Add_Question(1, 7585, -1);
	VK_Add_Question(1, 7595, -1);
	VK_Add_Question(1, 7600, -1);
	VK_Add_Question(2, 7605, -1);
	VK_Add_Question(2, 7620, -1);
	VK_Add_Question(2, 7635, -1);
	VK_Add_Question(2, 7670, -1);
	VK_Add_Question(2, 7680, -1);
	VK_Add_Question(2, 7690, -1);
	VK_Add_Question(2, 7705, -1);
	VK_Add_Question(2, 7740, -1);
	VK_Add_Question(2, 7750, -1);
	VK_Add_Question(2, 7770, -1);
	switch (a1) {
	default:
		return false;
	case 3:
	case 6:
	case 11:
	case 14:
	case 15:
		return true;

	}
}

void ScriptVK::SCRIPT_VK_DLL_Calibrate(int a1) {
	if (unknown1 == 0) {
		VK_Play_Speech_Line(0, 7370, 0.5f);
		VK_Play_Speech_Line(0, 7385, 0.5f);
		sub_40A300(a1, 7385);
	} else if (unknown1 == 1) {
		VK_Play_Speech_Line(0, 7390, 0.5f);
		sub_40A350(a1, 7390);
	} else if (unknown1 == 2) {
		VK_Play_Speech_Line(0, 7395, 0.5f);
		sub_40A3A0(a1, 7395);
	}
	unknown1++;
	if (unknown1 > 3) //bug?
	{
		unknown1 = 0;
	}
}

bool ScriptVK::SCRIPT_VK_DLL_Begin_Test() {
	unknown2 = 0;
	return false;
}

void ScriptVK::SCRIPT_VK_DLL_McCoy_Asks_Question(int a1, int a2) {
	switch (a2) {
	case 7400:
		VK_Play_Speech_Line(0, 7400, 0.5f);
		break;
	case 7405:
		VK_Play_Speech_Line(0, 7405, 0.5f);
		break;
	case 7410:
		VK_Play_Speech_Line(0, 7410, 0.5f);
		break;
	case 7415:
		VK_Play_Speech_Line(0, 7415, 0.5f);
		break;
	case 7420:
		VK_Play_Speech_Line(0, 7420, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(0, 7425, 0.5f);
		break;
	case 7430:
		VK_Play_Speech_Line(0, 7430, 0.5f);
		break;
	case 7435:
		VK_Play_Speech_Line(0, 7435, 0.5f);
		break;
	case 7440:
		VK_Play_Speech_Line(0, 7440, 0.5f);
		break;
	case 7445:
		VK_Play_Speech_Line(0, 7445, 0.5f);
		break;
	case 7450:
		VK_Play_Speech_Line(0, 7450, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(0, 7455, 0.5f);
		break;
	case 7460:
		VK_Play_Speech_Line(0, 7460, 0.5f);
		break;
	case 7465:
		VK_Play_Speech_Line(0, 7465, 0.5f);
		break;
	case 7470:
		VK_Play_Speech_Line(0, 7470, 0.5f);
		break;
	case 7475:
		VK_Play_Speech_Line(0, 7475, 0.5f);
		break;
	case 7480:
		VK_Play_Speech_Line(0, 7480, 0.5f);
		break;
	case 7485:
		VK_Play_Speech_Line(0, 7485, 0.5f);
		break;
	case 7490:
		VK_Play_Speech_Line(0, 7490, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(0, 7495, 0.5f);
		VK_Play_Speech_Line(0, 7500, 0.5f);
		VK_Play_Speech_Line(0, 7505, 0.5f);
		VK_Play_Speech_Line(0, 7510, 0.5f);
		break;
	case 7515:
		VK_Play_Speech_Line(0, 7515, 0.5f);
		VK_Play_Speech_Line(0, 7520, 0.5f);
		break;
	case 7525:
		VK_Play_Speech_Line(0, 7525, 0.5f);
		VK_Play_Speech_Line(0, 7530, 0.5f);
		break;
	case 7535:
		VK_Play_Speech_Line(0, 7535, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(0, 7540, 0.5f);
		VK_Play_Speech_Line(0, 7545, 0.5f);
		break;
	case 7550:
		VK_Play_Speech_Line(0, 7550, 0.5f);
		VK_Play_Speech_Line(0, 7555, 0.5f);
		VK_Play_Speech_Line(0, 7560, 0.5f);
		break;
	case 7565:
		VK_Play_Speech_Line(0, 7565, 0.5f);
		if (a1 != 14) {
			VK_Play_Speech_Line(0, 7570, 0.5f);
			VK_Play_Speech_Line(0, 7575, 0.5f);
		}
		break;
	case 7580:
		VK_Play_Speech_Line(0, 7580, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(0, 7585, 0.5f);
		if (a1 != 6 && a1 != 14) {
			VK_Play_Speech_Line(0, 7590, 0.5f);
		}
		break;
	case 7595:
		VK_Play_Speech_Line(0, 7595, 0.5f);
		break;
	case 7600:
		VK_Play_Speech_Line(0, 7600, 0.5f);
		break;
	case 7605:
		VK_Play_Speech_Line(0, 7605, 0.5f);
		VK_Play_Speech_Line(0, 7610, 0.5f);
		VK_Play_Speech_Line(0, 7615, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(0, 7620, 0.5f);
		VK_Play_Speech_Line(0, 7625, 0.5f);
		if (a1 != 14) {
			if (a1 == 3 && Game_Flag_Query(47)) {
				VK_Play_Speech_Line(3, 2330, 0.5f);
				VK_Play_Speech_Line(0, 7880, 0.5f);
			}
			VK_Play_Speech_Line(0, 7630, 0.5f);
		}
		break;
	case 7635:
		VK_Play_Speech_Line(0, 7635, 0.5f);
		VK_Play_Speech_Line(0, 7640, 0.5f);
		if (a1 != 11 && a1 != 14) {
			VK_Play_Speech_Line(0, 7645, 0.5f);
			VK_Play_Speech_Line(0, 7650, 0.5f);
			if (a1 != 6) {
				VK_Play_Speech_Line(0, 7655, 0.5f);
				VK_Play_Speech_Line(0, 7660, 0.5f);
				VK_Play_Speech_Line(0, 7665, 0.5f);
			}
		}
		break;
	case 7670:
		VK_Play_Speech_Line(0, 7670, 0.5f);
		VK_Play_Speech_Line(0, 7675, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(0, 7680, 0.5f);
		VK_Play_Speech_Line(0, 7685, 0.5f);
		break;
	case 7690:
		VK_Play_Speech_Line(0, 7690, 0.5f);
		if (a1 != 11) {
			VK_Play_Speech_Line(0, 7695, 0.5f);
			VK_Play_Speech_Line(0, 7700, 0.5f);
		}
		break;
	case 7705:
		VK_Play_Speech_Line(0, 7705, 0.5f);
		VK_Play_Speech_Line(0, 7710, 0.5f);
		VK_Play_Speech_Line(0, 7715, 0.5f);
		if (a1 != 11 && a1 != 14) {
			VK_Play_Speech_Line(0, 7720, 0.5f);
			VK_Play_Speech_Line(0, 7725, 0.5f);
			if (a1 != 6) {
				if (a1 == 3) {
					VK_Play_Speech_Line(3, 2490, 0.5f);
				}
				VK_Play_Speech_Line(0, 7730, 0.5f);
				VK_Play_Speech_Line(0, 7735, 0.5f);
			}
		}
		break;
	case 7740:
		VK_Play_Speech_Line(0, 7740, 0.5f);
		VK_Play_Speech_Line(0, 7745, 0.5f);
		break;
	case 7750:
		VK_Play_Speech_Line(0, 7750, 0.5f);
		VK_Play_Speech_Line(0, 7755, 0.5f);
		if (a1 == 3) {
			VK_Play_Speech_Line(3, 2570, 0.5f);
		}
		VK_Play_Speech_Line(0, 7760, 0.5f);
		VK_Play_Speech_Line(0, 7765, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(0, 7770, 0.5f);
		if (a1 == 3) {
			VK_Play_Speech_Line(3, 2620, 0.5f);
		}
		VK_Play_Speech_Line(0, 7775, 0.5f);
		VK_Play_Speech_Line(0, 7780, 0.5f);
		break;
	}
	if (++unknown2 >= 10) {
		VK_Subject_Reacts(5, 0, 0, 100);
	}
}

void ScriptVK::SCRIPT_VK_DLL_Question_Asked(int a1, int a2) {
	switch (a1) {
	case 15:
		sub_407CF8(a2);
		break;
	case 14:
		sub_40897C(a2);
		break;
	case 11:
		sub_404B44(a2);
		break;
	case 6:
		sub_402604(a2);
		break;
	case 3:
		sub_406088(a2);
		break;
	}
}

void ScriptVK::SCRIPT_VK_DLL_Shutdown(int a1, signed int a2, signed int a3) {
	if (a2 > 79 && a3 > 79) {
		VK_Play_Speech_Line(39, 450, 0.5f);
	} else if (a3 > 79) {
		VK_Play_Speech_Line(39, 420, 0.5f);
		VK_Play_Speech_Line(39, 430, 0.5f);
		switch (a1) {
		case 15:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKRunciterReplicant, 1, -1);
			break;
		case 14:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKBobGorskyReplicant, 1, -1);
			break;
		case 11:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKGrigorianReplicant, 1, -1);
			break;
		case 6:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKLucyReplicant, 1, -1);
			break;
		case 3:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKDektoraReplicant, 1, -1);
			break;
		}
	} else if (a2 > 79) {
		VK_Play_Speech_Line(39, 420, 0.5f);
		VK_Play_Speech_Line(39, 440, 0.5f);
		switch (a1) {
		case 15:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKRunciterHuman, 1, -1);
			break;
		case 14:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKBobGorskyHuman, 1, -1);
			break;
		case 11:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKGrigorianHuman, 1, -1);
			break;
		case 6:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKLucyHuman, 1, -1);
			break;
		case 3:
			Actor_Clue_Acquire(kActorMcCoy, kClueVKDektoraHuman, 1, -1);
			break;
		}
	}
	VK_Play_Speech_Line(39, 460, 0.5f);
}

void ScriptVK::sub_402604(int a1) {
	switch (a1) {
	case 7385:
		VK_Subject_Reacts(40, 0, 0, 0);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(6, 1240, 0.5f);
		break;
	case 7390:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1250, 0.5f);
			VK_Subject_Reacts(40, 0, 2, 5);
			VK_Play_Speech_Line(6, 1260, 0.5f);
			VK_Eye_Animates(3);
		} else {
			VK_Play_Speech_Line(6, 1270, 0.5f);
			VK_Subject_Reacts(40, 3, 0, 2);
		}
		break;
	case 7395:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1280, 0.5f);
			VK_Subject_Reacts(40, 0, 0, 0);
		} else {
			VK_Subject_Reacts(40, 0, 0, 0);
			VK_Play_Speech_Line(6, 1280, 0.5f);
		}
		break;
	case 7400:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, 0, 8, -5);
		} else {
			VK_Subject_Reacts(30, 9, 0, -10);
		}
		VK_Play_Speech_Line(6, 1300, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7405:
		VK_Play_Speech_Line(6, 1310, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(50, 1, 11, 5);
		} else {
			VK_Subject_Reacts(60, 11, 1, 5);
		}
		VK_Play_Speech_Line(6, 1320, 0.5f);
		break;
	case 7410:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, 1, 10, -5);
			VK_Eye_Animates(2);
		} else {
			VK_Subject_Reacts(40, 10, -2, -5);
		}
		VK_Play_Speech_Line(6, 1330, 0.5f);
		break;
	case 7415:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1340, 0.5f);
			VK_Subject_Reacts(50, 1, 11, -5);
			VK_Play_Speech_Line(0, 7935, 0.5f);
			VK_Play_Speech_Line(6, 1350, 0.5f);
		} else {
			VK_Play_Speech_Line(6, 1360, 0.5f);
			VK_Subject_Reacts(20, 9, -2, -5);
		}
		break;
	case 7420:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(6, 1370, 0.5f);
		VK_Play_Speech_Line(0, 8000, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(50, 1, 11, -8);
		} else {
			VK_Subject_Reacts(60, 11, -2, -8);
		}
		VK_Play_Speech_Line(6, 1390, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(6, 1400, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, -2, 9, -2);
		} else {
			VK_Subject_Reacts(20, 9, -2, -2);
		}
		VK_Play_Speech_Line(6, 1410, 0.5f);
		break;
	case 7430:
		VK_Play_Speech_Line(6, 1420, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, -1, 9, -3);
		} else {
			VK_Subject_Reacts(30, 9, -1, -3);
		}
		VK_Play_Speech_Line(6, 1430, 0.5f);
		VK_Play_Speech_Line(0, 7940, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7435:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1440, 0.5f);
			VK_Subject_Reacts(30, 5, 10, 2);
		} else {
			VK_Subject_Reacts(30, 9, 3, 2);
			VK_Play_Speech_Line(6, 1450, 0.5f);
		}
		break;
	case 7440:
		VK_Play_Speech_Line(6, 1460, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, 5, 10, 2);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(20, 9, -3, 2);
		}
		VK_Play_Speech_Line(6, 1470, 0.5f);
		break;
	case 7445:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1480, 0.5f);
			VK_Play_Speech_Line(0, 7940, 0.5f);
			VK_Subject_Reacts(50, 4, 11, 10);
			VK_Play_Speech_Line(6, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(30, 9, -1, -2);
			VK_Play_Speech_Line(6, 1510, 0.5f);
			VK_Play_Speech_Line(0, 7945, 0.5f);
			VK_Play_Speech_Line(6, 1520, 0.5f);
		}
		break;
	case 7450:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, 3, 9, -6);
		} else {
			VK_Subject_Reacts(30, 9, 4, -6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(6, 1530, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(6, 1540, 0.5f);
		VK_Play_Speech_Line(0, 7950, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(10, 1, 8, -5);
		} else {
			VK_Subject_Reacts(10, 9, -1, -2);
		}
		VK_Play_Speech_Line(6, 1550, 0.5f);
		break;
	case 7460:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(30, 1, 10, -5);
		} else {
			VK_Subject_Reacts(30, 9, 2, -5);
		}
		VK_Play_Speech_Line(6, 1560, 0.5f);
		VK_Play_Speech_Line(0, 7955, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(6, 1570, 0.5f);
		VK_Play_Speech_Line(6, 1580, 0.5f);
		break;
	case 7465:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, -1, 11, 2);
			VK_Play_Speech_Line(6, 1590, 0.5f);
		} else {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(6, 1590, 0.5f);
			VK_Subject_Reacts(20, 9, 2, -8);
		}
		break;
	case 7470:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 1610, 0.5f);
			VK_Subject_Reacts(20, 3, 9, -5);
			VK_Play_Speech_Line(6, 1620, 0.5f);
		} else {
			VK_Subject_Reacts(30, 9, 0, -5);
			VK_Play_Speech_Line(6, 1630, 0.5f);
			VK_Play_Speech_Line(6, 1640, 0.5f);
			VK_Play_Speech_Line(0, 7960, 0.5f);
			VK_Play_Speech_Line(6, 1650, 0.5f);
		}
		break;
	case 7475:
		if (Game_Flag_Query(46)) {
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(6, 1660, 0.5f);
			VK_Play_Speech_Line(0, 7965, 0.5f);
			VK_Subject_Reacts(70, -3, 13, 10);
			VK_Play_Speech_Line(6, 1670, 0.5f);
		} else {
			VK_Play_Speech_Line(6, 1680, 0.5f);
			VK_Subject_Reacts(60, 13, -4, 5);
			VK_Play_Speech_Line(6, 1690, 0.5f);
			VK_Play_Speech_Line(0, 7970, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(6, 1700, 0.5f);
			VK_Play_Speech_Line(6, 1710, 0.5f);
		}
		break;
	case 7480:
		VK_Play_Speech_Line(6, 1720, 0.5f);
		VK_Play_Speech_Line(0, 7975, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, 2, 10, 7);
		} else {
			VK_Subject_Reacts(50, 12, 3, 7);
		}
		VK_Play_Speech_Line(6, 1730, 0.5f);
		VK_Play_Speech_Line(0, 7980, 0.5f);
		break;
	case 7485:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, -2, 10, 6);
		} else {
			VK_Subject_Reacts(30, 10, -2, 6);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(6, 1740, 0.5f);
		break;
	case 7490:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(30, 3, 11, 9);
		} else {
			VK_Subject_Reacts(40, 11, 1, 8);
		}
		VK_Play_Speech_Line(6, 1750, 0.5f);
		VK_Play_Speech_Line(0, 7985, 0.5f);
		VK_Play_Speech_Line(0, 7990, 0.5f);
		VK_Play_Speech_Line(6, 1760, 0.5f);
		break;
	case 7495:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(10, 2, 8, 5);
		} else {
			VK_Subject_Reacts(30, 10, -1, 5);
		}
		VK_Play_Speech_Line(6, 1770, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(6, 1780, 0.5f);
		break;
	case 7515:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(50, -1, 12, 5);
			VK_Play_Speech_Line(6, 1790, 0.5f);
			VK_Play_Speech_Line(0, 7995, 0.5f);
			VK_Play_Speech_Line(6, 1800, 0.5f);
		} else {
			VK_Subject_Reacts(30, 10, 0, 3);
			VK_Play_Speech_Line(6, 1810, 0.5f);
		}
		break;
	case 7525:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, -4, 9, 5);
			VK_Eye_Animates(3);
		} else {
			VK_Subject_Reacts(40, 11, -3, 7);
			VK_Eye_Animates(2);
		}
		VK_Play_Speech_Line(6, 1820, 0.5f);
		break;
	case 7535:
		VK_Play_Speech_Line(6, 1830, 0.5f);
		VK_Play_Speech_Line(0, 8000, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(10, 1, 8, 0);
		} else {
			VK_Subject_Reacts(20, 9, -1, 0);
		}
		VK_Play_Speech_Line(6, 1840, 0.5f);
		VK_Play_Speech_Line(6, 1850, 0.5f);
		VK_Play_Speech_Line(0, 8005, 0.5f);
		break;
	case 7540:
		if (Game_Flag_Query(46)) {
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(6, 1860, 0.5f);
			VK_Play_Speech_Line(0, 8010, 0.5f);
			VK_Subject_Reacts(50, -2, 12, 5);
			VK_Play_Speech_Line(6, 1870, 0.5f);
		} else {
			VK_Subject_Reacts(60, 12, -2, 5);
			VK_Play_Speech_Line(6, 1880, 0.5f);
		}
		break;
	case 7550:
		VK_Play_Speech_Line(6, 1890, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, 2, 9, -1);
		} else {
			VK_Subject_Reacts(10, 8, -1, -2);
		}
		VK_Play_Speech_Line(6, 1900, 0.5f);
		VK_Play_Speech_Line(0, 8015, 0.5f);
		break;
	case 7565:
		VK_Play_Speech_Line(6, 1910, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(30, -2, 10, 8);
		} else {
			VK_Subject_Reacts(20, 9, -3, 6);
		}
		VK_Play_Speech_Line(6, 1920, 0.5f);
		VK_Play_Speech_Line(0, 8020, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7580:
		VK_Play_Speech_Line(6, 1930, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(20, 5, 10, -1);
		} else {
			VK_Subject_Reacts(30, 10, 3, 0);
		}
		VK_Play_Speech_Line(0, 8025, 0.5f);
		VK_Play_Speech_Line(6, 1940, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(6, 1950, 0.5f);
		VK_Play_Speech_Line(0, 8030, 0.5f);
		VK_Play_Speech_Line(6, 1960, 0.5f);
		VK_Play_Speech_Line(0, 8035, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(6, 1970, 0.5f);
		VK_Play_Speech_Line(0, 7590, 0.5f);
		VK_Play_Speech_Line(6, 1980, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, 1, 11, 5);
		} else {
			VK_Subject_Reacts(50, 12, -3, 5);
		}
		VK_Play_Speech_Line(6, 1990, 0.5f);
		VK_Play_Speech_Line(0, 8040, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(6, 2000, 0.5f);
		break;
	case 7595:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 2010, 0.5f);
			VK_Subject_Reacts(30, -2, 10, 5);
			VK_Play_Speech_Line(6, 2020, 0.5f);
			VK_Play_Speech_Line(0, 8045, 0.5f);
			VK_Play_Speech_Line(6, 2030, 0.5f);
		} else {
			VK_Subject_Reacts(60, 12, -3, 7);
			VK_Play_Speech_Line(6, 2040, 0.5f);
		}
		break;
	case 7600:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(75, 4, 13, 15);
		} else {
			VK_Subject_Reacts(60, 12, -2, 10);
		}
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(6, 2050, 0.5f);
		VK_Play_Speech_Line(0, 8050, 0.5f);
		VK_Subject_Reacts(90, 0, 0, 0);
		VK_Play_Speech_Line(6, 2060, 0.5f);
		break;
	case 7605:
		VK_Play_Speech_Line(6, 2070, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(30, 0, 11, 12);
		} else {
			VK_Subject_Reacts(50, 10, -3, 15);
		}
		VK_Play_Speech_Line(6, 2080, 0.5f);
		break;
	case 7620:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(60, -2, 14, 12);
		} else {
			VK_Subject_Reacts(70, 9, -1, 10);
		}
		VK_Play_Speech_Line(6, 2090, 0.5f);
		VK_Play_Speech_Line(0, 8055, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(6, 2100, 0.5f);
		break;
	case 7635:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(6, 2110, 0.5f);
		VK_Play_Speech_Line(0, 7655, 0.5f);
		VK_Play_Speech_Line(0, 7660, 0.5f);
		VK_Play_Speech_Line(0, 7665, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(80, 5, 14, 25);
		} else {
			VK_Subject_Reacts(70, 9, -2, 20);
		}
		VK_Play_Speech_Line(6, 2120, 0.5f);
		break;
	case 7670:
		if (Game_Flag_Query(46)) {
			VK_Play_Speech_Line(6, 2130, 0.5f);
			VK_Subject_Reacts(30, -3, 11, 8);
			VK_Play_Speech_Line(6, 2140, 0.5f);
		} else {
			VK_Subject_Reacts(20, 10, 0, 5);
			VK_Play_Speech_Line(6, 2150, 0.5f);
			VK_Play_Speech_Line(6, 2160, 0.5f);
		}
		break;
	case 7680:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(6, 2170, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(65, 1, 6, 5);
		} else {
			VK_Subject_Reacts(50, 10, 3, 4);
		}
		VK_Play_Speech_Line(6, 2180, 0.5f);
		break;
	case 7690:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, 0, 12, 0);
		} else {
			VK_Subject_Reacts(50, 13, 0, 0);
		}
		VK_Play_Speech_Line(6, 2190, 0.5f);
		VK_Play_Speech_Line(0, 8060, 0.5f);
		VK_Play_Speech_Line(6, 2200, 0.5f);
		VK_Subject_Reacts(30, -4, -4, -5);
		VK_Play_Speech_Line(6, 2210, 0.5f);
		break;
	case 7705:
		if (Game_Flag_Query(46)) {
			VK_Eye_Animates(3);
			VK_Subject_Reacts(30, 0, 0, 0);
			VK_Play_Speech_Line(6, 2220, 0.5f);
			VK_Play_Speech_Line(0, 7730, 0.5f);
			VK_Play_Speech_Line(0, 7735, 0.5f);
			VK_Subject_Reacts(50, 2, 10, 12);
			VK_Play_Speech_Line(6, 2230, 0.5f);
			VK_Play_Speech_Line(0, 8065, 0.5f);
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(6, 2240, 0.5f);
			VK_Play_Speech_Line(0, 8070, 0.5f);
			VK_Play_Speech_Line(0, 8075, 0.5f);
		} else {
			VK_Eye_Animates(2);
			VK_Subject_Reacts(50, 0, 0, 0);
			VK_Play_Speech_Line(6, 2250, 0.5f);
			VK_Play_Speech_Line(0, 7730, 0.5f);
			VK_Play_Speech_Line(0, 7735, 0.5f);
			VK_Eye_Animates(3);
			VK_Subject_Reacts(60, 12, 2, 12);
			VK_Play_Speech_Line(6, 2230, 0.5f);
			VK_Play_Speech_Line(6, 2270, 0.5f);
			VK_Play_Speech_Line(0, 8080, 0.5f);
			VK_Play_Speech_Line(6, 2280, 0.5f);
		}
		break;
	case 7740:
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(40, -3, -1, 3);
		} else {
			VK_Subject_Reacts(50, -1, -3, 3);
		}
		VK_Play_Speech_Line(6, 2290, 0.5f);
		VK_Play_Speech_Line(0, 8085, 0.5f);
		VK_Play_Speech_Line(6, 2300, 0.5f);
		VK_Play_Speech_Line(0, 8090, 0.5f);
		VK_Subject_Reacts(20, 2, 2, 0);
		VK_Play_Speech_Line(6, 2310, 0.5f);
		VK_Play_Speech_Line(6, 2320, 0.5f);
		break;
	case 7750:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(60, 4, 10, 15);
		} else {
			VK_Subject_Reacts(80, 12, -3, 18);
		}
		VK_Play_Speech_Line(6, 2330, 0.5f);
		VK_Play_Speech_Line(0, 8095, 0.5f);
		VK_Play_Speech_Line(6, 2340, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(6, 2350, 0.5f);
		if (Game_Flag_Query(46)) {
			VK_Subject_Reacts(90, -3, -1, 12);
		} else {
			VK_Eye_Animates(2);
			VK_Subject_Reacts(90, 13, -4, 12);
		}
		VK_Play_Speech_Line(6, 2360, 0.5f);
		break;
	}
}

void ScriptVK::sub_404B44(int a1) {
	switch (a1) {
	case 7385:
		VK_Subject_Reacts(20, 0, 0, 5);
		VK_Play_Speech_Line(11, 440, 0.5f);
		VK_Play_Speech_Line(0, 8100, 0.5f);
		VK_Play_Speech_Line(11, 450, 0.5f);
		VK_Play_Speech_Line(0, 8105, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(20, 0, 0, 3);
		VK_Play_Speech_Line(11, 460, 0.5f);
		break;
	case 7395:
		VK_Subject_Reacts(20, 0, 0, 7);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 470, 0.5f);
		VK_Play_Speech_Line(11, 480, 0.5f);
		VK_Play_Speech_Line(0, 8105, 0.5f);
		break;
	case 7400:
		VK_Subject_Reacts(10, 8, 0, -5);
		VK_Play_Speech_Line(11, 490, 0.5f);
		VK_Play_Speech_Line(11, 500, 0.5f);
		break;
	case 7405:
		VK_Play_Speech_Line(11, 510, 0.5f);
		VK_Subject_Reacts(30, 10, 7, 0);
		VK_Play_Speech_Line(11, 520, 0.5f);
		VK_Play_Speech_Line(0, 8115, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(20, 12, -3, -3);
		VK_Play_Speech_Line(11, 530, 0.5f);
		VK_Play_Speech_Line(11, 540, 0.5f);
		break;
	case 7415:
		VK_Play_Speech_Line(11, 550, 0.5f);
		VK_Subject_Reacts(25, 7, 8, 8);
		VK_Play_Speech_Line(11, 560, 0.5f);
		VK_Play_Speech_Line(0, 8120, 0.5f);
		VK_Play_Speech_Line(0, 8125, 0.5f);
		VK_Subject_Reacts(75, 8, 0, 0);
		VK_Play_Speech_Line(11, 570, 0.5f);
		break;
	case 7420:
		VK_Subject_Reacts(10, 6, 8, -5);
		VK_Play_Speech_Line(11, 580, 0.5f);
		break;
	case 7425:
		VK_Subject_Reacts(30, 8, -2, 5);
		VK_Play_Speech_Line(11, 590, 0.5f);
		break;
	case 7430:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(45, 8, 7, 15);
		VK_Play_Speech_Line(11, 600, 0.5f);
		VK_Play_Speech_Line(0, 8130, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 610, 0.5f);
		VK_Play_Speech_Line(0, 8135, 0.5f);
		VK_Play_Speech_Line(0, 8140, 0.5f);
		break;
	case 7435:
		VK_Play_Speech_Line(11, 620, 0.5f);
		VK_Subject_Reacts(15, 8, -3, -5);
		VK_Play_Speech_Line(11, 630, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(25, 4, 7, -3);
		VK_Play_Speech_Line(11, 640, 0.5f);
		break;
	case 7445:
		VK_Subject_Reacts(30, 8, 3, 5);
		VK_Play_Speech_Line(11, 650, 0.5f);
		break;
	case 7450:
		VK_Play_Speech_Line(11, 660, 0.5f);
		VK_Subject_Reacts(35, 4, 8, 3);
		VK_Play_Speech_Line(0, 8145, 0.5f);
		VK_Play_Speech_Line(11, 670, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(11, 680, 0.5f);
		VK_Subject_Reacts(20, 7, -3, -5);
		VK_Play_Speech_Line(11, 690, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(15, 4, 8, 0);
		VK_Play_Speech_Line(11, 700, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(5, 7, -4, -7);
		VK_Play_Speech_Line(11, 710, 0.5f);
		break;
	case 7470:
		VK_Subject_Reacts(30, 4, 8, -2);
		VK_Play_Speech_Line(11, 720, 0.5f);
		break;
	case 7475:
		VK_Subject_Reacts(15, 8, 2, 5);
		VK_Play_Speech_Line(11, 730, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(20, 9, 4, 8);
		VK_Play_Speech_Line(11, 740, 0.5f);
		VK_Play_Speech_Line(11, 750, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(15, 8, 8, 5);
		VK_Play_Speech_Line(11, 760, 0.5f);
		break;
	case 7490:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(30, 7, -4, 12);
		VK_Play_Speech_Line(11, 770, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(11, 780, 0.5f);
		VK_Subject_Reacts(30, 9, 6, 5);
		VK_Play_Speech_Line(11, 790, 0.5f);
		break;
	case 7515:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 800, 0.5f);
		VK_Play_Speech_Line(0, 8150, 0.5f);
		VK_Play_Speech_Line(0, 8155, 0.5f);
		VK_Subject_Reacts(30, 9, -5, 12);
		VK_Play_Speech_Line(11, 810, 0.5f);
		VK_Play_Speech_Line(0, 8160, 0.5f);
		break;
	case 7525:
		VK_Subject_Reacts(30, 8, 5, 8);
		VK_Play_Speech_Line(11, 820, 0.5f);
		break;
	case 7535:
		VK_Subject_Reacts(20, 9, 2, 4);
		VK_Play_Speech_Line(11, 830, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(11, 840, 0.5f);
		VK_Subject_Reacts(20, 5, 1, 8);
		break;
	case 7550:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(11, 850, 0.5f);
		VK_Play_Speech_Line(0, 8165, 0.5f);
		VK_Subject_Reacts(60, 6, 2, 15);
		VK_Play_Speech_Line(11, 860, 0.5f);
		VK_Play_Speech_Line(0, 8170, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7565:
		VK_Subject_Reacts(40, 8, 8, 12);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 870, 0.5f);
		VK_Play_Speech_Line(0, 8175, 0.5f);
		VK_Play_Speech_Line(11, 880, 0.5f);
		VK_Play_Speech_Line(11, 890, 0.5f);
		break;
	case 7580:
		VK_Play_Speech_Line(11, 900, 0.5f);
		VK_Play_Speech_Line(0, 8180, 0.5f);
		VK_Subject_Reacts(20, 8, 3, 6);
		VK_Play_Speech_Line(11, 910, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(50, 9, 3, 8);
		VK_Play_Speech_Line(11, 1250, 0.5f);
		break;
	case 7595:
		VK_Subject_Reacts(25, 8, -3, 5);
		VK_Play_Speech_Line(11, 920, 0.5f);
		VK_Play_Speech_Line(0, 8185, 0.5f);
		VK_Play_Speech_Line(11, 930, 0.5f);
		break;
	case 7600:
		VK_Subject_Reacts(20, 5, 9, 2);
		VK_Play_Speech_Line(11, 940, 0.5f);
		break;
	case 7605:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(40, 10, -3, 15);
		VK_Play_Speech_Line(11, 950, 0.5f);
		break;
	case 7620:
		VK_Subject_Reacts(30, 9, 10, 10);
		VK_Play_Speech_Line(11, 960, 0.5f);
		break;
	case 7635:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 970, 0.5f);
		VK_Play_Speech_Line(0, 7645, 0.5f);
		VK_Play_Speech_Line(0, 7650, 0.5f);
		VK_Play_Speech_Line(0, 7655, 0.5f);
		VK_Play_Speech_Line(0, 7660, 0.5f);
		VK_Play_Speech_Line(0, 7665, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 980, 0.5f);
		VK_Subject_Reacts(20, 10, 5, 12);
		break;
	case 7670:
		VK_Subject_Reacts(30, 4, 1, 10);
		VK_Play_Speech_Line(11, 990, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(11, 1000, 0.5f);
		VK_Subject_Reacts(30, 9, 3, 10);
		VK_Play_Speech_Line(11, 1010, 0.5f);
		VK_Play_Speech_Line(0, 8190, 0.5f);
		VK_Play_Speech_Line(11, 1020, 0.5f);
		VK_Play_Speech_Line(11, 1030, 0.5f);
		VK_Play_Speech_Line(11, 1040, 0.5f);
		break;
	case 7690:
		VK_Play_Speech_Line(11, 1050, 0.5f);
		VK_Play_Speech_Line(0, 7695, 0.5f);
		VK_Play_Speech_Line(0, 7700, 0.5f);
		VK_Subject_Reacts(60, 11, 9, 100);
		VK_Play_Speech_Line(11, 1060, 0.5f);
		break;
	case 7705:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(11, 1070, 0.5f);
		VK_Play_Speech_Line(0, 7720, 0.5f);
		VK_Play_Speech_Line(0, 7725, 0.5f);
		VK_Play_Speech_Line(0, 7730, 0.5f);
		VK_Play_Speech_Line(0, 7735, 0.5f);
		VK_Subject_Reacts(60, 14, 3, 20);
		VK_Play_Speech_Line(11, 1080, 0.5f);
		VK_Play_Speech_Line(0, 8195, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(11, 1090, 0.5f);
		VK_Play_Speech_Line(0, 8200, 0.5f);
		break;
	case 7740:
		VK_Subject_Reacts(40, 10, 1, 15);
		VK_Play_Speech_Line(11, 1100, 0.5f);
		VK_Play_Speech_Line(0, 8205, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(11, 1110, 0.5f);
		VK_Play_Speech_Line(0, 8210, 0.5f);
		break;
	case 7750:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 9, -4, 20);
		VK_Play_Speech_Line(11, 1120, 0.5f);
		VK_Play_Speech_Line(11, 1130, 0.5f);
		VK_Play_Speech_Line(11, 1140, 0.5f);
		VK_Play_Speech_Line(0, 8220, 0.5f);
		VK_Play_Speech_Line(11, 1150, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(11, 1160, 0.5f);
		VK_Subject_Reacts(5, -8, 7, 10);
		break;
	}
}

void ScriptVK::sub_406088(int a1) {
	switch (a1) {
	case 7385:
		VK_Subject_Reacts(36, 0, 0, 0);
		VK_Play_Speech_Line(3, 1440, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(40, 2, 2, 0);
		VK_Play_Speech_Line(3, 1450, 0.5f);
		VK_Play_Speech_Line(0, 7785, 0.5f);
		VK_Play_Speech_Line(3, 1460, 0.5f);
		VK_Play_Speech_Line(0, 7790, 0.5f);
		break;
	case 7395:
		VK_Play_Speech_Line(3, 1470, 0.5f);
		VK_Subject_Reacts(40, 4, 4, 0);
		VK_Play_Speech_Line(0, 7795, 0.5f);
		VK_Play_Speech_Line(3, 1480, 0.5f);
		break;
	case 7400:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 1490, 0.5f);
			VK_Subject_Reacts(15, -1, 9, 0);
			VK_Play_Speech_Line(3, 1500, 0.5f);
		} else {
			VK_Subject_Reacts(15, 13, -1, 0);
			VK_Play_Speech_Line(3, 1510, 0.5f);
		}
		break;
	case 7405:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 1520, 0.5f);
			VK_Play_Speech_Line(0, 7840, 0.5f);
			VK_Subject_Reacts(20, -1, 9, 10);
			VK_Play_Speech_Line(3, 1540, 0.80000001f);
			VK_Play_Speech_Line(3, 1550, 0.5f);
		} else {
			VK_Play_Speech_Line(3, 1560, 0.5f);
			VK_Subject_Reacts(25, 13, -3, 0);
			VK_Play_Speech_Line(3, 1570, 0.80000001f);
			VK_Play_Speech_Line(3, 1580, 0.5f);
		}
		break;
	case 7410:
		VK_Play_Speech_Line(3, 1590, 0.5f);
		VK_Subject_Reacts(50, 10, 10, 10);
		VK_Play_Speech_Line(3, 1600, 0.5f);
		break;
	case 7415:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(80, 6, 4, 10);
		VK_Play_Speech_Line(3, 1610, 0.5f);
		break;
	case 7420:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 1620, 0.5f);
			VK_Subject_Reacts(25, -1, 9, 0);
		} else {
			VK_Subject_Reacts(25, 14, -2, 0);
			VK_Play_Speech_Line(3, 1630, 0.89999998f);
			VK_Play_Speech_Line(3, 1640, 0.5f);
		}
		break;
	case 7425:
		VK_Subject_Reacts(40, -1, -1, 0);
		VK_Play_Speech_Line(3, 1650, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(65, 4, 6, 10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(3, 1660, 0.5f);
		break;
	case 7435:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 1670, 0.5f);
			VK_Subject_Reacts(60, -2, 9, 0);
			VK_Play_Speech_Line(3, 1680, 0.5f);
		} else {
			VK_Subject_Reacts(60, 14, 2, 0);
			VK_Play_Speech_Line(3, 1690, 0.5f);
		}
		break;
	case 7440:
		VK_Subject_Reacts(30, 3, 5, 0);
		VK_Play_Speech_Line(3, 1700, 0.5f);
		break;
	case 7445:
		VK_Play_Speech_Line(3, 1710, 0.5f);
		VK_Play_Speech_Line(0, 7800, 0.5f);
		VK_Play_Speech_Line(3, 1720, 0.5f);
		VK_Subject_Reacts(45, 4, 6, 0);
		VK_Play_Speech_Line(3, 1730, 0.5f);
		break;
	case 7450:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(60, 7, 7, 20);
		VK_Play_Speech_Line(3, 1740, 0.5f);
		VK_Play_Speech_Line(0, 7805, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(3, 1750, 0.89999998f);
		VK_Play_Speech_Line(3, 1760, 0.5f);
		break;
	case 7455:
		VK_Play_Speech_Line(3, 1780, 0.5f);
		VK_Subject_Reacts(35, 3, 5, 0);
		VK_Play_Speech_Line(3, 1790, 0.5f);
		VK_Play_Speech_Line(0, 7810, 0.5f);
		VK_Play_Speech_Line(3, 1800, 0.5f);
		VK_Play_Speech_Line(0, 7815, 0.5f);
		break;
	case 7460:
		if (Game_Flag_Query(47)) {
			VK_Subject_Reacts(40, -2, 10, 10);
			VK_Play_Speech_Line(3, 1810, 0.5f);
		} else {
			VK_Subject_Reacts(35, 14, 3, 0);
			VK_Play_Speech_Line(3, 1820, 0.5f);
		}
		break;
	case 7465:
		if (Game_Flag_Query(47)) {
			VK_Subject_Reacts(60, -3, 10, 5);
			VK_Play_Speech_Line(3, 1830, 0.5f);
		} else {
			VK_Play_Speech_Line(3, 1840, 0.5f);
			VK_Subject_Reacts(60, 13, 2, 5);
		}
		break;
	case 7470:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 1850, 0.5f);
			VK_Subject_Reacts(50, -2, 11, 0);
			VK_Play_Speech_Line(3, 1860, 0.5f);
		} else {
			VK_Subject_Reacts(90, 15, -5, 10);
			VK_Play_Speech_Line(3, 1870, 0.5f);
			VK_Play_Speech_Line(0, 8532, 0.5f);
			VK_Play_Speech_Line(3, 1890, 0.5f);
		}
		break;
	case 7475:
		VK_Subject_Reacts(28, 0, 0, 0);
		VK_Play_Speech_Line(3, 1900, 0.5f);
		break;
	case 7480:
		if (Game_Flag_Query(47)) {
			VK_Subject_Reacts(55, -3, 12, 5);
			VK_Play_Speech_Line(3, 1910, 0.5f);
		} else {
			VK_Play_Speech_Line(3, 1920, 0.5f);
			VK_Subject_Reacts(55, 17, -3, 5);
			VK_Play_Speech_Line(3, 1930, 0.5f);
		}
		break;
	case 7485:
		VK_Play_Speech_Line(3, 1940, 0.5f);
		VK_Subject_Reacts(38, 4, 9, 0);
		break;
	case 7490:
		VK_Subject_Reacts(43, 8, 8, 5);
		VK_Play_Speech_Line(3, 1950, 0.5f);
		VK_Play_Speech_Line(0, 7820, 0.5f);
		VK_Play_Speech_Line(3, 1960, 0.5f);
		VK_Play_Speech_Line(0, 7825, 0.5f);
		break;
	case 7495:
		VK_Play_Speech_Line(3, 1970, 0.5f);
		VK_Play_Speech_Line(0, 7830, 0.5f);
		VK_Play_Speech_Line(3, 1980, 0.5f);
		VK_Subject_Reacts(65, 4, 4, 5);
		VK_Play_Speech_Line(3, 1990, 0.5f);
		break;
	case 7515:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2000, 0.5f);
			VK_Subject_Reacts(72, -3, 12, 2);
			VK_Play_Speech_Line(3, 2010, 0.5f);
			VK_Play_Speech_Line(0, 7835, 0.5f);
			VK_Play_Speech_Line(3, 2020, 0.5f);
		} else {
			VK_Subject_Reacts(60, 16, -1, 2);
			VK_Play_Speech_Line(3, 2030, 0.5f);
			VK_Play_Speech_Line(0, 7840, 0.5f);
		}
		break;
	case 7525:
		VK_Subject_Reacts(40, 6, 6, 0);
		VK_Play_Speech_Line(3, 2040, 0.5f);
		VK_Play_Speech_Line(0, 8533, 0.5f);
		VK_Play_Speech_Line(3, 2060, 0.5f);
		VK_Play_Speech_Line(3, 2070, 0.5f);
		break;
	case 7535:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2080, 0.5f);
			VK_Play_Speech_Line(0, 7845, 0.5f);
			VK_Play_Speech_Line(3, 2090, 0.5f);
			VK_Subject_Reacts(60, -6, 11, 0);
			VK_Play_Speech_Line(3, 2100, 0.5f);
		} else {
			VK_Subject_Reacts(60, 17, -7, 0);
			VK_Play_Speech_Line(3, 2110, 0.5f);
			VK_Play_Speech_Line(0, 7850, 0.5f);
			VK_Play_Speech_Line(3, 2120, 0.5f);
			VK_Play_Speech_Line(0, 7855, 0.5f);
			VK_Play_Speech_Line(3, 2130, 0.5f);
		}
		break;
	case 7540:
		VK_Eye_Animates(2);
		if (Game_Flag_Query(47)) {
			VK_Subject_Reacts(70, -5, 12, 80);
			VK_Play_Speech_Line(3, 2140, 0.5f);
		} else {
			VK_Subject_Reacts(80, 17, -1, 80);
			VK_Play_Speech_Line(3, 2150, 1.0f);
			VK_Play_Speech_Line(3, 2160, 0.5f);
			VK_Play_Speech_Line(0, 7860, 0.5f);
		}
		break;
	case 7550:
		VK_Play_Speech_Line(3, 2170, 0.5f);
		VK_Play_Speech_Line(0, 7865, 0.5f);
		VK_Play_Speech_Line(3, 2180, 0.5f);
		VK_Subject_Reacts(55, 6, 5, 0);
		VK_Play_Speech_Line(3, 2190, 0.5f);
		break;
	case 7565:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(3, 2210, 0.5f);
		VK_Play_Speech_Line(0, 7870, 0.5f);
		VK_Play_Speech_Line(3, 2220, 0.5f);
		VK_Subject_Reacts(70, 8, 7, 10);
		VK_Play_Speech_Line(3, 2230, 0.5f);
		VK_Play_Speech_Line(0, 7875, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(3, 2240, 0.5f);
		VK_Play_Speech_Line(3, 2250, 0.5f);
		break;
	case 7580:
		VK_Play_Speech_Line(3, 2260, 0.5f);
		VK_Subject_Reacts(40, 5, 5, 10);
		VK_Play_Speech_Line(3, 2270, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(50, 8, 8, 7);
		VK_Play_Speech_Line(3, 2280, 0.5f);
		break;
	case 7595:
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(3, 2290, 0.5f);
		VK_Subject_Reacts(30, 5, 5, 8);
		break;
	case 7600:
		VK_Play_Speech_Line(3, 2300, 0.5f);
		VK_Subject_Reacts(30, 4, 4, 5);
		if (!Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2310, 0.5f);
		}
		break;
	case 7605:
		VK_Subject_Reacts(60, -1, -1, 5);
		VK_Play_Speech_Line(3, 2320, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(3, 2340, 0.5f);
		VK_Subject_Reacts(72, 9, 9, 5);
		VK_Play_Speech_Line(3, 2350, 0.5f);
		VK_Play_Speech_Line(0, 7885, 0.5f);
		VK_Play_Speech_Line(3, 2360, 0.5f);
		break;
	case 7635:
		VK_Subject_Reacts(60, 6, 7, 0);
		VK_Play_Speech_Line(3, 2370, 0.5f);
		break;
	case 7670:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2380, 0.5f);
			VK_Play_Speech_Line(0, 7890, 0.5f);
			VK_Play_Speech_Line(3, 2390, 0.5f);
			VK_Subject_Reacts(90, -3, 14, 50);
			VK_Play_Speech_Line(0, 7895, 0.5f);
		} else {
			VK_Subject_Reacts(80, 18, -3, 10);
			VK_Play_Speech_Line(3, 2410, 0.5f);
			VK_Play_Speech_Line(0, 8534, 0.5f);
			VK_Play_Speech_Line(3, 2430, 0.5f);
		}
		break;
	case 7680:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(47)) {
			VK_Subject_Reacts(70, -4, 14, 15);
			VK_Play_Speech_Line(3, 2440, 0.5f);
		} else {
			VK_Play_Speech_Line(3, 2450, 0.5f);
			VK_Subject_Reacts(70, 18, -4, 15);
			VK_Play_Speech_Line(3, 2460, 0.5f);
		}
		break;
	case 7690:
		VK_Play_Speech_Line(3, 2470, 0.5f);
		VK_Subject_Reacts(20, 9, 8, 5);
		VK_Play_Speech_Line(3, 2480, 0.5f);
		VK_Play_Speech_Line(0, 7900, 0.5f);
		break;
	case 7705:
		VK_Eye_Animates(3);
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2500, 0.5f);
			VK_Subject_Reacts(85, 7, 14, 20);
			VK_Play_Speech_Line(3, 2510, 0.5f);
		} else {
			VK_Subject_Reacts(99, 18, 7, 20);
			VK_Play_Speech_Line(3, 2530, 0.5f);
			VK_Play_Speech_Line(0, 7910, 0.5f);
			VK_Play_Speech_Line(3, 2550, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(0, 7915, 0.5f);
		}
		break;
	case 7740:
		VK_Subject_Reacts(60, 5, 6, 0);
		VK_Play_Speech_Line(3, 2560, 0.5f);
		break;
	case 7750:
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2580, 0.5f);
			VK_Subject_Reacts(90, -5, 14, 20);
			VK_Play_Speech_Line(3, 2590, 0.5f);
			VK_Play_Speech_Line(0, 7920, 0.5f);
		} else {
			VK_Subject_Reacts(90, 17, 3, 20);
			VK_Play_Speech_Line(3, 2600, 0.5f);
			VK_Play_Speech_Line(0, 7925, 0.5f);
			VK_Eye_Animates(3);
			VK_Play_Speech_Line(3, 2610, 0.5f);
			VK_Play_Speech_Line(0, 7930, 0.5f);
		}
		break;
	case 7770:
		VK_Eye_Animates(2);
		if (Game_Flag_Query(47)) {
			VK_Play_Speech_Line(3, 2630, 0.5f);
			VK_Subject_Reacts(99, 6, 15, 30);
		} else {
			VK_Play_Speech_Line(3, 2640, 0.5f);
			VK_Subject_Reacts(99, 15, -4, 30);
		}
		break;
	}
}

void ScriptVK::sub_407CF8(int a1) {
	switch (a1) {
	case 7385:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7390:
		VK_Eye_Animates(3);
		VK_Subject_Reacts(60, 15, -30, 2);
		VK_Play_Speech_Line(15, 1260, 0.5f);
		break;
	case 7395:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(90, -40, -10, 6);
		VK_Play_Speech_Line(15, 1280, 0.5f);
		break;
	case 7400:
		VK_Subject_Reacts(70, -10, -10, 0);
		VK_Play_Speech_Line(15, 1300, 0.5f);
		break;
	case 7405:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(100, 10, 10, 0);
		VK_Play_Speech_Line(15, 1330, 0.5f);
		break;
	case 7415:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7420:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7425:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7435:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7445:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7450:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7455:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7470:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7475:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7490:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7495:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7515:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7525:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7535:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7540:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7550:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7565:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7580:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7585:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7595:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7600:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7605:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7620:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7635:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7670:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7680:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7690:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7705:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7740:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7750:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	case 7770:
		VK_Subject_Reacts(20, 10, 20, 0);
		VK_Play_Speech_Line(15, 1240, 0.5f);
		break;
	}
}

void ScriptVK::sub_40897C(int a1) {
	switch (a1) {
	case 7385:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(54, 0, 0, 0);
		VK_Play_Speech_Line(14, 900, 0.5f);
		VK_Play_Speech_Line(14, 910, 0.5f);
		VK_Play_Speech_Line(0, 8225, 0.5f);
		break;
	case 7390:
		VK_Subject_Reacts(48, 0, 0, 3);
		VK_Play_Speech_Line(14, 920, 0.5f);
		VK_Play_Speech_Line(0, 8230, 0.5f);
		break;
	case 7395:
		VK_Play_Speech_Line(14, 930, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(50, 0, 0, 8);
		VK_Play_Speech_Line(14, 940, 0.5f);
		VK_Play_Speech_Line(0, 8235, 0.5f);
		break;
	case 7400:
		VK_Play_Speech_Line(14, 950, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, -5, -6, 2);
		VK_Play_Speech_Line(14, 960, 0.5f);
		break;
	case 7405:
		VK_Subject_Reacts(25, 8, 7, 4);
		VK_Play_Speech_Line(14, 970, 0.5f);
		VK_Play_Speech_Line(0, 8240, 0.5f);
		VK_Play_Speech_Line(14, 980, 0.5f);
		break;
	case 7410:
		VK_Subject_Reacts(40, -6, -5, 5);
		VK_Play_Speech_Line(14, 990, 0.5f);
		VK_Play_Speech_Line(0, 8245, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1000, 0.5f);
		VK_Play_Speech_Line(0, 8250, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(70, 8, 6, 0);
		VK_Play_Speech_Line(14, 1010, 0.5f);
		break;
	case 7415:
		VK_Subject_Reacts(25, 9, 6, 5);
		VK_Play_Speech_Line(14, 1020, 0.5f);
		break;
	case 7420:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1030, 0.5f);
		VK_Play_Speech_Line(0, 8255, 0.5f);
		VK_Subject_Reacts(30, 7, 5, 3);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(14, 1040, 0.5f);
		break;
	case 7425:
		VK_Play_Speech_Line(14, 1050, 0.5f);
		VK_Play_Speech_Line(14, 1060, 0.5f);
		VK_Play_Speech_Line(0, 8260, 0.5f);
		VK_Subject_Reacts(5, 5, 6, -5);
		VK_Play_Speech_Line(14, 1070, 0.5f);
		break;
	case 7430:
		VK_Subject_Reacts(15, 7, 6, -6);
		VK_Play_Speech_Line(14, 1080, 0.5f);
		break;
	case 7435:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1100, 0.5f);
		VK_Subject_Reacts(25, 8, 5, -7);
		VK_Play_Speech_Line(0, 8265, 0.5f);
		VK_Play_Speech_Line(14, 1110, 0.5f);
		VK_Play_Speech_Line(14, 1120, 0.5f);
		break;
	case 7440:
		VK_Subject_Reacts(30, 8, 6, 0);
		VK_Play_Speech_Line(14, 1130, 0.5f);
		break;
	case 7445:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1140, 0.5f);
		VK_Subject_Reacts(80, 8, 8, -10);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(14, 1150, 0.5f);
		break;
	case 7450:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(14, 1160, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(60, 8, 8, -5);
		VK_Play_Speech_Line(14, 1170, 0.5f);
		break;
	case 7455:
		VK_Subject_Reacts(30, 7, 6, 3);
		VK_Play_Speech_Line(14, 1180, 0.5f);
		break;
	case 7460:
		VK_Subject_Reacts(10, 4, 4, 2);
		VK_Play_Speech_Line(14, 1190, 0.5f);
		break;
	case 7465:
		VK_Subject_Reacts(15, 5, 3, -5);
		VK_Play_Speech_Line(14, 1200, 0.5f);
		if (Actor_Query_Friendliness_To_Other(14, 0) <= 40) {
			VK_Eye_Animates(2);
			VK_Play_Speech_Line(14, 1210, 0.5f);
			VK_Eye_Animates(1);
		}
		break;
	case 7470:
		VK_Subject_Reacts(50, -4, 0, -5);
		VK_Play_Speech_Line(14, 1240, 0.5f);
		break;
	case 7475:
		VK_Play_Speech_Line(14, 1250, 0.5f);
		VK_Subject_Reacts(30, 9, 7, -5);
		VK_Play_Speech_Line(14, 1260, 0.5f);
		break;
	case 7480:
		VK_Subject_Reacts(60, 10, 8, -6);
		VK_Play_Speech_Line(14, 1270, 0.5f);
		break;
	case 7485:
		VK_Subject_Reacts(70, 8, 9, 10);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1280, 0.5f);
		break;
	case 7490:
		VK_Play_Speech_Line(14, 1290, 0.5f);
		VK_Play_Speech_Line(14, 1300, 0.5f);
		VK_Subject_Reacts(10, 11, 10, 0);
		VK_Play_Speech_Line(14, 1310, 0.5f);
		break;
	case 7495:
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(14, 1320, 0.5f);
		VK_Subject_Reacts(10, 8, 7, 7);
		break;
	case 7515:
		VK_Subject_Reacts(25, 7, 7, 0);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1330, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1340, 0.5f);
		break;
	case 7525:
		VK_Play_Speech_Line(14, 1350, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 7, 6, 6);
		break;
	case 7535:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1360, 0.5f);
		VK_Play_Speech_Line(0, 8275, 0.5f);
		VK_Subject_Reacts(10, 9, 7, -4);
		VK_Play_Speech_Line(0, 8280, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1380, 0.5f);
		VK_Play_Speech_Line(14, 1390, 0.5f);
		VK_Play_Speech_Line(0, 8285, 0.5f);
		break;
	case 7540:
		VK_Play_Speech_Line(14, 1400, 0.5f);
		VK_Subject_Reacts(30, 10, 9, 10);
		VK_Play_Speech_Line(14, 1410, 0.5f);
		break;
	case 7550:
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1420, 0.5f);
		VK_Subject_Reacts(20, 7, 7, -5);
		VK_Play_Speech_Line(14, 1430, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		break;
	case 7565:
		VK_Play_Speech_Line(14, 1440, 0.5f);
		VK_Play_Speech_Line(0, 8295, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1450, 0.5f);
		VK_Play_Speech_Line(0, 7570, 0.5f);
		VK_Play_Speech_Line(0, 7575, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(14, 1460, 0.5f);
		VK_Play_Speech_Line(0, 8300, 0.5f);
		VK_Subject_Reacts(90, 8, 9, 18);
		VK_Play_Speech_Line(14, 1470, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7580:
		VK_Subject_Reacts(20, 9, 7, 0);
		VK_Play_Speech_Line(14, 1480, 0.5f);
		VK_Play_Speech_Line(0, 8305, 0.5f);
		VK_Play_Speech_Line(14, 1490, 0.5f);
		break;
	case 7585:
		VK_Play_Speech_Line(14, 1500, 0.5f);
		VK_Play_Speech_Line(0, 7590, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(10, 8, 7, 5);
		VK_Play_Speech_Line(14, 1510, 0.5f);
		VK_Play_Speech_Line(14, 1520, 0.5f);
		break;
	case 7595:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 10, 9, 15);
		VK_Play_Speech_Line(14, 1530, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7600:
		VK_Subject_Reacts(20, 8, 8, 5);
		VK_Play_Speech_Line(14, 1540, 0.5f);
		break;
	case 7605:
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 9, 8, 10);
		VK_Play_Speech_Line(14, 1550, 0.5f);
		break;
	case 7620:
		VK_Play_Speech_Line(14, 1560, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(0, 7630, 0.5f);
		VK_Eye_Animates(3);
		VK_Play_Speech_Line(14, 1570, 0.5f);
		VK_Subject_Reacts(10, 10, 9, 10);
		break;
	case 7635:
		VK_Play_Speech_Line(14, 1580, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(0, 8310, 0.5f);
		VK_Play_Speech_Line(0, 7645, 0.5f);
		VK_Play_Speech_Line(0, 7650, 0.5f);
		VK_Play_Speech_Line(0, 7655, 0.5f);
		VK_Play_Speech_Line(0, 7660, 0.5f);
		VK_Play_Speech_Line(0, 7665, 0.5f);
		VK_Eye_Animates(2);
		VK_Subject_Reacts(60, 8, 8, 40);
		VK_Play_Speech_Line(14, 1590, 0.5f);
		VK_Play_Speech_Line(0, 8315, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(85, 10, 11, 0);
		VK_Play_Speech_Line(14, 1600, 0.5f);
		VK_Eye_Animates(3);
		break;
	case 7670:
		VK_Subject_Reacts(50, 12, 7, 10);
		VK_Play_Speech_Line(14, 1620, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(14, 1630, 0.5f);
		break;
	case 7680:
		VK_Play_Speech_Line(14, 1640, 0.5f);
		VK_Subject_Reacts(15, 5, 6, 5);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(0, 8320, 0.5f);
		VK_Play_Speech_Line(14, 1650, 0.5f);
		break;
	case 7690:
		VK_Eye_Animates(2);
		VK_Subject_Reacts(50, 14, 13, 15);
		VK_Play_Speech_Line(14, 1660, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(0, 8325, 0.5f);
		VK_Play_Speech_Line(14, 1670, 0.5f);
		break;
	case 7705:
		VK_Play_Speech_Line(14, 1680, 0.5f);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(0, 7720, 0.5f);
		VK_Eye_Animates(1);
		VK_Subject_Reacts(40, 12, 10, 0);
		VK_Play_Speech_Line(0, 7725, 0.5f);
		VK_Play_Speech_Line(0, 7730, 0.5f);
		VK_Subject_Reacts(55, 6, 6, 0);
		VK_Eye_Animates(1);
		VK_Play_Speech_Line(0, 7735, 0.5f);
		VK_Eye_Animates(2);
		VK_Subject_Reacts(70, 11, 9, 100);
		VK_Play_Speech_Line(14, 1690, 0.5f);
		VK_Eye_Animates(2);
		break;
	case 7740:
		VK_Subject_Reacts(30, 4, 3, 3);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(14, 1700, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(14, 1710, 0.5f);
		VK_Play_Speech_Line(14, 1720, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(14, 1730, 0.5f);
		break;
	case 7750:
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Subject_Reacts(10, 8, 5, 0);
		VK_Play_Speech_Line(14, 1740, 0.5f);
		VK_Play_Speech_Line(0, 8330, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(14, 1750, 0.5f);
		VK_Subject_Reacts(25, 7, 5, 8);
		VK_Play_Speech_Line(14, 1760, 0.5f);
		VK_Play_Speech_Line(14, 1770, 0.5f);
		break;
	case 7770:
		VK_Play_Speech_Line(14, 1780, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Play_Speech_Line(0, 8335, 0.5f);
		VK_Eye_Animates(2);
		VK_Play_Speech_Line(14, 1790, 0.5f);
		if (Random_Query(0, 1) == 1) {
			VK_Eye_Animates(1);
		}
		VK_Subject_Reacts(30, 7, 7, 10);
		break;
	}
}

void ScriptVK::sub_40A300(int a1, int a2) {
	switch (a1) {
	case 15:
		sub_407CF8(7385);
		break;
	case 14:
		sub_40897C(7385);
		break;
	case 11:
		sub_404B44(7385);
		break;
	case 6:
		sub_402604(7385);
		break;
	case 3:
		sub_40A510(7385);
		break;
	}
}

void ScriptVK::sub_40A350(int a1, int a2) {
	switch (a1) {
	case 15:
		sub_407CF8(7390);
		break;
	case 14:
		sub_40897C(7390);
		break;
	case 11:
		sub_404B44(7390);
		break;
	case 6:
		sub_402604(7390);
		break;
	case 3:
		sub_40A470(7390);
		break;
	}
}

void ScriptVK::sub_40A3A0(int a1, int a2) {
	switch (a1) {
	case 15:
		sub_407CF8(7395);
		break;
	case 14:
		sub_40897C(7395);
		break;
	case 11:
		sub_404B44(7395);
		break;
	case 6:
		sub_402604(7395);
		break;
	case 3:
		sub_40A3F0(7395);
		break;
	}
}

void ScriptVK::sub_40A3F0(int a1) {
	VK_Play_Speech_Line(3, 1470, 0.5f);
	VK_Subject_Reacts(40, 4, 4, 0);
	VK_Play_Speech_Line(0, 7795, 0.5f);
	VK_Play_Speech_Line(3, 1480, 0.5f);
}

void ScriptVK::sub_40A470(int a1) {
	VK_Subject_Reacts(40, 2, 2, 0);
	VK_Play_Speech_Line(3, 1450, 0.5f);
	VK_Play_Speech_Line(0, 7785, 0.5f);
	VK_Play_Speech_Line(3, 1460, 0.5f);
	VK_Play_Speech_Line(0, 7790, 0.5f);
}

void ScriptVK::sub_40A510(int a1) {
	VK_Subject_Reacts(36, 0, 0, 0);
	VK_Play_Speech_Line(3, 1440, 0.5f);
}

} // End of namespace BladeRunner
