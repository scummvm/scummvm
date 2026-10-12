/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "harold/bcb/static_data.h"

namespace BCB {

const StaticRect kMapSubmodeMciControlHitRect = {120, 56, 520, 256};
const StaticRect kMapSubmodeMovieBackdropRect = {120, 56, 540, 342};
const int kMapSubmodeMovieBackdropSourceX = 0;
const int kMapSubmodeMovieBackdropSourceY = 0x210;

const ButtonControlRecord kTitleButtonTable[5] = {
	{50, 0, 0, 0, 0},
	{51, 429, 263, 162, 32},
	{52, 429, 302, 162, 32},
	{53, 429, 341, 162, 32},
	{54, 429, 386, 162, 32}};

const ButtonControlRecord kSystemSubmodeButtonTable[5] = {
	{500, 0, 0, 0, 0},
	{501, 380, 53, 240, 44},
	{502, 380, 120, 240, 44},
	{503, 380, 216, 240, 44},
	{504, 505, 404, 96, 48}};

const ButtonControlRecord kSystemButtonTable[9] = {
	{550, 0, 0, 0, 0},
	{551, 41, 46, 288, 32},
	{552, 41, 78, 288, 32},
	{553, 41, 110, 288, 32},
	{554, 41, 142, 288, 32},
	{555, 41, 174, 288, 32},
	{556, 41, 206, 288, 32},
	{557, 41, 238, 288, 32},
	{558, 41, 270, 288, 32}};

const ButtonControlRecord kSceneChoiceButtonTable[10] = {
	{100, 0, 0, 0, 0},
	{101, 432, 48, 192, 40},
	{102, 432, 94, 192, 40},
	{103, 432, 140, 192, 40},
	{104, 432, 186, 192, 40},
	{105, 432, 232, 192, 40},
	{106, 432, 278, 192, 40},
	{107, 432, 324, 192, 40},
	{108, 432, 370, 192, 40},
	{109, 432, 416, 192, 40}};

const ButtonControlRecord kMapButtonTable[19] = {
	{200, 0, 0, 0, 0},
	{201, 371, 359, 16, 16},
	{202, 285, 35, 16, 16},
	{203, 395, 187, 16, 16},
	{204, 412, 210, 16, 16},
	{205, 283, 245, 16, 16},
	{206, 401, 228, 16, 16},
	{207, 438, 254, 16, 16},
	{208, 344, 278, 16, 16},
	{209, 371, 301, 16, 16},
	{210, 285, 52, 16, 16},
	{211, 495, 408, 16, 16},
	{212, 197, 19, 16, 16},
	{213, 308, 164, 16, 16},
	{214, 302, 335, 16, 16},
	{215, 234, 115, 16, 16},
	{216, 270, 225, 16, 16},
	{217, 317, 392, 16, 16},
	{218, 393, 375, 16, 16}};

const int kMapButtonSecondaryStateIndexTable[19] = {
	0, 0, 0, 41, 42, 0, 42, 44, 0, 0, 40, 0, 0, 39, 45, 46, 43, 47, 48};

const ButtonControlRecord kComputerPrimaryButtonTable[2] = {
	{600, 0, 0, 0, 0},
	{601, 49, 40, 16, 16}};

const ButtonControlRecord kComputerDetailButtonTable[3] = {
	{750, 0, 0, 0, 0},
	{751, 54, 50, 17, 17},
	{752, 508, 402, 80, 24}};

const ButtonControlRecord kComputerChoiceLayoutSlots[21] = {
	{0, 0, 0, 0, 0},
	{0, 83, 125, 64, 64},
	{0, 171, 125, 64, 64},
	{0, 259, 125, 64, 64},
	{0, 347, 125, 64, 64},
	{0, 435, 125, 64, 64},
	{0, 83, 195, 64, 64},
	{0, 171, 195, 64, 64},
	{0, 259, 195, 64, 64},
	{0, 347, 195, 64, 64},
	{0, 435, 195, 64, 64},
	{0, 83, 265, 64, 64},
	{0, 171, 265, 64, 64},
	{0, 259, 265, 64, 64},
	{0, 347, 265, 64, 64},
	{0, 435, 265, 64, 64},
	{0, 83, 335, 64, 64},
	{0, 171, 335, 64, 64},
	{0, 259, 335, 64, 64},
	{0, 347, 335, 64, 64},
	{0, 435, 335, 64, 64}};

const ButtonControlRecord kComputerChoiceButtonTable[30] = {
	{701, 0, 480, 64, 64},
	{702, 64, 480, 64, 64},
	{703, 128, 480, 64, 64},
	{704, 192, 480, 64, 64},
	{705, 256, 480, 64, 64},
	{706, 320, 480, 64, 64},
	{707, 384, 480, 64, 64},
	{708, 448, 480, 64, 64},
	{709, 512, 480, 64, 64},
	{710, 576, 480, 64, 64},
	{711, 0, 544, 64, 64},
	{712, 64, 544, 64, 64},
	{713, 128, 544, 64, 64},
	{714, 192, 544, 64, 64},
	{715, 256, 544, 64, 64},
	{716, 320, 544, 64, 64},
	{717, 384, 544, 64, 64},
	{718, 448, 544, 64, 64},
	{719, 512, 544, 64, 64},
	{720, 0, 0, 64, 64},
	{721, 576, 544, 64, 64},
	{722, 0, 608, 64, 64},
	{723, 64, 608, 64, 64},
	{724, 128, 608, 64, 64},
	{725, 192, 608, 64, 64},
	{726, 256, 608, 64, 64},
	{727, 320, 608, 64, 64},
	{728, 384, 608, 64, 64},
	{729, 448, 608, 64, 64},
	{730, 512, 608, 64, 64}};

const StaticHitRect kComputerChoiceGroupHitRects[3] = {
	{60, 67, 131, 89},
	{137, 67, 208, 89},
	{214, 67, 285, 89}};

const byte kComputerChoiceDependencyStateIndex[30] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 30,
	31, 32, 33, 34, 0, 0, 0, 0, 35, 0,
	37, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const CompositeBitmapVariantSet kComputerDetailBitmapVariantsBySelectedValue[31] = {
	{0, {}},
	{1, {"bmp/comp/pcom_01.bmp"}},
	{1, {"bmp/comp/pcom_02.bmp"}},
	{1, {"bmp/comp/pcom_03.bmp"}},
	{1, {"bmp/comp/pcom_04.bmp"}},
	{1, {"bmp/comp/pcom_05.bmp"}},
	{1, {"bmp/comp/pcom_06.bmp"}},
	{1, {"bmp/comp/pcom_07.bmp"}},
	{1, {"bmp/comp/pcom_08.bmp"}},
	{1, {"bmp/comp/pcom_09.bmp"}},
	{1, {"bmp/comp/pcom_10.bmp"}},
	{1, {"bmp/comp/pcom_11.bmp"}},
	{1, {"bmp/comp/pcom_12.bmp"}},
	{1, {"bmp/comp/pcom_13.bmp"}},
	{1, {"bmp/comp/pcom_14.bmp"}},
	{1, {"bmp/comp/pcom_15.bmp"}},
	{1, {"bmp/comp/pcom_16.bmp"}},
	{1, {"bmp/comp/pcom_17.bmp"}},
	{1, {"bmp/comp/pcom_18.bmp"}},
	{1, {"bmp/comp/pcom_19.bmp"}},
	{1, {"bmp/comp/pcom_31a.bmp"}},
	{1, {"bmp/comp/pcom_21.bmp"}},
	{4, {"bmp/comp/pcom_22a.bmp", "bmp/comp/pcom_22b.bmp", "bmp/comp/pcom_22c.bmp", "bmp/comp/pcom_22d.bmp"}},
	{4, {"bmp/comp/pcom_23a.bmp", "bmp/comp/pcom_23b.bmp", "bmp/comp/pcom_23c.bmp", "bmp/comp/pcom_23d.bmp"}},
	{3, {"bmp/comp/pcom_24a.bmp", "bmp/comp/pcom_24b.bmp", "bmp/comp/pcom_24c.bmp"}},
	{2, {"bmp/comp/pcom_25a.bmp", "bmp/comp/pcom_25b.bmp"}},
	{2, {"bmp/comp/pcom_26a.bmp", "bmp/comp/pcom_26b.bmp"}},
	{1, {"bmp/comp/pcom_27a.bmp"}},
	{3, {"bmp/comp/pcom_28a.bmp", "bmp/comp/pcom_28b.bmp", "bmp/comp/pcom_28c.bmp"}},
	{3, {"bmp/comp/pcom_29a.bmp", "bmp/comp/pcom_29b.bmp", "bmp/comp/pcom_29c.bmp"}},
	{1, {"bmp/comp/pcom_30a.bmp"}}};

const byte kSceneChoiceValueByButtonIndex[10] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 255};

const int kStillImageDurationTicks[8] = {100, 100, 100, 100, 100, 600, 600, 200};

const char *const kStillImageHighColorPaths[8] = {
	"bmp/still/16m/pstill_1.bmp",
	"bmp/still/16m/pstill_2.bmp",
	"bmp/still/16m/pstill_3.bmp",
	"bmp/still/16m/pstill_4.bmp",
	"bmp/still/16m/pstill_5.bmp",
	"bmp/still/16m/pbad.bmp",
	"bmp/still/16m/pgood.bmp",
	"bmp/still/16m/pstaff.bmp"};

} // End of namespace BCB
