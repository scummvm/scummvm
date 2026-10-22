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

class ScriptExecutionContext {
public:
	ScriptExecutionContext() = default;

	static ScriptExecutionContext forNpcDeath(const NpcRecord &npc) {
		return ScriptExecutionContext(npc.roomName);
	}
	static ScriptExecutionContext forMonsterDeath(const MonsterRecord &monster) {
		return ScriptExecutionContext(monster.roomName);
	}

	const char *roomName() const {
		return _roomName ? _roomName->c_str() : "";
	}

private:
	friend struct SceneState;

	explicit ScriptExecutionContext(const Common::String &roomName) : _roomName(&roomName) {}

	const Common::String *const _roomName = nullptr;
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
	Common::String moveEntityToPlayerZName;
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
	bool requestPlayerGotoZ = false;
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
		const ScriptExecutionContext &executionContext);
	bool resolveRegionInteraction(const RegionRecord &region, InteractionResult &result,
		const ScriptExecutionContext &executionContext);
	bool resolveUseItemInteraction(const Common::String &itemName, const ObjectRecord &target,
		InteractionResult &result, const ScriptExecutionContext &executionContext);
	bool executeDebugCommand(const CommandRecord &command, InteractionResult &result,
		bool allowTransitions = true);
	bool executeActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext);
	bool executeTimerAction(const Common::String &timerName, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext);
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
	bool setPlayerCurrentHitPoints(int hitPoints);
	bool adjustPlayerCurrentHitPoints(int delta);
	bool setPlayerCombatResourceCount(int loadout, int count);
	bool setPlayerCombatLoadout(int loadout);
	bool setPlayerControlPaused(bool paused);
	bool syncRuntimeAnimState(const Common::String &animName, bool active, bool visible, int currentFrame);
	bool syncRuntimeMonsterRecord(const MonsterRecord &monster);
	bool syncRuntimeTimerRecord(const TimerRecord &timer);
	bool setRuntimeRegionEnabled(const Common::String &regionName, bool enabled);
	bool setRuntimeTimerEnabled(const Common::String &timerName, bool enabled);

private:
	static const char *const kInventoryOwnerName;
	static const int kDefaultPlayerCombatLoadout;

	struct CommandExecutionContext {
		const char *label;
		const Common::String &name;
		const Common::String &roomName;
		bool allowTransitions;
	};

	enum CommandOutputPolicy {
		kCommandOutputsInteraction,
		// Suppress destination rooms, death movies and main-menu requests.
		kCommandOutputsRoomExit
	};

	enum CommandStepResult {
		kCommandUnhandled,
		kCommandContinue,
		kCommandStop
	};

	bool loadConfig(ResourceManager &resources);
	void decode();
	void parseTownRecords(ResourceManager &resources);
	void parseTownRecordLine(ResourceManager &resources,
		const Common::String &rawLine, uint32 &anonymousObjectId);
	void parseFlagRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseCommandRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseExecListRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseTextRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseHeadRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseUseItemRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseEntranceRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseMapEntranceRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseMapLocationRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseRoomRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseNpcRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseMonsterRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseTimerRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseRegionRecord(const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseAnimRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex);
	void parseObjectRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex, uint32 &anonymousObjectId);
	static int parseDeathDamageType(const Common::String &value);
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
	const ObjectRecord *findRoomBackground(const RoomRecord &room, ResourceManager &resources) const;
	void collectRuntimeRoomRecords(const RoomRecord &room, RoomSetupState &state) const;
	bool buildRuntimeRoomState(const RoomRecord &room, const EntranceRecord *entrance,
		ResourceManager &resources, RoomSetupState &state) const;
	void executeCommandChain(const Common::String &initialTag, const CommandExecutionContext &context,
		CommandOutputPolicy outputPolicy, InteractionResult &result);
	CommandStepResult executeCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy,
		InteractionResult &result, Common::String &nextTag);
	CommandStepResult executeBranchCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result, Common::String &nextTag);
	CommandStepResult executeAudioCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeObjectCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeAnimationCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeRegionCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeNpcCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeMonsterCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeTimerCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executeExecListCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy, InteractionResult &result);
	CommandStepResult executeModalCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result);
	CommandStepResult executePlayerCommand(const CommandRecord &command,
		InteractionResult &result);
	CommandStepResult executeMovementCommand(const CommandRecord &command,
		InteractionResult &result);
	CommandStepResult executeTransitionCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy, InteractionResult &result);
	static bool hasDeferredInteractionOutputs(const InteractionResult &result, CommandOutputPolicy outputPolicy);
	static int resolveCombatResourcePickupMax(int loadout);
	bool probePickupBlockingCommandChain(const Common::String &initialTag,
		const Common::String &contextName, InteractionResult &result, uint recursionDepth) const;
	bool hasActionableCommandChain(const Common::String &initialTag) const;
	int *getPlayerCombatResourceCountPtr(int loadout);
	const int *getPlayerCombatResourceCountPtr(int loadout) const;
	bool adjustPlayerCombatResourceCount(int loadout, int delta, int maxCount,
		const Common::String &reason);
	void restoreRuntimeFlags(const Common::Array<FlagRecord> &currentFlags);
	void restoreRuntimeObjects(const Common::Array<ObjectRecord> &currentObjects);
	void restoreRuntimeAnimations(const Common::Array<AnimRecord> &currentAnimations);
	void restoreRuntimeRegions(const Common::Array<RegionRecord> &currentRegions);
	void restoreRuntimeNpcs(const Common::Array<NpcRecord> &currentNpcs);
	void restoreRuntimeMonsters(const Common::Array<MonsterRecord> &currentMonsters);
	void restoreRuntimeTimers(const Common::Array<TimerRecord> &currentTimers);
	void logRuntimeSaveFlags(const char *operation) const;
	void logRuntimeSaveObjects(const char *operation) const;
	void logRuntimeSaveNpcs(const char *operation) const;
	void logRuntimeSaveMonsters(const char *operation) const;
	void logRuntimeSaveTimers(const char *operation) const;
	static int clampPlayerHitPoints(int hitPoints);
	static int clampPlayerCombatLoadout(int loadout);
	static int clampCombatResourceCount(int loadout, int count);
	static int validatePlayerCombatLoadoutAgainstInventory(const Common::Array<ObjectRecord> &currentObjects,
		int loadout, const char *contextLabel);
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
