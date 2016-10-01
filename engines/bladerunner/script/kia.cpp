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

#include "bladerunner/script/kia.h"

#include "bladerunner/bladerunner.h"

namespace BladeRunner {

void ScriptKIA::SCRIPT_KIA_DLL_Play_Clue_Asset_Script(int a1, int clueId) {
	int v1;
	switch (clueId) {
	case 0:
		KIA_Play_Actor_Dialogue(23, 40);
		break;
	case 2:
		KIA_Play_Actor_Dialogue(23, 0);
		break;
	case 3:
		KIA_Play_Actor_Dialogue(99, 1970);
		KIA_Play_Actor_Dialogue(99, 1980);
		KIA_Play_Actor_Dialogue(99, 1990);
		break;
	case 4:
		KIA_Play_Actor_Dialogue(99, 1970);
		KIA_Play_Actor_Dialogue(99, 1980);
		KIA_Play_Actor_Dialogue(99, 1990);
		break;
	case 5:
		KIA_Play_Slice_Model(966);
		KIA_Play_Actor_Dialogue(99, 1960);
		break;
	case 6:
		KIA_Play_Slice_Model(933);
		break;
	case 7:
		KIA_Play_Slice_Model(971);
		break;
	case 8:
		KIA_Play_Slice_Model(937);
		KIA_Play_Actor_Dialogue(99, 2010);
		break;
	case 9:
		KIA_Play_Photograph(6);
		KIA_Play_Actor_Dialogue(99, 2020);
		KIA_Play_Actor_Dialogue(99, 2030);
		KIA_Play_Actor_Dialogue(99, 2040);
		break;
	case 10:
		KIA_Play_Actor_Dialogue(30, 140);
		break;
	case 11:
		KIA_Play_Actor_Dialogue(30, 50);
		KIA_Play_Actor_Dialogue(30, 60);
		KIA_Play_Actor_Dialogue(30, 70);
		KIA_Play_Actor_Dialogue(30, 80);
		KIA_Play_Actor_Dialogue(30, 90);
		break;
	case 12:
		KIA_Play_Slice_Model(975);
		break;
	case 13:
		KIA_Play_Photograph(5);
		break;
	case 14:
		KIA_Play_Photograph(4);
		KIA_Play_Actor_Dialogue(99, 4050);
		break;
	case 15:
		KIA_Play_Slice_Model(964);
		KIA_Play_Actor_Dialogue(15, 280);
		KIA_Play_Actor_Dialogue(15, 290);
		break;
	case 16:
		KIA_Play_Actor_Dialogue(23, 100);
		break;
	case 17:
		KIA_Play_Actor_Dialogue(23, 120);
		KIA_Play_Actor_Dialogue(23, 130);
		break;
	case 19:
		KIA_Play_Actor_Dialogue(0, 380);
		KIA_Play_Actor_Dialogue(19, 30);
		KIA_Play_Actor_Dialogue(19, 40);
		KIA_Play_Actor_Dialogue(0, 410);
		KIA_Play_Actor_Dialogue(19, 50);
		break;
	case 20:
		KIA_Play_Photograph(33);
		KIA_Play_Actor_Dialogue(99, 350);
		break;
	case 21:
		KIA_Play_Actor_Dialogue(12, 10);
		break;
	case 22:
		KIA_Play_Actor_Dialogue(15, 40);
		KIA_Play_Actor_Dialogue(15, 50);
		KIA_Play_Actor_Dialogue(0, 4565);
		KIA_Play_Actor_Dialogue(15, 60);
		break;
	case 23:
		KIA_Play_Actor_Dialogue(15, 250);
		KIA_Play_Actor_Dialogue(15, 270);
		break;
	case 24:
		KIA_Play_Actor_Dialogue(15, 260);
		KIA_Play_Actor_Dialogue(15, 270);
		break;
	case 25:
		KIA_Play_Actor_Dialogue(0, 295);
		KIA_Play_Actor_Dialogue(28, 90);
		KIA_Play_Actor_Dialogue(28, 100);
		break;
	case 26:
		KIA_Play_Actor_Dialogue(99, 1880);
		KIA_Play_Actor_Dialogue(99, 1890);
		break;
	case 27:
		KIA_Play_Slice_Model(938);
		break;
	case 28:
		KIA_Play_Photograph(11);
		break;
	case 29:
		KIA_Play_Photograph(12);
		break;
	case 30:
		KIA_Play_Photograph(10);
		break;
	case 31:
		KIA_Play_Photograph(9);
		break;
	case 32:
		KIA_Play_Slice_Model(987);
		KIA_Play_Actor_Dialogue(0, 5870);
		KIA_Play_Actor_Dialogue(4, 810);
		KIA_Play_Actor_Dialogue(4, 820);
		break;
	case 33:
		KIA_Play_Actor_Dialogue(53, 20);
		KIA_Play_Actor_Dialogue(0, 680);
		KIA_Play_Actor_Dialogue(53, 30);
		break;
	case 37:
		KIA_Play_Slice_Model(952);
		break;
	case 39:
		KIA_Play_Actor_Dialogue(30, 170);
		KIA_Play_Actor_Dialogue(30, 180);
		KIA_Play_Actor_Dialogue(30, 190);
		KIA_Play_Actor_Dialogue(30, 200);
		break;
	case 40:
		KIA_Play_Actor_Dialogue(38, 90);
		KIA_Play_Actor_Dialogue(38, 100);
		KIA_Play_Actor_Dialogue(38, 110);
		KIA_Play_Actor_Dialogue(38, 120);
		KIA_Play_Actor_Dialogue(38, 130);
		KIA_Play_Actor_Dialogue(38, 140);
		KIA_Play_Actor_Dialogue(38, 150);
		break;
	case 43:
		KIA_Play_Actor_Dialogue(4, 30);
		KIA_Play_Actor_Dialogue(4, 50);
		KIA_Play_Actor_Dialogue(4, 70);
		break;
	case 44:
		KIA_Play_Slice_Model(940);
		KIA_Play_Actor_Dialogue(99, 2140);
		KIA_Play_Actor_Dialogue(99, 2150);
		KIA_Play_Actor_Dialogue(99, 2160);
		break;
	case 45:
		KIA_Play_Slice_Model(975);
		break;
	case 46:
		KIA_Play_Actor_Dialogue(0, 5140);
		KIA_Play_Actor_Dialogue(17, 30);
		KIA_Play_Actor_Dialogue(17, 40);
		break;
	case 47:
		KIA_Play_Photograph(31);
		KIA_Play_Actor_Dialogue(99, 2140);
		KIA_Play_Actor_Dialogue(99, 2150);
		KIA_Play_Actor_Dialogue(99, 2160);
		break;
	case 49:
		KIA_Play_Slice_Model(974);
		KIA_Play_Actor_Dialogue(99, 2320);
		if (Game_Flag_Query(48)) {
			KIA_Play_Actor_Dialogue(99, 2330);
			KIA_Play_Actor_Dialogue(99, 2340);
		}
		KIA_Play_Actor_Dialogue(99, 2350);
		break;
	case 50:
		KIA_Play_Actor_Dialogue(17, 100);
		KIA_Play_Actor_Dialogue(17, 110);
		KIA_Play_Actor_Dialogue(17, 120);
		KIA_Play_Actor_Dialogue(17, 130);
		break;
	case 51:
		KIA_Play_Actor_Dialogue(99, 2170);
		KIA_Play_Actor_Dialogue(99, 2180);
		KIA_Play_Actor_Dialogue(99, 2190);
		KIA_Play_Actor_Dialogue(99, 2200);
		break;
	case 52:
		KIA_Play_Actor_Dialogue(1, 2230);
		KIA_Play_Actor_Dialogue(1, 2260);
		KIA_Play_Actor_Dialogue(1, 2270);
		KIA_Play_Actor_Dialogue(1, 2280);
		break;
	case 53:
		KIA_Play_Slice_Model(955);
		if (Query_Difficulty_Level() == 0) {
			KIA_Play_Actor_Dialogue(99, 4140);
		} else {
			KIA_Play_Actor_Dialogue(99, 4150);
		}
		break;
	case 54:
		KIA_Play_Slice_Model(973);
		KIA_Play_Actor_Dialogue(99, 4280);
		KIA_Play_Actor_Dialogue(99, 4290);
		break;
	case 55:
		KIA_Play_Slice_Model(973);
		KIA_Play_Actor_Dialogue(99, 4280);
		KIA_Play_Actor_Dialogue(99, 4300);
		break;
	case 56:
		KIA_Play_Actor_Dialogue(16, 90);
		KIA_Play_Actor_Dialogue(16, 100);
		KIA_Play_Actor_Dialogue(16, 110);
		KIA_Play_Actor_Dialogue(16, 120);
		KIA_Play_Actor_Dialogue(16, 130);
		break;
	case 57:
		KIA_Play_Actor_Dialogue(20, 90);
		KIA_Play_Actor_Dialogue(20, 100);
		break;
	case 58:
		KIA_Play_Actor_Dialogue(14, 320);
		KIA_Play_Actor_Dialogue(14, 330);
		KIA_Play_Actor_Dialogue(14, 340);
		KIA_Play_Actor_Dialogue(14, 380);
		KIA_Play_Actor_Dialogue(14, 390);
		KIA_Play_Actor_Dialogue(14, 400);
		break;
	case 59:
		KIA_Play_Actor_Dialogue(14, 320);
		KIA_Play_Actor_Dialogue(14, 330);
		KIA_Play_Actor_Dialogue(14, 410);
		KIA_Play_Actor_Dialogue(14, 420);
		KIA_Play_Actor_Dialogue(14, 440);
		KIA_Play_Actor_Dialogue(14, 450);
		break;
	case 60:
		KIA_Play_Actor_Dialogue(7, 210);
		KIA_Play_Actor_Dialogue(7, 220);
		KIA_Play_Actor_Dialogue(7, 240);
		KIA_Play_Actor_Dialogue(7, 250);
		break;
	case 61:
		KIA_Play_Actor_Dialogue(7, 750);
		KIA_Play_Actor_Dialogue(7, 760);
		KIA_Play_Actor_Dialogue(0, 5500);
		KIA_Play_Actor_Dialogue(7, 780);
		KIA_Play_Actor_Dialogue(7, 790);
		break;
	case 62:
		KIA_Play_Slice_Model(963);
		break;
	case 63:
		KIA_Play_Actor_Dialogue(14, 560);
		KIA_Play_Actor_Dialogue(14, 570);
		KIA_Play_Actor_Dialogue(14, 580);
		break;
	case 64:
		KIA_Play_Actor_Dialogue(29, 120);
		KIA_Play_Actor_Dialogue(29, 130);
		break;
	case 65:
		KIA_Play_Slice_Model(942);
		KIA_Play_Actor_Dialogue(99, 4160);
		break;
	case 66:
		KIA_Play_Actor_Dialogue(99, 2430);
		KIA_Play_Actor_Dialogue(99, 2440);
		KIA_Play_Actor_Dialogue(99, 2450);
		break;
	case 67:
		KIA_Play_Actor_Dialogue(52, 140);
		KIA_Play_Actor_Dialogue(52, 150);
		KIA_Play_Actor_Dialogue(52, 170);
		KIA_Play_Actor_Dialogue(52, 180);
		KIA_Play_Actor_Dialogue(52, 190);
		break;
	case 68:
		KIA_Play_Actor_Dialogue(35, 20);
		KIA_Play_Actor_Dialogue(35, 30);
		KIA_Play_Actor_Dialogue(35, 40);
		KIA_Play_Actor_Dialogue(35, 50);
		break;
	case 69:
		KIA_Play_Actor_Dialogue(2, 1010);
		KIA_Play_Actor_Dialogue(0, 6495);
		KIA_Play_Actor_Dialogue(2, 1020);
		KIA_Play_Actor_Dialogue(0, 6500);
		KIA_Play_Actor_Dialogue(2, 1030);
		break;
	case 70:
		KIA_Play_Actor_Dialogue(2, 1040);
		KIA_Play_Actor_Dialogue(2, 1050);
		KIA_Play_Actor_Dialogue(0, 6505);
		KIA_Play_Actor_Dialogue(2, 1060);
		KIA_Play_Actor_Dialogue(2, 1070);
		KIA_Play_Actor_Dialogue(0, 6510);
		KIA_Play_Actor_Dialogue(2, 1080);
		break;
	case 71:
		KIA_Play_Actor_Dialogue(56, 0);
		KIA_Play_Actor_Dialogue(56, 10);
		KIA_Play_Actor_Dialogue(56, 20);
		KIA_Play_Actor_Dialogue(56, 30);
		KIA_Play_Actor_Dialogue(56, 40);
		KIA_Play_Actor_Dialogue(56, 50);
		break;
	case 72:
		KIA_Play_Actor_Dialogue(99, 80);
		KIA_Play_Actor_Dialogue(99, 90);
		break;
	case 74:
		KIA_Play_Actor_Dialogue(99, 4370);
		KIA_Play_Actor_Dialogue(99, 4380);
		KIA_Play_Actor_Dialogue(99, 4390);
		KIA_Play_Actor_Dialogue(99, 4400);
		break;
	case 75:
		KIA_Play_Slice_Model(956);
		break;
	case 76:
		KIA_Play_Slice_Model(944);
		KIA_Play_Actor_Dialogue(99, 850);
		KIA_Play_Actor_Dialogue(99, 860);
		KIA_Play_Actor_Dialogue(99, 870);
		KIA_Play_Actor_Dialogue(99, 880);
		break;
	case 77:
		KIA_Play_Photograph(25);
		break;
	case 78:
		KIA_Play_Photograph(20);
		break;
	case 79:
		KIA_Play_Actor_Dialogue(0, 220);
		KIA_Play_Actor_Dialogue(16, 320);
		KIA_Play_Actor_Dialogue(0, 225);
		KIA_Play_Actor_Dialogue(16, 330);
		KIA_Play_Actor_Dialogue(0, 230);
		KIA_Play_Actor_Dialogue(16, 340);
		break;
	case 80:
		KIA_Play_Slice_Model(965);
		break;
	case 81:
		KIA_Play_Slice_Model(965);
		break;
	case 82:
		KIA_Play_Actor_Dialogue(4, 520);
		KIA_Play_Actor_Dialogue(4, 530);
		KIA_Play_Actor_Dialogue(4, 540);
		KIA_Play_Actor_Dialogue(4, 550);
		break;
	case 84:
		KIA_Play_Slice_Model(970);
		break;
	case 85:
		KIA_Play_Slice_Model(943);
		break;
	case 86:
		KIA_Play_Photograph(34);
		break;
	case 87:
		KIA_Play_Slice_Model(936);
		break;
	case 88:
		KIA_Play_Photograph(16);
		break;
	case 89:
		KIA_Play_Slice_Model(975);
		break;
	case 90:
		KIA_Play_Actor_Dialogue(16, 290);
		KIA_Play_Actor_Dialogue(16, 300);
		break;
	case 91:
		KIA_Play_Slice_Model(939);
		KIA_Play_Actor_Dialogue(99, 4050);
		break;
	case 92:
		KIA_Play_Actor_Dialogue(18, 140);
		KIA_Play_Actor_Dialogue(18, 150);
		break;
	case 93:
		KIA_Play_Slice_Model(969);
		break;
	case 94:
		KIA_Play_Actor_Dialogue(3, 650);
		KIA_Play_Actor_Dialogue(3, 660);
		KIA_Play_Actor_Dialogue(0, 3665);
		KIA_Play_Actor_Dialogue(3, 670);
		KIA_Play_Actor_Dialogue(3, 680);
		KIA_Play_Actor_Dialogue(3, 690);
		break;
	case 96:
		KIA_Play_Actor_Dialogue(3, 580);
		break;
	case 97:
		KIA_Play_Actor_Dialogue(0, 3600);
		KIA_Play_Actor_Dialogue(3, 550);
		break;
	case 98:
		KIA_Play_Slice_Model(935);
		break;
	case 99:
		KIA_Play_Slice_Model(957);
		break;
	case 100:
		KIA_Play_Slice_Model(961);
		break;
	case 101:
		KIA_Play_Actor_Dialogue(31, 210);
		KIA_Play_Actor_Dialogue(31, 220);
		KIA_Play_Actor_Dialogue(22, 140);
		KIA_Play_Actor_Dialogue(31, 230);
		break;
	case 102:
		KIA_Play_Actor_Dialogue(59, 210);
		KIA_Play_Actor_Dialogue(59, 260);
		KIA_Play_Actor_Dialogue(0, 1390);
		KIA_Play_Actor_Dialogue(59, 300);
		break;
	case 103:
		KIA_Play_Actor_Dialogue(2, 450);
		KIA_Play_Actor_Dialogue(0, 3280);
		break;
	case 104:
		KIA_Play_Actor_Dialogue(0, 3250);
		KIA_Play_Actor_Dialogue(2, 540);
		KIA_Play_Actor_Dialogue(2, 550);
		break;
	case 105:
		KIA_Play_Slice_Model(953);
		KIA_Play_Actor_Dialogue(99, 350);
		break;
	case 106:
		KIA_Play_Slice_Model(954);
		break;
	case 107:
		KIA_Play_Actor_Dialogue(0, 3860);
		KIA_Play_Actor_Dialogue(3, 1030);
		KIA_Play_Actor_Dialogue(3, 1040);
		KIA_Play_Actor_Dialogue(0, 3865);
		KIA_Play_Actor_Dialogue(3, 1050);
		KIA_Play_Actor_Dialogue(3, 1060);
		break;
	case 108:
		KIA_Play_Actor_Dialogue(33, 0);
		KIA_Play_Actor_Dialogue(33, 10);
		break;
	case 109:
		KIA_Play_Slice_Model(931);
		break;
	case 110:
		KIA_Play_Slice_Model(931);
		KIA_Play_Actor_Dialogue(99, 4420);
		break;
	case 112:
		KIA_Play_Actor_Dialogue(99, 3780);
		KIA_Play_Actor_Dialogue(99, 3790);
		break;
	case 113:
		KIA_Play_Actor_Dialogue(99, 3800);
		KIA_Play_Actor_Dialogue(99, 3810);
		KIA_Play_Actor_Dialogue(99, 3820);
		KIA_Play_Actor_Dialogue(99, 3830);
		break;
	case 114:
		KIA_Play_Actor_Dialogue(99, 3840);
		KIA_Play_Actor_Dialogue(99, 3850);
		KIA_Play_Actor_Dialogue(99, 3860);
		KIA_Play_Actor_Dialogue(99, 3870);
		break;
	case 115:
		KIA_Play_Actor_Dialogue(99, 3880);
		KIA_Play_Actor_Dialogue(99, 3890);
		KIA_Play_Actor_Dialogue(99, 3900);
		break;
	case 116:
		KIA_Play_Actor_Dialogue(9, 830);
		KIA_Play_Actor_Dialogue(9, 840);
		KIA_Play_Actor_Dialogue(9, 850);
		break;
	case 118:
		KIA_Play_Slice_Model(951);
		break;
	case 119:
		KIA_Play_Slice_Model(962);
		KIA_Play_Actor_Dialogue(99, 3930);
		KIA_Play_Actor_Dialogue(99, 3940);
		break;
	case 120:
		KIA_Play_Actor_Dialogue(99, 2550);
		KIA_Play_Actor_Dialogue(99, 2560);
		KIA_Play_Actor_Dialogue(99, 2570);
		KIA_Play_Actor_Dialogue(99, 2580);
		KIA_Play_Actor_Dialogue(99, 2590);
		break;
	case 121:
		KIA_Play_Actor_Dialogue(99, 2470);
		KIA_Play_Actor_Dialogue(99, 2480);
		KIA_Play_Actor_Dialogue(99, 2490);
		KIA_Play_Actor_Dialogue(99, 2500);
		break;
	case 122:
		KIA_Play_Actor_Dialogue(0, 5615);
		KIA_Play_Actor_Dialogue(12, 170);
		KIA_Play_Actor_Dialogue(0, 5625);
		KIA_Play_Actor_Dialogue(12, 180);
		KIA_Play_Actor_Dialogue(0, 5630);
		KIA_Play_Actor_Dialogue(12, 190);
		KIA_Play_Actor_Dialogue(0, 5635);
		KIA_Play_Actor_Dialogue(12, 200);
		break;
	case 123:
		KIA_Play_Actor_Dialogue(0, 5640);
		KIA_Play_Actor_Dialogue(12, 230);
		KIA_Play_Actor_Dialogue(0, 5645);
		KIA_Play_Actor_Dialogue(12, 240);
		KIA_Play_Actor_Dialogue(12, 250);
		KIA_Play_Actor_Dialogue(0, 5650);
		KIA_Play_Actor_Dialogue(12, 260);
		break;
	case 124:
		KIA_Play_Actor_Dialogue(12, 340);
		KIA_Play_Actor_Dialogue(12, 350);
		KIA_Play_Actor_Dialogue(12, 360);
		KIA_Play_Actor_Dialogue(99, 2710);
		KIA_Play_Actor_Dialogue(99, 2730);
		break;
	case 125:
		KIA_Play_Slice_Model(946);
		KIA_Play_Actor_Dialogue(99, 2740);
		KIA_Play_Actor_Dialogue(99, 2750);
		KIA_Play_Actor_Dialogue(99, 2760);
		KIA_Play_Actor_Dialogue(99, 2770);
		break;
	case 126:
		KIA_Play_Actor_Dialogue(99, 3320);
		break;
	case 127:
		KIA_Play_Slice_Model(959);
		break;
	case 128:
		KIA_Play_Slice_Model(958);
		break;
	case 129:
		KIA_Play_Slice_Model(934);
		break;
	case 131:
		KIA_Play_Slice_Model(945);
		break;
	case 136:
		KIA_Play_Actor_Dialogue(10, 240);
		KIA_Play_Actor_Dialogue(13, 200);
		KIA_Play_Actor_Dialogue(13, 210);
		KIA_Play_Actor_Dialogue(10, 260);
		KIA_Play_Actor_Dialogue(10, 270);
		break;
	case 139:
		KIA_Play_Actor_Dialogue(3, 360);
		KIA_Play_Actor_Dialogue(3, 380);
		break;
	case 140:
		KIA_Play_Actor_Dialogue(0, 2505);
		KIA_Play_Actor_Dialogue(3, 430);
		KIA_Play_Actor_Dialogue(3, 440);
		KIA_Play_Actor_Dialogue(0, 2530);
		KIA_Play_Actor_Dialogue(3, 450);
		KIA_Play_Actor_Dialogue(0, 2535);
		KIA_Play_Actor_Dialogue(3, 460);
		KIA_Play_Actor_Dialogue(3, 470);
		break;
	case 141:
		KIA_Play_Actor_Dialogue(6, 590);
		KIA_Play_Actor_Dialogue(6, 630);
		break;
	case 142:
		KIA_Play_Actor_Dialogue(6, 540);
		KIA_Play_Actor_Dialogue(6, 550);
		KIA_Play_Actor_Dialogue(0, 2550);
		KIA_Play_Actor_Dialogue(6, 560);
		break;
	case 143:
		KIA_Play_Actor_Dialogue(5, 530);
		KIA_Play_Actor_Dialogue(5, 540);
		break;
	case 144:
		KIA_Play_Actor_Dialogue(1, 700);
		KIA_Play_Actor_Dialogue(1, 750);
		KIA_Play_Actor_Dialogue(1, 760);
		break;
	case 145:
		KIA_Play_Slice_Model(960);
		break;
	case 146:
		KIA_Play_Slice_Model(932);
		break;
	case 147:
	case 148:
	case 149:
	case 150:
	case 151:
	case 152:
		v1 = Global_Variable_Query(48) - 1;
		if (v1 == 1) {
			KIA_Play_Slice_Model(988);
		} else if (v1 == 2) {
			KIA_Play_Slice_Model(990);
		} else if (v1 == 3) {
			KIA_Play_Slice_Model(991);
		} else if (v1 == 4) {
			KIA_Play_Slice_Model(993);
		}
		break;
	case 153:
		KIA_Play_Slice_Model(950);
		break;
	case 154:
		KIA_Play_Slice_Model(967);
		break;
	case 155:
		KIA_Play_Slice_Model(947);
		break;
	case 156:
		KIA_Play_Actor_Dialogue(19, 230);
		KIA_Play_Actor_Dialogue(19, 240);
		break;
	case 157:
		KIA_Play_Actor_Dialogue(19, 250);
		KIA_Play_Actor_Dialogue(19, 260);
		break;
	case 158:
		KIA_Play_Actor_Dialogue(19, 280);
		KIA_Play_Actor_Dialogue(0, 7350);
		KIA_Play_Actor_Dialogue(19, 290);
		KIA_Play_Actor_Dialogue(19, 300);
		KIA_Play_Actor_Dialogue(19, 310);
		break;
	case 162:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 163:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 164:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 165:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 166:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 167:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 168:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 169:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 170:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 171:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 172:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 173:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 174:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 175:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 176:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 177:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 178:
		KIA_Play_Actor_Dialogue(1, 3310);
		KIA_Play_Actor_Dialogue(1, 3320);
		KIA_Play_Actor_Dialogue(1, 3330);
		KIA_Play_Actor_Dialogue(1, 3350);
		KIA_Play_Actor_Dialogue(1, 3360);
		KIA_Play_Actor_Dialogue(1, 3370);
		KIA_Play_Actor_Dialogue(1, 3380);
		break;
	case 179:
		KIA_Play_Actor_Dialogue(1, 3390);
		KIA_Play_Actor_Dialogue(1, 3400);
		KIA_Play_Actor_Dialogue(1, 3410);
		KIA_Play_Actor_Dialogue(11, 1260);
		KIA_Play_Actor_Dialogue(1, 3420);
		KIA_Play_Actor_Dialogue(1, 3430);
		KIA_Play_Actor_Dialogue(1, 3440);
		KIA_Play_Actor_Dialogue(11, 1270);
		KIA_Play_Actor_Dialogue(1, 3450);
		KIA_Play_Actor_Dialogue(1, 3460);
		KIA_Play_Actor_Dialogue(11, 1280);
		KIA_Play_Actor_Dialogue(1, 3470);
		KIA_Play_Actor_Dialogue(11, 1300);
		KIA_Play_Actor_Dialogue(11, 1310);
		KIA_Play_Actor_Dialogue(1, 3480);
		KIA_Play_Actor_Dialogue(1, 3500);
		KIA_Play_Actor_Dialogue(11, 1320);
		KIA_Play_Actor_Dialogue(11, 1330);
		KIA_Play_Actor_Dialogue(1, 3510);
		KIA_Play_Actor_Dialogue(11, 1340);
		KIA_Play_Actor_Dialogue(1, 3520);
		KIA_Play_Actor_Dialogue(11, 1350);
		KIA_Play_Actor_Dialogue(1, 3530);
		KIA_Play_Actor_Dialogue(1, 3540);
		break;
	case 180:
		KIA_Play_Actor_Dialogue(1, 3550);
		KIA_Play_Actor_Dialogue(11, 1360);
		KIA_Play_Actor_Dialogue(11, 1370);
		KIA_Play_Actor_Dialogue(1, 3560);
		KIA_Play_Actor_Dialogue(1, 3570);
		break;
	case 181:
		KIA_Play_Actor_Dialogue(1, 3580);
		KIA_Play_Actor_Dialogue(11, 1400);
		KIA_Play_Actor_Dialogue(1, 3590);
		KIA_Play_Actor_Dialogue(11, 1410);
		KIA_Play_Actor_Dialogue(1, 3600);
		KIA_Play_Actor_Dialogue(11, 1420);
		KIA_Play_Actor_Dialogue(11, 1430);
		KIA_Play_Actor_Dialogue(1, 3610);
		KIA_Play_Actor_Dialogue(11, 1440);
		KIA_Play_Actor_Dialogue(1, 3620);
		KIA_Play_Actor_Dialogue(1, 3630);
		KIA_Play_Actor_Dialogue(11, 1450);
		KIA_Play_Actor_Dialogue(1, 3640);
		KIA_Play_Actor_Dialogue(11, 1460);
		KIA_Play_Actor_Dialogue(1, 3650);
		break;
	case 243:
		KIA_Play_Photograph(7);
		break;
	case 244:
		KIA_Play_Photograph(8);
		KIA_Play_Actor_Dialogue(99, 4110);
		break;
	case 245:
		KIA_Play_Photograph(15);
		break;
	case 246:
		KIA_Play_Photograph(17);
		break;
	case 247:
		KIA_Play_Photograph(18);
		break;
	case 248:
		KIA_Play_Photograph(1);
		KIA_Play_Actor_Dialogue(99, 4260);
		break;
	case 249:
		KIA_Play_Photograph(3);
		KIA_Play_Actor_Dialogue(99, 4230);
		break;
	case 250:
		KIA_Play_Photograph(2);
		KIA_Play_Actor_Dialogue(99, 4040);
		break;
	case 251:
		KIA_Play_Photograph(21);
		break;
	case 252:
		KIA_Play_Photograph(22);
		KIA_Play_Actor_Dialogue(99, 4180);
		break;
	case 253:
		KIA_Play_Photograph(23);
		break;
	case 254:
		KIA_Play_Photograph(24);
		break;
	case 255:
		KIA_Play_Photograph(26);
		break;
	case 256:
		KIA_Play_Photograph(27);
		break;
	case 257:
		KIA_Play_Slice_Model(975);
		break;
	case 258:
		KIA_Play_Photograph(0);
		break;
	case 259:
		KIA_Play_Photograph(28);
		break;
	case 260:
		KIA_Play_Photograph(19);
		break;
	case 261:
		KIA_Play_Photograph(32);
		break;
	case 262:
		KIA_Play_Photograph(30);
		KIA_Play_Actor_Dialogue(99, 4160);
		break;
	case 263:
		KIA_Play_Photograph(29);
		if (Query_Difficulty_Level() == 0) {
			KIA_Play_Actor_Dialogue(99, 4140);
		} else {
			KIA_Play_Actor_Dialogue(99, 4150);
		}
		break;
	case 264:
		KIA_Play_Slice_Model(985);
		KIA_Play_Actor_Dialogue(99, 1770);
		KIA_Play_Actor_Dialogue(99, 1150);
		KIA_Play_Actor_Dialogue(99, 1180);
		KIA_Play_Actor_Dialogue(99, 1190);
		break;
	case 265:
		KIA_Play_Slice_Model(986);
		break;
	case 269:
		KIA_Play_Actor_Dialogue(99, 730);
		KIA_Play_Actor_Dialogue(99, 740);
		KIA_Play_Actor_Dialogue(99, 750);
		KIA_Play_Actor_Dialogue(99, 760);
		break;
	case 270:
		KIA_Play_Actor_Dialogue(99, 670);
		KIA_Play_Actor_Dialogue(99, 680);
		KIA_Play_Actor_Dialogue(99, 700);
		KIA_Play_Actor_Dialogue(99, 710);
		KIA_Play_Actor_Dialogue(99, 720);
		break;
	case 271:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 430);
		break;
	case 272:
		KIA_Play_Actor_Dialogue(39, 420);
		KIA_Play_Actor_Dialogue(39, 440);
		break;
	case 273:
		KIA_Play_Actor_Dialogue(0, 1645);
		KIA_Play_Actor_Dialogue(6, 240);
		KIA_Play_Actor_Dialogue(6, 250);
		KIA_Play_Actor_Dialogue(0, 1675);
		KIA_Play_Actor_Dialogue(6, 260);
		KIA_Play_Actor_Dialogue(6, 270);
		break;
	case 274:
		KIA_Play_Photograph(35);
		break;
	case 275:
		KIA_Play_Photograph(36);
		KIA_Play_Actor_Dialogue(99, 4240);
		break;
	case 276:
		KIA_Play_Photograph(37);
		KIA_Play_Actor_Dialogue(99, 4220);
		break;
	case 277:
		KIA_Play_Photograph(38);
		break;
	case 278:
		KIA_Play_Actor_Dialogue(0, 5365);
		KIA_Play_Actor_Dialogue(57, 600);
		KIA_Play_Actor_Dialogue(0, 5370);
		KIA_Play_Actor_Dialogue(57, 610);
		break;
	case 279:
		KIA_Play_Actor_Dialogue(51, 0);
		KIA_Play_Actor_Dialogue(51, 10);
		KIA_Play_Actor_Dialogue(51, 20);
		KIA_Play_Actor_Dialogue(51, 30);
		break;
	case 280:
		KIA_Play_Actor_Dialogue(15, 630);
		KIA_Play_Actor_Dialogue(15, 640);
		KIA_Play_Actor_Dialogue(15, 650);
		break;
	}
}

} // End of namespace BladeRunner
