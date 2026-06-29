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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H
#define HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H

#include "common/types.h"

namespace Hollywood {

// Tracks a resource-backed sprite layer and the previous descriptor it occupied.
struct ResourceSpriteLayer {
	ResourceSpriteLayer() :
		visible(false),
		chunkIndex(0),
		descriptorCount(0),
		frameMap(nullptr),
		frameMapSize(0),
		frameIndex(0),
		previousDescriptorIndex(0),
		hasPreviousDescriptor(false) {
	}

	void configure(uint newChunkIndex, uint16 newDescriptorCount, const byte *newFrameMap, uint newFrameMapSize) {
		chunkIndex = newChunkIndex;
		descriptorCount = newDescriptorCount;
		frameMap = newFrameMap;
		frameMapSize = newFrameMapSize;
		reset(0);
	}

	void reset(byte newFrameIndex) {
		frameIndex = newFrameIndex;
		previousDescriptorIndex = descriptorIndex();
		hasPreviousDescriptor = false;
	}

	void setFrame(byte newFrameIndex) {
		const uint16 oldDescriptorIndex = descriptorIndex();
		frameIndex = newFrameIndex;
		const uint16 newDescriptorIndex = descriptorIndex();
		if (oldDescriptorIndex != newDescriptorIndex) {
			previousDescriptorIndex = oldDescriptorIndex;
			hasPreviousDescriptor = true;
		}
	}

	uint16 descriptorIndex() const {
		if (frameMap != nullptr && frameIndex < frameMapSize)
			return frameMap[frameIndex];
		return frameIndex;
	}

	bool visible;
	uint chunkIndex;
	uint16 descriptorCount;
	const byte *frameMap;
	uint frameMapSize;
	byte frameIndex;
	uint16 previousDescriptorIndex;
	bool hasPreviousDescriptor;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_ANIMATION_LAYERS_H
