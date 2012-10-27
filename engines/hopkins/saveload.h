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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_SAVELOAD_H
#define HOPKINS_SAVELOAD_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/str.h"

namespace Hopkins {

class HopkinsEngine;

#define HOPKINS_SAVEGAME_VERSION 1

struct hopkinsSavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
	int totalFrames;
};

class SaveLoadManager {
private:
	HopkinsEngine *_vm;
public:
	void setParent(HopkinsEngine *vm);

	void initSaves();
	bool bsave(const Common::String &file, const void *buf, size_t n);
	bool SAUVE_FICHIER(const Common::String &file, const void *buf, size_t n);
	void bload(const Common::String &file, byte *buf);

	static bool readSavegameHeader(Common::InSaveFile *in, hopkinsSavegameHeader &header);
	void writeSavegameHeader(Common::OutSaveFile *out, hopkinsSavegameHeader &header);
	Common::Error save(int slot, const Common::String &saveName);
	Common::Error restore(int slot);
};

} // End of namespace Hopkins

#endif /* HOPKINS_SAVELOAD_H */
