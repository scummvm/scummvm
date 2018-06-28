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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_SAVELOAD_H
#define MORTEVIELLE_SAVELOAD_H

#include "common/savefile.h"
#include "common/serializer.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "engines/savestate.h"

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

class MortevielleEngine;

class SavegameManager {
private:
	MortevielleEngine *_vm;
	byte _tabdonSaveBuffer[391];

	void sync_save(Common::Serializer &sz);
public:
	SavegameManager(MortevielleEngine *vm);

	bool loadSavegame(const Common::String &filename);
	Common::Error loadGame(const Common::String &filename);
	Common::Error saveGame(int n, const Common::String &saveName);
	Common::Error loadGame(int slot);
	Common::Error saveGame(int slot);

	void writeSavegameHeader(Common::OutSaveFile *out, const Common::String &saveName);
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, SavegameHeader &header, bool skipThumbnail = true);
	static SaveStateList listSaves(const Common::String &target);
	static SaveStateDescriptor querySaveMetaInfos(const Common::String &fileName);
};

} // End of namespace Mortevielle
#endif
