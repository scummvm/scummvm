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

#include "ripper/media.h"

#include "common/stream.h"

namespace Ripper {

MediaFormat detectMediaFormat(Common::SeekableReadStream &stream) {
	const int64 originalPosition = stream.pos();
	byte magic[8] = { 0 };
	const uint32 bytesRead = stream.read(magic, sizeof(magic));
	if (!stream.seek(originalPosition))
		return kMediaFormatUnknown;
	if (bytesRead >= 8 && memcmp(magic, "IAVF2.00", 8) == 0)
		return kMediaFormatIavf;
	if (bytesRead >= 4 &&
			(memcmp(magic, "SMK2", 4) == 0 || memcmp(magic, "SMK4", 4) == 0))
		return kMediaFormatSmacker;
	return kMediaFormatUnknown;
}

const char *mediaFormatName(MediaFormat format) {
	switch (format) {
	case kMediaFormatSmacker:
		return "smacker";
	case kMediaFormatIavf:
		return "iavf";
	default:
		return "unknown";
	}
}

Common::String describeSmackerPlaybackPlan(const SmackerPlaybackPlan &plan) {
	return Common::String::format(
		"route=%s position=%d,%d originY=%d scale=%u controls=%d "
		"sceneUi=%d palette=interface:%d,wac:%d,remember:%d "
		"frames=%u..%u previewLimit=%u loop=start:%u,fromStart:%d,bounded:%u "
		"timeline=%d callback=%d transparent=%d",
		plan.retailRoute, plan.placement.x, plan.placement.y,
		plan.placement.originY, plan.placement.displayScale,
		plan.input.allowEscSpace, plan.input.serviceSceneUi,
		plan.palette.patchInterfacePalette, plan.palette.patchWacMediaPalette,
		plan.palette.rememberVideoPalette, plan.frames.firstFrame,
		plan.frames.lastFrame, plan.frames.frameLimit, plan.loop.loopStartFrame,
		plan.loop.loopFromStart, plan.loop.boundedLoopStartFrame,
		plan.timeline.frameAudioOffsets != nullptr && plan.timeline.audioByteRate != 0,
		plan.callback.sequenceCallback != nullptr,
		plan.rendering.transparentFirstPixel);
}

} // End of namespace Ripper
