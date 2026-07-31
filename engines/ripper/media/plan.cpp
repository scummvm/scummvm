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

Common::String describeSmackerPlaybackRequest(const SmackerPlaybackRequest &request) {
	return Common::String::format(
		"route=%s position=%d,%d originY=%d scale=%u controls=%d "
		"sceneUi=%d palette=interface:%d,wac:%d,remember:%d "
		"frames=%u..%u previewLimit=%u loop=start:%u,fromStart:%d,bounded:%u "
		"timeline=%d callback=%d transparent=%d",
		request.retailRoute, request.x, request.y, request.originY,
		request.displayScale, request.allowEscSpace, request.serviceSceneUi,
		request.patchInterfacePalette, request.patchWacMediaPalette,
		request.rememberVideoPalette, request.firstFrame, request.lastFrame,
		request.frameLimit, request.loopStartFrame, request.loopFromStart,
		request.boundedLoopStartFrame,
		request.frameAudioOffsets != nullptr && request.audioByteRate != 0,
		request.sequenceCallback != nullptr, request.transparentFirstPixel);
}

} // End of namespace Ripper
