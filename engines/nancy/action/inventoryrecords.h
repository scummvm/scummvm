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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_INVENTORYRECORDS_H
#define NANCY_ACTION_INVENTORYRECORDS_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Simply adds an item to the player's inventory.
class AddInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _itemID = 0;
	bool _setCursor = false;
	bool _forceCursor = false;

protected:
	Common::String getRecordTypeName() const override { return "AddInventoryNoHS"; }
};

// Simply removes an item from the player's inventory.
class RemoveInventoryNoHS : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint _itemID;

protected:
	Common::String getRecordTypeName() const override { return "RemoveInventoryNoHS"; }
};

// Displays a static image inside the viewport. The static image corresponds to an
// inventory item, and is only displayed if the item is not in the player's possession.
// On click, it hides the image and adds the item to the inventory.
class ShowInventoryItem : public RenderActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	ShowInventoryItem() : RenderActionRecord(9) {}
	virtual ~ShowInventoryItem() { _fullSurface.free(); }

	void init() override;

	uint16 _objectID = 0;
	Common::Path _imageName;
	Common::Array<FrameBlitDescription> _blitDescriptions;

	int16 _drawnFrameID = -1;
	Graphics::ManagedSurface _fullSurface;

protected:
	bool canHaveHotspot() const override { return true; }
	Common::String getRecordTypeName() const override { return "ShowInventoryItem"; }
	bool isViewportRelative() const override { return true; }
};

// When clicking an ActionRecord hotspot with a kItem dependency, the engine
// checks if the required item is currently being held; when it isn't, it plays
// a specific sound to inform the player they need some item. This AR changes that
// sound and its related caption (or stops it from playing entirely).
class InventorySoundOverride : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	byte _command = 0;
	uint16 _itemID = 0;
	SoundDescription _sound;
	Common::String _caption;

protected:
	Common::String getRecordTypeName() const override { return "InventorySoundOverride"; }
};

// Temporarily disable (or re-enable) clicking on a specific item in the inventory box
class EnableDisableInventory : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

	uint16 _itemID = 0;
	byte _disabledState = 0;

protected:
	Common::String getRecordTypeName() const override { return "EnableDisableInventory"; }
};

// Pops the scene and item that get pushed when a player clicks a kInvItemNewSceneView item
class PopInvViewPriorScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "PopInvViewPriorScene"; }
};

class GoInvViewScene : public ActionRecord {
public:
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;

protected:
	Common::String getRecordTypeName() const override { return "GoInvViewScene"; }

	uint16 _itemID = 0;
	bool _addToInventory = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_INVENTORYRECORDS_H
