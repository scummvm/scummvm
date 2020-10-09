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

#ifndef STARK_UI_INVENTORY_WINDOW_H
#define STARK_UI_INVENTORY_WINDOW_H

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

class InventoryWindow : public Window {
public:
	InventoryWindow(Gfx::Driver *gfx, Cursor *cursor, ActionMenu *actionMenu);
	~InventoryWindow() override {}

	void open();
	void close();

	void setSelectedInventoryItem(int16 selectedInventoryItem);
	int16 getSelectedInventoryItem() const;

	/** Clear the location dependent state */
	void reset();

	/** Scroll the inventory */
	void scrollDown();
	void scrollUp();

protected:
	void onMouseMove(const Common::Point &pos) override;
	void onClick(const Common::Point &pos) override;
	void onRightClick(const Common::Point &pos) override;
	void onGameLoop() override;
	void onRender() override;

	void checkObjectAtPos(Common::Point pos, Resources::ItemVisual **item, int16 selectedInventoryItem, int16 &singlePossibleAction);
	Common::Rect getSlotRect(uint32 slot) const;
	Common::Rect getItemRect(uint32 slot, VisualImageXMG *image) const;
	bool isSlotVisible(uint32 slot) const;
	bool canScrollUp() const;
	bool canScrollDown() const;
	void drawScrollArrows() const;

private:
	ActionMenu *_actionMenu;

	VisualImageXMG *_backgroundTexture;
	Common::Rect _backgroundRect;

	VisualImageXMG *_scrollUpArrowImage;
	VisualImageXMG *_scrollDownArrowImage;
	Common::Rect _scrollUpArrowRect;
	Common::Rect _scrollDownArrowRect;

	uint32 _firstVisibleSlot;
	static const uint32 _visibleSlotsCount = 15;

	Gfx::RenderEntryArray _renderEntries;
	int16 _selectedInventoryItem;

	int32 _autoCloseTimeRemaining;
};

} // End of namespace Stark

#endif // STARK_UI_INVENTORY_WINDOW_H
