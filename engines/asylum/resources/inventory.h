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

#ifndef ASYLUM_RESOURCES_INVENTORY_H
#define ASYLUM_RESOURCES_INVENTORY_H

#include "common/serializer.h"
#include "common/stream.h"

#include "asylum/asylum.h"

namespace Asylum {

class Inventory : public Common::Serializable {
public:
	Inventory(AsylumEngine *vm, int32 &multiple);

	uint find(uint item = 0) const;
	void add(uint item, uint multipleIncr);
	void remove(uint item, uint multipleDecr);
	bool contains(uint item, uint multiple) const;
	void copyFrom(Inventory &inventory);

	uint getSelectedItem() const { return _selectedItem; }
	void selectItem(uint item) { _selectedItem = item; }

	void load(Common::SeekableReadStream *stream);
	void saveLoadWithSerializer(Common::Serializer &s);

	uint32 &operator[](uint index) { return _items[index]; }

	static Common::Point getInventoryRingPoint(AsylumEngine *vm, uint nPoints, uint index);
	static void describe(AsylumEngine *vm, uint item);

private:
	uint32 _items[8];
	uint32 _selectedItem;
	int32 &_multiple;
	AsylumEngine *_vm;
};

} // End of namespace Asylum

#endif // ASYLUM_RESOURCES_INVENTORY_H
