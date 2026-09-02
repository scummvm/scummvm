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
 * _paletteResource is the room palette, _paletteCurrent is the mutable display
 * palette, and _actorPaletteBase holds the actor colors before scene lighting.
 *
 * _baseFramebufferOriginal is the restoration copy of the selected room image;
 * _baseFramebuffer carries persistent scene patches and supplies clean visual
 * regions; _sceneFramebuffer is the per-frame composite. _savedFramebuffer is a
 * separate resource-derived image used for region, hotspot, and depth lookup,
 * not a save-game snapshot. _screen is the cropped presentation surface;
 * _presentedScreen caches its last backend submission.
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
	bool updatePresentedScreenCache();

	Common::Array<byte> _paletteResource;
	Common::Array<byte> _paletteCurrent;
	Common::Array<byte> _actorPaletteBase;
	Graphics::ManagedSurface _baseFramebufferOriginal;
	Graphics::ManagedSurface _baseFramebuffer;
	Graphics::ManagedSurface _sceneFramebuffer;
	Graphics::ManagedSurface _savedFramebuffer;
	Common::Array<byte> _fillRuns;
	Common::Array<byte> _paletteMaskOriginal;
	Common::Array<byte> _paletteMask;
	Common::Array<byte> _fullPaletteRegionMask;
	Common::Array<byte> _walkablePaletteMask;
	Common::Array<byte> _colorToActorDepthClassMap;
	Common::Array<byte> _presentationPaletteRemapTable;
	Common::Array<uint16> _actorDepthYThresholds;
	Common::Array<uint16> _drawActorDepthYThresholds;
	Graphics::ManagedSurface _screen;
	Graphics::ManagedSurface _presentedScreen;
	Palette6Bit _displayPalette;
	bool _hasPresentedScreen;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_SCENES_PLAYABLE_SCENE_SURFACE_STATE_H
