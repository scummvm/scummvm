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


#ifndef SNATCHER_SAVELOAD_H
#define SNATCHER_SAVELOAD_H

#include "common/ptr.h"
#include "common/str.h"
#include "common/system.h"
#include "snatcher/state.h"

namespace Common {
class SeekableReadStream;
class OutSaveFile;
}

namespace Graphics {
struct Surface;
}

namespace Snatcher {

class SnatcherEngine;
class SaveLoadManager {
public:
	SaveLoadManager(SnatcherEngine *vm);
	~SaveLoadManager();

	void loadSettings(Config &config);
	void saveSettings(const Config &config);
	void updateSaveSlotsState(GameState &state);

	void requestLoad(int slot);
	void requestSave(int slot, const Common::String &desc);

	void handleSaveLoad(GameState &state);

	bool isSaveSlotUsed(int16 slot) const;
	bool isSavingEnabled() const;
	void enableSaving(bool enable);

public:
	struct SaveHeader {
		SaveHeader() : version(0), lang(0), platform(0), desc(), thumbnail(), totalPlayTime(0), saveCount(0) {
			memset(&td, 0, sizeof(TimeDate));
		}
		uint32 version;
		uint32 lang;
		uint32 platform;
		Common::String desc;
		TimeDate td;
		Common::SharedPtr<Graphics::Surface> thumbnail;
		uint32 totalPlayTime;
		int16 saveCount;
	};

	static Common::String getSavegameFilename(int slot, Common::String target);
	static bool readSaveHeader(Common::SeekableReadStream *saveFile, SaveHeader &header);

private:
	void loadState(int slot, GameState &state);
	void saveState(int slot, GameState &state);

	Common::SeekableReadStream *openFileForLoading(int slot, GameState &state, SaveHeader &header);
	Common::OutSaveFile *openFileForSaving(int slot, const Common::String &desc, GameState &state);

	SnatcherEngine *_vm;
	Common::String _desc;
	int _pendingSaveLoad;
	bool _tryLoadFromLauncher;
	bool _enableSaving;
};

} // End of namespace Snatcher

#endif // SNATCHER_SAVELOAD_H
