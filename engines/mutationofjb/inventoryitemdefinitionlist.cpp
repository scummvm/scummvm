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

#include "mutationofjb/inventoryitemdefinitionlist.h"

#include "mutationofjb/encryptedfile.h"
#include "mutationofjb/util.h"

namespace MutationOfJB {

InventoryItemDefinitionList::InventoryItemDefinitionList() {
	parseFile();
}

int InventoryItemDefinitionList::findItemIndex(const Common::String &itemName) {
	const InventoryItemMap::const_iterator it = _inventoryItemMap.find(itemName);
	if (it == _inventoryItemMap.end())
		return -1;
	return it->_value;
}

bool InventoryItemDefinitionList::parseFile() {
	EncryptedFile file;
	const char *fileName = "fixitems.dat";
	file.open(fileName);
	if (!file.isOpen()) {
		reportFileMissingError(fileName);
		return false;
	}

	int itemIndex = 0;
	while (!file.eos()) {
		Common::String line = file.readLine();
		if (line.empty() || line.hasPrefix("#")) {
			continue;
		}

		Common::String::const_iterator firstSpace = Common::find(line.begin(), line.end(), ' ');
		if (firstSpace == line.end()) {
			continue;
		}
		const int len = firstSpace - line.begin();
		if (!len) {
			continue;
		}
		Common::String item(line.c_str(), len);
		_inventoryItemMap[item] = itemIndex;
		itemIndex++;
	}

	return true;
}

}
