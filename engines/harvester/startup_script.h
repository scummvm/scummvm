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

struct StartupNpcRecord {
	int posX = 0;
	int posY = 0;
	int posZ = 0;
	int frameDelay = 0;
	Common::String onDeathActionTag;
	Common::String modelPath;
	Common::String npcName;
	Common::String monsterfyTargetName;
	Common::String roomName;
	bool deathOrMonsterfyFlag = false;
	bool runtimeSpawned = false;
	bool active = false;
	bool visible = false;
	bool savedVisible = false;
	int deathDamageType = 0;
	Common::String audioPath;
	Common::String entityInitArg;
};

struct StartupMonsterRecord {
	int posX = 0;
	int posY = 0;
	int posZ = 0;
	int initialFacing = -1;
	Common::String roomName;
	Common::String monsterName;
	Common::String modelPath;
	Common::String onDeathActionTag;
	bool active = false;
	bool visible = false;
};

struct StartupRegionRecord {
	int left = 0;
	int top = 0;
	int right = 0;
	int bottom = 0;
	int minZ = 0;
	int maxZ = 0;
	int desiredFacing = -1;
	Common::String regionName;
	Common::String direction;
	Common::String roomName;
	Common::String actionTag;
	bool startEnabled = false;
	bool cursorEnabled = false;
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

struct StartupHeadRecord {
	Common::String headId;
	Common::String portraitPath;
};

struct StartupUseItemRecord {
	Common::String itemName;
	Common::String ownerOrRoom;
	Common::String targetName;
	Common::String actionTag;
};

enum StartupDialogueTextMode {
	kStartupDialogueTextNone = 0,
	kStartupDialogueTextYes = 1,
	kStartupDialogueTextClick = 2
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
	Common::Array<StartupNpcRecord> roomNpcs;
	Common::Array<StartupMonsterRecord> roomMonsters;
	Common::Array<StartupRegionRecord> roomRegions;
	Common::Array<StartupAudioCommand> audioCommands;
};

enum StartupRoomTransitionKind {
	kStartupRoomTransitionNone = 0,
	kStartupRoomTransitionChangeRoom,
	kStartupRoomTransitionCloseup
};

struct StartupInteractionResult {
	Common::String musicPath;
	Common::String nextRoomName;
	Common::String deathFlicPath;
	Common::String dialogueNpcName;
	Common::String dialogueContinuationTag;
	Common::Array<StartupAudioCommand> audioCommands;
	StartupRoomTransitionKind roomTransition = kStartupRoomTransitionNone;
	bool requestMainMenu = false;
	bool abortRemainingCommandChain = false;
	bool mutatedRuntimeState = false;
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
	const Common::Array<StartupNpcRecord> &getNpcs() const { return _npcs; }
	const Common::Array<StartupMonsterRecord> &getMonsters() const { return _monsters; }
	const Common::Array<StartupRegionRecord> &getRegions() const { return _regions; }
	const Common::Array<StartupFlagRecord> &getFlags() const { return _flags; }
	const Common::Array<StartupCommandRecord> &getCommands() const { return _commands; }
	const Common::Array<StartupTextRecord> &getTexts() const { return _texts; }
	const Common::Array<StartupHeadRecord> &getHeads() const { return _heads; }
	const Common::Array<StartupUseItemRecord> &getUseItems() const { return _useItems; }
	bool isQuickTipsEnabled() const { return _quickTipsEnabled; }
	void setQuickTipsEnabled(bool enabled) { _quickTipsEnabled = enabled; }
	const Common::String &getVoicePath() const { return _voicePath; }
	StartupDialogueTextMode getDialogueTextMode() const { return _dialogueTextMode; }
	void resetRuntimeState();
	bool resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources);
	bool materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		StartupRoomSetupState &state) const;
		bool executeRoomExitCommands(const Common::String &roomName, Common::Array<StartupAudioCommand> &audioCommands);
		bool resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result);
		bool resolveRegionInteraction(const StartupRegionRecord &region, StartupInteractionResult &result);
		bool resolveUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target,
			StartupInteractionResult &result);
		bool executeActionTag(const Common::String &tag, StartupInteractionResult &result,
			bool allowTransitions = true);
		bool executeNestedActionTag(const Common::String &tag, StartupInteractionResult &result,
			bool allowTransitions = true);
		bool isPickupObject(const StartupObjectRecord &object) const;
		bool hasObjectInteraction(const StartupObjectRecord &object) const;
		bool hasUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target) const;
		void getVisibleInventoryObjects(Common::Array<StartupObjectRecord> &objects) const;
		void markObjectIdentShown(const StartupObjectRecord &object);
		bool resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const;
		Common::String resolveObjectLabel(const StartupObjectRecord &object) const;
		Common::String resolveTextValue(const Common::String &key) const;
		const StartupHeadRecord *findHeadRecord(const Common::String &headId) const;
		bool getFlagValue(const Common::String &flagName) const;
		const StartupNpcRecord *findRuntimeNpcRecord(const Common::String &npcName) const;
		bool setRuntimeFlagValue(const Common::String &flagName, bool value);
		bool resetRuntimeObjectToInitialState(const Common::String &objectName);
		bool addRuntimeObjectToInventory(const Common::String &objectName);
		bool setRuntimeObjectVisible(const Common::String &ownerOrRoom, const Common::String &objectName,
			bool visible);
		bool setRuntimeNpcState(const Common::String &npcName, bool active, bool visible);
		bool triggerRuntimeNpcDeathOrMonsterfy(const Common::String &npcName);
		bool isNamedNpcDeathTypeClear(const Common::String &npcName) const;
		int getCurrentStoryDayIndex() const;

private:
	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);
	const StartupEntranceRecord *findEntranceRecord(const Common::String &entranceName) const;
	const StartupRoomRecord *findRoomRecord(const Common::String &roomName) const;
	const StartupCommandRecord *findCommandRecord(const Common::String &tag) const;
	const StartupTextRecord *findTextRecord(const Common::String &key) const;
	const StartupUseItemRecord *findUseItemRecord(const Common::String &itemName, const StartupObjectRecord &target) const;
	const StartupFlagRecord *findRuntimeFlag(const Common::String &flagName) const;
	StartupFlagRecord *findRuntimeFlag(const Common::String &flagName);
	StartupObjectRecord *findRuntimeObject(const Common::String &ownerOrRoom, const Common::String &objectName);
	StartupAnimRecord *findRuntimeAnim(const Common::String &animName);
	StartupRegionRecord *findRuntimeRegion(const Common::String &regionName);
	StartupNpcRecord *findRuntimeNpc(const Common::String &npcName);
	const StartupNpcRecord *findRuntimeNpc(const Common::String &npcName) const;
	StartupMonsterRecord *findRuntimeMonster(const Common::String &monsterName);
	const StartupMonsterRecord *findRuntimeMonster(const Common::String &monsterName) const;
	bool buildRuntimeRoomState(const StartupRoomRecord &room, const StartupEntranceRecord *entrance,
		StartupRoomSetupState &state) const;
	void executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, bool allowTransitions, Common::String *musicPath,
		Common::Array<StartupAudioCommand> *audioCommands, Common::String *nextRoomName,
		StartupRoomTransitionKind *roomTransition,
		Common::String *deathFlicPath, bool *requestMainMenu, Common::String *dialogueNpcName,
		Common::String *dialogueContinuationTag,
		bool *mutatedRuntimeState);
	bool hasActionableCommandChain(const Common::String &initialTag) const;

	Common::String _path;
	Common::Array<byte> _data;
	Common::Array<StartupEntranceRecord> _entrances;
	Common::Array<StartupRoomRecord> _rooms;
	Common::Array<StartupObjectRecord> _objects;
	Common::Array<StartupAnimRecord> _animations;
	Common::Array<StartupNpcRecord> _npcs;
	Common::Array<StartupMonsterRecord> _monsters;
	Common::Array<StartupRegionRecord> _regions;
	Common::Array<StartupFlagRecord> _flags;
	Common::Array<StartupCommandRecord> _commands;
	Common::Array<StartupTextRecord> _texts;
	Common::Array<StartupHeadRecord> _heads;
	Common::Array<StartupUseItemRecord> _useItems;
	Common::Array<StartupFlagRecord> _runtimeFlags;
	Common::Array<StartupObjectRecord> _runtimeObjects;
	Common::Array<StartupAnimRecord> _runtimeAnimations;
	Common::Array<StartupRegionRecord> _runtimeRegions;
	Common::Array<StartupNpcRecord> _runtimeNpcs;
	Common::Array<StartupMonsterRecord> _runtimeMonsters;
	bool _quickTipsEnabled = true;
	Common::String _voicePath = "SOUND/VOICE/";
	StartupDialogueTextMode _dialogueTextMode = kStartupDialogueTextYes;
};

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_SCRIPT_H
