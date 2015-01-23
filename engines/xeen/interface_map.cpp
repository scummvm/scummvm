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

OutdoorDrawList::OutdoorDrawList() : _skySprite(_data[1]), _groundSprite(_data[2]),
	_combatImgs1(&_data[124]), _combatImgs2(&_data[95]),
	_combatImgs3(&_data[76]), _combatImgs4(&_data[53]) {
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
	_data[28] = DrawStruct(1, -64, 61, 14, SPRFLAG_2000);
	_data[29] = DrawStruct(1, -40, 61, 14, 0);
	_data[30] = DrawStruct(1, -16, 61, 14, 0);
	_data[31] = DrawStruct(1, 8, 61, 14, 0);
	_data[32] = DrawStruct(1, 128, 61, 14, SPRFLAG_HORIZ_FLIPPED | SPRFLAG_2000);
	_data[33] = DrawStruct(1, 104, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[34] = DrawStruct(1, 80, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[35] = DrawStruct(1, 56, 61, 14, SPRFLAG_HORIZ_FLIPPED);
	_data[36] = DrawStruct(1, 32, 61, 14, 0);
	_data[37] = DrawStruct(0, -9, 61, 14, 0);
	_data[38] = DrawStruct(0, -58, 61, 14, 0);
	_data[39] = DrawStruct(0, 40, 61, 14, 0);
	_data[40] = DrawStruct(0, -82, 61, 14, 0);
	_data[41] = DrawStruct(0, 64, 61, 14, 0);
	_data[42] = DrawStruct(0, -41, 61, 14, 0);
	_data[43] = DrawStruct(0, -26, 61, 14, 0);
	_data[44] = DrawStruct(0, -34, 61, 14, 0);
	_data[45] = DrawStruct(0, -16, 61, 14, 0);
	_data[46] = DrawStruct(0, 23, 61, 14, 0);
	_data[47] = DrawStruct(0, 16, 61, 14, 0);
	_data[48] = DrawStruct(0, -58, 61, 14, 0);
	_data[49] = DrawStruct(0, 40, 61, 14, 0);
	_data[50] = DrawStruct(0, -17, 61, 14, 0);
	_data[51] = DrawStruct(0, -1, 58, 14, 0);
	_data[52] = DrawStruct(0, -9, 58, 14, 0);
	_data[53] = DrawStruct(0, 72, 58, 12, 0);
	_data[54] = DrawStruct(0, 72, 58, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[55] = DrawStruct(0, 69, 63, 12, 0);
	_data[56] = DrawStruct(0, 75, 63, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[57] = DrawStruct(0, 73, 53, 12, 0);
	_data[58] = DrawStruct(0, 71, 53, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[59] = DrawStruct(0, 80, 57, 12, 0);
	_data[60] = DrawStruct(0, 64, 57, 12, SPRFLAG_HORIZ_FLIPPED);
	_data[61] = DrawStruct(2, -11, 54, 8, 0);
	_data[62] = DrawStruct(1, -21, 54, 11, 0);
	_data[63] = DrawStruct(2, 165, 54, 8, SPRFLAG_HORIZ_FLIPPED);
	_data[64] = DrawStruct(1, 86, 54, 11, SPRFLAG_HORIZ_FLIPPED);
	_data[65] = DrawStruct(1, 33, 54, 11, 0);
	_data[66] = DrawStruct(0, -8, 54, 12, 0);
	_data[67] = DrawStruct(0, -73, 54, 12, 0);
	_data[68] = DrawStruct(0, 57, 54, 12, 0);
	_data[69] = DrawStruct(0, -65, 54, 12, 0);
	_data[70] = DrawStruct(0, -81, 54, 12, 0);
	_data[71] = DrawStruct(0, 49, 54, 12, 0);
	_data[72] = DrawStruct(0, 65, 54, 12, 0);
	_data[73] = DrawStruct(0, -24, 54, 12, 0);
	_data[74] = DrawStruct(0, 9, 50, 12, 0);
	_data[75] = DrawStruct(0, -8, 50, 12, 0);
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
	_data[87] = DrawStruct(0, -7, 30, 7, 0);
	_data[88] = DrawStruct(0, -112, 30, 7, SPRFLAG_2000);
	_data[89] = DrawStruct(0, 98, 30, 7, SPRFLAG_2000);
	_data[90] = DrawStruct(0, -112, 30, 8, SPRFLAG_2000);
	_data[91] = DrawStruct(0, 98, 30, 8, SPRFLAG_2000);
	_data[92] = DrawStruct(0, -38, 30, 8, 0);
	_data[93] = DrawStruct(0, 25, 30, 8, 0);
	_data[94] = DrawStruct(0, -7, 30, 8, 0);
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
	_data[106] = DrawStruct(0, -23, 40, 0, SPRFLAG_2000);
	_data[107] = DrawStruct(0, 200, 40, 0, SPRFLAG_HORIZ_FLIPPED | SPRFLAG_2000);
	_data[108] = DrawStruct(0, 8, 47);
	_data[109] = DrawStruct(0, 169, 47, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[110] = DrawStruct(1, -56, -4, 0x8000, SPRFLAG_4000 | SPRFLAG_2000);
	_data[111] = DrawStruct(0, -5, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[112] = DrawStruct(0, -67, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[113] = DrawStruct(0, 44, 73);
	_data[114] = DrawStruct(0, 44, 73);
	_data[115] = DrawStruct(0, 58, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[116] = DrawStruct(0, 169, 73);
	_data[117] = DrawStruct(0, 169, 73);
	_data[118] = DrawStruct(0, -5, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[119] = DrawStruct(0, 110, 73);
	_data[120] = DrawStruct(0, 110, 73);
	_data[121] = DrawStruct(0, -5, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
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

IndoorDrawList::IndoorDrawList() : 
	_sky1(_data[0]), _sky2(_data[1]), _ground(_data[2]), _horizon(_data[28]),
	_swl_0F1R(_data[146]), _swl_0F1L(_data[144]), _swl_1F1R(_data[134]), 
	_swl_1F1L(_data[133]), _swl_2F2R(_data[110]), _swl_2F1R(_data[109]), 
	_swl_2F1L(_data[108]), _swl_2F2L(_data[107]), _swl_3F1R(_data[ 78]), 
	_swl_3F2R(_data[ 77]), _swl_3F3R(_data[ 76]), _swl_3F4R(_data[ 75]),
	_swl_3F1L(_data[ 74]), _swl_3F2L(_data[ 73]), _swl_3F3L(_data[ 72]), 
	_swl_3F4L(_data[ 71]), _swl_4F4R(_data[ 33]), _swl_4F3R(_data[ 34]), 
	_swl_4F2R(_data[ 35]), _swl_4F1R(_data[ 36]), _swl_4F1L(_data[ 32]),
	_swl_4F2L(_data[ 31]), _swl_4F3L(_data[ 30]), _swl_4F4L(_data[ 29]),
	_fwl_4F4R(_data[ 45]), _fwl_4F3R(_data[ 44]), _fwl_4F2R(_data[ 43]), 
	_fwl_4F1R(_data[ 42]), _fwl_4F(  _data[ 41]), _fwl_4F1L(_data[ 40]), 
	_fwl_4F2L(_data[ 39]), _fwl_4F3L(_data[ 38]), _fwl_4F4L(_data[ 37]),
	_fwl_2F1R(_data[121]), _fwl_2F(  _data[120]), _fwl_2F1L(_data[119]), 
	_fwl_3F2R(_data[ 91]), _fwl_3F1R(_data[ 90]), _fwl_3F(  _data[ 89]), 
	_fwl_3F1L(_data[ 88]), _fwl_3F2L(_data[ 87]), _fwl_1F(  _data[147]),
	_fwl_1F1R(_data[145]), _fwl_1F1L(_data[143]),
	_groundTiles(&_data[3]),
	_objects0(_data[149]), _objects1(_data[125]), _objects2(_data[126]),
	_objects3(_data[127]), _objects4(_data[97]), _objects5(_data[98]),
	_objects6(_data[99]), _objects7(_data[55]), _objects8(_data[56]),
	_objects9(_data[58]), _objects10(_data[57]), _objects11(_data[59]),
	_combatImgs1(&_data[162]), _combatImgs2(&_data[135]),
	_combatImgs3(&_data[111]), _combatImgs4(&_data[80]) {
	// Setup draw structure positions
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
	_data[46] = DrawStruct(0, -64, 61, 14, 0);
	_data[47] = DrawStruct(0, -40, 61, 14, 0);
	_data[48] = DrawStruct(0, -16, 61, 14, 0);
	_data[49] = DrawStruct(0, 8, 61, 14, 0);
	_data[50] = DrawStruct(0, 32, 61, 14, 0);
	_data[51] = DrawStruct(0, 56, 61, 14, 0);
	_data[52] = DrawStruct(0, 80, 61, 14, 0);
	_data[53] = DrawStruct(0, 104, 61, 14, 0);
	_data[54] = DrawStruct(0, 128, 61, 14, 0);
	_data[55] = DrawStruct(0, -9, 58, 14, 0);
	_data[56] = DrawStruct(0, -34, 58, 14, 0);
	_data[57] = DrawStruct(0, 16, 58, 14, 0);
	_data[58] = DrawStruct(0, -58, 58, 14, 0);
	_data[59] = DrawStruct(0, 40, 58, 14, 0);
	_data[60] = DrawStruct(0, -41, 58, 14, 0);
	_data[61] = DrawStruct(0, -26, 58, 14, 0);
	_data[62] = DrawStruct(0, -34, 58, 14, 0);
	_data[63] = DrawStruct(0, -16, 58, 14, 0);
	_data[64] = DrawStruct(0, 23, 58, 14, 0);
	_data[65] = DrawStruct(0, 16, 58, 14, 0);
	_data[66] = DrawStruct(0, -58, 58, 14, 0);
	_data[67] = DrawStruct(0, 40, 58, 14, 0);
	_data[68] = DrawStruct(0, -17, 58, 14, 0);
	_data[69] = DrawStruct(0, -1, 58, 14, 0);
	_data[70] = DrawStruct(0, -9, 58, 14, 0);
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
	_data[87] = DrawStruct(7, -24, 52, 0, SPRFLAG_2000);
	_data[88] = DrawStruct(7, 32, 52);
	_data[89] = DrawStruct(7, 88, 52);
	_data[90] = DrawStruct(0, 144, 52);
	_data[91] = DrawStruct(0, 200, 52, 0, SPRFLAG_2000);
	_data[92] = DrawStruct(0, -79, 52, 11, SPRFLAG_2000);
	_data[93] = DrawStruct(0, -27, 52, 11, 0);
	_data[94] = DrawStruct(0, 32, 52, 11, 0);
	_data[95] = DrawStruct(0, 89, 52, 11, 0);
	_data[96] = DrawStruct(0, 145, 52, 11, SPRFLAG_2000);
	_data[97] = DrawStruct(0, -8, 50, 12, 0);
	_data[98] = DrawStruct(0, -65, 50, 12, 0);
	_data[99] = DrawStruct(0, 49, 50, 12, 0);
	_data[100] = DrawStruct(0, -65, 50, 12, 0);
	_data[101] = DrawStruct(0, -81, 50, 12, 0);
	_data[102] = DrawStruct(0, 49, 50, 12, 0);
	_data[103] = DrawStruct(0, 65, 50, 12, 0);
	_data[104] = DrawStruct(0, -24, 50, 12, 0);
	_data[105] = DrawStruct(0, 9, 50, 12, 0);
	_data[106] = DrawStruct(0, -8, 50, 12, 0);
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
	_data[119] = DrawStruct(6, -40, 40, 0, SPRFLAG_2000);
	_data[120] = DrawStruct(6, 64, 40);
	_data[121] = DrawStruct(0, 168, 40, 0, SPRFLAG_2000);
	_data[122] = DrawStruct(0, -72, 40, 6, SPRFLAG_2000);
	_data[123] = DrawStruct(0, 32, 40, 6, 0);
	_data[124] = DrawStruct(0, 137, 40, 6, SPRFLAG_2000);
	_data[125] = DrawStruct(0, -7, 25, 7, 0);
	_data[126] = DrawStruct(0, -112, 25, 7, SPRFLAG_2000);
	_data[127] = DrawStruct(0, 98, 25, 7, SPRFLAG_2000);
	_data[128] = DrawStruct(0, -112, 29, 8, SPRFLAG_2000);
	_data[129] = DrawStruct(0, 98, 29, 8, SPRFLAG_2000);
	_data[130] = DrawStruct(0, -38, 29, 8, 0);
	_data[131] = DrawStruct(0, 25, 29, 8, 0);
	_data[132] = DrawStruct(0, -7, 29, 8, 0);
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
	_data[143] = DrawStruct(0, -136, 24, 0, SPRFLAG_2000);
	_data[144] = DrawStruct(0, 8, 12);
	_data[145] = DrawStruct(0, 32, 24);
	_data[146] = DrawStruct(0, 200, 12, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[147] = DrawStruct(0, 200, 24, 0, SPRFLAG_2000);
	_data[148] = DrawStruct(0, 32, 24);
	_data[149] = DrawStruct(0, -5, 2, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[150] = DrawStruct(0, -67, 10, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[151] = DrawStruct(0, 44, 73);
	_data[152] = DrawStruct(0, 44, 73);
	_data[153] = DrawStruct(0, 58, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[154] = DrawStruct(0, 169, 73);
	_data[155] = DrawStruct(0, 169, 73);
	_data[156] = DrawStruct(0, -5, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
	_data[157] = DrawStruct(0, 110, 73);
	_data[158] = DrawStruct(0, 110, 73);
	_data[159] = DrawStruct(0, -5, 14, 0, SPRFLAG_4000 | SPRFLAG_2000);
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

InterfaceMap::InterfaceMap(XeenEngine *vm): _vm(vm) {
	Common::fill(&_wp[0], &_wp[20], 0);
	Common::fill(&_wo[0], &_wo[308], 0);
	_overallFrame = 0;
	_flipWater = false;
	_flipGround = false;
	_flipSky = false;
	_flipDefaultGround = false;
	_isShooting = false;
	_charsShooting = false;
	_objNumber = 0;
	_combatFloatCounter = 0;
	_thinWall = false;
	_isAnimReset = false;
	_upDoorText = false;
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
	_tillMove = 0;
	_flag1 = false;
	_overallFrame = 0;
	_face1State = _face2State = 0;
}

void InterfaceMap::setup() {
	_globalSprites.load("global.icn");
	_borderSprites.load("border.icn");
	_spellFxSprites.load("spellfx.icn");
	_fecpSprites.load("fecp.brd");
	_blessSprites.load("bless.icn");
	_charPowSprites.load("charpow.icn");
}

void InterfaceMap::draw3d(bool updateFlag) {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Scripts &scripts = *_vm->_scripts;

	if (screen._windows[11]._enabled)
		return;

	_flipUIFrame = (_flipUIFrame + 1) % 4;
	if (_flipUIFrame == 0)
		_flipWater = !_flipWater;
	if (_tillMove && (_vm->_mode == MODE_1 || _vm->_mode == MODE_2) &&
		!_flag1 && _vm->_moveMonsters) {
		if (--_tillMove == 0)
			moveMonsters();
	}

	MazeObject &objObject = map._mobData._objects[_objNumber];
	Direction partyDirection = _vm->_party->_mazeDirection;
	int objNum = _objNumber - 1;

	// Loop to update the frame numbers for each maze object, applying the animation frame
	// limits as specified by the map's _animationInfo listing
	for (uint idx = 0; idx < map._mobData._objects.size(); ++idx) {
		MazeObject &mazeObject = map._mobData._objects[idx];
		AnimationEntry &animEntry = map._animationInfo[mazeObject._spriteId];
		int directionIndex = DIRECTION_ANIM_POSITIONS[mazeObject._direction][partyDirection];

		if (_isAnimReset) {
			mazeObject._frame = animEntry._frame1._frames[directionIndex];
		}
		else {
			++mazeObject._frame;
			if ((int)idx == objNum && scripts._animCounter > 0 && (
				objObject._spriteId == (_vm->_files->_isDarkCc ? 15 : 16) ||
				objObject._spriteId == 58 || objObject._spriteId == 73)) {
				if (mazeObject._frame > 4 || mazeObject._spriteId == 58)
					mazeObject._frame = 1;
			}
			else if (mazeObject._frame >= animEntry._frame2._frames[directionIndex]) {
				mazeObject._frame = animEntry._frame1._frames[directionIndex];
			}
		}

		mazeObject._flipped = animEntry._flipped._flags[directionIndex];
	}

	if (map._isOutdoors) {
		error("TODO: draw3d outdoors handling");
	}
	else {
		// Default all the parts of draw struct not to be drawn by default
		for (int idx = 3; idx < _indoorList.size(); ++idx)
			_indoorList[idx]._frame = -1;

		if (_flag1) {
			for (int idx = 0; idx < 96; ++idx) {
				if (_indoorList[79 + idx]._sprites != nullptr) {
					_indoorList[79 + idx]._frame = 0;
				}
				else if (_indoorList[111 + idx]._sprites != nullptr) {
					_indoorList[111 + idx]._frame = 1;
				}
				else if (_indoorList[135 + idx]._sprites != nullptr) {
					_indoorList[135 + idx]._frame = 2;
				}
				else if (_indoorList[162 + idx]._sprites != nullptr) {
					_indoorList[162 + idx]._frame = 0;
				}
			}
		}
		else if (_charsShooting) {
			for (int idx = 0; idx < 96; ++idx) {
				if (_indoorList[162 + idx]._sprites != nullptr) {
					_indoorList[162 + idx]._frame = 0;
				}
				else if (_indoorList[135 + idx]._sprites != nullptr) {
					_indoorList[135 + idx]._frame = 1;
				}
				else if (_indoorList[111 + idx]._sprites != nullptr) {
					_indoorList[111 + idx]._frame = 2;
				}
				else if (_indoorList[79 + idx]._sprites != nullptr) {
					_indoorList[79 + idx]._frame = 0;
				}
			}
		}

		setMazeBits();
		_isAnimReset = false;
		const int INDOOR_INDEXES[3] = { 157, 151, 154 };
		const int INDOOR_COMBAT_POS[3][2] = { { 102, 134 }, { 36, 67 }, { 161, 161 } };
		const int INDOOR_COMBAT_POS2[4] = { 8, 6, 4, 2 };

		// Double check this, since it's not being used?
		//MazeObject &objObject = map._mobData._objects[_objNumber - 1];

		for (int idx = 0; idx < 3; ++idx) {
			DrawStruct &ds1 = _indoorList[INDOOR_INDEXES[idx]];
			DrawStruct &ds2 = _indoorList[INDOOR_INDEXES[idx] + 1];
			ds1._sprites = nullptr;
			ds2._sprites = nullptr;

			if (combat._charsArray1[idx]) {
				int posIndex = combat._attackMonsters[1] && !combat._attackMonsters[2] ? 1 : 0;
				--combat._charsArray1[idx];

				if (combat._monPow[idx]) {
					ds1._x = INDOOR_COMBAT_POS[idx][0];
					ds1._frame = 0;
					ds1._scale = combat._monsterScale[idx];
					if (ds1._scale == 0x8000) {
						ds1._x /= 3;
						ds1._y = 60;
					}
					else {
						ds1._y = 73;
					}

					ds1._flags = SPRFLAG_4000 | SPRFLAG_2000;
					ds1._sprites = &_charPowSprites;
				}

				if (combat._elemPow[idx]) {
					ds2._x = INDOOR_COMBAT_POS[idx][posIndex] + INDOOR_COMBAT_POS2[idx];
					ds2._frame = combat._elemPow[idx];
					ds2._scale = combat._elemScale[idx];
					if (ds2._scale == 0x8000)
						ds2._x /= 3;
					ds2._flags = SPRFLAG_4000 | SPRFLAG_2000;
					ds2._sprites = &_charPowSprites;
				}
			}
		}

		setIndoorsMonsters();
		setIndoorsObjects();
		setIndoorsWallPics();

		_indoorList[161]._sprites = nullptr;
		_indoorList[160]._sprites = nullptr;
		_indoorList[159]._sprites = nullptr;

		// Handle attacking monsters
		int monsterIndex = 0;
		if (combat._attackMonsters[0] != -1 && map._mobData._monsters[combat._attackMonsters[0]]._frame >= 0) {
			_indoorList[159] = _indoorList[156];
			_indoorList[160] = _indoorList[157];
			_indoorList[161] = _indoorList[158];
			_indoorList[158]._sprites = nullptr;
			_indoorList[156]._sprites = nullptr;
			_indoorList[157]._sprites = nullptr;
			monsterIndex = 1;
		}
		else if (combat._attackMonsters[1] != -1 && map._mobData._monsters[combat._attackMonsters[1]]._frame >= 0) {
			_indoorList[159] = _indoorList[150];
			_indoorList[160] = _indoorList[151];
			_indoorList[161] = _indoorList[152];
			_indoorList[152]._sprites = nullptr;
			_indoorList[151]._sprites = nullptr;
			_indoorList[150]._sprites = nullptr;
			monsterIndex = 2;
		}
		else if (combat._attackMonsters[2] != -1 && map._mobData._monsters[combat._attackMonsters[2]]._frame >= 0) {
			_indoorList[159] = _indoorList[153];
			_indoorList[160] = _indoorList[154];
			_indoorList[161] = _indoorList[155];
			_indoorList[153]._sprites = nullptr;
			_indoorList[154]._sprites = nullptr;
			_indoorList[155]._sprites = nullptr;
			monsterIndex = 3;
		}

		drawIndoors();

		switch (monsterIndex) {
		case 1:
			_indoorList[156] = _indoorList[159];
			_indoorList[157] = _indoorList[160];
			_indoorList[158] = _indoorList[161];
			break;
		case 2:
			_indoorList[150] = _indoorList[159];
			_indoorList[151] = _indoorList[160];
			_indoorList[152] = _indoorList[161];
			break;
		case 3:
			_indoorList[153] = _indoorList[159];
			_indoorList[154] = _indoorList[160];
			_indoorList[155] = _indoorList[161];
			break;
		default:
			break;
		}
	}

	animate3d();
	drawMiniMap();

	if (party._falling == 1) {
		error("TODO: Indoor falling");
	}

	if (party._falling == 2) {
		screen.saveBackground(1);
	}

	assembleBorder();

	// Draw any on-screen text if flagged to do so
	if (_upDoorText && combat._attackMonsters[0] == -1) {
		screen._windows[3].writeString(_screenText);
	}

	if (updateFlag) {
		screen._windows[1].update();
		screen._windows[3].update();
	}

	// TODO: more stuff

	_vm->_party->_stepped = false;
	if (_vm->_mode == MODE_9) {
		// TODO
	}
	events.wait(2);
}

/**
 * Handles animation of monsters, wall items, and combat within the 3d
 * view by cycling the appropriate frame numbers
 */
void InterfaceMap::animate3d() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	_overallFrame = (_overallFrame + 1) % 5;
	_combatFloatCounter = (_combatFloatCounter + 1) % 8;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];
		if (!monster._field7) {
			if (monster._frame < 8) {
				MonsterStruct &monsterData = map._monsterData[monster._spriteId];
				if (!monsterData._loopAnimation) {
					// Monster isn't specially looped, so cycle through the 8 frames
					monster._frame = (monster._frame + 1) % 8;
				} else if (!monster._field9) {
					monster._frame = (monster._frame + 1) % 8;
					if (monster._frame == 0) {
						monster._field9 ^= 1;
						monster._frame = 6;
					}
				} else {
					if (monster._frame)
						--monster._frame;
					if (monster._frame == 0)
						monster._field9 = 0;
				}
			} else if (monster._frame == 11) {
				--monster._fieldA;
				if (monster._fieldA == 0)
					monster._frame = 0;
			} else {
				++monster._frame;
				if (monster._frame == 11) {
					--monster._frame;
					monster._frame = monster._fieldA ? 10 : 0;
				}
			}
		}

		// Block 2
		if (monster._effect2) {
			if (monster._effect1) {
				if (monster._effect1 & 0x80) {
					if (monster._effect3)
						--monster._effect3;
					if (monster._effect3 == 0)
						monster._effect1 ^= 0x80;
				} else {
					monster._effect3 = (monster._effect3 + 1) % 3;
					if (monster._effect3 == 0) {
						monster._effect1 ^= 0x80;
						monster._effect3 = 2;
					}
				}
			}
		} else {
			monster._effect3 = (monster._effect3 + 1) % 8;
			if (monster._effect3 == 0) {
				MonsterStruct &monsterData = map._monsterData[monster._spriteId];
				monster._effect1 = monster._effect2 = monsterData._animationEffect;
			}
		}
	}

	DrawStruct *combatImgs1 = map._isOutdoors ? _outdoorList._combatImgs1 : _indoorList._combatImgs1;
	DrawStruct *combatImgs2 = map._isOutdoors ? _outdoorList._combatImgs2 : _indoorList._combatImgs2;
	DrawStruct *combatImgs3 = map._isOutdoors ? _outdoorList._combatImgs3 : _indoorList._combatImgs3;
	DrawStruct *combatImgs4 = map._isOutdoors ? _outdoorList._combatImgs4 : _indoorList._combatImgs4;

	if (_flag1) {
		for (int idx = 0; idx < 8; ++idx) {
			if (combatImgs1[idx]._sprites) {
				combatImgs1[idx]._sprites = nullptr;
				combat._shooting[idx] = false;
			} else if (combatImgs2[idx]._sprites) {
				combatImgs1[idx]._sprites = combatImgs2[idx]._sprites;
				combatImgs2[idx]._sprites = nullptr;
			} else if (combatImgs3[idx]._sprites) {
				combatImgs2[idx]._sprites = combatImgs3[idx]._sprites;
				combatImgs3[idx]._sprites = nullptr;
			} else if (combatImgs4[idx]._sprites) {
				combatImgs3[idx]._sprites = combatImgs4[idx]._sprites;
				combatImgs4[idx]._sprites = nullptr;
			}
		}
	} else if (_charsShooting) {
		for (int idx = 0; idx < 8; ++idx) {
			if (combatImgs4[idx]._sprites) {
				combatImgs4[idx]._sprites = nullptr;
			} else if (combatImgs3[idx]._sprites) {
				combatImgs4[idx]._sprites = combatImgs3[idx]._sprites;
				combatImgs3[idx]._sprites = nullptr;
			} else if (combatImgs2[idx]._sprites) {
				combatImgs3[idx]._sprites = combatImgs2[idx]._sprites;
				combatImgs2[idx]._sprites = nullptr;
			} else if (combatImgs1[idx]._sprites) {
				combatImgs2[idx]._sprites = combatImgs1[idx]._sprites;
				combatImgs1[idx]._sprites = nullptr;
			}
		}
	}

	for (uint idx = 0; idx < map._mobData._wallItems.size(); ++idx) {
		MazeWallItem &wallItem = map._mobData._wallItems[idx];
		wallItem._frame = (wallItem._frame + 1) % wallItem._sprites->size();
	}
}

void InterfaceMap::setMazeBits() {
	Common::fill(&_wo[0], &_wo[308], 0);

	switch (_vm->_map->getCell(0) - 1) {
	case 0:
		++_wo[125];
		break;
	case 1:
		++_wo[69];
		break;
	case 2:
		++_wo[185];
		break;
	case 3:
	case 12:
		++_wo[105];
		break;
	case 4:
	case 7:
		++_wo[25];
		break;
	case 5:
		++_wo[225];
		break;
	case 6:
		++_wo[205];
		break;
	case 8:
		++_wo[145];
		break;
	case 9:
		++_wo[305];
		break;
	case 10:
		++_wo[245];
		break;
	case 11:
		++_wo[165];
		break;
	case 13:
		++_wo[265];
		break;
	case 14:
		++_wo[285];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(1) - 1) {
	case 1:
		++_wo[72];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[28];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(2) - 1) {
	case 0:
		++_wo[127];
		break;
	case 1:
		++_wo[71];
		break;
	case 2:
		++_wo[187];
		break;
	case 3:
	case 12:
		++_wo[107];
		break;
	case 4:
	case 7:
		++_wo[27];
		break;
	case 5:
		++_wo[227];
		break;
	case 6:
		++_wo[207];
		break;
	case 8:
		++_wo[147];
		break;
	case 9:
		++_wo[307];
		break;
	case 10:
		++_wo[247];
		break;
	case 11:
		++_wo[167];
		break;
	case 13:
		++_wo[267];
		break;
	case 14:
		++_wo[287];
		break;
	default:
		break;
	}

	_vm->_party->handleLight();

	switch (_vm->_map->getCell(3) - 1) {
	case 1:
		++_wo[73];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[29];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(4) - 1) {
	case 0:
		++_wo[126];
		break;
	case 1:
		++_wo[70];
		break;
	case 2:
		++_wo[186];
		break;
	case 3:
	case 12:
		++_wo[106];
		break;
	case 4:
	case 7:
		++_wo[26];
		break;
	case 5:
		++_wo[226];
		break;
	case 6:
		++_wo[206];
	case 8:
		++_wo[146];
		break;
	case 9:
		++_wo[306];
		break;
	case 10:
		++_wo[246];
		break;
		break;
	case 11:
		++_wo[166];
		break;
	case 13:
		++_wo[266];
		break;
	case 14:
		++_wo[286];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(5) - 1) {
	case 0:
		++_wo[122];
		break;
	case 1:
		++_wo[64];
		break;
	case 2:
		++_wo[182];
		break;
	case 3:
	case 12:
		++_wo[102];
		break;
	case 4:
	case 7:
		++_wo[20];
		break;
	case 5:
		++_wo[222];
		break;
	case 6:
		++_wo[202];
		break;
	case 8:
		++_wo[142];
		break;
	case 9:
		++_wo[302];
		break;
	case 10:
		++_wo[242];
		break;
	case 11:
		++_wo[162];
		break;
	case 13:
		++_wo[262];
		break;
	case 14:
		++_wo[282];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(6) - 1) {
	case 1:
		++_wo[67];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[23];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(7) - 1) {
	case 0:
		++_wo[124];
		break;
	case 1:
		++_wo[66];
		break;
	case 2:
		++_wo[184];
		break;
	case 3:
	case 12:
		++_wo[104];
		break;
	case 4:
	case 7:
		++_wo[22];
		break;
	case 5:
		++_wo[224];
		break;
	case 6:
		++_wo[204];
		break;
	case 8:
		++_wo[144];
		break;
	case 9:
		++_wo[304];
		break;
	case 10:
		++_wo[244];
		break;
	case 11:
		++_wo[164];
		break;
	case 13:
		++_wo[264];
		break;
	case 14:
		++_wo[284];
		break;
	default:
		break;
	}

	_thinWall = (_vm->_map->_currentWall._data != INVALID_CELL) && _wo[27];

	switch (_vm->_map->getCell(8) - 1) {
	case 1:
		++_wo[68];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[24];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(9) - 1) {
	case 0:
		++_wo[123];
		break;
	case 1:
		++_wo[65];
		break;
	case 2:
		++_wo[183];
		break;
	case 3:
	case 12:
		++_wo[103];
		break;
	case 4:
	case 7:
		++_wo[21];
		break;
	case 5:
		++_wo[223];
		break;
	case 6:
		++_wo[203];
		break;
	case 8:
		++_wo[143];
		break;
	case 9:
		++_wo[3033];
		break;
	case 10:
		++_wo[243];
		break;
	case 11:
		++_wo[163];
		break;
	case 13:
		++_wo[263];
		break;
	case 14:
		++_wo[283];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(10) - 1) {
	case 0:
		++_wo[117];
		break;
	case 1:
		++_wo[55];
		break;
	case 2:
		++_wo[177];
		break;
	case 3:
	case 12:
		++_wo[97];
		break;
	case 4:
	case 7:
		++_wo[11];
		break;
	case 5:
		++_wo[217];
		break;
	case 6:
		++_wo[197];
		break;
	case 8:
		++_wo[137];
		break;
	case 9:
		++_wo[297];
		break;
	case 10:
		++_wo[237];
	case 11:
		++_wo[157];
		break;
	case 13:
		++_wo[257];
		break;
	case 14:
		++_wo[277];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(11) - 1) {
	case 1:
		++_wo[60];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[16];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(12) - 1) {
	case 0:
		++_wo[118];
		break;
	case 1:
		++_wo[56];
		break;
	case 2:
		++_wo[178];
		break;
	case 3:
	case 12:
		++_wo[98];
		break;
	case 4:
	case 7:
		++_wo[12];
		break;
	case 5:
		++_wo[218];
		break;
	case 6:
		++_wo[198];
		break;
	case 8:
		++_wo[138];
		break;
	case 9:
		++_wo[298];
		break;
	case 10:
		++_wo[238];
		break;
	case 11:
		++_wo[158];
		break;
	case 13:
		++_wo[258];
		break;
	case 14:
		++_wo[278];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(13) - 1) {
	case 1:
		++_wo[61];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[17];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(14) - 1) {
	case 0:
		++_wo[121];
		break;
	case 1:
		++_wo[59];
		break;
	case 2:
		++_wo[181];
		break;
	case 3:
	case 12:
		++_wo[101];
		break;
	case 4:
	case 7:
		++_wo[15];
		break;
	case 5:
		++_wo[221];
		break;
	case 6:
		++_wo[201];
		break;
	case 8:
		++_wo[141];
		break;
	case 9:
		++_wo[301];
		break;
	case 10:
		++_wo[241];
		break;
	case 11:
		++_wo[161];
		break;
	case 13:
		++_wo[261];
		break;
	case 14:
		++_wo[281];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(15) - 1) {
	case 1:
		++_wo[63];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[19];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(16) - 1) {
	case 0:
		++_wo[120];
		break;
	case 1:
		++_wo[58];
		break;
	case 2:
		++_wo[180];
		break;
	case 3:
	case 12:
		++_wo[100];
		break;
	case 4:
	case 7:
		++_wo[14];
		break;
	case 5:
		++_wo[220];
		break;
	case 6:
		++_wo[200];
		break;
	case 8:
		++_wo[140];
		break;
	case 9:
		++_wo[300];
		break;
	case 10:
		++_wo[240];
		break;
	case 11:
		++_wo[160];
		break;
	case 13:
		++_wo[260];
		break;
	case 14:
		++_wo[280];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(17) - 1) {
	case 1:
		++_wo[62];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[18];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(18) - 1) {
	case 0:
		++_wo[119];
		break;
	case 1:
		++_wo[57];
		break;
	case 2:
		++_wo[179];
		break;
	case 3:
	case 12:
		++_wo[99];
		break;
	case 4:
	case 7:
		++_wo[13];
		break;
	case 5:
		++_wo[219];
		break;
	case 6:
		++_wo[199];
		break;
	case 8:
		++_wo[139];
		break;
	case 9:
		++_wo[299];
		break;
	case 10:
		++_wo[239];
		break;
	case 11:
		++_wo[159];
		break;
	case 13:
		++_wo[259];
		break;
	case 14:
		++_wo[279];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(19) - 1) {
	case 0:
		++_wo[108];
		break;
	case 1:
		++_wo[78];
		break;
	case 2:
		++_wo[168];
	case 3:
	case 12:
		++_wo[88];
		break;
	case 4:
	case 7:
		++_wo[34];
		break;
	case 5:
		++_wo[208];
		break;
	case 6:
		++_wo[188];
		break;
	case 8:
		++_wo[128];
		break;
	case 9:
		++_wo[288];
		break;
	case 10:
		++_wo[228];
		break;
	case 11:
		++_wo[148];
		break;
	case 13:
		++_wo[248];
		break;
	case 14:
		++_wo[268];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(20) - 1) {
	case 1:
		++_wo[76];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[32];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(21) - 1) {
	case 0:
		++_wo[109];
		break;
	case 1:
		++_wo[44];
		break;
	case 2:
		++_wo[169];
		break;
	case 3:
	case 12:
		++_wo[89];
		break;
	case 4:
	case 7:
		++_wo[0];
		break;
	case 5:
		++_wo[209];
		break;
	case 6:
		++_wo[189];
		break;
	case 8:
		++_wo[129];
		break;
	case 9:
		++_wo[289];
		break;
	case 10:
		++_wo[229];
		break;
	case 11:
		++_wo[149];
		break;
	case 13:
		++_wo[249];
		break;
	case 14:
		++_wo[269];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(22) - 1) {
	case 1:
		++_wo[74];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[30];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(23) - 1) {
	case 0:
		++_wo[110];
		break;
	case 1:
		++_wo[45];
		break;
	case 2:
		++_wo[170];
		break;
	case 3:
	case 12:
		++_wo[90];
		break;
	case 4:
	case 7:
		++_wo[1];
		break;
	case 5:
		++_wo[210];
		break;
	case 6:
		++_wo[190];
		break;
	case 8:
		++_wo[130];
		break;
	case 9:
		++_wo[290];
		break;
	case 10:
		++_wo[230];
		break;
	case 11:
		++_wo[150];
		break;
	case 13:
		++_wo[250];
		break;
	case 14:
		++_wo[270];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(24) - 1) {
	case 1:
		++_wo[52];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[8];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(25) - 1) {
	case 0:
		++_wo[111];
		break;
	case 1:
		++_wo[46];
		break;
	case 2:
		++_wo[171];
		break;
	case 3:
	case 12:
		++_wo[91];
		break;
	case 4:
	case 7:
		++_wo[2];
		break;
	case 5:
		++_wo[211];
		break;
	case 6:
		++_wo[191];
		break;
	case 8:
		++_wo[131];
		break;
	case 9:
		++_wo[291];
		break;
	case 10:
		++_wo[231];
		break;
	case 11:
		++_wo[151];
		break;
	case 13:
		++_wo[251];
		break;
	case 14:
		++_wo[271];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(26) - 1) {
	case 1:
		++_wo[51];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[7];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(27) - 1) {
	case 0:
		++_wo[116];
		break;
	case 1:
		++_wo[50];
		break;
	case 2:
		++_wo[176];
		break;
	case 3:
	case 12:
		++_wo[96];
		break;
	case 4:
	case 7:
		++_wo[6];
		break;
	case 5:
		++_wo[216];
		break;
	case 6:
		++_wo[196];
		break;
	case 8:
		++_wo[136];
		break;
	case 9:
		++_wo[296];
		break;
	case 10:
		++_wo[236];
		break;
	case 11:
		++_wo[156];
		break;
	case 13:
		++_wo[256];
		break;
	case 14:
		++_wo[276];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(28) - 1) {
	case 1:
		++_wo[53];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[9];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(29) - 1) {
	case 0:
		++_wo[115];
		break;
	case 1:
		++_wo[49];
		break;
	case 2:
		++_wo[175];
		break;
	case 3:
	case 12:
		++_wo[95];
		break;
	case 4:
	case 7:
		++_wo[5];
		break;
	case 5:
		++_wo[215];
		break;
	case 6:
		++_wo[195];
		break;
	case 8:
		++_wo[135];
		break;
	case 9:
		++_wo[295];
		break;
	case 10:
		++_wo[235];
		break;
	case 11:
		++_wo[155];
		break;
	case 13:
		++_wo[255];
		break;
	case 14:
		++_wo[275];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(30) - 1) {
	case 1:
		++_wo[54];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[10];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(31) - 1) {
	case 0:
		++_wo[114];
		break;
	case 1:
		++_wo[48];
		break;
	case 2:
		++_wo[174];
		break;
	case 3:
	case 12:
		++_wo[94];
		break;
	case 4:
	case 7:
		++_wo[4];
		break;
	case 5:
		++_wo[214];
		break;
	case 6:
		++_wo[194];
		break;
	case 8:
		++_wo[134];
		break;
	case 9:
		++_wo[294];
		break;
	case 10:
		++_wo[234];
		break;
	case 11:
		++_wo[154];
		break;
	case 13:
		++_wo[254];
		break;
	case 14:
		++_wo[274];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(32) - 1) {
	case 1:
		++_wo[75];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[31];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(33) - 1) {
	case 0:
		++_wo[112];
		break;
	case 1:
		++_wo[47];
		break;
	case 2:
		++_wo[172];
		break;
	case 3:
	case 12:
		++_wo[92];
		break;
	case 4:
	case 7:
		++_wo[3];
		break;
	case 5:
		++_wo[212];
		break;
	case 6:
		++_wo[192];
		break;
	case 8:
		++_wo[132];
		break;
	case 9:
		++_wo[292];
		break;
	case 10:
		++_wo[232];
		break;
	case 11:
		++_wo[152];
		break;
	case 13:
		++_wo[252];
		break;
	case 14:
		++_wo[272];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(34) - 1) {
	case 1:
		++_wo[77];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[33];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(35) - 1) {
	case 0:
		++_wo[113];
		break;
	case 1:
		++_wo[79];
		break;
	case 2:
		++_wo[173];
		break;
	case 3:
	case 12:
		++_wo[93];
		break;
	case 4:
	case 7:
		++_wo[35];
		break;
	case 5:
		++_wo[213];
		break;
	case 6:
		++_wo[193];
		break;
	case 8:
		++_wo[133];
		break;
	case 9:
		++_wo[293];
		break;
	case 10:
		++_wo[233];
		break;
	case 11:
		++_wo[153];
		break;
	case 13:
		++_wo[253];
		break;
	case 14:
		++_wo[273];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(36) - 1) {
	case 1:
		++_wo[83];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[39];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(37) - 1) {
	case 1:
		++_wo[82];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[38];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(38) - 1) {
	case 1:
		++_wo[81];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[37];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(39) - 1) {
	case 1:
		++_wo[80];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[36];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(40) - 1) {
	case 1:
		++_wo[84];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[40];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(41) - 1) {
	case 1:
		++_wo[85];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[41];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(42) - 1) {
	case 1:
		++_wo[86];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[42];
		break;
	default:
		break;
	}

	switch (_vm->_map->getCell(43) - 1) {
	case 1:
		++_wo[87];
		break;
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
	case 13:
	case 14:
		++_wo[43];
		break;
	default:
		break;
	}
}

void InterfaceMap::setIndoorsMonsters() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	Common::Point mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;
	const int INDOOR_MONSTERS_Y[4] = { 2, 34, 53, 59 };

	combat.clear();
	for (uint monsterIdx = 0; monsterIdx < map._mobData._monsters.size(); ++monsterIdx) {
		MazeMonster &monster = map._mobData._monsters[monsterIdx];
		SpriteResource *sprites = monster._sprites;
		int frame = monster._frame;

		if (frame >= 8) {
			sprites = monster._attackSprites;
			frame -= 8;
		}

		// The following long sequence sets up monsters in the various positions
		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][2]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][2])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[0] == -1) {
				combat._attackMonsters[0] = monsterIdx;
				setMonsterSprite(_indoorList[156], monster, sprites, frame, INDOOR_MONSTERS_Y[0]);
			} else if (combat._attackMonsters[1] == -1) {
				combat._attackMonsters[1] = monsterIdx;
				setMonsterSprite(_indoorList[150], monster, sprites, frame, INDOOR_MONSTERS_Y[0]);
			} else if (combat._attackMonsters[2] == -1) {
				combat._attackMonsters[2] = monsterIdx;
				setMonsterSprite(_indoorList[153], monster, sprites, frame, INDOOR_MONSTERS_Y[0]);
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][7]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][7])) {
			monster._isAttacking = true;
			if (!_wo[27]) {
				if (combat._attackMonsters[3] == -1) {
					combat._attackMonsters[3] = monsterIdx;
					setMonsterSprite(_indoorList[132], monster, sprites, frame, INDOOR_MONSTERS_Y[1]);
				} else if (combat._attackMonsters[4] == -1) {
					combat._attackMonsters[4] = monsterIdx;
					setMonsterSprite(_indoorList[130], monster, sprites, frame, INDOOR_MONSTERS_Y[1]);
				} else if (combat._attackMonsters[2] == -1) {
					combat._attackMonsters[5] = monsterIdx;
					setMonsterSprite(_indoorList[131], monster, sprites, frame, INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][5]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][5])) {
			if (_wo[27] && _wo[25]) {
			} else if (_wo[27] && _wo[28]) {
			} else if (_wo[23] & _wo[25]) {
			} else if (_wo[23] && _wo[28]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[12] == -1) {
					combat._attackMonsters[12] = monsterIdx;
					setMonsterSprite(_indoorList[128], monster, sprites, frame, INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][9]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][9])) {
			if (_wo[27] && _wo[26]) {
			} else if (_wo[27] && _wo[29]) {
			} else if (_wo[24] & _wo[26]) {
			} else if (_wo[24] && _wo[29]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[13] == -1) {
					combat._attackMonsters[13] = monsterIdx;
					setMonsterSprite(_indoorList[129], monster, sprites, frame, INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][14]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][14])) {
			monster._isAttacking = true;

			if (!_wo[22] && !_wo[27]) {
				if (combat._attackMonsters[6] == -1) {
					combat._attackMonsters[6] = monsterIdx;
					setMonsterSprite(_indoorList[106], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				} else  if (combat._attackMonsters[7] == -1) {
					combat._attackMonsters[7] = monsterIdx;
					setMonsterSprite(_indoorList[104], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[8] == -1) {
					combat._attackMonsters[8] = monsterIdx;
					setMonsterSprite(_indoorList[105], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][12]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][12])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[22] & _wo[20]) {
			} else if (_wo[23] && _wo[17]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[14] == -1) {
					combat._attackMonsters[14] = monsterIdx;
					setMonsterSprite(_indoorList[100], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[20] == -1) {
					combat._attackMonsters[20] = monsterIdx;
					setMonsterSprite(_indoorList[101], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][16]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][16])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[22] & _wo[21]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[21] && _wo[19]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[15] == -1) {
					combat._attackMonsters[15] = monsterIdx;
					setMonsterSprite(_indoorList[102], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[21] == -1) {
					combat._attackMonsters[21] = monsterIdx;
					setMonsterSprite(_indoorList[103], monster, sprites, frame, INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][27]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][27])) {
			if (!_wo[27] && !_wo[22] && !_wo[15]) {
				monster._isAttacking = true;

				if (combat._attackMonsters[9] == -1) {
					combat._attackMonsters[9] = monsterIdx;
					setMonsterSprite(_indoorList[70], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[10] == -1) {
					combat._attackMonsters[10] = monsterIdx;
					setMonsterSprite(_indoorList[68], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[11] == -1) {
					combat._attackMonsters[11] = monsterIdx;
					setMonsterSprite(_indoorList[69], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][25]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][25])) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[17]) {
			} else if (_wo[15] && _wo[12]) {
			} else if (_wo[12] && _wo[7]) {
			} else if (_wo[17] && _wo[7]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[16] == -1) {
					combat._attackMonsters[16] = monsterIdx;
					setMonsterSprite(_indoorList[62], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[22] == -1) {
					combat._attackMonsters[22] = monsterIdx;
					setMonsterSprite(_indoorList[60], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[24] == -1) {
					combat._attackMonsters[24] = monsterIdx;
					setMonsterSprite(_indoorList[61], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][23]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][23])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (_wo[12] || _wo[8]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[18] == -1) {
					combat._attackMonsters[18] = monsterIdx;
					setMonsterSprite(_indoorList[66], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][29]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][29])) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[19]) {
			} else if (_wo[15] && _wo[14]) {
			} else if (_wo[14] && _wo[9]) {
			} else if (_wo[19] && _wo[9]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[17] == -1) {
					combat._attackMonsters[17] = monsterIdx;
					setMonsterSprite(_indoorList[65], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[23] == -1) {
					combat._attackMonsters[23] = monsterIdx;
					setMonsterSprite(_indoorList[63], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[25] == -1) {
					combat._attackMonsters[25] = monsterIdx;
					setMonsterSprite(_indoorList[64], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + SCREEN_POSITIONING_X[dir][31]) &&
				monster._position.y == (mazePos.y + SCREEN_POSITIONING_Y[dir][31])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[14] || _wo[10]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[19] == -1) {
					combat._attackMonsters[19] = monsterIdx;
					setMonsterSprite(_indoorList[67], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[23] == -1) {
					combat._attackMonsters[23] = monsterIdx;
					setMonsterSprite(_indoorList[63], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[25] == -1) {
					combat._attackMonsters[25] = monsterIdx;
					setMonsterSprite(_indoorList[64], monster, sprites, frame, INDOOR_MONSTERS_Y[3]);
				}
			}
		}
	}
}

void InterfaceMap::setMonsterSprite(DrawStruct &drawStruct, MazeMonster &monster, SpriteResource *sprites, 
		int frame, int defaultY) {
	MonsterStruct &monsterData = _vm->_map->_monsterData[monster._spriteId];
	bool flying = monsterData._flying;

	drawStruct._frame = frame;
	drawStruct._sprites = sprites;
	drawStruct._y = defaultY;

	if (flying) {
		drawStruct._x = COMBAT_FLOAT_X[_combatFloatCounter];
		drawStruct._y = COMBAT_FLOAT_Y[_combatFloatCounter];
	} else {
		drawStruct._x = 0;
	}

	drawStruct._flags &= SPRFLAG_HORIZ_FLIPPED | SPRFLAG_4000 | SPRFLAG_2000;
	if (monster._effect2)
		drawStruct._flags = MONSTER_EFFECT_FLAGS[monster._effect2][monster._effect3];
}

void InterfaceMap::setIndoorsObjects() {
	Common::Point mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;
	Common::Point pt;
	_objNumber = 0;

	Common::Array<MazeObject> &objects = _vm->_map->_mobData._objects;
	for (uint idx = 0; idx < objects.size(); ++idx) {
		MazeObject &mazeObject = objects[idx];

		// Determine which half of the X/Y lists to use
		int listOffset;
		if (_vm->_files->_isDarkCc) {
			listOffset = mazeObject._spriteId == 47 ? 1 : 0;
		} else {
			listOffset = mazeObject._spriteId == 113 ? 1 : 0;
		}

		// Position 1
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][2]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][2]) == mazeObject._position.y
			&& _indoorList._objects0._frame == -1) {
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
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][7]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][7]) == mazeObject._position.y
			&& !_wo[27] && _indoorList._objects1._frame == -1) {
			_indoorList._objects1._x = INDOOR_OBJECT_X[listOffset][1];
			_indoorList._objects1._y = INDOOR_OBJECT_Y[listOffset][1];
			_indoorList._objects1._frame = mazeObject._frame;
			_indoorList._objects1._sprites = mazeObject._sprites;
			_indoorList._objects1._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects1._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 3
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][5]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][5]) == mazeObject._position.y) {
			if (_wo[27] && _wo[25]) {
			} else if (_wo[27] && _wo[28]) {
			} else if (_wo[23] && _wo[25]) {
			} else if (_wo[23] && _wo[28]) {
			} else if (_indoorList._objects2._frame == -1) {
				_indoorList._objects2._x = INDOOR_OBJECT_X[listOffset][2];
				_indoorList._objects2._y = INDOOR_OBJECT_Y[listOffset][2];
				_indoorList._objects2._frame = mazeObject._frame;
				_indoorList._objects2._sprites = mazeObject._sprites;
				_indoorList._objects2._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects2._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 4
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][9]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][9]) == mazeObject._position.y) {
			if (_wo[27] && _wo[26]) {
			} else if (_wo[27] && _wo[29]) {
			} else if (_wo[24] && _wo[26]) {
			} else if (_wo[24] && _wo[29]) {
			} else if (_indoorList._objects3._frame == -1) {
				_indoorList._objects3._x = INDOOR_OBJECT_X[listOffset][3];
				_indoorList._objects3._y = INDOOR_OBJECT_Y[listOffset][3];
				_indoorList._objects3._frame = mazeObject._frame;
				_indoorList._objects3._sprites = mazeObject._sprites;
				_indoorList._objects3._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects3._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 5
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][14]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][14]) == mazeObject._position.y) {
			if (!_wo[22] && !_wo[27] && _indoorList._objects4._frame == -1) {
				_indoorList._objects4._x = INDOOR_OBJECT_X[listOffset][4];
				_indoorList._objects4._y = INDOOR_OBJECT_Y[listOffset][4];
				_indoorList._objects4._frame = mazeObject._frame;
				_indoorList._objects4._sprites = mazeObject._sprites;
				_indoorList._objects4._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects4._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 6
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][12]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][12]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_indoorList._objects5._frame == -1) {
				_indoorList._objects5._x = INDOOR_OBJECT_X[listOffset][5];
				_indoorList._objects5._y = INDOOR_OBJECT_Y[listOffset][5];
				_indoorList._objects5._frame = mazeObject._frame;
				_indoorList._objects5._sprites = mazeObject._sprites;
				_indoorList._objects5._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects5._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 7
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][16]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][16]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_indoorList._objects6._frame == -1) {
				_indoorList._objects6._x = INDOOR_OBJECT_X[listOffset][6];
				_indoorList._objects6._y = INDOOR_OBJECT_Y[listOffset][6];
				_indoorList._objects6._frame = mazeObject._frame;
				_indoorList._objects6._sprites = mazeObject._sprites;
				_indoorList._objects6._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects6._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 8
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][27]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][27]) == mazeObject._position.y) {
			if (!_wo[27] && !_wo[22] && !_wo[15] && _indoorList._objects7._frame == -1) {
				_indoorList._objects7._x = INDOOR_OBJECT_X[listOffset][7];
				_indoorList._objects7._y = INDOOR_OBJECT_Y[listOffset][7];
				_indoorList._objects7._frame = mazeObject._frame;
				_indoorList._objects7._sprites = mazeObject._sprites;
				_indoorList._objects7._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects7._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 9
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][25]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][25]) == mazeObject._position.y) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[17]) {
			} else if (_wo[15] && _wo[12]) {
			} else if (_wo[12] && _wo[7]) {
			} else if (_wo[17] && _wo[7]) {
			} else if (_indoorList._objects8._frame == -1) {
				_indoorList._objects8._x = INDOOR_OBJECT_X[listOffset][8];
				_indoorList._objects8._y = INDOOR_OBJECT_Y[listOffset][8];
				_indoorList._objects8._frame = mazeObject._frame;
				_indoorList._objects8._sprites = mazeObject._sprites;
				_indoorList._objects8._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects8._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 10
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][23]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][23]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (!_wo[12] && !_wo[8] && _indoorList._objects9._frame == -1) {
				_indoorList._objects9._x = INDOOR_OBJECT_X[listOffset][9];
				_indoorList._objects9._y = INDOOR_OBJECT_Y[listOffset][9];
				_indoorList._objects9._frame = mazeObject._frame;
				_indoorList._objects9._sprites = mazeObject._sprites;
				_indoorList._objects9._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects9._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Block 11
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][29]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][29]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[15] && _wo[19]) {
			} else if (_wo[15] && _wo[14]) {
			} else if (_wo[14] && _wo[9]) {
			} else if (_wo[19] && _wo[9]) {
			} else if (_indoorList._objects10._frame == -1) {
				_indoorList._objects10._x = INDOOR_OBJECT_X[listOffset][10];
				_indoorList._objects10._y = INDOOR_OBJECT_Y[listOffset][10];
				_indoorList._objects10._frame = mazeObject._frame;
				_indoorList._objects10._sprites = mazeObject._sprites;
				_indoorList._objects10._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects10._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Block 12
		if ((mazePos.x + SCREEN_POSITIONING_X[dir][31]) == mazeObject._position.x
			&& (mazePos.y + SCREEN_POSITIONING_Y[dir][31]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (!_wo[14] && !_wo[10] && _indoorList._objects11._frame == -1) {
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
}

void InterfaceMap::setIndoorsWallPics() {
	Map &map = *_vm->_map;
	const Common::Point &mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;

	Common::fill(&_wp[0], &_wp[20], -1);

	for (uint idx = 0; idx < map._mobData._wallItems.size(); ++idx) {
		MazeWallItem &wallItem = map._mobData._wallItems[idx];
		if (wallItem._direction != dir)
			continue;

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][2]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][2])) {
			if (_wp[1] == -1) {
				_indoorList[148]._frame = wallItem._frame;
				_indoorList[148]._sprites = wallItem._sprites;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][7]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][7])) {
			if (!_wo[27] && _wp[1] == -1) {
				_indoorList[123]._frame = wallItem._frame;
				_indoorList[123]._sprites = wallItem._sprites;
				_wp[4] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][5]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][5])) {
			if (_wo[27] && _wo[25]) {
			} else if (_wo[27] && _wo[28]) {
			} else if (_wo[23] && _wo[25]) {
			} else if (_wo[23] && _wo[28]) {
			} else if (_wp[3] == -1) {
				_indoorList[122]._frame = wallItem._frame;
				_indoorList[122]._sprites = wallItem._sprites;
				_wp[3] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][9]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][9])) {
			if (_wo[27] && _wo[26]) {
			} else if (_wo[27] && _wo[29]) {
			} else if (_wo[24] && _wo[26]) {
			} else if (_wo[24] && _wo[29]) {
			} else if (_wp[5] == -1) {
				_indoorList[124]._frame = wallItem._frame;
				_indoorList[124]._sprites = wallItem._sprites;
				_wp[5] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][14]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][14])) {
			if (!_wo[22] && !_wo[27] && !_wp[8]) {
				_indoorList[94]._frame = wallItem._frame;
				_indoorList[94]._sprites = wallItem._sprites;
				_wp[8] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][12]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][12])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_wp[7] == -1) {
				_indoorList[93]._frame = wallItem._frame;
				_indoorList[93]._sprites = wallItem._sprites;
				_wp[7] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][16]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][16])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_wp[9] == -1) {
				_indoorList[95]._frame = wallItem._frame;
				_indoorList[95]._sprites = wallItem._sprites;
				_wp[9] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][12]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][12])) {
			if (_wo[27]) {
			} else if (_wo[25] && _wo[28]) {
			} else if (_wo[20] && _wo[16]) {
			} else if (_wp[6] == -1) {
				_indoorList[92]._frame = wallItem._frame;
				_indoorList[92]._sprites = wallItem._sprites;
				_wp[6] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][16]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][16])) {
			if (!_wo[26] && !_wo[29] && !_wo[21] && !_wo[18] && _wp[10] == -1) {
				_indoorList[96]._frame = wallItem._frame;
				_indoorList[96]._sprites = wallItem._sprites;
				_wp[10] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][27]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][27])) {
			if (!_wo[27] && !_wo[22] && !_wo[15] && _wp[15] == -1) {
				_indoorList[50]._frame = wallItem._frame;
				_indoorList[50]._sprites = wallItem._sprites;
				_wp[15] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][25]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][25])) {
			if (_wo[27]) {
			} else if (_wo[27] && _wo[22]) {
			} else if (_wo[15] && _wo[17]) {
			} else if (_wo[15] && _wo[12]) {
			} else if (_wo[12] && _wo[7]) {
			} else if (_wo[17] && _wo[7]) {
			} else if (_wp[14] == -1) {
				_indoorList[49]._frame = wallItem._frame;
				_indoorList[49]._sprites = wallItem._sprites;
				_wp[14] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][23]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][23])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (_wo[12] && _wo[8]) {
			} else if (_wp[13] == -1) {
				_indoorList[48]._frame = wallItem._frame;
				_indoorList[48]._sprites = wallItem._sprites;
				_wp[13] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][29]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][29])) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[19]) {
			} else if (_wo[15] && _wo[14]) {
			} else if (_wo[14] && _wo[9]) {
			} else if (_wo[19] && _wo[9]) {
			} else if (_wp[16] == -1) {
				_indoorList[51]._frame = wallItem._frame;
				_indoorList[51]._sprites = wallItem._sprites;
				_wp[16] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][31]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][31])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (!_wo[14] && !_wo[10] && _wp[17] == -1) {
				_indoorList[52]._frame = wallItem._frame;
				_indoorList[52]._sprites = wallItem._sprites;
				_wp[17] = idx;
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][23]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][23])) {
			if (!_wo[27] && !_wo[20] && !_wo[12] && !_wo[23] && !_wo[8] && !_wo[30]) {
				if (_wp[12] == -1) {
					_indoorList[47]._frame = wallItem._frame;
					_indoorList[47]._sprites = wallItem._sprites;
					_wp[12] = idx;
				}
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][31]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][31])) {
			if (!_wo[27] && !_wo[21] && !_wo[14] && !_wo[24] && !_wo[10] && !_wo[31]) {
				if (_wp[18] == -1) {
					_indoorList[53]._frame = wallItem._frame;
					_indoorList[53]._sprites = wallItem._sprites;
					_wp[18] = idx;
				}
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][23]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][23])) {
			if (!_wo[25] && !_wo[28] && !_wo[20] && !_wo[11] && !_wo[16] && !_wo[30] && !_wo[32]) {
				if (_wp[11] == -1) {
					_indoorList[46]._frame = wallItem._frame;
					_indoorList[46]._sprites = wallItem._sprites;
					_wp[11] = idx;
				}
			}
		}

		if (mazePos.x == (wallItem._position.x + SCREEN_POSITIONING_X[dir][31]) &&
				mazePos.y == (wallItem._position.y + SCREEN_POSITIONING_Y[dir][31])) {
			if (!_wo[26] && !_wo[20] && !_wo[21] && !_wo[13] && !_wo[18] && !_wo[31] && !_wo[33]) {
				if (_wp[19] == -1) {
					_indoorList[54]._frame = wallItem._frame;
					_indoorList[54]._sprites = wallItem._sprites;
					_wp[19] = idx;
				}
			}
		}
	}
}

void InterfaceMap::setOutdoorsMonsters() {

}

void InterfaceMap::setOutdoorsObjects() {

}

void InterfaceMap::drawIndoors() {
	Map &map = *_vm->_map;
	int surfaceId;

	// Draw any surface tiles on top of the default ground
	for (int cellIndex = 0; cellIndex < 25; ++cellIndex) {
		map.getCell(DRAW_NUMBERS[cellIndex]);

		DrawStruct &drawStruct = _indoorList._groundTiles[cellIndex];
		SpriteResource &sprites = map._surfaceSprites[map._currentSurfaceId];
		drawStruct._sprites = sprites.empty() ? (SpriteResource *)nullptr : &sprites;

		surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];
		if (surfaceId == SURFTYPE_WATER || surfaceId == SURFTYPE_LAVA || 
				surfaceId == SURFTYPE_SEWER) {
			drawStruct._frame = DRAW_FRAMES[cellIndex][_flipWater ? 1 : 0];
			drawStruct._flags = _flipWater ? SPRFLAG_HORIZ_FLIPPED : 0;
		} else {
			drawStruct._frame = DRAW_FRAMES[cellIndex][_flipGround ? 1 : 0];
			drawStruct._flags = _flipGround ? SPRFLAG_HORIZ_FLIPPED : 0;
		}
	}

	if (!_wo[27] && !_wo[20] && !_wo[23] && !_wo[12] && !_wo[8] && !_wo[30]) {
		if (_wo[39])
			_indoorList._swl_4F4L._frame = 22;
		else if (_wo[83])
			_indoorList._swl_4F4L._frame = 46;
	}

	if (!_wo[27] && !_wo[22] && !_wo[17] && !_wo[12] && !_wo[8]) {
		if (_wo[38])
			_indoorList._swl_4F3L._frame = 20;
		else if (_wo[82])
			_indoorList._swl_4F3L._frame = 44;
	}

	if (!_wo[27] && !_wo[22] && !_wo[15] && !_wo[2] && !_wo[7]) {
		if (_wo[37])
			_indoorList._swl_4F2L._frame = 18;
		else if (_wo[81])
			_indoorList._swl_4F2L._frame = 42;
	}

	if (!_wo[27] && !_wo[22] && !_wo[15] && !_wo[6]) {
		if (_wo[36])
			_indoorList._swl_4F1L._frame = 16;
		else if (_wo[80])
			_indoorList._swl_4F1L._frame = 40;
	}

	if (!_wo[27] && !_wo[21] && !_wo[24] && !_wo[14] && !_wo[10] && !_wo[31]) {
		if (_wo[43])
			_indoorList._swl_4F4R._frame = 23;
		else if (_wo[87])
			_indoorList._swl_4F4R._frame = 47;
	}

	if (!_wo[27] && !_wo[22] && !_wo[19] && !_wo[14] && !_wo[10]) {
		if (_wo[42])
			_indoorList._swl_4F3R._frame = 21;
		else if (_wo[86])
			_indoorList._swl_4F3R._frame = 45;
	}

	if (!_wo[27] && !_wo[22] && !_wo[15] && !_wo[5] && !_wo[9]) {
		if (_wo[41])
			_indoorList._swl_4F2R._frame = 19;
		else if (_wo[85])
			_indoorList._swl_4F2R._frame = 43;
	}

	if (!_wo[27] && !_wo[22] && !_wo[15] && !_wo[6]) {
		if (_wo[40])
			_indoorList._swl_4F1R._frame = 17;
		else if (_wo[84])
			_indoorList._swl_4F1R._frame = 41;
	}

	if (!_wo[25] && !_wo[28] && !_wo[20] && !_wo[11] &&
		!_wo[16] && !_wo[30] && !_wo[32]) {
		if (_wo[88])
			_indoorList._fwl_4F4L._frame = 7;
		else if (_wo[78])
			_indoorList._fwl_4F4L._frame = 16;
		else if (_wo[148])
			_indoorList._fwl_4F4L._frame = _overallFrame + 1;
		else if (_wo[108])
			_indoorList._fwl_4F4L._frame = 8;
		else if (_wo[168])
			_indoorList._fwl_4F4L._frame = 10;
		else if (_wo[128])
			_indoorList._fwl_4F4L._frame = 9;
		else if (_wo[34])
			_indoorList._fwl_4F4L._frame = 0;
		else if (_wo[188])
			_indoorList._fwl_4F4L._frame = 15;
		else if (_wo[208])
			_indoorList._fwl_4F4L._frame = 14;
		else if (_wo[228])
			_indoorList._fwl_4F4L._frame = 6;
		else if (_wo[248])
			_indoorList._fwl_4F4L._frame = 11;
		else if (_wo[268])
			_indoorList._fwl_4F4L._frame = 12;
		else if (_wo[288])
			_indoorList._fwl_4F4L._frame = 13;
	}

	if (!_wo[26] && !_wo[29] && !_wo[21] && !_wo[13] && !_wo[18] && !_wo[31] && !_wo[33]) {
		if (_wo[93])
			_indoorList._fwl_4F4R._frame = 7;
		else if (_wo[79])
			_indoorList._fwl_4F4R._frame = 16;
		else if (_wo[153])
			_indoorList._fwl_4F4R._frame = _overallFrame + 1;
		else if (_wo[113])
			_indoorList._fwl_4F4R._frame = 8;
		else if (_wo[173])
			_indoorList._fwl_4F4R._frame = 10;
		else if (_wo[133])
			_indoorList._fwl_4F4R._frame = 9;
		else if (_wo[35])
			_indoorList._fwl_4F4R._frame = 0;
		else if (_wo[79])
			_indoorList._fwl_4F4R._frame = 15;
		else if (_wo[213])
			_indoorList._fwl_4F4R._frame = 14;
		else if (_wo[233])
			_indoorList._fwl_4F4R._frame = 6;
		else if (_wo[253])
			_indoorList._fwl_4F4R._frame = 11;
		else if (_wo[273])
			_indoorList._fwl_4F4R._frame = 12;
		else if (_wo[293])
			_indoorList._fwl_4F4R._frame = 13;
	}

	if (!_wo[25] && !_wo[28] && !_wo[20] && !_wo[11] && !_wo[16] && !_wo[30]) {
		if (_wo[32])
			_indoorList._swl_3F4L._frame = 14;
		else if (_wo[76])
			_indoorList._swl_3F4L._frame = 38;
	}

	if (!_wo[26] && !_wo[29] && !_wo[21] && !_wo[13] && !_wo[18] && !_wo[31]) {
		if (_wo[33])
			_indoorList._fwl_3F1R._frame = 15;
		else if (_wo[77])
			_indoorList._swl_3F1R._frame = 39;
	}

	if (_wo[28] && _wo[27]) {
	} else if (_wo[28] && _wo[12]) {
	} else if (_wo[28] && _wo[23]) {
	} else if (_wo[28] && _wo[8]) {
	} else if (_wo[25] && _wo[27]) {
	} else if (_wo[25] && _wo[12]) {
	} else if (_wo[25] && _wo[23]) {
	} else if (_wo[25] && _wo[8]) {
	} else if (_wo[11] && _wo[27]) {
	} else if (_wo[11] && _wo[12]) {
	} else if (_wo[11] && _wo[23]) {
	} else if (_wo[11] && _wo[8]) {
	} else if (_wo[17] && _wo[27]) {
	} else if (_wo[17] && _wo[12]) {
	} else if (_wo[17] && _wo[23]) {
	} else if (_wo[17] && _wo[8]) {
	} else if (_wo[20]) {
	} else if (_wo[30]) {
		_indoorList._swl_3F3L._frame = 12;
	} else if (_wo[74]) {
		_indoorList._swl_3F3L._frame = 36;
	}

	if (_wo[29] && _wo[27]) {
	} else if (_wo[29] && _wo[14]) {
	} else if (_wo[29] && _wo[24]) {
	} else if (_wo[29] && _wo[10]) {
	} else if (_wo[26] && _wo[27]) {
	} else if (_wo[26] && _wo[14]) {
	} else if (_wo[26] && _wo[24]) {
	} else if (_wo[26] && _wo[10]) {
	} else if (_wo[13] && _wo[27]) {
	} else if (_wo[13] && _wo[14]) {
	} else if (_wo[13] && _wo[24]) {
	} else if (_wo[13] && _wo[10]) {
	} else if (_wo[19] && _wo[27]) {
	} else if (_wo[19] && _wo[24]) {
	} else if (_wo[19] && _wo[10]) {
	} else if (_wo[21]) {
	} else if (_wo[31]) {
		_indoorList._swl_3F2R._frame = 13;
	} else if (_wo[75]) {
		_indoorList._swl_3F2R._frame = 37;
	}

	if (!_wo[27] && !_wo[20] && !_wo[12] && !_wo[23] && !_wo[8] && !_wo[30]) {
		if (_wo[89])
			_indoorList._fwl_4F3L._frame = 7;
		else if (_wo[44])
			_indoorList._fwl_4F3L._frame = 16;
		else if (_wo[149])
			_indoorList._fwl_4F3L._frame = _overallFrame + 1;
		else if (_wo[109])
			_indoorList._fwl_4F3L._frame = 8;
		else if (_wo[169])
			_indoorList._fwl_4F3L._frame = 10;
		else if (_wo[129])
			_indoorList._fwl_4F3L._frame = 9;
		else if (_wo[0])
			_indoorList._fwl_4F3L._frame = 0;
		else if (_wo[189])
			_indoorList._fwl_4F3L._frame = 15;
		else if (_wo[209])
			_indoorList._fwl_4F3L._frame = 14;
		else if (_wo[229])
			_indoorList._fwl_4F3L._frame = 6;
		else if (_wo[249])
			_indoorList._fwl_4F3L._frame = 11;
		else if (_wo[269])
			_indoorList._fwl_4F3L._frame = 12;
		else if (_wo[289])
			_indoorList._fwl_4F3L._frame = 13;
	}

	if (_wo[22] && _wo[20]) {
	} else if (_wo[22] && _wo[23]) {
	} else if (_wo[20] && _wo[17]) {
	} else if (_wo[23] && _wo[17]) {
	} else if (_wo[12]) {
	} else if (_wo[8]) {
	} else if (_wo[90]) {
		_indoorList._fwl_4F2L._frame = 7;
	} else if (_wo[45])  {
		_indoorList._fwl_4F2L._frame = 16;
	} else if (_wo[150]) {
		_indoorList._fwl_4F2L._frame = _overallFrame + 1;
	} else if (_wo[110]) {
		_indoorList._fwl_4F2L._frame = 8;
	} else if (_wo[170]) {
		_indoorList._fwl_4F2L._frame = 10;
	} else if (_wo[130]) {
		_indoorList._fwl_4F2L._frame = 9;
	} else if (_wo[1]) {
		_indoorList._fwl_4F2L._frame = 0;
	} else if (_wo[190]) {
		_indoorList._fwl_4F2L._frame = 15;
	} else if (_wo[210]) {
		_indoorList._fwl_4F2L._frame = 14;
	} else if (_wo[230]) {
		_indoorList._fwl_4F2L._frame = 6;
	} else if (_wo[250]) {
		_indoorList._fwl_4F2L._frame = 11;
	} else if (_wo[270]) {
		_indoorList._fwl_4F2L._frame = 12;
	} else if (_wo[290]) {
		_indoorList._fwl_4F2L._frame = 13;
	}

	if (_wo[15] && _wo[17]) {
	} else if (_wo[15] && _wo[12]) {
	} else if (_wo[12] && _wo[7]) {
	} else if (_wo[17] && _wo[7]) {
	} else if (_wo[91]) {
		_indoorList._fwl_4F1L._frame = 7;
	} else if (_wo[46]) {
		_indoorList._fwl_4F1L._frame = 16;
	} else if (_wo[151]) {
		_indoorList._fwl_4F1L._frame = _overallFrame + 1;
	} else if (_wo[111]) {
		_indoorList._fwl_4F1L._frame = 8;
	} else if (_wo[171]) {
		_indoorList._fwl_4F1L._frame = 10;
	} else if (_wo[131]) {
		_indoorList._fwl_4F1L._frame = 9;
	} else if (_wo[2]) {
		_indoorList._fwl_4F1L._frame = 0;
	} else if (_wo[191]) {
		_indoorList._fwl_4F1L._frame = 15;
	} else if (_wo[211]) {
		_indoorList._fwl_4F1L._frame = 14;
	} else if (_wo[231]) {
		_indoorList._fwl_4F1L._frame = 6;
	} else if (_wo[251]) {
		_indoorList._fwl_4F1L._frame = 11;
	} else if (_wo[271]) {
		_indoorList._fwl_4F1L._frame = 12;
	} else if (_wo[291]) {
		_indoorList._fwl_4F1L._frame = 13;
	}

	if (!_wo[27] && !_wo[21] && !_wo[14] && !_wo[24] && !_wo[10] && !_wo[31]) {
		if (_wo[92]) {
			_indoorList._fwl_4F3R._frame = 7;
		} else if (_wo[47]) {
			_indoorList._fwl_4F3R._frame = 16;
		} else if (_wo[152]) {
			_indoorList._fwl_4F3R._frame = _overallFrame + 1;
		} else if (_wo[112]) {
			_indoorList._fwl_4F3R._frame = 8;
		} else if (_wo[172]) {
			_indoorList._fwl_4F3R._frame = 10;
		} else if (_wo[132]) {
			_indoorList._fwl_4F3R._frame = 9;
		} else if (_wo[3]) {
			_indoorList._fwl_4F3R._frame = 0;
		} else if (_wo[192]) {
			_indoorList._fwl_4F3R._frame = 15;
		} else if (_wo[212]) {
			_indoorList._fwl_4F3R._frame = 14;
		} else if (_wo[232]) {
			_indoorList._fwl_4F3R._frame = 6;
		} else if (_wo[252]) {
			_indoorList._fwl_4F3R._frame = 11;
		} else if (_wo[272]) {
			_indoorList._fwl_4F3R._frame = 12;
		} else if (_wo[292]) {
			_indoorList._fwl_4F3R._frame = 13;
		}
	}

	if (_wo[22] && _wo[21]) {
	} else if (_wo[22] && _wo[24]) {
	} else if (_wo[21] && _wo[19]) {
	} else if (_wo[24] && _wo[19]) {
	} else if (_wo[14] || _wo[10]) {
	} else if (_wo[94]) {
		_indoorList._fwl_4F2R._frame = 7;
	} else if (_wo[48]) {
		_indoorList._fwl_4F2R._frame = 16;
	} else if (_wo[154]) {
		_indoorList._fwl_4F2R._frame = _overallFrame + 1;
	} else if (_wo[114]) {
		_indoorList._fwl_4F2R._frame = 8;
	} else if (_wo[174]) {
		_indoorList._fwl_4F2R._frame = 10;
	} else if (_wo[134]) {
		_indoorList._fwl_4F2R._frame = 9;
	} else if (_wo[4]) {
		_indoorList._fwl_4F2R._frame = 0;
	} else if (_wo[194]) {
		_indoorList._fwl_4F2R._frame = 15;
	} else if (_wo[214]) {
		_indoorList._fwl_4F2R._frame = 14;
	} else if (_wo[234]) {
		_indoorList._fwl_4F2R._frame = 6;
	} else if (_wo[254]) {
		_indoorList._fwl_4F2R._frame = 11;
	} else if (_wo[274]) {
		_indoorList._fwl_4F2R._frame = 12;
	} else if (_wo[294]) {
		_indoorList._fwl_4F2R._frame = 13;
	}

	if (_wo[15] && _wo[19]) {
	} else if (_wo[15] && _wo[14]) {
	} else if (_wo[14] && _wo[9]) {
	} else if (_wo[19] && _wo[9]) {
	} else if (_wo[95]) {
		_indoorList._fwl_4F1R._frame = 7;
	} else if (_wo[49]) {
		_indoorList._fwl_4F1R._frame = 16;
	} else if (_wo[155]) {
		_indoorList._fwl_4F1R._frame = _overallFrame + 1;
	} else if (_wo[115]) {
		_indoorList._fwl_4F1R._frame = 8;
	} else if (_wo[175]) {
		_indoorList._fwl_4F1R._frame = 10;
	} else if (_wo[135]) {
		_indoorList._fwl_4F1R._frame = 9;
	} else if (_wo[5]) {
		_indoorList._fwl_4F1R._frame = 0;
	} else if (_wo[195]) {
		_indoorList._fwl_4F1R._frame = 15;
	} else if (_wo[215]) {
		_indoorList._fwl_4F1R._frame = 14;
	} else if (_wo[235]) {
		_indoorList._fwl_4F1R._frame = 6;
	} else if (_wo[255]) {
		_indoorList._fwl_4F1R._frame = 11;
	} else if (_wo[275]) {
		_indoorList._fwl_4F1R._frame = 12;
	} else if (_wo[295]) {
		_indoorList._fwl_4F1R._frame = 13;
	}

	if (_wo[27] || _wo[22] || _wo[15] || _wo[96]) {
	} else if (_wo[50]) {
		_indoorList._fwl_4F._frame = 16;
	} else if (_wo[156]) {
		_indoorList._fwl_4F._frame = _overallFrame + 1;
	} else if (_wo[116]) {
		_indoorList._fwl_4F._frame = 8;
	} else if (_wo[176]) {
		_indoorList._fwl_4F._frame = 10;
	} else if (_wo[136]) {
		_indoorList._fwl_4F._frame = 9;
	} else if (_wo[6]) {
		_indoorList._fwl_4F._frame = 0;
	} else if (_wo[196]) {
		_indoorList._fwl_4F._frame = 15;
	} else if (_wo[216]) {
		_indoorList._fwl_4F._frame = 14;
	} else if (_wo[236]) {
		_indoorList._fwl_4F._frame = 6;
	} else if (_wo[256]) {
		_indoorList._fwl_4F._frame = 11;
	} else if (_wo[276]) {
		_indoorList._fwl_4F._frame = 12;
	} else if (_wo[296]) {
		_indoorList._fwl_4F._frame = 13;
	}

	if (!_wo[27] && !_wo[22] && !_wo[15]) {
		if (_wo[7])
			_indoorList._swl_3F1L._frame = 8;
		else if (_wo[51])
			_indoorList._swl_3F1L._frame = 32;
	}

	if (_wo[22] && _wo[23]) {
	} else if (_wo[22] && _wo[20]) {
	} else if (_wo[17] && _wo[23]) {
	} else if (_wo[17] && _wo[20]) {
	} else if (_wo[8]) {
		_indoorList._swl_3F2L._frame = 10;
	} else if (_wo[52]) {
		_indoorList._swl_3F2L._frame = 34;
	}

	if (_wo[27] || _wo[22] || _wo[15]) {
	} else if (_wo[9]) {
		_indoorList._swl_3F4R._frame = 9;
	} else if (_wo[53]) {
		_indoorList._swl_3F4R._frame = 33;
	}

	if (_wo[22] && _wo[24]) {
	} else if (_wo[22] && _wo[21]) {
	} else if (_wo[19] && _wo[24]) {
	} else if (_wo[19] && _wo[21]) {
	} else if (_wo[14]) {
	} else if (_wo[10]) {
		_indoorList._swl_3F3R._frame = 11;
	} else if (_wo[54]) {
		_indoorList._swl_3F3R._frame = 35;
	}

	if (_wo[25] || _wo[28] || _wo[20] || _wo[16]) {
	} else if (_wo[97]) {
		_indoorList._fwl_3F2L._frame = 24;
	} else if (_wo[55]) {
		_indoorList._fwl_3F2L._frame = 33;
	} else if (_wo[137]) {
		_indoorList._fwl_3F2L._frame = 26;
	} else if (_wo[157]) {
		_indoorList._fwl_3F2L._frame = _overallFrame + 18;
	} else if (_wo[117]) {
		_indoorList._fwl_3F2L._frame = 25;
	} else if (_wo[177]) {
		_indoorList._fwl_3F2L._frame = 27;
	} else if (_wo[11]) {
		_indoorList._fwl_3F2L._frame = 17;
	} else if (_wo[197]) {
		_indoorList._fwl_3F2L._frame = 32;
	} else if (_wo[217]) {
		_indoorList._fwl_3F2L._frame = 31;
	} else if (_wo[237]) {
		_indoorList._fwl_3F2L._frame = 23;
	} else if (_wo[257]) {
		_indoorList._fwl_3F2L._frame = 28;
	} else if (_wo[277]) {
		_indoorList._fwl_3F2L._frame = 29;
	} else if (_wo[297]) {
		_indoorList._fwl_3F2L._frame = 30;
	}

	if (_wo[22] && _wo[23]) {
	} else if (_wo[22] && _wo[20]) {
	} else if (_wo[23] && _wo[17]) {
	} else if (_wo[20] && _wo[17]) {
	} else if (_wo[98]) {
		_indoorList._fwl_3F1L._frame = 24;
	} else if (_wo[56]) {
		_indoorList._fwl_3F1L._frame = 33;
	} else if (_wo[178]) {
		_indoorList._fwl_3F1L._frame = 27;
	} else if (_wo[118]) {
		_indoorList._fwl_3F1L._frame = 25;
	} else if (_wo[158]) {
		_indoorList._fwl_3F1L._frame = _overallFrame + 18;
	} else if (_wo[138]) {
		_indoorList._fwl_3F1L._frame = 26;
	} else if (_wo[12]) {
		_indoorList._fwl_3F1L._frame = 17;
	} else if (_wo[198]) {
		_indoorList._fwl_3F1L._frame = 32;
	} else if (_wo[218]) {
		_indoorList._fwl_3F1L._frame = 31;
	} else if (_wo[238]) {
		_indoorList._fwl_3F1L._frame = 23;
	} else if (_wo[258]) {
		_indoorList._fwl_3F1L._frame = 28;
	} else if (_wo[278]) {
		_indoorList._fwl_3F1L._frame = 29;
	} else if (_wo[298]) {
		_indoorList._fwl_3F1L._frame = 30;
	}

	if (_wo[26] || _wo[29] || _wo[21] || _wo[18]) {
	} else if (_wo[99]) {
		_indoorList._fwl_3F2R._frame = 24;
	} else if (_wo[57]) {
		_indoorList._fwl_3F2R._frame = 33;
	} else if (_wo[139]) {
		_indoorList._fwl_3F2R._frame = 26;
	} else if (_wo[159]) {
		_indoorList._fwl_3F2R._frame = _overallFrame + 18;
	} else if (_wo[119]) {
		_indoorList._fwl_3F2R._frame = 25;
	} else if (_wo[179]) {
		_indoorList._fwl_3F2R._frame = 27;
	} else if (_wo[13]) {
		_indoorList._fwl_3F2R._frame = 17;
	} else if (_wo[199]) {
		_indoorList._fwl_3F2R._frame = 32;
	} else if (_wo[219]) {
		_indoorList._fwl_3F2R._frame = 31;
	} else if (_wo[239]) {
		_indoorList._fwl_3F2R._frame = 23;
	} else if (_wo[259]) {
		_indoorList._fwl_3F2R._frame = 28;
	} else if (_wo[279]) {
		_indoorList._fwl_3F2R._frame = 29;
	} else if (_wo[299]) {
		_indoorList._fwl_3F2R._frame = 30;
	}

	if (_wo[22] && _wo[24]) {
	} else if (_wo[22] && _wo[21]) {
	} else if (_wo[24] && _wo[19]) {
	} else if (_wo[21] && _wo[19]) {
	} else if (_wo[100]) {
		_indoorList._fwl_3F1R._frame = 24;
	} else if (_wo[58]) {
		_indoorList._fwl_3F1R._frame = 33;
	} else if (_wo[140]) {
		_indoorList._fwl_3F1R._frame = 26;
	} else if (_wo[160]) {
		_indoorList._fwl_3F1R._frame = _overallFrame + 18;
	} else if (_wo[120]) {
		_indoorList._fwl_3F1R._frame = 25;
	} else if (_wo[180]) {
		_indoorList._fwl_3F1R._frame = 27;
	} else if (_wo[14]) {
		_indoorList._fwl_3F1R._frame = 17;
	} else if (_wo[200]) {
		_indoorList._fwl_3F1R._frame = 32;
	} else if (_wo[220]) {
		_indoorList._fwl_3F1R._frame = 31;
	} else if (_wo[240]) {
		_indoorList._fwl_3F1R._frame = 23;
	} else if (_wo[260]) {
		_indoorList._fwl_3F1R._frame = 28;
	} else if (_wo[280]) {
		_indoorList._fwl_3F1R._frame = 29;
	} else if (_wo[300]) {
		_indoorList._fwl_3F1R._frame = 30;
	}

	if (_wo[22] || _wo[27]) {
	} else if (_wo[101]) {
		_indoorList._fwl_3F._frame = 24;
	} else if (_wo[59]) {
		_indoorList._fwl_3F._frame = 33;
	} else if (_wo[141]) {
		_indoorList._fwl_3F._frame = 26;
	} else if (_wo[161]) {
		_indoorList._fwl_3F._frame = _overallFrame + 18;
	} else if (_wo[121]) {
		_indoorList._fwl_3F._frame = 25;
	} else if (_wo[181]) {
		_indoorList._fwl_3F._frame = 27;
	} else if (_wo[15]) {
		_indoorList._fwl_3F._frame = 17;
	} else if (_wo[201]) {
		_indoorList._fwl_3F._frame = 32;
	} else if (_wo[221]) {
		_indoorList._fwl_3F._frame = 31;
	} else if (_wo[241]) {
		_indoorList._fwl_3F._frame = 23;
	} else if (_wo[261]) {
		_indoorList._fwl_3F._frame = 28;
	} else if (_wo[281]) {
		_indoorList._fwl_3F._frame = 29;
	} else if (_wo[301]) {
		_indoorList._fwl_3F._frame = 30;
	}

	if (_wo[25] || _wo[28] || _wo[20]) {
	} else if (_wo[16]) {
		_indoorList._swl_2F2L._frame = 6;
	} else if (_wo[60]) {
		_indoorList._swl_2F2L._frame = 30;
	}

	if (_wo[27] || _wo[22]) {
	} else if (_wo[17]) {
		_indoorList._swl_2F1L._frame = 4;
	} else if (_wo[61]) {
		_indoorList._swl_2F1L._frame = 28;
	}

	if (_wo[26] || _wo[29] || _wo[21]) {
	} else if (_wo[18]) {
		_indoorList._swl_2F2R._frame = 7;
	} else if (_wo[62]) {
		_indoorList._swl_2F2R._frame = 31;
	}

	if (_wo[27] || _wo[22]) {
	} else if (_wo[19]) {
		_indoorList._swl_2F1R._frame = 5;
	} else if (_wo[63]) {
		_indoorList._swl_2F1R._frame = 29;
	}

	if (_wo[27] && _wo[25]) {
	} else if (_wo[27] && _wo[28]) {
	} else if (_wo[23] & _wo[25]) {
	} else if (_wo[23] && _wo[28]) {
	} else if (_wo[102]) {
		_indoorList._fwl_2F1L._frame = 7;
	} else if (_wo[64]) {
		_indoorList._fwl_2F1L._frame = 16;
	} else if (_wo[182]) {
		_indoorList._fwl_2F1L._frame = 10;
	} else if (_wo[122]) {
		_indoorList._fwl_2F1L._frame = 8;
	} else if (_wo[142]) {
		_indoorList._fwl_2F1L._frame = 9;
	} else if (_wo[162]) {
		_indoorList._fwl_2F1L._frame = _overallFrame + 1;
	} else if (_wo[20]) {
		_indoorList._fwl_2F1L._frame = 0;
	} else if (_wo[202]) {
		_indoorList._fwl_2F1L._frame = 15;
	} else if (_wo[222]) {
		_indoorList._fwl_2F1L._frame = 14;
	} else if (_wo[242]) {
		_indoorList._fwl_2F1L._frame = 6;
	} else if (_wo[262]) {
		_indoorList._fwl_2F1L._frame = 11;
	} else if (_wo[282]) {
		_indoorList._fwl_2F1L._frame = 12;
	} else if (_wo[302]) {
		_indoorList._fwl_2F1L._frame = 13;
	}

	if (_wo[27] && _wo[26]) {
	} else if (_wo[27] && _wo[29]) {
	} else if (_wo[24] && _wo[26]) {
	} else if (_wo[24] && _wo[29]) {
	} else if (_wo[103]) {
		_indoorList._fwl_2F1R._frame = 7;
	} else if (_wo[65]) {
		_indoorList._fwl_2F1R._frame = 16;
	} else if (_wo[183]) {
		_indoorList._fwl_2F1R._frame = 10;
	} else if (_wo[123]) {
		_indoorList._fwl_2F1R._frame = 8;
	} else if (_wo[143]) {
		_indoorList._fwl_2F1R._frame = 9;
	} else if (_wo[163]) {
		_indoorList._fwl_2F1R._frame = _overallFrame + 1;
	} else if (_wo[21]) {
		_indoorList._fwl_2F1R._frame = 0;
	} else if (_wo[203]) {
		_indoorList._fwl_2F1R._frame = 15;
	} else if (_wo[223]) {
		_indoorList._fwl_2F1R._frame = 14;
	} else if (_wo[243]) {
		_indoorList._fwl_2F1R._frame = 6;
	} else if (_wo[263]) {
		_indoorList._fwl_2F1R._frame = 11;
	} else if (_wo[283]) {
		_indoorList._fwl_2F1R._frame = 12;
	} else if (_wo[303]) {
		_indoorList._fwl_2F1R._frame = 13;
	}

	if (_wo[27]) {
	} else if (_wo[104]) {
		_indoorList._fwl_2F._frame = 7;
	} else if (_wo[66]) {
		_indoorList._fwl_2F._frame = 16;
	} else if (_wo[184]) {
		_indoorList._fwl_2F._frame = 10;
	} else if (_wo[124]) {
		_indoorList._fwl_2F._frame = 8;
	} else if (_wo[144]) {
		_indoorList._fwl_2F._frame = 9;
	} else if (_wo[164]) {
		_indoorList._fwl_2F._frame = _overallFrame + 1;
	} else if (_wo[22]) {
		_indoorList._fwl_2F._frame = 0;
	} else if (_wo[204]) {
		_indoorList._fwl_2F._frame = 15;
	} else if (_wo[224]) {
		_indoorList._fwl_2F._frame = 14;
	} else if (_wo[244]) {
		_indoorList._fwl_2F._frame = 6;
	} else if (_wo[264]) {
		_indoorList._fwl_2F._frame = 11;
	} else if (_wo[284]) {
		_indoorList._fwl_2F._frame = 12;
	} else if (_wo[304]) {
		_indoorList._fwl_2F._frame = 13;
	}

	if (_wo[27]) {
	} else if (_wo[23]) {
		_indoorList._swl_1F1L._frame = 2;
	} else if (_wo[67]) {
		_indoorList._swl_1F1L._frame = 26;
	}

	if (_wo[27]) {
	} else if (_wo[24]) {
		_indoorList._swl_1F1R._frame = 3;
	} else if (_wo[68]) {
		_indoorList._swl_1F1R._frame = 27;
	}

	if (_wo[28]) {
	} else if (_wo[105] || _wo[25] || _wo[165] || _wo[125] || _wo[185] || _wo[145]) {
		_indoorList._fwl_1F1L._frame = 0;
		_indoorList._fwl_1F1L._sprites = &map._wallSprites._fwl1;
	} else if (_wo[69]) {
		_indoorList._fwl_1F1L._frame = 9;
		_indoorList._fwl_1F1L._sprites = &map._wallSprites._fwl2;
	}

	if (_wo[29]) {
	} else if (_wo[106] || _wo[26] || _wo[166] || _wo[126] || _wo[186] || _wo[146]) {
		_indoorList._fwl_1F._frame = 0;
		_indoorList._fwl_1F._sprites = &map._wallSprites._fwl1;
	} else if (_wo[70]) {
		_indoorList._fwl_1F._frame = 9;
		_indoorList._fwl_1F._sprites = &map._wallSprites._fwl2;
	}

	if (_wo[107]) {
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
		if (_vm->_openDoor)
			_indoorList._fwl_1F1R._frame = 0;
		else
			_indoorList._fwl_1F1R._frame = map.mazeData()._wallKind ? 1 : 10;
	} else if (_wo[71]) {
		_indoorList._fwl_1F1R._frame = 9;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[167]) {
		_indoorList._fwl_1F1R._frame = _overallFrame + 1;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[127]) {
		_indoorList._fwl_1F1R._frame = 1;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[147]) {
		_indoorList._fwl_1F1R._frame = 2;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[187]) {
		_indoorList._fwl_1F1R._frame = 3;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[27]) {
		_indoorList._fwl_1F1R._frame = 0;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl1;
	} else if (_wo[207]) {
		_indoorList._fwl_1F1R._frame = 8;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[227]) {
		_indoorList._fwl_1F1R._frame = 7;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[247]) {
		_indoorList._fwl_1F1R._frame = 6;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl1;
	} else if (_wo[267]) {
		_indoorList._fwl_1F1R._frame = 4;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[287]) {
		_indoorList._fwl_1F1R._frame = 5;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[307]) {
		_indoorList._fwl_1F1R._frame = 6;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	}

	if (_wo[28]) {
		_indoorList._swl_0F1L._frame = 0;
	} else if (_wo[72]) {
		_indoorList._swl_0F1L._frame = 24;
	}

	if (_wo[29]) {
		_indoorList._swl_0F1R._frame = 1;
	} else if (_wo[73]) {
		_indoorList._swl_0F1R._frame = 25;
	}

	map.cellFlagLookup(_vm->_party->_mazePosition);

	assert(map._currentSky < 2);
	_indoorList[0]._sprites = &map._skySprites[map._currentSky];
	_indoorList[0]._flags = _flipSky ? SPRFLAG_HORIZ_FLIPPED : 0;

	if (_vm->_openDoor) {
		Common::Point pt(
			_vm->_party->_mazePosition.x + SCREEN_POSITIONING_X[
				_vm->_party->_mazeDirection][_vm->_party->_mazePosition.x],
			_vm->_party->_mazePosition.y + SCREEN_POSITIONING_Y[
				_vm->_party->_mazeDirection][_vm->_party->_mazePosition.y]
			);
		map.cellFlagLookup(pt);
		
		_indoorList._sky2._sprites = &map._skySprites[0];
	} else {
		_indoorList._sky2._sprites = _indoorList[0]._sprites;
	}

	_indoorList._sky2._flags = _flipSky ? SPRFLAG_HORIZ_FLIPPED : 0;
	_indoorList._ground._flags = _flipDefaultGround ? SPRFLAG_HORIZ_FLIPPED : 0;
	_indoorList._horizon._frame = 7;

	// Finally draw the darn indoor scene
	_vm->_screen->_windows[3].drawList(&_indoorList[0], _indoorList.size());

	// Check for any character shooting
	_isShooting = false;
	for (int i = 0; i < _vm->_party->_partyCount; ++i) {
		if (_vm->_combat->_shooting[i])
			_isShooting = true;
	}
	
	_charsShooting = _isShooting;
}

void InterfaceMap::moveMonsters() {
	// TODO
}

void InterfaceMap::assembleBorder() {
	Screen &screen = *_vm->_screen;

	// Draw the outer frame
	_globalSprites.draw(screen._windows[0], 0, Common::Point(8, 8));

	// Draw the animating bat character used to show when levitate is active
	_borderSprites.draw(screen._windows[0], _vm->_party->_levitateActive ? _batUIFrame + 16 : 16,
		Common::Point(0, 82));
	_batUIFrame = (_batUIFrame + 1) % 12;

	// Draw UI element to indicate whether can spot hidden doors
	_borderSprites.draw(screen,
		(_thinWall && _vm->_party->checkSkill(SPOT_DOORS)) ? _spotDoorsUIFrame + 28 : 28,
		Common::Point(194, 91));
	_spotDoorsUIFrame = (_spotDoorsUIFrame + 1) % 12;

	// Draw UI element to indicate whether can sense danger
	_borderSprites.draw(screen,
		(_vm->_dangerSenseAllowed && _vm->_party->checkSkill(DANGER_SENSE)) ? _spotDoorsUIFrame + 40 : 40,
		Common::Point(107, 9));
	_dangerSenseUIFrame = (_dangerSenseUIFrame + 1) % 12;

	// Handle the face UI elements for indicating clairvoyance status
	_face1UIFrame = (_face1UIFrame + 1) % 4;
	if (_face1State == 0)
		_face1UIFrame += 4;
	else if (_face1State == 2)
		_face1UIFrame = 0;

	_face2UIFrame = (_face2UIFrame + 1) % 4 + 12;
	if (_face2State == 0)
		_face2UIFrame += 252;
	else if (_face2State == 2)
		_face2UIFrame = 0;

	if (!_vm->_party->_clairvoyanceActive) {
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
		_fecpSprites.draw(screen, _vm->_party->_fireResistence ? 1 : 0,
			Common::Point(2, 2));
		_fecpSprites.draw(screen, _vm->_party->_electricityResistence ? 3 : 2,
			Common::Point(219, 2));
		_fecpSprites.draw(screen, _vm->_party->_coldResistence ? 5 : 4,
			Common::Point(2, 134));
		_fecpSprites.draw(screen, _vm->_party->_poisonResistence ? 7 : 6,
			Common::Point(219, 134));
	} else {
		_fecpSprites.draw(screen, _vm->_party->_fireResistence ? 9 : 8,
			Common::Point(8, 8));
		_fecpSprites.draw(screen, _vm->_party->_electricityResistence ? 10 : 11,
			Common::Point(219, 8));
		_fecpSprites.draw(screen, _vm->_party->_coldResistence ? 12 : 13,
			Common::Point(8, 134));
		_fecpSprites.draw(screen, _vm->_party->_poisonResistence ? 14 : 15,
			Common::Point(219, 134));
	}

	// Draw UI element for blessed
	_blessSprites.draw(screen, 16, Common::Point(33, 137));
	if (_vm->_party->_blessedActive) {
		_blessedUIFrame = (_blessedUIFrame + 1) % 4;
		_blessSprites.draw(screen, _blessedUIFrame, Common::Point(33, 137));
	}

	// Draw UI element for power shield
	if (_vm->_party->_powerShieldActive) {
		_powerShieldUIFrame = (_powerShieldUIFrame + 1) % 4;
		_blessSprites.draw(screen, _powerShieldUIFrame + 4,
			Common::Point(55, 137));
	}

	// Draw UI element for holy bonus
	if (_vm->_party->_holyBonusActive) {
		_holyBonusUIFrame = (_holyBonusUIFrame + 1) % 4;
		_blessSprites.draw(screen, _holyBonusUIFrame + 8, Common::Point(160, 137));
	}

	// Draw UI element for heroism
	if (_vm->_party->_heroismActive) {
		_heroismUIFrame = (_heroismUIFrame + 1) % 4;
		_blessSprites.draw(screen, _heroismUIFrame + 12, Common::Point(182, 137));
	}

	// Draw direction character if direction sense is active
	if (_vm->_party->checkSkill(DIRECTION_SENSE) && !_vm->_noDirectionSense) {
		const char *dirText = DIRECTION_TEXT[_vm->_party->_mazeDirection];
		Common::String msg = Common::String::format(
			"\002""08\003""c\013""139\011""116%c\014""d\001", *dirText);
		screen._windows[0].writeString(msg);
	}

	// Draw view frame
	if (screen._windows[12]._enabled)
		screen._windows[12].frame();
}

void InterfaceMap::drawMiniMap() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Window &window1 = screen._windows[1];

	if (screen._windows[2]._enabled || screen._windows[10]._enabled)
		return;
	if (!party._automapOn && !party._wizardEyeActive) {
		// Draw the Might & Magic logo
		_globalSprites.draw(window1, 5, Common::Point(232, 9));
		return;
	}

	int v, frame;
	int frame2 = _overallFrame * 2;
	bool eyeActive = party._wizardEyeActive;
	if (party._automapOn)
		party._wizardEyeActive = false;

	if (map._isOutdoors) {
		_globalSprites.draw(window1, 15, Common::Point(237, 12));

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);
				frame = map.mazeDataCurrent()._surfaceTypes[v];

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}
			}
		}

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);
				frame = map.mazeData()._wallTypes[v];

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame + 16, Common::Point(xp, yp));
				}
			}
		}

		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					4);

				if (v != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, v + 32, Common::Point(xp, yp));
				}
			}
		}
		
		// Draw the direction arrow
		_globalSprites.draw(window1, party._mazeDirection + 1,
			Common::Point(267, 36));
	} else {
		frame2 = (frame2 + 2) % 8;

		// First draw the default surface bases for each cell to show
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);

				if (v != INVALID_CELL && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, 0, Common::Point(xp, yp));
				}
			}
		}
		
		// Draw correct surface bases for revealed tiles
		for (int rowNum = 0, yp = 17, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 242, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);
				int surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];

				if (v != INVALID_CELL && map._currentSurfaceId &&
						(map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, surfaceId + 36, Common::Point(xp, yp));
				}
			}
		}
		
		v = map.mazeLookup(Common::Point(party._mazePosition.x - 4, party._mazePosition.y + 4), 0xffff, 0);
		if (v != INVALID_CELL && map._currentSurfaceId &&
				(map._currentSteppedOn || party._wizardEyeActive)) {
			map._tileSprites.draw(window1,
				map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
				Common::Point(232, 9));
		}
		
		// Handle drawing surface sprites partially clipped at the left edge
		for (int rowNum = 0, yp = 17, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, --yDiff, yp += 8) {
			v = map.mazeLookup(
					Common::Point(party._mazePosition.x - 4, party._mazePosition.y + yDiff),
					0, 0xffff);

			if (v != INVALID_CELL && map._currentSurfaceId &&
					(map._currentSteppedOn || party._wizardEyeActive)) {
				map._tileSprites.draw(window1,
					map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
					Common::Point(232, yp));
			}
		}

		// Handle drawing surface sprites partially clipped at the top edge
		for (int colNum = 0, xp = 242, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, ++xDiff, xp += 8) {
			v = map.mazeLookup(
				Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + 4),
				0, 0xffff);

			if (v != INVALID_CELL && map._currentSurfaceId &&
					(map._currentSteppedOn || party._wizardEyeActive)) {
				map._tileSprites.draw(window1,
					map.mazeData()._surfaceTypes[map._currentSurfaceId] + 36,
					Common::Point(xp, 9));
			}
		}
		
		//
		for (int idx = 0, xp = 237, yp = 60, xDiff = -3; idx < MINIMAP_SIZE; 
				++idx, ++xDiff, xp += 10, yp -= 8) {
			v = map.mazeLookup(
				Common::Point(party._mazePosition.x - 4, party._mazePosition.y - 3 + idx),
				12, 0xffff);

			switch (v) {
			case 1:
				frame = 18;
				break;
			case 3:
				frame = 22;
				break;
			case 4:
			case 13:
				frame = 16;
				break;
			case 5:
			case 8:
				frame = 2;
				break;
			case 6:
				frame = 30;
				break;
			case 7:
				frame = 32;
				break;
			case 9:
				frame = 24;
				break;
			case 10:
				frame = 28;
				break;
			case 11:
				frame = 14;
				break;
			case 12:
				frame = frame2 + 4;
				break;
			case 14:
				frame = 24;
				break;
			case 15:
				frame = 26;
				break;
			default:
				frame = -1;
				break;
			}

			if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive))
				map._tileSprites.draw(window1, frame, Common::Point(222, yp));

			v = map.mazeLookup(
				Common::Point(party._mazePosition.x - 3 + idx, party._mazePosition.y + 4),
				0);

			switch (v) {
			case 1:
				frame = 19;
				break;
			case 2:
				frame = 35;
				break;
			case 3:
				frame = 23;
				break;
			case 4:
			case 13:
				frame = 17;
				break;
			case 5:
			case 8:
				frame = 3;
				break;
			case 6:
				frame = 31;
				break;
			case 7:
				frame = 33;
				break;
			case 9:
				frame = 21;
				break;
			case 10:
				frame = 29;
				break;
			case 11:
				frame = 15;
				break;
			case 12:
				frame = frame2 + 5;
				break;
			case 14:
				frame = 25;
				break;
			case 15:
				frame = 27;
				break;
			default:
				frame = -1;
				break;
			}

			if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive))
				map._tileSprites.draw(window1, frame, Common::Point(xp, 4));
		}

		// Draw the front/back walls of cells in the minimap
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE;
				++rowNum, --yDiff, yp += 8) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE;
					++colNum, ++xDiff, xp += 10) {
				if (colNum == 4 && rowNum == 4) {
					// Center of the minimap. Draw the direction arrow
					_globalSprites.draw(window1, party._mazeDirection + 1,
						Common::Point(272, 40));
				}

				v = map.mazeLookup(Common::Point(party._mazePosition.x + xDiff,
					party._mazePosition.y + yDiff), 12, 0xffff);
				switch (v) {
				case 1:
					frame = 18;
					break;
				case 3:
					frame = 22;
					break;
				case 4:
				case 13:
					frame = 16;
					break;
				case 5:
				case 8:
					frame = 2;
					break;
				case 6:
					frame = 30;
					break;
				case 7:
					frame = 32;
					break;
				case 9:
					frame = 20;
					break;
				case 10:
					frame = 28;
					break;
				case 11:
					frame = 14;
					break;
				case 12:
					frame = frame2 + 4;
					break;
				case 14:
					frame = 24;
					break;
				case 15:
					frame = 26;
					break;
				default:
					frame = -1;
					break;
				}

				if (frame != -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}

				v = map.mazeLookup(Common::Point(party._mazePosition.x + xDiff,
					party._mazePosition.y + yDiff), 12, 0xffff);
				switch (v) {
				case 1:
					frame = 19;
					break;
				case 2:
					frame = 35;
					break;
				case 3:
					frame = 23;
					break;
				case 4:
				case 13:
					frame = 17;
					break;
				case 5:
				case 8:
					frame = 3;
					break;
				case 6:
					frame = 31;
					break;
				case 7:
					frame = 33;
					break;
				case 9:
					frame = 21;
					break;
				case 10:
					frame = 29;
					break;
				case 11:
					frame = 15;
					break;
				case 12:
					frame = frame2 + 5;
					break;
				case 14:
					frame = 25;
					break;
				case 15:
					frame = 27;
					break;
				default:
					frame = -1;
					break;
				}

				if (v == -1 && (map._currentSteppedOn || party._wizardEyeActive)) {
					map._tileSprites.draw(window1, frame, Common::Point(xp, yp));
				}
			}
		}

		// Draw the top of blocked/wall cells on the map
		for (int rowNum = 0, yp = 12, yDiff = 3; rowNum < MINIMAP_SIZE; ++rowNum, yp += 8, --yDiff) {
			for (int colNum = 0, xp = 237, xDiff = -3; colNum < MINIMAP_SIZE; ++colNum, xp += 10, ++xDiff) {
				v = map.mazeLookup(
					Common::Point(party._mazePosition.x + xDiff, party._mazePosition.y + yDiff),
					0, 0xffff);

				if (v == INVALID_CELL || (!map._currentSteppedOn && !party._wizardEyeActive)) {
					map._tileSprites.draw(window1, 1, Common::Point(xp, yp));
				}
			}
		}
	}

	// Draw outer rectangle around the automap
	_globalSprites.draw(window1, 6, Common::Point(223, 3));
	party._wizardEyeActive = eyeActive;
}

void InterfaceMap::drawTownAnim(int v) {
	warning("TODO");
}

} // End of namespace Xeen
