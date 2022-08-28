/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_SAVEGAME_H
#define WINTERMUTE_SAVEGAME_H

#include "common/str.h"

namespace Wintermute {
class BaseGame;
class SaveLoad {
public:
	static bool emptySaveSlot(int slot);
	static bool isSaveSlotUsed(int slot);
	static Common::String getSaveSlotDescription(int slot);
	static void getSaveSlotTimestamp(int slot, TimeDate *time);
	static Common::String getSaveSlotFilename(int slot);

	static bool loadGame(const Common::String &filename, BaseGame *gameRef);
	static bool saveGame(int slot, const char *desc, bool quickSave, BaseGame *gameRef);
	static bool initAfterLoad();
	static void afterLoadScene(void *scene, void *data);
	static void afterLoadRegion(void *region, void *data);
private:
	static void afterLoadSubFrame(void *subframe, void *data);
	static void afterLoadSound(void *sound, void *data);
	static void afterLoadFont(void *font, void *data);
#ifdef ENABLE_WME3D
	static void afterLoadXModel(void *model, void *data);
#endif
	static void afterLoadScript(void *script, void *data);
};

} // End of namespace Wintermute

#endif
