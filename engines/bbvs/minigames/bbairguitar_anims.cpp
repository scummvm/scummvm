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

#include "bbvs/minigames/bbairguitar.h"

namespace Bbvs {

static const int kAnim0FrameIndices[] = {0, 1};
static const int16 kAnim0FrameTicks[] = {6, 6};
static const BBRect kAnim0FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim1FrameIndices[] = {2, 3};
static const int16 kAnim1FrameTicks[] = {6, 6};
static const BBRect kAnim1FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim2FrameIndices[] = {4, 5};
static const int16 kAnim2FrameTicks[] = {6, 6};
static const BBRect kAnim2FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim3FrameIndices[] = {6};
static const int16 kAnim3FrameTicks[] = {6};
static const BBRect kAnim3FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim4FrameIndices[] = {7};
static const int16 kAnim4FrameTicks[] = {6};
static const BBRect kAnim4FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim5FrameIndices[] = {8};
static const int16 kAnim5FrameTicks[] = {6};
static const BBRect kAnim5FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim6FrameIndices[] = {9, 10, 11};
static const int16 kAnim6FrameTicks[] = {6, 6, 6};
static const BBRect kAnim6FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim7FrameIndices[] = {12, 13, 14, 15};
static const int16 kAnim7FrameTicks[] = {10, 10, 10, 10};
static const BBRect kAnim7FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim8FrameIndices[] = {16};
static const int16 kAnim8FrameTicks[] = {10};
static const BBRect kAnim8FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim9FrameIndices[] = {17};
static const int16 kAnim9FrameTicks[] = {10};
static const BBRect kAnim9FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim10FrameIndices[] = {18};
static const int16 kAnim10FrameTicks[] = {6};
static const BBRect kAnim10FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim11FrameIndices[] = {19, 20, 21, 22, 23, 24, 25, 26, 27};
static const int16 kAnim11FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim11FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim12FrameIndices[] = {28, 29, 30, 31, 32, 33};
static const int16 kAnim12FrameTicks[] = {10, 10, 10, 10, 10, 10};
static const BBRect kAnim12FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim13FrameIndices[] = {34, 35};
static const int16 kAnim13FrameTicks[] = {6, 6};
static const BBRect kAnim13FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim14FrameIndices[] = {36, 37};
static const int16 kAnim14FrameTicks[] = {6, 6};
static const BBRect kAnim14FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim15FrameIndices[] = {38, 39};
static const int16 kAnim15FrameTicks[] = {6, 6};
static const BBRect kAnim15FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim16FrameIndices[] = {40, 41};
static const int16 kAnim16FrameTicks[] = {6, 6};
static const BBRect kAnim16FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim17FrameIndices[] = {42};
static const int16 kAnim17FrameTicks[] = {6};
static const BBRect kAnim17FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim18FrameIndices[] = {43};
static const int16 kAnim18FrameTicks[] = {6};
static const BBRect kAnim18FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim19FrameIndices[] = {44, 45};
static const int16 kAnim19FrameTicks[] = {6, 6};
static const BBRect kAnim19FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim20FrameIndices[] = {46, 47};
static const int16 kAnim20FrameTicks[] = {6, 6};
static const BBRect kAnim20FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim21FrameIndices[] = {48, 49};
static const int16 kAnim21FrameTicks[] = {6, 6};
static const BBRect kAnim21FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim22FrameIndices[] = {50, 51};
static const int16 kAnim22FrameTicks[] = {10, 10};
static const BBRect kAnim22FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim23FrameIndices[] = {52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64};
static const int16 kAnim23FrameTicks[] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
static const BBRect kAnim23FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim24FrameIndices[] = {65, 66, 67};
static const int16 kAnim24FrameTicks[] = {11, 16, 6};
static const BBRect kAnim24FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim25FrameIndices[] = {68, 67, 69, 67};
static const int16 kAnim25FrameTicks[] = {6, 6, 11, 6};
static const BBRect kAnim25FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim26FrameIndices[] = {70, 71, 72, 71};
static const int16 kAnim26FrameTicks[] = {6, 6, 6, 6};
static const BBRect kAnim26FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim27FrameIndices[] = {73, 74, 75, 74};
static const int16 kAnim27FrameTicks[] = {6, 6, 6, 6};
static const BBRect kAnim27FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim28FrameIndices[] = {76};
static const int16 kAnim28FrameTicks[] = {6};
static const BBRect kAnim28FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim29FrameIndices[] = {77, 78, 79, 78};
static const int16 kAnim29FrameTicks[] = {6, 6, 18, 6};
static const BBRect kAnim29FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim30FrameIndices[] = {77, 80, 81, 80};
static const int16 kAnim30FrameTicks[] = {6, 6, 10, 6};
static const BBRect kAnim30FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim31FrameIndices[] = {82, 83, 84, 83};
static const int16 kAnim31FrameTicks[] = {6, 6, 6, 6};
static const BBRect kAnim31FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim32FrameIndices[] = {85, 86, 87, 86};
static const int16 kAnim32FrameTicks[] = {6, 6, 6, 6};
static const BBRect kAnim32FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim33FrameIndices[] = {88};
static const int16 kAnim33FrameTicks[] = {6};
static const BBRect kAnim33FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim34FrameIndices[] = {89};
static const int16 kAnim34FrameTicks[] = {6};
static const BBRect kAnim34FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim35FrameIndices[] = {90};
static const int16 kAnim35FrameTicks[] = {6};
static const BBRect kAnim35FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim36FrameIndices[] = {91, 92, 93, 91, 93, 91, 92, 93, 92, 91, 92, 93, 91, 93, 91, 92, 93, 92};
static const int16 kAnim36FrameTicks[] = {10, 6, 8, 6, 6, 8, 6, 6, 6, 10, 6, 8, 6, 6, 8, 6, 6, 6};
static const BBRect kAnim36FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim37FrameIndices[] = {94, 95, 96, 94, 96, 94, 95, 96, 95, 94, 95, 96, 94, 96, 94, 95, 96, 95};
static const int16 kAnim37FrameTicks[] = {10, 6, 8, 6, 6, 8, 6, 6, 6, 10, 6, 8, 6, 6, 8, 6, 6, 6};
static const BBRect kAnim37FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const ObjAnimation kAnimations[] = {
	{2, kAnim0FrameIndices, kAnim0FrameTicks, kAnim0FrameRects},
	{2, kAnim1FrameIndices, kAnim1FrameTicks, kAnim1FrameRects},
	{2, kAnim2FrameIndices, kAnim2FrameTicks, kAnim2FrameRects},
	{1, kAnim3FrameIndices, kAnim3FrameTicks, kAnim3FrameRects},
	{1, kAnim4FrameIndices, kAnim4FrameTicks, kAnim4FrameRects},
	{1, kAnim5FrameIndices, kAnim5FrameTicks, kAnim5FrameRects},
	{3, kAnim6FrameIndices, kAnim6FrameTicks, kAnim6FrameRects},
	{4, kAnim7FrameIndices, kAnim7FrameTicks, kAnim7FrameRects},
	{1, kAnim8FrameIndices, kAnim8FrameTicks, kAnim8FrameRects},
	{1, kAnim9FrameIndices, kAnim9FrameTicks, kAnim9FrameRects},
	{1, kAnim10FrameIndices, kAnim10FrameTicks, kAnim10FrameRects},
	{9, kAnim11FrameIndices, kAnim11FrameTicks, kAnim11FrameRects},
	{6, kAnim12FrameIndices, kAnim12FrameTicks, kAnim12FrameRects},
	{2, kAnim13FrameIndices, kAnim13FrameTicks, kAnim13FrameRects},
	{2, kAnim14FrameIndices, kAnim14FrameTicks, kAnim14FrameRects},
	{2, kAnim15FrameIndices, kAnim15FrameTicks, kAnim15FrameRects},
	{2, kAnim16FrameIndices, kAnim16FrameTicks, kAnim16FrameRects},
	{1, kAnim17FrameIndices, kAnim17FrameTicks, kAnim17FrameRects},
	{1, kAnim18FrameIndices, kAnim18FrameTicks, kAnim18FrameRects},
	{2, kAnim19FrameIndices, kAnim19FrameTicks, kAnim19FrameRects},
	{2, kAnim20FrameIndices, kAnim20FrameTicks, kAnim20FrameRects},
	{2, kAnim21FrameIndices, kAnim21FrameTicks, kAnim21FrameRects},
	{2, kAnim22FrameIndices, kAnim22FrameTicks, kAnim22FrameRects},
	{13, kAnim23FrameIndices, kAnim23FrameTicks, kAnim23FrameRects},
	{3, kAnim24FrameIndices, kAnim24FrameTicks, kAnim24FrameRects},
	{4, kAnim25FrameIndices, kAnim25FrameTicks, kAnim25FrameRects},
	{4, kAnim26FrameIndices, kAnim26FrameTicks, kAnim26FrameRects},
	{4, kAnim27FrameIndices, kAnim27FrameTicks, kAnim27FrameRects},
	{1, kAnim28FrameIndices, kAnim28FrameTicks, kAnim28FrameRects},
	{4, kAnim29FrameIndices, kAnim29FrameTicks, kAnim29FrameRects},
	{4, kAnim30FrameIndices, kAnim30FrameTicks, kAnim30FrameRects},
	{4, kAnim31FrameIndices, kAnim31FrameTicks, kAnim31FrameRects},
	{4, kAnim32FrameIndices, kAnim32FrameTicks, kAnim32FrameRects},
	{1, kAnim33FrameIndices, kAnim33FrameTicks, kAnim33FrameRects},
	{1, kAnim34FrameIndices, kAnim34FrameTicks, kAnim34FrameRects},
	{1, kAnim35FrameIndices, kAnim35FrameTicks, kAnim35FrameRects},
	{18, kAnim36FrameIndices, kAnim36FrameTicks, kAnim36FrameRects},
	{18, kAnim37FrameIndices, kAnim37FrameTicks, kAnim37FrameRects}
};

const ObjAnimation *MinigameBbAirGuitar::getAnimation(int animIndex) {
	return &kAnimations[animIndex];
}

} // End of namespace Bbvs
