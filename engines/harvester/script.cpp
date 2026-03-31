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

#include "harvester/script.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/parse_utils.h"
#include "harvester/player.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kDefaultVoicePath = "SOUND/VOICE/";
static const byte kTownScriptXorKey = 0xaa;
static const float kDefaultPaletteBrightness = 1.0f;
static const float kDimmedPaletteBrightness = 0.6f;
static const int kDefaultPlayerCombatLoadout = 0;
static const int kMaxPlayerCombatLoadout = 0x14;
static const int kMaxStartupOptionLevel = 9;

struct InventoryCombatLoadoutEntry {
	int loadoutId;
	const char *objectName;
};

static const InventoryCombatLoadoutEntry kInventoryCombatLoadoutMap[] = {
	{ 1, "CLEAVER" },
	{ 2, "NAILGUN" },
	{ 3, "SHOTGUN" },
	{ 4, "9GUN" },
	{ 5, "38GUN" },
	{ 6, "TOMAHAWK" },
	{ 7, "KNIFE" },
	{ 8, "FLAIL" },
	{ 9, "HANDAXE" },
	{ 10, "WRENCH" },
	{ 11, "PITCHFORK" },
	{ 12, "SCYTHE" },
	{ 13, "SWORD" },
	{ 14, "CHAINSAW" },
	{ 15, "HARVEST_BLADE" },
	{ 16, "SHOVEL" },
	{ 17, "FIREAXE" },
	{ 18, "BAT" },
	{ 19, "RAZOR" },
	{ 20, "POOLSTICK" }
};

static void syncBool(Common::Serializer &s, bool &value) {
	byte serialized = value ? 1 : 0;
	s.syncAsByte(serialized);
	if (s.isLoading())
		value = serialized != 0;
}

template <typename RecordType, typename SyncFn>
static void syncRecordArray(Common::Serializer &s, Common::Array<RecordType> &records, SyncFn syncFn) {
	uint32 count = (uint32)records.size();
	s.syncAsUint32LE(count);
	if (s.isLoading())
		records.resize(count);
	for (uint32 i = 0; i < count; ++i)
		syncFn(s, records[i]);
}

static void syncStartupFlagRecord(Common::Serializer &s, StartupFlagRecord &record) {
	s.syncString(record.name);
	syncBool(s, record.value);
}

static void syncStartupObjectRecord(Common::Serializer &s, StartupObjectRecord &record) {
	s.syncAsSint32LE(record.initialX);
	s.syncAsSint32LE(record.initialY);
	s.syncAsSint32LE(record.initialZ);
	s.syncAsSint32LE(record.currentX);
	s.syncAsSint32LE(record.currentY);
	s.syncAsSint32LE(record.currentZ);
	s.syncAsSint32LE(record.boundsX2);
	s.syncAsSint32LE(record.boundsY2);
	s.syncAsSint32LE(record.zExtent);
	s.syncString(record.actionTag);
	s.syncString(record.spritePath);
	s.syncString(record.altSpritePath);
	s.syncString(record.objectName);
	s.syncString(record.reservedXFlag);
	s.syncString(record.identTextKey);
	s.syncString(record.currentOwnerOrRoom);
	s.syncString(record.reservedString40);
	s.syncString(record.inventoryTextKey);
	s.syncString(record.initialOwnerOrRoom);
	s.syncString(record.interactionLabel);
	syncBool(s, record.operatable);
	syncBool(s, record.visible);
	syncBool(s, record.runtimeVisible);
	syncBool(s, record.identShown);
}

static void syncStartupAnimRecord(Common::Serializer &s, StartupAnimRecord &record) {
	s.syncAsSint32LE(record.x);
	s.syncAsSint32LE(record.y);
	s.syncAsSint32LE(record.z);
	s.syncAsSint32LE(record.frameDelay);
	s.syncString(record.roomName);
	s.syncString(record.resourcePath);
	s.syncString(record.animName);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.looping);
	syncBool(s, record.backward);
	syncBool(s, record.pingPong);
	syncBool(s, record.remove);
	syncBool(s, record.runtimeActive);
	syncBool(s, record.runtimeVisible);
	s.syncAsSint32LE(record.runtimeState);
}

static void syncStartupNpcRecord(Common::Serializer &s, StartupNpcRecord &record) {
	s.syncAsSint32LE(record.posX);
	s.syncAsSint32LE(record.posY);
	s.syncAsSint32LE(record.posZ);
	s.syncAsSint32LE(record.frameDelay);
	s.syncString(record.onDeathActionTag);
	s.syncString(record.modelPath);
	s.syncString(record.npcName);
	s.syncString(record.monsterfyTargetName);
	s.syncString(record.roomName);
	syncBool(s, record.deathOrMonsterfyFlag);
	syncBool(s, record.runtimeSpawned);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.savedVisible);
	s.syncAsSint32LE(record.deathDamageType);
	s.syncString(record.audioPath);
	s.syncString(record.entityInitArg);
}

static void syncStartupMonsterRecord(Common::Serializer &s, StartupMonsterRecord &record) {
	s.syncAsSint32LE(record.posX);
	s.syncAsSint32LE(record.posY);
	s.syncAsSint32LE(record.posZ);
	s.syncAsSint32LE(record.facing);
	s.syncString(record.roomName);
	s.syncString(record.monsterName);
	s.syncString(record.modelPath);
	s.syncString(record.onDeathActionTag);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	// FIXME: Remove the pre-v2 monster save-layout fallback after release; clean
	// Harvester saves will always serialize the full combat payload.
	if (s.getVersion() < 2)
		return;

	s.syncAsSint32LE(record.initialHitPoints);
	s.syncAsSint32LE(record.currentHitPoints);
	s.syncAsSint32LE(record.damageAmount);
	s.syncAsSint32LE(record.engageDistance);
	s.syncAsSint32LE(record.damageType);
	s.syncString(record.reservedString38);
	s.syncString(record.reservedString3c);
	s.syncString(record.reservedString44);
	s.syncString(record.reservedString48);
	s.syncString(record.attackSound1);
	s.syncString(record.attackSound2);
	s.syncString(record.attackSound3);
	s.syncString(record.hitSound1);
	s.syncString(record.hitSound2);
	s.syncString(record.hitSound3);
	s.syncString(record.footstepSoundLeft);
	s.syncString(record.footstepSoundRight);
	s.syncString(record.deathSound);
	syncBool(s, record.savedVisible);
	syncBool(s, record.runtimeSpawned);
	s.syncAsSint32LE(record.screenMinXBound);
	s.syncAsSint32LE(record.screenMaxXBound);
	s.syncAsSint32LE(record.attackSoundTriggerFrame);
	s.syncAsSint32LE(record.hitSoundTriggerFrame);
	s.syncAsSint32LE(record.footstepSoundTriggerFrame);
	s.syncAsSint32LE(record.deathSoundTriggerFrame);
	// FIXME: Remove this version-16 gate before release; clean Harvester saves
	// should always serialize monster runtimeState.
	if (s.getVersion() >= 16)
		s.syncAsSint32LE(record.runtimeState);
}

// FIXME: Remove this pre-release monster combat migration after Harvester ships and
// only clean release-era saves remain.
static void migrateLegacyMonsterCombatFields(const StartupMonsterRecord &baseMonster,
		StartupMonsterRecord &runtimeMonster) {
	const int legacyInitialHitPoints = MAX(0, runtimeMonster.initialHitPoints);
	const int legacyCurrentHitPoints = CLIP<int>(runtimeMonster.currentHitPoints, 0, legacyInitialHitPoints);
	const int damageTaken = legacyInitialHitPoints - legacyCurrentHitPoints;

	runtimeMonster.engageDistance = baseMonster.engageDistance;
	runtimeMonster.initialHitPoints = baseMonster.initialHitPoints;
	runtimeMonster.currentHitPoints = MAX(0, baseMonster.initialHitPoints - damageTaken);
	runtimeMonster.damageAmount = baseMonster.damageAmount;
}

static bool hasLegacyMonsterCombatFieldLayout(const StartupMonsterRecord &baseMonster,
		const StartupMonsterRecord &runtimeMonster) {
	return runtimeMonster.initialHitPoints != baseMonster.initialHitPoints ||
		runtimeMonster.damageAmount != baseMonster.damageAmount ||
		runtimeMonster.engageDistance != baseMonster.engageDistance;
}

static void syncStartupTimerRecord(Common::Serializer &s, StartupTimerRecord &record) {
	s.syncAsSint32LE(record.initialValue);
	s.syncAsSint32LE(record.currentValue);
	s.syncString(record.timerName);
	s.syncString(record.arg1);
	s.syncString(record.arg2);
	syncBool(s, record.enabled);
	syncBool(s, record.looping);
	syncBool(s, record.global);
}

static void syncStartupRegionRecord(Common::Serializer &s, StartupRegionRecord &record) {
	s.syncAsSint32LE(record.left);
	s.syncAsSint32LE(record.top);
	s.syncAsSint32LE(record.right);
	s.syncAsSint32LE(record.bottom);
	s.syncAsSint32LE(record.minZ);
	s.syncAsSint32LE(record.maxZ);
	s.syncAsSint32LE(record.desiredFacing);
	s.syncString(record.regionName);
	s.syncString(record.direction);
	s.syncString(record.roomName);
	s.syncString(record.actionTag);
	syncBool(s, record.startEnabled);
	syncBool(s, record.cursorEnabled);
}

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kMaxStartupOptionLevel)
		return kMaxStartupOptionLevel;

	return level;
}

static int clampPlayerHitPoints(int hitPoints) {
	if (hitPoints < 0)
		return 0;
	if (hitPoints > Script::kDefaultPlayerHitPoints)
		return Script::kDefaultPlayerHitPoints;

	return hitPoints;
}

static int clampPlayerCombatLoadout(int loadout) {
	return CLIP<int>(loadout, 0, kMaxPlayerCombatLoadout);
}

static const char *resolveCombatLoadoutInventoryObjectName(int loadout) {
	for (const InventoryCombatLoadoutEntry &entry : kInventoryCombatLoadoutMap) {
		if (entry.loadoutId == loadout)
			return entry.objectName;
	}

	return nullptr;
}

static int validatePlayerCombatLoadoutAgainstInventory(const Common::Array<StartupObjectRecord> &runtimeObjects,
		int loadout, const char *contextLabel) {
	const int clampedLoadout = clampPlayerCombatLoadout(loadout);
	if (clampedLoadout == kDefaultPlayerCombatLoadout)
		return clampedLoadout;

	const char *requiredObjectName = resolveCombatLoadoutInventoryObjectName(clampedLoadout);
	if (!requiredObjectName)
		return clampedLoadout;

	for (const StartupObjectRecord &object : runtimeObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) &&
				object.objectName.equalsIgnoreCase(requiredObjectName))
			return clampedLoadout;
	}

	debugC(1, kDebugGeneral,
		"Harvester: cleared player combat loadout %d('%s') after %s because inventory object '%s' is absent",
		clampedLoadout, Player::describeCombatLoadout(clampedLoadout),
		contextLabel ? contextLabel : "runtime restore", requiredObjectName);
	return kDefaultPlayerCombatLoadout;
}

static const char *resolveInventoryStatusObjectName(int hitPoints) {
	if (hitPoints < 8)
		return "INV_STAT4";
	if (hitPoints < 15)
		return "INV_STAT3";
	if (hitPoints < 23)
		return "INV_STAT2";

	return "INV_STAT1";
}

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
}

static Common::String normalizeInteractionLabel(const Common::String &value) {
	Common::String label = value;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static Common::String toNativeConfigPath(const Common::String &value) {
	Common::String result(value);
	for (uint i = 0; i < result.size(); ++i) {
		if (result[i] == '/')
			result.setChar('\\', i);
	}

	return result;
}

static Common::String dialogueTextModeToConfigValue(StartupDialogueTextMode mode) {
	switch (mode) {
	case kStartupDialogueTextNone:
		return "NO";
	case kStartupDialogueTextClick:
		return "CLICK";
	case kStartupDialogueTextYes:
	default:
		return "YES";
	}
}

static int parseEntranceFacing(const Common::String &direction) {
	if (direction.equalsIgnoreCase("FRONT"))
		return 0;
	if (direction.equalsIgnoreCase("LEFT"))
		return 1;
	if (direction.equalsIgnoreCase("RIGHT"))
		return 2;
	if (direction.equalsIgnoreCase("BACK"))
		return 3;

	return -1;
}

static int parseNpcDeathDamageType(const Common::String &value) {
	if (value.equalsIgnoreCase("BLUDGE"))
		return 1;
	if (value.equalsIgnoreCase("SLASH"))
		return 2;
	if (value.equalsIgnoreCase("PROJ"))
		return 4;

	return 0;
}

static bool appendStartupAudioCommand(const StartupCommandRecord &command, Common::Array<StartupAudioCommand> &commands) {
	if (command.opcodeName.equalsIgnoreCase("START_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartWav;
		audioCommand.path = command.arg1;
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartSingleWav;
		audioCommand.path = command.arg1;
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("LOAD_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandLoadWav;
		audioCommand.path = command.arg1;
		audioCommand.slot = command.arg2.empty() ? -1 : parseAsciiIntOrZero(command.arg2);
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("PLAY_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandPlayWav;
		audioCommand.slot = command.arg1.empty() ? -1 : parseAsciiIntOrZero(command.arg1);
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("DELETE_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandDeleteWav;
		audioCommand.slot = command.arg1.empty() ? -1 : parseAsciiIntOrZero(command.arg1);
		commands.push_back(audioCommand);
		return true;
	}

	return false;
}

static bool loadBitmapDimensions(ResourceManager &resources, const Common::String &path,
		uint32 &width, uint32 &height) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 8)
		return false;

	width = READ_LE_UINT32(data.data());
	height = READ_LE_UINT32(data.data() + 4);
	return true;
}

static bool isRoomBackgroundObject(const StartupObjectRecord &candidate, const StartupRoomRecord &room,
		ResourceManager &resources) {
	if (!candidate.currentOwnerOrRoom.equalsIgnoreCase(room.roomName) ||
			candidate.spritePath.empty() ||
			candidate.initialX != 0 || candidate.initialY != 0) {
		return false;
	}

	uint32 width = 0;
	uint32 height = 0;
	if (!loadBitmapDimensions(resources, candidate.spritePath, width, height))
		return false;

	return width == 640 && height == 480;
}

static void tokenizeTownScriptLine(const Common::String &line, Common::Array<Common::String> &tokens) {
	tokens.clear();

	for (uint i = 0; i < line.size();) {
		while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
			++i;
		if (i >= line.size())
			break;

		if (line[i] == '"') {
			++i;
			Common::String token;
			while (i < line.size() && line[i] != '"')
				token += line[i++];
			if (i < line.size() && line[i] == '"')
				++i;
			tokens.push_back(token);
			continue;
		}

		Common::String token;
		while (i < line.size() && line[i] != ' ' && line[i] != '\t')
			token += line[i++];
		tokens.push_back(token);
	}
}

bool Script::load(ResourceManager &resources) {
	_path = kDefaultTownScript;
	_data.clear();
	_entrances.clear();
	_mapEntrances.clear();
	_mapLocations.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_npcs.clear();
	_monsters.clear();
	_timers.clear();
	_regions.clear();
	_flags.clear();
	_commands.clear();
	_execLists.clear();
	_texts.clear();
	_heads.clear();
	_useItems.clear();
	_runtimeFlags.clear();
	_runtimeObjects.clear();
	_runtimeAnimations.clear();
	_runtimeNpcs.clear();
	_runtimeMonsters.clear();
	_runtimeTimers.clear();
	_runtimeRegions.clear();
	_playerCurrentHitPoints = Script::kDefaultPlayerHitPoints;
	_playerCombatLoadout = kDefaultPlayerCombatLoadout;
	_playerControlPaused = false;
	_fxVolumeLevel = 9;
	_musicVolumeLevel = 3;
	_gammaLevel = 0;
	_quickTipsEnabled = true;
	_goreEnabled = true;
	_voicePath = kDefaultVoicePath;
	_parentalPassword.clear();
	_savePath = "./";
	_townPath = kDefaultTownScript;
	_cdRomPath.clear();
	_drive1Path.clear();
	_drive2Path.clear();
	_drive3Path.clear();
	_dialogueTextMode = kStartupDialogueTextYes;

	loadConfig(resources);

	if (!resources.loadFile(_path, _data)) {
		warning("Harvester: unable to load town script '%s'", _path.c_str());
		return false;
	}

	decode();
	parseTownRecords(resources);
	resetRuntimeState();
	debug(1, "Harvester: loaded startup script '%s' (%u bytes)", _path.c_str(), (uint)_data.size());
	return true;
}

bool Script::reloadTownWorld(ResourceManager &resources) {
	Common::Array<byte> reloadedData;
	if (!resources.loadFile(_path, reloadedData)) {
		warning("Harvester: unable to reload town script '%s'", _path.c_str());
		return false;
	}

	const Common::Array<StartupFlagRecord> runtimeFlags = _runtimeFlags;
	const Common::Array<StartupObjectRecord> runtimeObjects = _runtimeObjects;
	const Common::Array<StartupAnimRecord> runtimeAnimations = _runtimeAnimations;
	const Common::Array<StartupRegionRecord> runtimeRegions = _runtimeRegions;
	const Common::Array<StartupNpcRecord> runtimeNpcs = _runtimeNpcs;
	const Common::Array<StartupMonsterRecord> runtimeMonsters = _runtimeMonsters;
	const Common::Array<StartupTimerRecord> runtimeTimers = _runtimeTimers;
	const int playerCurrentHitPoints = _playerCurrentHitPoints;
	const int playerCombatLoadout = _playerCombatLoadout;
	const bool playerControlPaused = _playerControlPaused;

	_data = reloadedData;
	decode();
	parseTownRecords(resources);
	resetRuntimeState();

	for (const StartupFlagRecord &flag : runtimeFlags) {
		StartupFlagRecord *runtimeFlag = findRuntimeFlag(flag.name);
		if (runtimeFlag) {
			runtimeFlag->value = flag.value;
			continue;
		}

		_runtimeFlags.push_back(flag);
	}

	for (const StartupObjectRecord &object : runtimeObjects) {
		StartupObjectRecord *runtimeObject = findRuntimeObject(object.initialOwnerOrRoom, object.objectName);
		if (!runtimeObject)
			runtimeObject = findRuntimeObject(Common::String(), object.objectName);
		if (!runtimeObject)
			continue;

		runtimeObject->currentX = object.currentX;
		runtimeObject->currentY = object.currentY;
		runtimeObject->currentZ = object.currentZ;
		runtimeObject->currentOwnerOrRoom = object.currentOwnerOrRoom;
		runtimeObject->visible = object.visible;
		runtimeObject->runtimeVisible = object.runtimeVisible;
		runtimeObject->identShown = object.identShown;
	}

	for (const StartupAnimRecord &anim : runtimeAnimations) {
		StartupAnimRecord *runtimeAnim = findRuntimeAnim(anim.animName);
		if (!runtimeAnim)
			continue;

		runtimeAnim->active = anim.active;
		runtimeAnim->visible = anim.visible;
		runtimeAnim->runtimeActive = anim.runtimeActive;
		runtimeAnim->runtimeVisible = anim.runtimeVisible;
		runtimeAnim->runtimeState = anim.runtimeState;
	}

	for (const StartupRegionRecord &region : runtimeRegions) {
		StartupRegionRecord *runtimeRegion = findRuntimeRegion(region.regionName);
		if (!runtimeRegion)
			continue;

		runtimeRegion->startEnabled = region.startEnabled;
	}

	for (const StartupNpcRecord &npc : runtimeNpcs) {
		StartupNpcRecord *runtimeNpc = findRuntimeNpc(npc.npcName);
		if (!runtimeNpc)
			continue;

		runtimeNpc->active = npc.active;
		runtimeNpc->visible = npc.visible;
		runtimeNpc->savedVisible = npc.savedVisible;
		runtimeNpc->runtimeSpawned = npc.runtimeSpawned;
		runtimeNpc->deathOrMonsterfyFlag = npc.deathOrMonsterfyFlag;
		runtimeNpc->deathDamageType = npc.deathDamageType;
	}

	for (const StartupMonsterRecord &monster : runtimeMonsters) {
		StartupMonsterRecord *runtimeMonster = findRuntimeMonster(monster.monsterName);
		if (!runtimeMonster)
			continue;

		runtimeMonster->posX = monster.posX;
		runtimeMonster->posY = monster.posY;
		runtimeMonster->posZ = monster.posZ;
		runtimeMonster->facing = monster.facing;
		runtimeMonster->active = monster.active;
		runtimeMonster->visible = monster.visible;
		runtimeMonster->savedVisible = monster.savedVisible;
		runtimeMonster->runtimeSpawned = monster.runtimeSpawned;
		runtimeMonster->runtimeState = monster.runtimeState;
		runtimeMonster->currentHitPoints = monster.currentHitPoints;
		runtimeMonster->screenMinXBound = monster.screenMinXBound;
		runtimeMonster->screenMaxXBound = monster.screenMaxXBound;
	}

	for (const StartupTimerRecord &timer : runtimeTimers) {
		StartupTimerRecord *runtimeTimer = findRuntimeTimer(timer.timerName);
		if (!runtimeTimer)
			continue;

		runtimeTimer->initialValue = timer.initialValue;
		runtimeTimer->currentValue = timer.currentValue;
		runtimeTimer->enabled = timer.enabled;
		runtimeTimer->looping = timer.looping;
		runtimeTimer->global = timer.global;
	}

	_playerCurrentHitPoints = clampPlayerHitPoints(playerCurrentHitPoints);
	_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
		_runtimeObjects, playerCombatLoadout, "town reload");
	_playerControlPaused = playerControlPaused;

	debugC(1, kDebugGeneral,
		"Harvester: reloaded town script '%s' for disc %d while preserving runtime state",
		_path.c_str(), resources.getCurrentDisc());
	return true;
}

bool Script::loadConfig(ResourceManager &resources) {
	Common::Array<byte> configData;
	if (!resources.loadFile(kStartupConfigPath, configData)) {
		warning("Harvester: unable to load startup config '%s', using defaults", kStartupConfigPath);
		return false;
	}

	Common::MemoryReadStream stream(configData.data(), configData.size());
	Common::INIFile config;
	config.setDefaultSectionName(kConfigSectionName);
	config.requireKeyValueDelimiter();
	config.suppressValuelessLineWarning();

	if (!config.loadFromStream(stream)) {
		warning("Harvester: unable to parse startup config '%s', using defaults", kStartupConfigPath);
		return false;
	}

	Common::String townPath;
	if (config.getKey("TOWN", kConfigSectionName, townPath)) {
		_townPath = townPath;
		_path = resources.normalizeResourcePath(townPath);
	}

	Common::String savePath;
	if (config.getKey("SAVE", kConfigSectionName, savePath))
		_savePath = savePath;

	(void)config.getKey("CD_ROM", kConfigSectionName, _cdRomPath);
	(void)config.getKey("DRIVE_1", kConfigSectionName, _drive1Path);
	(void)config.getKey("DRIVE_2", kConfigSectionName, _drive2Path);
	(void)config.getKey("DRIVE_3", kConfigSectionName, _drive3Path);

	Common::String quickTipsValue;
	if (config.getKey("QUICK_TIPS", kConfigSectionName, quickTipsValue))
		_quickTipsEnabled = quickTipsValue.equalsIgnoreCase("ON");

	Common::String voicePath;
	if (config.getKey("VOICE", kConfigSectionName, voicePath))
		_voicePath = resources.normalizeResourcePath(voicePath);

	Common::String fxVolume;
	if (config.getKey("FX_VOLUME", kConfigSectionName, fxVolume))
		_fxVolumeLevel = clampStartupOptionLevel(parseAsciiIntOrZero(fxVolume));

	Common::String musicVolume;
	if (config.getKey("MUSIC_VOLUME", kConfigSectionName, musicVolume))
		_musicVolumeLevel = clampStartupOptionLevel(parseAsciiIntOrZero(musicVolume));

	Common::String gammaValue;
	if (config.getKey("GAMMA", kConfigSectionName, gammaValue))
		_gammaLevel = clampStartupOptionLevel(parseAsciiIntOrZero(gammaValue));

	Common::String textMode;
	if (config.getKey("TEXT", kConfigSectionName, textMode)) {
		if (textMode.equalsIgnoreCase("NO"))
			_dialogueTextMode = kStartupDialogueTextNone;
		else if (textMode.equalsIgnoreCase("CLICK"))
			_dialogueTextMode = kStartupDialogueTextClick;
		else
			_dialogueTextMode = kStartupDialogueTextYes;
	}

	Common::String goreValue;
	if (config.getKey("GORE", kConfigSectionName, goreValue))
		_goreEnabled = !goreValue.equalsIgnoreCase("NO") && !goreValue.equalsIgnoreCase("OFF");

	Common::String passwordValue;
	if (config.getKey("PASSWORD", kConfigSectionName, passwordValue))
		_parentalPassword = passwordValue;

	return true;
}

bool Script::saveConfig() const {
	Common::DumpFile file;
	const Common::Path configPath = ConfMan.getPath("path").join(kStartupConfigPath, Common::Path::kNoSeparator);
	if (!file.open(configPath)) {
		warning("Harvester: unable to save startup config '%s'", configPath.toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	auto writeKey = [&](const char *key, const Common::String &value) {
		file.writeString(Common::String::format("%s=%s\n", key, value.c_str()));
	};

	writeKey("FX_VOLUME", Common::String::format("%d", _fxVolumeLevel));
	writeKey("MUSIC_VOLUME", Common::String::format("%d", _musicVolumeLevel));
	writeKey("TEXT", dialogueTextModeToConfigValue(_dialogueTextMode));
	writeKey("GORE", _goreEnabled ? "YES" : "NO");
	writeKey("QUICK_TIPS", _quickTipsEnabled ? "ON" : "OFF");
	if (!_savePath.empty())
		writeKey("SAVE", toNativeConfigPath(_savePath));
	writeKey("VOICE", toNativeConfigPath(_voicePath));
	writeKey("GAMMA", Common::String::format("%d", _gammaLevel));
	writeKey("TOWN", toNativeConfigPath(_townPath));
	if (!_parentalPassword.empty())
		writeKey("PASSWORD", _parentalPassword);
	if (!_cdRomPath.empty())
		writeKey("CD_ROM", _cdRomPath);
	if (!_drive1Path.empty())
		writeKey("DRIVE_1", _drive1Path);
	if (!_drive2Path.empty())
		writeKey("DRIVE_2", _drive2Path);
	if (!_drive3Path.empty())
		writeKey("DRIVE_3", _drive3Path);

	file.flush();
	return !file.err();
}

void Script::setFxVolumeLevel(int level) {
	_fxVolumeLevel = clampStartupOptionLevel(level);
}

void Script::setMusicVolumeLevel(int level) {
	_musicVolumeLevel = clampStartupOptionLevel(level);
}

void Script::setGammaLevel(int level) {
	_gammaLevel = clampStartupOptionLevel(level);
}

void Script::decode() {
	for (byte &value : _data) {
		if (value == '\r' || value == '\n')
			continue;

		value ^= kTownScriptXorKey;
	}
}

void Script::parseTownRecords(ResourceManager &resources) {
	_entrances.clear();
	_mapEntrances.clear();
	_mapLocations.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_npcs.clear();
	_monsters.clear();
	_timers.clear();
	_regions.clear();
	_flags.clear();
	_commands.clear();
	_execLists.clear();
	_texts.clear();
	_heads.clear();
	_useItems.clear();

	uint32 anonymousObjectId = 0;
	auto parseLine = [&](const Common::String &rawLine) {
		const Common::String line = trimAsciiLine(rawLine);
		if (line.empty() || line[0] == '{')
			return;

		Common::Array<Common::String> tokens;
		tokenizeTownScriptLine(line, tokens);
		if (tokens.empty())
			return;

		uint tagIndex = tokens.size();
		for (uint i = 0; i < tokens.size(); ++i) {
			if (tokens[i] == "ANIM" || tokens[i] == "ENTRANCE" || tokens[i] == "MAP_ENTRANCE" ||
				tokens[i] == "MAP_LOCATION" || tokens[i] == "ROOM" || tokens[i] == "OBJECT" ||
				tokens[i] == "NPC" || tokens[i] == "MONSTER" || tokens[i] == "REGION" ||
				tokens[i] == "HEAD" || tokens[i] == "FLAG" || tokens[i] == "COMMAND" ||
				tokens[i] == "EXEC_LIST" || tokens[i] == "TEXT" || tokens[i] == "TIMER" ||
				tokens[i] == "USEITEM") {
				tagIndex = i;
				break;
			}
		}
		if (tagIndex == tokens.size())
			return;

		const Common::String &tag = tokens[tagIndex];
		if (tag == "FLAG") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupFlagRecord flag;
			flag.name = tokens[tagIndex + 1];
			flag.value = tokens[tagIndex + 2].equalsIgnoreCase("T");
			if (!flag.name.empty())
				_flags.push_back(flag);
			return;
		}

		if (tag == "COMMAND") {
			if (tokens.size() < tagIndex + 6)
				return;

			StartupCommandRecord command;
			command.triggerTag = tokens[tagIndex + 1];
			command.opcodeName = tokens[tagIndex + 2];
			command.arg1 = resources.normalizeResourcePath(tokens[tagIndex + 3]);
			command.arg2 = tokens[tagIndex + 4];
			command.arg3 = tokens[tagIndex + 5];
			command.arg4 = tokens.size() > tagIndex + 6 ? tokens[tagIndex + 6] : Common::String();

			// Only path-like operands should be normalized.
			if (!command.opcodeName.equalsIgnoreCase("SPOOL_MUSIC") &&
				!command.opcodeName.equalsIgnoreCase("GOFLIC") &&
				!command.opcodeName.equalsIgnoreCase("START_WAV") &&
				!command.opcodeName.equalsIgnoreCase("LOAD_WAV") &&
				!command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
				command.arg1 = tokens[tagIndex + 3];
			}

			if (!command.triggerTag.empty() && !command.opcodeName.empty())
				_commands.push_back(command);
			return;
		}

		if (tag == "EXEC_LIST") {
			if (tokens.size() < tagIndex + 2)
				return;

			StartupExecListRecord execList;
			execList.listName = tokens[tagIndex + 1];
			for (uint i = tagIndex + 2; i < tokens.size(); ++i) {
				if (tokens[i].empty())
					break;
				execList.entries.push_back(tokens[i]);
			}
			if (!execList.listName.empty())
				_execLists.push_back(execList);
			return;
		}

		if (tag == "TEXT") {
			if (tokens.size() < tagIndex + 4)
				return;

			StartupTextRecord textRecord;
			textRecord.key = tokens[tagIndex + 1];
			textRecord.boxName = tokens[tagIndex + 2];
			textRecord.value = tokens[tagIndex + 3];
			for (uint i = 0; i < textRecord.value.size(); ++i) {
				if (textRecord.value[i] == '_')
					textRecord.value.setChar(' ', i);
			}
			if (!textRecord.key.empty())
				_texts.push_back(textRecord);
			return;
		}

		if (tag == "HEAD") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupHeadRecord head;
			head.headId = tokens[tagIndex + 1];
			head.portraitPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			if (!head.headId.empty() && !head.portraitPath.empty())
				_heads.push_back(head);
			return;
		}

		if (tag == "USEITEM") {
			if (tokens.size() < tagIndex + 5)
				return;

			StartupUseItemRecord useItem;
			useItem.itemName = tokens[tagIndex + 1];
			useItem.ownerOrRoom = tokens[tagIndex + 2];
			useItem.targetName = tokens[tagIndex + 3];
			useItem.actionTag = tokens[tagIndex + 4];
			if (!useItem.itemName.empty() && !useItem.targetName.empty())
				_useItems.push_back(useItem);
			return;
		}

		if (tag == "ENTRANCE") {
			if (tokens.size() < tagIndex + 4)
				return;

			StartupEntranceRecord entrance;
			if (tagIndex >= 3) {
				entrance.posX = parseAsciiIntOrZero(tokens[0]);
				entrance.posY = parseAsciiIntOrZero(tokens[1]);
				entrance.posZ = parseAsciiIntOrZero(tokens[2]);
			}
			entrance.direction = tokens[tagIndex + 1];
			entrance.facing = parseEntranceFacing(entrance.direction);
			entrance.roomName = tokens[tagIndex + 2];
			entrance.entranceName = tokens[tagIndex + 3];
			if (!entrance.roomName.empty() && !entrance.entranceName.empty())
				_entrances.push_back(entrance);
			return;
		}

		if (tag == "MAP_ENTRANCE") {
			if (tokens.size() < tagIndex + 2)
				return;

			StartupMapEntranceRecord mapEntrance;
			if (tagIndex >= 3) {
				mapEntrance.mapX = parseAsciiIntOrZero(tokens[0]);
				mapEntrance.mapY = parseAsciiIntOrZero(tokens[1]);
				mapEntrance.initialPanelIndex = parseAsciiIntOrZero(tokens[2]);
			}
			mapEntrance.entryName = tokens[tagIndex + 1];
			if (!mapEntrance.entryName.empty())
				_mapEntrances.push_back(mapEntrance);
			return;
		}

		if (tag == "MAP_LOCATION") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupMapLocationRecord mapLocation;
			if (tagIndex >= 7) {
				mapLocation.minX = parseAsciiIntOrZero(tokens[0]);
				mapLocation.minY = parseAsciiIntOrZero(tokens[1]);
				mapLocation.maxX = parseAsciiIntOrZero(tokens[2]);
				mapLocation.maxY = parseAsciiIntOrZero(tokens[3]);
				mapLocation.panelIndex = parseAsciiIntOrZero(tokens[4]);
				mapLocation.labelX = parseAsciiIntOrZero(tokens[5]);
				mapLocation.labelY = parseAsciiIntOrZero(tokens[6]);
			}
			mapLocation.labelText = tokens[tagIndex + 1];
			for (uint i = 0; i < mapLocation.labelText.size(); ++i) {
				if (mapLocation.labelText[i] == '_')
					mapLocation.labelText.setChar(' ', i);
			}
			mapLocation.destinationEntranceName = tokens[tagIndex + 2];
			if (!mapLocation.destinationEntranceName.empty())
				_mapLocations.push_back(mapLocation);
			return;
		}

		if (tag == "ROOM") {
			if (tokens.size() < tagIndex + 10)
				return;

			StartupRoomRecord room;
			if (tagIndex >= 6) {
				room.minZ = parseAsciiIntOrZero(tokens[0]);
				room.maxZ = parseAsciiIntOrZero(tokens[1]);
				room.maxZScreenY = parseAsciiIntOrZero(tokens[2]);
				room.minZScreenY = parseAsciiIntOrZero(tokens[3]);
				room.fullScaleZ = parseAsciiIntOrZero(tokens[4]);
				room.maxZScalePercent = parseAsciiIntOrZero(tokens[5]);
				if (room.maxZ != room.fullScaleZ) {
					room.perspectiveScale = ((100.0f - (float)room.maxZScalePercent) /
						(float)(room.maxZ - room.fullScaleZ)) * 0.01f;
				}
			}
			room.roomName = tokens[tagIndex + 1];
			room.musicPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			room.reservedString38 = tokens[tagIndex + 3];
			room.reservedString3c = tokens[tagIndex + 4];
			room.reservedString40 = tokens[tagIndex + 5];
			room.palettePath = resources.normalizeResourcePath(tokens[tagIndex + 6]);
			room.dimmable = tokens[tagIndex + 7].equalsIgnoreCase("T");
			room.onEnterCommand = tokens[tagIndex + 8];
			room.onExitCommand = tokens[tagIndex + 9];
			if (!room.roomName.empty())
				_rooms.push_back(room);
			return;
		}

		if (tag == "NPC") {
			if (tokens.size() < tagIndex + 10)
				return;

			StartupNpcRecord npc;
			if (tagIndex >= 4) {
				npc.posX = parseAsciiIntOrZero(tokens[0]);
				npc.posY = parseAsciiIntOrZero(tokens[1]);
				npc.posZ = parseAsciiIntOrZero(tokens[2]);
				npc.frameDelay = parseAsciiIntOrZero(tokens[3]);
			}
			npc.roomName = tokens[tagIndex + 1];
			npc.modelPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			npc.npcName = tokens[tagIndex + 3];
			npc.monsterfyTargetName = tokens[tagIndex + 4];
			npc.active = tokens[tagIndex + 5].equalsIgnoreCase("T");
			npc.visible = tokens[tagIndex + 6].equalsIgnoreCase("T");
			if (npc.active)
				npc.visible = true;
			npc.savedVisible = npc.visible;
			npc.onDeathActionTag = tokens[tagIndex + 7];
			npc.audioPath = resources.normalizeResourcePath(tokens[tagIndex + 8]);
			npc.entityInitArg = tokens[tagIndex + 9];
			if (!npc.roomName.empty() && !npc.modelPath.empty() && !npc.npcName.empty())
				_npcs.push_back(npc);
			return;
		}

		if (tag == "MONSTER") {
			if (tokens.size() < tagIndex + 24)
				return;

			StartupMonsterRecord monster;
			if (tagIndex >= 6) {
				monster.posX = parseAsciiIntOrZero(tokens[0]);
				monster.posY = parseAsciiIntOrZero(tokens[1]);
				monster.posZ = parseAsciiIntOrZero(tokens[2]);
				// Native MonsterRecord numeric order is engage distance, initial HP, then damage.
				monster.engageDistance = parseAsciiIntOrZero(tokens[3]);
				monster.initialHitPoints = parseAsciiIntOrZero(tokens[4]);
				monster.damageAmount = parseAsciiIntOrZero(tokens[5]);
			}
			if (tagIndex >= 10) {
				monster.attackSoundTriggerFrame = parseAsciiIntOrZero(tokens[6]);
				monster.hitSoundTriggerFrame = parseAsciiIntOrZero(tokens[7]);
				monster.footstepSoundTriggerFrame = parseAsciiIntOrZero(tokens[8]);
				monster.deathSoundTriggerFrame = parseAsciiIntOrZero(tokens[9]);
			}
			monster.roomName = tokens[tagIndex + 1];
			monster.monsterName = tokens[tagIndex + 2];
			monster.modelPath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
			monster.reservedString44 = tokens[tagIndex + 4];
			monster.reservedString48 = tokens[tagIndex + 5];
			monster.reservedString38 = tokens[tagIndex + 6];
			monster.reservedString3c = tokens[tagIndex + 7];
			monster.facing = parseEntranceFacing(tokens[tagIndex + 8]);
			monster.damageType = parseNpcDeathDamageType(tokens[tagIndex + 9]);
			monster.attackSound1 = resources.normalizeResourcePath(tokens[tagIndex + 10]);
			monster.attackSound2 = resources.normalizeResourcePath(tokens[tagIndex + 11]);
			monster.attackSound3 = resources.normalizeResourcePath(tokens[tagIndex + 12]);
			monster.hitSound1 = resources.normalizeResourcePath(tokens[tagIndex + 13]);
			monster.hitSound2 = resources.normalizeResourcePath(tokens[tagIndex + 14]);
			monster.hitSound3 = resources.normalizeResourcePath(tokens[tagIndex + 15]);
			monster.footstepSoundLeft = resources.normalizeResourcePath(tokens[tagIndex + 16]);
			monster.footstepSoundRight = resources.normalizeResourcePath(tokens[tagIndex + 17]);
			monster.deathSound = resources.normalizeResourcePath(tokens[tagIndex + 18]);
			monster.active = tokens[tagIndex + 19].equalsIgnoreCase("T");
			monster.visible = tokens[tagIndex + 20].equalsIgnoreCase("T");
			monster.onDeathActionTag = tokens[tagIndex + 21];
			if (tokens.size() > tagIndex + 22 && !tokens[tagIndex + 22].empty())
				monster.screenMinXBound = parseAsciiIntOrZero(tokens[tagIndex + 22]);
			if (tokens.size() > tagIndex + 23 && !tokens[tagIndex + 23].empty())
				monster.screenMaxXBound = parseAsciiIntOrZero(tokens[tagIndex + 23]);
			monster.currentHitPoints = monster.initialHitPoints;
			monster.savedVisible = monster.visible;
			if (monster.active)
				monster.visible = true;
			if (!monster.roomName.empty() && !monster.monsterName.empty() && !monster.modelPath.empty())
				_monsters.push_back(monster);
			return;
		}

		if (tag == "TIMER") {
			if (tokens.size() < tagIndex + 7)
				return;

			StartupTimerRecord timer;
			if (tagIndex >= 1) {
				timer.initialValue = parseAsciiIntOrZero(tokens[0]);
				timer.currentValue = timer.initialValue;
			}
			timer.timerName = tokens[tagIndex + 1];
			timer.arg1 = tokens[tagIndex + 2];
			timer.arg2 = tokens[tagIndex + 3];
			timer.enabled = tokens[tagIndex + 4].equalsIgnoreCase("T");
			timer.looping = tokens[tagIndex + 5].equalsIgnoreCase("T");
			timer.global = tokens[tagIndex + 6].equalsIgnoreCase("T");
			if (!timer.timerName.empty())
				_timers.push_back(timer);
			return;
		}

		if (tag == "REGION") {
			if (tokens.size() < tagIndex + 7)
				return;

			StartupRegionRecord region;
			if (tagIndex >= 6) {
				region.left = parseAsciiIntOrZero(tokens[0]);
				region.top = parseAsciiIntOrZero(tokens[1]);
				region.right = parseAsciiIntOrZero(tokens[2]);
				region.bottom = parseAsciiIntOrZero(tokens[3]);
				region.minZ = parseAsciiIntOrZero(tokens[4]);
				region.maxZ = parseAsciiIntOrZero(tokens[5]);
			}
			region.regionName = tokens[tagIndex + 1];
			region.direction = tokens[tagIndex + 2];
			region.desiredFacing = parseEntranceFacing(region.direction);
			region.roomName = tokens[tagIndex + 3];
			region.actionTag = tokens[tagIndex + 4];
			region.startEnabled = tokens[tagIndex + 5].equalsIgnoreCase("T");
			region.cursorEnabled = tokens[tagIndex + 6].equalsIgnoreCase("T");
			if (!region.roomName.empty() && !region.regionName.empty())
				_regions.push_back(region);
			return;
		}

		if (tag == "ANIM") {
			if (tokens.size() < tagIndex + 10)
				return;

			StartupAnimRecord anim;
			if (tagIndex >= 4) {
				anim.x = parseAsciiIntOrZero(tokens[0]);
				anim.y = parseAsciiIntOrZero(tokens[1]);
				anim.z = parseAsciiIntOrZero(tokens[2]);
				anim.frameDelay = parseAsciiIntOrZero(tokens[3]);
			}
			anim.roomName = tokens[tagIndex + 1];
			anim.resourcePath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			anim.animName = tokens[tagIndex + 3];
			anim.active = tokens[tagIndex + 4].equalsIgnoreCase("T");
			anim.visible = tokens[tagIndex + 5].equalsIgnoreCase("T");
			anim.looping = tokens[tagIndex + 6].equalsIgnoreCase("T");
			anim.backward = tokens[tagIndex + 7].equalsIgnoreCase("T");
			anim.pingPong = tokens[tagIndex + 8].equalsIgnoreCase("T");
			anim.remove = tokens[tagIndex + 9].equalsIgnoreCase("T");
			anim.runtimeActive = anim.active;
			anim.runtimeVisible = anim.visible;
			if (!anim.roomName.empty() && !anim.resourcePath.empty() && !anim.animName.empty())
				_animations.push_back(anim);
			return;
		}

		if (tokens.size() < tagIndex + 13)
			return;

		StartupObjectRecord object;
		if (tagIndex >= 6) {
			object.initialX = parseAsciiIntOrZero(tokens[0]);
			object.initialY = parseAsciiIntOrZero(tokens[1]);
			object.boundsX2 = parseAsciiIntOrZero(tokens[2]);
			object.boundsY2 = parseAsciiIntOrZero(tokens[3]);
			object.initialZ = parseAsciiIntOrZero(tokens[4]);
			object.zExtent = parseAsciiIntOrZero(tokens[5]);
		}
		object.currentX = object.initialX;
		object.currentY = object.initialY;
		object.currentZ = object.initialZ;
		object.initialOwnerOrRoom = tokens[tagIndex + 1];
		object.objectName = tokens[tagIndex + 2];
		object.spritePath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
		object.altSpritePath = resources.normalizeResourcePath(tokens[tagIndex + 4]);
		object.reservedString40 = tokens[tagIndex + 5];
		object.inventoryTextKey = tokens[tagIndex + 6];
		object.reservedXFlag = tokens[tagIndex + 7];
		object.identTextKey = tokens[tagIndex + 8];
		object.operatable = tokens[tagIndex + 9].equalsIgnoreCase("T");
		object.visible = tokens[tagIndex + 10].equalsIgnoreCase("T");
		object.actionTag = tokens[tagIndex + 11];
		object.interactionLabel = tokens[tagIndex + 12];
		object.currentOwnerOrRoom = object.initialOwnerOrRoom;
		object.runtimeVisible = object.visible;
		object.identShown = object.identTextKey.empty();
		if (object.objectName.empty()) {
			// Native room scripts use unnamed OBJECT entries as blocker rectangles.
			object.objectName = Common::String::format("__ANON_OBJECT_%u", anonymousObjectId++);
		}
		if (!object.initialOwnerOrRoom.empty())
			_objects.push_back(object);
		};

	Common::String line;
	for (uint i = 0; i < _data.size(); ++i) {
		const char ch = (char)_data[i];
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			parseLine(line);
			line.clear();
			continue;
		}

		line += ch;
	}

	parseLine(line);

	debug(1, "Harvester: parsed %u entrances, %u map entrances, %u map locations, %u rooms, %u objects, %u anims, %u npcs, %u regions, %u flags, %u commands, %u exec_lists, %u texts, %u heads, %u useitems from '%s'",
		(uint)_entrances.size(), (uint)_mapEntrances.size(), (uint)_mapLocations.size(),
		(uint)_rooms.size(), (uint)_objects.size(), (uint)_animations.size(),
		(uint)_npcs.size(), (uint)_regions.size(),
		(uint)_flags.size(), (uint)_commands.size(), (uint)_execLists.size(),
		(uint)_texts.size(), (uint)_heads.size(),
		(uint)_useItems.size(), _path.c_str());
}

bool Script::resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources) {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = entrance ? entrance->roomName : entranceName;
	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved room setup target '%s' (resolved room '%s')",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	if (!buildRuntimeRoomState(*room, entrance, resources, state))
		return false;

	debugC(1, kDebugRoom,
		"Harvester: resolveRoomSetupState('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		entranceName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		(double)state.paletteBrightness, (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), (uint)state.roomNpcs.size(),
		(uint)state.roomMonsters.size(), (uint)state.roomTimers.size(),
		(uint)state.roomRegions.size());

	return true;
}

bool Script::resolveRoomSetupStateByRoomName(const Common::String &roomName, StartupRoomSetupState &state,
		ResourceManager &resources) {
	state = StartupRoomSetupState();

	const StartupRoomRecord *room = findRoomRecord(roomName);
	if (!room) {
		warning("Harvester: unresolved room setup room '%s'", roomName.c_str());
		return false;
	}

	const StartupEntranceRecord *entrance = nullptr;
	for (const StartupEntranceRecord &candidate : _entrances) {
		if (!candidate.roomName.equalsIgnoreCase(room->roomName))
			continue;

		entrance = &candidate;
		break;
	}

	if (!buildRuntimeRoomState(*room, entrance, resources, state))
		return false;

	debugC(1, kDebugRoom,
		"Harvester: resolveRoomSetupStateByRoomName('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		roomName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		(double)state.paletteBrightness, (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), (uint)state.roomNpcs.size(),
		(uint)state.roomMonsters.size(), (uint)state.roomTimers.size(),
		(uint)state.roomRegions.size());

	return true;
}

void Script::resetRuntimeState() {
	_runtimeFlags = _flags;
	_runtimeObjects = _objects;
	_runtimeAnimations = _animations;
	_runtimeRegions = _regions;
	_runtimeNpcs = _npcs;
	_runtimeMonsters = _monsters;
	_runtimeTimers = _timers;
	_playerCurrentHitPoints = Script::kDefaultPlayerHitPoints;
	_playerCombatLoadout = kDefaultPlayerCombatLoadout;
	_playerControlPaused = false;

	for (StartupObjectRecord &object : _runtimeObjects) {
		object.currentX = object.initialX;
		object.currentY = object.initialY;
		object.currentZ = object.initialZ;
		object.currentOwnerOrRoom = object.initialOwnerOrRoom;
		object.runtimeVisible = object.visible;
		object.identShown = object.identTextKey.empty();
	}

	for (StartupAnimRecord &anim : _runtimeAnimations) {
		anim.runtimeActive = anim.active;
		anim.runtimeVisible = anim.visible;
		anim.runtimeState = -1;
	}

	for (StartupNpcRecord &npc : _runtimeNpcs) {
		npc.runtimeSpawned = false;
		npc.savedVisible = npc.visible;
	}

	for (StartupMonsterRecord &monster : _runtimeMonsters) {
		monster.currentHitPoints = monster.initialHitPoints;
		monster.runtimeSpawned = false;
		monster.runtimeState = -1;
		monster.savedVisible = monster.visible;
		if (monster.active)
			monster.visible = true;
	}

	for (StartupTimerRecord &timer : _runtimeTimers)
		timer.currentValue = timer.initialValue;
}

void Script::logRuntimeSaveState(const char *operation) const {
	debugC(1, kDebugGeneral,
		"Harvester: %s runtime save state flags=%u objects=%u anims=%u regions=%u npcs=%u monsters=%u timers=%u hp=%d loadout=%d paused=%d",
		operation, (uint)_runtimeFlags.size(), (uint)_runtimeObjects.size(),
		(uint)_runtimeAnimations.size(), (uint)_runtimeRegions.size(),
		(uint)_runtimeNpcs.size(), (uint)_runtimeMonsters.size(),
		(uint)_runtimeTimers.size(), _playerCurrentHitPoints, _playerCombatLoadout,
		_playerControlPaused);

	for (const StartupFlagRecord &flag : _runtimeFlags) {
		const StartupFlagRecord *baseFlag = nullptr;
		for (const StartupFlagRecord &candidate : _flags) {
			if (candidate.name.equalsIgnoreCase(flag.name)) {
				baseFlag = &candidate;
				break;
			}
		}
		if (!baseFlag || flag.value != baseFlag->value) {
			debugC(1, kDebugGeneral,
				"Harvester: %s runtime flag '%s' value=%d base=%d defined_in_script=%d",
				operation, flag.name.c_str(), flag.value, baseFlag ? baseFlag->value : 0,
				baseFlag != nullptr);
		}
	}

	for (const StartupObjectRecord &object : _runtimeObjects) {
		const StartupObjectRecord *baseObject = nullptr;
		for (const StartupObjectRecord &candidate : _objects) {
			if (candidate.objectName.equalsIgnoreCase(object.objectName)) {
				baseObject = &candidate;
				break;
			}
		}

		const Common::String &defaultOwner = baseObject ? baseObject->initialOwnerOrRoom : object.initialOwnerOrRoom;
		const int defaultX = baseObject ? baseObject->initialX : object.initialX;
		const int defaultY = baseObject ? baseObject->initialY : object.initialY;
		const int defaultZ = baseObject ? baseObject->initialZ : object.initialZ;
		const bool defaultRuntimeVisible = baseObject ? baseObject->visible : object.visible;
		const bool defaultVisible = baseObject ? baseObject->visible : object.visible;
		const bool defaultIdentShown = baseObject ? baseObject->identTextKey.empty() : object.identTextKey.empty();
		if (!baseObject ||
				!object.currentOwnerOrRoom.equalsIgnoreCase(defaultOwner) ||
				object.currentX != defaultX ||
				object.currentY != defaultY ||
				object.currentZ != defaultZ ||
				object.runtimeVisible != defaultRuntimeVisible ||
				object.visible != defaultVisible ||
				object.identShown != defaultIdentShown) {
			debugC(1, kDebugGeneral,
				"Harvester: %s runtime object '%s' owner='%s' pos=(%d,%d,%d) visible=%d runtimeVisible=%d identShown=%d base_owner='%s' base_pos=(%d,%d,%d) base_visible=%d base_runtimeVisible=%d base_identShown=%d",
				operation, object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
				object.currentX, object.currentY, object.currentZ,
				object.visible, object.runtimeVisible, object.identShown,
				defaultOwner.c_str(), defaultX, defaultY, defaultZ,
				defaultVisible, defaultRuntimeVisible, defaultIdentShown);
		}
	}

	for (const StartupNpcRecord &npc : _runtimeNpcs) {
		const StartupNpcRecord *baseNpc = nullptr;
		for (const StartupNpcRecord &candidate : _npcs) {
			if (candidate.npcName.equalsIgnoreCase(npc.npcName)) {
				baseNpc = &candidate;
				break;
			}
		}

		const bool defaultActive = baseNpc ? baseNpc->active : npc.active;
		const bool defaultVisible = baseNpc ? baseNpc->visible : npc.visible;
		const bool defaultSavedVisible = baseNpc ? baseNpc->visible : npc.visible;
		const bool defaultDeathOrMonsterfy = baseNpc ? baseNpc->deathOrMonsterfyFlag : npc.deathOrMonsterfyFlag;
		const int defaultDeathDamageType = baseNpc ? baseNpc->deathDamageType : npc.deathDamageType;
		if (!baseNpc ||
				npc.active != defaultActive ||
				npc.visible != defaultVisible ||
				npc.savedVisible != defaultSavedVisible ||
				npc.deathOrMonsterfyFlag != defaultDeathOrMonsterfy ||
				npc.deathDamageType != defaultDeathDamageType ||
				npc.runtimeSpawned) {
			debugC(1, kDebugGeneral,
				"Harvester: %s runtime npc '%s' active=%d visible=%d savedVisible=%d spawned=%d deathOrMonsterfy=%d damageType=%d base_active=%d base_visible=%d base_savedVisible=%d base_deathOrMonsterfy=%d base_damageType=%d",
				operation, npc.npcName.c_str(), npc.active, npc.visible, npc.savedVisible,
				npc.runtimeSpawned, npc.deathOrMonsterfyFlag, npc.deathDamageType,
				defaultActive, defaultVisible, defaultSavedVisible,
				defaultDeathOrMonsterfy, defaultDeathDamageType);
		}
	}

	for (const StartupMonsterRecord &monster : _runtimeMonsters) {
		const StartupMonsterRecord *baseMonster = nullptr;
		for (const StartupMonsterRecord &candidate : _monsters) {
			if (candidate.monsterName.equalsIgnoreCase(monster.monsterName)) {
				baseMonster = &candidate;
				break;
			}
		}

		const bool defaultActive = baseMonster ? baseMonster->active : monster.active;
		const bool defaultVisible = baseMonster
			? (baseMonster->active ? true : baseMonster->visible)
			: (monster.active ? true : monster.visible);
		const bool defaultSavedVisible = defaultVisible;
		const int defaultCurrentHitPoints = baseMonster ? baseMonster->initialHitPoints : monster.initialHitPoints;
		const int defaultScreenMinXBound = baseMonster ? baseMonster->screenMinXBound : monster.screenMinXBound;
		const int defaultScreenMaxXBound = baseMonster ? baseMonster->screenMaxXBound : monster.screenMaxXBound;
		if (!baseMonster ||
				monster.active != defaultActive ||
				monster.visible != defaultVisible ||
				monster.savedVisible != defaultSavedVisible ||
				monster.currentHitPoints != defaultCurrentHitPoints ||
				monster.runtimeSpawned ||
				monster.runtimeState != -1 ||
				monster.screenMinXBound != defaultScreenMinXBound ||
				monster.screenMaxXBound != defaultScreenMaxXBound) {
			debugC(1, kDebugGeneral,
				"Harvester: %s runtime monster '%s' active=%d visible=%d savedVisible=%d spawned=%d runtimeState=%d hp=%d/%d screen_bounds=[%d,%d] base_active=%d base_visible=%d base_savedVisible=%d base_runtimeState=%d base_hp=%d/%d base_screen_bounds=[%d,%d]",
				operation, monster.monsterName.c_str(), monster.active, monster.visible,
				monster.savedVisible, monster.runtimeSpawned, monster.runtimeState,
				monster.currentHitPoints, monster.initialHitPoints,
				monster.screenMinXBound, monster.screenMaxXBound,
				defaultActive, defaultVisible, defaultSavedVisible, -1,
				defaultCurrentHitPoints, baseMonster ? baseMonster->initialHitPoints : monster.initialHitPoints,
				defaultScreenMinXBound, defaultScreenMaxXBound);
		}
	}

	for (const StartupTimerRecord &timer : _runtimeTimers) {
		const StartupTimerRecord *baseTimer = nullptr;
		for (const StartupTimerRecord &candidate : _timers) {
			if (candidate.timerName.equalsIgnoreCase(timer.timerName)) {
				baseTimer = &candidate;
				break;
			}
		}

		const int defaultInitialValue = baseTimer ? baseTimer->initialValue : timer.initialValue;
		const int defaultCurrentValue = defaultInitialValue;
		const bool defaultEnabled = baseTimer ? baseTimer->enabled : timer.enabled;
		const bool defaultLooping = baseTimer ? baseTimer->looping : timer.looping;
		const bool defaultGlobal = baseTimer ? baseTimer->global : timer.global;
		if (!baseTimer ||
				timer.initialValue != defaultInitialValue ||
				timer.currentValue != defaultCurrentValue ||
				timer.enabled != defaultEnabled ||
				timer.looping != defaultLooping ||
				timer.global != defaultGlobal) {
			debugC(1, kDebugGeneral,
				"Harvester: %s runtime timer '%s' current=%d initial=%d enabled=%d loop=%d global=%d base_current=%d base_initial=%d base_enabled=%d base_loop=%d base_global=%d",
				operation, timer.timerName.c_str(), timer.currentValue, timer.initialValue,
				timer.enabled, timer.looping, timer.global,
				defaultCurrentValue, defaultInitialValue, defaultEnabled, defaultLooping, defaultGlobal);
		}
	}
}

void Script::syncRuntimeSaveState(Common::Serializer &s) {
	if (s.isSaving())
		logRuntimeSaveState("saving");

	syncRecordArray(s, _runtimeFlags, syncStartupFlagRecord);
	syncRecordArray(s, _runtimeObjects, syncStartupObjectRecord);
	syncRecordArray(s, _runtimeAnimations, syncStartupAnimRecord);
	syncRecordArray(s, _runtimeRegions, syncStartupRegionRecord);
	syncRecordArray(s, _runtimeNpcs, syncStartupNpcRecord);
	syncRecordArray(s, _runtimeMonsters, syncStartupMonsterRecord);
	s.syncAsSint32LE(_playerCurrentHitPoints);
	// FIXME: Remove this pre-v2 save-layout gate after release; clean Harvester
	// saves always carry timer state, combat loadout, and paused-state fields.
	if (s.getVersion() >= 2) {
		syncRecordArray(s, _runtimeTimers, syncStartupTimerRecord);
		s.syncAsSint32LE(_playerCombatLoadout);
		syncBool(s, _playerControlPaused);
	}
	if (s.isLoading()) {
		_playerCurrentHitPoints = clampPlayerHitPoints(_playerCurrentHitPoints);
		_playerCombatLoadout = clampPlayerCombatLoadout(_playerCombatLoadout);
		// FIXME: Drop this compatibility pass after release; it exists only to repair
		// pre-release saves written before the monster combat field layout stabilized.
		if (s.getVersion() >= 2) {
			for (StartupMonsterRecord &runtimeMonster : _runtimeMonsters) {
				const StartupMonsterRecord *baseMonster = nullptr;
				for (const StartupMonsterRecord &monster : _monsters) {
					if (monster.monsterName.equalsIgnoreCase(runtimeMonster.monsterName)) {
						baseMonster = &monster;
						break;
					}
				}
				if (!baseMonster)
					continue;
				if (!hasLegacyMonsterCombatFieldLayout(*baseMonster, runtimeMonster))
					continue;

				migrateLegacyMonsterCombatFields(*baseMonster, runtimeMonster);
			}
		}
		// FIXME: Remove the pre-v2 save fallback after release; clean Harvester saves will
		// always serialize timer state and the full monster combat payload.
		if (s.getVersion() < 2) {
			_runtimeTimers = _timers;
			for (StartupMonsterRecord &runtimeMonster : _runtimeMonsters) {
				const StartupMonsterRecord *baseMonster = nullptr;
				for (const StartupMonsterRecord &monster : _monsters) {
					if (monster.monsterName.equalsIgnoreCase(runtimeMonster.monsterName)) {
						baseMonster = &monster;
						break;
					}
				}
				if (!baseMonster)
					continue;

				runtimeMonster.initialHitPoints = baseMonster->initialHitPoints;
				runtimeMonster.currentHitPoints = runtimeMonster.initialHitPoints;
				runtimeMonster.damageAmount = baseMonster->damageAmount;
				runtimeMonster.engageDistance = baseMonster->engageDistance;
				runtimeMonster.damageType = baseMonster->damageType;
				runtimeMonster.reservedString38 = baseMonster->reservedString38;
				runtimeMonster.reservedString3c = baseMonster->reservedString3c;
				runtimeMonster.reservedString44 = baseMonster->reservedString44;
				runtimeMonster.reservedString48 = baseMonster->reservedString48;
				runtimeMonster.attackSound1 = baseMonster->attackSound1;
				runtimeMonster.attackSound2 = baseMonster->attackSound2;
				runtimeMonster.attackSound3 = baseMonster->attackSound3;
				runtimeMonster.hitSound1 = baseMonster->hitSound1;
				runtimeMonster.hitSound2 = baseMonster->hitSound2;
				runtimeMonster.hitSound3 = baseMonster->hitSound3;
				runtimeMonster.footstepSoundLeft = baseMonster->footstepSoundLeft;
				runtimeMonster.footstepSoundRight = baseMonster->footstepSoundRight;
				runtimeMonster.deathSound = baseMonster->deathSound;
				runtimeMonster.savedVisible = runtimeMonster.visible;
				runtimeMonster.runtimeSpawned = false;
				runtimeMonster.runtimeState = -1;
				runtimeMonster.screenMinXBound = baseMonster->screenMinXBound;
				runtimeMonster.screenMaxXBound = baseMonster->screenMaxXBound;
				runtimeMonster.attackSoundTriggerFrame = baseMonster->attackSoundTriggerFrame;
				runtimeMonster.hitSoundTriggerFrame = baseMonster->hitSoundTriggerFrame;
				runtimeMonster.footstepSoundTriggerFrame = baseMonster->footstepSoundTriggerFrame;
				runtimeMonster.deathSoundTriggerFrame = baseMonster->deathSoundTriggerFrame;
			}
			_playerCombatLoadout = kDefaultPlayerCombatLoadout;
			_playerControlPaused = false;
		}
		_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
			_runtimeObjects, _playerCombatLoadout, "save load");
		logRuntimeSaveState("loaded");
	}
}

bool Script::materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		StartupRoomSetupState &state, ResourceManager &resources) const {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = !roomName.empty()
		? roomName
		: (entrance ? entrance->roomName : Common::String());
	if (resolvedRoomName.empty())
		return false;

	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved materialized room target entrance='%s' room='%s'",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	return buildRuntimeRoomState(*room, entrance, resources, state);
}

bool Script::executeRoomEnterCommands(const Common::String &roomName,
		StartupInteractionResult &result) {
	result = StartupInteractionResult();

	const StartupRoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	executeCommandChain(room->onEnterCommand, "room entry command", room->roomName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);
	return true;
}

bool Script::executeRoomExitCommands(const Common::String &roomName,
		StartupInteractionResult &result) {
	result = StartupInteractionResult();

	const StartupRoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	executeCommandChain(room->onExitCommand, "room exit command", room->roomName, false,
		&result.musicPath, &result.audioCommands, nullptr, nullptr, &result.cutscenePath,
		nullptr, nullptr, &result.cdChangeDisc, &result.dialogueNpcName, &result.dialogueContinuationTag,
		&result.continuationTag, &result.modalText, &result.lightingCommand,
		&result.requestPlayerGotoXZ, &result.playerGotoX, &result.playerGotoZ,
		&result.mutatedRuntimeState);
	return true;
}

bool Script::resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result) {
	result = StartupInteractionResult();

	if (isPickupObject(object)) {
		if (StartupObjectRecord *runtimeObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName)) {
			if (!runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
				runtimeObject->currentOwnerOrRoom = kInventoryOwnerName;
				runtimeObject->identShown = true;
				result.mutatedRuntimeState = true;
			}
		}
	}
	if (object.actionTag.empty())
		return result.mutatedRuntimeState;

	executeCommandChain(object.actionTag, "interaction command", object.objectName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone || result.requestPlayerGotoXZ ||
		result.mutatedRuntimeState || hasActionableCommandChain(object.actionTag);
}

bool Script::resolveRegionInteraction(const StartupRegionRecord &region, StartupInteractionResult &result) {
	result = StartupInteractionResult();
	if (region.actionTag.empty())
		return false;

	executeCommandChain(region.actionTag, "region command", region.regionName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);
	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone || result.requestPlayerGotoXZ ||
		result.mutatedRuntimeState || hasActionableCommandChain(region.actionTag);
}

bool Script::resolveUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target,
		StartupInteractionResult &result) {
	result = StartupInteractionResult();

	const StartupUseItemRecord *useItem = findUseItemRecord(itemName, target);
	if (!useItem)
		return false;

	executeCommandChain(useItem->actionTag, "useitem command",
		Common::String::format("%s -> %s", itemName.c_str(), target.objectName.c_str()), true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);
	return true;
}

bool Script::executeDebugCommand(const StartupCommandRecord &command,
		StartupInteractionResult &result, bool allowTransitions) {
	result = StartupInteractionResult();
	if (command.opcodeName.empty())
		return false;

	// Reuse the normal action-tag executor by injecting a one-off command node and
	// then removing it once the synthetic debug command has been evaluated.
	Common::String debugTag;
	for (uint suffix = 0;; ++suffix) {
		debugTag = Common::String::format("__DEBUG_COMMAND_%u", suffix);
		if (!findCommandRecord(debugTag))
			break;
	}

	StartupCommandRecord debugCommand = command;
	debugCommand.triggerTag = debugTag;
	_commands.push_back(debugCommand);
	const bool handled = executeActionTag(debugTag, result, allowTransitions);
	_commands.pop_back();
	return handled;
}

bool Script::executeActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions) {
	result = StartupInteractionResult();
	if (tag.empty())
		return false;

	executeCommandChain(tag, "action tag", tag, allowTransitions,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone || result.requestPlayerGotoXZ ||
		result.mutatedRuntimeState || hasActionableCommandChain(tag);
}

bool Script::executeTimerAction(const Common::String &timerName, StartupInteractionResult &result,
		bool allowTransitions) {
	result = StartupInteractionResult();
	if (timerName.empty())
		return false;

	const StartupTimerRecord *timer = findRuntimeTimer(timerName);
	if (!timer) {
		debug(1, "Harvester: unresolved timer record '%s'", timerName.c_str());
		return false;
	}
	if (timer->arg2.empty()) {
		debug(1, "Harvester: timer '%s' has no action tag", timerName.c_str());
		return false;
	}

	executeCommandChain(timer->arg2, "timer command", timer->timerName, allowTransitions,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.cutscenePath, &result.deathFlicPath, &result.requestMainMenu, &result.cdChangeDisc,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.continuationTag,
		&result.modalText, &result.lightingCommand, &result.requestPlayerGotoXZ,
		&result.playerGotoX, &result.playerGotoZ, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone || result.requestPlayerGotoXZ ||
		result.mutatedRuntimeState || hasActionableCommandChain(timer->arg2);
}

bool Script::executeNestedActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions) {
	const bool handled = executeActionTag(tag, result, allowTransitions);
	if (handled)
		result.abortRemainingCommandChain = true;

	return handled;
}

bool Script::isPickupObject(const StartupObjectRecord &object) const {
	return !object.altSpritePath.empty() &&
		!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName);
}

bool Script::isPickupBlockedByAction(const StartupObjectRecord &object,
		StartupInteractionResult *result) const {
	if (result)
		*result = StartupInteractionResult();
	if (!isPickupObject(object) || object.actionTag.empty())
		return false;
	if (!result) {
		StartupInteractionResult ignoredResult;
		return probePickupBlockingCommandChain(object.actionTag, object.objectName, ignoredResult, 0);
	}

	return probePickupBlockingCommandChain(object.actionTag, object.objectName, *result, 0);
}

bool Script::hasObjectInteraction(const StartupObjectRecord &object) const {
	if (isPickupObject(object))
		return true;
	if (object.actionTag.empty())
		return false;

	return hasActionableCommandChain(object.actionTag);
}

bool Script::hasUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target) const {
	return findUseItemRecord(itemName, target) != nullptr;
}

void Script::getVisibleInventoryObjects(Common::Array<StartupObjectRecord> &objects) const {
	objects.clear();
	const char *const statusObjectName = resolveInventoryStatusObjectName(_playerCurrentHitPoints);
	const StartupObjectRecord *statusObject = nullptr;

	for (const StartupObjectRecord &object : _runtimeObjects) {
		if (object.objectName.hasPrefixIgnoreCase("INV_STAT")) {
			if (object.objectName.equalsIgnoreCase(statusObjectName))
				statusObject = &object;
			continue;
		}

		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) && object.visible)
			objects.push_back(object);
	}

	if (!statusObject)
		return;

	StartupObjectRecord activeStatusObject = *statusObject;
	activeStatusObject.currentOwnerOrRoom = kInventoryOwnerName;
	activeStatusObject.visible = true;
	activeStatusObject.runtimeVisible = true;
	activeStatusObject.identShown = true;
	objects.push_back(activeStatusObject);
}

bool Script::isObjectInInventory(const Common::String &objectName) const {
	if (objectName.empty())
		return false;

	for (const StartupObjectRecord &object : _runtimeObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) &&
				object.objectName.equalsIgnoreCase(objectName))
			return true;
	}

	return false;
}

void Script::markObjectIdentShown(const StartupObjectRecord &object) {
	if (StartupObjectRecord *runtimeObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName))
		runtimeObject->identShown = true;
}

const StartupEntranceRecord *Script::findEntranceRecord(const Common::String &entranceName) const {
	if (entranceName.empty())
		return nullptr;

	for (const StartupEntranceRecord &entrance : _entrances) {
		if (entrance.entranceName.equalsIgnoreCase(entranceName))
			return &entrance;
	}

	return nullptr;
}

const StartupMapEntranceRecord *Script::findMapEntranceRecord(const Common::String &entryName) const {
	if (entryName.empty())
		return nullptr;

	for (const StartupMapEntranceRecord &mapEntrance : _mapEntrances) {
		if (mapEntrance.entryName.equalsIgnoreCase(entryName))
			return &mapEntrance;
	}

	return nullptr;
}

const StartupUseItemRecord *Script::findUseItemRecord(const Common::String &itemName,
		const StartupObjectRecord &target) const {
	if (itemName.empty() || target.objectName.empty())
		return nullptr;

	for (const StartupUseItemRecord &useItem : _useItems) {
		if (!useItem.itemName.equalsIgnoreCase(itemName) ||
			!useItem.targetName.equalsIgnoreCase(target.objectName)) {
			continue;
		}

		if (!useItem.ownerOrRoom.empty() &&
			!useItem.ownerOrRoom.equalsIgnoreCase(target.currentOwnerOrRoom)) {
			continue;
		}

		return &useItem;
	}

	return nullptr;
}

const StartupRoomRecord *Script::findRoomRecord(const Common::String &roomName) const {
	if (roomName.empty())
		return nullptr;

	for (const StartupRoomRecord &room : _rooms) {
		if (room.roomName.equalsIgnoreCase(roomName))
			return &room;
	}

	return nullptr;
}

const StartupCommandRecord *Script::findCommandRecord(const Common::String &tag) const {
	if (tag.empty())
		return nullptr;

	for (const StartupCommandRecord &command : _commands) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

const StartupExecListRecord *Script::findExecListRecord(const Common::String &name) const {
	if (name.empty())
		return nullptr;

	for (const StartupExecListRecord &execList : _execLists) {
		if (execList.listName.equalsIgnoreCase(name))
			return &execList;
	}

	return nullptr;
}

const StartupFlagRecord *Script::findRuntimeFlag(const Common::String &flagName) const {
	if (flagName.empty())
		return nullptr;

	for (const StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupFlagRecord *Script::findRuntimeFlag(const Common::String &flagName) {
	if (flagName.empty())
		return nullptr;

	for (StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupObjectRecord *Script::findRuntimeObject(const Common::String &ownerOrRoom,
		const Common::String &objectName) {
	if (objectName.empty())
		return nullptr;

	StartupObjectRecord *fallback = nullptr;
	for (StartupObjectRecord &object : _runtimeObjects) {
		if (!object.objectName.equalsIgnoreCase(objectName))
			continue;

		if (ownerOrRoom.empty())
			return &object;

		if (object.currentOwnerOrRoom.equalsIgnoreCase(ownerOrRoom) ||
			object.initialOwnerOrRoom.equalsIgnoreCase(ownerOrRoom)) {
			return &object;
		}

		if (!fallback)
			fallback = &object;
	}

	return fallback;
}

StartupAnimRecord *Script::findRuntimeAnim(const Common::String &animName) {
	if (animName.empty())
		return nullptr;

	for (StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.animName.equalsIgnoreCase(animName))
			return &anim;
	}

	return nullptr;
}

StartupRegionRecord *Script::findRuntimeRegion(const Common::String &regionName) {
	if (regionName.empty())
		return nullptr;

	for (StartupRegionRecord &region : _runtimeRegions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

StartupNpcRecord *Script::findRuntimeNpc(const Common::String &npcName) {
	if (npcName.empty())
		return nullptr;

	for (StartupNpcRecord &npc : _runtimeNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

StartupMonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) {
	if (monsterName.empty())
		return nullptr;

	for (StartupMonsterRecord &monster : _runtimeMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

StartupTimerRecord *Script::findRuntimeTimer(const Common::String &timerName) {
	if (timerName.empty())
		return nullptr;

	for (StartupTimerRecord &timer : _runtimeTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const StartupNpcRecord *Script::findRuntimeNpc(const Common::String &npcName) const {
	if (npcName.empty())
		return nullptr;

	for (const StartupNpcRecord &npc : _runtimeNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

const StartupMonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) const {
	if (monsterName.empty())
		return nullptr;

	for (const StartupMonsterRecord &monster : _runtimeMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

const StartupTimerRecord *Script::findRuntimeTimer(const Common::String &timerName) const {
	if (timerName.empty())
		return nullptr;

	for (const StartupTimerRecord &timer : _runtimeTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const StartupNpcRecord *Script::findRuntimeNpcRecord(const Common::String &npcName) const {
	return findRuntimeNpc(npcName);
}

bool Script::addRuntimeObjectToInventory(const Common::String &objectName) {
	StartupObjectRecord *runtimeObject = findRuntimeObject(Common::String(), objectName);
	if (!runtimeObject)
		return false;

	const bool changed = !runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) ||
		!runtimeObject->visible || !runtimeObject->runtimeVisible || !runtimeObject->identShown;
	runtimeObject->currentOwnerOrRoom = kInventoryOwnerName;
	runtimeObject->visible = true;
	runtimeObject->runtimeVisible = true;
	runtimeObject->identShown = true;
	return changed;
}

bool Script::syncRuntimeObjectRecord(const StartupObjectRecord &object) {
	if (object.objectName.empty())
		return false;

	StartupObjectRecord *runtimeObject = findRuntimeObject(Common::String(), object.objectName);
	if (!runtimeObject)
		return false;

	const bool changed = runtimeObject->currentX != object.currentX ||
		runtimeObject->currentY != object.currentY ||
		runtimeObject->currentZ != object.currentZ ||
		!runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) ||
		runtimeObject->visible != object.visible ||
		runtimeObject->runtimeVisible != object.runtimeVisible ||
		runtimeObject->identShown != object.identShown;
	*runtimeObject = object;
	return changed;
}

bool Script::setRuntimeObjectVisible(const Common::String &ownerOrRoom,
		const Common::String &objectName, bool visible) {
	if (objectName.empty())
		return false;

	StartupObjectRecord *runtimeObject = findRuntimeObject(ownerOrRoom, objectName);
	if (!runtimeObject)
		return false;

	const bool changed = runtimeObject->visible != visible ||
		runtimeObject->runtimeVisible != visible;
	runtimeObject->visible = visible;
	runtimeObject->runtimeVisible = visible;
	if (visible && runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName))
		runtimeObject->identShown = true;

	return changed;
}

bool Script::setRuntimeNpcState(const Common::String &npcName, bool active, bool visible) {
	if (npcName.empty())
		return false;

	StartupNpcRecord *runtimeNpc = findRuntimeNpc(npcName);
	if (!runtimeNpc)
		return false;

	const bool changed = runtimeNpc->active != active || runtimeNpc->visible != visible;
	runtimeNpc->active = active;
	runtimeNpc->visible = visible;
	return changed;
}

bool Script::setPlayerCurrentHitPoints(int hitPoints) {
	const int clampedHitPoints = clampPlayerHitPoints(hitPoints);
	const bool changed = _playerCurrentHitPoints != clampedHitPoints;
	_playerCurrentHitPoints = clampedHitPoints;
	return changed;
}

bool Script::adjustPlayerCurrentHitPoints(int delta) {
	return setPlayerCurrentHitPoints(_playerCurrentHitPoints + delta);
}

bool Script::setPlayerCombatLoadout(int loadout) {
	const int clampedLoadout = clampPlayerCombatLoadout(loadout);
	const bool changed = _playerCombatLoadout != clampedLoadout;
	_playerCombatLoadout = clampedLoadout;
	return changed;
}

bool Script::setPlayerControlPaused(bool paused) {
	const bool changed = _playerControlPaused != paused;
	_playerControlPaused = paused;
	return changed;
}

bool Script::syncRuntimeAnimState(const Common::String &animName, bool active, bool visible, int currentFrame) {
	StartupAnimRecord *runtimeAnim = findRuntimeAnim(animName);
	if (!runtimeAnim)
		return false;

	const bool changed = runtimeAnim->active != active ||
		runtimeAnim->visible != visible ||
		runtimeAnim->runtimeActive != active ||
		runtimeAnim->runtimeVisible != visible ||
		runtimeAnim->runtimeState != currentFrame;
	runtimeAnim->active = active;
	runtimeAnim->visible = visible;
	runtimeAnim->runtimeActive = active;
	runtimeAnim->runtimeVisible = visible;
	runtimeAnim->runtimeState = currentFrame;
	return changed;
}

bool Script::syncRuntimeMonsterRecord(const StartupMonsterRecord &monster) {
	StartupMonsterRecord *runtimeMonster = findRuntimeMonster(monster.monsterName);
	if (!runtimeMonster)
		return false;

	*runtimeMonster = monster;
	return true;
}

bool Script::syncRuntimeTimerRecord(const StartupTimerRecord &timer) {
	StartupTimerRecord *runtimeTimer = findRuntimeTimer(timer.timerName);
	if (!runtimeTimer)
		return false;

	*runtimeTimer = timer;
	return true;
}

bool Script::setRuntimeRegionEnabled(const Common::String &regionName, bool enabled) {
	StartupRegionRecord *runtimeRegion = findRuntimeRegion(regionName);
	if (!runtimeRegion)
		return false;

	const bool changed = runtimeRegion->startEnabled != enabled;
	runtimeRegion->startEnabled = enabled;
	return changed;
}

bool Script::setRuntimeTimerEnabled(const Common::String &timerName, bool enabled) {
	StartupTimerRecord *runtimeTimer = findRuntimeTimer(timerName);
	if (!runtimeTimer)
		return false;

	const bool changed = runtimeTimer->enabled != enabled ||
		(enabled && runtimeTimer->currentValue != runtimeTimer->initialValue);
	runtimeTimer->enabled = enabled;
	if (enabled)
		runtimeTimer->currentValue = runtimeTimer->initialValue;
	return changed;
}

bool Script::triggerRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType) {
	if (npcName.empty())
		return false;

	StartupNpcRecord *runtimeNpc = findRuntimeNpc(npcName);
	if (!runtimeNpc)
		return false;

	bool monsterChanged = false;
	if (!runtimeNpc->monsterfyTargetName.empty()) {
		StartupMonsterRecord *runtimeMonster = findRuntimeMonster(runtimeNpc->monsterfyTargetName);
		if (runtimeMonster) {
			monsterChanged = !runtimeMonster->active || !runtimeMonster->visible;
			runtimeMonster->active = true;
			runtimeMonster->visible = true;
			runtimeMonster->runtimeSpawned = false;
			runtimeMonster->runtimeState = -1;
			if (runtimeMonster->currentHitPoints <= 0)
				runtimeMonster->currentHitPoints = runtimeMonster->initialHitPoints;
		} else {
			debug(1, "Harvester: unresolved monsterfy target for dialogue npc='%s' target='%s'",
				runtimeNpc->npcName.c_str(), runtimeNpc->monsterfyTargetName.c_str());
		}
	}

	const bool changed = !runtimeNpc->deathOrMonsterfyFlag ||
		(deathDamageType != 0 && runtimeNpc->deathDamageType != deathDamageType);
	runtimeNpc->deathOrMonsterfyFlag = true;
	if (deathDamageType != 0)
		runtimeNpc->deathDamageType = deathDamageType;
	return changed || monsterChanged;
}

bool Script::buildRuntimeRoomState(const StartupRoomRecord &room, const StartupEntranceRecord *entrance,
		ResourceManager &resources, StartupRoomSetupState &state) const {
	state = StartupRoomSetupState();

	const StartupObjectRecord *background = nullptr;
	for (const StartupObjectRecord &candidate : _runtimeObjects) {
		if (isRoomBackgroundObject(candidate, room, resources)) {
			background = &candidate;
			break;
		}
	}
	if (!background) {
		warning("Harvester: no fullscreen background object found for room '%s'", room.roomName.c_str());
		return false;
	}

	state.roomName = room.roomName;
	state.palettePath = room.palettePath;
	state.backgroundPath = background->spritePath;
	state.roomMinZ = room.minZ;
	state.roomMaxZ = room.maxZ;
	state.roomMaxZScreenY = room.maxZScreenY;
	state.roomMinZScreenY = room.minZScreenY;
	state.roomFullScaleZ = room.fullScaleZ;
	state.roomMaxZScalePercent = room.maxZScalePercent;
	state.roomPerspectiveScale = room.perspectiveScale;
	state.roomZVelocityStep = room.zVelocityStep;
	state.musicPath = room.musicPath;
	if (entrance && entrance->roomName.equalsIgnoreCase(room.roomName)) {
		state.entranceName = entrance->entranceName;
		state.hasEntrance = true;
		state.playerSpawnX = entrance->posX;
		state.playerSpawnY = entrance->posY;
		state.playerSpawnZ = entrance->posZ;
		state.playerFacing = entrance->facing;
	}
	for (const StartupObjectRecord &object : _runtimeObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(room.roomName))
			state.roomObjects.push_back(object);
	}
	if (!state.hasEntrance) {
		static const char *const kCloseupExitObjects[] = { "EXIT_BM", "EXIT_HS" };
		for (const char *name : kCloseupExitObjects) {
			for (const StartupObjectRecord &object : _runtimeObjects) {
				if (object.objectName.equalsIgnoreCase(name)) {
					state.roomObjects.push_back(object);
					break;
				}
			}
		}
	}
	for (const StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.roomName.equalsIgnoreCase(room.roomName))
			state.roomAnimations.push_back(anim);
	}
	for (const StartupNpcRecord &npc : _runtimeNpcs) {
		if (!npc.roomName.equalsIgnoreCase(room.roomName) || !npc.visible)
			continue;
		if (npc.deathOrMonsterfyFlag) {
			debugC(1, kDebugRoom,
				"Harvester: suppressed room npc room='%s' npc='%s' death_or_monsterfy=%d damage_type=%d model='%s' monsterfy_target='%s'",
				room.roomName.c_str(), npc.npcName.c_str(), npc.deathOrMonsterfyFlag, npc.deathDamageType,
				npc.modelPath.c_str(), npc.monsterfyTargetName.c_str());
			continue;
		}

		state.roomNpcs.push_back(npc);
	}
	for (const StartupMonsterRecord &monster : _runtimeMonsters) {
		if (!monster.roomName.equalsIgnoreCase(room.roomName))
			continue;
		if (!monster.active && !monster.visible)
			continue;

		state.roomMonsters.push_back(monster);
	}
	for (const StartupTimerRecord &timer : _runtimeTimers) {
		if (timer.arg1.equalsIgnoreCase(room.roomName))
			state.roomTimers.push_back(timer);
	}
	for (const StartupRegionRecord &region : _runtimeRegions) {
		if (region.roomName.equalsIgnoreCase(room.roomName) && region.startEnabled)
			state.roomRegions.push_back(region);
	}

	const StartupFlagRecord *dayFlag = findRuntimeFlag("DAY_FLAG");
	state.paletteBrightness = (room.dimmable && (!dayFlag || !dayFlag->value))
		? kDimmedPaletteBrightness
		: kDefaultPaletteBrightness;

	for (const StartupObjectRecord &object : state.roomObjects) {
		debugC(1, kDebugRoom,
			"Harvester: materialized room object room='%s' object='%s' owner='%s' visible=%d runtimeVisible=%d sprite='%s' alt='%s' pos=(%d,%d,%d) bounds=(%d,%d)-(%d,%d) action='%s'",
			state.roomName.c_str(), object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
			object.visible, object.runtimeVisible, object.spritePath.c_str(), object.altSpritePath.c_str(),
			object.currentX, object.currentY, object.currentZ,
			object.currentX, object.currentY, object.boundsX2, object.boundsY2, object.actionTag.c_str());
	}
	for (const StartupNpcRecord &npc : state.roomNpcs) {
		debugC(1, kDebugRoom,
			"Harvester: materialized room npc room='%s' npc='%s' visible=%d active=%d pos=(%d,%d,%d) frame_delay=%d model='%s' on_death='%s' audio='%s'",
			state.roomName.c_str(), npc.npcName.c_str(), npc.visible, npc.active,
			npc.posX, npc.posY, npc.posZ, npc.frameDelay,
			npc.modelPath.c_str(), npc.onDeathActionTag.c_str(), npc.audioPath.c_str());
	}
	for (const StartupMonsterRecord &monster : state.roomMonsters) {
		debugC(1, kDebugRoom,
			"Harvester: materialized room monster room='%s' monster='%s' visible=%d active=%d spawned=%d runtimeState=%d pos=(%d,%d,%d) facing=%d hp=%d/%d damage=%d engage=%d damage_type='%s' model='%s' on_death='%s'",
			state.roomName.c_str(), monster.monsterName.c_str(), monster.visible, monster.active,
			monster.runtimeSpawned, monster.runtimeState,
			monster.posX, monster.posY, monster.posZ, monster.facing,
			monster.currentHitPoints, monster.initialHitPoints,
			monster.damageAmount, monster.engageDistance, Player::describeCombatDamageType(monster.damageType),
			monster.modelPath.c_str(), monster.onDeathActionTag.c_str());
	}
	for (const StartupTimerRecord &timer : state.roomTimers) {
		debugC(1, kDebugRoom,
			"Harvester: materialized room timer room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
			state.roomName.c_str(), timer.timerName.c_str(), timer.currentValue, timer.initialValue,
			timer.enabled, timer.looping, timer.global);
	}

	debugC(1, kDebugRoom,
		"Harvester: materializeRoomState room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		state.roomName.c_str(), state.entranceName.c_str(), state.playerSpawnX, state.playerSpawnY,
		state.playerSpawnZ, state.playerFacing, state.palettePath.c_str(), state.backgroundPath.c_str(),
		state.musicPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.roomAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)state.roomMonsters.size(),
		(uint)state.roomTimers.size(), (uint)state.roomRegions.size());

	return true;
}

void Script::executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, bool allowTransitions, Common::String *musicPath,
		Common::Array<StartupAudioCommand> *audioCommands, Common::String *nextRoomName,
		StartupRoomTransitionKind *roomTransition,
		Common::String *cutscenePath, Common::String *deathFlicPath, bool *requestMainMenu,
		int *cdChangeDisc,
		Common::String *dialogueNpcName, Common::String *dialogueContinuationTag,
		Common::String *continuationTag, StartupResolvedText *modalText,
		StartupLightingCommand *lightingCommand, bool *requestPlayerGotoXZ,
		int *playerGotoX, int *playerGotoZ,
		bool *mutatedRuntimeState) {
	auto isTruthy = [](const Common::String &value) {
		return value.equalsIgnoreCase("T") || value.equalsIgnoreCase("ON") || value.equalsIgnoreCase("TRUE");
	};
	auto noteMutation = [&](bool changed) {
		if (changed && mutatedRuntimeState)
			*mutatedRuntimeState = true;
	};
	auto hasDeferredInteractionOutputs = [&]() {
		return (nextRoomName && !nextRoomName->empty()) ||
			(cutscenePath && !cutscenePath->empty()) ||
			(deathFlicPath && !deathFlicPath->empty()) ||
			(requestMainMenu && *requestMainMenu) ||
			(dialogueNpcName && !dialogueNpcName->empty()) ||
			(continuationTag && !continuationTag->empty()) ||
			(modalText && !modalText->value.empty()) ||
			(lightingCommand && *lightingCommand != kStartupLightingCommandNone) ||
			(requestPlayerGotoXZ && *requestPlayerGotoXZ);
	};

	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved %s tag '%s' for '%s'",
				contextLabel, currentTag.c_str(), contextName.c_str());
			break;
		}

		debugC(1, kDebugScene,
			"Harvester: %s '%s' step=%u tag='%s' opcode='%s' args=['%s','%s','%s','%s']",
			contextLabel, contextName.c_str(), step, currentTag.c_str(), command->opcodeName.c_str(),
			command->arg1.c_str(), command->arg2.c_str(), command->arg3.c_str(), command->arg4.c_str());

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			const bool flagValue = flag && flag->value;
			debugC(1, kDebugScene, "Harvester: %s '%s' flag '%s' -> %d",
				contextLabel, contextName.c_str(), command->arg1.c_str(), flagValue);
			currentTag = flagValue ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_PERC")) {
			const int threshold = CLIP<int>(parseAsciiIntOrZero(command->arg1), 0, 100);
			const int roll = g_engine ? (int)g_engine->getRandomNumber(99) : 0;
			currentTag = roll < threshold ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG")) {
			const bool flagValue = isTruthy(command->arg2);
			StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			bool changed = false;
			bool oldValue = false;
			const bool existed = flag != nullptr;
			if (flag) {
				oldValue = flag->value;
				changed = flag->value != flagValue;
				flag->value = flagValue;
			} else {
				StartupFlagRecord newFlag;
				newFlag.name = command->arg1;
				newFlag.value = flagValue;
				_runtimeFlags.push_back(newFlag);
				changed = true;
			}
			debugC(1, kDebugGeneral,
				"Harvester: %s '%s' SET_FLAG '%s' %d -> %d existed=%d changed=%d",
				contextLabel, contextName.c_str(), command->arg1.c_str(), oldValue, flagValue,
				existed, changed);
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC")) {
			if (musicPath)
				*musicPath = command->arg1;
			currentTag = command->arg4;
			continue;
		}

		if (audioCommands) {
			if (appendStartupAudioCommand(*command, *audioCommands)) {
				currentTag = command->arg4;
				continue;
			}
		} else {
			Common::Array<StartupAudioCommand> ignoredAudioCommands;
			if (appendStartupAudioCommand(*command, ignoredAudioCommands)) {
				currentTag = command->arg4;
				continue;
			}
		}

		if (command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE")) {
			StartupObjectRecord *runtimeObject = findRuntimeObject(command->arg1, command->arg2);
			if (!runtimeObject) {
				debug(1, "Harvester: unresolved object for %s '%s' owner='%s' object='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str(), command->arg2.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool visible = command->opcodeName.equalsIgnoreCase("ADD");
			const bool changed = runtimeObject->visible != visible ||
				runtimeObject->runtimeVisible != visible;
			runtimeObject->visible = visible;
			runtimeObject->runtimeVisible = visible;
			if (visible && runtimeObject->currentOwnerOrRoom.equalsIgnoreCase("INVENTORY"))
				runtimeObject->identShown = true;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("ADD2INV")) {
			noteMutation(addRuntimeObjectToInventory(command->arg1));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_ANIM")) {
			StartupAnimRecord *runtimeAnim = findRuntimeAnim(command->arg1);
			if (!runtimeAnim) {
				debug(1, "Harvester: unresolved anim for %s '%s' anim='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool wasVisible = runtimeAnim->visible;
			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			int runtimeState = runtimeAnim->runtimeState;
			if (!wasVisible && visible)
				runtimeState = 0;
			const bool changed = runtimeAnim->active != active ||
				runtimeAnim->visible != visible ||
				runtimeAnim->runtimeActive != active ||
				runtimeAnim->runtimeVisible != visible ||
				runtimeAnim->runtimeState != runtimeState;
			runtimeAnim->active = active;
			runtimeAnim->visible = visible;
			runtimeAnim->runtimeActive = active;
			runtimeAnim->runtimeVisible = visible;
			runtimeAnim->runtimeState = runtimeState;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_REGION")) {
			StartupRegionRecord *runtimeRegion = findRuntimeRegion(command->arg1);
			if (!runtimeRegion) {
				debug(1, "Harvester: unresolved region for %s '%s' region='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool enabled = !command->arg2.equalsIgnoreCase("F");
			const bool changed = runtimeRegion->startEnabled != enabled;
			runtimeRegion->startEnabled = enabled;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_NPC")) {
			StartupNpcRecord *runtimeNpc = findRuntimeNpc(command->arg1);
			if (!runtimeNpc) {
				debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			const bool changed = runtimeNpc->active != active || runtimeNpc->visible != visible;
			runtimeNpc->active = active;
			runtimeNpc->visible = visible;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_MONSTER")) {
			StartupMonsterRecord *runtimeMonster = findRuntimeMonster(command->arg1);
			if (!runtimeMonster) {
				debug(1, "Harvester: unresolved monster for %s '%s' monster='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			const bool changed = runtimeMonster->active != active || runtimeMonster->visible != visible;
			runtimeMonster->active = active;
			runtimeMonster->visible = active ? true : visible;
			if (active) {
				runtimeMonster->runtimeSpawned = false;
				runtimeMonster->runtimeState = -1;
			}
			if (!runtimeMonster->visible) {
				runtimeMonster->runtimeSpawned = false;
				runtimeMonster->runtimeState = -1;
			}
			if (active && runtimeMonster->currentHitPoints <= 0)
				runtimeMonster->currentHitPoints = runtimeMonster->initialHitPoints;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_TIMER")) {
			noteMutation(setRuntimeTimerEnabled(command->arg1, command->arg2.equalsIgnoreCase("ON")));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("KILL_TIMER")) {
			noteMutation(setRuntimeTimerEnabled(command->arg1, false));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("EXEC_LIST")) {
			const StartupExecListRecord *execList = findExecListRecord(command->arg1);
			if (!execList) {
				debug(1, "Harvester: unresolved exec list for %s '%s' list='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			for (const Common::String &entry : execList->entries) {
				executeCommandChain(entry, "exec list entry", execList->listName, allowTransitions,
					musicPath, audioCommands, nextRoomName, roomTransition, cutscenePath,
					deathFlicPath, requestMainMenu, cdChangeDisc, dialogueNpcName, dialogueContinuationTag,
					continuationTag, modalText, lightingCommand, requestPlayerGotoXZ,
					playerGotoX, playerGotoZ, mutatedRuntimeState);
				if (hasDeferredInteractionOutputs())
					return;
			}
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("START_DIALOG")) {
			if (!dialogueNpcName || !dialogueContinuationTag) {
				debug(1, "Harvester: unsupported startup command '%s' for %s '%s' without dialogue context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				return;
			}
			if (command->arg1.empty()) {
				currentTag = command->arg4;
				continue;
			}

			*dialogueNpcName = command->arg1;
			*dialogueContinuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("GOFLIC")) {
			if (!cutscenePath) {
				debug(1, "Harvester: deferred startup command '%s' for %s '%s' has no cutscene context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				currentTag = command->arg4;
				continue;
			}

			*cutscenePath = command->arg1;
			if (continuationTag)
				*continuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("GODEATHFLIC")) {
			if (!deathFlicPath || !requestMainMenu) {
				debug(1, "Harvester: unsupported startup command '%s' for %s '%s' without menu-exit context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				return;
			}

			if (allowTransitions) {
				*deathFlicPath = command->arg1;
				*requestMainMenu = true;
			} else {
				debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
			}
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
				command->opcodeName.equalsIgnoreCase("MONSTERFY")) {
			StartupNpcRecord *runtimeNpc = findRuntimeNpc(command->arg1);
			if (!runtimeNpc) {
				debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const int deathDamageType = parseNpcDeathDamageType(command->arg2);
			bool monsterChanged = false;
			if (command->opcodeName.equalsIgnoreCase("MONSTERFY") && !runtimeNpc->monsterfyTargetName.empty()) {
				StartupMonsterRecord *runtimeMonster = findRuntimeMonster(runtimeNpc->monsterfyTargetName);
				if (runtimeMonster) {
					monsterChanged = !runtimeMonster->active || !runtimeMonster->visible;
					runtimeMonster->active = true;
					runtimeMonster->visible = true;
					runtimeMonster->runtimeSpawned = false;
					runtimeMonster->runtimeState = -1;
					if (runtimeMonster->currentHitPoints <= 0)
						runtimeMonster->currentHitPoints = runtimeMonster->initialHitPoints;
				} else {
					debug(1, "Harvester: unresolved monsterfy target for %s '%s' npc='%s' target='%s'",
						contextLabel, contextName.c_str(), command->arg1.c_str(),
						runtimeNpc->monsterfyTargetName.c_str());
				}
			}

			const bool changed = !runtimeNpc->deathOrMonsterfyFlag ||
				(deathDamageType != 0 && runtimeNpc->deathDamageType != deathDamageType);
			runtimeNpc->deathOrMonsterfyFlag = true;
			if (deathDamageType != 0)
				runtimeNpc->deathDamageType = deathDamageType;
			noteMutation(changed || monsterChanged);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SHOW_TEXT")) {
			if (!modalText) {
				debug(1, "Harvester: deferred startup command '%s' for %s '%s' has no modal-text context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				currentTag = command->arg4;
				continue;
			}

			if (!resolveTextRecord(command->arg1, *modalText)) {
				debug(1, "Harvester: unresolved SHOW_TEXT record for %s '%s' key='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}
			if (continuationTag)
				*continuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP")) {
			noteMutation(setPlayerCurrentHitPoints(
				_playerCurrentHitPoints + parseAsciiIntOrZero(command->arg1)));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("KILL_PC")) {
			noteMutation(setPlayerCurrentHitPoints(0));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("PAUSE_PC")) {
			noteMutation(setPlayerControlPaused(true));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("RESUME_PC")) {
			noteMutation(setPlayerControlPaused(false));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("PC_GOTO_XZ")) {
			if (!requestPlayerGotoXZ || !playerGotoX || !playerGotoZ) {
				debug(1, "Harvester: deferred startup command '%s' for %s '%s' has no player-move context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				currentTag = command->arg4;
				continue;
			}

			*requestPlayerGotoXZ = true;
			*playerGotoX = parseAsciiIntOrZero(command->arg1);
			*playerGotoZ = parseAsciiIntOrZero(command->arg2);
			if (continuationTag)
				*continuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("CHANGE_LIGHTING")) {
			if (!lightingCommand) {
				debug(1, "Harvester: deferred startup command '%s' for %s '%s' has no lighting context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				currentTag = command->arg4;
				continue;
			}

			if (command->arg1.equalsIgnoreCase("DIM"))
				*lightingCommand = kStartupLightingCommandDim;
			else if (command->arg1.equalsIgnoreCase("NORMAL"))
				*lightingCommand = kStartupLightingCommandNormal;
			else if (command->arg1.equalsIgnoreCase("NONE"))
				*lightingCommand = kStartupLightingCommandBlack;
			else if (command->arg1.equalsIgnoreCase("FADE_IN"))
				*lightingCommand = kStartupLightingCommandFadeIn;
			else
				debug(1, "Harvester: unsupported CHANGE_LIGHTING mode '%s' for %s '%s', preserving control flow",
					command->arg1.c_str(), contextLabel, contextName.c_str());
			if (continuationTag)
				*continuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("CHANGE_CD")) {
			if (!cdChangeDisc) {
				debug(1, "Harvester: deferred startup command '%s' for %s '%s' has no CD-change context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				currentTag = command->arg4;
				continue;
			}

			const int requestedDisc = parseAsciiIntOrZero(command->arg1);
			if (requestedDisc > 0)
				*cdChangeDisc = requestedDisc;
			else
				debug(1, "Harvester: invalid CHANGE_CD target '%s' for %s '%s'",
					command->arg1.c_str(), contextLabel, contextName.c_str());
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			if (allowTransitions && nextRoomName) {
				*nextRoomName = command->arg1;
				if (roomTransition) {
					*roomTransition = command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")
						? kStartupRoomTransitionChangeRoom
						: kStartupRoomTransitionCloseup;
				}
			} else {
				debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
			}
			currentTag = command->arg4;
			continue;
		}

		debug(1, "Harvester: unsupported startup command '%s' for %s '%s', continuing",
			command->opcodeName.c_str(), contextLabel, contextName.c_str());
		currentTag = command->arg4;
	}
}

bool Script::probePickupBlockingCommandChain(const Common::String &initialTag,
		const Common::String &contextName, StartupInteractionResult &result, uint recursionDepth) const {
	if (initialTag.empty() || recursionDepth > 8)
		return false;

	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved pickup preflight tag '%s' for '%s'",
				currentTag.c_str(), contextName.c_str());
			return false;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			currentTag = (flag && flag->value) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_PERC"))
			return false;

		if (command->opcodeName.equalsIgnoreCase("SHOW_TEXT")) {
			if (!resolveTextRecord(command->arg1, result.modalText))
				return false;
			result.continuationTag = command->arg4;
			return !result.modalText.value.empty();
		}

		if (command->opcodeName.equalsIgnoreCase("EXEC_LIST")) {
			const StartupExecListRecord *execList = findExecListRecord(command->arg1);
			if (!execList)
				return false;

			for (const Common::String &entry : execList->entries) {
				if (probePickupBlockingCommandChain(entry, contextName, result, recursionDepth + 1))
					return true;
			}
			return false;
		}

		Common::Array<StartupAudioCommand> audioCommands;
		if (appendStartupAudioCommand(*command, audioCommands))
			return false;

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG") ||
			command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC") ||
			command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE") ||
			command->opcodeName.equalsIgnoreCase("ADD2INV") ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("SET_REGION") ||
			command->opcodeName.equalsIgnoreCase("SET_NPC") ||
			command->opcodeName.equalsIgnoreCase("SET_MONSTER") ||
			command->opcodeName.equalsIgnoreCase("SET_TIMER") ||
			command->opcodeName.equalsIgnoreCase("KILL_TIMER") ||
			command->opcodeName.equalsIgnoreCase("START_DIALOG") ||
			command->opcodeName.equalsIgnoreCase("GOFLIC") ||
			command->opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
			command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command->opcodeName.equalsIgnoreCase("MONSTERFY") ||
			command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP") ||
			command->opcodeName.equalsIgnoreCase("KILL_PC") ||
			command->opcodeName.equalsIgnoreCase("PAUSE_PC") ||
			command->opcodeName.equalsIgnoreCase("RESUME_PC") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_CD") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_XZ") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_LIGHTING") ||
			command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			return false;
		}

		currentTag = command->arg4;
	}

	return false;
}

bool Script::hasActionableCommandChain(const Common::String &initialTag) const {
	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved interaction probe tag '%s'", currentTag.c_str());
			break;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			currentTag = (flag && flag->value) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_PERC")) {
			currentTag = command->arg2.empty() ? command->arg3 : command->arg2;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG") ||
			command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC") ||
			command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE") ||
			command->opcodeName.equalsIgnoreCase("ADD2INV") ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("SET_REGION") ||
			command->opcodeName.equalsIgnoreCase("SET_NPC") ||
			command->opcodeName.equalsIgnoreCase("SET_MONSTER") ||
			command->opcodeName.equalsIgnoreCase("SET_TIMER") ||
			command->opcodeName.equalsIgnoreCase("KILL_TIMER") ||
			command->opcodeName.equalsIgnoreCase("EXEC_LIST") ||
			command->opcodeName.equalsIgnoreCase("START_DIALOG") ||
			command->opcodeName.equalsIgnoreCase("GOFLIC") ||
			command->opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
			command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command->opcodeName.equalsIgnoreCase("MONSTERFY") ||
			command->opcodeName.equalsIgnoreCase("SHOW_TEXT") ||
			command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP") ||
			command->opcodeName.equalsIgnoreCase("KILL_PC") ||
			command->opcodeName.equalsIgnoreCase("PAUSE_PC") ||
			command->opcodeName.equalsIgnoreCase("RESUME_PC") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_CD") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_XZ") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_LIGHTING") ||
			command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			return true;
		}

		Common::Array<StartupAudioCommand> audioCommands;
		if (appendStartupAudioCommand(*command, audioCommands))
			return true;

		currentTag = command->arg4;
	}

	return false;
}

const StartupTextRecord *Script::findTextRecord(const Common::String &key) const {
	if (key.empty())
		return nullptr;

	for (const StartupTextRecord &textRecord : _texts) {
		if (textRecord.key.equalsIgnoreCase(key))
			return &textRecord;
	}

	return nullptr;
}

bool Script::resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const {
	text = StartupResolvedText();

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

bool Script::resolveTextRecord(const Common::String &key, StartupResolvedText &text) const {
	text = StartupResolvedText();

	const StartupTextRecord *textRecord = findTextRecord(key);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

Common::String Script::resolveInventoryTooltipText(const StartupObjectRecord &object) const {
	const StartupTextRecord *textRecord = findTextRecord(object.inventoryTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return Common::String();
}

Common::String Script::resolveObjectLabel(const StartupObjectRecord &object) const {
	if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		const Common::String inventoryLabel = resolveInventoryTooltipText(object);
		if (!inventoryLabel.empty())
			return inventoryLabel;
	}

	if (!object.interactionLabel.empty() && !object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return normalizeInteractionLabel(object.interactionLabel);
	if (object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return Common::String();

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = normalizeInteractionLabel(object.objectName);
	if (!label.empty() && !label.equalsIgnoreCase("NULL ID"))
		return label;

	return Common::String();
}

Common::String Script::resolveTextValue(const Common::String &key) const {
	const StartupTextRecord *textRecord = findTextRecord(key);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return normalizeInteractionLabel(key);
}

const StartupHeadRecord *Script::findHeadRecord(const Common::String &headId) const {
	if (headId.empty())
		return nullptr;

	for (const StartupHeadRecord &head : _heads) {
		if (head.headId.equalsIgnoreCase(headId))
			return &head;
	}

	return nullptr;
}

bool Script::getFlagValue(const Common::String &flagName) const {
	const StartupFlagRecord *flag = findRuntimeFlag(flagName);
	return flag && flag->value;
}

bool Script::setRuntimeFlagValue(const Common::String &flagName, bool value) {
	if (flagName.empty())
		return false;

	StartupFlagRecord *flag = findRuntimeFlag(flagName);
	if (flag) {
		const bool changed = flag->value != value;
		const bool oldValue = flag->value;
		flag->value = value;
		debugC(1, kDebugGeneral,
			"Harvester: direct runtime flag '%s' %d -> %d existed=1 changed=%d",
			flagName.c_str(), oldValue, value, changed);
		return changed;
	}

	StartupFlagRecord newFlag;
	newFlag.name = flagName;
	newFlag.value = value;
	_runtimeFlags.push_back(newFlag);
	debugC(1, kDebugGeneral,
		"Harvester: direct runtime flag '%s' %d -> %d existed=0 changed=1",
		flagName.c_str(), 0, value);
	return true;
}

bool Script::resetRuntimeObjectToInitialState(const Common::String &objectName) {
	if (objectName.empty())
		return false;

	StartupObjectRecord *runtimeObject = findRuntimeObject(Common::String(), objectName);
	if (!runtimeObject)
		return false;

	const StartupObjectRecord *baseObject = nullptr;
	for (const StartupObjectRecord &object : _objects) {
		if (object.objectName.equalsIgnoreCase(objectName)) {
			baseObject = &object;
			break;
		}
	}
	if (!baseObject)
		return false;

	const bool changed = runtimeObject->currentX != runtimeObject->initialX ||
		runtimeObject->currentY != runtimeObject->initialY ||
		runtimeObject->currentZ != runtimeObject->initialZ ||
		!runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(runtimeObject->initialOwnerOrRoom) ||
		runtimeObject->visible != baseObject->visible ||
		runtimeObject->runtimeVisible != baseObject->visible ||
		runtimeObject->identShown != baseObject->identShown;

	runtimeObject->currentX = runtimeObject->initialX;
	runtimeObject->currentY = runtimeObject->initialY;
	runtimeObject->currentZ = runtimeObject->initialZ;
	runtimeObject->currentOwnerOrRoom = runtimeObject->initialOwnerOrRoom;
	runtimeObject->visible = baseObject->visible;
	runtimeObject->runtimeVisible = baseObject->visible;
	runtimeObject->identShown = baseObject->identShown;
	return changed;
}

bool Script::isNamedNpcDeathTypeClear(const Common::String &npcName) const {
	const StartupNpcRecord *npc = findRuntimeNpc(npcName);
	return npc && npc->deathDamageType == 0;
}

int Script::getCurrentStoryDayIndex() const {
	if (getFlagValue("DAY_1"))
		return 1;
	if (getFlagValue("DAY_2") || getFlagValue("NIGHT_2"))
		return 2;
	if (getFlagValue("DAY_3") || getFlagValue("NIGHT_3"))
		return 3;
	if (getFlagValue("DAY_4") || getFlagValue("NIGHT_4"))
		return 4;
	if (getFlagValue("DAY_5") || getFlagValue("NIGHT_5"))
		return 5;
	if (getFlagValue("DAY_6"))
		return 6;

	return 0;
}

} // End of namespace Harvester
