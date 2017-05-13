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

#ifndef SCI_ENGINE_FILE_H
#define SCI_ENGINE_FILE_H

#include "common/str-array.h"
#include "common/stream.h"

namespace Sci {

enum kFileOpenMode {
	_K_FILE_MODE_OPEN_OR_CREATE = 0,
	_K_FILE_MODE_OPEN_OR_FAIL = 1,
	_K_FILE_MODE_CREATE = 2
};

enum {
	SCI_MAX_SAVENAME_LENGTH = 36, ///< Maximum length of a savegame name (including terminator character).
	MAX_SAVEGAME_NR = 20 ///< Maximum number of savegames
};

#ifdef ENABLE_SCI32
enum {
	kAutoSaveId = 0,  ///< The save game slot number for autosaves
	kNewGameId = 999, ///< The save game slot number for a "new game" save

	// SCI engine expects game IDs to start at 0, but slot 0 in ScummVM is
	// reserved for autosave, so non-autosave games get their IDs shifted up
	// when saving or restoring, and shifted down when enumerating save games
	kSaveIdShift = 1
};
#endif

#define VIRTUALFILE_HANDLE_START 32000
#define VIRTUALFILE_HANDLE_SCI32SAVE 32100
#define VIRTUALFILE_HANDLE_SCIAUDIO 32300
#define VIRTUALFILE_HANDLE_END 32300

struct SavegameDesc {
	int16 id;
	int virtualId; // straight numbered, according to id but w/o gaps
	int date;
	int time;
	int version;
	char name[SCI_MAX_SAVENAME_LENGTH];
	Common::String gameVersion;
	uint32 script0Size;
	uint32 gameObjectOffset;
#ifdef ENABLE_SCI32
	// Used by Shivers 1
	uint16 lowScore;
	uint16 highScore;
	// Used by MGDX
	uint8 avatarId;
#endif
};

class FileHandle {
public:
	Common::String _name;
	Common::SeekableReadStream *_in;
	Common::WriteStream *_out;

public:
	FileHandle();
	~FileHandle();

	void close();
	bool isOpen() const;
};


class DirSeeker {
protected:
	reg_t _outbuffer;
	Common::StringArray _files;
	Common::StringArray _virtualFiles;
	Common::StringArray::const_iterator _iter;

public:
	DirSeeker() {
		_outbuffer = NULL_REG;
		_iter = _files.begin();
	}

	reg_t firstFile(const Common::String &mask, reg_t buffer, SegManager *segMan);
	reg_t nextFile(SegManager *segMan);

	Common::String getVirtualFilename(uint fileNumber);

private:
	void addAsVirtualFiles(Common::String title, Common::String fileMask);
};

uint findFreeFileHandle(EngineState *s);

} // End of namespace Sci

#endif // SCI_ENGINE_FILE_H
