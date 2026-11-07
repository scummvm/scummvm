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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/media/playback_plan.h"

namespace Ripper {

Common::String describeSmackerPlaybackPlan(const SmackerPlaybackPlan &plan) {
	return Common::String::format(
		"position=%d,%d originY=%d scale=%u controls=%d "
		"sceneUi=%d mouseStop=%d segmentAdvance=%d palette=interface:%d,wac:%d,preserve:%d,remember:%d "
		"frames=%u..%u previewLimit=%u loop=start:%u,fromStart:%d,bounded:%u "
		"timeline=%d callback=%d transparent=%d,retainFinal=%d",
		plan.placement.x, plan.placement.y,
		plan.placement.originY, plan.placement.displayScale,
		plan.input.allowEscSpace, plan.input.serviceSceneUi,
		plan.input.stopOnMouse, plan.input.allowSegmentAdvance,
		plan.palette.patchInterfacePalette, plan.palette.patchWacMediaPalette,
		plan.palette.preserveDisplayPalette, plan.palette.rememberVideoPalette,
		plan.frames.firstFrame,
		plan.frames.lastFrame, plan.frames.frameLimit, plan.loop.loopStartFrame,
		plan.loop.loopFromStart, plan.loop.boundedLoopStartFrame,
		plan.timeline.frameAudioOffsets != nullptr && plan.timeline.audioByteRate != 0,
		plan.callback.sequenceCallback != nullptr,
		plan.rendering.transparentFirstPixel,
		plan.rendering.retainFinalTransparentFrame);
}

} // End of namespace Ripper
