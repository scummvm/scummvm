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
	_sky(_data[1]), _ground(_data[2]), _horizon(_data[28]),
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
	_objects0(_data[149]), _objects1(_data[125]), _objects2(_data[126]),
	_objects3(_data[127]), _objects4(_data[97]), _objects5(_data[98]),
	_objects6(_data[99]), _objects7(_data[55]), _objects8(_data[56]),
	_objects9(_data[58]), _objects10(_data[57]), _objects11(_data[59]) {
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
	_flipWtr = false;
	_flipWall = false;
	_flipSky = false;
	_flipGround = false;
	_isShooting = false;
	_charsShooting = false;
	_objNumber = 0;
	_combatFloatCounter = 0;
}

void InterfaceMap::setIndoorsMonsters() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	Common::Point mazePos = _vm->_party._mazePosition;
	Direction dir = _vm->_party._mazeDirection;
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
	Common::Point mazePos = _vm->_party._mazePosition;
	Direction dir = _vm->_party._mazeDirection;
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
			} else if (!_wo[14] && !_wo[10] && !_indoorList._objects11._frame == -1) {
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
	const Common::Point &mazePos = _vm->_party._mazePosition;
	Direction dir = _vm->_party._mazeDirection;

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

	for (int cellIndex = 0; cellIndex < 25; ++cellIndex) {
		map.getCell(DRAW_NUMBERS[cellIndex]);

		DrawStruct &drawStruct = _indoorList[3 + cellIndex];
		if (!map._surfaceSprites[map._currentSurfaceId].empty())
			drawStruct._sprites = &map._surfaceSprites[map._currentSurfaceId];

		surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];
		if (surfaceId == 0 || surfaceId == 5 || surfaceId == 12) {
			drawStruct._flags = _flipWtr ? 1 : 0;
			drawStruct._frame = DRAW_FRAMES[cellIndex][_flipWtr ? 1 : 0];
		} else {
			drawStruct._frame = DRAW_FRAMES[cellIndex][_flipWall ? 1 : 0];
			drawStruct._flags = _flipWall ? SPRFLAG_HORIZ_FLIPPED : 0;
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

	map.cellFlagLookup(_vm->_party._mazePosition);

	// WORKAROUND: Original did an array lookup on _skySprites.
	// Was this a feature for multiple skys that was abandoned?
	assert(!map._currentSky);
	_indoorList[0]._sprites = &map._skySprites;
	_indoorList[0]._flags = _flipSky ? SPRFLAG_HORIZ_FLIPPED : 0;

	if (_vm->_openDoor) {
		Common::Point pt(
			_vm->_party._mazePosition.x + SCREEN_POSITIONING_X[
				_vm->_party._mazeDirection][_vm->_party._mazePosition.x],
			_vm->_party._mazePosition.y + SCREEN_POSITIONING_Y[
				_vm->_party._mazeDirection][_vm->_party._mazePosition.y]
			);
		map.cellFlagLookup(pt);
		
		_indoorList._sky._sprites = &map._skySprites;
	} else {
		_indoorList._sky._sprites = _indoorList[0]._sprites;
	}

	_indoorList._sky._flags = _flipSky ? SPRFLAG_HORIZ_FLIPPED : 0;
	_indoorList._ground._flags = _flipGround ? SPRFLAG_HORIZ_FLIPPED : 0;
	_indoorList._horizon._frame = 7;

	// Finally draw the darn indoor scene
	_vm->_screen->_windows[3].drawList(&_indoorList[0], _indoorList.size());

	// Check for any character shooting
	_isShooting = false;
	for (int i = 0; i < _vm->_party._partyCount; ++i) {
		if (_vm->_combat->_shooting[i])
			_isShooting = true;
	}
	
	_charsShooting = _isShooting;

	// TODO
}

} // End of namespace Xeen
