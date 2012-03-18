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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_SAVELOAD_H
#define MORTEVIELLE_SAVELOAD_H

#include "common/savefile.h"
#include "common/serializer.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"

#define SAVEGAME_VERSION 1

namespace Mortevielle {

struct SavegameHeader {
	uint8 version;
	Common::String saveName;
	Graphics::Surface *thumbnail;
	int saveYear, saveMonth, saveDay;
	int saveHour, saveMinutes;
	int totalFrames;
};

class SavegameManager {
private:
	void sync_save(Common::Serializer &sz);
public:
	void loadSavegame(int n);
	Common::Error loadGame(int n);
	Common::Error saveGame(int n, const Common::String &saveName);

	static void writeSavegameHeader(Common::OutSaveFile *out, const Common::String &saveName);
	static bool readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header);
	static Common::String generateSaveName(int slotNumber);
	static SaveStateList listSaves(const char *target);
	static SaveStateDescriptor querySaveMetaInfos(int slot);
};

} // End of namespace Mortevielle
#endif
