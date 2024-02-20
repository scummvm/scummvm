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

#ifndef TWP_SAVEGAME_H
#define TWP_SAVEGAME_H

#include "common/stream.h"
#include "common/formats/json.h"

namespace Twp {

struct SaveGame {
	int32 hashData = 0;
	int64 time = 0;
	int64 gameTime = 0;
	bool easyMode = false;
	Common::ScopedPtr<Common::JSONValue> jSavegame;
};

class SaveGameManager {
public:
	static bool getSaveGame(Common::SeekableReadStream *stream, SaveGame &savegame);
	bool loadGame(const SaveGame &savegame);
	void saveGame(Common::WriteStream *stream);

private:
	void loadGameScene(const Common::JSONObject &json);
	void loadDialog(const Common::JSONObject &json);
	void loadCallbacks(const Common::JSONObject &json);
	void loadGlobals(const Common::JSONObject &json);
	void loadActors(const Common::JSONObject &json);
	void loadInventory(const Common::JSONValue *json);
	void loadRooms(const Common::JSONObject &json);
	void loadObjects(const Common::JSONObject &json);

public:
	bool _allowSaveGame = true;
	bool _autoSave = false;
};

} // namespace Twp

#endif
