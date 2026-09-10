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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_SECTOR_SCAN_H
#define ULTIMA1_VIEWS_INTERACTIONS_SECTOR_SCAN_H

#include "ultima/ultima1/views/interactions/interaction.h"
#include "ultima/shared/gfx/gfx_surface.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * The outer-space Inform command's "Sector Scan" dialog (SPACE.EXE's own
 * inform): a 7x7 grid of the galaxy, one small glyph per sector showing
 * whether it holds hostile ships, a station/encounter, a star hazard, or
 * nothing, with the player's current sector blinking. Any key closes it.
 */
class SectorScan : public Interaction {
private:
	int _blinkCtr = 0;
	bool _blipVisible = true;

	void drawSectorIcon(Shared::Gfx::GfxSurface &s, int sx, int sy);
	void drawEnemyMarker(Shared::Gfx::GfxSurface &s, int x, int y);
	void drawStationMarker(Shared::Gfx::GfxSurface &s, int x, int y);
	void drawStarMarker(Shared::Gfx::GfxSurface &s, int x, int y);
	void drawEmptyMarker(Shared::Gfx::GfxSurface &s, int x, int y);

public:
	SectorScan() : Interaction("SectorScan") {
	}
	~SectorScan() override {
	}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void draw() override;
	bool tick() override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
