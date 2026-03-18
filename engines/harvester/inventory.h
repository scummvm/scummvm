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

#ifndef HARVESTER_INVENTORY_H
#define HARVESTER_INVENTORY_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "harvester/art.h"
#include "harvester/room_support.h"

namespace Graphics {
class Screen;
}

namespace Harvester {

class HarvesterEngine;

struct StartupInventoryVisual {
	StartupObjectRecord object;
	IndexedBitmap bitmap;
	Common::Rect bounds;
	bool hasBitmap = false;
};

class InventorySystem {
public:
	explicit InventorySystem(HarvesterEngine &engine);

	bool refresh();
	bool open();
	bool close();
	bool clearSelection();
	bool isOpen() const;
	bool hasSelection() const;
	const Common::String &getSelectedItemName() const;
	Common::String resolveSelectedLabel() const;
	Common::String buildSelectedPrompt(const Common::String &targetLabel) const;
	void selectItem(const Common::String &objectName);
	void setPromptText(const Common::String &promptText);
	const Common::String &getPromptText() const;
	const StartupInventoryVisual *findItemAtPoint(const Common::Point &point) const;
	Common::Rect getPanelBounds() const;
	void drawOverlay(Graphics::Screen &screen) const;
	void drawSelectedDragItem(Graphics::Screen &screen, const Common::Point &point) const;

	static bool isExitObject(const StartupObjectRecord &object);
	static bool isStatusObject(const StartupObjectRecord &object);

private:
	Common::Array<StartupInventoryVisual> _items;
	bool _open = false;
	Common::String _selectedItemName;
	Common::String _promptText;
	HarvesterEngine &_engine;
};

} // End of namespace Harvester

#endif // HARVESTER_INVENTORY_H
