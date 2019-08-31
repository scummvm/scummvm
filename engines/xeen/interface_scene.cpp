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

#include "xeen/interface_scene.h"
#include "xeen/dialogs/dialogs_message.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

const int COMBAT_POS_X[3][2] = { { 102, 134 },{ 36, 67 },{ 161, 161 } };
const int INDOOR_POW_INDEXES[3] = { 157, 151, 154 };
const int OUTDOOR_POW_INDEXES[3] = { 119, 113, 116 };
const int COMBAT_OFFSET_X[4] = { 8, 6, 4, 2 };

OutdoorDrawList::OutdoorDrawList() : _sky1(_data[0]), _sky2(_data[1]),
	_groundSprite(_data[2]), _attackImgs1(&_data[124]), _attackImgs2(&_data[95]),
	_attackImgs3(&_data[76]), _attackImgs4(&_data[53]), _groundTiles(&_data[3]) {
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
	_data[28] = DrawStruct(1, -64, 61, 14);
	_data[29] = DrawStruct(1, -40, 61, 14, 0);
	_data[30] = DrawStruct(1, -16, 61, 14, 0);
	_data[31] = DrawStruct(1, 8, 61, 14, 0);
	_data[32] = DrawStruct(1, 128, 61, 14, SPRFLAG_HORIZ_FLIPPED);
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
	_data[88] = DrawStruct(0, -112, 30, 7);
	_data[89] = DrawStruct(0, 98, 30, 7);
	_data[90] = DrawStruct(0, -112, 30, 8);
	_data[91] = DrawStruct(0, 98, 30, 8);
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
	_data[106] = DrawStruct(0, -23, 40, 0);
	_data[107] = DrawStruct(0, 200, 40, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[108] = DrawStruct(0, 8, 47);
	_data[109] = DrawStruct(0, 169, 47, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[110] = DrawStruct(1, -56, -4, SCALE_ENLARGE, SPRFLAG_BOTTOM_CLIPPED);
	_data[111] = DrawStruct(0, -5, 2, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[112] = DrawStruct(0, -67, 2, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[113] = DrawStruct(0, 44, 73);
	_data[114] = DrawStruct(0, 44, 73);
	_data[115] = DrawStruct(0, 58, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[116] = DrawStruct(0, 169, 73);
	_data[117] = DrawStruct(0, 169, 73);
	_data[118] = DrawStruct(0, -5, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[119] = DrawStruct(0, 110, 73);
	_data[120] = DrawStruct(0, 110, 73);
	_data[121] = DrawStruct(0, -5, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
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

	for (int idx = 0; idx < 132; ++idx)
		_data[idx]._flags |= SPRFLAG_SCENE_CLIPPED;
}

void OutdoorDrawList::draw() {
	// Mark all items to be drawn as being clipped to the scene area
	for (int idx = 0; idx < size(); ++idx)
		_data[idx]._flags |= SPRFLAG_SCENE_CLIPPED;

	// Draw the list
	(*g_vm->_windows)[3].drawList(_data, size());
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
	_attackImgs1(&_data[162]), _attackImgs2(&_data[135]),
	_attackImgs3(&_data[111]), _attackImgs4(&_data[79]) {
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
	_data[87] = DrawStruct(7, -24, 52, 0);
	_data[88] = DrawStruct(7, 32, 52);
	_data[89] = DrawStruct(7, 88, 52);
	_data[90] = DrawStruct(0, 144, 52);
	_data[91] = DrawStruct(0, 200, 52, 0);
	_data[92] = DrawStruct(0, -79, 52, 11);
	_data[93] = DrawStruct(0, -27, 52, 11, 0);
	_data[94] = DrawStruct(0, 32, 52, 11, 0);
	_data[95] = DrawStruct(0, 89, 52, 11, 0);
	_data[96] = DrawStruct(0, 145, 52, 11);
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
	_data[119] = DrawStruct(6, -40, 40, 0);
	_data[120] = DrawStruct(6, 64, 40);
	_data[121] = DrawStruct(0, 168, 40, 0);
	_data[122] = DrawStruct(0, -72, 40, 6);
	_data[123] = DrawStruct(0, 32, 40, 6, 0);
	_data[124] = DrawStruct(0, 137, 40, 6);
	_data[125] = DrawStruct(0, -7, 25, 7, 0);
	_data[126] = DrawStruct(0, -112, 25, 7);
	_data[127] = DrawStruct(0, 98, 25, 7);
	_data[128] = DrawStruct(0, -112, 29, 8);
	_data[129] = DrawStruct(0, 98, 29, 8);
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
	_data[143] = DrawStruct(0, -136, 24, 0);
	_data[144] = DrawStruct(0, 8, 12);
	_data[145] = DrawStruct(0, 32, 24);
	_data[146] = DrawStruct(0, 200, 12, 0, SPRFLAG_HORIZ_FLIPPED);
	_data[147] = DrawStruct(0, 200, 24, 0);
	_data[148] = DrawStruct(0, 32, 24);
	_data[149] = DrawStruct(0, -5, 2, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[150] = DrawStruct(0, -67, 10, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[151] = DrawStruct(0, 44, 73);
	_data[152] = DrawStruct(0, 44, 73);
	_data[153] = DrawStruct(0, 58, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[154] = DrawStruct(0, 169, 73);
	_data[155] = DrawStruct(0, 169, 73);
	_data[156] = DrawStruct(0, -5, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
	_data[157] = DrawStruct(0, 110, 73);
	_data[158] = DrawStruct(0, 110, 73);
	_data[159] = DrawStruct(0, -5, 14, 0, SPRFLAG_BOTTOM_CLIPPED);
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

void IndoorDrawList::draw() {
	// Mark all items to be drawn as being clipped to the scene area
	for (int idx = 0; idx < size(); ++idx)
		_data[idx]._flags |= SPRFLAG_SCENE_CLIPPED;

	// Draw the list
	(*g_vm->_windows)[3].drawList(_data, size());
}

/*------------------------------------------------------------------------*/

InterfaceScene::InterfaceScene(XeenEngine *vm): _vm(vm) {
	Common::fill(&_wp[0], &_wp[20], 0);
	Common::fill(&_wo[0], &_wo[308], 0);
	_overallFrame = 0;
	_flipWater = false;
	_flipGround = false;
	_flipSky = false;
	_flipDefaultGround = false;
	_isAttacking = false;
	_charsShooting = false;
	_objNumber = -1;
	_combatFloatCounter = 0;
	_thinWall = false;
	_isAnimReset = false;
	_overallFrame = 0;
	_openDoor = false;
}

void InterfaceScene::drawScene() {
	Map &map = *_vm->_map;
	Scripts &scripts = *_vm->_scripts;

	MazeObject *obj = (_objNumber == -1) ? nullptr : &map._mobData._objects[_objNumber];
	Direction partyDirection = _vm->_party->_mazeDirection;

	// Loop to update the frame numbers for each maze object, applying the animation frame
	// limits as specified by the map's _animationInfo listing
	for (uint idx = 0; idx < map._mobData._objects.size(); ++idx) {
		MazeObject &mazeObject = map._mobData._objects[idx];
		if (mazeObject._spriteId == -1)
			continue;

		AnimationEntry &animEntry = map._animationInfo[mazeObject._spriteId];
		int directionIndex = Res.DIRECTION_ANIM_POSITIONS[mazeObject._direction][partyDirection];

		if (_isAnimReset) {
			mazeObject._frame = animEntry._frame1._frames[directionIndex];
		} else {
			++mazeObject._frame;
			if ((int)idx == _objNumber && scripts._animCounter > 0 && (
				obj->_spriteId == (_vm->_files->_ccNum ? 15 : 16) ||
				obj->_spriteId == 58 || obj->_spriteId == 73)) {
				if (mazeObject._frame > 4 || mazeObject._spriteId == 58)
					mazeObject._frame = 1;
			} else if (mazeObject._frame >= animEntry._frame2._frames[directionIndex]) {
				mazeObject._frame = animEntry._frame1._frames[directionIndex];
			}
		}

		mazeObject._flipped = animEntry._flipped._flags[directionIndex];
	}

	if (map._isOutdoors)
		drawOutdoorsScene();
	else
		drawIndoorsScene();

	animate3d();
}

void InterfaceScene::drawOutdoorsScene() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;

	for (int idx = 0; idx < 44; ++idx)
		_outdoorList[Res.OUTDOOR_DRAWSTRUCT_INDEXES[idx]]._frame = -1;

	if (combat._monstersAttacking) {
		for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
			if (_outdoorList._attackImgs4[idx]._sprites)
				_outdoorList._attackImgs4[idx]._frame = 0;
			else if (_outdoorList._attackImgs3[idx]._sprites)
				_outdoorList._attackImgs3[idx]._frame = 1;
			else if (_outdoorList._attackImgs2[idx]._sprites)
				_outdoorList._attackImgs2[idx]._frame = 2;
			else if (_outdoorList._attackImgs1[idx]._sprites)
				_outdoorList._attackImgs1[idx]._frame = 0;
		}
	} else if (_charsShooting) {
		for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
			if (_outdoorList._attackImgs1[idx]._sprites)
				_outdoorList._attackImgs1[idx]._frame = 0;
			else if (_outdoorList._attackImgs2[idx]._sprites)
				_outdoorList._attackImgs2[idx]._frame = 1;
			else if (_outdoorList._attackImgs3[idx]._sprites)
				_outdoorList._attackImgs3[idx]._frame = 2;
			else if (_outdoorList._attackImgs4[idx]._sprites)
				_outdoorList._attackImgs1[idx]._frame = 0;
		}
	}

	_isAnimReset = false;
	int attackMon2 = combat._attackMonsters[2];

	// Only the front rank of pow points result in a Pow splatter effect
	for (int idx = 0; idx < 3; ++idx) {
		DrawStruct &ds1 = _outdoorList[OUTDOOR_POW_INDEXES[idx] + 1];
		DrawStruct &ds2 = _outdoorList[OUTDOOR_POW_INDEXES[idx]];
		ds1._sprites = nullptr;
		ds2._sprites = nullptr;

		if (combat._pow[idx]._duration) {
			int vIndex = combat._attackMonsters[1] && !attackMon2 ? 1 : 0;
			combat._pow[idx]._duration--;

			if (combat._pow[idx]._active) {
				ds2._x = COMBAT_POS_X[idx][vIndex];
				ds2._frame = 0;
				ds2._scale = combat._pow[idx]._scale;

				if (ds2._scale == SCALE_ENLARGE) {
					ds2._x /= 3;
					ds2._y = 60;
				} else {
					ds2._y = 73;
				}

				ds2._flags = SPRFLAG_BOTTOM_CLIPPED | SPRFLAG_SCENE_CLIPPED;
				ds2._sprites = &_charPowSprites;
			}

			if (combat._pow[idx]._elemFrame) {
				ds1._x = COMBAT_POS_X[idx][vIndex] + COMBAT_OFFSET_X[idx];
				ds1._frame = combat._pow[idx]._elemFrame;
				ds1._scale = combat._pow[idx]._elemScale;

				if (ds1._scale == SCALE_ENLARGE)
					ds1._x /= 3;
				ds1._flags = SPRFLAG_BOTTOM_CLIPPED | SPRFLAG_SCENE_CLIPPED;
				ds1._sprites = &_charPowSprites;
			}
		}
	}

	setOutdoorsMonsters();
	setOutdoorsObjects();

	_outdoorList[123]._sprites = nullptr;
	_outdoorList[122]._sprites = nullptr;
	_outdoorList[121]._sprites = nullptr;

	int monsterIndex;
	if (combat._attackMonsters[0] != -1 && map._mobData._monsters[combat._attackMonsters[0]]._frame >= 8) {
		_outdoorList[121] = _outdoorList[118];
		_outdoorList[122] = _outdoorList[119];
		_outdoorList[123] = _outdoorList[120];
		_outdoorList[118]._sprites = nullptr;
		_outdoorList[119]._sprites = nullptr;
		_outdoorList[120]._sprites = nullptr;
		monsterIndex = 1;
	} else if (combat._attackMonsters[1] != -1 && map._mobData._monsters[combat._attackMonsters[1]]._frame >= 8) {
		_outdoorList[121] = _outdoorList[112];
		_outdoorList[122] = _outdoorList[113];
		_outdoorList[123] = _outdoorList[114];
		_outdoorList[112]._sprites = nullptr;
		_outdoorList[113]._sprites = nullptr;
		_outdoorList[114]._sprites = nullptr;
		monsterIndex = 2;
	} else if (combat._attackMonsters[2] != -1 && map._mobData._monsters[combat._attackMonsters[2]]._frame >= 8) {
		_outdoorList[121] = _outdoorList[115];
		_outdoorList[122] = _outdoorList[116];
		_outdoorList[123] = _outdoorList[117];
		_outdoorList[115]._sprites = nullptr;
		_outdoorList[116]._sprites = nullptr;
		_outdoorList[117]._sprites = nullptr;
		monsterIndex = 3;
	} else {
		monsterIndex = 0;
	}

	drawOutdoors();

	switch (monsterIndex) {
	case 1:
		_outdoorList[118] = _outdoorList[121];
		_outdoorList[119] = _outdoorList[122];
		_outdoorList[120] = _outdoorList[123];
		break;
	case 2:
		_outdoorList[112] = _outdoorList[121];
		_outdoorList[113] = _outdoorList[122];
		_outdoorList[114] = _outdoorList[123];
		break;
	case 3:
		_outdoorList[115] = _outdoorList[121];
		_outdoorList[116] = _outdoorList[122];
		_outdoorList[117] = _outdoorList[123];
		break;
	default:
		break;
	}
}

void InterfaceScene::drawIndoorsScene() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;

	// Default all the parts of draw struct not to be drawn by default
	for (int idx = 3; idx < _indoorList.size(); ++idx)
		_indoorList[idx]._frame = -1;

	if (combat._monstersAttacking) {
		for (int idx = 0; idx < 8; ++idx) {
			if (_indoorList[79 + idx]._sprites != nullptr) {
				_indoorList[79 + idx]._frame = 0;
			} else if (_indoorList[111 + idx]._sprites != nullptr) {
				_indoorList[111 + idx]._frame = 1;
			} else if (_indoorList[135 + idx]._sprites != nullptr) {
				_indoorList[135 + idx]._frame = 2;
			} else if (_indoorList[162 + idx]._sprites != nullptr) {
				_indoorList[162 + idx]._frame = 0;
			}
		}
	} else if (_charsShooting) {
		for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
			if (_indoorList._attackImgs1[idx]._sprites != nullptr) {
				_indoorList._attackImgs1[idx]._frame = 0;
			} else if (_indoorList._attackImgs2[idx]._sprites != nullptr) {
				_indoorList._attackImgs2[idx]._frame = 1;
			} else if (_indoorList._attackImgs3[idx]._sprites != nullptr) {
				_indoorList._attackImgs3[idx]._frame = 2;
			} else if (_indoorList._attackImgs4[idx]._sprites != nullptr) {
				_indoorList._attackImgs4[idx]._frame = 0;
			}
		}
	}

	setMazeBits();
	_isAnimReset = false;

	// Code in the original that's not being used
	//MazeObject &objObject = map._mobData._objects[_objNumber];

	// Only the front rank of pow points result in a Pow splatter effect
	for (int idx = 0; idx < 3; ++idx) {
		DrawStruct &ds1 = _indoorList[INDOOR_POW_INDEXES[idx]];
		DrawStruct &ds2 = _indoorList[INDOOR_POW_INDEXES[idx] + 1];
		ds1._sprites = nullptr;
		ds2._sprites = nullptr;

		if (combat._pow[idx]._duration) {
			int posIndex = combat._attackMonsters[1] && !combat._attackMonsters[2] ? 1 : 0;
			--combat._pow[idx]._duration;

			if (combat._pow[idx]._active) {
				ds1._x = COMBAT_POS_X[idx][posIndex];
				ds1._frame = 0;
				ds1._scale = combat._pow[idx]._scale;
				if (ds1._scale == SCALE_ENLARGE) {
					ds1._x /= 3;
					ds1._y = 60;
				} else {
					ds1._y = 73;
				}

				ds1._flags = SPRFLAG_BOTTOM_CLIPPED | SPRFLAG_SCENE_CLIPPED;
				ds1._sprites = &_charPowSprites;
			}

			if (combat._pow[idx]._elemFrame) {
				ds2._x = COMBAT_POS_X[idx][posIndex] + COMBAT_OFFSET_X[idx];
				ds2._frame = combat._pow[idx]._elemFrame;
				ds2._scale = combat._pow[idx]._elemScale;
				if (ds2._scale == SCALE_ENLARGE)
					ds2._x /= 3;
				ds2._flags = SPRFLAG_BOTTOM_CLIPPED | SPRFLAG_SCENE_CLIPPED;
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
	if (combat._attackMonsters[0] != -1 && map._mobData._monsters[combat._attackMonsters[0]]._frame >= 8) {
		_indoorList[159] = _indoorList[156];
		_indoorList[160] = _indoorList[157];
		_indoorList[161] = _indoorList[158];
		_indoorList[158]._sprites = nullptr;
		_indoorList[156]._sprites = nullptr;
		_indoorList[157]._sprites = nullptr;
		monsterIndex = 1;
	} else if (combat._attackMonsters[1] != -1 && map._mobData._monsters[combat._attackMonsters[1]]._frame >= 8) {
		_indoorList[159] = _indoorList[150];
		_indoorList[160] = _indoorList[151];
		_indoorList[161] = _indoorList[152];
		_indoorList[152]._sprites = nullptr;
		_indoorList[151]._sprites = nullptr;
		_indoorList[150]._sprites = nullptr;
		monsterIndex = 2;
	} else if (combat._attackMonsters[2] != -1 && map._mobData._monsters[combat._attackMonsters[2]]._frame >= 8) {
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

void InterfaceScene::animate3d() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	_overallFrame = (_overallFrame + 1) % 5;
	_combatFloatCounter = (_combatFloatCounter + 1) % 8;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];
		if (monster._damageType == DT_PHYSICAL) {
			if (monster._frame < 8) {
				MonsterStruct &monsterData = *monster._monsterData;
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
				--monster._postAttackDelay;
				if (monster._postAttackDelay == 0)
					monster._frame = 0;
			} else {
				++monster._frame;
				if (monster._frame == 11) {
					--monster._postAttackDelay;
					monster._frame = monster._postAttackDelay ? 10 : 0;
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
				MonsterStruct &monsterData = *monster._monsterData;
				monster._effect1 = monster._effect2 = monsterData._animationEffect;
			}
		}
	}

	DrawStruct *rangedImgs1 = map._isOutdoors ? _outdoorList._attackImgs1 : _indoorList._attackImgs1;
	DrawStruct *rangedImgs2 = map._isOutdoors ? _outdoorList._attackImgs2 : _indoorList._attackImgs2;
	DrawStruct *rangedImgs3 = map._isOutdoors ? _outdoorList._attackImgs3 : _indoorList._attackImgs3;
	DrawStruct *rangedImgs4 = map._isOutdoors ? _outdoorList._attackImgs4 : _indoorList._attackImgs4;

	if (combat._monstersAttacking) {
		// Monsters doing ranged attacks. Sequentially move the attack from
		// whichever row it started in to the front (where the party is)
		for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
			if (rangedImgs1[idx]._sprites) {
				rangedImgs1[idx]._sprites = nullptr;
				combat._shootingRow[idx] = 0;
			} else if (rangedImgs2[idx]._sprites) {
				rangedImgs1[idx]._sprites = rangedImgs2[idx]._sprites;
				rangedImgs2[idx]._sprites = nullptr;
			} else if (rangedImgs3[idx]._sprites) {
				rangedImgs2[idx]._sprites = rangedImgs3[idx]._sprites;
				rangedImgs3[idx]._sprites = nullptr;
			} else if (rangedImgs4[idx]._sprites) {
				rangedImgs3[idx]._sprites = rangedImgs4[idx]._sprites;
				rangedImgs4[idx]._sprites = nullptr;
			}
		}
	} else if (_charsShooting) {
		// Characters shooting at monsters. Sequentially move the attack
		// away from the party
		for (int idx = 0; idx < MAX_PARTY_COUNT; ++idx) {
			if (rangedImgs4[idx]._sprites) {
				rangedImgs4[idx]._sprites = nullptr;
			} else if (rangedImgs3[idx]._sprites) {
				rangedImgs4[idx]._sprites = rangedImgs3[idx]._sprites;
				rangedImgs3[idx]._sprites = nullptr;
			} else if (rangedImgs2[idx]._sprites) {
				rangedImgs3[idx]._sprites = rangedImgs2[idx]._sprites;
				rangedImgs2[idx]._sprites = nullptr;
			} else if (rangedImgs1[idx]._sprites) {
				rangedImgs2[idx]._sprites = rangedImgs1[idx]._sprites;
				rangedImgs1[idx]._sprites = nullptr;
			}
		}
	}

	for (uint idx = 0; idx < map._mobData._wallItems.size(); ++idx) {
		MazeWallItem &wallItem = map._mobData._wallItems[idx];
		wallItem._frame = (wallItem._frame + 1) % wallItem._sprites->size();
	}
}

void InterfaceScene::setMazeBits() {
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
		break;
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

	_thinWall = (_vm->_map->_currentWall != INVALID_CELL) && _wo[27];

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
		++_wo[303];
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
		break;
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
		break;
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

void InterfaceScene::setIndoorsMonsters() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	Common::Point mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;

	// Reset the list of attacking monsters
	combat.clearAttackers();

	// Iterate through the monsters list checking for proximity to party
	for (uint monsterIdx = 0; monsterIdx < map._mobData._monsters.size(); ++monsterIdx) {
		MazeMonster &monster = map._mobData._monsters[monsterIdx];
		SpriteResource *sprites = monster._sprites;
		int frame = monster._frame;

		if (frame >= 8) {
			sprites = monster._attackSprites;
			frame -= 8;
		}

		// The following long sequence sets up monsters in the various positions
		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][2]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][2])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[0] == -1) {
				combat._attackMonsters[0] = monsterIdx;
				setMonsterSprite(_indoorList[156], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[0]);
			} else if (combat._attackMonsters[1] == -1) {
				combat._attackMonsters[1] = monsterIdx;
				setMonsterSprite(_indoorList[150], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[0]);
			} else if (combat._attackMonsters[2] == -1) {
				combat._attackMonsters[2] = monsterIdx;
				setMonsterSprite(_indoorList[153], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[0]);
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][7]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][7])) {
			monster._isAttacking = true;
			if (!_wo[27]) {
				if (combat._attackMonsters[3] == -1) {
					combat._attackMonsters[3] = monsterIdx;
					setMonsterSprite(_indoorList[132], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[1]);
				} else if (combat._attackMonsters[4] == -1) {
					combat._attackMonsters[4] = monsterIdx;
					setMonsterSprite(_indoorList[130], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[1]);
				} else if (combat._attackMonsters[2] == -1) {
					combat._attackMonsters[5] = monsterIdx;
					setMonsterSprite(_indoorList[131], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][5]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][5])) {
			if (_wo[27] && _wo[25]) {
			} else if (_wo[27] && _wo[28]) {
			} else if (_wo[23] & _wo[25]) {
			} else if (_wo[23] && _wo[28]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[12] == -1) {
					combat._attackMonsters[12] = monsterIdx;
					setMonsterSprite(_indoorList[128], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][9]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][9])) {
			if (_wo[27] && _wo[26]) {
			} else if (_wo[27] && _wo[29]) {
			} else if (_wo[24] & _wo[26]) {
			} else if (_wo[24] && _wo[29]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[13] == -1) {
					combat._attackMonsters[13] = monsterIdx;
					setMonsterSprite(_indoorList[129], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[1]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][14]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][14])) {
			monster._isAttacking = true;

			if (!_wo[22] && !_wo[27]) {
				if (combat._attackMonsters[6] == -1) {
					combat._attackMonsters[6] = monsterIdx;
					setMonsterSprite(_indoorList[106], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				} else  if (combat._attackMonsters[7] == -1) {
					combat._attackMonsters[7] = monsterIdx;
					setMonsterSprite(_indoorList[104], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[8] == -1) {
					combat._attackMonsters[8] = monsterIdx;
					setMonsterSprite(_indoorList[105], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][12]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][12])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[22] & _wo[20]) {
			} else if (_wo[23] && _wo[17]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[14] == -1) {
					combat._attackMonsters[14] = monsterIdx;
					setMonsterSprite(_indoorList[100], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[20] == -1) {
					combat._attackMonsters[20] = monsterIdx;
					setMonsterSprite(_indoorList[101], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][16]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][16])) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[22] & _wo[21]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[21] && _wo[19]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[15] == -1) {
					combat._attackMonsters[15] = monsterIdx;
					setMonsterSprite(_indoorList[102], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				} else if (combat._attackMonsters[21] == -1) {
					combat._attackMonsters[21] = monsterIdx;
					setMonsterSprite(_indoorList[103], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[2]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][27]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][27])) {
			if (!_wo[27] && !_wo[22] && !_wo[15]) {
				monster._isAttacking = true;

				if (combat._attackMonsters[9] == -1) {
					combat._attackMonsters[9] = monsterIdx;
					setMonsterSprite(_indoorList[70], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[10] == -1) {
					combat._attackMonsters[10] = monsterIdx;
					setMonsterSprite(_indoorList[68], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[11] == -1) {
					combat._attackMonsters[11] = monsterIdx;
					setMonsterSprite(_indoorList[69], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][25]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][25])) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[17]) {
			} else if (_wo[15] && _wo[12]) {
			} else if (_wo[12] && _wo[7]) {
			} else if (_wo[17] && _wo[7]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[16] == -1) {
					combat._attackMonsters[16] = monsterIdx;
					setMonsterSprite(_indoorList[62], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[22] == -1) {
					combat._attackMonsters[22] = monsterIdx;
					setMonsterSprite(_indoorList[60], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[24] == -1) {
					combat._attackMonsters[24] = monsterIdx;
					setMonsterSprite(_indoorList[61], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][23])) {
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
					setMonsterSprite(_indoorList[66], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][29]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][29])) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[19]) {
			} else if (_wo[15] && _wo[14]) {
			} else if (_wo[14] && _wo[9]) {
			} else if (_wo[19] && _wo[9]) {
			} else {
				monster._isAttacking = true;

				if (combat._attackMonsters[17] == -1) {
					combat._attackMonsters[17] = monsterIdx;
					setMonsterSprite(_indoorList[65], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[23] == -1) {
					combat._attackMonsters[23] = monsterIdx;
					setMonsterSprite(_indoorList[63], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[25] == -1) {
					combat._attackMonsters[25] = monsterIdx;
					setMonsterSprite(_indoorList[64], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				}
			}
		}

		if (monster._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				monster._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][31])) {
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
					setMonsterSprite(_indoorList[67], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[23] == -1) {
					combat._attackMonsters[23] = monsterIdx;
					setMonsterSprite(_indoorList[63], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				} else if (combat._attackMonsters[25] == -1) {
					combat._attackMonsters[25] = monsterIdx;
					setMonsterSprite(_indoorList[64], monster, sprites, frame, Res.INDOOR_MONSTERS_Y[3]);
				}
			}
		}
	}

	_indoorList[153]._x += 58;
	_indoorList[131]._x += 25;
	_indoorList[105]._x += 9;
	_indoorList[69]._x--;
	_indoorList[61]._x -= 26;
	_indoorList[64]._x += 23;
	_indoorList[66]._x -= 58;
	_indoorList[67]._x += 40;
	_indoorList[100]._x -= 65;
	_indoorList[101]._x -= 85;
	_indoorList[102]._x += 49;
	_indoorList[103]._x += 65;
	_indoorList[128]._x -= 112;
	_indoorList[129]._x += 98;

	if (combat._attackMonsters[1] != -1 && combat._attackMonsters[2] == -1) {
		_indoorList[156]._x += 31;
		_indoorList[150]._x -= 36;
	} else {
		_indoorList[156]._x -= 5;
		_indoorList[150]._x -= 67;
	}

	if (combat._attackMonsters[4] != -1 && combat._attackMonsters[5] == -1) {
		_indoorList[132]._x += 8;
		_indoorList[130]._x -= 23;
	} else {
		_indoorList[132]._x -= 7;
		_indoorList[130]._x -= 38;
	}

	if (combat._attackMonsters[7] != -1 && combat._attackMonsters[8] == -1) {
		_indoorList[104]._x -= 16;
	} else {
		_indoorList[106]._x -= 8;
		_indoorList[104]._x -= 24;
	}

	if (combat._attackMonsters[10] != -1 && combat._attackMonsters[11] == -1) {
		_indoorList[70]._x -= 5;
		_indoorList[68]._x -= 13;
	} else {
		_indoorList[70]._x -= 9;
		_indoorList[68]._x -= 17;
	}

	if (combat._attackMonsters[22] == -1 && combat._attackMonsters[24] == -1) {
		_indoorList[62]._x -= 27;
		_indoorList[60]._x -= 37;
	} else {
		_indoorList[62]._x -= 34;
		_indoorList[60]._x -= 41;
	}

	if (combat._attackMonsters[23] != -1 && combat._attackMonsters[25] == -1) {
		_indoorList[65]._x += 20;
		_indoorList[63]._x -= 12;
	} else {
		_indoorList[65]._x += 16;
		_indoorList[63]._x -= 16;
	}
}

void InterfaceScene::setMonsterSprite(DrawStruct &drawStruct, MazeMonster &monster, SpriteResource *sprites,
		int frame, int defaultY) {
	MonsterStruct &monsterData = *monster._monsterData;
	bool flying = monsterData._flying;

	drawStruct._frame = frame;
	drawStruct._sprites = sprites;
	drawStruct._y = defaultY;

	if (flying) {
		drawStruct._x = Res.COMBAT_FLOAT_X[_combatFloatCounter];
		drawStruct._y += Res.COMBAT_FLOAT_Y[_combatFloatCounter];
	} else {
		drawStruct._x = 0;
	}

	drawStruct._flags &= ~0xFFF;
	if (monster._effect2)
		drawStruct._flags = Res.MONSTER_EFFECT_FLAGS[monster._effect2 - 1][monster._effect3];
}

void InterfaceScene::setIndoorsObjects() {
	Common::Point mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;
	Common::Point pt;
	_objNumber = -1;

	Common::Array<MazeObject> &objects = _vm->_map->_mobData._objects;
	for (uint idx = 0; idx < objects.size(); ++idx) {
		MazeObject &mazeObject = objects[idx];

		// Determine which half of the X/Y lists to use
		int listOffset;
		if (_vm->_files->_ccNum) {
			listOffset = mazeObject._spriteId == 47 ? 1 : 0;
		} else {
			listOffset = mazeObject._spriteId == 113 ? 1 : 0;
		}

		// Position 1
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][2]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][2]) == mazeObject._position.y
			&& _indoorList._objects0._frame == -1) {
			_indoorList._objects0._x = Res.INDOOR_OBJECT_X[listOffset][0];
			_indoorList._objects0._y = Res.MAP_OBJECT_Y[listOffset][0];
			_indoorList._objects0._frame = mazeObject._frame;
			_indoorList._objects0._sprites = mazeObject._sprites;
			_indoorList._objects0._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects0._flags |= SPRFLAG_HORIZ_FLIPPED;
			_objNumber = idx;
		}

		// Position 2
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][7]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][7]) == mazeObject._position.y
			&& !_wo[27] && _indoorList._objects1._frame == -1) {
			_indoorList._objects1._x = Res.INDOOR_OBJECT_X[listOffset][1];
			_indoorList._objects1._y = Res.MAP_OBJECT_Y[listOffset][1];
			_indoorList._objects1._frame = mazeObject._frame;
			_indoorList._objects1._sprites = mazeObject._sprites;
			_indoorList._objects1._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (mazeObject._flipped)
				_indoorList._objects1._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		// Position 3
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][5]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][5]) == mazeObject._position.y) {
			if (_wo[27] && _wo[25]) {
			} else if (_wo[27] && _wo[28]) {
			} else if (_wo[23] && _wo[25]) {
			} else if (_wo[23] && _wo[28]) {
			} else if (_indoorList._objects2._frame == -1) {
				_indoorList._objects2._x = Res.INDOOR_OBJECT_X[listOffset][2];
				_indoorList._objects2._y = Res.MAP_OBJECT_Y[listOffset][2];
				_indoorList._objects2._frame = mazeObject._frame;
				_indoorList._objects2._sprites = mazeObject._sprites;
				_indoorList._objects2._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects2._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 4
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][9]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][9]) == mazeObject._position.y) {
			if (_wo[27] && _wo[26]) {
			} else if (_wo[27] && _wo[29]) {
			} else if (_wo[24] && _wo[26]) {
			} else if (_wo[24] && _wo[29]) {
			} else if (_indoorList._objects3._frame == -1) {
				_indoorList._objects3._x = Res.INDOOR_OBJECT_X[listOffset][3];
				_indoorList._objects3._y = Res.MAP_OBJECT_Y[listOffset][3];
				_indoorList._objects3._frame = mazeObject._frame;
				_indoorList._objects3._sprites = mazeObject._sprites;
				_indoorList._objects3._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects3._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 5
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][14]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][14]) == mazeObject._position.y) {
			if (!_wo[22] && !_wo[27] && _indoorList._objects4._frame == -1) {
				_indoorList._objects4._x = Res.INDOOR_OBJECT_X[listOffset][4];
				_indoorList._objects4._y = Res.MAP_OBJECT_Y[listOffset][4];
				_indoorList._objects4._frame = mazeObject._frame;
				_indoorList._objects4._sprites = mazeObject._sprites;
				_indoorList._objects4._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects4._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 6
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][12]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][12]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_indoorList._objects5._frame == -1) {
				_indoorList._objects5._x = Res.INDOOR_OBJECT_X[listOffset][5];
				_indoorList._objects5._y = Res.MAP_OBJECT_Y[listOffset][5];
				_indoorList._objects5._frame = mazeObject._frame;
				_indoorList._objects5._sprites = mazeObject._sprites;
				_indoorList._objects5._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects5._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 7
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][16]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][16]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_indoorList._objects6._frame == -1) {
				_indoorList._objects6._x = Res.INDOOR_OBJECT_X[listOffset][6];
				_indoorList._objects6._y = Res.MAP_OBJECT_Y[listOffset][6];
				_indoorList._objects6._frame = mazeObject._frame;
				_indoorList._objects6._sprites = mazeObject._sprites;
				_indoorList._objects6._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects6._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 8
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][27]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][27]) == mazeObject._position.y) {
			if (!_wo[27] && !_wo[22] && !_wo[15] && _indoorList._objects7._frame == -1) {
				_indoorList._objects7._x = Res.INDOOR_OBJECT_X[listOffset][7];
				_indoorList._objects7._y = Res.MAP_OBJECT_Y[listOffset][7];
				_indoorList._objects7._frame = mazeObject._frame;
				_indoorList._objects7._sprites = mazeObject._sprites;
				_indoorList._objects7._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects7._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 9
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][25]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][25]) == mazeObject._position.y) {
			if (_wo[27] || _wo[22]) {
			} else if (_wo[15] && _wo[17]) {
			} else if (_wo[15] && _wo[12]) {
			} else if (_wo[12] && _wo[7]) {
			} else if (_wo[17] && _wo[7]) {
			} else if (_indoorList._objects8._frame == -1) {
				_indoorList._objects8._x = Res.INDOOR_OBJECT_X[listOffset][8];
				_indoorList._objects8._y = Res.MAP_OBJECT_Y[listOffset][8];
				_indoorList._objects8._frame = mazeObject._frame;
				_indoorList._objects8._sprites = mazeObject._sprites;
				_indoorList._objects8._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects8._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Position 10
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][23]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][23]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[20]) {
			} else if (_wo[22] && _wo[23]) {
			} else if (_wo[20] && _wo[17]) {
			} else if (_wo[23] && _wo[17]) {
			} else if (!_wo[12] && !_wo[8] && _indoorList._objects9._frame == -1) {
				_indoorList._objects9._x = Res.INDOOR_OBJECT_X[listOffset][10];
				_indoorList._objects9._y = Res.MAP_OBJECT_Y[listOffset][10];
				_indoorList._objects9._frame = mazeObject._frame;
				_indoorList._objects9._sprites = mazeObject._sprites;
				_indoorList._objects9._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects9._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Block 11
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][29]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][29]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[15] && _wo[19]) {
			} else if (_wo[15] && _wo[14]) {
			} else if (_wo[14] && _wo[9]) {
			} else if (_wo[19] && _wo[9]) {
			} else if (_indoorList._objects10._frame == -1) {
				_indoorList._objects10._x = Res.INDOOR_OBJECT_X[listOffset][9];
				_indoorList._objects10._y = Res.MAP_OBJECT_Y[listOffset][9];
				_indoorList._objects10._frame = mazeObject._frame;
				_indoorList._objects10._sprites = mazeObject._sprites;
				_indoorList._objects10._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects10._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}

		// Block 12
		if ((mazePos.x + Res.SCREEN_POSITIONING_X[dir][31]) == mazeObject._position.x
			&& (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][31]) == mazeObject._position.y) {
			if (_wo[27]) {
			} else if (_wo[22] && _wo[21]) {
			} else if (_wo[22] && _wo[24]) {
			} else if (_wo[21] && _wo[19]) {
			} else if (_wo[24] && _wo[19]) {
			} else if (!_wo[14] && !_wo[10] && _indoorList._objects11._frame == -1) {
				_indoorList._objects11._x = Res.INDOOR_OBJECT_X[listOffset][11];
				_indoorList._objects11._y = Res.MAP_OBJECT_Y[listOffset][11];
				_indoorList._objects11._frame = mazeObject._frame;
				_indoorList._objects11._sprites = mazeObject._sprites;
				_indoorList._objects11._flags &= ~SPRFLAG_HORIZ_FLIPPED;
				if (mazeObject._flipped)
					_indoorList._objects11._flags |= SPRFLAG_HORIZ_FLIPPED;
			}
		}
	}
}

void InterfaceScene::setIndoorsWallPics() {
	Map &map = *_vm->_map;
	const Common::Point &mazePos = _vm->_party->_mazePosition;
	Direction dir = _vm->_party->_mazeDirection;

	Common::fill(&_wp[0], &_wp[20], -1);

	for (uint idx = 0; idx < map._mobData._wallItems.size(); ++idx) {
		MazeWallItem &wallItem = map._mobData._wallItems[idx];
		if (wallItem._direction != dir)
			continue;

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][2]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][2])) {
			if (_wp[1] == -1) {
				_indoorList[148]._frame = wallItem._frame;
				_indoorList[148]._sprites = wallItem._sprites;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][7]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][7])) {
			if (!_wo[27] && _wp[1] == -1) {
				_indoorList[123]._frame = wallItem._frame;
				_indoorList[123]._sprites = wallItem._sprites;
				_wp[4] = idx;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][5]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][5])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][9]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][9])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][14]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][14])) {
			if (!_wo[22] && !_wo[27] && _wp[8] == -1) {
				_indoorList[94]._frame = wallItem._frame;
				_indoorList[94]._sprites = wallItem._sprites;
				_wp[8] = idx;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][12]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][12])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][16]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][16])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][12]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][12])) {
			if (_wo[27]) {
			} else if (_wo[25] && _wo[28]) {
			} else if (_wo[20] && _wo[16]) {
			} else if (_wp[6] == -1) {
				_indoorList[92]._frame = wallItem._frame;
				_indoorList[92]._sprites = wallItem._sprites;
				_wp[6] = idx;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][16]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][16])) {
			if (!_wo[26] && !_wo[29] && !_wo[21] && !_wo[18] && _wp[10] == -1) {
				_indoorList[96]._frame = wallItem._frame;
				_indoorList[96]._sprites = wallItem._sprites;
				_wp[10] = idx;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][27]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][27])) {
			if (!_wo[27] && !_wo[22] && !_wo[15] && _wp[15] == -1) {
				_indoorList[50]._frame = wallItem._frame;
				_indoorList[50]._sprites = wallItem._sprites;
				_wp[15] = idx;
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][25]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][25])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][23])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][29]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][29])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][31])) {
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

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][23])) {
			if (!_wo[27] && !_wo[20] && !_wo[12] && !_wo[23] && !_wo[8] && !_wo[30]) {
				if (_wp[12] == -1) {
					_indoorList[47]._frame = wallItem._frame;
					_indoorList[47]._sprites = wallItem._sprites;
					_wp[12] = idx;
				}
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][31])) {
			if (!_wo[27] && !_wo[21] && !_wo[14] && !_wo[24] && !_wo[10] && !_wo[31]) {
				if (_wp[18] == -1) {
					_indoorList[53]._frame = wallItem._frame;
					_indoorList[53]._sprites = wallItem._sprites;
					_wp[18] = idx;
				}
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][23])) {
			if (!_wo[25] && !_wo[28] && !_wo[20] && !_wo[11] && !_wo[16] && !_wo[30] && !_wo[32]) {
				if (_wp[11] == -1) {
					_indoorList[46]._frame = wallItem._frame;
					_indoorList[46]._sprites = wallItem._sprites;
					_wp[11] = idx;
				}
			}
		}

		if (wallItem._position.x == (mazePos.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				wallItem._position.y == (mazePos.y + Res.SCREEN_POSITIONING_Y[dir][31])) {
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

void InterfaceScene::setOutdoorsMonsters() {
	Combat &combat = *_vm->_combat;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Direction dir = party._mazeDirection;
	Common::Point pt = party._mazePosition;

	// Reset the list of attacking monsters
	combat.clearAttackers();

	// Iterate through the monsters list checking for proximity to party
	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][2]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][2])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[0] == -1) {
				_outdoorList[118]._frame = idx;
				combat._attackMonsters[0] = idx;
			} else if (combat._attackMonsters[1] == -1) {
				_outdoorList[112]._frame = idx;
				combat._attackMonsters[1] = idx;
			} else if (combat._attackMonsters[2] == -1) {
				_outdoorList[115]._frame = idx;
				combat._attackMonsters[2] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][7]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][7])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[3] == -1) {
				_outdoorList[94]._frame = idx;
				combat._attackMonsters[3] = idx;
			} else if (combat._attackMonsters[4] == -1) {
				_outdoorList[92]._frame = idx;
				combat._attackMonsters[4] = idx;
			} else if (combat._attackMonsters[5] == -1) {
				_outdoorList[93]._frame = idx;
				combat._attackMonsters[5] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][5]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][5])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[12] == -1) {
				_outdoorList[90]._frame = idx;
				combat._attackMonsters[12] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][9]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][9])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[13] == -1) {
				_outdoorList[91]._frame = idx;
				combat._attackMonsters[13] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][14]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][14])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[6] == -1) {
				_outdoorList[75]._frame = idx;
				combat._attackMonsters[6] = idx;
			} else if (combat._attackMonsters[7] == -1) {
				_outdoorList[73]._frame = idx;
				combat._attackMonsters[7] = idx;
			} else if (combat._attackMonsters[8] == -1) {
				_outdoorList[74]._frame = idx;
				combat._attackMonsters[8] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][12]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][12])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[14] == -1) {
				_outdoorList[69]._frame = idx;
				combat._attackMonsters[14] = idx;
			} else if (combat._attackMonsters[20] == -1) {
				_outdoorList[70]._frame = idx;
				combat._attackMonsters[20] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][16]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][16])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[15] == -1) {
				_outdoorList[71]._frame = idx;
				combat._attackMonsters[15] = idx;
			} else if (combat._attackMonsters[21] == -1) {
				_outdoorList[72]._frame = idx;
				combat._attackMonsters[21] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][27]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][27])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[9] == -1) {
				_outdoorList[52]._frame = idx;
				combat._attackMonsters[9] = idx;
			} else if (combat._attackMonsters[10] == -1) {
				_outdoorList[50]._frame = idx;
				combat._attackMonsters[10] = idx;
			} else if (combat._attackMonsters[11] == -1) {
				_outdoorList[51]._frame = idx;
				combat._attackMonsters[11] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][25]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][25])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[16] == -1) {
				_outdoorList[44]._frame = idx;
				combat._attackMonsters[16] = idx;
			} else if (combat._attackMonsters[22] == -1) {
				_outdoorList[42]._frame = idx;
				combat._attackMonsters[22] = idx;
			} else if (combat._attackMonsters[24] == -1) {
				_outdoorList[43]._frame = idx;
				combat._attackMonsters[24] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][23])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[18] == -1) {
				_outdoorList[48]._frame = idx;
				combat._attackMonsters[18] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][29]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][29])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[17] == -1) {
				_outdoorList[47]._frame = idx;
				combat._attackMonsters[17] = idx;
			} else if (combat._attackMonsters[23] == -1) {
				_outdoorList[45]._frame = idx;
				combat._attackMonsters[23] = idx;
			} else if (combat._attackMonsters[25] == -1) {
				_outdoorList[46]._frame = idx;
				combat._attackMonsters[25] = idx;
			}
		}

		if (monster._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				monster._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][31])) {
			monster._isAttacking = true;
			if (combat._attackMonsters[19] == -1) {
				_outdoorList[49]._frame = idx;
				combat._attackMonsters[19] = idx;
			}
		}
	}

	_outdoorList[115]._x = 58;
	_outdoorList[93]._x = 25;
	_outdoorList[74]._x = 9;
	_outdoorList[51]._x = -1;
	_outdoorList[43]._x = -26;
	_outdoorList[46]._x = 23;
	_outdoorList[48]._x = -58;
	_outdoorList[49]._x = 40;
	_outdoorList[69]._x = -65;
	_outdoorList[70]._x = -85;
	_outdoorList[71]._x = 49;
	_outdoorList[72]._x = 65;
	_outdoorList[90]._x = -112;
	_outdoorList[91]._x = 98;

	if (combat._attackMonsters[1] != -1 && combat._attackMonsters[2] == -1) {
		_outdoorList[118]._x = 31;
		_outdoorList[112]._x = -36;
	} else {
		_outdoorList[118]._x = -5;
		_outdoorList[112]._x = -67;
	}
	if (combat._attackMonsters[4] != -1 && combat._attackMonsters[5] == -1) {
		_outdoorList[94]._x = 8;
		_outdoorList[92]._x = -23;
	} else {
		_outdoorList[94]._x = -7;
		_outdoorList[92]._x = -38;
	}
	if (combat._attackMonsters[7] != -1 && combat._attackMonsters[8] == -1) {
		_outdoorList[75]._x = 0;
		_outdoorList[73]._x = -16;
	} else {
		_outdoorList[75]._x = -8;
		_outdoorList[73]._x = -24;
	}
	if (combat._attackMonsters[10] != -1 && combat._attackMonsters[11] == -1) {
		_outdoorList[52]._x = -5;
		_outdoorList[50]._x = -13;
	} else {
		_outdoorList[52]._x = -9;
		_outdoorList[50]._x = -17;
	}
	if (combat._attackMonsters[22] != -1 && combat._attackMonsters[24] == -1) {
		_outdoorList[44]._x = -27;
		_outdoorList[42]._x = -37;
	} else {
		_outdoorList[44]._x = -34;
		_outdoorList[42]._x = -41;
	}
	if (combat._attackMonsters[23] != -1 && combat._attackMonsters[25] == -1) {
		_outdoorList[47]._x = 20;
		_outdoorList[45]._x = -12;
	} else {
		_outdoorList[47]._x = 16;
		_outdoorList[45]._x = -16;
	}

	for (int idx = 0; idx < ATTACK_MONSTERS_COUNT; ++idx) {
		DrawStruct &ds = _outdoorList[Res.OUTDOOR_MONSTER_INDEXES[idx]];

		if (ds._frame != -1) {
			ds._flags &= ~0xfff;

			// Use the frame number as an index to the correct monster,
			// and replace the frame number with the monster's current frame
			MazeMonster &monster = map._mobData._monsters[ds._frame];
			ds._frame = monster._frame;

			if (monster._effect2) {
				ds._flags |= Res.MONSTER_EFFECT_FLAGS[monster._effect2 - 1][monster._effect3];
			}

			if (monster._frame > 7) {
				ds._frame -= 8;
				ds._sprites = monster._attackSprites;
			} else {
				ds._sprites = monster._sprites;
			}

			ds._y = Res.OUTDOOR_MONSTERS_Y[idx];

			MonsterStruct &monsterData = *monster._monsterData;
			if (monsterData._flying) {
				ds._x += Res.COMBAT_FLOAT_X[_combatFloatCounter];
				ds._y += Res.COMBAT_FLOAT_Y[_combatFloatCounter];
			}
		}
	}
}

void InterfaceScene::setOutdoorsObjects() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	const Common::Point &pt = party._mazePosition;
	Direction dir = party._mazeDirection;
	int posIndex;
	_objNumber = -1;

	for (uint idx = 0; idx < map._mobData._objects.size(); ++idx) {
		MazeObject &obj = map._mobData._objects[idx];

		if (_vm->_files->_ccNum) {
			posIndex = obj._spriteId == 47 ? 1 : 0;
		} else {
			posIndex = obj._spriteId == 113 ? 1 : 0;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][2]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][2]) &&
				_outdoorList[111]._frame == -1) {
			DrawStruct &ds = _outdoorList[111];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][0];
			ds._y = Res.MAP_OBJECT_Y[posIndex][0];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
			_objNumber = idx;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][7]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][7]) &&
				_outdoorList[87]._frame == -1) {
			DrawStruct &ds = _outdoorList[87];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][1];
			ds._y = Res.MAP_OBJECT_Y[posIndex][1];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][5]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][5]) &&
				_outdoorList[88]._frame == -1) {
			DrawStruct &ds = _outdoorList[88];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][2];
			ds._y = Res.MAP_OBJECT_Y[posIndex][2];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][9]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][9]) &&
				_outdoorList[89]._frame == -1) {
			DrawStruct &ds = _outdoorList[89];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][3];
			ds._y = Res.MAP_OBJECT_Y[posIndex][3];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][14]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][14]) &&
				_outdoorList[66]._frame == -1) {
			DrawStruct &ds = _outdoorList[66];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][4];
			ds._y = Res.MAP_OBJECT_Y[posIndex][4];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][12]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][12]) &&
				_outdoorList[67]._frame == -1) {
			DrawStruct &ds = _outdoorList[67];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][5];
			ds._y = Res.MAP_OBJECT_Y[posIndex][5];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][16]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][16]) &&
				_outdoorList[68]._frame == -1) {
			DrawStruct &ds = _outdoorList[68];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][6];
			ds._y = Res.MAP_OBJECT_Y[posIndex][6];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][27]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][27]) &&
				_outdoorList[37]._frame == -1) {
			DrawStruct &ds = _outdoorList[37];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][7];
			ds._y = Res.MAP_OBJECT_Y[posIndex][7];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][25]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][25]) &&
				_outdoorList[38]._frame == -1) {
			DrawStruct &ds = _outdoorList[38];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][8];
			ds._y = Res.MAP_OBJECT_Y[posIndex][8];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][23]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][23]) &&
				_outdoorList[40]._frame == -1) {
			DrawStruct &ds = _outdoorList[40];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][10];
			ds._y = Res.MAP_OBJECT_Y[posIndex][10];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][29]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][29]) &&
				_outdoorList[39]._frame == -1) {
			DrawStruct &ds = _outdoorList[39];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][9];
			ds._y = Res.MAP_OBJECT_Y[posIndex][9];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}

		if (obj._position.x == (pt.x + Res.SCREEN_POSITIONING_X[dir][31]) &&
				obj._position.y == (pt.y + Res.SCREEN_POSITIONING_Y[dir][31]) &&
				_outdoorList[41]._frame == -1) {
			DrawStruct &ds = _outdoorList[41];
			ds._x = Res.OUTDOOR_OBJECT_X[posIndex][11];
			ds._y = Res.MAP_OBJECT_Y[posIndex][11];
			ds._frame = obj._frame;
			ds._sprites = obj._sprites;

			ds._flags &= ~SPRFLAG_HORIZ_FLIPPED;
			if (obj._flipped)
				ds._flags |= SPRFLAG_HORIZ_FLIPPED;
		}
	}
}

void InterfaceScene::drawIndoors() {
	Map &map = *_vm->_map;
	int surfaceId;

	// Draw any surface tiles on top of the default ground
	for (int cellIndex = 0; cellIndex < 25; ++cellIndex) {
		map.getCell(Res.DRAW_NUMBERS[cellIndex]);

		DrawStruct &drawStruct = _indoorList._groundTiles[cellIndex];
		SpriteResource &sprites = map._surfaceSprites[map._currentSurfaceId];
		drawStruct._sprites = sprites.empty() ? (SpriteResource *)nullptr : &sprites;

		surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];
		if (surfaceId == SURFTYPE_WATER || surfaceId == SURFTYPE_LAVA ||
				surfaceId == SURFTYPE_SEWER) {
			drawStruct._frame = Res.DRAW_FRAMES[cellIndex][_flipWater ? 1 : 0];
			drawStruct._flags = _flipWater ? SPRFLAG_HORIZ_FLIPPED : 0;
		} else {
			drawStruct._frame = Res.DRAW_FRAMES[cellIndex][_flipGround ? 1 : 0];
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
		/* TODO: Duplicated switch in the original executable.. original bug meant to check some other index?
		else if (_wo[79])
			_indoorList._fwl_4F4R._frame = 15;*/
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
			_indoorList._swl_3F1R._frame = 15;
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

	if (_wo[27] || _wo[22] || _wo[15]) {
	} else if (_wo[96]) {
		_indoorList._fwl_4F._frame = 7;
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
		if (!_openDoor)
			_indoorList._fwl_1F1R._frame = 0;
		else
			_indoorList._fwl_1F1R._frame = map.mazeData()._wallKind ? 1 : 10;
	} else if (_wo[71]) {
		_indoorList._fwl_1F1R._frame = 9;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl2;
	} else if (_wo[167]) {
		_indoorList._fwl_1F1R._frame = _overallFrame + 1;
		_indoorList._fwl_1F1R._sprites = &map._wallSprites._fwl1;
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

	if (_openDoor) {
		Common::Point pt(
			_vm->_party->_mazePosition.x + Res.SCREEN_POSITIONING_X[
				_vm->_party->_mazeDirection][_vm->_party->_mazePosition.x],
			_vm->_party->_mazePosition.y + Res.SCREEN_POSITIONING_Y[
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
	_indoorList.draw();

	// Check for any character shooting
	_isAttacking = false;
	for (uint idx = 0; idx < _vm->_party->_activeParty.size(); ++idx) {
		if (_vm->_combat->_shootingRow[idx])
			_isAttacking = true;
	}

	_charsShooting = _isAttacking;
}

void InterfaceScene::drawOutdoors() {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int surfaceId;

	// Draw any surface tiles on top of the default ground
	for (int cellIndex = 0; cellIndex < 25; ++cellIndex) {
		map.getCell(cellIndex == 24 ? 2 : Res.DRAW_NUMBERS[cellIndex]);

		DrawStruct &drawStruct = _outdoorList._groundTiles[cellIndex];
		SpriteResource &sprites = map._surfaceSprites[map._currentSurfaceId];
		drawStruct._sprites = sprites.empty() ? (SpriteResource *)nullptr : &sprites;

		surfaceId = map.mazeData()._surfaceTypes[map._currentSurfaceId];
		if (surfaceId == SURFTYPE_DWATER || surfaceId == SURFTYPE_LAVA) {
			drawStruct._frame = Res.DRAW_FRAMES[cellIndex][_flipWater ? 1 : 0];
			drawStruct._flags = _flipWater ? SPRFLAG_HORIZ_FLIPPED : 0;
		} else {
			drawStruct._frame = Res.DRAW_FRAMES[cellIndex][_flipGround ? 1 : 0];
			drawStruct._flags = _flipGround ? SPRFLAG_HORIZ_FLIPPED : 0;
		}
	}

	party.handleLight();

	// Set up terrain draw entries
	const int TERRAIN_INDEXES1[9] = { 44, 36, 37, 38, 45, 43, 42, 41, 39 };
	const int TERRAIN_INDEXES2[5] = { 22, 24, 31, 29, 26 };
	const int TERRAIN_INDEXES3[3] = { 11, 16, 13 };
	const int TERRAIN_INDEXES4[5] = { 5, 9, 7, 0, 4 };

	// Loops to set draw entries for the terrain
	assert(map._currentWall != INVALID_CELL);
	for (int idx = 0; idx < 9; ++idx) {
		map.getCell(TERRAIN_INDEXES1[idx]);
		assert(map._currentWall != INVALID_CELL);
		SpriteResource &spr = map._wallSprites._surfaces[map._currentWall];
		_outdoorList[28 + idx]._sprites = spr.empty() ? (SpriteResource *)nullptr : &spr;
	}
	for (int idx = 0; idx < 5; ++idx) {
		map.getCell(TERRAIN_INDEXES2[idx]);
		assert(map._currentWall != INVALID_CELL);
		SpriteResource &spr = map._wallSprites._surfaces[map._currentWall];
		_outdoorList[61 + idx]._sprites = spr.empty() ? (SpriteResource *)nullptr : &spr;
	}
	for (int idx = 0; idx < 3; ++idx) {
		map.getCell(TERRAIN_INDEXES3[idx]);
		assert(map._currentWall != INVALID_CELL);
		SpriteResource &spr = map._wallSprites._surfaces[map._currentWall];
		_outdoorList[84 + idx]._sprites = spr.empty() ? (SpriteResource *)nullptr : &spr;
	}
	for (int idx = 0; idx < 5; ++idx) {
		map.getCell(TERRAIN_INDEXES4[idx]);
		assert(map._currentWall != INVALID_CELL);
		SpriteResource &spr = map._wallSprites._surfaces[map._currentWall];
		_outdoorList[103 + idx]._sprites = spr.empty() ? (SpriteResource *)nullptr : &spr;
	}

	map.getCell(1);
	assert(map._currentWall != INVALID_CELL);
	SpriteResource &surface = map._wallSprites._surfaces[map._currentWall];
	_outdoorList[108]._sprites = surface.empty() ? (SpriteResource *)nullptr : &surface;
	_outdoorList[109]._sprites = _outdoorList[108]._sprites;
	_outdoorList[110]._sprites = _outdoorList[108]._sprites;
	_outdoorList._sky1._flags = _outdoorList._sky2._flags = _flipSky ? SPRFLAG_HORIZ_FLIPPED : 0;
	_outdoorList._groundSprite._flags = _flipWater ? SPRFLAG_HORIZ_FLIPPED : 0;

	// Finally render the outdoor scene
	_outdoorList.draw();

	// Check for any character shooting
	_isAttacking = false;
	for (uint idx = 0; idx < _vm->_party->_activeParty.size(); ++idx) {
		if (_vm->_combat->_shootingRow[idx])
			_isAttacking = true;
	}

	_charsShooting = _isAttacking;
}

} // End of namespace Xeen
