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
#include "titanic/core/file_item.h"
#include "titanic/core/list.h"

namespace Titanic {

class CGameManager;
class CPetControl;

/**
 * File list item
 */
class CFileListItem : public ListItem {
public:
	CString _name;

	virtual const char *getClassName() const { return "CFileListItem"; }

	/**
	 * Save the data for the class to file
	 */
	virtual void save(SimpleFile *file, int indent) const;

	/**
	 * Load the data for the class from file
	 */
	virtual void load(SimpleFile *file);	
	
};

/**
 * Filename list
 */
class CFileList: public List<CFileListItem> {
public:
	virtual const char *getClassName() const { return "CFileList"; }
};


class CProjectItem : public CFileItem {
private:
	CString _filename;
	CFileList _files;
	int _nextRoomNumber;
	int _nextMessageNumber;
	int _nextObjectNumber;
	CGameManager *_gameManager;

	/**
	 * Called during save, iterates through the children to do some stuff
	 */
	void buildFilesList();
private:
	/**
	 * Load project data from the passed file
	 */
	CProjectItem *loadData(SimpleFile *file);

	/**
	 * Save project data to the passed file
	 */
	void saveData(SimpleFile *file, CTreeItem *item) const;

	/**
	 * Does post-loading processing
	 */
	void gameLoaded();
public:
	CProjectItem();

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
	 * Get the game manager for the project
	 */
	virtual CGameManager *getGameManager();

	/**
	 * Get a reference to the PET control
	 */
	CPetControl *getPetControl();

	/**
	 * Resets the game manager field
	 */
	void resetGameManager();

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

	/**
	 * Set the proejct's name
	 */
	void setFilename(const CString &name) { _filename = name; }
};

} // End of namespace Titanic

#endif /* TITANIC_PROJECT_ITEM_H */
