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

#ifndef HARVESTER_SCRIPT_H
#define HARVESTER_SCRIPT_H

#include "common/array.h"
#include "common/str.h"
#include "harvester/monster.h"

namespace Common {
class Serializer;
}

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

struct StartupMapEntranceRecord {
	int mapX = 0;
	int mapY = 0;
	int initialPanelIndex = 0;
	Common::String entryName;
};

struct StartupMapLocationRecord {
	int minX = 0;
	int minY = 0;
	int maxX = 0;
	int maxY = 0;
	int panelIndex = 0;
	int labelX = 0;
	int labelY = 0;
	Common::String labelText;
	Common::String destinationEntranceName;
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
	Common::String reservedString38;
	Common::String reservedString3c;
	Common::String reservedString40;
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
	Common::String reservedXFlag;
	Common::String identTextKey;
	Common::String currentOwnerOrRoom;
	Common::String reservedString40;
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

struct StartupTimerRecord {
	int initialValue = 0;
	int currentValue = 0;
	Common::String timerName;
	Common::String arg1;
	Common::String arg2;
	bool enabled = false;
	bool looping = false;
	bool global = false;
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

struct StartupExecListRecord {
	Common::String listName;
	Common::Array<Common::String> entries;
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
	Common::Array<StartupTimerRecord> roomTimers;
	Common::Array<StartupRegionRecord> roomRegions;
	Common::Array<StartupAudioCommand> audioCommands;
};

enum StartupRoomTransitionKind {
	kStartupRoomTransitionNone = 0,
	kStartupRoomTransitionChangeRoom,
	kStartupRoomTransitionCloseup
};

enum StartupLightingCommand {
	kStartupLightingCommandNone = 0,
	kStartupLightingCommandDim,
	kStartupLightingCommandNormal,
	kStartupLightingCommandBlack,
	kStartupLightingCommandFadeIn
};

struct StartupResolvedText {
	Common::String boxName;
	Common::String value;
};

struct StartupInteractionResult {
	Common::String musicPath;
	Common::String nextRoomName;
	Common::String cutscenePath;
	Common::String deathFlicPath;
	Common::String dialogueNpcName;
	Common::String dialogueContinuationTag;
	Common::String continuationTag;
	StartupResolvedText modalText;
	Common::Array<StartupAudioCommand> audioCommands;
	StartupRoomTransitionKind roomTransition = kStartupRoomTransitionNone;
	StartupLightingCommand lightingCommand = kStartupLightingCommandNone;
	int playerGotoX = 0;
	int playerGotoZ = 0;
	bool requestPlayerGotoXZ = false;
	bool requestMainMenu = false;
	bool abortRemainingCommandChain = false;
	bool mutatedRuntimeState = false;
};

class Script {
public:
	bool load(ResourceManager &resources);

	const Common::String &getPath() const { return _path; }
	const Common::Array<byte> &getData() const { return _data; }
	const Common::Array<StartupEntranceRecord> &getEntrances() const { return _entrances; }
	const Common::Array<StartupMapEntranceRecord> &getMapEntrances() const { return _mapEntrances; }
	const Common::Array<StartupMapLocationRecord> &getMapLocations() const { return _mapLocations; }
	const Common::Array<StartupRoomRecord> &getRooms() const { return _rooms; }
	const Common::Array<StartupObjectRecord> &getObjects() const { return _objects; }
	const Common::Array<StartupAnimRecord> &getAnimations() const { return _animations; }
	const Common::Array<StartupNpcRecord> &getNpcs() const { return _npcs; }
	const Common::Array<StartupMonsterRecord> &getMonsters() const { return _monsters; }
	const Common::Array<StartupTimerRecord> &getTimers() const { return _timers; }
	const Common::Array<StartupRegionRecord> &getRegions() const { return _regions; }
	const Common::Array<StartupFlagRecord> &getFlags() const { return _flags; }
	const Common::Array<StartupCommandRecord> &getCommands() const { return _commands; }
	const Common::Array<StartupExecListRecord> &getExecLists() const { return _execLists; }
	const Common::Array<StartupTextRecord> &getTexts() const { return _texts; }
	const Common::Array<StartupHeadRecord> &getHeads() const { return _heads; }
	const Common::Array<StartupUseItemRecord> &getUseItems() const { return _useItems; }
	bool isQuickTipsEnabled() const { return _quickTipsEnabled; }
	void setQuickTipsEnabled(bool enabled) { _quickTipsEnabled = enabled; }
	const Common::String &getVoicePath() const { return _voicePath; }
	bool isGoreEnabled() const { return _goreEnabled; }
	void setGoreEnabled(bool enabled) { _goreEnabled = enabled; }
	StartupDialogueTextMode getDialogueTextMode() const { return _dialogueTextMode; }
	void setDialogueTextMode(StartupDialogueTextMode mode) { _dialogueTextMode = mode; }
	int getFxVolumeLevel() const { return _fxVolumeLevel; }
	void setFxVolumeLevel(int level);
	int getMusicVolumeLevel() const { return _musicVolumeLevel; }
	void setMusicVolumeLevel(int level);
	int getGammaLevel() const { return _gammaLevel; }
	void setGammaLevel(int level);
	const Common::String &getParentalPassword() const { return _parentalPassword; }
	void setParentalPassword(const Common::String &password) { _parentalPassword = password; }
	bool saveConfig() const;
	void resetRuntimeState();
	void syncRuntimeSaveState(Common::Serializer &s);
	bool resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources);
	bool materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		StartupRoomSetupState &state, ResourceManager &resources) const;
	bool executeRoomExitCommands(const Common::String &roomName, StartupInteractionResult &result);
	bool resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result);
	bool resolveRegionInteraction(const StartupRegionRecord &region, StartupInteractionResult &result);
	bool resolveUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target,
		StartupInteractionResult &result);
	bool executeActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions = true);
	bool executeTimerAction(const Common::String &timerName, StartupInteractionResult &result,
		bool allowTransitions = true);
	bool executeNestedActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions = true);
	bool isPickupObject(const StartupObjectRecord &object) const;
	bool hasObjectInteraction(const StartupObjectRecord &object) const;
	bool hasUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target) const;
	void getVisibleInventoryObjects(Common::Array<StartupObjectRecord> &objects) const;
	bool isObjectInInventory(const Common::String &objectName) const;
	void markObjectIdentShown(const StartupObjectRecord &object);
	bool resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const;
	bool resolveTextRecord(const Common::String &key, StartupResolvedText &text) const;
	Common::String resolveInventoryTooltipText(const StartupObjectRecord &object) const;
	Common::String resolveObjectLabel(const StartupObjectRecord &object) const;
	Common::String resolveTextValue(const Common::String &key) const;
	const StartupHeadRecord *findHeadRecord(const Common::String &headId) const;
	const StartupEntranceRecord *findEntranceRecord(const Common::String &entranceName) const;
	const StartupMapEntranceRecord *findMapEntranceRecord(const Common::String &entryName) const;
	bool getFlagValue(const Common::String &flagName) const;
	const StartupNpcRecord *findRuntimeNpcRecord(const Common::String &npcName) const;
	bool setRuntimeFlagValue(const Common::String &flagName, bool value);
	bool resetRuntimeObjectToInitialState(const Common::String &objectName);
	bool addRuntimeObjectToInventory(const Common::String &objectName);
	bool syncRuntimeObjectRecord(const StartupObjectRecord &object);
	bool setRuntimeObjectVisible(const Common::String &ownerOrRoom, const Common::String &objectName,
		bool visible);
	bool setRuntimeNpcState(const Common::String &npcName, bool active, bool visible);
	bool triggerRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType = 0);
	bool isNamedNpcDeathTypeClear(const Common::String &npcName) const;
	int getCurrentStoryDayIndex() const;
	int getPlayerCurrentHitPoints() const { return _playerCurrentHitPoints; }
	int getPlayerCombatLoadout() const { return _playerCombatLoadout; }
	bool isPlayerControlPaused() const { return _playerControlPaused; }
	bool adjustPlayerCurrentHitPoints(int delta);
	bool setPlayerCombatLoadout(int loadout);
	bool setPlayerControlPaused(bool paused);
	bool syncRuntimeAnimState(const Common::String &animName, bool active, bool visible, int currentFrame);
	bool syncRuntimeMonsterRecord(const StartupMonsterRecord &monster);
	bool syncRuntimeTimerRecord(const StartupTimerRecord &timer);
	bool setRuntimeTimerEnabled(const Common::String &timerName, bool enabled);

private:
	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);
	const StartupRoomRecord *findRoomRecord(const Common::String &roomName) const;
	const StartupCommandRecord *findCommandRecord(const Common::String &tag) const;
	const StartupExecListRecord *findExecListRecord(const Common::String &name) const;
	const StartupTextRecord *findTextRecord(const Common::String &key) const;
	const StartupUseItemRecord *findUseItemRecord(const Common::String &itemName,
		const StartupObjectRecord &target) const;
	const StartupFlagRecord *findRuntimeFlag(const Common::String &flagName) const;
	StartupFlagRecord *findRuntimeFlag(const Common::String &flagName);
	StartupObjectRecord *findRuntimeObject(const Common::String &ownerOrRoom, const Common::String &objectName);
	StartupAnimRecord *findRuntimeAnim(const Common::String &animName);
	StartupRegionRecord *findRuntimeRegion(const Common::String &regionName);
	StartupNpcRecord *findRuntimeNpc(const Common::String &npcName);
	const StartupNpcRecord *findRuntimeNpc(const Common::String &npcName) const;
	StartupMonsterRecord *findRuntimeMonster(const Common::String &monsterName);
	const StartupMonsterRecord *findRuntimeMonster(const Common::String &monsterName) const;
	StartupTimerRecord *findRuntimeTimer(const Common::String &timerName);
	const StartupTimerRecord *findRuntimeTimer(const Common::String &timerName) const;
	bool buildRuntimeRoomState(const StartupRoomRecord &room, const StartupEntranceRecord *entrance,
		ResourceManager &resources, StartupRoomSetupState &state) const;
	void executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, bool allowTransitions, Common::String *musicPath,
		Common::Array<StartupAudioCommand> *audioCommands, Common::String *nextRoomName,
		StartupRoomTransitionKind *roomTransition,
		Common::String *cutscenePath, Common::String *deathFlicPath, bool *requestMainMenu,
		Common::String *dialogueNpcName, Common::String *dialogueContinuationTag,
		Common::String *continuationTag, StartupResolvedText *modalText,
		StartupLightingCommand *lightingCommand, bool *requestPlayerGotoXZ,
		int *playerGotoX, int *playerGotoZ,
		bool *mutatedRuntimeState);
	bool hasActionableCommandChain(const Common::String &initialTag) const;
	bool setPlayerCurrentHitPoints(int hitPoints);
	void logRuntimeSaveState(const char *operation) const;

	Common::String _path;
	Common::Array<byte> _data;
	Common::Array<StartupEntranceRecord> _entrances;
	Common::Array<StartupMapEntranceRecord> _mapEntrances;
	Common::Array<StartupMapLocationRecord> _mapLocations;
	Common::Array<StartupRoomRecord> _rooms;
	Common::Array<StartupObjectRecord> _objects;
	Common::Array<StartupAnimRecord> _animations;
	Common::Array<StartupNpcRecord> _npcs;
	Common::Array<StartupMonsterRecord> _monsters;
	Common::Array<StartupTimerRecord> _timers;
	Common::Array<StartupRegionRecord> _regions;
	Common::Array<StartupFlagRecord> _flags;
	Common::Array<StartupCommandRecord> _commands;
	Common::Array<StartupExecListRecord> _execLists;
	Common::Array<StartupTextRecord> _texts;
	Common::Array<StartupHeadRecord> _heads;
	Common::Array<StartupUseItemRecord> _useItems;
	Common::Array<StartupFlagRecord> _runtimeFlags;
	Common::Array<StartupObjectRecord> _runtimeObjects;
	Common::Array<StartupAnimRecord> _runtimeAnimations;
	Common::Array<StartupRegionRecord> _runtimeRegions;
	Common::Array<StartupNpcRecord> _runtimeNpcs;
	Common::Array<StartupMonsterRecord> _runtimeMonsters;
	Common::Array<StartupTimerRecord> _runtimeTimers;
	int _playerCurrentHitPoints = 30;
	int _playerCombatLoadout = 0;
	bool _playerControlPaused = false;
	int _fxVolumeLevel = 9;
	int _musicVolumeLevel = 3;
	int _gammaLevel = 0;
	bool _quickTipsEnabled = true;
	bool _goreEnabled = true;
	Common::String _voicePath = "SOUND/VOICE/";
	Common::String _parentalPassword;
	Common::String _savePath = "./";
	Common::String _townPath = "HARVEST.SCR";
	Common::String _cdRomPath;
	Common::String _drive1Path;
	Common::String _drive2Path;
	Common::String _drive3Path;
	StartupDialogueTextMode _dialogueTextMode = kStartupDialogueTextYes;
};

} // End of namespace Harvester

#endif // HARVESTER_SCRIPT_H
