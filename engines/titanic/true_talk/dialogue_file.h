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

#ifndef TITANIC_DIALOGUE_FILE_H
#define TITANIC_DIALOGUE_FILE_H

#include "common/file.h"
#include "titanic/support/string.h"

namespace Titanic {

class CDialogueFile {
	struct CDialogueFileEntry {
		uint v1;
		uint v2;
	};
	struct EntryRec {
		uint v1, v2, v3, v4, v5;
	};
private:
	Common::File _file;
	Common::Array<CDialogueFileEntry> _entries;
	Common::Array<EntryRec> _data1;
public:
	CDialogueFile(const CString &filename, uint count);
	~CDialogueFile();

	/**
	 * Clear the loaded data
	 */
	void clear();
};

} // End of namespace Titanic

#endif /* TITANIC_TITLE_ENGINE_H */
