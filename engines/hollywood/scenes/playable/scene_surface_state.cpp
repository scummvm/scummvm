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

#include "hollywood/scenes/playable/scene_surface_state.h"

#include "common/rect.h"
#include "graphics/pixelformat.h"

#include "hollywood/gameplay/actor_renderer.h"
#include "hollywood/hollywood.h"

namespace Hollywood {

void SceneSurfaceState::initialize(uint paletteSize, uint paletteMaskOriginalSize,
		uint paletteMaskUsedBytes, uint paletteMapPageSize, uint paletteRegionCount,
		uint actorPaletteBaseSize) {
	paletteResource.resize(paletteSize);
	paletteCurrent.resize(paletteSize);
	actorPaletteBase.resize(actorPaletteBaseSize);
	initializeFramebuffers();
	paletteMaskOriginal.resize(paletteMaskOriginalSize);
	fullPaletteRegionMask.resize(paletteMaskUsedBytes);
	walkablePaletteMask.resize(paletteMaskUsedBytes);
	colorToActorDepthClassMap.resize(paletteMapPageSize);
	presentationPaletteRemapTable.resize(paletteMapPageSize);
	for (uint i = 0; i < presentationPaletteRemapTable.size(); ++i)
		presentationPaletteRemapTable[i] = 0;
	actorDepthYThresholds.resize(paletteRegionCount);
	drawActorDepthYThresholds.resize(paletteRegionCount);
	screen.create(HollywoodEngine::kScreenWidth, HollywoodEngine::kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
}

void SceneSurfaceState::initializeFramebuffers() {
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	baseFramebufferOriginal.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);
	baseFramebuffer.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);
	sceneFramebuffer.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);
	savedFramebuffer.create(HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight, format);
}

uint SceneSurfaceState::framebufferByteCount() const {
	return HollywoodEngine::kSceneBufferWidth * HollywoodEngine::kSceneBufferHeight;
}

byte *SceneSurfaceState::framebufferPixels(Graphics::ManagedSurface &surface) {
	return (byte *)surface.getPixels();
}

const byte *SceneSurfaceState::framebufferPixels(const Graphics::ManagedSurface &surface) const {
	return (const byte *)surface.getPixels();
}

void SceneSurfaceState::copyBaseFramebufferToSceneFramebuffer() {
	sceneFramebuffer.copyRectToSurface(baseFramebuffer.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
}

void SceneSurfaceState::restoreBaseFramebufferFromOriginal() {
	if (baseFramebufferOriginal.empty())
		return;

	baseFramebuffer.copyRectToSurface(baseFramebufferOriginal.rawSurface(), 0, 0,
		Common::Rect(0, 0, HollywoodEngine::kSceneBufferWidth, HollywoodEngine::kSceneBufferHeight));
}

bool SceneSurfaceState::isFramebufferOffsetValid(uint offset) const {
	return offset < framebufferByteCount();
}

byte SceneSurfaceState::savedFramebufferPixelAt(uint offset) const {
	if (!isFramebufferOffsetValid(offset))
		return 0;

	const uint x = offset % HollywoodEngine::kSceneBufferWidth;
	const uint y = offset / HollywoodEngine::kSceneBufferWidth;
	return *(const byte *)savedFramebuffer.getBasePtr(x, y);
}

void SceneSurfaceState::setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue) {
	const uint paletteOffset = colorIndex * 3;
	if (paletteCurrent.size() <= paletteOffset + 2)
		return;

	paletteCurrent[paletteOffset] = red;
	paletteCurrent[paletteOffset + 1] = green;
	paletteCurrent[paletteOffset + 2] = blue;
}

byte SceneSurfaceState::paletteEntryComponent6Bit(byte colorIndex, uint component) const {
	if (component >= 3)
		return 0;

	const uint paletteOffset = colorIndex * 3 + component;
	if (paletteOffset >= paletteCurrent.size())
		return 0;

	return paletteCurrent[paletteOffset];
}

void SceneSurfaceState::rebuildPresentationPaletteRemapTable() {
	buildPresentationPaletteRemapTable(paletteCurrent, presentationPaletteRemapTable);
}

} // End of namespace Hollywood
