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

#ifndef HOLLYWOOD_SCENES_RESOURCE_DELTA_CLIP_PLAYER_H
#define HOLLYWOOD_SCENES_RESOURCE_DELTA_CLIP_PLAYER_H

#include "common/array.h"
#include "common/types.h"

namespace Hollywood {

/**
 * Applies one resource delta frame to an existing indexed framebuffer.
 *
 * Frames contain bounded rows of literal or repeated pixel runs. drawFrame()
 * validates resource and destination bounds and clips runs to the row width;
 * pixels outside those runs retain their previous values.
 */
class ResourceDeltaClipPlayer {
public:
	static bool drawFrame(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex, byte *pixels,
		uint framebufferByteCount);
	static bool drawFrame(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex, byte *pixels,
		uint destinationWidth, uint destinationHeight, uint destinationPitch,
		uint destinationByteCount);
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_RESOURCE_DELTA_CLIP_PLAYER_H
