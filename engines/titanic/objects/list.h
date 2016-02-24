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

#ifndef TITANIC_LIST_H
#define TITANIC_LIST_H

#include "common/scummsys.h"
#include "common/list.h"
#include "titanic/simple_file.h"
#include "titanic/objects/saveable_object.h"

namespace Titanic {

/**
 * Base list item class
 */
class ListItem: public CSaveableObject {
public:
	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "ListItem"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);
};

template<typename T>
class List : public CSaveableObject, public Common::List<T *> {
public:
	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return nullptr; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const {
		file->writeNumberLine(0, indent);

		// Write out number of items
		file->writeQuotedLine("L", indent);
		file->writeNumberLine(Common::List<T *>::size(), indent);

		// Iterate through writing entries
		Common::List<T *>::const_iterator i;
		for (i = Common::List<T *>::begin(); i != Common::List<T *>::end(); ++i) {
			const ListItem *item = *i;
			item->saveHeader(file, indent);
			item->save(file, indent + 1);
			item->saveFooter(file, indent);
		}

	}

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file) {
		file->readNumber();
		file->readBuffer();

		Common::List<T *>::clear();
		uint count = file->readNumber();

		for (uint idx = 0; idx < count; ++idx) {
			// Validate the class start header
			if (!file->IsClassStart())
				error("Unexpected class end");

			// Get item's class name and use it to instantiate an item
			CString className = file->readString();
			T *newItem = dynamic_cast<T *>(CSaveableObject::createInstance(className));
			if (!newItem)
				error("Could not create instance of %s", className.c_str());

			// Load the item's data and add it to the list
			newItem->load(file);
			Common::List<T *>::push_back(newItem);

			// Validate the class end footer
			if (file->IsClassStart())
				error("Unexpected class start");
		}
	}

	/**
	 * Clear the list and destroy any items in it
	 */
	void destroyContents() {
		for (Common::List<T *>::iterator i = Common::List<T *>::begin(); 
				i != Common::List<T *>::end(); ++i) {
			CSaveableObject *obj = *i;
			delete obj;
		}

		Common::List<T *>::clear();
	}

	/**
	 * Add a new item to the list of the type the list contains
	 */
	T *List::add() {
		T *item = new T();
		Common::List<T *>::push_back(item);
		return item;
	}
};

} // End of namespace Titanic

#endif /* TITANIC_LIST_H */
