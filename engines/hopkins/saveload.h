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

#ifndef HOPKINS_SAVELOAD_H
#define HOPKINS_SAVELOAD_H

#include "hopkins/globals.h"
#include "hopkins/graphics.h"

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

#define HOPKINS_SAVEGAME_VERSION 4

struct hopkinsSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class SaveLoadManager {
private:
	HopkinsEngine *_vm;

	void createThumbnail(Graphics::Surface *s);
	void syncSavegameData(Common::Serializer &s, int version);
	void syncCharacterLocation(Common::Serializer &s, CharacterLocation &item);
public:
	SaveLoadManager(HopkinsEngine *vm);

	bool saveExists(const Common::String &file);
	bool save(const Common::String &file, const void *buf, size_t n);
	bool saveFile(const Common::String &file, const void *buf, size_t n);
	void load(const Common::String &file, byte *buf);

	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, hopkinsSavegameHeader &header, bool skipThumbnail = true);
	void writeSavegameHeader(Common::OutSaveFile *out, hopkinsSavegameHeader &header);
	WARN_UNUSED_RESULT bool readSavegameHeader(int slot, hopkinsSavegameHeader &header, bool skipThumbnail = true);
	Common::Error saveGame(int slot, const Common::String &saveName);
	Common::Error loadGame(int slot);

	/**
	 * Converts a 16-bit thumbnail to 8 bit paletted using the currently active palette.
	 */
	void convertThumb16To8(Graphics::Surface *thumb16, Graphics::Surface *thumb8);
};

} // End of namespace Hopkins

#endif /* HOPKINS_SAVELOAD_H */
