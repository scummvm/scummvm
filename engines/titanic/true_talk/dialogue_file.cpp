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

#include "titanic/true_talk/dialogue_file.h"

namespace Titanic {

CDialogueFile::CDialogueFile(const CString &filename, uint count) {
	if (!_file.open(filename))
		error("Could not locate dialogue file - %s", filename.c_str());
	
	_data1.resize(count);

	_file.readUint32LE();		// Skip over file Id
	_entries.resize(_file.readUint32LE());

	// Read in the entries
	for (uint idx = 0; idx < _entries.size(); ++idx) {
		_entries[idx].v1 = _file.readUint32LE();
		_entries[idx].v2 = _file.readUint32LE();
	}
}

CDialogueFile::~CDialogueFile() {
	clear();
}

void CDialogueFile::clear() {
	_file.close();
}

} // End of namespace Titanic
