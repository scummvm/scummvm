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
#include "titanic/support/simple_file.h"
#include "titanic/core/saveable_object.h"

namespace Titanic {

/**
 * Base list item class
 */
class ListItem: public CSaveableObject {
public:
	CLASSDEF;

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override;
};

/**
 * List item macro for managed pointers an item
 */
#define PTR_LIST_ITEM(T) class T##ListItem : public ListItem { \
	public: T *_item; \
	T##ListItem() : _item(nullptr) {} \
	T##ListItem(T *item) : _item(item) {} \
	virtual ~T##ListItem() { delete _item; } \
	}

template<typename T>
class PtrListItem : public ListItem {
public:
	T *_item;
public:
	PtrListItem() : _item(nullptr) {}
	PtrListItem(T *item) : _item(item) {}
	~PtrListItem() override { delete _item; }
};

template<typename T>
class List : public CSaveableObject, public Common::List<T *> {
public:
	~List() override { destroyContents(); }

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file, int indent) override {
		file->writeNumberLine(0, indent);

		// Write out number of items
		file->writeQuotedLine("L", indent);
		file->writeNumberLine(Common::List<T *>::size(), indent);

		// Iterate through writing entries
		typename Common::List<T *>::iterator i;
		for (i = Common::List<T *>::begin(); i != Common::List<T *>::end(); ++i) {
			ListItem *item = *i;
			item->saveHeader(file, indent);
			item->save(file, indent + 1);
			item->saveFooter(file, indent);
		}

	}

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file) override {
		file->readNumber();
		file->readBuffer();

		Common::List<T *>::clear();
		uint count = file->readNumber();

		for (uint idx = 0; idx < count; ++idx) {
			// Validate the class start header
			if (!file->isClassStart())
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
			if (file->isClassStart())
				error("Unexpected class start");
		}
	}

	/**
	 * Clear the list and destroy any items in it
	 */
	void destroyContents() {
		typename Common::List<T *>::iterator i;
		for (i = Common::List<T *>::begin();
				i != Common::List<T *>::end(); ++i) {
			CSaveableObject *obj = *i;
			delete obj;
		}

		Common::List<T *>::clear();
	}

	/**
	 * Add a new item to the list of the type the list contains
	 */
	T *add() {
		T *item = new T();
		Common::List<T *>::push_back(item);
		return item;
	}

	bool contains(const T *item) const {
		for (typename Common::List<T *>::const_iterator i = Common::List<T *>::begin();
				i != Common::List<T *>::end(); ++i) {
			if (*i == item)
				return true;
		}

		return false;
	}
};

} // End of namespace Titanic

#endif /* TITANIC_LIST_H */
