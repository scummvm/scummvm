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

#include "xeen/interface.h"
#include "xeen/dialogs_error.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

OutdoorDrawList::OutdoorDrawList() : _skySprite(_data[1]), _groundSprite(_data[2]) {
	_data[0] = DrawStruct(0, 8, 8);
	_data[1] = DrawStruct(1, 8, 25);
	_data[2] = DrawStruct(0, 8, 67);
	_data[3] = DrawStruct(0, 8, 67);
	_data[4] = DrawStruct(0, 38, 67);
	_data[5] = DrawStruct(0, 84, 67);
	_data[6] = DrawStruct(0, 134, 67);
	_data[7] = DrawStruct(0, 117, 67);
	_data[8] = DrawStruct(0, 117, 67);
	_data[9] = DrawStruct(0, 103, 67);
	_data[10] = DrawStruct(0, 8, 73);
	_data[11] = DrawStruct(0, 8, 73);
	_data[12] = DrawStruct(0, 30, 73);
	_data[13] = DrawStruct(0, 181, 73);
	_data[14] = DrawStruct(0, 154, 73);
	_data[15] = DrawStruct(0, 129, 73);
	_data[16] = DrawStruct(0, 87, 73);
	_data[17] = DrawStruct(0, 8, 81);
	_data[18] = DrawStruct(0, 8, 81);
	_data[19] = DrawStruct(0, 202, 81);
	_data[20] = DrawStruct(0, 145, 81);
	_data[21] = DrawStruct(0, 63, 81);
	_data[22] = DrawStruct(0, 8, 93);
	_data[23] = DrawStruct(0, 169, 93);
	_data[24] = DrawStruct(0, 31, 93);
	_data[25] = DrawStruct(0, 8, 109);
	_data[26] = DrawStruct(0, 201, 109);
	_data[27] = DrawStruct(0, 8, 109);
	_data[28] = DrawStruct(1, 65472, 61, 14, SPRFLAG_2000);
	_data[29] = DrawStruct(1, 65496, 61, 14, 0);
	_data[30] = DrawStruct(1, 65520, 61, 14, 0);
	_data[31] = DrawStruct(1, 8, 61, 14, 0);
	_data[32] = DrawStruct(1, 128, 61, 14, SPRFLAG_HORIZ_FLIPPED | SPRFLAG_2000);
	_data[33] = DrawStruct(1, 104, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[34] = DrawStruct(1, 80, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[35] = DrawStruct(1, 56, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[36] = DrawStruct(1, 32, 61, 14, 0);
	_data[37] = DrawStruct(0, 65527, 61, 14, 0);
	_data[38] = DrawStruct(0, 65478, 61, 14, 0);
	_data[39] = DrawStruct(0, 40, 61, 14, 0);
	_data[40] = DrawStruct(0, 65454, 61, 14, 0);
	_data[41] = DrawStruct(0, 64, 61, 14, 0);
	_data[42] = DrawStruct(0, 65495, 61, 14, 0);
	_data[43] = DrawStruct(0, 65510, 61, 14, 0);
	_data[44] = DrawStruct(0, 65502, 61, 14, 0);
	_data[45] = DrawStruct(0, 65520, 61, 14, 0);
	_data[46] = DrawStruct(0, 23, 61, 14, 0);
	_data[47] = DrawStruct(0, 16, 61, 14, 0);
	_data[48] = DrawStruct(0, 65478, 61, 14, 0);
	_data[49] = DrawStruct(0, 40, 61, 14, 0);
	_data[50] = DrawStruct(0, 65519, 61, 14, 0);
	_data[51] = DrawStruct(0, 65535, 58, 14, 0);
	_data[52] = DrawStruct(0, 65527, 58, 14, 0);
	_data[53] = DrawStruct(0, 72, 58, 12, 0);
	_data[54] = DrawStruct(0, 72, 58, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[55] = DrawStruct(0, 69, 63, 12, 0);
	_data[56] = DrawStruct(0, 75, 63, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[57] = DrawStruct(0, 73, 53, 12, 0);
	_data[58] = DrawStruct(0, 71, 53, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[59] = DrawStruct(0, 80, 57, 12, 0);
	_data[60] = DrawStruct(0, 64, 57, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[61] = DrawStruct(2, 65525, 54, 8, 0);
	_data[62] = DrawStruct(1, 65515, 54, 11, 0);
	_data[63] = DrawStruct(2, 165, 54, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[64] = DrawStruct(1, 86, 54, 11, SPRFLAG_HORIZ_FLIPPED);
	_data[65] = DrawStruct(1, 33, 54, 11, 0);
	_data[66] = DrawStruct(0, 65528, 54, 12, 0);
	_data[67] = DrawStruct(0, 65463, 54, 12, 0);
	_data[68] = DrawStruct(0, 57, 54, 12, 0);
	_data[69] = DrawStruct(0, 65471, 54, 12, 0);
	_data[70] = DrawStruct(0, 65455, 54, 12, 0);
	_data[71] = DrawStruct(0, 49, 54, 12, 0);
	_data[72] = DrawStruct(0, 65, 54, 12, 0);
	_data[73] = DrawStruct(0, 65512, 54, 12, 0);
	_data[74] = DrawStruct(0, 9, 50, 12, 0);
	_data[75] = DrawStruct(0, 65528, 50, 12, 0);
	_data[76] = DrawStruct(0, 72, 53, 8, 0);
	_data[77] = DrawStruct(0, 72, 53, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[78] = DrawStruct(0, 77, 58, 8, 0);
	_data[79] = DrawStruct(0, 67, 58, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[80] = DrawStruct(0, 81, 47, 8, 0);
	_data[81] = DrawStruct(0, 63, 47, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[82] = DrawStruct(0, 94, 52, 8, 0);
	_data[83] = DrawStruct(0, 50, 52, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[84] = DrawStruct(2, 8, 40);
	_data[85] = DrawStruct(2, 146, 40, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[86] = DrawStruct(1, 32, 40, 6, 0);
	_data[87] = DrawStruct(0, 65529, 30, 7, 0);
	_data[88] = DrawStruct(0, 65424, 30, 7, SPRFLAG_2000);
	_data[89] = DrawStruct(0, 98, 30, 7, SPRFLAG_2000);
	_data[90] = DrawStruct(0, 65424, 30, 8, SPRFLAG_2000);
	_data[91] = DrawStruct(0, 98, 30, 8, SPRFLAG_2000);
	_data[92] = DrawStruct(0, 65498, 30, 8, 0);
	_data[93] = DrawStruct(0, 25, 30, 8, 0);
	_data[94] = DrawStruct(0, 65529, 30, 8, 0);
	_data[95] = DrawStruct(0, 72, 48, 4, 0);
	_data[96] = DrawStruct(0, 72, 48, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[97] = DrawStruct(0, 85, 53, 4, 0);
	_data[98] = DrawStruct(0, 59, 53, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[99] = DrawStruct(0, 89, 41, 4, 0);
	_data[100] = DrawStruct(0, 55, 41, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[101] = DrawStruct(0, 106, 47, 4, 0);
	_data[102] = DrawStruct(0, 38, 47, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[103] = DrawStruct(0, 8, 24);
	_data[104] = DrawStruct(0, 169, 24, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[105] = DrawStruct(1, 32, 24);
	_data[106] = DrawStruct(0, 65513, 40, 0, SPRFLAG_2000);
	_data[107] = DrawStruct(0, 200, 40, 0, SPRFLAG_HORIZ_FLIPPED | SPRFLAG_2000);
	_data[108] = DrawStruct(0, 8, 47);
	_data[109] = DrawStruct(0, 169, 47, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[110] = DrawStruct(1, 65480, 65532, 32768, SPRFLAG_4000 | SPRFLAG_2000);
	_data[111] = DrawStruct(0, 65531, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[112] = DrawStruct(0, 65469, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[113] = DrawStruct(0, 44, 73);
	_data[114] = DrawStruct(0, 44, 73);
	_data[115] = DrawStruct(0, 58, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[116] = DrawStruct(0, 169, 73);
	_data[117] = DrawStruct(0, 169, 73);
	_data[118] = DrawStruct(0, 65531, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[119] = DrawStruct(0, 110, 73);
	_data[120] = DrawStruct(0, 110, 73);
	_data[121] = DrawStruct(0, 65531, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[122] = DrawStruct(0, 110, 73);
	_data[123] = DrawStruct(0, 110, 73);
	_data[124] = DrawStruct(0, 72, 43);
	_data[125] = DrawStruct(0, 72, 43, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[126] = DrawStruct(0, 93, 48);
	_data[127] = DrawStruct(0, 51, 48, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[128] = DrawStruct(0, 97, 36);
	_data[129] = DrawStruct(0, 47, 36, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[130] = DrawStruct(0, 118, 42);
	_data[131] = DrawStruct(0, 26, 42, 0, SPRFLAG_HORIZ_FLIPPED);
}

/*------------------------------------------------------------------------*/

IndoorDrawList::IndoorDrawList() : _skySprite(_data[1]), _groundSprite(_data[2]),
	_objects0(_data[149]), _objects1(_data[125]), _objects2(_data[126]),
	_objects3(_data[127]), _objects4(_data[97]), _objects5(_data[98]),
	_objects6(_data[99]), _objects7(_data[55]), _objects8(_data[56]),
	_objects9(_data[58]), _objects10(_data[57]), _objects11(_data[59]) {
	_data[0] = DrawStruct(0, 8, 8);
	_data[1] = DrawStruct(1, 8, 25);
	_data[2] = DrawStruct(0, 8, 67);
	_data[3] = DrawStruct(0, 8, 67);
	_data[4] = DrawStruct(0, 38, 67);
	_data[5] = DrawStruct(0, 84, 67);
	_data[6] = DrawStruct(0, 134, 67);
	_data[7] = DrawStruct(0, 117, 67);
	_data[8] = DrawStruct(0, 117, 67);
	_data[9] = DrawStruct(0, 103, 67);
	_data[10] = DrawStruct(0, 8, 73);
	_data[11] = DrawStruct(0, 8, 73);
	_data[12] = DrawStruct(0, 30, 73);
	_data[13] = DrawStruct(0, 181, 73);
	_data[14] = DrawStruct(0, 154, 73);
	_data[15] = DrawStruct(0, 129, 73);
	_data[16] = DrawStruct(0, 87, 73);
	_data[17] = DrawStruct(0, 8, 81);
	_data[18] = DrawStruct(0, 8, 81);
	_data[19] = DrawStruct(0, 202, 81);
	_data[20] = DrawStruct(0, 145, 81);
	_data[21] = DrawStruct(0, 63, 81);
	_data[22] = DrawStruct(0, 8, 93);
	_data[23] = DrawStruct(0, 169, 93);
	_data[24] = DrawStruct(0, 31, 93);
	_data[25] = DrawStruct(0, 8, 109);
	_data[26] = DrawStruct(0, 201, 109);
	_data[27] = DrawStruct(0, 8, 109);
	_data[28] = DrawStruct(7, 8, 64);
	_data[29] = DrawStruct(22, 32, 60);
	_data[30] = DrawStruct(20, 56, 60);
	_data[31] = DrawStruct(18, 80, 60);
	_data[32] = DrawStruct(16, 104, 60);
	_data[33] = DrawStruct(23, 152, 60, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[34] = DrawStruct(21, 144, 60, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[35] = DrawStruct(19, 131, 60, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[36] = DrawStruct(17, 120, 60, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[37] = DrawStruct(14, 8, 60);
	_data[38] = DrawStruct(12, 32, 60);
	_data[39] = DrawStruct(10, 56, 60);
	_data[40] = DrawStruct(14, 80, 60);
	_data[41] = DrawStruct(14, 104, 60);
	_data[42] = DrawStruct(14, 128, 60);
	_data[43] = DrawStruct(14, 152, 60);
	_data[44] = DrawStruct(8, 176, 60);
	_data[45] = DrawStruct(8, 200, 60);
	_data[46] = DrawStruct(0, 65472, 61, 14, 0);
	_data[47] = DrawStruct(0, 65496, 61, 14, 0);
	_data[48] = DrawStruct(0, 65520, 61, 14, 0);
	_data[49] = DrawStruct(0, 8, 61, 14, 0);
	_data[50] = DrawStruct(0, 32, 61, 14, 0);
	_data[51] = DrawStruct(0, 56, 61, 14, 0);
	_data[52] = DrawStruct(0, 80, 61, 14, 0);
	_data[53] = DrawStruct(0, 104, 61, 14, 0);
	_data[54] = DrawStruct(0, 128, 61, 14, 0);
	_data[55] = DrawStruct(0, 65527, 58, 14, 0);
	_data[56] = DrawStruct(0, 65502, 58, 14, 0);
	_data[57] = DrawStruct(0, 16, 58, 14, 0);
	_data[58] = DrawStruct(0, 65478, 58, 14, 0);
	_data[59] = DrawStruct(0, 40, 58, 14, 0);
	_data[60] = DrawStruct(0, 65495, 58, 14, 0);
	_data[61] = DrawStruct(0, 65510, 58, 14, 0);
	_data[62] = DrawStruct(0, 65502, 58, 14, 0);
	_data[63] = DrawStruct(0, 65520, 58, 14, 0);
	_data[64] = DrawStruct(0, 23, 58, 14, 0);
	_data[65] = DrawStruct(0, 16, 58, 14, 0);
	_data[66] = DrawStruct(0, 65478, 58, 14, 0);
	_data[67] = DrawStruct(0, 40, 58, 14, 0);
	_data[68] = DrawStruct(0, 65519, 58, 14, 0);
	_data[69] = DrawStruct(0, 65535, 58, 14, 0);
	_data[70] = DrawStruct(0, 65527, 58, 14, 0);
	_data[71] = DrawStruct(14, 8, 58);
	_data[72] = DrawStruct(12, 8, 55);
	_data[73] = DrawStruct(10, 32, 52);
	_data[74] = DrawStruct(14, 88, 52);
	_data[75] = DrawStruct(14, 128, 52, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[76] = DrawStruct(14, 152, 52, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[77] = DrawStruct(0, 176, 55, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[78] = DrawStruct(0, 200, 58, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[79] = DrawStruct(0, 72, 58, 12, 0);
	_data[80] = DrawStruct(0, 72, 58, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[81] = DrawStruct(0, 69, 63, 12, 0);
	_data[82] = DrawStruct(0, 75, 63, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[83] = DrawStruct(0, 73, 53, 12, 0);
	_data[84] = DrawStruct(0, 71, 53, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[85] = DrawStruct(0, 80, 57, 12, 0);
	_data[86] = DrawStruct(0, 64, 57, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[87] = DrawStruct(7, 65512, 52, 0, SPRFLAG_2000);
	_data[88] = DrawStruct(7, 32, 52);
	_data[89] = DrawStruct(7, 88, 52);
	_data[90] = DrawStruct(0, 144, 52);
	_data[91] = DrawStruct(0, 200, 52, 0, SPRFLAG_2000);
	_data[92] = DrawStruct(0, 65457, 52, 11, SPRFLAG_2000);
	_data[93] = DrawStruct(0, 65509, 52, 11, 0);
	_data[94] = DrawStruct(0, 32, 52, 11, 0);
	_data[95] = DrawStruct(0, 89, 52, 11, 0);
	_data[96] = DrawStruct(0, 145, 52, 11, SPRFLAG_2000);
	_data[97] = DrawStruct(0, 65528, 50, 12, 0);
	_data[98] = DrawStruct(0, 65471, 50, 12, 0);
	_data[99] = DrawStruct(0, 49, 50, 12, 0);
	_data[100] = DrawStruct(0, 65471, 50, 12, 0);
	_data[101] = DrawStruct(0, 65455, 50, 12, 0);
	_data[102] = DrawStruct(0, 49, 50, 12, 0);
	_data[103] = DrawStruct(0, 65, 50, 12, 0);
	_data[104] = DrawStruct(0, 65512, 50, 12, 0);
	_data[105] = DrawStruct(0, 9, 50, 12, 0);
	_data[106] = DrawStruct(0, 65528, 50, 12, 0);
	_data[107] = DrawStruct(7, 8, 48);
	_data[108] = DrawStruct(7, 64, 40);
	_data[109] = DrawStruct(6, 144, 40, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[110] = DrawStruct(6, 200, 48, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[111] = DrawStruct(0, 72, 53, 8, 0);
	_data[112] = DrawStruct(0, 72, 53, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[113] = DrawStruct(0, 77, 58, 8, 0);
	_data[114] = DrawStruct(0, 67, 58, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[115] = DrawStruct(0, 81, 47, 8, 0);
	_data[116] = DrawStruct(0, 63, 47, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[117] = DrawStruct(0, 94, 52, 8, 0);
	_data[118] = DrawStruct(0, 50, 52, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[119] = DrawStruct(6, 65496, 40, 0, SPRFLAG_2000);
	_data[120] = DrawStruct(6, 64, 40);
	_data[121] = DrawStruct(0, 168, 40, 0, SPRFLAG_2000);
	_data[122] = DrawStruct(0, 65464, 40, 6, SPRFLAG_2000);
	_data[123] = DrawStruct(0, 32, 40, 6, 0);
	_data[124] = DrawStruct(0, 137, 40, 6, SPRFLAG_2000);
	_data[125] = DrawStruct(0, 65529, 25, 7, 0);
	_data[126] = DrawStruct(0, 65424, 25, 7, SPRFLAG_2000);
	_data[127] = DrawStruct(0, 98, 25, 7, SPRFLAG_2000);
	_data[128] = DrawStruct(0, 65424, 29, 8, SPRFLAG_2000);
	_data[129] = DrawStruct(0, 98, 29, 8, SPRFLAG_2000);
	_data[130] = DrawStruct(0, 65498, 29, 8, 0);
	_data[131] = DrawStruct(0, 25, 29, 8, 0);
	_data[132] = DrawStruct(0, 65529, 29, 8, 0);
	_data[133] = DrawStruct(6, 32, 24);
	_data[134] = DrawStruct(0, 168, 24, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[135] = DrawStruct(0, 72, 48, 4, 0);
	_data[136] = DrawStruct(0, 72, 48, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[137] = DrawStruct(0, 85, 53, 4, 0);
	_data[138] = DrawStruct(0, 59, 53, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[139] = DrawStruct(0, 89, 41, 4, 0);
	_data[140] = DrawStruct(0, 55, 41, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[141] = DrawStruct(0, 106, 47, 4, 0);
	_data[142] = DrawStruct(0, 38, 47, 4, SPRFLAG_HORIZ_FLIPPED);
	_data[143] = DrawStruct(0, 65400, 24, 0, SPRFLAG_2000);
	_data[144] = DrawStruct(0, 8, 12);
	_data[145] = DrawStruct(0, 32, 24);
	_data[146] = DrawStruct(0, 200, 12, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[147] = DrawStruct(0, 200, 24, 0, SPRFLAG_2000);
	_data[148] = DrawStruct(0, 32, 24);
	_data[149] = DrawStruct(0, 65531, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[150] = DrawStruct(0, 65469, 10, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[151] = DrawStruct(0, 44, 73);
	_data[152] = DrawStruct(0, 44, 73);
	_data[153] = DrawStruct(0, 58, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[154] = DrawStruct(0, 169, 73);
	_data[155] = DrawStruct(0, 169, 73);
	_data[156] = DrawStruct(0, 65531, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[157] = DrawStruct(0, 110, 73);
	_data[158] = DrawStruct(0, 110, 73);
	_data[159] = DrawStruct(0, 65531, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[160] = DrawStruct(0, 110, 73);
	_data[161] = DrawStruct(0, 110, 73);
	_data[162] = DrawStruct(0, 72, 43);
	_data[163] = DrawStruct(0, 72, 43, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[164] = DrawStruct(0, 93, 48);
	_data[165] = DrawStruct(0, 51, 48, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[166] = DrawStruct(0, 97, 36);
	_data[167] = DrawStruct(0, 47, 36, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[168] = DrawStruct(0, 118, 42);
	_data[169] = DrawStruct(0, 26, 42, 0, SPRFLAG_HORIZ_FLIPPED);
}

/*------------------------------------------------------------------------*/

Interface::Interface(XeenEngine *vm) : ButtonContainer(), _vm(vm) {
	Common::fill(&_partyFaces[0], &_partyFaces[MAX_ACTIVE_PARTY], nullptr);
	_batUIFrame = 0;
	_spotDoorsUIFrame = 0;
	_dangerSenseUIFrame = 0;
	_face1UIFrame = 0;
	_face2UIFrame = 0;
	_blessedUIFrame = 0;
	_powerShieldUIFrame = 0;
	_holyBonusUIFrame = 0;
	_heroismUIFrame = 0;
	_flipUIFrame = 0;
	_newDay = false;
	_buttonsLoaded = false;
	_hiliteChar = -1;
	_intrIndex1 = 0;
	_flipWtr = false;
	_flag1 = false;
	_flag2 = false;
	_tillMove = 0;
	_objNumber = 0;

	Common::fill(&_combatCharIds[0], &_combatCharIds[8], 0);
	Common::fill(&_wp[0], &_wp[20], 0);
	Common::fill(&_wo[0], &_wo[308], 0);

	initDrawStructs();
}

void Interface::initDrawStructs() {
	_faceDrawStructs[0] = DrawStruct(0, 0, 0);
	_faceDrawStructs[1] = DrawStruct(0, 101, 0);
	_faceDrawStructs[2] = DrawStruct(0, 0, 43);
	_faceDrawStructs[3] = DrawStruct(0, 101, 43);

	_mainList[0] = DrawStruct(7, 232, 74);
	_mainList[1] = DrawStruct(0, 235, 75);
	_mainList[2] = DrawStruct(2, 260, 75);
	_mainList[3] = DrawStruct(4, 286, 75);
	_mainList[4] = DrawStruct(6, 235, 96);
	_mainList[5] = DrawStruct(8, 260, 96);
	_mainList[6] = DrawStruct(10, 286, 96);
	_mainList[7] = DrawStruct(12, 235, 117);
	_mainList[8] = DrawStruct(14, 260, 117);
	_mainList[9] = DrawStruct(16, 286, 117);
	_mainList[10] = DrawStruct(20, 235, 148);
	_mainList[11] = DrawStruct(22, 260, 148);
	_mainList[12] = DrawStruct(24, 286, 148);
	_mainList[13] = DrawStruct(26, 235, 169);
	_mainList[14] = DrawStruct(28, 260, 169);
	_mainList[15] = DrawStruct(30, 286, 169);
}

void Interface::setup() {
	_globalSprites.load("global.icn");
	_borderSprites.load("border.icn");
	_spellFxSprites.load("spellfx.icn");
	_fecpSprites.load("fecp.brd");
	_blessSprites.load("bless.icn");
	_restoreSprites.load("restorex.icn");
	_hpSprites.load("hpbars.icn");
	_uiSprites.load("inn.icn");

	// Get mappings to the active characters in the party
	_vm->_party._activeParty.resize(_vm->_party._partyCount);
	for (int i = 0; i < _vm->_party._partyCount; ++i) {
		_vm->_party._activeParty[i] = _vm->_roster[_vm->_party._partyMembers[i]];
	}

	_newDay = _vm->_party._minutes >= 300;
}

void Interface::manageCharacters(bool soundPlayed) {
	Screen &screen = *_vm->_screen;
	EventsManager &events = *_vm->_events;
	bool flag = false;

start:
	if (_vm->_party._mazeId != 0) {
		_vm->_mode = MODE_0;
		_buttonsLoaded = true;
	} else {
		if (!soundPlayed) {
			warning("TODO: loadSound?");
		}

		if (!_partyFaces[0]) {
			// Xeen only uses 24 of possible 30 character slots
			loadCharIcons();

			for (int i = 0; i < _vm->_party._partyCount; ++i)
				_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
		}

		_vm->_mode = MODE_1;
		Common::Array<int> xeenSideChars;

		// Build up a list of characters on the same Xeen side being loaded
		for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
			PlayerStruct &player = _vm->_roster[i];
			if (player._name.empty() || player._xeenSide != _vm->_loadDarkSide)
				continue;

			xeenSideChars.push_back(i);
		}

		// Add in buttons for the UI
		_interfaceText = "";
		_buttonsLoaded = true;
		addButton(Common::Rect(16, 100, 40, 120), 242, &_uiSprites, true);
		addButton(Common::Rect(52, 100, 76, 120), 243, &_uiSprites, true);
		addButton(Common::Rect(87, 100, 111, 120), 68, &_uiSprites, true);
		addButton(Common::Rect(122, 100, 146, 120), 82, &_uiSprites, true);
		addButton(Common::Rect(157, 100, 181, 120), 67, &_uiSprites, true);
		addButton(Common::Rect(192, 100, 216, 120), 88, &_uiSprites, true);
		addButton(Common::Rect(), 27, &_uiSprites, false);
		addButton(Common::Rect(16, 16, 48, 48), 49, &_uiSprites, false);
		addButton(Common::Rect(117, 16, 139, 48), 50, &_uiSprites, false);
		addButton(Common::Rect(16, 59, 48, 81), 51, &_uiSprites, false);
		addButton(Common::Rect(117, 59, 149, 81), 52, &_uiSprites, false);

		setupBackground();
		Window &w = screen._windows[11];
		w.open();
		setupFaces(0, xeenSideChars, false);
		w.writeString(_interfaceText);
		w.drawList(&_faceDrawStructs[0], 4);

		_uiSprites.draw(w, 0, Common::Point(16, 100));
		_uiSprites.draw(w, 2, Common::Point(52, 100));
		_uiSprites.draw(w, 4, Common::Point(87, 100));
		_uiSprites.draw(w, 6, Common::Point(122, 100));
		_uiSprites.draw(w, 8, Common::Point(157, 100));
		_uiSprites.draw(w, 10, Common::Point(192, 100));

		screen.loadPalette("mm4.pal");

		if (flag) {
			screen._windows[0].update();
			events.setCursor(0);
			screen.fadeIn(4);
		} else {
			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeOut(4);
				screen._windows[0].update();
			}

			doScroll(_vm, false, false);
			events.setCursor(0);

			if (_vm->getGameID() == GType_DarkSide) {
				screen.fadeIn(4);
			}
		}

		// TODO
		bool breakFlag = false;
		while (!_vm->shouldQuit() && !breakFlag) {
			events.pollEventsAndWait();
			checkEvents(_vm);

			switch (_buttonValue) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_e:
			case Common::KEYCODE_x:
				if (_vm->_party._partyCount == 0) {
					ErrorScroll::show(_vm, NO_ONE_TO_ADVENTURE_WITH);
				} else {
					if (_vm->_mode != MODE_0) {
						for (_intrIndex1 = 4; _intrIndex1 >= 0; --_intrIndex1) {
							events.updateGameCounter();
							drawViewBackground(_intrIndex1);
							w.update();

							while (events.timeElapsed() < 1)
								events.pollEventsAndWait();
						}
					}

					w.close();
					_vm->_party._realPartyCount = _vm->_party._partyCount;
					_vm->_party._mazeId = _vm->_party._priorMazeId;

					_vm->_party.copyPartyToRoster(_vm->_roster);
					_vm->_saves->writeCharFile();
					breakFlag = true;
					break;
				}
				break;
			case Common::KEYCODE_1:
				break;
			case Common::KEYCODE_2:
				break;
			case Common::KEYCODE_3:
				break;
			case Common::KEYCODE_4:
				break;
			case Common::KEYCODE_c:
				if (xeenSideChars.size() == 24) {
					ErrorScroll::show(_vm, YOUR_ROSTER_IS_FULL);
				} else {
					screen.fadeOut(4);
					w.close();
					moveCharacterToRoster();
					_vm->_saves->writeCharFile();
					screen.fadeOut(4);
					flag = true;
					_buttonsLoaded = true;
					goto start;
				}
				break;
			case Common::KEYCODE_d:
				break;
			case Common::KEYCODE_r:
				if (_vm->_party._partyCount > 0) {
					// TODO
				}
				break;
			case 201:
				// TODO
				break;
			case 202:
				// TODO
				break;
			case 203:
				// TODO
				break;
			case 204:
				// TODO
				break;
			case 205:
				// TODO
				break;
			case 206:
				// TODO
				break;
			case 242:
				// TODO
				break;
			case 243:
				// TODO
				break;
			default:
				break;
			}
		}
	}

	for (int i = 0; i < TOTAL_CHARACTERS; ++i)
		_charFaces[i].clear();
	_globalSprites.clear();
	_borderSprites.clear();
	_spellFxSprites.clear();
	_fecpSprites.clear();
	_blessSprites.clear();
	_restoreSprites.clear();
	_hpSprites.clear();
	_uiSprites.clear();
}

void Interface::loadCharIcons() {
	for (int i = 0; i < XEEN_TOTAL_CHARACTERS; ++i) {
		// Load new character resource
		Common::String name = Common::String::format("char%02d.fac", i + 1);
		_charFaces[i].load(name);
	}

	_dseFace.load("dse.fac");
}

void Interface::loadPartyIcons() {
	for (int i = 0; i < _vm->_party._partyCount; ++i)
		_partyFaces[i] = &_charFaces[_vm->_party._partyMembers[i]];
}

void Interface::setupBackground() {
	_vm->_screen->loadBackground("back.raw");
	assembleBorder();
}

void Interface::assembleBorder() {
	Screen &screen = *_vm->_screen;
	Window &gameWindow = screen._windows[28];

	// Draw the outer frame
	_globalSprites.draw(gameWindow, 0);

	// Draw the animating bat character used to show when levitate is active
	_borderSprites.draw(screen, _vm->_party._levitateActive ? _batUIFrame + 16 : 16);
	_batUIFrame = (_batUIFrame + 1) % 12;

	// Draw UI element to indicate whether can spot hidden doors
	_borderSprites.draw(screen,
		(_vm->_thinWall && _vm->_party.checkSkill(SPOT_DOORS)) ? _spotDoorsUIFrame + 28 : 28,
		Common::Point(194, 91));
	_spotDoorsUIFrame = (_spotDoorsUIFrame + 1) % 12;

	// Draw UI element to indicate whether can sense danger
	_borderSprites.draw(screen,
		(_vm->_dangerSenseAllowed && _vm->_party.checkSkill(DANGER_SENSE)) ? _spotDoorsUIFrame + 40 : 40,
		Common::Point(107, 9));
	_dangerSenseUIFrame = (_dangerSenseUIFrame + 1) % 12;

	// Handle the face UI elements for indicating clairvoyance status
	_face1UIFrame = (_face1UIFrame + 1) % 4;
	if (_vm->_face1State == 0)
		_face1UIFrame += 4;
	else if (_vm->_face1State == 2)
		_face1UIFrame = 0;

	_face2UIFrame = (_face2UIFrame + 1) % 4 + 12;
	if (_vm->_face2State == 0)
		_face2UIFrame += 252;
	else if (_vm->_face2State == 2)
		_face2UIFrame = 0;

	if (!_vm->_party._clairvoyanceActive) {
		_face1UIFrame = 0;
		_face2UIFrame = 8;
	}

	_borderSprites.draw(screen, _face1UIFrame, Common::Point(0, 32));
	_borderSprites.draw(screen,
		screen._windows[10]._enabled || screen._windows[2]._enabled ?
		52 : _face2UIFrame,
		Common::Point(215, 32));

	// Draw resistence indicators
	if (!screen._windows[10]._enabled && !screen._windows[2]._enabled
		&& screen._windows[38]._enabled) {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 1 : 0,
			Common::Point(2, 2));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 3 : 2,
			Common::Point(219, 2));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 5 : 4,
			Common::Point(2, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 7 : 6,
			Common::Point(219, 134));
	} else {
		_fecpSprites.draw(screen, _vm->_party._fireResistence ? 9 : 8,
			Common::Point(8, 8));
		_fecpSprites.draw(screen, _vm->_party._electricityResistence ? 10 : 11,
			Common::Point(219, 8));
		_fecpSprites.draw(screen, _vm->_party._coldResistence ? 12 : 13,
			Common::Point(8, 134));
		_fecpSprites.draw(screen, _vm->_party._poisonResistence ? 14 : 15,
			Common::Point(219, 134));
	}

	// Draw UI element for blessed
	_blessSprites.draw(screen, 16, Common::Point(33, 137));
	if (_vm->_party._blessedActive) {
		_blessedUIFrame = (_blessedUIFrame + 1) % 4;
		_blessSprites.draw(screen, _blessedUIFrame, Common::Point(33, 137));
	}

	// Draw UI element for power shield
	if (_vm->_party._powerShieldActive) {
		_powerShieldUIFrame = (_powerShieldUIFrame + 1) % 4;
		_blessSprites.draw(screen, _powerShieldUIFrame + 4,
			Common::Point(55, 137));
	}

	// Draw UI element for holy bonus
	if (_vm->_party._holyBonusActive) {
		_holyBonusUIFrame = (_holyBonusUIFrame + 1) % 4;
		_blessSprites.draw(screen, _holyBonusUIFrame + 8, Common::Point(160, 137));
	}

	// Draw UI element for heroism
	if (_vm->_party._heroismActive) {
		_heroismUIFrame = (_heroismUIFrame + 1) % 4;
		_blessSprites.draw(screen, _heroismUIFrame + 12, Common::Point(182, 137));
	}

	// Draw direction character if direction sense is active
	if (_vm->_party.checkSkill(DIRECTION_SENSE) && !_vm->_noDirectionSense) {
		const char *dirText = DIRECTION_TEXT[_vm->_party._mazeDirection];
		Common::String msg = Common::String::format(
			"\002""08\003""c\013""139\011""116%c\014""d\001", *dirText);
		screen._windows[0].writeString(msg);
	}

	// Draw view frame
	if (screen._windows[12]._enabled)
		screen._windows[12].frame();
}

void Interface::setupFaces(int charIndex, Common::Array<int> xeenSideChars, bool updateFlag) {
	Common::String playerNames[4];
	Common::String playerRaces[4];
	Common::String playerSex[4];
	Common::String playerClass[4];
	int posIndex;
	int charId;

	for (posIndex = 0; posIndex < 4; ++posIndex) {
		int charId = xeenSideChars[charIndex];
		bool isInParty = _vm->_party.isInParty(charId);

		if (charId == 0xff) {
			while ((int)_buttons.size() > (7 + posIndex))
				_buttons.remove_at(_buttons.size() - 1);
			break;
		}

		Common::Rect &b = _buttons[7 + posIndex]._bounds;
		b.moveTo((posIndex & 1) ? 117 : 16, b.top);
		PlayerStruct &ps = _vm->_roster[xeenSideChars[charIndex + posIndex]];
		playerNames[posIndex] = isInParty ? IN_PARTY : ps._name;
		playerRaces[posIndex] = RACE_NAMES[ps._race];
		playerSex[posIndex] = SEX_NAMES[ps._sex];
		playerClass[posIndex] = CLASS_NAMES[ps._class];
	}

	charIconsPrint(updateFlag);

	// Set up the sprite set to use for each face
	charId = xeenSideChars[charIndex];
	_faceDrawStructs[0]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 1];
	_faceDrawStructs[1]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 2];
	_faceDrawStructs[2]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];
	charId = xeenSideChars[charIndex + 3];
	_faceDrawStructs[3]._sprites = (charId == 0xff) ? (SpriteResource *)nullptr : &_charFaces[charId];

	_interfaceText = Common::String::format(PARTY_DETAILS,
		playerNames[0].c_str(), playerRaces[0].c_str(), playerSex[0].c_str(), playerClass[0].c_str(),
		playerNames[1].c_str(), playerRaces[1].c_str(), playerSex[1].c_str(), playerClass[1].c_str(),
		playerNames[2].c_str(), playerRaces[2].c_str(), playerSex[2].c_str(), playerClass[2].c_str(),
		playerNames[3].c_str(), playerRaces[3].c_str(), playerSex[3].c_str(), playerClass[3].c_str()
	);
}

void Interface::charIconsPrint(bool updateFlag) {
	Screen &screen = *_vm->_screen;
	bool stateFlag = _vm->_mode == MODE_2;
	_restoreSprites.draw(screen, 0, Common::Point(8, 149));

	// Handle drawing the party faces
	for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount : 
			_vm->_party._partyCount); ++idx) {
		int charIndex = stateFlag ? _combatCharIds[idx] : idx;
		PlayerStruct &ps = _vm->_party._activeParty[charIndex];
		Condition charCondition = ps.worstCondition();
		int charFrame = FACE_CONDITION_FRAMES[charCondition];
		
		SpriteResource *sprites = (charFrame > 4 && !_charFaces[0].empty()) ?
			&_dseFace : _partyFaces[charIndex];
		if (charFrame > 4)
			charFrame -= 5;

		sprites->draw(screen, charFrame, Common::Point(CHAR_FACES_X[idx], 150));
	}

	if (!_hpSprites.empty()) {
		for (int idx = 0; idx < (stateFlag ? _vm->_party._combatPartyCount :
			_vm->_party._partyCount); ++idx) {
			int charIndex = stateFlag ? _combatCharIds[idx] : idx;
			PlayerStruct &ps = _vm->_party._activeParty[charIndex];

			// Draw the Hp bar
			int maxHp = ps.getMaxHp();
			int frame;
			if (ps._currentHp < 1)
				frame = 4;
			else if (ps._currentHp > maxHp)
				frame = 3;
			else if (ps._currentHp == maxHp)
				frame = 0;
			else if (ps._currentHp < (maxHp / 4))
				frame = 2;
			else
				frame = 1;

			_hpSprites.draw(screen, frame, Common::Point(HP_BARS_X[idx], 182));
		}
	}

	if (_hiliteChar != -1)
		_globalSprites.draw(screen, 8, Common::Point(CHAR_FACES_X[_hiliteChar] - 1, 149));

	if (updateFlag)
		screen._windows[33].update();
}

void Interface::drawViewBackground(int bgType) {
	if (bgType >= 4)
		return;

	if (bgType == 0) {
		// Totally black background
		_vm->_screen->fillRect(Common::Rect(8, 8, 224, 140), 0);
	} else {
		const byte *lookup = BACKGROUND_XLAT + bgType;
		for (int yp = 8; yp < 140; ++yp) {
			byte *destP = (byte *)_vm->_screen->getBasePtr(8, yp);
			for (int xp = 8; xp < 224; ++xp, ++destP)
				*destP = lookup[*destP];
		}
	}
}

void Interface::moveCharacterToRoster() {
	error("TODO");
}

void Interface::draw3d(bool flag) {
	Screen &screen = *_vm->_screen;
//	EventsManager &events = *_vm->_events;

	if (!screen._windows[11]._enabled)
		return;

	_flipUIFrame = (_flipUIFrame + 1) % 4;
	if (_flipUIFrame == 0)
		_flipWtr = !_flipWtr;
	if (_tillMove && (_vm->_mode == MODE_1 || _vm->_mode == MODE_2) &&
			!_flag1 && _vm->_moveMonsters) {
		if (--_tillMove == 0)
			moveMonsters();
	}

	// TODO: more

	warning("TODO");
}

void Interface::animate3d() {

}

void Interface::setIndoorsMonsters() {

}

void Interface::setIndoorObjects() {
	Common::Point mazePos = _vm->_party._mazePosition;
	_objNumber = 0;
	const int8 *posOffset = &SCREEN_POSITIONING[_vm->_party._mazeDirection * 48];
	Common::Point pt;

	Common::Array<MazeObject> &objects = _vm->_map->_mobData._objects;
	for (uint idx = 0; idx < objects.size(); ++idx) {
		MazeObject &mazeObject = objects[idx];

		// Determine which half of the X/Y lists to use
		int listOffset;
		if (_vm->_files->_isDarkCc) {
			listOffset = mazeObject._spriteId == 47 ? 1 : 0;
		}
		else {
			listOffset = mazeObject._spriteId == 113 ? 1 : 0;
		}

		// Position 1
		pt = Common::Point(mazePos.x + posOffset[2], mazePos.y + posOffset[194]);
		if (pt == mazeObject._position && _indoorList._objects0._frame == -1) {
			_indoorList._objects0._x = INDOOR_OBJECT_X[listOffset][0];
			_indoorList._objects0._y = INDOOR_OBJECT_Y[listOffset][0];
			_indoorList._objects0._frame = mazeObject._frame;
			_indoorList._objects0._sprites = mazeObject._sprites;
			_indoorList._objects0._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects0._flags |= SPRFLAG_HORIZ_FLIPPED;
			_objNumber = idx;
		}

		// Position 2
		pt = Common::Point(mazePos.x + posOffset[7], mazePos.y + posOffset[199]);
		if (pt == mazeObject._position && !_wo[27] && _indoorList._objects1._frame == -1) {
			_indoorList._objects1._x = INDOOR_OBJECT_X[listOffset][1];
			_indoorList._objects1._y = INDOOR_OBJECT_Y[listOffset][1];
			_indoorList._objects1._frame = mazeObject._frame;
			_indoorList._objects1._sprites = mazeObject._sprites;
			_indoorList._objects1._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects1._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 3
		pt = Common::Point(mazePos.x + posOffset[5], mazePos.y + posOffset[197]);
		if (pt == mazeObject._position && (!_wo[27] || !_wo[25]) && (!_wo[27] || !_wo[28]) &&
				(!_wo[23] || !_wo[25]) && (!_wo[23] || !_wo[28]) && 
				_indoorList._objects2._frame == -1) {
			_indoorList._objects2._x = INDOOR_OBJECT_X[listOffset][2];
			_indoorList._objects2._y = INDOOR_OBJECT_Y[listOffset][2];
			_indoorList._objects2._frame = mazeObject._frame;
			_indoorList._objects2._sprites = mazeObject._sprites;
			_indoorList._objects2._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects2._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 4
		pt = Common::Point(mazePos.x + posOffset[9], mazePos.y + posOffset[201]);
		if (pt == mazeObject._position && (!_wo[27] || !_wo[26]) && (!_wo[27] || !_wo[29]) &&
				(!_wo[24] || !_wo[26]) && (!_wo[24] || !_wo[29]) && _indoorList._objects3._frame == -1) {
			_indoorList._objects3._x = INDOOR_OBJECT_X[listOffset][3];
			_indoorList._objects3._y = INDOOR_OBJECT_Y[listOffset][3];
			_indoorList._objects3._frame = mazeObject._frame;
			_indoorList._objects3._sprites = mazeObject._sprites;
			_indoorList._objects3._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects3._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 5
		pt = Common::Point(mazePos.x + posOffset[14], mazePos.y + posOffset[206]);
		if (pt == mazeObject._position && !_wo[22] && !_wo[27] && _indoorList._objects4._frame == -1) {
			_indoorList._objects4._x = INDOOR_OBJECT_X[listOffset][4];
			_indoorList._objects4._y = INDOOR_OBJECT_Y[listOffset][4];
			_indoorList._objects4._frame = mazeObject._frame;
			_indoorList._objects4._sprites = mazeObject._sprites;
			_indoorList._objects4._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects4._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 6
		pt = Common::Point(mazePos.x + posOffset[12], mazePos.y + posOffset[204]);
		if (pt == mazeObject._position && !_wo[27] && (!_wo[22] || !_wo[23]) && (!_wo[22] || !_wo[20]) &&
				(!_wo[23] || !_wo[17]) && (!_wo[20] || !_wo[17]) && _indoorList._objects5._frame == -1) {
			_indoorList._objects5._x = INDOOR_OBJECT_X[listOffset][5];
			_indoorList._objects5._y = INDOOR_OBJECT_Y[listOffset][5];
			_indoorList._objects5._frame = mazeObject._frame;
			_indoorList._objects5._sprites = mazeObject._sprites;
			_indoorList._objects5._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects5._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 7
		pt = Common::Point(mazePos.x + posOffset[16], mazePos.y + posOffset[208]);
		if (pt == mazeObject._position && !_wo[27] && (!_wo[22] || !_wo[24]) && (!_wo[22] || !_wo[21]) &&
			(!_wo[24] || !_wo[19]) && (!_wo[21] || !_wo[19]) && _indoorList._objects6._frame == -1) {
			_indoorList._objects6._x = INDOOR_OBJECT_X[listOffset][6];
			_indoorList._objects6._y = INDOOR_OBJECT_Y[listOffset][6];
			_indoorList._objects6._frame = mazeObject._frame;
			_indoorList._objects6._sprites = mazeObject._sprites;
			_indoorList._objects6._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects6._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 8
		pt = Common::Point(mazePos.x + posOffset[27], mazePos.y + posOffset[219]);
		if (pt == mazeObject._position && !_wo[27] && !_wo[22] && !_wo[15] &&  _indoorList._objects7._frame == -1) {
			_indoorList._objects7._x = INDOOR_OBJECT_X[listOffset][7];
			_indoorList._objects7._y = INDOOR_OBJECT_Y[listOffset][7];
			_indoorList._objects7._frame = mazeObject._frame;
			_indoorList._objects7._sprites = mazeObject._sprites;
			_indoorList._objects7._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects7._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 9
		pt = Common::Point(mazePos.x + posOffset[25], mazePos.y + posOffset[217]);
		if (pt == mazeObject._position && !_wo[27] && (!_wo[15] || !_wo[17]) && (!_wo[15] || !_wo[12]) &&
				(!_wo[12] || !_wo[7]) && (!_wo[17] || !_wo[7]) && _indoorList._objects5._frame == -1) {
			_indoorList._objects8._x = INDOOR_OBJECT_X[listOffset][8];
			_indoorList._objects8._y = INDOOR_OBJECT_Y[listOffset][8];
			_indoorList._objects8._frame = mazeObject._frame;
			_indoorList._objects8._sprites = mazeObject._sprites;
			_indoorList._objects8._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects8._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 10
		pt = Common::Point(mazePos.x + posOffset[23], mazePos.y + posOffset[215]);
		if (pt == mazeObject._position && !_wo[27] && (!_wo[22] || !_wo[20]) && (!_wo[22] || !_wo[23]) &&
				(!_wo[20] || !_wo[17]) && (!_wo[23] || !_wo[17]) && !_wo[12] && !_wo[8] &&			
				_indoorList._objects9._frame == -1) {
			_indoorList._objects9._x = INDOOR_OBJECT_X[listOffset][9];
			_indoorList._objects9._y = INDOOR_OBJECT_Y[listOffset][9];
			_indoorList._objects9._frame = mazeObject._frame;
			_indoorList._objects9._sprites = mazeObject._sprites;
			_indoorList._objects9._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects9._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Block 11
		pt = Common::Point(mazePos.x + posOffset[29], mazePos.y + posOffset[221]);
		if (pt == mazeObject._position && !_wo[27] && !_wo[22] && (!_wo[15] || !_wo[19]) && 
				(!_wo[15] || !_wo[14]) && (!_wo[14] || !_wo[9]) && (!_wo[19] || !_wo[9]) && 
				_indoorList._objects10._frame == -1) {
			_indoorList._objects10._x = INDOOR_OBJECT_X[listOffset][10];
			_indoorList._objects10._y = INDOOR_OBJECT_Y[listOffset][10];
			_indoorList._objects10._frame = mazeObject._frame;
			_indoorList._objects10._sprites = mazeObject._sprites;
			_indoorList._objects10._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects10._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Block 12
		pt = Common::Point(mazePos.x + posOffset[31], mazePos.y + posOffset[223]);
		if (pt == mazeObject._position && !_wo[27] && (!_wo[22] || !_wo[21]) && (!_wo[22] || !_wo[24]) && 
				(!_wo[21] || !_wo[19]) && (!_wo[24] || !_wo[19]) && !_wo[14] && !_wo[10] &&
				_indoorList._objects11._frame == -1) {
			_indoorList._objects11._x = INDOOR_OBJECT_X[listOffset][11];
			_indoorList._objects11._y = INDOOR_OBJECT_Y[listOffset][11];
			_indoorList._objects11._frame = mazeObject._frame;
			_indoorList._objects11._sprites = mazeObject._sprites;
			_indoorList._objects11._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects11._flags |= SPRFLAG_HORIZ_FLIPPED;
		}
	}
}

void Interface::setOutdoorsMonsters() {

}

void Interface::setOutdoorsObjects() {

}

void Interface::startup() {
	Screen &screen = *_vm->_screen;
	loadCharIcons();
	_iconSprites.load("main.icn");

	animate3d();
	if (_vm->_map->_isOutdoors) {
		setIndoorsMonsters();
		setIndoorObjects();
	} else {
		setOutdoorsMonsters();
		setOutdoorsObjects();
	}
	draw3d(false);

	_globalSprites.draw(screen._windows[1], 5, Common::Point(232, 9));
	charIconsPrint(false);

	_mainList[0]._sprites = &_globalSprites;
	for (int i = 1; i < 16; ++i)
		_mainList[i]._sprites = &_iconSprites;

	setMainButtons();

	_tillMove = false;
}

void Interface::mainIconsPrint() {
	Screen &screen = *_vm->_screen;
	screen._windows[38].close();
	screen._windows[12].close();
	screen._windows[0].drawList(_mainList, 16);
	screen._windows[34].update();
}

void Interface::moveMonsters() {

}

void Interface::setMainButtons() {
	clearButtons();

	addButton(Common::Rect(235,  75, 259,  95),  83, &_iconSprites);
	addButton(Common::Rect(260,  75, 284,  95),  67, &_iconSprites);
	addButton(Common::Rect(286,  75, 310,  95),  82, &_iconSprites);
	addButton(Common::Rect(235,  96, 259, 116),  66, &_iconSprites);
	addButton(Common::Rect(260,  96, 284, 116),  68, &_iconSprites);
	addButton(Common::Rect(286,  96, 310, 116),  86, &_iconSprites);
	addButton(Common::Rect(235, 117, 259, 137),  77, &_iconSprites);
	addButton(Common::Rect(260, 117, 284, 137),  73, &_iconSprites);
	addButton(Common::Rect(286, 117, 310, 137),  81, &_iconSprites);
	addButton(Common::Rect(109, 137, 122, 147),   9, &_iconSprites);
	addButton(Common::Rect(235, 148, 259, 168), 240, &_iconSprites);
	addButton(Common::Rect(260, 148, 284, 168), 242, &_iconSprites);
	addButton(Common::Rect(286, 148, 310, 168), 241, &_iconSprites);
	addButton(Common::Rect(235, 169, 259, 189), 176, &_iconSprites);
	addButton(Common::Rect(260, 169, 284, 189), 243, &_iconSprites);
	addButton(Common::Rect(286, 169, 310, 189), 177, &_iconSprites);
	addButton(Common::Rect(236,  11, 308,  69),  61, &_iconSprites, false);
	addButton(Common::Rect(239,  27, 312,  37),  49, &_iconSprites, false);
	addButton(Common::Rect(239,  37, 312,  47),  50, &_iconSprites, false);
	addButton(Common::Rect(239,  47, 312,  57),  51, &_iconSprites, false);
}

} // End of namespace Xeen
