/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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

#ifndef RIVEN_INVENTORY_H
#define RIVEN_INVENTORY_H

#include "common/rect.h"

namespace Mohawk {

class MohawkEngine_Riven;

/**
 * The player's inventory
 *
 * Is responsible for drawing the bottom part of the screen.
 */
class RivenInventory {
public:
	explicit RivenInventory(MohawkEngine_Riven *vm);
	virtual ~RivenInventory();

	/** Handle a click event in the inventory area */
	void checkClick(const Common::Point &mousePos);

	/** Go back to the game from an inventory item detail view */
	void backFromItemScript() const;

	/** Make the inventory visible and draw it as necessary */
	void onFrame();

	/** Force the inventory to be visible even in situations where it usually is not */
	void forceVisible(bool visible);

	/** Force the inventory to be hidden even in situations where it usually is not */
	void forceHidden(bool hidden);

private:
	bool isVisible() const;
	void draw();
	void clearArea();

	MohawkEngine_Riven *_vm;

	bool _inventoryDrawn;
	bool _forceVisible;
	bool _forceHidden;

	// Rects for the inventory object positions
	Common::Rect _atrusJournalRect1;
	Common::Rect _atrusJournalRect2;
	Common::Rect _cathJournalRect2;
	Common::Rect _atrusJournalRect3;
	Common::Rect _cathJournalRect3;
	Common::Rect _trapBookRect3;
	Common::Rect _demoExitRect;
};

} // End of namespace Mohawk

#endif
