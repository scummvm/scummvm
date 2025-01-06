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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ASYLUM_STATICRES_H
#define ASYLUM_STATICRES_H

#include "common/rect.h"

namespace Asylum {

/** This fixes the menu icons text x position on screen */
const int16 menuRects[12][4] = {{ 40,  19,   93,  68},
								{140,  19,  193,  68},
								{239,  19,  292,  68},
								{340,  19,  393,  68},
								{439,  19,  492,  68},
								{538,  19,  591,  68},
								{ 40, 400,   93, 449},
								{140, 400,  193, 449},
								{239, 400,  292, 449},
								{340, 400,  393, 449},
								{538, 400,  591, 449}, // The last two icons are swapped
								{439, 400,  492, 449}};

const uint32 eyeFrameIndex[8] = {3, 5, 1, 7, 4, 8, 2, 6};

const int moviesCd[49] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                          1, 1, 1, 1, 1, 1, 1, 2, 2, 2,
                          2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                          2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
                          3, 3, 3, 3, 3, 3, 3, 3, 2};

const int chapterIndexes[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 4, 4, 8, 9, 0, 0};

/** Default Actor Indices by Scene */
const ActorIndexes actorType[16] = {kActorMax,   kActorMax, kActorMax,     kActorMax,
                                    kActorSarah, kActorMax, kActorCyclops, kActorMax,
                                    kActorAztec, kActorMax, kActorSarah,   kActorSarah,
                                    kActorAztec, kActorMax, kActorMax,     kActorMax};

/** Speech indexes */
const uint speechIndex[20]       = {17, 22, 27, 37, 45, 12, 16, 19, 25, 29, 14, 18, 23, 29, 35,  6,  9, 13, 19, 27};
const uint speechIndexRandom[20] = { 5,  5, 10,  8,  6,  4,  3,  6,  4,  4,  4,  5,  6,  6,  6,  3,  4,  6,  8,  4};

/** Encounter indexes */
const int encounterPortrait1Index[16] = {-1, 36, 20, 31, 1, 37, 7, 0, 0, 0, 1, 2, 3, 0, 0, 0};
const int encounterPortrait2Index[80] = {32, 33, 34, 21, 22, 23, 24, 25, 26, 27,
                                         28, 29, 30, 31, 32,  1,  2,  3,  1,  2,
                                          3,  4,  5,  6,  7,  8,  9, 10, 11, 12,
                                         13, 14, 19, 20, 21, 22, 23, 24, 25, 26,
                                         27, 28, 29, 30,  2,  0,  3,  4,  5,  6,
                                          7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
                                         17, 18, 19, 20, 21, 22,  0,  1,  2,  3,
                                          4,  5,  6, 35, 33, 34, 35, 36, 37, 36};

// Delta array for points
static const int16 deltaPointsArray[8][2] = {
	{ 0, -1}, {-1, -1}, {-1,  0}, {-1,  1},
	{ 0,  1}, { 1,  1}, { 1,  0}, { 1, -1}
};

// We hardcode all the text resources here. It makes the resulting code easier,
// otherwise we'll have to read the text resources in the same obscure way
// they're stored in VIDS.CAP.
// A value of -1 means that the video has no subtitles, -2 that it doesn't exist.
// The negative values aren't used in the code, they just make the table
// easier to understand.
const int32 video_subtitle_resourceIds[49] = {
	-1,   1088, 1279, 1122, 1286, 1132, 1133, 1134, 1135, 1136, // 0 - 9
	-1,   -2,   1140, 1141, -2,   -1,   1142, -1,   -2,   1155, // 10 - 19
	1157, 1159, 1162, 1164, -2,   1171, 1177, 1184, 1190, 1201, // 20 - 29
	-2,   -2,   -2,   1207, 1213, 1217, 1223, 1227, -2,   1228, // 30 - 39
	-2,   1244, 1247, 1250, 1256, 1120, 1127, -1,   -1 };       // 40 - 48

// Angle Tables used by getAngle()
// The angle values are in degrees units
const uint8 angleTable01[256] = {
	 0,  0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  5,  5,  5,  5,  6,  6,  6,  6,  6,  7,  7,
	 7,  7,  8,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10, 10,
	11, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 13, 13, 14, 14,
	14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 17,
	17, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20,
	21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23,
	24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26,
	27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 29, 29, 29, 29, 29,
	29, 30, 30, 30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 32, 32, 32,
	32, 32, 32, 32, 33, 33, 33, 33, 33, 33, 34, 34, 34, 34, 34, 34,
	35, 35, 35, 35, 35, 35, 35, 36, 36, 36, 36, 36, 36, 36, 37, 37,
	37, 37, 37, 37, 37, 38, 38, 38, 38, 38, 38, 38, 39, 39, 39, 39,
	39, 39, 39, 39, 40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41,
	41, 41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43, 43,
	43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44, 44, 45, 45, 45, 45
};

const uint8 angleTable02[256] = {
	 0,  4,  7, 11, 14, 17, 21, 24, 27, 29, 32, 35, 37, 39, 41, 43,
	45, 47, 48, 50, 51, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	63, 64, 65, 65, 66, 67, 67, 68, 68, 69, 69, 70, 70, 70, 71, 71,
	72, 72, 72, 73, 73, 73, 73, 74, 74, 74, 75, 75, 75, 75, 76, 76,
	76, 76, 76, 77, 77, 77, 77, 77, 77, 78, 78, 78, 78, 78, 78, 79,
	79, 79, 79, 79, 79, 79, 79, 80, 80, 80, 80, 80, 80, 80, 80, 80,
	81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 81, 82, 82, 82, 82,
	82, 82, 82, 82, 82, 82, 82, 82, 82, 82, 83, 83, 83, 83, 83, 83,
	83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 83, 84, 84, 84,
	84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84, 84,
	84, 84, 84, 84, 84, 84, 84, 85, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85,
	85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 85, 86, 86, 86, 86,
	86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
	86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86,
	86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86, 86
};

const uint8 angleTable03[256] = {
	 0, 45, 63, 72, 76, 79, 81, 82, 83, 84, 84, 85, 85, 86, 86, 86,
	86, 87, 87, 87, 87, 87, 87, 88, 88, 88, 88, 88, 88, 88, 88, 88,
	88, 88, 88, 88, 88, 88, 88, 89, 89, 89, 89, 89, 89, 89, 89, 89,
	89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
	89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
	89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
	89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
	89, 89, 89, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90, 90
};

} // end of namespace Asylum

#endif // ASYLUM_STATICRES_H
