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

#include "hollywood/scenes/resource_delta_clip_player.h"

#include "hollywood/graphics.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

bool ResourceDeltaClipPlayer::drawFrame(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex, byte *pixels,
		uint framebufferByteCount) {
	return drawFrame(resource, frameTableOffset, chunkSize, tableEntryCount, frameIndex, pixels,
		HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight,
		HollywoodEngine::kSceneBufferWidth, framebufferByteCount);
}

bool ResourceDeltaClipPlayer::drawFrame(const Common::Array<byte> &resource, uint32 frameTableOffset,
		uint32 chunkSize, uint tableEntryCount, byte frameIndex, byte *pixels,
		uint destinationWidth, uint destinationHeight, uint destinationPitch,
		uint destinationByteCount) {
	if (!pixels || frameIndex >= tableEntryCount)
		return false;

	if (frameTableOffset > resource.size() || chunkSize > resource.size() - frameTableOffset ||
			tableEntryCount > chunkSize / 4)
		return false;

	const uint32 chunkEnd = frameTableOffset + chunkSize;
	const uint32 tableByteCount = (uint32)tableEntryCount * 4;
	if (tableByteCount > chunkEnd - frameTableOffset)
		return false;

	const uint32 tableEntryOffset = frameTableOffset + ((uint32)frameIndex * 4);
	if (tableEntryOffset + 4 > chunkEnd)
		return false;

	const uint32 frameOffset = frameTableOffset + tableByteCount +
		readUint32LE(resource, tableEntryOffset);
	if (frameOffset + 4 > chunkEnd)
		return false;

	const uint16 firstRow = readUint16LE(resource, frameOffset);
	const uint16 lastRow = readUint16LE(resource, frameOffset + 2);
	if (firstRow > lastRow)
		return false;

	uint cursor = frameOffset + 4;
	for (uint row = firstRow; row <= lastRow && row < destinationHeight; ++row) {
		if (cursor >= chunkEnd)
			return false;

		byte runCount = resource[cursor++];
		for (; runCount != 0; --runCount) {
			if (cursor + 3 > chunkEnd)
				return false;

			const uint x = readUint16LE(resource, cursor);
			const byte literalLength = resource[cursor + 2];
			const uint destinationOffset = row * destinationPitch + x;

			if (literalLength == 0) {
				if (cursor + 5 > chunkEnd)
					return false;

				const byte fillValue = resource[cursor + 3];
				const uint fillLength = resource[cursor + 4];
				cursor += 5;
				if (x < destinationWidth && destinationOffset < destinationByteCount) {
					const uint copyLength = MIN<uint>(fillLength,
						destinationWidth - x);
					if (destinationOffset + copyLength <= destinationByteCount)
						memset(pixels + destinationOffset, fillValue, copyLength);
				}
			} else {
				const uint literalOffset = cursor + 3;
				if (literalOffset + literalLength > chunkEnd)
					return false;

				if (x < destinationWidth && destinationOffset < destinationByteCount) {
					const uint copyLength = MIN<uint>((uint)literalLength,
						destinationWidth - x);
					if (destinationOffset + copyLength <= destinationByteCount)
						memcpy(pixels + destinationOffset, resource.data() + literalOffset, copyLength);
				}
				cursor = literalOffset + literalLength;
			}
		}
	}

	return true;
}

} // End of namespace Hollywood
