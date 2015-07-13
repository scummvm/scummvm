/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARK_GAME_WINDOW_H
#define STARK_GAME_WINDOW_H

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/ui/window.h"

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/array.h"

namespace Stark {

class ActionMenu;
class InventoryInterface;

class GameWindow : public Window {
public:
	GameWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu, InventoryInterface *inventory);
	virtual ~GameWindow() {}

protected:
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRightClick(const Common::Point &pos) override;
	void onRender() override;

	void checkObjectAtPos(Common::Point pos, int16 selectedTool, int16 &possibleTool);

	ActionMenu *_actionMenu;
	InventoryInterface *_inventory;

	Gfx::RenderEntryArray _renderEntries;
	Resources::ItemVisual *_objectUnderCursor;
	Common::Point _objectRelativePosition;
};

} // End of namespace Stark

#endif // STARK_GAME_WINDOW_H
