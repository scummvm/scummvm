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

#ifndef ULTIMA2_VIEWS_MAP_H
#define ULTIMA2_VIEWS_MAP_H

#include "ultima/shared/engine/view.h"
#include "ultima/ultima2/data/tiles.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {

using namespace Graphics::Views;

/**
 * Base class for map views - forwards player input into the active Logic
 */
class Map : public Shared::View {
public:
private:
	uint32 _lastAnimation = 0;
	uint32 _lastInput = 0;

protected:
	/**
	 * Returns the view's TILE_COUNT tile graphics
	 */
	virtual Graphics::Surface *tileGraphics() = 0;

	/**
	 * Passes the turn if no command has been given for a while
	 */
	void checkIdle();

public:
	/**
	 * Re-reads whatever tile/sprite graphics this view uses for the
	 * current render mode. A no-op by default; overridden by subclasses
	 * that actually own tile surfaces (OverworldMap, LocationMap), since
	 * those are constructed once up front as part of Views::Views and so
	 * don't otherwise notice a render mode change made after the fact
	 */
	virtual void reloadTiles() {}

	Map(const Common::String &name) : View(name) {
		setBounds(Graphics::Views::TextRect(0, 0, 39, 19));
	}
	~Map() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool tick() override;
};

} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
