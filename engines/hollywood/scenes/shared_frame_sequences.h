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

#ifndef HOLLYWOOD_SCENES_SHARED_FRAME_SEQUENCES_H
#define HOLLYWOOD_SCENES_SHARED_FRAME_SEQUENCES_H

#include "common/types.h"

namespace Hollywood {

// Descriptor timelines reused by matching assets or standard animation patterns.
enum {
	kCoffinDeltaClipFrameCount = 44,
	kQuillDeskFrameCount = 41,
	kTitleBlinkFrameCount = 32,
	kTravelUnlockFrameCount = 31,
	kDualPoseSpeakerFrameCount = 25,
	kTwoActorGestureFrameCount = 10,
	kHoldFirstNineFrameCount = 10,
	kFiveFramePingPongFrameCount = 9
};

extern const byte kCoffinDeltaClipFrames[kCoffinDeltaClipFrameCount];
extern const byte kQuillDeskFrames[kQuillDeskFrameCount];
extern const byte kTitleBlinkFrames[kTitleBlinkFrameCount];
extern const byte kTravelUnlockFrames[kTravelUnlockFrameCount];
extern const byte kDualPoseSpeakerFrames[kDualPoseSpeakerFrameCount];
extern const byte kTwoActorGestureLeftFrames[kTwoActorGestureFrameCount];
extern const byte kTwoActorGestureRightFrames[kTwoActorGestureFrameCount];
extern const byte kHoldFirstNineFrames[kHoldFirstNineFrameCount];
extern const byte kFiveFramePingPongFrames[kFiveFramePingPongFrameCount];

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_SHARED_FRAME_SEQUENCES_H
