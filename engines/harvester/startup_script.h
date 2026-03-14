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

#ifndef HARVESTER_STARTUP_SCRIPT_H
#define HARVESTER_STARTUP_SCRIPT_H

#include "common/array.h"
#include "common/str.h"

namespace Harvester {

class ResourceManager;

struct StartupEntranceRecord {
	Common::String direction;
	Common::String roomName;
	Common::String entranceName;
};

struct StartupRoomRecord {
	Common::String roomName;
	Common::String palettePath;
	Common::String onEnterCommand;
	Common::String onExitCommand;
	bool dimmable = false;
};

struct StartupObjectRecord {
	Common::String ownerOrRoom;
	Common::String objectName;
	Common::String resourcePath;
	Common::String identTextKey;
	Common::String displayName;
	bool visible = false;
	bool active = false;
};

class StartupScript {
public:
	bool load(ResourceManager &resources);

	const Common::String &getPath() const { return _path; }
	const Common::Array<byte> &getData() const { return _data; }
	const Common::Array<StartupEntranceRecord> &getEntrances() const { return _entrances; }
	const Common::Array<StartupRoomRecord> &getRooms() const { return _rooms; }
	const Common::Array<StartupObjectRecord> &getObjects() const { return _objects; }
	bool isQuickTipsEnabled() const { return _quickTipsEnabled; }
	void setQuickTipsEnabled(bool enabled) { _quickTipsEnabled = enabled; }
	bool resolveRoomSetup(const Common::String &entranceName, Common::String &roomName,
		Common::String &palettePath, Common::String &backgroundPath) const;

private:
	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);

	Common::String _path;
	Common::Array<byte> _data;
	Common::Array<StartupEntranceRecord> _entrances;
	Common::Array<StartupRoomRecord> _rooms;
	Common::Array<StartupObjectRecord> _objects;
	bool _quickTipsEnabled = true;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_SCRIPT_H
