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

#ifndef SHERLOCK_SAVELOAD_H
#define SHERLOCK_SAVELOAD_H

#include "common/scummsys.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str-array.h"
#include "engines/savestate.h"
#include "graphics/surface.h"

namespace Sherlock {

#define MAX_SAVEGAME_SLOTS 99
#define SHERLOCK_SAVEGAME_VERSION 1

enum SaveMode { SAVEMODE_NONE = 0, SAVEMODE_LOAD = 1, SAVEMODE_SAVE = 2 };

extern const int ENV_POINTS[6][3];

struct SherlockSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

class SherlockEngine;

class SaveManager {
private:
	SherlockEngine *_vm;
	Common::String _target;
	Graphics::Surface *_saveThumb;

	void createSavegameList();

	Common::String generateSaveName(int slot);

	void synchronize(Common::Serializer &s);
public:
	Common::StringArray _savegames;
	int _savegameIndex;
	SaveMode _envMode;
	bool _justLoaded;
public:
	SaveManager(SherlockEngine *vm, const Common::String &target);
	~SaveManager();

	void drawInterface();

	void createThumbnail();

	static SaveStateList getSavegameList(const Common::String &target);

	void writeSavegameHeader(Common::OutSaveFile *out, SherlockSavegameHeader &header);

	static bool readSavegameHeader(Common::InSaveFile *in, SherlockSavegameHeader &header);

	int getHighlightedButton() const;

	void highlightButtons(int btnIndex);

	void loadGame(int slot);
	void saveGame(int slot, const Common::String &name);

	bool checkGameOnScreen(int slot);

	bool getFilename(int slot);
};

} // End of namespace Sherlock

#endif
