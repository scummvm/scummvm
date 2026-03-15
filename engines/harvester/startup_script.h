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
	int posX = 0;
	int posY = 0;
	int posZ = 0;
	int facing = -1;
	Common::String direction;
	Common::String roomName;
	Common::String entranceName;
};

struct StartupRoomRecord {
	int minZ = 0;
	int maxZ = 0;
	int maxZScreenY = 0;
	int minZScreenY = 0;
	int fullScaleZ = 0;
	int maxZScalePercent = 100;
	float perspectiveScale = 0.0f;
	float zVelocityStep = 1.0f;
	Common::String roomName;
	Common::String musicPath;
	Common::String field38;
	Common::String field3c;
	Common::String field40;
	Common::String palettePath;
	Common::String onEnterCommand;
	Common::String onExitCommand;
	bool dimmable = false;
};

struct StartupObjectRecord {
	int initialX = 0;
	int initialY = 0;
	int initialZ = 0;
	int currentX = 0;
	int currentY = 0;
	int currentZ = 0;
	int boundsX2 = 0;
	int boundsY2 = 0;
	int zExtent = 0;
	Common::String actionTag;
	Common::String spritePath;
	Common::String altSpritePath;
	Common::String objectName;
	Common::String field34;
	Common::String identTextKey;
	Common::String currentOwnerOrRoom;
	Common::String field40;
	Common::String inventoryTextKey;
	Common::String initialOwnerOrRoom;
	Common::String interactionLabel;
	bool operatable = false;
	bool visible = false;
	bool runtimeVisible = false;
	bool identShown = false;
};

struct StartupAnimRecord {
	int x = 0;
	int y = 0;
	int z = 0;
	int frameDelay = 0;
	Common::String roomName;
	Common::String resourcePath;
	Common::String animName;
	bool active = false;
	bool visible = false;
	bool looping = false;
	bool backward = false;
	bool pingPong = false;
	bool remove = false;
	bool runtimeActive = false;
	bool runtimeVisible = false;
	int runtimeState = -1;
};

struct StartupFlagRecord {
	Common::String name;
	bool value = false;
};

struct StartupCommandRecord {
	Common::String triggerTag;
	Common::String opcodeName;
	Common::String arg1;
	Common::String arg2;
	Common::String arg3;
	Common::String arg4;
};

struct StartupTextRecord {
	Common::String key;
	Common::String boxName;
	Common::String value;
};

enum StartupAudioCommandType {
	kStartupAudioCommandStartWav,
	kStartupAudioCommandStartSingleWav,
	kStartupAudioCommandLoadWav,
	kStartupAudioCommandPlayWav,
	kStartupAudioCommandDeleteWav
};

struct StartupAudioCommand {
	StartupAudioCommandType type = kStartupAudioCommandStartWav;
	Common::String path;
	int slot = -1;
};

struct StartupRoomSetupState {
	Common::String entranceName;
	Common::String roomName;
	Common::String palettePath;
	Common::String backgroundPath;
	Common::String musicPath;
	int roomMinZ = 0;
	int roomMaxZ = 0;
	int roomMaxZScreenY = 0;
	int roomMinZScreenY = 0;
	int roomFullScaleZ = 0;
	int roomMaxZScalePercent = 100;
	float roomPerspectiveScale = 0.0f;
	float roomZVelocityStep = 1.0f;
	bool hasEntrance = false;
	int playerSpawnX = 0;
	int playerSpawnY = 0;
	int playerSpawnZ = 0;
	int playerFacing = -1;
	float paletteBrightness = 1.0f;
	Common::Array<StartupObjectRecord> activeObjects;
	Common::Array<StartupObjectRecord> roomObjects;
	Common::Array<StartupAnimRecord> roomAnimations;
	Common::Array<StartupAudioCommand> audioCommands;
	Common::Array<StartupAudioCommand> exitAudioCommands;
};

struct StartupInteractionResult {
	Common::String musicPath;
	Common::String nextRoomName;
	Common::Array<StartupAudioCommand> audioCommands;
};

struct StartupResolvedText {
	Common::String boxName;
	Common::String value;
};

class StartupScript {
public:
	bool load(ResourceManager &resources);

	const Common::String &getPath() const { return _path; }
	const Common::Array<byte> &getData() const { return _data; }
	const Common::Array<StartupEntranceRecord> &getEntrances() const { return _entrances; }
	const Common::Array<StartupRoomRecord> &getRooms() const { return _rooms; }
	const Common::Array<StartupObjectRecord> &getObjects() const { return _objects; }
	const Common::Array<StartupAnimRecord> &getAnimations() const { return _animations; }
	const Common::Array<StartupFlagRecord> &getFlags() const { return _flags; }
	const Common::Array<StartupCommandRecord> &getCommands() const { return _commands; }
	const Common::Array<StartupTextRecord> &getTexts() const { return _texts; }
	bool isQuickTipsEnabled() const { return _quickTipsEnabled; }
	void setQuickTipsEnabled(bool enabled) { _quickTipsEnabled = enabled; }
	bool resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources) const;
	bool resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result) const;
	bool resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const;
	Common::String resolveObjectLabel(const StartupObjectRecord &object) const;
	Common::String resolveTextValue(const Common::String &key) const;

private:
	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);
	const StartupTextRecord *findTextRecord(const Common::String &key) const;

	Common::String _path;
	Common::Array<byte> _data;
	Common::Array<StartupEntranceRecord> _entrances;
	Common::Array<StartupRoomRecord> _rooms;
	Common::Array<StartupObjectRecord> _objects;
	Common::Array<StartupAnimRecord> _animations;
	Common::Array<StartupFlagRecord> _flags;
	Common::Array<StartupCommandRecord> _commands;
	Common::Array<StartupTextRecord> _texts;
	bool _quickTipsEnabled = true;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_SCRIPT_H
