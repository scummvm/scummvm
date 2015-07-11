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

#ifndef STARK_INVENTORYINTERFACE_H
#define STARK_INVENTORYINTERFACE_H

#include "engines/stark/gfx/renderentry.h"

#include "engines/stark/ui/window.h"

#include "common/scummsys.h"
#include "common/rect.h"
#include "common/array.h"

namespace Stark {

class ActionMenu;

namespace Resources {
class Anim;
}

class InventoryInterface : public Window {
public:
	InventoryInterface(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu);
	virtual ~InventoryInterface() {}

	void open();
	void close();

	void setSelectedInventoryItem(uint16 selectedItem);

protected:
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRightClick(const Common::Point &pos) override;
	void onRender() override;

	void checkObjectAtPos(Common::Point pos, Resources::ItemVisual **item, int16 selectedTool, int16 &possibleTool);
	Common::Rect getSlotRect(uint32 slot) const;
	Common::Rect getItemRect(uint32 slot, VisualImageXMG *image) const;

private:
	ActionMenu *_actionMenu;

	VisualImageXMG *_backgroundTexture;
	Common::Rect _backgroundRect;

	int16 _selectedInventoryItem;
};

} // End of namespace Stark

#endif // STARK_INVENTORYINTERFACE_H
