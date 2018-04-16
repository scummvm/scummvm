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

#include "bbvs/minigames/bbtennis.h"

namespace Bbvs {

static const int kAnim0FrameIndices[] = {0, 1, 2, 3};
static const int16 kAnim0FrameTicks[] = {6, 6, 6, 6};
static const BBRect kAnim0FrameRects[] = {{-15, -11, 22, 10}, {-15, -12, 23, 10}, {-14, -11, 22, 8}, {-13, -11, 20, 10}};
static const int kAnim1FrameIndices[] = {4, 5, 6, 7, 8, 3};
static const int16 kAnim1FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim1FrameRects[] = {{-16, -3, 7, 6}, {-13, -8, 11, 10}, {-14, -12, 15, 12}, {-15, -10, 17, 10}, {-17, -10, 22, 9}, {-13, -12, 20, 12}};
static const int kAnim2FrameIndices[] = {9, 10, 11, 12};
static const int16 kAnim2FrameTicks[] = {6, 8, 8, 8};
static const BBRect kAnim2FrameRects[] = {{-11, -14, 20, 14}, {-1, -14, 10, 15}, {3, -9, 6, 10}, {2, -5, 7, 6}};
static const int kAnim3FrameIndices[] = {13, 14, 15, 16, 17};
static const int16 kAnim3FrameTicks[] = {8, 8, 6, 6, 6};
static const BBRect kAnim3FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim4FrameIndices[] = {18, 19};
static const int16 kAnim4FrameTicks[] = {61, 22};
static const BBRect kAnim4FrameRects[] = {{-8, -12, 14, 11}, {-8, -12, 14, 11}};
static const int kAnim5FrameIndices[] = {20};
static const int16 kAnim5FrameTicks[] = {6};
static const BBRect kAnim5FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim6FrameIndices[] = {21, 22, 23, 24, 25, 26, 27, 28, 29};
static const int16 kAnim6FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim6FrameRects[] = {{-59, -43, 114, 114}, {-24, -13, 44, 46}, {-12, -5, 24, 25}, {-8, -3, 15, 15}, {-5, -3, 8, 8}, {-3, -2, 5, 5}, {-1, -1, 3, 3}, {0, 0, 2, 2}, {-56, 25, 102, 50}};
static const int kAnim7FrameIndices[] = {30};
static const int16 kAnim7FrameTicks[] = {6};
static const BBRect kAnim7FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim8FrameIndices[] = {31};
static const int16 kAnim8FrameTicks[] = {6};
static const BBRect kAnim8FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim9FrameIndices[] = {32};
static const int16 kAnim9FrameTicks[] = {6};
static const BBRect kAnim9FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim10FrameIndices[] = {33};
static const int16 kAnim10FrameTicks[] = {6};
static const BBRect kAnim10FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim11FrameIndices[] = {34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 58, 59, 60, 61, 62, 63, 64, 42, 65};
static const int16 kAnim11FrameTicks[] = {10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 6, 6, 6, 10, 10, 10, 10};
static const BBRect kAnim11FrameRects[] = {{0, -50, 16, 47}, {1, -49, 16, 47}, {-1, -49, 17, 46}, {0, -47, 16, 45}, {2, -46, 15, 46}, {0, -48, 17, 45}, {2, -50, 14, 49}, {-2, -46, 17, 46}, {0, -57, 15, 57}, {-2, -56, 14, 56}, {-4, -56, 13, 56}, {-4, -56, 15, 56}, {5, -51, 14, 49}, {4, -52, 15, 52}, {-1, -57, 13, 57}, {0, -55, 14, 55}, {-5, -50, 17, 49}, {-9, -50, 17, 49}, {-9, -48, 16, 47}, {-6, -49, 14, 48}, {-8, -50, 17, 50}, {-10, -48, 19, 48}, {-2, -50, 14, 50}, {2, -47, 13, 48}, {-1, -57, 13, 57}, {4, -55, 12, 56}, {4, -58, 13, 59}, {5, -58, 12, 59}, {5, -57, 15, 58}, {1, -57, 14, 57}, {-7, -51, 15, 51}, {-5, -53, 16, 53}, {0, -57, 15, 57}, {1, -55, 14, 55}};
static const int kAnim12FrameIndices[] = {66, 67, 68, 69, 70, 71, 72, 73, 69, 68, 67, 66, 74, 75};
static const int16 kAnim12FrameTicks[] = {10, 10, 10, 20, 10, 10, 6, 10, 20, 10, 10, 10, 8, 6};
static const BBRect kAnim12FrameRects[] = {{-5, -8, 12, 6}, {-12, -17, 24, 15}, {-12, -28, 24, 28}, {-10, -36, 20, 35}, {-9, -36, 18, 37}, {-11, -37, 17, 38}, {-6, -36, 16, 34}, {-5, -35, 20, 39}, {-10, -36, 20, 35}, {-12, -28, 24, 28}, {-12, -17, 24, 15}, {-5, -8, 12, 6}, {-15, -27, 23, 38}, {-19, -17, 15, 17}};
static const int kAnim13FrameIndices[] = {76, 77, 78, 77, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 98, 102, 103, 90};
static const int16 kAnim13FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim13FrameRects[] = {{-21, -61, 16, 52}, {-42, -76, 7, 14}, {-43, -75, 13, 24}, {-42, -76, 7, 14}, {-42, -75, 4, 42}, {-42, -76, 11, 57}, {-40, -74, 13, 55}, {-36, -74, 11, 55}, {-31, -72, 12, 56}, {-27, -71, 14, 57}, {-20, -69, 15, 55}, {-12, -65, 15, 51}, {-7, -57, 18, 44}, {-3, -43, 18, 29}, {4, -27, 20, 14}, {0, -28, 13, 14}, {0, -38, 14, 24}, {-1, -49, 19, 36}, {0, -61, 17, 47}, {-2, -63, 19, 49}, {-5, -64, 19, 50}, {-3, -62, 18, 48}, {0, -61, 19, 47}, {0, -61, 16, 47}, {-4, -48, 17, 34}, {-9, -37, 15, 23}, {-13, -26, 14, 12}, {0, -61, 16, 47}, {0, -50, 15, 36}, {0, -39, 13, 25}, {0, -28, 12, 14}};
static const int kAnim14FrameIndices[] = {104, 105, 106, 105, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 126, 130, 131, 118};
static const int16 kAnim14FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim14FrameRects[] = {{6, -61, 14, 52}, {35, -77, 7, 16}, {29, -76, 13, 24}, {35, -77, 7, 16}, {38, -76, 4, 43}, {32, -75, 10, 55}, {24, -74, 16, 54}, {22, -74, 14, 53}, {18, -72, 12, 55}, {12, -71, 15, 57}, {2, -69, 17, 55}, {-5, -65, 18, 51}, {-13, -57, 18, 43}, {-20, -43, 23, 29}, {-26, -30, 25, 16}, {-13, -28, 13, 14}, {-16, -38, 24, 24}, {-16, -49, 20, 35}, {-15, -61, 17, 47}, {-15, -63, 17, 49}, {-13, -64, 17, 51}, {-14, -62, 15, 48}, {-19, -61, 19, 47}, {-16, -61, 16, 48}, {-18, -48, 22, 34}, {-6, -37, 14, 23}, {0, -27, 12, 14}, {-16, -61, 16, 48}, {-16, -50, 19, 36}, {-14, -39, 15, 25}, {-12, -28, 12, 15}};
static const int kAnim15FrameIndices[] = {132, 133, 134, 133, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 154, 158, 159, 146};
static const int16 kAnim15FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim15FrameRects[] = {{-21, -61, 16, 52}, {-42, -76, 7, 14}, {-43, -75, 13, 24}, {-42, -76, 7, 14}, {-42, -75, 4, 42}, {-42, -76, 11, 57}, {-40, -74, 13, 55}, {-36, -74, 11, 55}, {-31, -72, 12, 56}, {-27, -71, 14, 57}, {-20, -69, 15, 55}, {-12, -65, 15, 51}, {-7, -57, 18, 44}, {-3, -43, 18, 29}, {4, -27, 20, 14}, {0, -28, 13, 14}, {0, -38, 14, 24}, {-1, -49, 19, 36}, {0, -61, 17, 47}, {-2, -63, 19, 49}, {-5, -64, 19, 50}, {-3, -62, 18, 48}, {0, -61, 19, 47}, {0, -61, 16, 47}, {-4, -48, 17, 34}, {-9, -37, 15, 23}, {-13, -26, 14, 12}, {0, -61, 16, 47}, {0, -50, 15, 36}, {0, -39, 13, 25}, {0, -28, 12, 14}};
static const int kAnim16FrameIndices[] = {160, 161, 162, 161, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 182, 186, 187, 174};
static const int16 kAnim16FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim16FrameRects[] = {{6, -61, 14, 52}, {35, -77, 7, 16}, {29, -76, 13, 24}, {35, -77, 7, 16}, {38, -76, 4, 43}, {32, -75, 10, 55}, {24, -74, 16, 54}, {22, -74, 14, 53}, {18, -72, 12, 55}, {12, -71, 15, 57}, {2, -69, 17, 55}, {-5, -65, 18, 51}, {-13, -57, 18, 43}, {-20, -43, 23, 29}, {-26, -30, 25, 16}, {-13, -28, 13, 14}, {-16, -38, 24, 24}, {-16, -49, 20, 35}, {-15, -61, 17, 47}, {-15, -63, 17, 49}, {-13, -64, 17, 51}, {-14, -62, 15, 48}, {-19, -61, 19, 47}, {-16, -61, 16, 48}, {-18, -48, 22, 34}, {-6, -37, 14, 23}, {0, -27, 12, 14}, {-16, -61, 16, 48}, {-16, -50, 19, 36}, {-14, -39, 15, 25}, {-12, -28, 12, 15}};
static const int kAnim17FrameIndices[] = {188, 189, 190, 189, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 210, 214, 215, 202};
static const int16 kAnim17FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim17FrameRects[] = {{-21, -61, 16, 52}, {-42, -76, 7, 14}, {-43, -75, 13, 24}, {-42, -76, 7, 14}, {-42, -75, 4, 42}, {-42, -76, 11, 57}, {-40, -74, 13, 55}, {-36, -74, 11, 55}, {-31, -72, 12, 56}, {-27, -71, 14, 57}, {-20, -69, 15, 55}, {-12, -65, 15, 51}, {-7, -57, 18, 44}, {-3, -43, 18, 29}, {4, -27, 20, 14}, {0, -28, 13, 14}, {0, -38, 14, 24}, {-1, -49, 19, 36}, {0, -61, 17, 47}, {-2, -63, 19, 49}, {-5, -64, 19, 50}, {-3, -62, 18, 48}, {0, -61, 19, 47}, {0, -61, 16, 47}, {-4, -48, 17, 34}, {-9, -37, 15, 23}, {-13, -26, 14, 12}, {0, -61, 16, 47}, {0, -50, 15, 36}, {0, -39, 13, 25}, {0, -28, 12, 14}};
static const int kAnim18FrameIndices[] = {216, 217, 218, 217, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 238, 242, 243, 230};
static const int16 kAnim18FrameTicks[] = {16, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 6, 10, 10, 6, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim18FrameRects[] = {{6, -61, 14, 52}, {35, -77, 7, 16}, {29, -76, 13, 24}, {35, -77, 7, 16}, {38, -76, 4, 43}, {32, -75, 10, 55}, {24, -74, 16, 54}, {22, -74, 14, 53}, {18, -72, 12, 55}, {12, -71, 15, 57}, {2, -69, 17, 55}, {-5, -65, 18, 51}, {-13, -57, 18, 43}, {-20, -43, 23, 29}, {-26, -30, 25, 16}, {-13, -28, 13, 14}, {-16, -38, 24, 24}, {-16, -49, 20, 35}, {-15, -61, 17, 47}, {-15, -63, 17, 49}, {-13, -64, 17, 51}, {-14, -62, 15, 48}, {-19, -61, 19, 47}, {-16, -61, 16, 48}, {-18, -48, 22, 34}, {-6, -37, 14, 23}, {0, -27, 12, 14}, {-16, -61, 16, 48}, {-16, -50, 19, 36}, {-14, -39, 15, 25}, {-12, -28, 12, 15}};
static const int kAnim19FrameIndices[] = {244};
static const int16 kAnim19FrameTicks[] = {6};
static const BBRect kAnim19FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim20FrameIndices[] = {245, 246, 247, 248, 249, 250, 251, 252, 253, 254};
static const int16 kAnim20FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim20FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim21FrameIndices[] = {255};
static const int16 kAnim21FrameTicks[] = {1};
static const BBRect kAnim21FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim22FrameIndices[] = {256};
static const int16 kAnim22FrameTicks[] = {5};
static const BBRect kAnim22FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim23FrameIndices[] = {257};
static const int16 kAnim23FrameTicks[] = {1};
static const BBRect kAnim23FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim24FrameIndices[] = {258, 259};
static const int16 kAnim24FrameTicks[] = {6, 6};
static const BBRect kAnim24FrameRects[] = {{-9, -9, 17, 15}, {-11, -10, 19, 16}};
static const int kAnim25FrameIndices[] = {260, 261, 262, 263, 264, 265};
static const int16 kAnim25FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim25FrameRects[] = {{-22, -91, 45, 93}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}};
static const int kAnim26FrameIndices[] = {266, 267, 268, 269, 270, 271};
static const int16 kAnim26FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim26FrameRects[] = {{-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}};
static const ObjAnimation kAnimations[] = {
	{4, kAnim0FrameIndices, kAnim0FrameTicks, kAnim0FrameRects},
	{6, kAnim1FrameIndices, kAnim1FrameTicks, kAnim1FrameRects},
	{4, kAnim2FrameIndices, kAnim2FrameTicks, kAnim2FrameRects},
	{5, kAnim3FrameIndices, kAnim3FrameTicks, kAnim3FrameRects},
	{2, kAnim4FrameIndices, kAnim4FrameTicks, kAnim4FrameRects},
	{1, kAnim5FrameIndices, kAnim5FrameTicks, kAnim5FrameRects},
	{9, kAnim6FrameIndices, kAnim6FrameTicks, kAnim6FrameRects},
	{1, kAnim7FrameIndices, kAnim7FrameTicks, kAnim7FrameRects},
	{1, kAnim8FrameIndices, kAnim8FrameTicks, kAnim8FrameRects},
	{1, kAnim9FrameIndices, kAnim9FrameTicks, kAnim9FrameRects},
	{1, kAnim10FrameIndices, kAnim10FrameTicks, kAnim10FrameRects},
	{34, kAnim11FrameIndices, kAnim11FrameTicks, kAnim11FrameRects},
	{14, kAnim12FrameIndices, kAnim12FrameTicks, kAnim12FrameRects},
	{31, kAnim13FrameIndices, kAnim13FrameTicks, kAnim13FrameRects},
	{31, kAnim14FrameIndices, kAnim14FrameTicks, kAnim14FrameRects},
	{31, kAnim15FrameIndices, kAnim15FrameTicks, kAnim15FrameRects},
	{31, kAnim16FrameIndices, kAnim16FrameTicks, kAnim16FrameRects},
	{31, kAnim17FrameIndices, kAnim17FrameTicks, kAnim17FrameRects},
	{31, kAnim18FrameIndices, kAnim18FrameTicks, kAnim18FrameRects},
	{1, kAnim19FrameIndices, kAnim19FrameTicks, kAnim19FrameRects},
	{10, kAnim20FrameIndices, kAnim20FrameTicks, kAnim20FrameRects},
	{1, kAnim21FrameIndices, kAnim21FrameTicks, kAnim21FrameRects},
	{1, kAnim22FrameIndices, kAnim22FrameTicks, kAnim22FrameRects},
	{1, kAnim23FrameIndices, kAnim23FrameTicks, kAnim23FrameRects},
	{2, kAnim24FrameIndices, kAnim24FrameTicks, kAnim24FrameRects},
	{6, kAnim25FrameIndices, kAnim25FrameTicks, kAnim25FrameRects},
	{6, kAnim26FrameIndices, kAnim26FrameTicks, kAnim26FrameRects}
};

const ObjAnimation *MinigameBbTennis::getAnimation(int animIndex) {
	return &kAnimations[animIndex];
}

} // End of namespace Bbvs
