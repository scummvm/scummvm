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

#include "hollywood/gameplay/actor_renderer.h"

#include "graphics/surface.h"

#include "hollywood/hollywood.h"

namespace Hollywood {

const uint kPaletteRemapTableSize = 256;
const uint kPaletteTripletSize = 3;
const uint kActorPaletteFirstColor = 0xd0;
const uint kActorFacingCount = 6;
const uint kActorCelsPerFacing = 13;
const uint kActorFacingRunStride = 160000;
const uint kShadowDarkenNumerator = 3;
const uint kShadowDarkenDenominator = 4;

uint colorDistanceSquared(int redA, int greenA, int blueA, int redB, int greenB, int blueB) {
	const int redDelta = redA - redB;
	const int greenDelta = greenA - greenB;
	const int blueDelta = blueA - blueB;
	return redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;
}

byte nearestDarkerPaletteColor(const Common::Array<byte> &palette, byte sourceColor) {
	const uint sourceOffset = sourceColor * kPaletteTripletSize;
	const uint sourceRed = palette[sourceOffset];
	const uint sourceGreen = palette[sourceOffset + 1];
	const uint sourceBlue = palette[sourceOffset + 2];
	const uint targetRed = sourceRed * kShadowDarkenNumerator / kShadowDarkenDenominator;
	const uint targetGreen = sourceGreen * kShadowDarkenNumerator / kShadowDarkenDenominator;
	const uint targetBlue = sourceBlue * kShadowDarkenNumerator / kShadowDarkenDenominator;
	const bool sourceUsesActorPalette = sourceColor >= kActorPaletteFirstColor;

	byte bestColor = sourceColor;
	uint bestDistance = 0xffffffffU;
	for (uint color = 0; color < kPaletteRemapTableSize; ++color) {
		const bool candidateUsesActorPalette = color >= kActorPaletteFirstColor;
		if (sourceUsesActorPalette != candidateUsesActorPalette)
			continue;

		const uint candidateOffset = color * kPaletteTripletSize;
		const uint distance = colorDistanceSquared(targetRed, targetGreen, targetBlue,
			palette[candidateOffset], palette[candidateOffset + 1], palette[candidateOffset + 2]);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = (byte)color;
		}
	}

	return bestColor;
}

bool actorDepthTestEnabled(const ActorDepthTest *depthTest) {
	return depthTest != nullptr &&
		depthTest->enabled &&
		depthTest->savedFramebuffer != nullptr &&
		depthTest->colorToDepthClassMap != nullptr &&
		depthTest->depthYThresholds != nullptr;
}

bool actorPixelPassesDepthTest(const ActorDepthTest &depthTest, uint framebufferOffset) {
	if (depthTest.savedFramebuffer->format.bytesPerPixel != 1 ||
			framebufferOffset >= (uint)depthTest.savedFramebuffer->w * (uint)depthTest.savedFramebuffer->h)
		return false;

	const uint x = framebufferOffset % HollywoodEngine::kSceneBufferWidth;
	const uint y = framebufferOffset / HollywoodEngine::kSceneBufferWidth;
	if (x >= (uint)depthTest.savedFramebuffer->w || y >= (uint)depthTest.savedFramebuffer->h)
		return false;

	const byte savedColor = *(const byte *)depthTest.savedFramebuffer->getBasePtr(x, y);
	if (savedColor >= depthTest.colorToDepthClassMap->size())
		return false;

	const byte depthClass = (*depthTest.colorToDepthClassMap)[savedColor];
	if (depthClass >= depthTest.depthYThresholds->size())
		return false;

	return (*depthTest.depthYThresholds)[depthClass] < depthTest.actorWorldY;
}

int drawActorRunStream(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, Graphics::Surface &destination,
		const ActorDepthTest *depthTest, uint *nextCursor) {
	const bool useDepthTest = actorDepthTestEnabled(depthTest);
	if (destination.format.bytesPerPixel != 1)
		return minimumYExclusive;

	cursor += runBase;
	int lastRunY = minimumYExclusive;

	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size()) {
			if (nextCursor != nullptr)
				*nextCursor = cursor - runBase;
			return lastRunY;
		}

		const int xOffset = runStreams[cursor++];
		const int yOffset = runStreams[cursor++];
		const uint pixelCount = runStreams[cursor++];
		if (cursor + pixelCount > runStreams.size()) {
			if (nextCursor != nullptr)
				*nextCursor = cursor - runBase;
			return lastRunY;
		}

		const int dstY = spriteY + yOffset;
		lastRunY = dstY;
		if (dstY > minimumYExclusive && dstY >= 0 && dstY < HollywoodEngine::kSceneBufferHeight) {
			int dstX = spriteX + xOffset;
			uint sourceOffset = 0;
			uint copyCount = pixelCount;
			if (dstX < 0) {
				const uint clipped = MIN<uint>(copyCount, (uint)-dstX);
				sourceOffset += clipped;
				copyCount -= clipped;
				dstX = 0;
			}
			if (dstX + (int)copyCount > HollywoodEngine::kSceneBufferWidth)
				copyCount = MAX<int>(0, HollywoodEngine::kSceneBufferWidth - dstX);

			if (copyCount != 0) {
				const uint destinationOffset = dstX + dstY * HollywoodEngine::kSceneBufferWidth;
				if (dstX + copyCount <= (uint)destination.w) {
					byte *destinationPixels = (byte *)destination.getBasePtr(dstX, dstY);
					if (useDepthTest) {
						for (uint i = 0; i < copyCount; ++i) {
							if (actorPixelPassesDepthTest(*depthTest, destinationOffset + i))
								destinationPixels[i] = runStreams[cursor + sourceOffset + i];
						}
					} else {
						memcpy(destinationPixels, runStreams.data() + cursor + sourceOffset, copyCount);
					}
				}
			}
		}

		cursor += pixelCount;
	}

	if (nextCursor != nullptr)
		*nextCursor = cursor - runBase;
	return lastRunY;
}

uint skipActorRunStream(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount) {
	cursor += runBase;
	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size())
			return cursor - runBase;

		cursor += 2;
		const uint pixelCount = runStreams[cursor++];
		if (cursor + pixelCount > runStreams.size())
			return cursor - runBase;

		cursor += pixelCount;
	}

	return cursor - runBase;
}

int drawActorPaletteRemapRunStream(const Common::Array<byte> &runStreams, uint cursor, uint runBase, uint runCount,
		int spriteX, int spriteY, int minimumYExclusive, Graphics::Surface &destination,
		const Common::Array<byte> &paletteRemapTable, const ActorDepthTest *depthTest) {
	const bool useDepthTest = actorDepthTestEnabled(depthTest);
	if (destination.format.bytesPerPixel != 1 || paletteRemapTable.size() < kPaletteRemapTableSize)
		return minimumYExclusive;

	cursor += runBase;
	int lastRunY = minimumYExclusive;

	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size())
			return lastRunY;

		const int xOffset = runStreams[cursor++];
		const int yOffset = runStreams[cursor++];
		uint copyCount = runStreams[cursor++];

		const int dstY = spriteY + yOffset;
		lastRunY = dstY;
		if (dstY <= minimumYExclusive || dstY < 0 || dstY >= HollywoodEngine::kSceneBufferHeight)
			continue;

		int dstX = spriteX + xOffset;
		if (dstX < 0) {
			const uint clipped = MIN<uint>(copyCount, (uint)-dstX);
			copyCount -= clipped;
			dstX = 0;
		}
		if (dstX + (int)copyCount > HollywoodEngine::kSceneBufferWidth)
			copyCount = MAX<int>(0, HollywoodEngine::kSceneBufferWidth - dstX);
		if (copyCount == 0 || dstX + copyCount > (uint)destination.w)
			continue;

		const uint destinationOffset = dstX + dstY * HollywoodEngine::kSceneBufferWidth;
		byte *destinationPixels = (byte *)destination.getBasePtr(dstX, dstY);
		for (uint i = 0; i < copyCount; ++i) {
			if (!useDepthTest || actorPixelPassesDepthTest(*depthTest, destinationOffset + i))
				destinationPixels[i] = paletteRemapTable[destinationPixels[i]];
		}
	}

	return lastRunY;
}

void drawActorSpriteFrame(const ActorSpriteBank &bank, byte facing, byte cel, int worldX, int worldY,
		int minimumYExclusive, Graphics::Surface &destination, const Common::Array<byte> &paletteRemapTable,
		const ActorDepthTest *depthTest) {
	if (facing >= kActorFacingCount || cel >= kActorCelsPerFacing)
		return;

	const uint descriptorIndex = facing * kActorCelsPerFacing + cel;
	if (descriptorIndex >= bank.descriptors.size())
		return;

	const ActorSpriteDescriptor &descriptor = bank.descriptors[descriptorIndex];
	const uint runBase = facing * kActorFacingRunStride;
	if (runBase + descriptor.runStreamOffset >= bank.runStreams.size())
		return;

	const int spriteX = worldX - descriptor.anchorX;
	const int spriteY = worldY - descriptor.anchorY;
	const uint paletteRunCursor = skipActorRunStream(bank.runStreams,
		descriptor.runStreamOffset, runBase, descriptor.opaqueRunCount);
	drawActorPaletteRemapRunStream(bank.runStreams, paletteRunCursor, runBase,
		descriptor.paletteRunCount, spriteX, spriteY, minimumYExclusive, destination,
		paletteRemapTable, depthTest);
	drawActorRunStream(bank.runStreams, descriptor.runStreamOffset, runBase,
		descriptor.opaqueRunCount, spriteX, spriteY, minimumYExclusive, destination, depthTest);
}

void buildPresentationPaletteRemapTable(const Common::Array<byte> &palette, Common::Array<byte> &paletteRemapTable) {
	paletteRemapTable.resize(kPaletteRemapTableSize);
	if (palette.size() < kPaletteRemapTableSize * kPaletteTripletSize) {
		for (uint i = 0; i < paletteRemapTable.size(); ++i)
			paletteRemapTable[i] = (byte)i;
		return;
	}

	for (uint color = 0; color < paletteRemapTable.size(); ++color)
		paletteRemapTable[color] = nearestDarkerPaletteColor(palette, (byte)color);
}

} // End of namespace Hollywood
