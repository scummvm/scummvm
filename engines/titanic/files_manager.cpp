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

#include "common/file.h"
#include "titanic/files_manager.h"
#include "titanic/game_manager.h"

namespace Titanic {

CFilesManager::CFilesManager() : _gameManager(nullptr), 
		_assetsPath("Assets"), _field0(0), _field14(0), 
		_field18(0), _field1C(0), _field3C(0) {
}

bool CFilesManager::fn1(const CString &name) {
	if (name.empty())
		return 0;

	CString str = name;
	str.toLowercase();
	
	if (str[0] == 'z' || str[0] == 'y') {
		return 1;
	} else if (str[0] < 'a' || str[0] > 'c') {
		return 0;
	}

	CString tempStr = str;
	int idx = tempStr.indexOf('#');
	if (idx >= 0) {
		tempStr = tempStr.left(idx);
		str = str.c_str() + idx + 1;
		str += ".st";
	}



	return true;
}

bool CFilesManager::fileExists(const CString &name) {
	Common::File f;
	return f.exists(name);
}

} // End of namespace Titanic
