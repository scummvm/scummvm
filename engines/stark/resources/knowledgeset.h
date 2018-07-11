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

#ifndef STARK_RESOURCES_KNOWLEDGE_SET_H
#define STARK_RESOURCES_KNOWLEDGE_SET_H

#include "common/str.h"

#include "engines/stark/gfx/renderentry.h"
#include "engines/stark/resources/object.h"

namespace Stark {

namespace Formats {
class XRCReadStream;
}

namespace Resources {

class InventoryItem;

/**
 * A typed collection of Knowledge resources
 */
class KnowledgeSet : public Object {
public:
	static const Type::ResourceType TYPE = Type::kKnowledgeSet;

	enum SubType {
		kInventory = 1,
		kState     = 2,
		kPersons   = 3,
		kLocations = 4,
		kDiary     = 5
	};

	KnowledgeSet(Object *parent, byte subType, uint16 index, const Common::String &name);
	virtual ~KnowledgeSet();

	// Resource API
	void saveLoad(ResourceSerializer *serializer) override;

	/** Add an inventory item, and keep track of its acquisition order */
	void addItem(InventoryItem *item);

	/** Remove an inventory item */
	void removeItem(InventoryItem *item);

	/** Get the render entries for the inventory items, in the order they were obtained */
	Gfx::RenderEntryArray getInventoryRenderEntries() const;

	/** Get a cursor style visual for an inventory item */
	Visual *getInventoryItemVisual(uint16 itemIndex);

	/** Get the index of a neighbor inventory item of a selected item, return -1 in corner case */
	int16 getNeighborInventoryItem(int16 selectedItem, bool forward);

protected:
	void printData() override;

	Common::Array<uint16> _inventoryItemOrder;
};

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_KNOWLEDGE_SET_H
