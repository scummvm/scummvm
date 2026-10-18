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

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/parse_utils.h"
#include "harvester/player.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kDefaultVoicePath = "SOUND/VOICE/";
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

static void syncStartupFlagRecord(Common::Serializer &s, FlagRecord &record) {
	s.syncString(record.name);
	syncBool(s, record.value);
}

static void syncStartupObjectRecord(Common::Serializer &s, ObjectRecord &record) {
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

static void syncStartupAnimRecord(Common::Serializer &s, AnimRecord &record) {
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

static void syncStartupNpcRecord(Common::Serializer &s, NpcRecord &record) {
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
	s.syncAsSint32LE(record.runtimeState);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.savedVisible);
	s.syncAsSint32LE(record.deathDamageType);
	s.syncString(record.audioPath);
	s.syncString(record.entityInitArg);
}

static void syncStartupMonsterRecord(Common::Serializer &s, MonsterRecord &record) {
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
	s.syncAsSint32LE(record.runtimeState);
}

static bool matchesMonsterIdentity(const MonsterRecord &candidate, const MonsterRecord &monster) {
	if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
		return false;

	if (candidate.recordIndex >= 0 && monster.recordIndex >= 0)
		return candidate.recordIndex == monster.recordIndex;

	if (!monster.roomName.empty())
		return candidate.roomName.equalsIgnoreCase(monster.roomName);

	return true;
}

static void restoreMonsterRecordIndices(const Common::Array<MonsterRecord> &baseMonsters,
		Common::Array<MonsterRecord> &runtimeMonsters) {
	for (uint i = 0; i < runtimeMonsters.size(); ++i) {
		if (i < baseMonsters.size() &&
				baseMonsters[i].monsterName.equalsIgnoreCase(runtimeMonsters[i].monsterName) &&
				baseMonsters[i].roomName.equalsIgnoreCase(runtimeMonsters[i].roomName)) {
			runtimeMonsters[i].recordIndex = baseMonsters[i].recordIndex;
			continue;
		}

		runtimeMonsters[i].recordIndex = -1;
		for (const MonsterRecord &baseMonster : baseMonsters) {
			if (!baseMonster.monsterName.equalsIgnoreCase(runtimeMonsters[i].monsterName) ||
					!baseMonster.roomName.equalsIgnoreCase(runtimeMonsters[i].roomName)) {
				continue;
			}

			runtimeMonsters[i].recordIndex = baseMonster.recordIndex;
			break;
		}
	}
}

static void syncStartupTimerRecord(Common::Serializer &s, TimerRecord &record) {
	s.syncAsSint32LE(record.initialValue);
	s.syncAsSint32LE(record.currentValue);
	s.syncString(record.timerName);
	s.syncString(record.arg1);
	s.syncString(record.arg2);
	syncBool(s, record.enabled);
	syncBool(s, record.looping);
	syncBool(s, record.global);
}

static void syncStartupRegionRecord(Common::Serializer &s, RegionRecord &record) {
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

static int resolveCombatResourceDisplayMax(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 14:
		return 16;
	case 4:
		return 8;
	case 5:
		return 6;
	default:
		return 0;
	}
}

static int clampCombatResourceCount(int loadout, int count) {
	const int maxCount = resolveCombatResourceDisplayMax(loadout);
	if (maxCount <= 0)
		return 0;

	return CLIP<int>(count, 0, maxCount);
}

int Script::resolveCombatResourcePickupMax(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 14:
		return 15;
	case 4:
		return 7;
	case 5:
		return 5;
	default:
		return 0;
	}
}

static const char *resolveCombatLoadoutInventoryObjectName(int loadout) {
	for (const InventoryCombatLoadoutEntry &entry : kInventoryCombatLoadoutMap) {
		if (entry.loadoutId == loadout)
			return entry.objectName;
	}

	return nullptr;
}

static int validatePlayerCombatLoadoutAgainstInventory(const Common::Array<ObjectRecord> &currentObjects,
		int loadout, const char *contextLabel) {
	const int clampedLoadout = clampPlayerCombatLoadout(loadout);
	if (clampedLoadout == kDefaultPlayerCombatLoadout)
		return clampedLoadout;

	const char *requiredObjectName = resolveCombatLoadoutInventoryObjectName(clampedLoadout);
	if (!requiredObjectName)
		return clampedLoadout;

	for (const ObjectRecord &object : currentObjects) {
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
	_currentFlags.clear();
	_currentObjects.clear();
	_currentAnimations.clear();
	_currentNpcs.clear();
	_currentMonsters.clear();
	_currentTimers.clear();
	_currentRegions.clear();
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

	const Common::Array<FlagRecord> currentFlags = _currentFlags;
	const Common::Array<ObjectRecord> currentObjects = _currentObjects;
	const Common::Array<AnimRecord> currentAnimations = _currentAnimations;
	const Common::Array<RegionRecord> currentRegions = _currentRegions;
	const Common::Array<NpcRecord> currentNpcs = _currentNpcs;
	const Common::Array<MonsterRecord> currentMonsters = _currentMonsters;
	const Common::Array<TimerRecord> currentTimers = _currentTimers;
	const int nailgunAmmoCount = _nailgunAmmoCount;
	const int shotgunShellCount = _shotgunShellCount;
	const int nineGunBulletCount = _nineGunBulletCount;
	const int thirtyEightGunBulletCount = _thirtyEightGunBulletCount;
	const int chainsawFuelCount = _chainsawFuelCount;
	const int playerCurrentHitPoints = _playerCurrentHitPoints;
	const int playerCombatLoadout = _playerCombatLoadout;
	const bool playerControlPaused = _playerControlPaused;

	_data = Common::move(reloadedData);
	decode();
	parseTownRecords(resources);
	resetRuntimeState();

	for (const FlagRecord &flag : currentFlags) {
		FlagRecord *currentFlag = findRuntimeFlag(flag.name);
		if (currentFlag) {
			currentFlag->value = flag.value;
			continue;
		}

		_currentFlags.push_back(flag);
	}

	for (const ObjectRecord &object : currentObjects) {
		ObjectRecord *currentObject = findRuntimeObject(object.initialOwnerOrRoom, object.objectName);
		if (!currentObject)
			currentObject = findRuntimeObject(Common::String(), object.objectName);
		if (!currentObject) {
			_currentObjects.push_back(object);
			continue;
		}

		currentObject->currentX = object.currentX;
		currentObject->currentY = object.currentY;
		currentObject->currentZ = object.currentZ;
		currentObject->currentOwnerOrRoom = object.currentOwnerOrRoom;
		currentObject->visible = object.visible;
		currentObject->runtimeVisible = object.runtimeVisible;
		currentObject->identShown = object.identShown;
	}

	for (const AnimRecord &anim : currentAnimations) {
		AnimRecord *currentAnimation = findRuntimeAnim(anim.animName);
		if (!currentAnimation) {
			_currentAnimations.push_back(anim);
			continue;
		}

		currentAnimation->active = anim.active;
		currentAnimation->visible = anim.visible;
		currentAnimation->runtimeActive = anim.runtimeActive;
		currentAnimation->runtimeVisible = anim.runtimeVisible;
		currentAnimation->runtimeState = anim.runtimeState;
	}

	for (const RegionRecord &region : currentRegions) {
		RegionRecord *currentRegion = findRuntimeRegion(region.regionName);
		if (!currentRegion) {
			_currentRegions.push_back(region);
			continue;
		}

		currentRegion->startEnabled = region.startEnabled;
	}

	for (const NpcRecord &npc : currentNpcs) {
		NpcRecord *currentNpc = findRuntimeNpc(npc.npcName);
		if (!currentNpc) {
			_currentNpcs.push_back(npc);
			continue;
		}

		currentNpc->active = npc.active;
		currentNpc->visible = npc.visible;
		currentNpc->savedVisible = npc.savedVisible;
		currentNpc->runtimeSpawned = npc.runtimeSpawned;
		currentNpc->runtimeState = npc.runtimeState;
		currentNpc->deathOrMonsterfyFlag = npc.deathOrMonsterfyFlag;
		currentNpc->deathDamageType = npc.deathDamageType;
	}

	for (const MonsterRecord &monster : currentMonsters) {
		MonsterRecord *currentMonster = findRuntimeMonster(monster);
		if (!currentMonster) {
			_currentMonsters.push_back(monster);
			continue;
		}

		currentMonster->posX = monster.posX;
		currentMonster->posY = monster.posY;
		currentMonster->posZ = monster.posZ;
		currentMonster->facing = monster.facing;
		currentMonster->active = monster.active;
		currentMonster->visible = monster.visible;
		currentMonster->savedVisible = monster.savedVisible;
		currentMonster->runtimeSpawned = monster.runtimeSpawned;
		currentMonster->runtimeState = monster.runtimeState;
		currentMonster->currentHitPoints = monster.currentHitPoints;
		currentMonster->screenMinXBound = monster.screenMinXBound;
		currentMonster->screenMaxXBound = monster.screenMaxXBound;
	}

	for (const TimerRecord &timer : currentTimers) {
		TimerRecord *currentTimer = findRuntimeTimer(timer.timerName);
		if (!currentTimer) {
			_currentTimers.push_back(timer);
			continue;
		}

		currentTimer->initialValue = timer.initialValue;
		currentTimer->currentValue = timer.currentValue;
		currentTimer->enabled = timer.enabled;
		currentTimer->looping = timer.looping;
		currentTimer->global = timer.global;
	}

	_playerCurrentHitPoints = clampPlayerHitPoints(playerCurrentHitPoints);
	_nailgunAmmoCount = clampCombatResourceCount(2, nailgunAmmoCount);
	_shotgunShellCount = clampCombatResourceCount(3, shotgunShellCount);
	_nineGunBulletCount = clampCombatResourceCount(4, nineGunBulletCount);
	_thirtyEightGunBulletCount = clampCombatResourceCount(5, thirtyEightGunBulletCount);
	_chainsawFuelCount = clampCombatResourceCount(14, chainsawFuelCount);
	_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
		_currentObjects, playerCombatLoadout, "town reload");
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
	const Common::FSNode gamePathNode(ConfMan.getPath("path"));
	const Common::FSNode configNode = gamePathNode.getChild(kStartupConfigPath);
	if (!file.open(configNode)) {
		warning("Harvester: unable to save startup config '%s'",
			configNode.getPath().toString(Common::Path::kNativeSeparator).c_str());
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

void Script::resetRuntimeState() {
	_currentFlags = _flags;
	_currentObjects = _objects;
	_currentAnimations = _animations;
	_currentRegions = _regions;
	_currentNpcs = _npcs;
	_currentMonsters = _monsters;
	_currentTimers = _timers;
	_nailgunAmmoCount = 0;
	_shotgunShellCount = 0;
	_nineGunBulletCount = 0;
	_thirtyEightGunBulletCount = 0;
	_chainsawFuelCount = 0;
	_playerCurrentHitPoints = Script::kDefaultPlayerHitPoints;
	_playerCombatLoadout = kDefaultPlayerCombatLoadout;
	_playerControlPaused = false;

	for (ObjectRecord &object : _currentObjects) {
		object.currentX = object.initialX;
		object.currentY = object.initialY;
		object.currentZ = object.initialZ;
		object.currentOwnerOrRoom = object.initialOwnerOrRoom;
		object.runtimeVisible = object.visible;
		object.identShown = object.identTextKey.empty();
	}

	for (AnimRecord &anim : _currentAnimations) {
		anim.runtimeActive = anim.active;
		anim.runtimeVisible = anim.visible;
		anim.runtimeState = -1;
	}

	for (NpcRecord &npc : _currentNpcs) {
		npc.runtimeSpawned = false;
		npc.runtimeState = -1;
		npc.savedVisible = npc.visible;
	}

	for (MonsterRecord &monster : _currentMonsters) {
		monster.currentHitPoints = monster.initialHitPoints;
		monster.runtimeSpawned = false;
		monster.runtimeState = -1;
		monster.savedVisible = monster.visible;
		if (monster.active)
			monster.visible = true;
	}

	for (TimerRecord &timer : _currentTimers)
		timer.currentValue = timer.initialValue;
}

void Script::logRuntimeSaveState(const char *operation) const {
	debugC(1, kDebugGeneral,
		"Harvester: %s runtime save state flags=%u objects=%u anims=%u regions=%u npcs=%u monsters=%u timers=%u hp=%d loadout=%d paused=%d ammo=[nails:%d shells:%d bullets9:%d bullets38:%d gas:%d]",
		operation, (uint)_currentFlags.size(), (uint)_currentObjects.size(),
		(uint)_currentAnimations.size(), (uint)_currentRegions.size(),
		(uint)_currentNpcs.size(), (uint)_currentMonsters.size(),
		(uint)_currentTimers.size(), _playerCurrentHitPoints, _playerCombatLoadout,
		_playerControlPaused, _nailgunAmmoCount, _shotgunShellCount,
		_nineGunBulletCount, _thirtyEightGunBulletCount, _chainsawFuelCount);

	for (const FlagRecord &flag : _currentFlags) {
		const FlagRecord *baseFlag = nullptr;
		for (const FlagRecord &candidate : _flags) {
			if (candidate.name.equalsIgnoreCase(flag.name)) {
				baseFlag = &candidate;
				break;
			}
		}
		if (!baseFlag || flag.value != baseFlag->value) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime flag '%s' value=%d base=%d defined_in_script=%d",
				operation, flag.name.c_str(), flag.value, baseFlag ? baseFlag->value : 0,
				baseFlag != nullptr);
		}
	}

	for (const ObjectRecord &object : _currentObjects) {
		const ObjectRecord *baseObject = nullptr;
		for (const ObjectRecord &candidate : _objects) {
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
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime object '%s' owner='%s' pos=(%d,%d,%d) visible=%d runtimeVisible=%d identShown=%d base_owner='%s' base_pos=(%d,%d,%d) base_visible=%d base_runtimeVisible=%d base_identShown=%d",
				operation, object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
				object.currentX, object.currentY, object.currentZ,
				object.visible, object.runtimeVisible, object.identShown,
				defaultOwner.c_str(), defaultX, defaultY, defaultZ,
				defaultVisible, defaultRuntimeVisible, defaultIdentShown);
		}
	}

	for (const NpcRecord &npc : _currentNpcs) {
		const NpcRecord *baseNpc = nullptr;
		for (const NpcRecord &candidate : _npcs) {
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
		const int defaultRuntimeState = -1;
		if (!baseNpc ||
				npc.active != defaultActive ||
				npc.visible != defaultVisible ||
				npc.savedVisible != defaultSavedVisible ||
				npc.deathOrMonsterfyFlag != defaultDeathOrMonsterfy ||
				npc.deathDamageType != defaultDeathDamageType ||
				npc.runtimeSpawned ||
				npc.runtimeState != defaultRuntimeState) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime npc '%s' active=%d visible=%d savedVisible=%d spawned=%d runtimeState=%d deathOrMonsterfy=%d damageType=%d base_active=%d base_visible=%d base_savedVisible=%d base_spawned=%d base_runtimeState=%d base_deathOrMonsterfy=%d base_damageType=%d",
				operation, npc.npcName.c_str(), npc.active, npc.visible, npc.savedVisible,
				npc.runtimeSpawned, npc.runtimeState, npc.deathOrMonsterfyFlag, npc.deathDamageType,
				defaultActive, defaultVisible, defaultSavedVisible,
				0, defaultRuntimeState, defaultDeathOrMonsterfy, defaultDeathDamageType);
		}
	}

	for (const MonsterRecord &monster : _currentMonsters) {
		const MonsterRecord *baseMonster = findBaseMonster(monster);

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
			debugC(3, kDebugGeneral,
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

	for (const TimerRecord &timer : _currentTimers) {
		const TimerRecord *baseTimer = nullptr;
		for (const TimerRecord &candidate : _timers) {
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
			debugC(3, kDebugGeneral,
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

	syncRecordArray(s, _currentFlags, syncStartupFlagRecord);
	syncRecordArray(s, _currentObjects, syncStartupObjectRecord);
	syncRecordArray(s, _currentAnimations, syncStartupAnimRecord);
	syncRecordArray(s, _currentRegions, syncStartupRegionRecord);
	syncRecordArray(s, _currentNpcs, syncStartupNpcRecord);
	syncRecordArray(s, _currentMonsters, syncStartupMonsterRecord);
	if (s.isLoading())
		restoreMonsterRecordIndices(_monsters, _currentMonsters);
	s.syncAsSint32LE(_playerCurrentHitPoints);
	syncRecordArray(s, _currentTimers, syncStartupTimerRecord);
	s.syncAsSint32LE(_playerCombatLoadout);
	syncBool(s, _playerControlPaused);
	s.syncAsSint32LE(_nailgunAmmoCount);
	s.syncAsSint32LE(_shotgunShellCount);
	s.syncAsSint32LE(_nineGunBulletCount);
	s.syncAsSint32LE(_thirtyEightGunBulletCount);
	s.syncAsSint32LE(_chainsawFuelCount);
	if (s.isLoading()) {
		_playerCurrentHitPoints = clampPlayerHitPoints(_playerCurrentHitPoints);
		_playerCombatLoadout = clampPlayerCombatLoadout(_playerCombatLoadout);
		_nailgunAmmoCount = clampCombatResourceCount(2, _nailgunAmmoCount);
		_shotgunShellCount = clampCombatResourceCount(3, _shotgunShellCount);
		_nineGunBulletCount = clampCombatResourceCount(4, _nineGunBulletCount);
		_thirtyEightGunBulletCount = clampCombatResourceCount(5, _thirtyEightGunBulletCount);
		_chainsawFuelCount = clampCombatResourceCount(14, _chainsawFuelCount);
		_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
			_currentObjects, _playerCombatLoadout, "save load");
		logRuntimeSaveState("loaded");
	}
}

bool Script::executeRoomEnterCommands(const Common::String &roomName,
		InteractionResult &result) {
	result = InteractionResult();

	const RoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	const CommandExecutionContext context = {
		"room entry command", room->roomName, room->roomName, true
	};
	executeCommandChain(room->onEnterCommand, context, kCommandOutputsInteraction, result);
	return true;
}

bool Script::executeRoomExitCommands(const Common::String &roomName,
		InteractionResult &result) {
	result = InteractionResult();

	const RoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	const CommandExecutionContext context = {
		"room exit command", room->roomName, room->roomName, false
	};
	executeCommandChain(room->onExitCommand, context, kCommandOutputsRoomExit, result);
	return true;
}

bool Script::resolveObjectInteraction(const ObjectRecord &object, InteractionResult &result,
		const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	Common::String commandRoomName = roomName;
	if (commandRoomName.empty() &&
			!object.currentOwnerOrRoom.empty() &&
			!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		commandRoomName = object.currentOwnerOrRoom;
	}

	if (isPickupObject(object)) {
		if (ObjectRecord *currentObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName)) {
			if (!currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
				const bool affectsCurrentRoom = !commandRoomName.empty() &&
					currentObject->currentOwnerOrRoom.equalsIgnoreCase(commandRoomName);
				currentObject->currentOwnerOrRoom = kInventoryOwnerName;
				currentObject->identShown = true;
				result.mutatedRuntimeState = true;
				result.visualRuntimeStateChanged = affectsCurrentRoom;
			}
		}
	}
	if (object.actionTag.empty())
		return result.mutatedRuntimeState;

	const CommandExecutionContext context = {
		"interaction command", object.objectName, commandRoomName, true
	};
	executeCommandChain(object.actionTag, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(object.actionTag);
}

bool Script::resolveRegionInteraction(const RegionRecord &region, InteractionResult &result,
		const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (region.actionTag.empty())
		return false;
	const Common::String &commandRoomName = roomName.empty() ? region.roomName : roomName;

	const CommandExecutionContext context = {
		"region command", region.regionName, commandRoomName, true
	};
	executeCommandChain(region.actionTag, context, kCommandOutputsInteraction, result);
	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(region.actionTag);
}

bool Script::resolveUseItemInteraction(const Common::String &itemName, const ObjectRecord &target,
		InteractionResult &result, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();

	const UseItemRecord *useItem = findUseItemRecord(itemName, target);
	if (!useItem)
		return false;
	Common::String commandRoomName = roomName;
	if (commandRoomName.empty() &&
			!target.currentOwnerOrRoom.empty() &&
			!target.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		commandRoomName = target.currentOwnerOrRoom;
	}

	const Common::String contextName = Common::String::format("%s -> %s",
		itemName.c_str(), target.objectName.c_str());
	const CommandExecutionContext context = {
		"useitem command", contextName, commandRoomName, true
	};
	executeCommandChain(useItem->actionTag, context, kCommandOutputsInteraction, result);
	return true;
}

bool Script::executeDebugCommand(const CommandRecord &command,
		InteractionResult &result, bool allowTransitions) {
	result = InteractionResult();
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

	CommandRecord debugCommand = command;
	debugCommand.triggerTag = debugTag;
	_commands.push_back(Common::move(debugCommand));
	const bool handled = executeActionTag(debugTag, result, allowTransitions, ScriptExecutionContext());
	_commands.pop_back();
	return handled;
}

bool Script::executeActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (tag.empty())
		return false;

	const CommandExecutionContext context = {
		"action tag", tag, roomName, allowTransitions
	};
	executeCommandChain(tag, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(tag);
}

bool Script::executeTimerAction(const Common::String &timerName, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (timerName.empty())
		return false;

	const TimerRecord *timer = findRuntimeTimer(timerName);
	if (!timer) {
		debug(1, "Harvester: unresolved timer record '%s'", timerName.c_str());
		return false;
	}
	if (timer->arg2.empty()) {
		debug(1, "Harvester: timer '%s' has no action tag", timerName.c_str());
		return false;
	}
	const Common::String &commandRoomName = roomName.empty() ? timer->arg1 : roomName;

	const CommandExecutionContext context = {
		"timer command", timer->timerName, commandRoomName, allowTransitions
	};
	executeCommandChain(timer->arg2, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(timer->arg2);
}

bool Script::executeNestedActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions) {
	const bool handled = executeActionTag(tag, result, allowTransitions, ScriptExecutionContext());
	if (handled)
		result.abortRemainingCommandChain = true;

	return handled;
}

bool Script::isPickupObject(const ObjectRecord &object) const {
	return !object.altSpritePath.empty() &&
		!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName);
}

bool Script::isPickupBlockedByAction(const ObjectRecord &object,
		InteractionResult *result) const {
	if (result)
		*result = InteractionResult();
	if (!isPickupObject(object) || object.actionTag.empty())
		return false;
	if (!result) {
		InteractionResult ignoredResult;
		return probePickupBlockingCommandChain(object.actionTag, object.objectName, ignoredResult, 0);
	}

	return probePickupBlockingCommandChain(object.actionTag, object.objectName, *result, 0);
}

bool Script::hasObjectInteraction(const ObjectRecord &object) const {
	if (isPickupObject(object))
		return true;
	if (object.actionTag.empty())
		return false;

	return hasActionableCommandChain(object.actionTag);
}

bool Script::hasUseItemInteraction(const Common::String &itemName, const ObjectRecord &target) const {
	return findUseItemRecord(itemName, target) != nullptr;
}

void Script::getVisibleInventoryObjects(Common::Array<ObjectRecord> &objects) const {
	objects.clear();
	const char *const statusObjectName = resolveInventoryStatusObjectName(_playerCurrentHitPoints);
	const ObjectRecord *statusObject = nullptr;

	for (const ObjectRecord &object : _currentObjects) {
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

	ObjectRecord activeStatusObject = *statusObject;
	activeStatusObject.currentOwnerOrRoom = kInventoryOwnerName;
	activeStatusObject.visible = true;
	activeStatusObject.runtimeVisible = true;
	activeStatusObject.identShown = true;
	objects.push_back(Common::move(activeStatusObject));
}

bool Script::isObjectInInventory(const Common::String &objectName) const {
	if (objectName.empty())
		return false;

	for (const ObjectRecord &object : _currentObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) &&
				object.objectName.equalsIgnoreCase(objectName))
			return true;
	}

	return false;
}

void Script::markObjectIdentShown(const ObjectRecord &object) {
	if (ObjectRecord *currentObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName))
		currentObject->identShown = true;
}

const EntranceRecord *Script::findEntranceRecord(const Common::String &entranceName) const {
	if (entranceName.empty())
		return nullptr;

	for (const EntranceRecord &entrance : _entrances) {
		if (entrance.entranceName.equalsIgnoreCase(entranceName))
			return &entrance;
	}

	return nullptr;
}

const MapEntranceRecord *Script::findMapEntranceRecord(const Common::String &entryName) const {
	if (entryName.empty())
		return nullptr;

	for (const MapEntranceRecord &mapEntrance : _mapEntrances) {
		if (mapEntrance.entryName.equalsIgnoreCase(entryName))
			return &mapEntrance;
	}

	return nullptr;
}

const UseItemRecord *Script::findUseItemRecord(const Common::String &itemName,
		const ObjectRecord &target) const {
	if (itemName.empty() || target.objectName.empty())
		return nullptr;

	for (const UseItemRecord &useItem : _useItems) {
		if (!useItem.itemName.equalsIgnoreCase(itemName) ||
			!useItem.targetName.equalsIgnoreCase(target.objectName)) {
			continue;
		}

		// Native dispatch ignores the parsed owner/group string and matches only
		// the carried item name plus the overlapped target entity name.
		return &useItem;
	}

	return nullptr;
}

const RoomRecord *Script::findRoomRecord(const Common::String &roomName) const {
	if (roomName.empty())
		return nullptr;

	for (const RoomRecord &room : _rooms) {
		if (room.roomName.equalsIgnoreCase(roomName))
			return &room;
	}

	return nullptr;
}

const CommandRecord *Script::findCommandRecord(const Common::String &tag) const {
	if (tag.empty())
		return nullptr;

	for (const CommandRecord &command : _commands) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

const ExecListRecord *Script::findExecListRecord(const Common::String &name) const {
	if (name.empty())
		return nullptr;

	for (const ExecListRecord &execList : _execLists) {
		if (execList.listName.equalsIgnoreCase(name))
			return &execList;
	}

	return nullptr;
}

const FlagRecord *Script::findRuntimeFlag(const Common::String &flagName) const {
	if (flagName.empty())
		return nullptr;

	for (const FlagRecord &flag : _currentFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

FlagRecord *Script::findRuntimeFlag(const Common::String &flagName) {
	if (flagName.empty())
		return nullptr;

	for (FlagRecord &flag : _currentFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

ObjectRecord *Script::findRuntimeObject(const Common::String &ownerOrRoom,
		const Common::String &objectName) {
	if (objectName.empty())
		return nullptr;

	ObjectRecord *fallback = nullptr;
	for (ObjectRecord &object : _currentObjects) {
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

AnimRecord *Script::findRuntimeAnim(const Common::String &animName) {
	if (animName.empty())
		return nullptr;

	for (AnimRecord &anim : _currentAnimations) {
		if (anim.animName.equalsIgnoreCase(animName))
			return &anim;
	}

	return nullptr;
}

RegionRecord *Script::findRuntimeRegion(const Common::String &regionName) {
	if (regionName.empty())
		return nullptr;

	for (RegionRecord &region : _currentRegions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

NpcRecord *Script::findRuntimeNpc(const Common::String &npcName) {
	if (npcName.empty())
		return nullptr;

	for (NpcRecord &npc : _currentNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

MonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) {
	if (monsterName.empty())
		return nullptr;

	for (MonsterRecord &monster : _currentMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

MonsterRecord *Script::findRuntimeMonster(const MonsterRecord &monster) {
	if (monster.monsterName.empty())
		return nullptr;

	MonsterRecord *fallback = nullptr;
	for (MonsterRecord &candidate : _currentMonsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

TimerRecord *Script::findRuntimeTimer(const Common::String &timerName) {
	if (timerName.empty())
		return nullptr;

	for (TimerRecord &timer : _currentTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const NpcRecord *Script::findRuntimeNpc(const Common::String &npcName) const {
	if (npcName.empty())
		return nullptr;

	for (const NpcRecord &npc : _currentNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

const MonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) const {
	if (monsterName.empty())
		return nullptr;

	for (const MonsterRecord &monster : _currentMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

const MonsterRecord *Script::findRuntimeMonster(const MonsterRecord &monster) const {
	if (monster.monsterName.empty())
		return nullptr;

	const MonsterRecord *fallback = nullptr;
	for (const MonsterRecord &candidate : _currentMonsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

const MonsterRecord *Script::findBaseMonster(const MonsterRecord &monster) const {
	if (monster.monsterName.empty())
		return nullptr;

	const MonsterRecord *fallback = nullptr;
	for (const MonsterRecord &candidate : _monsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

const TimerRecord *Script::findRuntimeTimer(const Common::String &timerName) const {
	if (timerName.empty())
		return nullptr;

	for (const TimerRecord &timer : _currentTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const TimerRecord *Script::findRuntimeTimerRecord(const Common::String &timerName) const {
	return findRuntimeTimer(timerName);
}

const NpcRecord *Script::findRuntimeNpcRecord(const Common::String &npcName) const {
	return findRuntimeNpc(npcName);
}

bool Script::addRuntimeObjectToInventory(const Common::String &objectName) {
	ObjectRecord *currentObject = findRuntimeObject(Common::String(), objectName);
	if (!currentObject)
		return false;

	const bool changed = !currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) ||
		!currentObject->visible || !currentObject->runtimeVisible || !currentObject->identShown;
	currentObject->currentOwnerOrRoom = kInventoryOwnerName;
	currentObject->visible = true;
	currentObject->runtimeVisible = true;
	currentObject->identShown = true;
	return changed;
}

bool Script::syncRuntimeObjectRecord(const ObjectRecord &object) {
	if (object.objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(Common::String(), object.objectName);
	if (!currentObject)
		return false;

	const bool changed = currentObject->currentX != object.currentX ||
		currentObject->currentY != object.currentY ||
		currentObject->currentZ != object.currentZ ||
		!currentObject->currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) ||
		currentObject->visible != object.visible ||
		currentObject->runtimeVisible != object.runtimeVisible ||
		currentObject->identShown != object.identShown;
	*currentObject = object;
	return changed;
}

bool Script::setRuntimeObjectVisible(const Common::String &ownerOrRoom,
		const Common::String &objectName, bool visible) {
	if (objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(ownerOrRoom, objectName);
	if (!currentObject)
		return false;

	const bool changed = currentObject->visible != visible ||
		currentObject->runtimeVisible != visible;
	currentObject->visible = visible;
	currentObject->runtimeVisible = visible;
	if (visible && currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName))
		currentObject->identShown = true;

	return changed;
}

bool Script::setRuntimeNpcState(const Common::String &npcName, bool active, bool visible) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc)
		return false;

	const bool changed = currentNpc->active != active || currentNpc->visible != visible;
	currentNpc->active = active;
	currentNpc->visible = visible;
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

int Script::getPlayerCombatResourceCount(int loadout) const {
	const int *count = getPlayerCombatResourceCountPtr(loadout);
	return count ? *count : 0;
}

bool Script::consumePlayerCombatResourceUnit(int loadout) {
	const int maxCount = resolveCombatResourcePickupMax(loadout);
	if (maxCount <= 0)
		return true;

	if (getPlayerCombatResourceCount(loadout) < 1)
		return false;

	return adjustPlayerCombatResourceCount(loadout, -1, maxCount, "PLAYER_ATTACK");
}

bool Script::setPlayerCombatResourceCount(int loadout, int count) {
	int *currentCount = getPlayerCombatResourceCountPtr(loadout);
	const int maxCount = resolveCombatResourceDisplayMax(loadout);
	if (!currentCount || maxCount <= 0)
		return false;

	const int clampedCount = CLIP<int>(count, 0, maxCount);
	const bool changed = *currentCount != clampedCount;
	*currentCount = clampedCount;
	return changed;
}

int *Script::getPlayerCombatResourceCountPtr(int loadout) {
	switch (loadout) {
	case 2:
		return &_nailgunAmmoCount;
	case 3:
		return &_shotgunShellCount;
	case 4:
		return &_nineGunBulletCount;
	case 5:
		return &_thirtyEightGunBulletCount;
	case 14:
		return &_chainsawFuelCount;
	default:
		return nullptr;
	}
}

const int *Script::getPlayerCombatResourceCountPtr(int loadout) const {
	switch (loadout) {
	case 2:
		return &_nailgunAmmoCount;
	case 3:
		return &_shotgunShellCount;
	case 4:
		return &_nineGunBulletCount;
	case 5:
		return &_thirtyEightGunBulletCount;
	case 14:
		return &_chainsawFuelCount;
	default:
		return nullptr;
	}
}

bool Script::adjustPlayerCombatResourceCount(int loadout, int delta, int maxCount,
		const Common::String &reason) {
	int *count = getPlayerCombatResourceCountPtr(loadout);
	if (!count || maxCount <= 0)
		return false;

	const int oldCount = *count;
	const int newCount = CLIP<int>(oldCount + delta, 0, maxCount);
	*count = newCount;
	debugC(1, kDebugGeneral,
		"Harvester: %s player combat resource loadout=%d('%s') delta=%d count=%d->%d max=%d changed=%d",
		reason.c_str(), loadout, Player::describeCombatLoadout(loadout),
		delta, oldCount, newCount, maxCount, oldCount != newCount);
	return oldCount != newCount;
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
	AnimRecord *currentAnimation = findRuntimeAnim(animName);
	if (!currentAnimation)
		return false;

	const bool changed = currentAnimation->active != active ||
		currentAnimation->visible != visible ||
		currentAnimation->runtimeActive != active ||
		currentAnimation->runtimeVisible != visible ||
		currentAnimation->runtimeState != currentFrame;
	currentAnimation->active = active;
	currentAnimation->visible = visible;
	currentAnimation->runtimeActive = active;
	currentAnimation->runtimeVisible = visible;
	currentAnimation->runtimeState = currentFrame;
	return changed;
}

bool Script::syncRuntimeMonsterRecord(const MonsterRecord &monster) {
	MonsterRecord *currentMonster = findRuntimeMonster(monster);
	if (!currentMonster)
		return false;

	*currentMonster = monster;
	return true;
}

bool Script::syncRuntimeTimerRecord(const TimerRecord &timer) {
	TimerRecord *currentTimer = findRuntimeTimer(timer.timerName);
	if (!currentTimer)
		return false;

	*currentTimer = timer;
	return true;
}

bool Script::setRuntimeRegionEnabled(const Common::String &regionName, bool enabled) {
	RegionRecord *currentRegion = findRuntimeRegion(regionName);
	if (!currentRegion)
		return false;

	const bool changed = currentRegion->startEnabled != enabled;
	currentRegion->startEnabled = enabled;
	return changed;
}

bool Script::setRuntimeTimerEnabled(const Common::String &timerName, bool enabled) {
	TimerRecord *currentTimer = findRuntimeTimer(timerName);
	if (!currentTimer)
		return false;

	const bool wasEnabled = currentTimer->enabled;
	const bool changed = wasEnabled != enabled ||
		(enabled && !wasEnabled && currentTimer->currentValue != currentTimer->initialValue);
	currentTimer->enabled = enabled;
	if (enabled && !wasEnabled)
		currentTimer->currentValue = currentTimer->initialValue;
	return changed;
}

bool Script::queueRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc || currentNpc->deathOrMonsterfyFlag)
		return false;

	// Native live MONSTERFY queues actor state 0x35 without a damage argument.
	// Use bludge as the queued animation selector when scripts leave it blank.
	const int queuedDeathDamageType = deathDamageType != 0 ? deathDamageType : 1;
	if (currentNpc->deathDamageType == queuedDeathDamageType)
		return false;

	currentNpc->deathDamageType = queuedDeathDamageType;
	return true;
}

bool Script::finalizeRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType,
		bool preserveCorpse, int corpseFrame, int monsterfyPosZ) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc)
		return false;

	bool monsterChanged = false;
	if (!currentNpc->monsterfyTargetName.empty()) {
		MonsterRecord monsterLookup;
		monsterLookup.roomName = currentNpc->roomName;
		monsterLookup.monsterName = currentNpc->monsterfyTargetName;
		MonsterRecord *currentMonster = findRuntimeMonster(monsterLookup);
		if (currentMonster) {
			const bool overrideMonsterPosZ = monsterfyPosZ != kNoMonsterfyPosZOverride;
			monsterChanged = !currentMonster->active || !currentMonster->visible ||
				(overrideMonsterPosZ && currentMonster->posZ != monsterfyPosZ);
			if (overrideMonsterPosZ)
				currentMonster->posZ = monsterfyPosZ;
			currentMonster->active = true;
			currentMonster->visible = true;
			currentMonster->runtimeSpawned = false;
			currentMonster->runtimeState = -1;
			if (currentMonster->currentHitPoints <= 0)
				currentMonster->currentHitPoints = currentMonster->initialHitPoints;
		} else {
			debug(1, "Harvester: unresolved monsterfy target for dialogue npc='%s' target='%s'",
				currentNpc->npcName.c_str(), currentNpc->monsterfyTargetName.c_str());
		}
	}

	const bool changed = !currentNpc->deathOrMonsterfyFlag ||
		currentNpc->active ||
		currentNpc->visible ||
		currentNpc->runtimeSpawned ||
		currentNpc->runtimeState != (preserveCorpse && corpseFrame >= 0 ? corpseFrame : -1) ||
		(deathDamageType != 0 && currentNpc->deathDamageType != deathDamageType);
	currentNpc->active = false;
	currentNpc->visible = preserveCorpse && corpseFrame >= 0 && currentNpc->monsterfyTargetName.empty();
	currentNpc->savedVisible = currentNpc->visible;
	currentNpc->deathOrMonsterfyFlag = !currentNpc->visible;
	currentNpc->runtimeSpawned = currentNpc->visible;
	currentNpc->runtimeState = currentNpc->visible ? corpseFrame : -1;
	if (deathDamageType != 0)
		currentNpc->deathDamageType = deathDamageType;
	return changed || monsterChanged;
}

void Script::pushDeferredLiveNpcDeathTransitions() {
	++_deferredLiveNpcDeathTransitionDepth;
}

void Script::popDeferredLiveNpcDeathTransitions() {
	if (_deferredLiveNpcDeathTransitionDepth > 0)
		--_deferredLiveNpcDeathTransitionDepth;
}

const TextRecord *Script::findTextRecord(const Common::String &key) const {
	if (key.empty())
		return nullptr;

	for (const TextRecord &textRecord : _texts) {
		if (textRecord.key.equalsIgnoreCase(key))
			return &textRecord;
	}

	return nullptr;
}

bool Script::resolveObjectInspectText(const ObjectRecord &object, ResolvedText &text) const {
	text = ResolvedText();

	const TextRecord *textRecord = findTextRecord(object.identTextKey);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

bool Script::resolveTextRecord(const Common::String &key, ResolvedText &text) const {
	text = ResolvedText();

	const TextRecord *textRecord = findTextRecord(key);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

Common::String Script::resolveInventoryTooltipText(const ObjectRecord &object) const {
	const TextRecord *textRecord = findTextRecord(object.inventoryTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return Common::String();
}

Common::String Script::resolveObjectLabel(const ObjectRecord &object) const {
	if (!object.interactionLabel.empty() && !object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return normalizeInteractionLabel(object.interactionLabel);
	if (object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return Common::String();

	if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		const Common::String inventoryLabel = resolveInventoryTooltipText(object);
		if (!inventoryLabel.empty())
			return inventoryLabel;
	}

	const TextRecord *textRecord = findTextRecord(object.identTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = normalizeInteractionLabel(object.objectName);
	if (!label.empty() && !label.equalsIgnoreCase("NULL ID"))
		return label;

	return Common::String();
}

Common::String Script::resolveTextValue(const Common::String &key) const {
	const TextRecord *textRecord = findTextRecord(key);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return normalizeInteractionLabel(key);
}

const HeadRecord *Script::findHeadRecord(const Common::String &headId) const {
	if (headId.empty())
		return nullptr;

	for (const HeadRecord &head : _heads) {
		if (head.headId.equalsIgnoreCase(headId))
			return &head;
	}

	return nullptr;
}

bool Script::getFlagValue(const Common::String &flagName) const {
	const FlagRecord *flag = findRuntimeFlag(flagName);
	return flag && flag->value;
}

bool Script::setRuntimeFlagValue(const Common::String &flagName, bool value) {
	if (flagName.empty())
		return false;

	FlagRecord *flag = findRuntimeFlag(flagName);
	if (flag) {
		const bool changed = flag->value != value;
		const bool oldValue = flag->value;
		flag->value = value;
		debugC(1, kDebugGeneral,
			"Harvester: direct runtime flag '%s' %d -> %d existed=1 changed=%d",
			flagName.c_str(), oldValue, value, changed);
		return changed;
	}

	FlagRecord newFlag;
	newFlag.name = flagName;
	newFlag.value = value;
	_currentFlags.push_back(Common::move(newFlag));
	debugC(1, kDebugGeneral,
		"Harvester: direct runtime flag '%s' %d -> %d existed=0 changed=1",
		flagName.c_str(), 0, value);
	return true;
}

bool Script::resetRuntimeObjectToInitialState(const Common::String &objectName) {
	if (objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(Common::String(), objectName);
	if (!currentObject)
		return false;

	const ObjectRecord *baseObject = nullptr;
	for (const ObjectRecord &object : _objects) {
		if (object.objectName.equalsIgnoreCase(objectName)) {
			baseObject = &object;
			break;
		}
	}
	if (!baseObject)
		return false;

	const bool changed = currentObject->currentX != currentObject->initialX ||
		currentObject->currentY != currentObject->initialY ||
		currentObject->currentZ != currentObject->initialZ ||
		!currentObject->currentOwnerOrRoom.equalsIgnoreCase(currentObject->initialOwnerOrRoom) ||
		currentObject->visible != baseObject->visible ||
		currentObject->runtimeVisible != baseObject->visible ||
		currentObject->identShown != baseObject->identShown;

	currentObject->currentX = currentObject->initialX;
	currentObject->currentY = currentObject->initialY;
	currentObject->currentZ = currentObject->initialZ;
	currentObject->currentOwnerOrRoom = currentObject->initialOwnerOrRoom;
	currentObject->visible = baseObject->visible;
	currentObject->runtimeVisible = baseObject->visible;
	currentObject->identShown = baseObject->identShown;
	return changed;
}

bool Script::isNamedNpcDeathTypeClear(const Common::String &npcName) const {
	const NpcRecord *npc = findRuntimeNpc(npcName);
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
