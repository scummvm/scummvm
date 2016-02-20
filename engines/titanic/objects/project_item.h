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

#ifndef TITANIC_PROJECT_ITEM_H
#define TITANIC_PROJECT_ITEM_H

#include "common/scummsys.h"
#include "titanic/simple_file.h"
#include "titanic/objects/file_item.h"

namespace Titanic {

class CProjectItem : public CFileItem {
private:
	CString _filename;
private:
	/**
	 * Load project data from the passed file
	 */
	CProjectItem *loadData(SimpleFile *file);

	/**
	 * Save project data to the passed file
	 */
	void saveData(SimpleFile *file, CTreeItem *item) const;
public:
	/**
	 * Return the class name
	 */
	virtual const char *getClassName() const { return "CProjectItem"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);	

	/**
	 * Load the entire project data for a given slot Id
	 */
	void loadGame(int slotId);

	/**
	 * Save the entire project data to a given savegame slot
	 */
	void saveGame(int slotId);

	/**
	 * Clear any currently loaded project
	 */
	void clear();
};

} // End of namespace Titanic

#endif /* TITANIC_PROJECT_ITEM_H */
