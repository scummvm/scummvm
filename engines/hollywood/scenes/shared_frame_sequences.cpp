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

#include "hollywood/scenes/shared_frame_sequences.h"

namespace Hollywood {

const byte kCoffinDeltaClipFrames[kCoffinDeltaClipFrameCount] = {
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21,
	22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22
};

const byte kQuillDeskFrames[kQuillDeskFrameCount] = {
	0, 31, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13,
	32, 33, 34, 35, 14, 15, 16, 16, 17, 18, 19, 29, 20, 21, 22, 23,
	23, 24, 25, 26, 30, 22, 21, 20, 16
};

const byte kTitleBlinkFrames[kTitleBlinkFrameCount] = {
	0, 11, 0, 1, 2, 1, 0, 0,
	1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 10, 10, 10, 10, 9, 8,
	7, 6, 5, 4, 3, 2, 1, 0
};

const byte kTravelUnlockFrames[kTravelUnlockFrameCount] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	9, 10, 11, 12, 12, 13, 14, 15, 16, 11,
	10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

const byte kDualPoseSpeakerFrames[kDualPoseSpeakerFrameCount] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
	7, 6, 5, 0, 13, 14, 15, 16, 17, 18, 19, 0
};

const byte kTwoActorGestureLeftFrames[kTwoActorGestureFrameCount] = {
	0, 0, 0, 0, 5, 6, 7, 8, 9, 10
};

const byte kTwoActorGestureRightFrames[kTwoActorGestureFrameCount] = {
	5, 6, 7, 8, 8, 8, 8, 9, 10, 10
};

const byte kHoldFirstNineFrames[kHoldFirstNineFrameCount] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8
};

const byte kFiveFramePingPongFrames[kFiveFramePingPongFrameCount] = {
	0, 1, 2, 3, 4, 3, 2, 1, 0
};

} // End of namespace Hollywood
