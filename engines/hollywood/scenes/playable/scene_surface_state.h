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

#ifndef HOLLYWOOD_SCENES_PLAYABLE_SCENE_SURFACE_STATE_H
#define HOLLYWOOD_SCENES_PLAYABLE_SCENE_SURFACE_STATE_H

#include "common/array.h"
#include "common/types.h"
#include "graphics/managed_surface.h"

#include "hollywood/graphics.h"

namespace Hollywood {

/**
 * Owns the palettes, masks, and indexed surfaces used by a playable scene.
 *
 * paletteResource is the room palette, paletteCurrent is the mutable display
 * palette, and actorPaletteBase holds the actor colors before scene lighting.
 *
 * baseFramebufferOriginal is the restoration copy of the selected room image;
 * baseFramebuffer carries persistent scene patches; sceneFramebuffer is the
 * per-frame composite. savedFramebuffer is a separate resource-derived image
 * used for region, hotspot, and depth lookup and panel restoration, not a
 * save-game snapshot. screen is the cropped presentation surface.
 */
class SceneSurfaceState {
public:
	void initialize(uint paletteSize, uint paletteMaskOriginalSize, uint paletteMaskUsedBytes,
		uint paletteMapPageSize, uint paletteRegionCount, uint actorPaletteBaseSize);
	void initializeFramebuffers();
	uint framebufferByteCount() const;
	byte *framebufferPixels(Graphics::ManagedSurface &surface);
	const byte *framebufferPixels(const Graphics::ManagedSurface &surface) const;
	void copyBaseFramebufferToSceneFramebuffer();
	void restoreBaseFramebufferFromOriginal();
	bool isFramebufferOffsetValid(uint offset) const;
	byte savedFramebufferPixelAt(uint offset) const;
	void setPaletteEntry6Bit(byte colorIndex, byte red, byte green, byte blue);
	byte paletteEntryComponent6Bit(byte colorIndex, uint component) const;
	void rebuildPresentationPaletteRemapTable();

	Common::Array<byte> paletteResource;
	Common::Array<byte> paletteCurrent;
	Common::Array<byte> actorPaletteBase;
	Graphics::ManagedSurface baseFramebufferOriginal;
	Graphics::ManagedSurface baseFramebuffer;
	Graphics::ManagedSurface sceneFramebuffer;
	Graphics::ManagedSurface savedFramebuffer;
	Common::Array<byte> fillRuns;
	Common::Array<byte> paletteMaskOriginal;
	Common::Array<byte> paletteMask;
	Common::Array<byte> fullPaletteRegionMask;
	Common::Array<byte> walkablePaletteMask;
	Common::Array<byte> colorToActorDepthClassMap;
	Common::Array<byte> presentationPaletteRemapTable;
	Common::Array<uint16> actorDepthYThresholds;
	Common::Array<uint16> drawActorDepthYThresholds;
	Graphics::ManagedSurface screen;
	Palette6Bit displayPalette;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_SURFACE_STATE_H
