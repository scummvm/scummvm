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

#ifndef GARGOYLE_FILES_H
#define GARGOYLE_FILES_H

#include "gargoyle/glk_types.h"
#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"

namespace Gargoyle {

enum FileUsage {
	fileusage_Data        = 0x00,
	fileusage_SavedGame   = 0x01,
	fileusage_Transcript  = 0x02,
	fileusage_InputRecord = 0x03,
	fileusage_TypeMask    = 0x0f,

	fileusage_TextMode    = 0x100,
	fileusage_BinaryMode  = 0x000,
};

enum FileMode {
	filemode_Write       = 0x01,
	filemode_Read        = 0x02,
	filemode_ReadWrite   = 0x03,
	filemode_WriteAppend = 0x05,
};

enum SeekMode {
	seekmode_Start   = 0,
	seekmode_Current = 1,
	seekmode_End     = 2,
};


/**
 * File details
 */
struct FileReference {
	glui32 _rock;
	int _slotNumber;
	Common::String _description;
	Common::String _filename;
	FileUsage _fileType;
	bool _textMode;
	gidispatch_rock_t _dispRock;

	/**
	 * Constructor
	 */
	FileReference() : _rock(0), _slotNumber(-1), _fileType(fileusage_Data), _textMode(false) {}

	/**
	 * Get savegame filename
	 */
	const Common::String getSaveName() const;

	/**
	 * Returns true if the given file exists
	 */
	bool exists() const;

	/**
	 * Delete the given file
	 */
	void deleteFile();
};

typedef FileReference *frefid_t;
typedef Common::Array< Common::SharedPtr<FileReference> > FileRefArray;

class Files {
private:
	FileRefArray _fileReferences;
public:
	/**
	 * Prompt for a savegame to load or save, and populate a file reference from the result
	 */
	frefid_t createByPrompt(glui32 usage, FileMode fmode, glui32 rock);

	/**
	 * Create a new file reference
	 */
	frefid_t createRef(int slot, const Common::String &desc, glui32 usage, glui32 rock);

	/**
	 * Create a new file reference
	 */
	frefid_t createRef(const Common::String &filename, glui32 usage, glui32 rock);

	/**
	 * Create a new temporary file reference
	 */
	frefid_t createTemp(glui32 usage, glui32 rock);

	/**
	 * Create a new file reference from an old one
	 */
	frefid_t createFromRef(frefid_t fref, glui32 usage, glui32 rock);

	/**
	 * Delete a file reference
	 */
	void deleteRef(frefid_t fref);

	/**
	 * Iterates to the next file reference following the specified one,
	 * or the first if null is passed
	 */
	frefid_t iterate(frefid_t fref, glui32 *rock);
};

} // End of namespace Gargoyle

#endif
