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

#include "bbvs/minigames/bbant.h"

namespace Bbvs {

static const int kAnim0FrameIndices[] = {0, 1, 2};
static const int16 kAnim0FrameTicks[] = {10, 8, 8};
static const BBRect kAnim0FrameRects[] = {{-3, -8, 6, 14}, {-3, -8, 6, 13}, {-3, -7, 6, 12}};
static const int kAnim1FrameIndices[] = {3, 4, 5};
static const int16 kAnim1FrameTicks[] = {10, 8, 8};
static const BBRect kAnim1FrameRects[] = {{-5, -6, 13, 9}, {-5, -6, 13, 10}, {-5, -6, 13, 10}};
static const int kAnim2FrameIndices[] = {6, 7, 8};
static const int16 kAnim2FrameTicks[] = {10, 8, 8};
static const BBRect kAnim2FrameRects[] = {{-6, -6, 17, 7}, {-6, -6, 15, 6}, {-7, -6, 17, 6}};
static const int kAnim3FrameIndices[] = {9, 10, 11};
static const int16 kAnim3FrameTicks[] = {10, 8, 8};
static const BBRect kAnim3FrameRects[] = {{-5, -7, 13, 8}, {-5, -7, 12, 7}, {-5, -7, 12, 9}};
static const int kAnim4FrameIndices[] = {12, 13, 14};
static const int16 kAnim4FrameTicks[] = {10, 8, 8};
static const BBRect kAnim4FrameRects[] = {{-3, -9, 7, 11}, {-3, -9, 7, 11}, {-3, -9, 7, 11}};
static const int kAnim5FrameIndices[] = {15, 16, 17};
static const int16 kAnim5FrameTicks[] = {10, 8, 8};
static const BBRect kAnim5FrameRects[] = {{-7, -8, 13, 9}, {-7, -7, 13, 8}, {-7, -7, 13, 8}};
static const int kAnim6FrameIndices[] = {18, 19, 20};
static const int16 kAnim6FrameTicks[] = {10, 8, 8};
static const BBRect kAnim6FrameRects[] = {{-10, -6, 17, 7}, {-11, -6, 18, 7}, {-11, -6, 18, 6}};
static const int kAnim7FrameIndices[] = {21, 22, 23};
static const int16 kAnim7FrameTicks[] = {10, 8, 8};
static const BBRect kAnim7FrameRects[] = {{-7, -6, 13, 8}, {-7, -7, 12, 9}, {-7, -7, 13, 9}};
static const int kAnim8FrameIndices[] = {24};
static const int16 kAnim8FrameTicks[] = {8};
static const BBRect kAnim8FrameRects[] = {{-3, -9, 6, 12}};
static const int kAnim9FrameIndices[] = {25};
static const int16 kAnim9FrameTicks[] = {8};
static const BBRect kAnim9FrameRects[] = {{-5, -6, 12, 7}};
static const int kAnim10FrameIndices[] = {26};
static const int16 kAnim10FrameTicks[] = {8};
static const BBRect kAnim10FrameRects[] = {{-4, -6, 13, 6}};
static const int kAnim11FrameIndices[] = {27};
static const int16 kAnim11FrameTicks[] = {8};
static const BBRect kAnim11FrameRects[] = {{-5, -7, 11, 8}};
static const int kAnim12FrameIndices[] = {28};
static const int16 kAnim12FrameTicks[] = {8};
static const BBRect kAnim12FrameRects[] = {{-2, -10, 5, 12}};
static const int kAnim13FrameIndices[] = {29};
static const int16 kAnim13FrameTicks[] = {8};
static const BBRect kAnim13FrameRects[] = {{-6, -8, 13, 9}};
static const int kAnim14FrameIndices[] = {30};
static const int16 kAnim14FrameTicks[] = {8};
static const BBRect kAnim14FrameRects[] = {{-8, -6, 13, 6}};
static const int kAnim15FrameIndices[] = {31};
static const int16 kAnim15FrameTicks[] = {8};
static const BBRect kAnim15FrameRects[] = {{-7, -7, 12, 8}};
static const int kAnim16FrameIndices[] = {0, 1, 2};
static const int16 kAnim16FrameTicks[] = {6, 6, 6};
static const BBRect kAnim16FrameRects[] = {{-3, -8, 6, 14}, {-3, -8, 6, 13}, {-3, -7, 6, 12}};
static const int kAnim17FrameIndices[] = {3, 4, 5};
static const int16 kAnim17FrameTicks[] = {6, 6, 6};
static const BBRect kAnim17FrameRects[] = {{-5, -6, 13, 9}, {-5, -6, 13, 10}, {-5, -6, 13, 10}};
static const int kAnim18FrameIndices[] = {6, 7, 8};
static const int16 kAnim18FrameTicks[] = {6, 6, 6};
static const BBRect kAnim18FrameRects[] = {{-6, -6, 17, 7}, {-6, -6, 15, 6}, {-7, -6, 17, 6}};
static const int kAnim19FrameIndices[] = {9, 10, 11};
static const int16 kAnim19FrameTicks[] = {6, 6, 6};
static const BBRect kAnim19FrameRects[] = {{-5, -7, 13, 8}, {-5, -7, 12, 7}, {-5, -7, 12, 9}};
static const int kAnim20FrameIndices[] = {12, 13, 14};
static const int16 kAnim20FrameTicks[] = {6, 6, 6};
static const BBRect kAnim20FrameRects[] = {{-3, -9, 7, 11}, {-3, -9, 7, 11}, {-3, -9, 7, 11}};
static const int kAnim21FrameIndices[] = {15, 16, 17};
static const int16 kAnim21FrameTicks[] = {6, 6, 6};
static const BBRect kAnim21FrameRects[] = {{-7, -8, 13, 9}, {-7, -7, 13, 8}, {-7, -7, 13, 8}};
static const int kAnim22FrameIndices[] = {18, 19, 20};
static const int16 kAnim22FrameTicks[] = {6, 6, 6};
static const BBRect kAnim22FrameRects[] = {{-10, -6, 17, 7}, {-11, -6, 18, 7}, {-11, -6, 18, 6}};
static const int kAnim23FrameIndices[] = {21, 22, 23};
static const int16 kAnim23FrameTicks[] = {6, 6, 6};
static const BBRect kAnim23FrameRects[] = {{-7, -6, 13, 8}, {-7, -7, 12, 9}, {-7, -7, 13, 9}};
static const int kAnim24FrameIndices[] = {32, 33, 34, 35, 36, 37, 36, 37, 36, 37, 36, 37, 36, 38};
static const int16 kAnim24FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim24FrameRects[] = {{-3, -14, 12, 10}, {-2, -21, 11, 11}, {0, -23, 8, 14}, {-6, -15, 13, 11}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 5}};
static const int kAnim25FrameIndices[] = {39, 40, 41, 42, 43, 44, 43, 44, 43, 44, 43, 44, 43, 45};
static const int16 kAnim25FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim25FrameRects[] = {{-9, -14, 13, 10}, {-8, -22, 12, 12}, {-8, -24, 8, 15}, {-7, -15, 13, 10}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}};
static const int kAnim26FrameIndices[] = {46, 47, 48};
static const int16 kAnim26FrameTicks[] = {10, 8, 8};
static const BBRect kAnim26FrameRects[] = {{-3, -8, 6, 14}, {-3, -8, 6, 13}, {-3, -7, 6, 12}};
static const int kAnim27FrameIndices[] = {49, 50, 51};
static const int16 kAnim27FrameTicks[] = {10, 8, 8};
static const BBRect kAnim27FrameRects[] = {{-5, -6, 13, 9}, {-5, -6, 13, 10}, {-5, -6, 13, 10}};
static const int kAnim28FrameIndices[] = {52, 53, 54};
static const int16 kAnim28FrameTicks[] = {10, 8, 8};
static const BBRect kAnim28FrameRects[] = {{-6, -6, 17, 7}, {-6, -6, 15, 6}, {-7, -6, 17, 6}};
static const int kAnim29FrameIndices[] = {55, 56, 57};
static const int16 kAnim29FrameTicks[] = {10, 8, 8};
static const BBRect kAnim29FrameRects[] = {{-5, -7, 13, 8}, {-5, -7, 12, 7}, {-5, -7, 12, 9}};
static const int kAnim30FrameIndices[] = {58, 59, 60};
static const int16 kAnim30FrameTicks[] = {10, 8, 8};
static const BBRect kAnim30FrameRects[] = {{-3, -9, 7, 11}, {-3, -9, 7, 11}, {-3, -9, 7, 11}};
static const int kAnim31FrameIndices[] = {61, 62, 63};
static const int16 kAnim31FrameTicks[] = {10, 8, 8};
static const BBRect kAnim31FrameRects[] = {{-7, -8, 13, 9}, {-7, -7, 13, 8}, {-7, -7, 13, 8}};
static const int kAnim32FrameIndices[] = {64, 65, 66};
static const int16 kAnim32FrameTicks[] = {10, 8, 8};
static const BBRect kAnim32FrameRects[] = {{-10, -6, 17, 7}, {-11, -6, 18, 7}, {-11, -6, 18, 6}};
static const int kAnim33FrameIndices[] = {67, 68, 69};
static const int16 kAnim33FrameTicks[] = {10, 8, 8};
static const BBRect kAnim33FrameRects[] = {{-7, -6, 13, 8}, {-7, -7, 12, 9}, {-7, -7, 13, 9}};
static const int kAnim34FrameIndices[] = {70};
static const int16 kAnim34FrameTicks[] = {8};
static const BBRect kAnim34FrameRects[] = {{-3, -9, 6, 12}};
static const int kAnim35FrameIndices[] = {71};
static const int16 kAnim35FrameTicks[] = {8};
static const BBRect kAnim35FrameRects[] = {{-5, -6, 12, 7}};
static const int kAnim36FrameIndices[] = {72};
static const int16 kAnim36FrameTicks[] = {8};
static const BBRect kAnim36FrameRects[] = {{-4, -6, 13, 6}};
static const int kAnim37FrameIndices[] = {73};
static const int16 kAnim37FrameTicks[] = {8};
static const BBRect kAnim37FrameRects[] = {{-5, -7, 11, 8}};
static const int kAnim38FrameIndices[] = {74};
static const int16 kAnim38FrameTicks[] = {8};
static const BBRect kAnim38FrameRects[] = {{-2, -10, 5, 12}};
static const int kAnim39FrameIndices[] = {75};
static const int16 kAnim39FrameTicks[] = {8};
static const BBRect kAnim39FrameRects[] = {{-6, -8, 13, 9}};
static const int kAnim40FrameIndices[] = {76};
static const int16 kAnim40FrameTicks[] = {8};
static const BBRect kAnim40FrameRects[] = {{-8, -6, 13, 6}};
static const int kAnim41FrameIndices[] = {77};
static const int16 kAnim41FrameTicks[] = {8};
static const BBRect kAnim41FrameRects[] = {{-7, -7, 12, 8}};
static const int kAnim42FrameIndices[] = {46, 47, 48};
static const int16 kAnim42FrameTicks[] = {6, 6, 6};
static const BBRect kAnim42FrameRects[] = {{-3, -8, 6, 14}, {-3, -8, 6, 13}, {-3, -7, 6, 12}};
static const int kAnim43FrameIndices[] = {49, 50, 51};
static const int16 kAnim43FrameTicks[] = {6, 6, 6};
static const BBRect kAnim43FrameRects[] = {{-5, -6, 13, 9}, {-5, -6, 13, 10}, {-5, -6, 13, 10}};
static const int kAnim44FrameIndices[] = {52, 53, 54};
static const int16 kAnim44FrameTicks[] = {6, 6, 6};
static const BBRect kAnim44FrameRects[] = {{-6, -6, 17, 7}, {-6, -6, 15, 6}, {-7, -6, 17, 6}};
static const int kAnim45FrameIndices[] = {55, 56, 57};
static const int16 kAnim45FrameTicks[] = {6, 6, 6};
static const BBRect kAnim45FrameRects[] = {{-5, -7, 13, 8}, {-5, -7, 12, 7}, {-5, -7, 12, 9}};
static const int kAnim46FrameIndices[] = {58, 59, 60};
static const int16 kAnim46FrameTicks[] = {6, 6, 6};
static const BBRect kAnim46FrameRects[] = {{-3, -9, 7, 11}, {-3, -9, 7, 11}, {-3, -9, 7, 11}};
static const int kAnim47FrameIndices[] = {61, 62, 63};
static const int16 kAnim47FrameTicks[] = {6, 6, 6};
static const BBRect kAnim47FrameRects[] = {{-7, -8, 13, 9}, {-7, -7, 13, 8}, {-7, -7, 13, 8}};
static const int kAnim48FrameIndices[] = {64, 65, 66};
static const int16 kAnim48FrameTicks[] = {6, 6, 6};
static const BBRect kAnim48FrameRects[] = {{-10, -6, 17, 7}, {-11, -6, 18, 7}, {-11, -6, 18, 6}};
static const int kAnim49FrameIndices[] = {67, 68, 69};
static const int16 kAnim49FrameTicks[] = {6, 6, 6};
static const BBRect kAnim49FrameRects[] = {{-7, -6, 13, 8}, {-7, -7, 12, 9}, {-7, -7, 13, 9}};
static const int kAnim50FrameIndices[] = {78, 79, 80, 81, 82, 83, 82, 83, 82, 83, 82, 83, 82, 84};
static const int16 kAnim50FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim50FrameRects[] = {{-3, -14, 12, 10}, {-2, -21, 11, 11}, {0, -23, 8, 14}, {-6, -15, 13, 11}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 6}, {-8, -4, 15, 5}, {-9, -4, 16, 5}};
static const int kAnim51FrameIndices[] = {85, 86, 87, 88, 89, 90, 89, 90, 89, 90, 89, 90, 89, 91};
static const int16 kAnim51FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim51FrameRects[] = {{-9, -14, 13, 10}, {-8, -22, 12, 12}, {-8, -24, 8, 15}, {-7, -15, 13, 10}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}, {-6, -4, 15, 7}, {-7, -4, 16, 6}};
static const int kAnim52FrameIndices[] = {92, 93, 94};
static const int16 kAnim52FrameTicks[] = {10, 8, 8};
static const BBRect kAnim52FrameRects[] = {{-6, -14, 13, 24}, {-7, -13, 14, 23}, {-6, -13, 12, 22}};
static const int kAnim53FrameIndices[] = {95, 96, 97};
static const int16 kAnim53FrameTicks[] = {10, 8, 8};
static const BBRect kAnim53FrameRects[] = {{-4, -12, 19, 17}, {-3, -12, 18, 18}, {-2, -12, 17, 18}};
static const int kAnim54FrameIndices[] = {98, 99, 100};
static const int16 kAnim54FrameTicks[] = {10, 8, 8};
static const BBRect kAnim54FrameRects[] = {{-6, -16, 23, 14}, {-6, -15, 24, 13}, {-7, -15, 25, 14}};
static const int kAnim55FrameIndices[] = {101, 102, 103};
static const int16 kAnim55FrameTicks[] = {10, 8, 8};
static const BBRect kAnim55FrameRects[] = {{-4, -22, 16, 20}, {-3, -23, 14, 22}, {-4, -23, 14, 22}};
static const int kAnim56FrameIndices[] = {104, 105, 106};
static const int16 kAnim56FrameTicks[] = {10, 8, 8};
static const BBRect kAnim56FrameRects[] = {{-5, -24, 11, 23}, {-5, -25, 11, 25}, {-5, -25, 11, 26}};
static const int kAnim57FrameIndices[] = {107, 108, 109};
static const int16 kAnim57FrameTicks[] = {10, 8, 8};
static const BBRect kAnim57FrameRects[] = {{-10, -23, 15, 21}, {-11, -22, 16, 20}, {-11, -23, 17, 21}};
static const int kAnim58FrameIndices[] = {110, 111, 112};
static const int16 kAnim58FrameTicks[] = {10, 8, 8};
static const BBRect kAnim58FrameRects[] = {{-17, -15, 25, 15}, {-17, -15, 25, 14}, {-17, -15, 25, 14}};
static const int kAnim59FrameIndices[] = {113, 114, 115};
static const int16 kAnim59FrameTicks[] = {10, 8, 8};
static const BBRect kAnim59FrameRects[] = {{-14, -12, 20, 17}, {-14, -13, 19, 18}, {-14, -13, 19, 18}};
static const int kAnim60FrameIndices[] = {116};
static const int16 kAnim60FrameTicks[] = {6};
static const BBRect kAnim60FrameRects[] = {{-6, -12, 12, 23}};
static const int kAnim61FrameIndices[] = {117};
static const int16 kAnim61FrameTicks[] = {6};
static const BBRect kAnim61FrameRects[] = {{-5, -11, 20, 19}};
static const int kAnim62FrameIndices[] = {118};
static const int16 kAnim62FrameTicks[] = {6};
static const BBRect kAnim62FrameRects[] = {{-8, -14, 27, 15}};
static const int kAnim63FrameIndices[] = {119};
static const int16 kAnim63FrameTicks[] = {6};
static const BBRect kAnim63FrameRects[] = {{-4, -22, 17, 20}};
static const int kAnim64FrameIndices[] = {120};
static const int16 kAnim64FrameTicks[] = {6};
static const BBRect kAnim64FrameRects[] = {{-6, -25, 13, 25}};
static const int kAnim65FrameIndices[] = {121};
static const int16 kAnim65FrameTicks[] = {6};
static const BBRect kAnim65FrameRects[] = {{-11, -23, 17, 23}};
static const int kAnim66FrameIndices[] = {122};
static const int16 kAnim66FrameTicks[] = {6};
static const BBRect kAnim66FrameRects[] = {{-18, -13, 29, 13}};
static const int kAnim67FrameIndices[] = {123};
static const int16 kAnim67FrameTicks[] = {6};
static const BBRect kAnim67FrameRects[] = {{-14, -12, 21, 19}};
static const int kAnim68FrameIndices[] = {92, 93, 94};
static const int16 kAnim68FrameTicks[] = {6, 6, 6};
static const BBRect kAnim68FrameRects[] = {{-6, -14, 13, 24}, {-7, -13, 14, 23}, {-6, -13, 12, 22}};
static const int kAnim69FrameIndices[] = {95, 96, 97};
static const int16 kAnim69FrameTicks[] = {6, 6, 6};
static const BBRect kAnim69FrameRects[] = {{-4, -12, 19, 17}, {-3, -12, 18, 18}, {-2, -12, 17, 18}};
static const int kAnim70FrameIndices[] = {98, 99, 100};
static const int16 kAnim70FrameTicks[] = {6, 6, 6};
static const BBRect kAnim70FrameRects[] = {{-6, -16, 23, 14}, {-6, -15, 24, 13}, {-7, -15, 25, 14}};
static const int kAnim71FrameIndices[] = {101, 102, 103};
static const int16 kAnim71FrameTicks[] = {6, 6, 6};
static const BBRect kAnim71FrameRects[] = {{-4, -22, 16, 20}, {-3, -23, 14, 22}, {-4, -23, 14, 22}};
static const int kAnim72FrameIndices[] = {104, 105, 106};
static const int16 kAnim72FrameTicks[] = {6, 6, 6};
static const BBRect kAnim72FrameRects[] = {{-5, -24, 11, 23}, {-5, -25, 11, 25}, {-5, -25, 11, 26}};
static const int kAnim73FrameIndices[] = {107, 108, 109};
static const int16 kAnim73FrameTicks[] = {6, 6, 6};
static const BBRect kAnim73FrameRects[] = {{-10, -23, 15, 21}, {-11, -22, 16, 20}, {-11, -23, 17, 21}};
static const int kAnim74FrameIndices[] = {110, 111, 112};
static const int16 kAnim74FrameTicks[] = {6, 6, 6};
static const BBRect kAnim74FrameRects[] = {{-17, -15, 25, 15}, {-17, -15, 25, 14}, {-17, -15, 25, 14}};
static const int kAnim75FrameIndices[] = {113, 114, 115};
static const int16 kAnim75FrameTicks[] = {6, 6, 6};
static const BBRect kAnim75FrameRects[] = {{-14, -12, 20, 17}, {-14, -13, 19, 18}, {-14, -13, 19, 18}};
static const int kAnim76FrameIndices[] = {124, 125, 126, 127, 128, 129, 128, 129, 128, 129, 128, 129, 128, 130};
static const int16 kAnim76FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim76FrameRects[] = {{-14, -23, 23, 18}, {-12, -32, 18, 23}, {-16, -29, 18, 22}, {-17, -17, 23, 17}, {-17, -10, 26, 14}, {-17, -12, 25, 15}, {-17, -10, 26, 14}, {-17, -12, 25, 15}, {-17, -10, 26, 14}, {-17, -12, 25, 15}, {-17, -10, 26, 14}, {-17, -12, 25, 15}, {-17, -10, 26, 14}, {-18, -13, 28, 14}};
static const int kAnim77FrameIndices[] = {131, 132, 133, 134, 135, 136, 135, 136, 135, 136, 135, 136, 135, 137};
static const int16 kAnim77FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim77FrameRects[] = {{-6, -24, 21, 19}, {-5, -33, 19, 24}, {-1, -29, 18, 22}, {-5, -17, 22, 17}, {-6, -10, 23, 14}, {-7, -10, 26, 13}, {-6, -10, 23, 14}, {-7, -10, 26, 13}, {-6, -10, 23, 14}, {-7, -10, 26, 13}, {-6, -10, 23, 14}, {-7, -10, 26, 13}, {-6, -10, 23, 14}, {-7, -12, 26, 14}};
static const int kAnim78FrameIndices[] = {138, 139, 140};
static const int16 kAnim78FrameTicks[] = {10, 8, 8};
static const BBRect kAnim78FrameRects[] = {{-3, -17, 7, 20}, {-3, -16, 7, 19}, {-3, -16, 7, 19}};
static const int kAnim79FrameIndices[] = {141, 142, 143};
static const int16 kAnim79FrameTicks[] = {10, 8, 8};
static const BBRect kAnim79FrameRects[] = {{-6, -14, 13, 15}, {-7, -13, 14, 14}, {-6, -13, 13, 14}};
static const int kAnim80FrameIndices[] = {144, 145, 146};
static const int16 kAnim80FrameTicks[] = {10, 8, 8};
static const BBRect kAnim80FrameRects[] = {{-10, -10, 20, 9}, {-9, -9, 19, 8}, {-9, -9, 19, 8}};
static const int kAnim81FrameIndices[] = {147, 148, 149};
static const int16 kAnim81FrameTicks[] = {10, 8, 8};
static const BBRect kAnim81FrameRects[] = {{-7, -11, 16, 10}, {-7, -11, 16, 10}, {-7, -11, 16, 10}};
static const int kAnim82FrameIndices[] = {150, 151, 152};
static const int16 kAnim82FrameTicks[] = {10, 8, 8};
static const BBRect kAnim82FrameRects[] = {{-3, -13, 7, 16}, {-3, -13, 7, 16}, {-3, -12, 7, 15}};
static const int kAnim83FrameIndices[] = {153, 154, 155};
static const int16 kAnim83FrameTicks[] = {10, 8, 8};
static const BBRect kAnim83FrameRects[] = {{-8, -11, 18, 10}, {-7, -11, 16, 11}, {-7, -10, 17, 9}};
static const int kAnim84FrameIndices[] = {156, 157, 158};
static const int16 kAnim84FrameTicks[] = {10, 8, 8};
static const BBRect kAnim84FrameRects[] = {{-8, -9, 20, 7}, {-9, -9, 21, 8}, {-9, -9, 21, 8}};
static const int kAnim85FrameIndices[] = {159, 160, 161};
static const int16 kAnim85FrameTicks[] = {10, 8, 8};
static const BBRect kAnim85FrameRects[] = {{-6, -14, 15, 15}, {-5, -13, 12, 14}, {-6, -13, 14, 14}};
static const int kAnim86FrameIndices[] = {162};
static const int16 kAnim86FrameTicks[] = {6};
static const BBRect kAnim86FrameRects[] = {{-3, -15, 8, 18}};
static const int kAnim87FrameIndices[] = {163};
static const int16 kAnim87FrameTicks[] = {6};
static const BBRect kAnim87FrameRects[] = {{-7, -13, 14, 14}};
static const int kAnim88FrameIndices[] = {164};
static const int16 kAnim88FrameTicks[] = {6};
static const BBRect kAnim88FrameRects[] = {{-11, -9, 21, 8}};
static const int kAnim89FrameIndices[] = {165};
static const int16 kAnim89FrameTicks[] = {6};
static const BBRect kAnim89FrameRects[] = {{-9, -11, 18, 11}};
static const int kAnim90FrameIndices[] = {166};
static const int16 kAnim90FrameTicks[] = {6};
static const BBRect kAnim90FrameRects[] = {{-3, -12, 7, 15}};
static const int kAnim91FrameIndices[] = {167};
static const int16 kAnim91FrameTicks[] = {6};
static const BBRect kAnim91FrameRects[] = {{-8, -11, 17, 12}};
static const int kAnim92FrameIndices[] = {168};
static const int16 kAnim92FrameTicks[] = {6};
static const BBRect kAnim92FrameRects[] = {{-9, -10, 21, 9}};
static const int kAnim93FrameIndices[] = {169};
static const int16 kAnim93FrameTicks[] = {6};
static const BBRect kAnim93FrameRects[] = {{-6, -14, 14, 15}};
static const int kAnim94FrameIndices[] = {138, 139, 140};
static const int16 kAnim94FrameTicks[] = {6, 6, 6};
static const BBRect kAnim94FrameRects[] = {{-3, -17, 7, 20}, {-3, -16, 7, 19}, {-3, -16, 7, 19}};
static const int kAnim95FrameIndices[] = {141, 142, 143};
static const int16 kAnim95FrameTicks[] = {6, 6, 6};
static const BBRect kAnim95FrameRects[] = {{-6, -14, 13, 15}, {-7, -13, 14, 14}, {-6, -13, 13, 14}};
static const int kAnim96FrameIndices[] = {144, 145, 146};
static const int16 kAnim96FrameTicks[] = {6, 6, 6};
static const BBRect kAnim96FrameRects[] = {{-10, -10, 20, 9}, {-9, -9, 19, 8}, {-9, -9, 19, 8}};
static const int kAnim97FrameIndices[] = {147, 148, 149};
static const int16 kAnim97FrameTicks[] = {6, 6, 6};
static const BBRect kAnim97FrameRects[] = {{-7, -11, 16, 10}, {-7, -11, 16, 10}, {-7, -11, 16, 10}};
static const int kAnim98FrameIndices[] = {150, 151, 152};
static const int16 kAnim98FrameTicks[] = {6, 6, 6};
static const BBRect kAnim98FrameRects[] = {{-3, -13, 7, 16}, {-3, -13, 7, 16}, {-3, -12, 7, 15}};
static const int kAnim99FrameIndices[] = {153, 154, 155};
static const int16 kAnim99FrameTicks[] = {6, 6, 6};
static const BBRect kAnim99FrameRects[] = {{-8, -11, 18, 10}, {-7, -11, 16, 11}, {-7, -10, 17, 9}};
static const int kAnim100FrameIndices[] = {156, 157, 158};
static const int16 kAnim100FrameTicks[] = {6, 6, 6};
static const BBRect kAnim100FrameRects[] = {{-8, -9, 20, 7}, {-9, -9, 21, 8}, {-9, -9, 21, 8}};
static const int kAnim101FrameIndices[] = {159, 160, 161};
static const int16 kAnim101FrameTicks[] = {6, 6, 6};
static const BBRect kAnim101FrameRects[] = {{-6, -14, 15, 15}, {-5, -13, 12, 14}, {-6, -13, 14, 14}};
static const int kAnim102FrameIndices[] = {170, 171, 172, 173, 174, 175, 174, 175, 174, 175, 174, 175, 174, 176};
static const int16 kAnim102FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim102FrameRects[] = {{-7, -18, 15, 14}, {-6, -24, 11, 18}, {-6, -24, 9, 17}, {-5, -14, 16, 11}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}};
static const int kAnim103FrameIndices[] = {177, 178, 179, 180, 181, 182, 181, 182, 181, 182, 181, 182, 181, 183};
static const int16 kAnim103FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim103FrameRects[] = {{-9, -18, 16, 15}, {-6, -24, 12, 18}, {-6, -24, 13, 16}, {-12, -15, 17, 13}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -6, 21, 6}};
static const int kAnim104FrameIndices[] = {184, 185, 186};
static const int16 kAnim104FrameTicks[] = {10, 8, 8};
static const BBRect kAnim104FrameRects[] = {{-3, -17, 7, 20}, {-3, -16, 7, 19}, {-3, -16, 7, 19}};
static const int kAnim105FrameIndices[] = {187, 188, 189};
static const int16 kAnim105FrameTicks[] = {10, 8, 8};
static const BBRect kAnim105FrameRects[] = {{-6, -14, 13, 15}, {-7, -13, 14, 14}, {-6, -13, 13, 14}};
static const int kAnim106FrameIndices[] = {190, 191, 192};
static const int16 kAnim106FrameTicks[] = {10, 8, 8};
static const BBRect kAnim106FrameRects[] = {{-10, -10, 20, 9}, {-9, -9, 19, 8}, {-9, -9, 19, 8}};
static const int kAnim107FrameIndices[] = {193, 194, 195};
static const int16 kAnim107FrameTicks[] = {10, 8, 8};
static const BBRect kAnim107FrameRects[] = {{-7, -11, 16, 10}, {-7, -11, 16, 10}, {-7, -11, 16, 10}};
static const int kAnim108FrameIndices[] = {196, 197, 198};
static const int16 kAnim108FrameTicks[] = {10, 8, 8};
static const BBRect kAnim108FrameRects[] = {{-3, -13, 7, 16}, {-3, -13, 7, 16}, {-3, -12, 7, 15}};
static const int kAnim109FrameIndices[] = {199, 200, 201};
static const int16 kAnim109FrameTicks[] = {10, 8, 8};
static const BBRect kAnim109FrameRects[] = {{-8, -11, 18, 10}, {-7, -11, 16, 11}, {-7, -10, 17, 9}};
static const int kAnim110FrameIndices[] = {202, 203, 204};
static const int16 kAnim110FrameTicks[] = {10, 8, 8};
static const BBRect kAnim110FrameRects[] = {{-8, -9, 20, 7}, {-9, -9, 21, 8}, {-9, -9, 21, 8}};
static const int kAnim111FrameIndices[] = {205, 206, 207};
static const int16 kAnim111FrameTicks[] = {10, 8, 8};
static const BBRect kAnim111FrameRects[] = {{-6, -14, 15, 15}, {-5, -13, 12, 14}, {-6, -13, 14, 14}};
static const int kAnim112FrameIndices[] = {208};
static const int16 kAnim112FrameTicks[] = {6};
static const BBRect kAnim112FrameRects[] = {{-3, -15, 8, 18}};
static const int kAnim113FrameIndices[] = {209};
static const int16 kAnim113FrameTicks[] = {6};
static const BBRect kAnim113FrameRects[] = {{-7, -13, 14, 14}};
static const int kAnim114FrameIndices[] = {210};
static const int16 kAnim114FrameTicks[] = {6};
static const BBRect kAnim114FrameRects[] = {{-11, -9, 21, 8}};
static const int kAnim115FrameIndices[] = {211};
static const int16 kAnim115FrameTicks[] = {6};
static const BBRect kAnim115FrameRects[] = {{-9, -11, 18, 11}};
static const int kAnim116FrameIndices[] = {212};
static const int16 kAnim116FrameTicks[] = {6};
static const BBRect kAnim116FrameRects[] = {{-3, -12, 7, 15}};
static const int kAnim117FrameIndices[] = {213};
static const int16 kAnim117FrameTicks[] = {6};
static const BBRect kAnim117FrameRects[] = {{-8, -11, 17, 12}};
static const int kAnim118FrameIndices[] = {214};
static const int16 kAnim118FrameTicks[] = {6};
static const BBRect kAnim118FrameRects[] = {{-9, -10, 21, 9}};
static const int kAnim119FrameIndices[] = {215};
static const int16 kAnim119FrameTicks[] = {6};
static const BBRect kAnim119FrameRects[] = {{-6, -14, 14, 15}};
static const int kAnim120FrameIndices[] = {184, 185, 186};
static const int16 kAnim120FrameTicks[] = {6, 6, 6};
static const BBRect kAnim120FrameRects[] = {{-3, -17, 7, 20}, {-3, -16, 7, 19}, {-3, -16, 7, 19}};
static const int kAnim121FrameIndices[] = {187, 188, 189};
static const int16 kAnim121FrameTicks[] = {6, 6, 6};
static const BBRect kAnim121FrameRects[] = {{-6, -14, 13, 15}, {-7, -13, 14, 14}, {-6, -13, 13, 14}};
static const int kAnim122FrameIndices[] = {190, 191, 192};
static const int16 kAnim122FrameTicks[] = {6, 6, 6};
static const BBRect kAnim122FrameRects[] = {{-10, -10, 20, 9}, {-9, -9, 19, 8}, {-9, -9, 19, 8}};
static const int kAnim123FrameIndices[] = {193, 194, 195};
static const int16 kAnim123FrameTicks[] = {6, 6, 6};
static const BBRect kAnim123FrameRects[] = {{-7, -11, 16, 10}, {-7, -11, 16, 10}, {-7, -11, 16, 10}};
static const int kAnim124FrameIndices[] = {196, 197, 198};
static const int16 kAnim124FrameTicks[] = {6, 6, 6};
static const BBRect kAnim124FrameRects[] = {{-3, -13, 7, 16}, {-3, -13, 7, 16}, {-3, -12, 7, 15}};
static const int kAnim125FrameIndices[] = {199, 200, 201};
static const int16 kAnim125FrameTicks[] = {6, 6, 6};
static const BBRect kAnim125FrameRects[] = {{-8, -11, 18, 10}, {-7, -11, 16, 11}, {-7, -10, 17, 9}};
static const int kAnim126FrameIndices[] = {202, 203, 204};
static const int16 kAnim126FrameTicks[] = {6, 6, 6};
static const BBRect kAnim126FrameRects[] = {{-8, -9, 20, 7}, {-9, -9, 21, 8}, {-9, -9, 21, 8}};
static const int kAnim127FrameIndices[] = {205, 206, 207};
static const int16 kAnim127FrameTicks[] = {6, 6, 6};
static const BBRect kAnim127FrameRects[] = {{-6, -14, 15, 15}, {-5, -13, 12, 14}, {-6, -13, 14, 14}};
static const int kAnim128FrameIndices[] = {216, 217, 218, 219, 220, 221, 220, 221, 220, 221, 220, 221, 220, 222};
static const int16 kAnim128FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim128FrameRects[] = {{-7, -18, 15, 14}, {-6, -24, 11, 18}, {-6, -24, 9, 17}, {-5, -14, 16, 11}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}, {-7, -6, 18, 7}, {-8, -7, 19, 8}};
static const int kAnim129FrameIndices[] = {223, 224, 225, 226, 227, 228, 227, 228, 227, 228, 227, 228, 227, 229};
static const int16 kAnim129FrameTicks[] = {6, 6, 6, 6, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};
static const BBRect kAnim129FrameRects[] = {{-9, -18, 16, 15}, {-6, -24, 12, 18}, {-6, -24, 13, 16}, {-12, -15, 17, 13}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -7, 21, 9}, {-10, -7, 19, 8}, {-11, -6, 21, 6}};
static const int kAnim130FrameIndices[] = {230};
static const int16 kAnim130FrameTicks[] = {6};
static const BBRect kAnim130FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim131FrameIndices[] = {231};
static const int16 kAnim131FrameTicks[] = {6};
static const BBRect kAnim131FrameRects[] = {{-8, -9, 16, 12}};
static const int kAnim132FrameIndices[] = {231, 232, 233};
static const int16 kAnim132FrameTicks[] = {6, 6, 6};
static const BBRect kAnim132FrameRects[] = {{-8, -9, 16, 12}, {-8, -11, 16, 12}, {-8, -13, 16, 12}};
static const int kAnim133FrameIndices[] = {233};
static const int16 kAnim133FrameTicks[] = {6};
static const BBRect kAnim133FrameRects[] = {{-8, -13, 16, 12}};
static const int kAnim134FrameIndices[] = {234};
static const int16 kAnim134FrameTicks[] = {6};
static const BBRect kAnim134FrameRects[] = {{-7, -6, 14, 10}};
static const int kAnim135FrameIndices[] = {234, 235, 236};
static const int16 kAnim135FrameTicks[] = {6, 6, 6};
static const BBRect kAnim135FrameRects[] = {{-7, -6, 14, 10}, {-7, -9, 14, 9}, {-7, -12, 14, 9}};
static const int kAnim136FrameIndices[] = {236};
static const int16 kAnim136FrameTicks[] = {6};
static const BBRect kAnim136FrameRects[] = {{-7, -12, 14, 9}};
static const int kAnim137FrameIndices[] = {237};
static const int16 kAnim137FrameTicks[] = {6};
static const BBRect kAnim137FrameRects[] = {{-7, -8, 16, 13}};
static const int kAnim138FrameIndices[] = {237, 238, 239};
static const int16 kAnim138FrameTicks[] = {6, 6, 6};
static const BBRect kAnim138FrameRects[] = {{-7, -8, 16, 13}, {-7, -11, 16, 12}, {-7, -14, 16, 13}};
static const int kAnim139FrameIndices[] = {239};
static const int16 kAnim139FrameTicks[] = {6};
static const BBRect kAnim139FrameRects[] = {{-7, -14, 16, 13}};
static const int kAnim140FrameIndices[] = {240};
static const int16 kAnim140FrameTicks[] = {6};
static const BBRect kAnim140FrameRects[] = {{-4, -4, 11, 7}};
static const int kAnim141FrameIndices[] = {240, 241, 242};
static const int16 kAnim141FrameTicks[] = {6, 6, 6};
static const BBRect kAnim141FrameRects[] = {{-4, -4, 11, 7}, {-5, -7, 12, 7}, {-5, -10, 12, 7}};
static const int kAnim142FrameIndices[] = {242};
static const int16 kAnim142FrameTicks[] = {6};
static const BBRect kAnim142FrameRects[] = {{-5, -10, 12, 7}};
static const int kAnim143FrameIndices[] = {243};
static const int16 kAnim143FrameTicks[] = {6};
static const BBRect kAnim143FrameRects[] = {{-5, -4, 12, 7}};
static const int kAnim144FrameIndices[] = {243, 244, 245};
static const int16 kAnim144FrameTicks[] = {6, 6, 6};
static const BBRect kAnim144FrameRects[] = {{-5, -4, 12, 7}, {-5, -7, 12, 7}, {-5, -10, 11, 7}};
static const int kAnim145FrameIndices[] = {245};
static const int16 kAnim145FrameTicks[] = {6};
static const BBRect kAnim145FrameRects[] = {{-5, -10, 11, 7}};
static const int kAnim146FrameIndices[] = {246};
static const int16 kAnim146FrameTicks[] = {6};
static const BBRect kAnim146FrameRects[] = {{-9, -11, 19, 15}};
static const int kAnim147FrameIndices[] = {246, 247, 248};
static const int16 kAnim147FrameTicks[] = {6, 6, 6};
static const BBRect kAnim147FrameRects[] = {{-9, -11, 19, 15}, {-9, -13, 19, 14}, {-9, -17, 19, 15}};
static const int kAnim148FrameIndices[] = {248};
static const int16 kAnim148FrameTicks[] = {6};
static const BBRect kAnim148FrameRects[] = {{-9, -17, 19, 15}};
static const int kAnim149FrameIndices[] = {249};
static const int16 kAnim149FrameTicks[] = {6};
static const BBRect kAnim149FrameRects[] = {{-9, -12, 22, 17}};
static const int kAnim150FrameIndices[] = {249, 250, 251};
static const int16 kAnim150FrameTicks[] = {6, 6, 6};
static const BBRect kAnim150FrameRects[] = {{-9, -12, 22, 17}, {-9, -15, 22, 17}, {-9, -18, 22, 17}};
static const int kAnim151FrameIndices[] = {251};
static const int16 kAnim151FrameTicks[] = {6};
static const BBRect kAnim151FrameRects[] = {{-9, -18, 22, 17}};
static const int kAnim152FrameIndices[] = {252};
static const int16 kAnim152FrameTicks[] = {6};
static const BBRect kAnim152FrameRects[] = {{-8, -5, 18, 9}};
static const int kAnim153FrameIndices[] = {252, 253, 254};
static const int16 kAnim153FrameTicks[] = {6, 6, 6};
static const BBRect kAnim153FrameRects[] = {{-8, -5, 18, 9}, {-7, -9, 17, 9}, {-8, -11, 19, 9}};
static const int kAnim154FrameIndices[] = {254};
static const int16 kAnim154FrameTicks[] = {6};
static const BBRect kAnim154FrameRects[] = {{-8, -11, 19, 9}};
static const int kAnim155FrameIndices[] = {255};
static const int16 kAnim155FrameTicks[] = {6};
static const BBRect kAnim155FrameRects[] = {{-8, -9, 18, 13}};
static const int kAnim156FrameIndices[] = {255, 256, 257};
static const int16 kAnim156FrameTicks[] = {6, 6, 6};
static const BBRect kAnim156FrameRects[] = {{-8, -9, 18, 13}, {-8, -12, 18, 13}, {-7, -15, 17, 13}};
static const int kAnim157FrameIndices[] = {257};
static const int16 kAnim157FrameTicks[] = {6};
static const BBRect kAnim157FrameRects[] = {{-7, -15, 17, 13}};
static const int kAnim158FrameIndices[] = {258, 259, 260, 261, 262, 263};
static const int16 kAnim158FrameTicks[] = {6, 8, 8, 8, 6, 6};
static const BBRect kAnim158FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim159FrameIndices[] = {264, 265, 266};
static const int16 kAnim159FrameTicks[] = {1, 1, 1};
static const BBRect kAnim159FrameRects[] = {{-9, -8, 18, 16}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim160FrameIndices[] = {267};
static const int16 kAnim160FrameTicks[] = {6};
static const BBRect kAnim160FrameRects[] = {{-25, -83, 43, 54}};
static const int kAnim161FrameIndices[] = {268};
static const int16 kAnim161FrameTicks[] = {6};
static const BBRect kAnim161FrameRects[] = {{-33, -93, 41, 60}};
static const int kAnim162FrameIndices[] = {269};
static const int16 kAnim162FrameTicks[] = {1};
static const BBRect kAnim162FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim163FrameIndices[] = {270};
static const int16 kAnim163FrameTicks[] = {5};
static const BBRect kAnim163FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim164FrameIndices[] = {271};
static const int16 kAnim164FrameTicks[] = {1};
static const BBRect kAnim164FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim165FrameIndices[] = {272};
static const int16 kAnim165FrameTicks[] = {1};
static const BBRect kAnim165FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim166FrameIndices[] = {273};
static const int16 kAnim166FrameTicks[] = {2};
static const BBRect kAnim166FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim167FrameIndices[] = {274, 275, 276, 277, 278, 279, 280, 281, 282, 283, 284, 285, 286};
static const int16 kAnim167FrameTicks[] = {6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
static const BBRect kAnim167FrameRects[] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
static const int kAnim168FrameIndices[] = {287};
static const int16 kAnim168FrameTicks[] = {1};
static const BBRect kAnim168FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim169FrameIndices[] = {288};
static const int16 kAnim169FrameTicks[] = {6};
static const BBRect kAnim169FrameRects[] = {{0, 0, 0, 0}};
static const int kAnim170FrameIndices[] = {289, 290, 291, 292, 293, 294};
static const int16 kAnim170FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim170FrameRects[] = {{-22, -91, 45, 93}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}, {-21, -92, 43, 95}};
static const int kAnim171FrameIndices[] = {295, 296, 297, 298, 299, 300};
static const int16 kAnim171FrameTicks[] = {6, 6, 6, 6, 6, 6};
static const BBRect kAnim171FrameRects[] = {{-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}, {-21, -85, 38, 86}};
static const int kAnim172FrameIndices[] = {301, 302};
static const int16 kAnim172FrameTicks[] = {6, 6};
static const BBRect kAnim172FrameRects[] = {{-9, -9, 17, 15}, {-11, -10, 19, 16}};
static const ObjAnimation kAnimations[] = {
	{3, kAnim0FrameIndices, kAnim0FrameTicks, kAnim0FrameRects},
	{3, kAnim1FrameIndices, kAnim1FrameTicks, kAnim1FrameRects},
	{3, kAnim2FrameIndices, kAnim2FrameTicks, kAnim2FrameRects},
	{3, kAnim3FrameIndices, kAnim3FrameTicks, kAnim3FrameRects},
	{3, kAnim4FrameIndices, kAnim4FrameTicks, kAnim4FrameRects},
	{3, kAnim5FrameIndices, kAnim5FrameTicks, kAnim5FrameRects},
	{3, kAnim6FrameIndices, kAnim6FrameTicks, kAnim6FrameRects},
	{3, kAnim7FrameIndices, kAnim7FrameTicks, kAnim7FrameRects},
	{1, kAnim8FrameIndices, kAnim8FrameTicks, kAnim8FrameRects},
	{1, kAnim9FrameIndices, kAnim9FrameTicks, kAnim9FrameRects},
	{1, kAnim10FrameIndices, kAnim10FrameTicks, kAnim10FrameRects},
	{1, kAnim11FrameIndices, kAnim11FrameTicks, kAnim11FrameRects},
	{1, kAnim12FrameIndices, kAnim12FrameTicks, kAnim12FrameRects},
	{1, kAnim13FrameIndices, kAnim13FrameTicks, kAnim13FrameRects},
	{1, kAnim14FrameIndices, kAnim14FrameTicks, kAnim14FrameRects},
	{1, kAnim15FrameIndices, kAnim15FrameTicks, kAnim15FrameRects},
	{3, kAnim16FrameIndices, kAnim16FrameTicks, kAnim16FrameRects},
	{3, kAnim17FrameIndices, kAnim17FrameTicks, kAnim17FrameRects},
	{3, kAnim18FrameIndices, kAnim18FrameTicks, kAnim18FrameRects},
	{3, kAnim19FrameIndices, kAnim19FrameTicks, kAnim19FrameRects},
	{3, kAnim20FrameIndices, kAnim20FrameTicks, kAnim20FrameRects},
	{3, kAnim21FrameIndices, kAnim21FrameTicks, kAnim21FrameRects},
	{3, kAnim22FrameIndices, kAnim22FrameTicks, kAnim22FrameRects},
	{3, kAnim23FrameIndices, kAnim23FrameTicks, kAnim23FrameRects},
	{14, kAnim24FrameIndices, kAnim24FrameTicks, kAnim24FrameRects},
	{14, kAnim25FrameIndices, kAnim25FrameTicks, kAnim25FrameRects},
	{3, kAnim26FrameIndices, kAnim26FrameTicks, kAnim26FrameRects},
	{3, kAnim27FrameIndices, kAnim27FrameTicks, kAnim27FrameRects},
	{3, kAnim28FrameIndices, kAnim28FrameTicks, kAnim28FrameRects},
	{3, kAnim29FrameIndices, kAnim29FrameTicks, kAnim29FrameRects},
	{3, kAnim30FrameIndices, kAnim30FrameTicks, kAnim30FrameRects},
	{3, kAnim31FrameIndices, kAnim31FrameTicks, kAnim31FrameRects},
	{3, kAnim32FrameIndices, kAnim32FrameTicks, kAnim32FrameRects},
	{3, kAnim33FrameIndices, kAnim33FrameTicks, kAnim33FrameRects},
	{1, kAnim34FrameIndices, kAnim34FrameTicks, kAnim34FrameRects},
	{1, kAnim35FrameIndices, kAnim35FrameTicks, kAnim35FrameRects},
	{1, kAnim36FrameIndices, kAnim36FrameTicks, kAnim36FrameRects},
	{1, kAnim37FrameIndices, kAnim37FrameTicks, kAnim37FrameRects},
	{1, kAnim38FrameIndices, kAnim38FrameTicks, kAnim38FrameRects},
	{1, kAnim39FrameIndices, kAnim39FrameTicks, kAnim39FrameRects},
	{1, kAnim40FrameIndices, kAnim40FrameTicks, kAnim40FrameRects},
	{1, kAnim41FrameIndices, kAnim41FrameTicks, kAnim41FrameRects},
	{3, kAnim42FrameIndices, kAnim42FrameTicks, kAnim42FrameRects},
	{3, kAnim43FrameIndices, kAnim43FrameTicks, kAnim43FrameRects},
	{3, kAnim44FrameIndices, kAnim44FrameTicks, kAnim44FrameRects},
	{3, kAnim45FrameIndices, kAnim45FrameTicks, kAnim45FrameRects},
	{3, kAnim46FrameIndices, kAnim46FrameTicks, kAnim46FrameRects},
	{3, kAnim47FrameIndices, kAnim47FrameTicks, kAnim47FrameRects},
	{3, kAnim48FrameIndices, kAnim48FrameTicks, kAnim48FrameRects},
	{3, kAnim49FrameIndices, kAnim49FrameTicks, kAnim49FrameRects},
	{14, kAnim50FrameIndices, kAnim50FrameTicks, kAnim50FrameRects},
	{14, kAnim51FrameIndices, kAnim51FrameTicks, kAnim51FrameRects},
	{3, kAnim52FrameIndices, kAnim52FrameTicks, kAnim52FrameRects},
	{3, kAnim53FrameIndices, kAnim53FrameTicks, kAnim53FrameRects},
	{3, kAnim54FrameIndices, kAnim54FrameTicks, kAnim54FrameRects},
	{3, kAnim55FrameIndices, kAnim55FrameTicks, kAnim55FrameRects},
	{3, kAnim56FrameIndices, kAnim56FrameTicks, kAnim56FrameRects},
	{3, kAnim57FrameIndices, kAnim57FrameTicks, kAnim57FrameRects},
	{3, kAnim58FrameIndices, kAnim58FrameTicks, kAnim58FrameRects},
	{3, kAnim59FrameIndices, kAnim59FrameTicks, kAnim59FrameRects},
	{1, kAnim60FrameIndices, kAnim60FrameTicks, kAnim60FrameRects},
	{1, kAnim61FrameIndices, kAnim61FrameTicks, kAnim61FrameRects},
	{1, kAnim62FrameIndices, kAnim62FrameTicks, kAnim62FrameRects},
	{1, kAnim63FrameIndices, kAnim63FrameTicks, kAnim63FrameRects},
	{1, kAnim64FrameIndices, kAnim64FrameTicks, kAnim64FrameRects},
	{1, kAnim65FrameIndices, kAnim65FrameTicks, kAnim65FrameRects},
	{1, kAnim66FrameIndices, kAnim66FrameTicks, kAnim66FrameRects},
	{1, kAnim67FrameIndices, kAnim67FrameTicks, kAnim67FrameRects},
	{3, kAnim68FrameIndices, kAnim68FrameTicks, kAnim68FrameRects},
	{3, kAnim69FrameIndices, kAnim69FrameTicks, kAnim69FrameRects},
	{3, kAnim70FrameIndices, kAnim70FrameTicks, kAnim70FrameRects},
	{3, kAnim71FrameIndices, kAnim71FrameTicks, kAnim71FrameRects},
	{3, kAnim72FrameIndices, kAnim72FrameTicks, kAnim72FrameRects},
	{3, kAnim73FrameIndices, kAnim73FrameTicks, kAnim73FrameRects},
	{3, kAnim74FrameIndices, kAnim74FrameTicks, kAnim74FrameRects},
	{3, kAnim75FrameIndices, kAnim75FrameTicks, kAnim75FrameRects},
	{14, kAnim76FrameIndices, kAnim76FrameTicks, kAnim76FrameRects},
	{14, kAnim77FrameIndices, kAnim77FrameTicks, kAnim77FrameRects},
	{3, kAnim78FrameIndices, kAnim78FrameTicks, kAnim78FrameRects},
	{3, kAnim79FrameIndices, kAnim79FrameTicks, kAnim79FrameRects},
	{3, kAnim80FrameIndices, kAnim80FrameTicks, kAnim80FrameRects},
	{3, kAnim81FrameIndices, kAnim81FrameTicks, kAnim81FrameRects},
	{3, kAnim82FrameIndices, kAnim82FrameTicks, kAnim82FrameRects},
	{3, kAnim83FrameIndices, kAnim83FrameTicks, kAnim83FrameRects},
	{3, kAnim84FrameIndices, kAnim84FrameTicks, kAnim84FrameRects},
	{3, kAnim85FrameIndices, kAnim85FrameTicks, kAnim85FrameRects},
	{1, kAnim86FrameIndices, kAnim86FrameTicks, kAnim86FrameRects},
	{1, kAnim87FrameIndices, kAnim87FrameTicks, kAnim87FrameRects},
	{1, kAnim88FrameIndices, kAnim88FrameTicks, kAnim88FrameRects},
	{1, kAnim89FrameIndices, kAnim89FrameTicks, kAnim89FrameRects},
	{1, kAnim90FrameIndices, kAnim90FrameTicks, kAnim90FrameRects},
	{1, kAnim91FrameIndices, kAnim91FrameTicks, kAnim91FrameRects},
	{1, kAnim92FrameIndices, kAnim92FrameTicks, kAnim92FrameRects},
	{1, kAnim93FrameIndices, kAnim93FrameTicks, kAnim93FrameRects},
	{3, kAnim94FrameIndices, kAnim94FrameTicks, kAnim94FrameRects},
	{3, kAnim95FrameIndices, kAnim95FrameTicks, kAnim95FrameRects},
	{3, kAnim96FrameIndices, kAnim96FrameTicks, kAnim96FrameRects},
	{3, kAnim97FrameIndices, kAnim97FrameTicks, kAnim97FrameRects},
	{3, kAnim98FrameIndices, kAnim98FrameTicks, kAnim98FrameRects},
	{3, kAnim99FrameIndices, kAnim99FrameTicks, kAnim99FrameRects},
	{3, kAnim100FrameIndices, kAnim100FrameTicks, kAnim100FrameRects},
	{3, kAnim101FrameIndices, kAnim101FrameTicks, kAnim101FrameRects},
	{14, kAnim102FrameIndices, kAnim102FrameTicks, kAnim102FrameRects},
	{14, kAnim103FrameIndices, kAnim103FrameTicks, kAnim103FrameRects},
	{3, kAnim104FrameIndices, kAnim104FrameTicks, kAnim104FrameRects},
	{3, kAnim105FrameIndices, kAnim105FrameTicks, kAnim105FrameRects},
	{3, kAnim106FrameIndices, kAnim106FrameTicks, kAnim106FrameRects},
	{3, kAnim107FrameIndices, kAnim107FrameTicks, kAnim107FrameRects},
	{3, kAnim108FrameIndices, kAnim108FrameTicks, kAnim108FrameRects},
	{3, kAnim109FrameIndices, kAnim109FrameTicks, kAnim109FrameRects},
	{3, kAnim110FrameIndices, kAnim110FrameTicks, kAnim110FrameRects},
	{3, kAnim111FrameIndices, kAnim111FrameTicks, kAnim111FrameRects},
	{1, kAnim112FrameIndices, kAnim112FrameTicks, kAnim112FrameRects},
	{1, kAnim113FrameIndices, kAnim113FrameTicks, kAnim113FrameRects},
	{1, kAnim114FrameIndices, kAnim114FrameTicks, kAnim114FrameRects},
	{1, kAnim115FrameIndices, kAnim115FrameTicks, kAnim115FrameRects},
	{1, kAnim116FrameIndices, kAnim116FrameTicks, kAnim116FrameRects},
	{1, kAnim117FrameIndices, kAnim117FrameTicks, kAnim117FrameRects},
	{1, kAnim118FrameIndices, kAnim118FrameTicks, kAnim118FrameRects},
	{1, kAnim119FrameIndices, kAnim119FrameTicks, kAnim119FrameRects},
	{3, kAnim120FrameIndices, kAnim120FrameTicks, kAnim120FrameRects},
	{3, kAnim121FrameIndices, kAnim121FrameTicks, kAnim121FrameRects},
	{3, kAnim122FrameIndices, kAnim122FrameTicks, kAnim122FrameRects},
	{3, kAnim123FrameIndices, kAnim123FrameTicks, kAnim123FrameRects},
	{3, kAnim124FrameIndices, kAnim124FrameTicks, kAnim124FrameRects},
	{3, kAnim125FrameIndices, kAnim125FrameTicks, kAnim125FrameRects},
	{3, kAnim126FrameIndices, kAnim126FrameTicks, kAnim126FrameRects},
	{3, kAnim127FrameIndices, kAnim127FrameTicks, kAnim127FrameRects},
	{14, kAnim128FrameIndices, kAnim128FrameTicks, kAnim128FrameRects},
	{14, kAnim129FrameIndices, kAnim129FrameTicks, kAnim129FrameRects},
	{1, kAnim130FrameIndices, kAnim130FrameTicks, kAnim130FrameRects},
	{1, kAnim131FrameIndices, kAnim131FrameTicks, kAnim131FrameRects},
	{3, kAnim132FrameIndices, kAnim132FrameTicks, kAnim132FrameRects},
	{1, kAnim133FrameIndices, kAnim133FrameTicks, kAnim133FrameRects},
	{1, kAnim134FrameIndices, kAnim134FrameTicks, kAnim134FrameRects},
	{3, kAnim135FrameIndices, kAnim135FrameTicks, kAnim135FrameRects},
	{1, kAnim136FrameIndices, kAnim136FrameTicks, kAnim136FrameRects},
	{1, kAnim137FrameIndices, kAnim137FrameTicks, kAnim137FrameRects},
	{3, kAnim138FrameIndices, kAnim138FrameTicks, kAnim138FrameRects},
	{1, kAnim139FrameIndices, kAnim139FrameTicks, kAnim139FrameRects},
	{1, kAnim140FrameIndices, kAnim140FrameTicks, kAnim140FrameRects},
	{3, kAnim141FrameIndices, kAnim141FrameTicks, kAnim141FrameRects},
	{1, kAnim142FrameIndices, kAnim142FrameTicks, kAnim142FrameRects},
	{1, kAnim143FrameIndices, kAnim143FrameTicks, kAnim143FrameRects},
	{3, kAnim144FrameIndices, kAnim144FrameTicks, kAnim144FrameRects},
	{1, kAnim145FrameIndices, kAnim145FrameTicks, kAnim145FrameRects},
	{1, kAnim146FrameIndices, kAnim146FrameTicks, kAnim146FrameRects},
	{3, kAnim147FrameIndices, kAnim147FrameTicks, kAnim147FrameRects},
	{1, kAnim148FrameIndices, kAnim148FrameTicks, kAnim148FrameRects},
	{1, kAnim149FrameIndices, kAnim149FrameTicks, kAnim149FrameRects},
	{3, kAnim150FrameIndices, kAnim150FrameTicks, kAnim150FrameRects},
	{1, kAnim151FrameIndices, kAnim151FrameTicks, kAnim151FrameRects},
	{1, kAnim152FrameIndices, kAnim152FrameTicks, kAnim152FrameRects},
	{3, kAnim153FrameIndices, kAnim153FrameTicks, kAnim153FrameRects},
	{1, kAnim154FrameIndices, kAnim154FrameTicks, kAnim154FrameRects},
	{1, kAnim155FrameIndices, kAnim155FrameTicks, kAnim155FrameRects},
	{3, kAnim156FrameIndices, kAnim156FrameTicks, kAnim156FrameRects},
	{1, kAnim157FrameIndices, kAnim157FrameTicks, kAnim157FrameRects},
	{6, kAnim158FrameIndices, kAnim158FrameTicks, kAnim158FrameRects},
	{3, kAnim159FrameIndices, kAnim159FrameTicks, kAnim159FrameRects},
	{1, kAnim160FrameIndices, kAnim160FrameTicks, kAnim160FrameRects},
	{1, kAnim161FrameIndices, kAnim161FrameTicks, kAnim161FrameRects},
	{1, kAnim162FrameIndices, kAnim162FrameTicks, kAnim162FrameRects},
	{1, kAnim163FrameIndices, kAnim163FrameTicks, kAnim163FrameRects},
	{1, kAnim164FrameIndices, kAnim164FrameTicks, kAnim164FrameRects},
	{1, kAnim165FrameIndices, kAnim165FrameTicks, kAnim165FrameRects},
	{1, kAnim166FrameIndices, kAnim166FrameTicks, kAnim166FrameRects},
	{13, kAnim167FrameIndices, kAnim167FrameTicks, kAnim167FrameRects},
	{1, kAnim168FrameIndices, kAnim168FrameTicks, kAnim168FrameRects},
	{1, kAnim169FrameIndices, kAnim169FrameTicks, kAnim169FrameRects},
	{6, kAnim170FrameIndices, kAnim170FrameTicks, kAnim170FrameRects},
	{6, kAnim171FrameIndices, kAnim171FrameTicks, kAnim171FrameRects},
	{2, kAnim172FrameIndices, kAnim172FrameTicks, kAnim172FrameRects}
};

static const MinigameBbAnt::ObjInit kObjInits[] = {
	{&kAnimations[131], &kAnimations[132], &kAnimations[133], 160, 120},
	{&kAnimations[134], &kAnimations[135], &kAnimations[136], 155, 130},
	{&kAnimations[137], &kAnimations[138], &kAnimations[139], 150, 100},
	{&kAnimations[140], &kAnimations[141], &kAnimations[142], 195, 150},
	{&kAnimations[143], &kAnimations[144], &kAnimations[145], 120, 110},
	{&kAnimations[146], &kAnimations[147], &kAnimations[148], 170, 170},
	{&kAnimations[149], &kAnimations[150], &kAnimations[151], 175, 95},
	{&kAnimations[152], &kAnimations[153], &kAnimations[154], 145, 165},
	{&kAnimations[155], &kAnimations[156], &kAnimations[157], 110, 175}
};
static const ObjAnimation * const kAnimationsTbl[] = {&kAnimations[0], &kAnimations[1], &kAnimations[2], &kAnimations[3], &kAnimations[4], &kAnimations[5], &kAnimations[6], &kAnimations[7], &kAnimations[16], &kAnimations[17], &kAnimations[18], &kAnimations[19], &kAnimations[20], &kAnimations[21], &kAnimations[22], &kAnimations[23], &kAnimations[24], &kAnimations[25], &kAnimations[26], &kAnimations[27], &kAnimations[28], &kAnimations[29], &kAnimations[30], &kAnimations[31], &kAnimations[32], &kAnimations[33], &kAnimations[42], &kAnimations[43], &kAnimations[44], &kAnimations[45], &kAnimations[46], &kAnimations[47], &kAnimations[48], &kAnimations[49], &kAnimations[50], &kAnimations[51], &kAnimations[52], &kAnimations[53], &kAnimations[54], &kAnimations[55], &kAnimations[56], &kAnimations[57], &kAnimations[58], &kAnimations[59], &kAnimations[68], &kAnimations[69], &kAnimations[70], &kAnimations[71], &kAnimations[72], &kAnimations[73], &kAnimations[74], &kAnimations[75], &kAnimations[76], &kAnimations[77], &kAnimations[78], &kAnimations[79], &kAnimations[80], &kAnimations[81], &kAnimations[82], &kAnimations[83], &kAnimations[84], &kAnimations[85], &kAnimations[94], &kAnimations[95], &kAnimations[96], &kAnimations[97], &kAnimations[98], &kAnimations[99], &kAnimations[100], &kAnimations[101], &kAnimations[102], &kAnimations[103], &kAnimations[104], &kAnimations[105], &kAnimations[106], &kAnimations[107], &kAnimations[108], &kAnimations[109], &kAnimations[110], &kAnimations[111], &kAnimations[120], &kAnimations[121], &kAnimations[122], &kAnimations[123], &kAnimations[124], &kAnimations[125], &kAnimations[126], &kAnimations[127], &kAnimations[128], &kAnimations[129]};

static const ObjAnimation * const * const kObjKindAnimTables[] = {
	0, &kAnimationsTbl[0],
	&kAnimationsTbl[18], &kAnimationsTbl[36],
	&kAnimationsTbl[54], &kAnimationsTbl[72]
};

const ObjAnimation *MinigameBbAnt::getAnimation(int animIndex) {
	return &kAnimations[animIndex];
}

const MinigameBbAnt::ObjInit *MinigameBbAnt::getObjInit(int index) {
	return &kObjInits[index];
}

const ObjAnimation * const *MinigameBbAnt::getObjKindAnimTable(int kind) {
	return kObjKindAnimTables[kind];
}

const ObjAnimation *MinigameBbAnt::getObjAnim(int index) {
	return kAnimationsTbl[index];
}

} // End of namespace Bbvs
