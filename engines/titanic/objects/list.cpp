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

#include "titanic/objects/list.h"

namespace Titanic {

void ListItem::save(SimpleFile *file, int indent) const {
	// Should always be overriden in descendents, so just write a dummy value
	file->writeNumberLine(0, indent);
}

void ListItem::load(SimpleFile *file) {
	// Should always be overriden in descendents, so just read the dummy value
	file->readNumber();
}

/*------------------------------------------------------------------------*/

List::List() {
}

void List::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(0, indent);
	saveItems(file, indent);
}

void List::load(SimpleFile *file) {
	file->readNumber();
	loadItems(file);
}

void List::saveItems(SimpleFile *file, int indent) const {
	// Write out number of items
	file->writeQuotedLine("L", indent);
	file->writeNumberLine(size(), indent);

	// Iterate through writing entries
	List::const_iterator i;
	for (i = begin(); i != end(); ++i) {
		const ListItem *item = *i;
		item->saveHeader(file, indent);
		item->save(file, indent + 1);
		item->saveFooter(file, indent);
	}
}

void List::loadItems(SimpleFile *file) {
	file->readBuffer();
	uint count = file->readNumber();
	
	for (uint idx = 0; idx < count; ++idx) {
		// Validate the class start header
		if (!file->IsClassStart())
			error("Unexpected class end");

		// Get item's class name and use it to instantiate an item
		CString className = file->readString();
		CSaveableObject *newItem = CSaveableObject::createInstance(className);
		if (!newItem)
			error("Could not create instance of %s", className.c_str());

		// Validate the class end footer
		if (file->IsClassStart())
			error("Unexpected class start");
	}
}

} // End of namespace Titanic
