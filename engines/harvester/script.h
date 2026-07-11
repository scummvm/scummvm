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

struct EntranceRecord {
	int posX = 0;
	int posY = 0;
	int posZ = 0;
	int facing = -1;
	Common::String direction;
	Common::String roomName;
	Common::String entranceName;
};

struct MapEntranceRecord {
	int mapX = 0;
	int mapY = 0;
	int initialPanelIndex = 0;
	Common::String entryName;
};

struct MapLocationRecord {
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

struct RoomRecord {
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

struct ObjectRecord {
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

struct AnimRecord {
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

struct NpcRecord {
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
	int runtimeState = -1;
	bool active = false;
	bool visible = false;
	bool savedVisible = false;
	int deathDamageType = 0;
	Common::String audioPath;
	Common::String entityInitArg;
};

struct TimerRecord {
	int initialValue = 0;
	int currentValue = 0;
	Common::String timerName;
	Common::String arg1;
	Common::String arg2;
	bool enabled = false;
	bool looping = false;
	bool global = false;
};

struct RegionRecord {
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

struct FlagRecord {
	Common::String name;
	bool value = false;
};

struct CommandRecord {
	Common::String triggerTag;
	Common::String opcodeName;
	Common::String arg1;
	Common::String arg2;
	Common::String arg3;
	Common::String arg4;
};

struct ExecListRecord {
	Common::String listName;
	Common::Array<Common::String> entries;
};

struct TextRecord {
	Common::String key;
	Common::String boxName;
	Common::String value;
};

struct HeadRecord {
	Common::String headId;
	Common::String portraitPath;
};

struct UseItemRecord {
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

struct AudioCommand {
	StartupAudioCommandType type = kStartupAudioCommandStartWav;
	Common::String path;
	int slot = -1;
};

struct RoomSetupState {
	Common::String entranceName;
	Common::String roomName;
	Common::String backgroundObjectName;
	Common::String palettePath;
	Common::String backgroundPath;
	Common::String musicPath;
	int discNumber = 0;
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
	Common::Array<ObjectRecord> activeObjects;
	Common::Array<ObjectRecord> roomObjects;
	Common::Array<AnimRecord> roomAnimations;
	Common::Array<NpcRecord> roomNpcs;
	Common::Array<MonsterRecord> roomMonsters;
	Common::Array<TimerRecord> roomTimers;
	Common::Array<RegionRecord> roomRegions;
	Common::Array<AudioCommand> audioCommands;
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

struct ResolvedText {
	Common::String boxName;
	Common::String value;
};

struct InteractionResult {
	Common::String musicPath;
	Common::String nextRoomName;
	Common::String cutscenePath;
	Common::String deathFlicPath;
	Common::String dialogueNpcName;
	Common::String dialogueContinuationTag;
	Common::String continuationTag;
	ResolvedText modalText;
	Common::Array<AudioCommand> audioCommands;
	Common::Array<TimerRecord> previousTimerRecords;
	StartupRoomTransitionKind roomTransition = kStartupRoomTransitionNone;
	StartupLightingCommand lightingCommand = kStartupLightingCommandNone;
	int cdChangeDisc = 0;
	int playerDeathDamageType = 0;
	int playerGotoX = 0;
	int playerGotoZ = 0;
	bool requestPlayerDeath = false;
	bool requestPlayerGotoXZ = false;
	bool requestRoomRestart = false;
	Common::String roomRestartTargetName;
	bool requestCloseupExit = false;
	bool requestMainMenu = false;
	bool requestDemoEnding = false;
	bool abortRemainingCommandChain = false;
	bool mutatedRuntimeState = false;
	bool visualRuntimeStateChanged = false;
};

class Script {
public:
	static const int kDefaultPlayerHitPoints = 30;

	bool load(ResourceManager &resources);
	bool reloadTownWorld(ResourceManager &resources);

	const Common::String &getPath() const { return _path; }
	const Common::Array<byte> &getData() const { return _data; }
	const Common::Array<EntranceRecord> &getEntrances() const { return _entrances; }
	const Common::Array<MapEntranceRecord> &getMapEntrances() const { return _mapEntrances; }
	const Common::Array<MapLocationRecord> &getMapLocations() const { return _mapLocations; }
	const Common::Array<RoomRecord> &getRooms() const { return _rooms; }
	const Common::Array<ObjectRecord> &getObjects() const { return _objects; }
	const Common::Array<AnimRecord> &getAnimations() const { return _animations; }
	const Common::Array<NpcRecord> &getNpcs() const { return _npcs; }
	const Common::Array<MonsterRecord> &getMonsters() const { return _monsters; }
	const Common::Array<TimerRecord> &getTimers() const { return _timers; }
	const Common::Array<RegionRecord> &getRegions() const { return _regions; }
	const Common::Array<FlagRecord> &getFlags() const { return _flags; }
	const Common::Array<CommandRecord> &getCommands() const { return _commands; }
	const Common::Array<ExecListRecord> &getExecLists() const { return _execLists; }
	const Common::Array<TextRecord> &getTexts() const { return _texts; }
	const Common::Array<HeadRecord> &getHeads() const { return _heads; }
	const Common::Array<UseItemRecord> &getUseItems() const { return _useItems; }
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
	bool resolveRoomSetupState(const Common::String &entranceName, RoomSetupState &state,
		ResourceManager &resources);
	bool resolveRoomSetupStateByRoomName(const Common::String &roomName, RoomSetupState &state,
		ResourceManager &resources);
	bool materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		RoomSetupState &state, ResourceManager &resources) const;
	bool hasRoomSetupTarget(const Common::String &entranceName, const Common::String &roomName) const;
	bool executeRoomEnterCommands(const Common::String &roomName, InteractionResult &result);
	bool executeRoomExitCommands(const Common::String &roomName, InteractionResult &result);
	bool resolveObjectInteraction(const ObjectRecord &object, InteractionResult &result,
		const Common::String &roomName = Common::String());
	bool resolveRegionInteraction(const RegionRecord &region, InteractionResult &result,
		const Common::String &roomName = Common::String());
	bool resolveUseItemInteraction(const Common::String &itemName, const ObjectRecord &target,
		InteractionResult &result, const Common::String &roomName = Common::String());
	bool executeDebugCommand(const CommandRecord &command, InteractionResult &result,
		bool allowTransitions = true);
	bool executeActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions = true, const Common::String &roomName = Common::String());
	bool executeTimerAction(const Common::String &timerName, InteractionResult &result,
		bool allowTransitions = true, const Common::String &roomName = Common::String());
	bool executeNestedActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions = true);
	bool isPickupObject(const ObjectRecord &object) const;
	bool isPickupBlockedByAction(const ObjectRecord &object,
		InteractionResult *result = nullptr) const;
	bool hasObjectInteraction(const ObjectRecord &object) const;
	bool hasUseItemInteraction(const Common::String &itemName, const ObjectRecord &target) const;
	void getVisibleInventoryObjects(Common::Array<ObjectRecord> &objects) const;
	bool isObjectInInventory(const Common::String &objectName) const;
	void markObjectIdentShown(const ObjectRecord &object);
	bool resolveObjectInspectText(const ObjectRecord &object, ResolvedText &text) const;
	bool resolveTextRecord(const Common::String &key, ResolvedText &text) const;
	Common::String resolveInventoryTooltipText(const ObjectRecord &object) const;
	Common::String resolveObjectLabel(const ObjectRecord &object) const;
	Common::String resolveTextValue(const Common::String &key) const;
	const HeadRecord *findHeadRecord(const Common::String &headId) const;
	const EntranceRecord *findEntranceRecord(const Common::String &entranceName) const;
	const MapEntranceRecord *findMapEntranceRecord(const Common::String &entryName) const;
	bool getFlagValue(const Common::String &flagName) const;
	const TimerRecord *findRuntimeTimerRecord(const Common::String &timerName) const;
	const NpcRecord *findRuntimeNpcRecord(const Common::String &npcName) const;
	bool setRuntimeFlagValue(const Common::String &flagName, bool value);
	bool resetRuntimeObjectToInitialState(const Common::String &objectName);
	bool addRuntimeObjectToInventory(const Common::String &objectName);
	bool syncRuntimeObjectRecord(const ObjectRecord &object);
	bool setRuntimeObjectVisible(const Common::String &ownerOrRoom, const Common::String &objectName,
		bool visible);
	bool setRuntimeNpcState(const Common::String &npcName, bool active, bool visible);
	bool queueRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType = 0);
	enum { kNoMonsterfyPosZOverride = 0x7fffffff };
	bool finalizeRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType = 0,
		bool preserveCorpse = false, int corpseFrame = -1,
		int monsterfyPosZ = kNoMonsterfyPosZOverride);
	void pushDeferredLiveNpcDeathTransitions();
	void popDeferredLiveNpcDeathTransitions();
	bool isNamedNpcDeathTypeClear(const Common::String &npcName) const;
	int getCurrentStoryDayIndex() const;
	int getPlayerCurrentHitPoints() const { return _playerCurrentHitPoints; }
	int getPlayerCombatLoadout() const { return _playerCombatLoadout; }
	int getPlayerCombatResourceCount(int loadout) const;
	bool consumePlayerCombatResourceUnit(int loadout);
	bool isPlayerControlPaused() const { return _playerControlPaused; }
	bool adjustPlayerCurrentHitPoints(int delta);
	bool setPlayerCombatLoadout(int loadout);
	bool setPlayerControlPaused(bool paused);
	bool syncRuntimeAnimState(const Common::String &animName, bool active, bool visible, int currentFrame);
	bool syncRuntimeMonsterRecord(const MonsterRecord &monster);
	bool syncRuntimeTimerRecord(const TimerRecord &timer);
	bool setRuntimeRegionEnabled(const Common::String &regionName, bool enabled);
	bool setRuntimeTimerEnabled(const Common::String &timerName, bool enabled);

private:
	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);
	const RoomRecord *findRoomRecord(const Common::String &roomName) const;
	const CommandRecord *findCommandRecord(const Common::String &tag) const;
	const ExecListRecord *findExecListRecord(const Common::String &name) const;
	const TextRecord *findTextRecord(const Common::String &key) const;
	const UseItemRecord *findUseItemRecord(const Common::String &itemName,
		const ObjectRecord &target) const;
	const FlagRecord *findRuntimeFlag(const Common::String &flagName) const;
	FlagRecord *findRuntimeFlag(const Common::String &flagName);
	ObjectRecord *findRuntimeObject(const Common::String &ownerOrRoom, const Common::String &objectName);
	AnimRecord *findRuntimeAnim(const Common::String &animName);
	RegionRecord *findRuntimeRegion(const Common::String &regionName);
	NpcRecord *findRuntimeNpc(const Common::String &npcName);
	const NpcRecord *findRuntimeNpc(const Common::String &npcName) const;
	MonsterRecord *findRuntimeMonster(const Common::String &monsterName);
	const MonsterRecord *findRuntimeMonster(const Common::String &monsterName) const;
	MonsterRecord *findRuntimeMonster(const MonsterRecord &monster);
	const MonsterRecord *findRuntimeMonster(const MonsterRecord &monster) const;
	const MonsterRecord *findBaseMonster(const MonsterRecord &monster) const;
	TimerRecord *findRuntimeTimer(const Common::String &timerName);
	const TimerRecord *findRuntimeTimer(const Common::String &timerName) const;
	bool buildRuntimeRoomState(const RoomRecord &room, const EntranceRecord *entrance,
		ResourceManager &resources, RoomSetupState &state) const;
	void executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, const Common::String &contextRoomName,
		bool allowTransitions, Common::String *musicPath,
		Common::Array<AudioCommand> *audioCommands, Common::String *nextRoomName,
		StartupRoomTransitionKind *roomTransition,
		Common::String *cutscenePath, Common::String *deathFlicPath, bool *requestMainMenu,
		bool *requestDemoEnding,
		int *cdChangeDisc,
		Common::String *dialogueNpcName, Common::String *dialogueContinuationTag,
		Common::String *continuationTag, ResolvedText *modalText,
		StartupLightingCommand *lightingCommand, bool *requestPlayerGotoXZ,
		int *playerGotoX, int *playerGotoZ,
		bool *requestPlayerDeath, int *playerDeathDamageType,
		bool *mutatedRuntimeState, bool *visualRuntimeStateChanged,
		Common::Array<TimerRecord> *previousTimerRecords, bool *requestCloseupExit);
	bool probePickupBlockingCommandChain(const Common::String &initialTag,
		const Common::String &contextName, InteractionResult &result, uint recursionDepth) const;
	bool hasActionableCommandChain(const Common::String &initialTag) const;
	bool setPlayerCurrentHitPoints(int hitPoints);
	int *getPlayerCombatResourceCountPtr(int loadout);
	const int *getPlayerCombatResourceCountPtr(int loadout) const;
	bool adjustPlayerCombatResourceCount(int loadout, int delta, int maxCount,
		const Common::String &reason);
	void logRuntimeSaveState(const char *operation) const;

	Common::String _path;
	Common::Array<byte> _data;
	Common::Array<EntranceRecord> _entrances;
	Common::Array<MapEntranceRecord> _mapEntrances;
	Common::Array<MapLocationRecord> _mapLocations;
	Common::Array<RoomRecord> _rooms;
	Common::Array<ObjectRecord> _objects;
	Common::Array<AnimRecord> _animations;
	Common::Array<NpcRecord> _npcs;
	Common::Array<MonsterRecord> _monsters;
	Common::Array<TimerRecord> _timers;
	Common::Array<RegionRecord> _regions;
	Common::Array<FlagRecord> _flags;
	Common::Array<CommandRecord> _commands;
	Common::Array<ExecListRecord> _execLists;
	Common::Array<TextRecord> _texts;
	Common::Array<HeadRecord> _heads;
	Common::Array<UseItemRecord> _useItems;
	Common::Array<FlagRecord> _currentFlags;
	Common::Array<ObjectRecord> _currentObjects;
	Common::Array<AnimRecord> _currentAnimations;
	Common::Array<RegionRecord> _currentRegions;
	Common::Array<NpcRecord> _currentNpcs;
	Common::Array<MonsterRecord> _currentMonsters;
	Common::Array<TimerRecord> _currentTimers;
	int _nailgunAmmoCount = 0;
	int _shotgunShellCount = 0;
	int _nineGunBulletCount = 0;
	int _thirtyEightGunBulletCount = 0;
	int _chainsawFuelCount = 0;
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
	int _deferredLiveNpcDeathTransitionDepth = 0;
};

} // End of namespace Harvester

#endif // HARVESTER_SCRIPT_H
