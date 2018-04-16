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

#include "bbvs/minigames/bbloogie.h"

namespace Bbvs {

static const int kAnim0FrameIndices[] = {0, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 0, 5, 6, 7, 0, 0, 5, 6, 7, 8, 0, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 0};
static const int16 kAnim0FrameTicks[] = {22, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 20, 8, 8, 10, 10, 10, 8, 22, 6, 12, 20, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 22};
static const BBRect kAnim0FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim1FrameIndices[] = {9, 10, 11, 12, 13, 10, 11, 12, 13, 10, 11, 12, 13, 9, 14, 15, 16, 9, 9, 14, 15, 16, 17, 9, 13, 12, 11, 10, 13, 12, 11, 10, 13, 12, 11, 10, 9};
static const int16 kAnim1FrameTicks[] = {22, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 20, 8, 8, 10, 10, 10, 8, 22, 6, 12, 20, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 22};
static const BBRect kAnim1FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim2FrameIndices[] = {18, 19, 20, 18, 21, 22};
static const int16 kAnim2FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim2FrameRects[] = {{-45, -43, 86, 38}, {-45, -43, 86, 38}, {-45, -43, 86, 38}, {-45, -43, 86, 38}, {-45, -43, 86, 38}, {-45, -43, 86, 38}};
static const int kAnim3FrameIndices[] = {23, 24, 25, 26, 27, 28, 27};
static const int16 kAnim3FrameTicks[] = {6, 6, 6, 6, 6, 7, 6};
static const BBRect kAnim3FrameRects[] = {{-24, -17, 48, 14}, {-24, -17, 48, 14}, {-24, -17, 48, 14}, {-24, -17, 48, 14}, {-24, -17, 48, 14}, {-24, -17, 48, 14}, {-24, -17, 48, 14}};
static const int kAnim4FrameIndices[] = {29, 30, 31, 32, 33, 34, 35, 36};
static const int16 kAnim4FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim4FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim5FrameIndices[] = {37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70};
static const int16 kAnim5FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim5FrameRects[] = {{-1, -11, 4, 11}, {-2, -15, 6, 8}, {-3, -24, 8, 8}, {-3, -31, 7, 9}, {-3, -33, 8, 8}, {-3, -34, 8, 10}, {-2, -34, 7, 8}, {-1, -34, 6, 7}, {-1, -34, 5, 6}, {-1, -34, 4, 4}, {0, -34, 3, 4}, {-1, -34, 4, 3}, {0, -34, 3, 4}, {0, -33, 3, 3}, {-1, -35, 5, 5}, {-3, -37, 9, 9}, {-4, -39, 12, 13}, {-3, -11, 7, 8}, {-3, -15, 8, 9}, {-5, -24, 11, 13}, {-4, -31, 10, 13}, {-5, -34, 11, 13}, {-5, -34, 11, 11}, {-4, -34, 9, 10}, {-4, -34, 9, 9}, {-3, -34, 7, 8}, {-2, -34, 6, 7}, {-2, -34, 5, 6}, {-2, -34, 4, 5}, {-7, -38, 13, 13}, {-10, -44, 22, 22}, {-13, -47, 27, 27}, {-17, -49, 32, 30}, {-17, -50, 34, 33}};
static const int kAnim6FrameIndices[] = {71};
static const int16 kAnim6FrameTicks[] = {1};
static const BBRect kAnim6FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim7FrameIndices[] = {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 80, 79, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 110, 109, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129};
static const int16 kAnim7FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 30, 6, 20, 6, 30, 6, 6, 6, 6, 6, 6, 6, 6, 6, 30, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 30, 6, 20, 6, 30, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim7FrameRects[] = {{-46, -6, 7, 6}, {-46, -12, 11, 12}, {-47, -15, 17, 15}, {-46, -14, 20, 12}, {-47, -10, 24, 10}, {-41, -11, 22, 11}, {-33, -10, 15, 10}, {-32, -10, 14, 10}, {-32, -9, 13, 9}, {-32, -9, 13, 9}, {-32, -9, 13, 9}, {-32, -10, 13, 10}, {-34, -11, 24, 11}, {-30, -12, 25, 9}, {-24, -10, 24, 10}, {-18, -11, 22, 11}, {-14, -11, 24, 10}, {-9, -12, 25, 9}, {-3, -10, 24, 10}, {4, -11, 22, 11}, {11, -10, 15, 10}, {12, -10, 13, 10}, {10, -11, 24, 11}, {15, -12, 25, 9}, {22, -16, 22, 16}, {34, -16, 9, 16}, {35, -12, 9, 12}, {38, -6, 6, 6}, {38, -6, 4, 4}, {36, -6, 7, 6}, {31, -12, 12, 12}, {27, -15, 17, 15}, {24, -12, 20, 12}, {19, -11, 22, 11}, {13, -11, 24, 11}, {7, -11, 25, 9}, {4, -10, 24, 10}, {1, -11, 22, 11}, {1, -10, 15, 10}, {2, -10, 13, 10}, {2, -10, 13, 10}, {2, -9, 13, 9}, {2, -10, 13, 10}, {2, -10, 13, 10}, {-7, -11, 24, 11}, {-14, -11, 25, 9}, {-21, -10, 24, 11}, {-27, -11, 23, 11}, {-34, -12, 24, 11}, {-44, -18, 22, 16}, {-44, -16, 9, 16}, {-46, -12, 9, 12}, {-45, -6, 7, 6}, {-45, -4, 6, 5}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim8FrameIndices[] = {130};
static const int16 kAnim8FrameTicks[] = {6};
static const BBRect kAnim8FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim9FrameIndices[] = {131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141};
static const int16 kAnim9FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim9FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim10FrameIndices[] = {142};
static const int16 kAnim10FrameTicks[] = {2};
static const BBRect kAnim10FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim11FrameIndices[] = {143};
static const int16 kAnim11FrameTicks[] = {1};
static const BBRect kAnim11FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim12FrameIndices[] = {144};
static const int16 kAnim12FrameTicks[] = {1};
static const BBRect kAnim12FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim13FrameIndices[] = {145};
static const int16 kAnim13FrameTicks[] = {1};
static const BBRect kAnim13FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim14FrameIndices[] = {146};
static const int16 kAnim14FrameTicks[] = {1};
static const BBRect kAnim14FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim15FrameIndices[] = {147};
static const int16 kAnim15FrameTicks[] = {1};
static const BBRect kAnim15FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim16FrameIndices[] = {148, 149, 150, 151, 152, 153, 154, 155};
static const int16 kAnim16FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim16FrameRects[] = {{-5, -5, 9, 9}, {-6, -5, 11, 11}, {-6, -4, 9, 9}, {-5, -5, 10, 10}, {-5, -3, 9, 9}, {-6, -5, 10, 10}, {-4, -4, 9, 9}, {-6, -4, 10, 10}};
static const int kAnim17FrameIndices[] = {156, 157};
static const int16 kAnim17FrameTicks[] = {6, 6};
static const BBRect kAnim17FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim18FrameIndices[] = {158, 159, 160, 161, 160, 162, 163, 162, 164, 165, 166, 167, 168, 167, 169, 170, 169, 171, 172, 173, 174, 175, 174, 176, 177, 176, 178, 179, 180, 181, 182, 181, 183, 184, 183, 185, 186, 187, 188, 189, 190, 188, 189, 191, 188, 190, 189, 187, 186};
static const int16 kAnim18FrameTicks[] = {10, 20, 8, 8, 8, 8, 8, 8, 6, 10, 20, 8, 8, 8, 8, 8, 8, 6, 10, 20, 8, 8, 8, 8, 8, 8, 6, 10, 20, 8, 8, 8, 8, 8, 8, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim18FrameRects[] = {{-13, -16, 26, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-10, -19, 18, 20}, {-8, -20, 15, 23}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-10, -18, 19, 20}, {-12, -17, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-12, -16, 24, 16}, {-10, -18, 20, 19}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-8, -20, 16, 22}, {-9, -19, 18, 20}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}, {-12, -17, 24, 17}};
static const int kAnim19FrameIndices[] = {192};
static const int16 kAnim19FrameTicks[] = {8};
static const BBRect kAnim19FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim20FrameIndices[] = {193};
static const int16 kAnim20FrameTicks[] = {5};
static const BBRect kAnim20FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim21FrameIndices[] = {194};
static const int16 kAnim21FrameTicks[] = {6};
static const BBRect kAnim21FrameRects[] = {{-7, -80, 17, 81}};
static const int kAnim22FrameIndices[] = {195, 196, 197, 198, 199, 200};
static const int16 kAnim22FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim22FrameRects[] = {{-22, -91, 45, 93}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}};
static const int kAnim23FrameIndices[] = {201};
static const int16 kAnim23FrameTicks[] = {6};
static const BBRect kAnim23FrameRects[] = {{-12, -75, 21, 75}};
static const int kAnim24FrameIndices[] = {202, 203, 204, 205, 206, 207};
static const int16 kAnim24FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim24FrameRects[] = {{-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}};
static const int kAnim25FrameIndices[] = {208, 209};
static const int16 kAnim25FrameTicks[] = {6, 6};
static const BBRect kAnim25FrameRects[] = {{-9, -9, 17, 15}, {-11, -10, 19, 16}};
static const ObjAnimation kAnimations[] = {
	{37, kAnim0FrameIndices, kAnim0FrameTicks, kAnim0FrameRects},
	{37, kAnim1FrameIndices, kAnim1FrameTicks, kAnim1FrameRects},
	{6, kAnim2FrameIndices, kAnim2FrameTicks, kAnim2FrameRects},
	{7, kAnim3FrameIndices, kAnim3FrameTicks, kAnim3FrameRects},
	{8, kAnim4FrameIndices, kAnim4FrameTicks, kAnim4FrameRects},
	{34, kAnim5FrameIndices, kAnim5FrameTicks, kAnim5FrameRects},
	{1, kAnim6FrameIndices, kAnim6FrameTicks, kAnim6FrameRects},
	{62, kAnim7FrameIndices, kAnim7FrameTicks, kAnim7FrameRects},
	{1, kAnim8FrameIndices, kAnim8FrameTicks, kAnim8FrameRects},
	{11, kAnim9FrameIndices, kAnim9FrameTicks, kAnim9FrameRects},
	{1, kAnim10FrameIndices, kAnim10FrameTicks, kAnim10FrameRects},
	{1, kAnim11FrameIndices, kAnim11FrameTicks, kAnim11FrameRects},
	{1, kAnim12FrameIndices, kAnim12FrameTicks, kAnim12FrameRects},
	{1, kAnim13FrameIndices, kAnim13FrameTicks, kAnim13FrameRects},
	{1, kAnim14FrameIndices, kAnim14FrameTicks, kAnim14FrameRects},
	{1, kAnim15FrameIndices, kAnim15FrameTicks, kAnim15FrameRects},
	{8, kAnim16FrameIndices, kAnim16FrameTicks, kAnim16FrameRects},
	{2, kAnim17FrameIndices, kAnim17FrameTicks, kAnim17FrameRects},
	{49, kAnim18FrameIndices, kAnim18FrameTicks, kAnim18FrameRects},
	{1, kAnim19FrameIndices, kAnim19FrameTicks, kAnim19FrameRects},
	{1, kAnim20FrameIndices, kAnim20FrameTicks, kAnim20FrameRects},
	{1, kAnim21FrameIndices, kAnim21FrameTicks, kAnim21FrameRects},
	{6, kAnim22FrameIndices, kAnim22FrameTicks, kAnim22FrameRects},
	{1, kAnim23FrameIndices, kAnim23FrameTicks, kAnim23FrameRects},
	{6, kAnim24FrameIndices, kAnim24FrameTicks, kAnim24FrameRects},
	{2, kAnim25FrameIndices, kAnim25FrameTicks, kAnim25FrameRects}
};

const ObjAnimation *MinigameBbLoogie::getAnimation(int animIndex) {
	return &kAnimations[animIndex];
}

} // End of namespace Bbvs
