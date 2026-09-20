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

#ifndef SCOOBY_FRAME_PRESENTER_H
#define SCOOBY_FRAME_PRESENTER_H

#include "common/func.h"

#include "scooby/common/span.h"
#include "scooby/graphics/indexed_frame.h"
#include "scooby/graphics/palette_color.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/raylib_host.h"
#include "scooby/rendering/frame_clock.h"

// Owns retrace pacing, logical-frame presentation, and palette transitions
// shared by recovered sequences.

namespace Scooby {

class FramePresenter {
public:
	FramePresenter(TileScene &scene, IndexedFrame &frame, RaylibHost &host,
				   FrameClock &clock);

	// Ghidra fadePaletteOut (0x00009DE0), including its leading retrace.
	// Returns false when the host requests shutdown during the fade.
	bool fadePaletteOut(const Common::Functor0<void> *beforeRetrace = nullptr);

	// Ghidra waitForFrames (0x00009742). The 68000 DBF loop waits
	// frameCounter plus one retraces. Returns false when the host requests
	// shutdown during the wait.
	bool waitForFrames(int frameCounter, const Common::Functor0<void> *beforeRetrace = nullptr);

	// Ghidra waitForVerticalBlank (0x0000974C) at the raylib host boundary.
	// Returns false after the native window receives a close request.
	bool waitForVerticalBlank();

	// Ghidra fadePaletteIn (0x00009E7E) for a target assembled from one or
	// more recovered asset ranges. Returns false when the host requests
	// shutdown during the fade.
	bool fadePaletteIn(Span<const PaletteColor> targetPalette,
					   const Common::Functor0<void> *beforeRetrace = nullptr);

private:
	FrameClock &_clock;
	IndexedFrame &_frame;
	RaylibHost &_host;
	TileScene &_scene;
};
} // namespace Scooby

#endif // SCOOBY_FRAME_PRESENTER_H
