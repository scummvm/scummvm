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

#ifndef TITANIC_FILES_MANAGER_H
#define TITANIC_FILES_MANAGER_H

#include "titanic/core/list.h"

namespace Titanic {

class CGameManager;

class CFilesManagerList : public List<ListItem> {
};

class CFilesManager {
private:
	CGameManager *_gameManager;
	CFilesManagerList _list;
	CString _string1;
	CString _string2;
	int _field0;
	int _field14;
	int _field18;
	int _field1C;
	int _field3C;
public:
	CFilesManager();

	/**
	 * Sets the game manager
	 */
	void setGameManager(CGameManager *gameManager) {
		_gameManager = gameManager;
	}

	int fn1(const CString &name);
};

} // End of namespace Titanic

#endif /* TITANIC_FILES_MANAGER_H */
