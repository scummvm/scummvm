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

#ifndef SCOOBY_STUDIO_LOGO_SEQUENCE_H
#define SCOOBY_STUDIO_LOGO_SEQUENCE_H

#include "common/func.h"

#include "scooby/assets/rom.h"
#include "scooby/common/span.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/presentation/frame_presenter.h"
#include "scooby/raylib_host.h"

// Owns the tile, tilemap, palette, and timing sequence that bridges the
// intro into the main menu.

namespace Scooby {

class StudioLogoSequence {
public:
	// Binds the recovered studio-logo assets to the shared scene and
	// retrace presenter. handleRoomVBlank is the room callback installed
	// before the startup logo sequence begins.
	StudioLogoSequence(const ScoobyDooRom &rom, TileScene &scene,
					   FramePresenter &presenter,
					   RaylibHost &host, Common::Functor0<void> &handleRoomVBlank);

	// Runs the opening portion of Ghidra RunAdventure at 0x00000BF4 without
	// changing its asset offsets, inclusive waits, or tilemap upload order.
	// Returns false when the host requests shutdown during a retrace wait.
	//
	// Ring-LZ assets at 0x000287C8, 0x00028220, and 0x0002864C provide
	// tiles, staged bands, and the final map. Palette 0x0002967A is shared
	// by both logo phases. The staged map is published as 20x5 cells at
	// logical foreground origin (8,8), four cycles of fifteen bands, with
	// each inclusive delay producing three retraces.
	bool run();

private:
	enum class StepResult {
		Completed,
		Skipped,
		HostClosed
	};

	StepResult runAnimatedStep(Span<const uint8> stagedTilemap,
							   Span<const PaletteColor> targetPalette);
	StepResult runFinalStep();
	StepResult fadeIn(Span<const PaletteColor> targetPalette);
	StepResult fadeOut();
	StepResult waitForFrames(int frameCounter);
	StepResult waitForFrame();

	// Ghidra uploadStagedTilemapBand (0x00000BC8) as one logical layer
	// update; its hardware-only DMA/Z80 bus path has no managed counterpart.
	void uploadStagedTilemapBand(Span<const uint8> source, int band);

	Common::Functor0<void> &_handleRoomVBlank;
	FramePresenter &_presenter;
	RaylibHost &_host;
	const ScoobyDooRom &_rom;
	TileScene &_scene;
};
} // namespace Scooby

#endif // SCOOBY_STUDIO_LOGO_SEQUENCE_H
