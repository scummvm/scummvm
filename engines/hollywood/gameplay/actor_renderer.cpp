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
		const ActorDepthTest *depthTest) {
	const bool useDepthTest = actorDepthTestEnabled(depthTest);
	if (destination.format.bytesPerPixel != 1)
		return minimumYExclusive;

	cursor += runBase;
	int lastRunY = minimumYExclusive;

	for (uint runIndex = 0; runIndex < runCount; ++runIndex) {
		if (cursor + 3 > runStreams.size())
			return lastRunY;

		const int xOffset = runStreams[cursor++];
		const int yOffset = runStreams[cursor++];
		const uint pixelCount = runStreams[cursor++];
		if (cursor + pixelCount > runStreams.size())
			return lastRunY;

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

	return lastRunY;
}

} // End of namespace Hollywood
