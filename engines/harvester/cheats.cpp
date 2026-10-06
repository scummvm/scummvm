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

#include "harvester/cheats.h"

#include "common/keyboard.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/script.h"

namespace Harvester {

static const char *const kCheatSoundPath = "1:/sound/effects/shotgun2.wav";
static const char *const kHarvestBladeObjectName = "HARVEST_BLADE";

static const char *const kTownWeaponObjects[] = {
	"9GUN",
	"WRENCH",
	"PITCHFORK",
	"SHOVEL",
	"FIREAXE",
	"BAT"
};

static const char *const kLodgeWeaponObjects[] = {
	"CLEAVER",
	"NAILGUN",
	"SHOTGUN",
	"FLAIL",
	"WRENCH",
	"SCYTHE",
	"SWORD",
	"CHAINSAW",
	"BAT",
	"POOLSTICK"
};

static const char *const kTownItemObjects[] = {
	"BROOMKEY",
	"GOOJF_CARD",
	"PC_PEN",
	"NEWSPAPER",
	"FLATHEAD",
	"ST_ASPRIN",
	"ST_COUGHMED",
	"ST_VITAMN",
	"SHOVEL"
};

static const char *const kLodgeItemObjects[] = {
	"WATERGLASFUL",
	"BARCASHFIVE",
	"CHUNK_O_MEAT",
	"CHUNK_O_MEAT2",
	"BARKEY",
	"FOUNTKEY",
	"WEEDKILLER",
	"ARTKEY",
	"KEWPIE_KEY",
	"KEWPIE",
	"CHESSKEY",
	"DRFIREWOOD",
	"SUPLY2_BOILKEY",
	"STAG_PLANK1",
	"STAG_PLANK2",
	"STAG_PLANK3",
	"FLAGA",
	"FLAGB",
	"BARCASH200",
	"CLUE"
};

static bool addInventoryObjects(Script &script, const char *const *objectNames, uint count) {
	bool changed = false;
	for (uint i = 0; i < count; ++i)
		changed = script.addRuntimeObjectToInventory(objectNames[i]) || changed;

	return changed;
}

const char *CheatSystem::describe(CheatId cheatId) {
	switch (cheatId) {
	case kCheatMaxHealth:
		return "max_health";
	case kCheatAllWeapons:
		return "all_weapons";
	case kCheatAllItems:
		return "all_items";
	case kCheatLodgeLevel2:
		return "lodge_level_2";
	case kCheatLodgeLevel3:
		return "lodge_level_3";
	case kCheatLodgeFinalLevel:
		return "lodge_final_level";
	case kCheatInvincibility:
		return "invincibility";
	case kCheatLodgeLevel1:
		return "lodge_level_1";
	default:
		return "unknown";
	}
}

CheatSystem::CheatSystem(HarvesterEngine &engine) : _engine(engine) {
}

CheatInputResult CheatSystem::matchKey(uint16 ascii, CheatId &cheatId) {
	struct CheatDefinition {
		const char *phrase;
		CheatId id;
	};
	static const CheatDefinition cheatDefinitions[] = {
		{ "NICK", kCheatMaxHealth },
		{ "MURDERER", kCheatAllWeapons },
		{ "SON OF SAM", kCheatAllItems },
		{ "BOSTON STRANGLER", kCheatLodgeLevel2 },
		{ "HELTER SKELTER", kCheatLodgeLevel3 },
		{ "CHARLES MANSON", kCheatLodgeFinalLevel },
		{ "BRUCE", kCheatInvincibility },
		{ "DUSTIN", kCheatLodgeLevel1 }
	};

	char character = 0;
	if (ascii >= 'a' && ascii <= 'z')
		character = (char)(ascii - 'a' + 'A');
	else if ((ascii >= 'A' && ascii <= 'Z') || ascii == ' ')
		character = (char)ascii;
	else {
		if (ascii >= 0x20 && ascii <= 0x7e)
			_inputBuffer.clear();
		return kCheatInputIgnored;
	}

	if (character != ' ' || (!_inputBuffer.empty() && _inputBuffer.lastChar() != ' '))
		_inputBuffer += character;

	for (uint i = 0; i < ARRAYSIZE(cheatDefinitions); ++i) {
		if (_inputBuffer == cheatDefinitions[i].phrase) {
			cheatId = cheatDefinitions[i].id;
			debugC(3, kDebugGeneral, "Harvester: cheat input matched phrase='%s'",
				cheatDefinitions[i].phrase);
			_inputBuffer.clear();
			return kCheatInputComplete;
		}
	}

	for (uint start = 0; start < _inputBuffer.size(); ++start) {
		const Common::String suffix = _inputBuffer.substr(start);
		for (uint i = 0; i < ARRAYSIZE(cheatDefinitions); ++i) {
			if (Common::String(cheatDefinitions[i].phrase).hasPrefix(suffix)) {
				_inputBuffer = suffix;
				debugC(3, kDebugGeneral, "Harvester: cheat input partial='%s'",
					_inputBuffer.c_str());
				return kCheatInputPartial;
			}
		}
	}

	_inputBuffer.clear();
	return kCheatInputIgnored;
}

CheatInputResult CheatSystem::processKey(const Common::KeyState &key, Script &script,
		const Common::String &roomName, InteractionResult &interaction,
		CheatActivation &activation) {
	interaction = InteractionResult();
	activation = CheatActivation();

	CheatId cheatId = kCheatMaxHealth;
	const CheatInputResult inputResult = matchKey(key.ascii, cheatId);
	if (inputResult == kCheatInputComplete)
		execute(cheatId, script, roomName, interaction, activation);

	return inputResult;
}

void CheatSystem::execute(CheatId cheatId, Script &script, const Common::String &roomName,
		InteractionResult &interaction, CheatActivation &activation) {
	const bool hasHarvestBlade = script.isObjectInInventory(kHarvestBladeObjectName);
	const bool requiresHarvestBlade = cheatId == kCheatLodgeLevel2 ||
		cheatId == kCheatLodgeLevel3 || cheatId == kCheatLodgeFinalLevel;
	const bool requiresTown = cheatId == kCheatLodgeLevel1;
	if ((requiresHarvestBlade && !hasHarvestBlade) || (requiresTown && hasHarvestBlade)) {
		debugC(2, kDebugGeneral,
			"Harvester: cheat unavailable id='%s' harvest_blade=%d",
			describe(cheatId), hasHarvestBlade ? 1 : 0);
		return;
	}

	activation.activated = true;
	if (!_engine.playSound(kCheatSoundPath)) {
		debugC(1, kDebugGeneral, "Harvester: cheat sound failed path='%s'",
			kCheatSoundPath);
	}

	switch (cheatId) {
	case kCheatMaxHealth:
		activation.playerStateChanged =
			script.setPlayerCurrentHitPoints(Script::kDefaultPlayerHitPoints);
		break;
	case kCheatAllWeapons:
		if (hasHarvestBlade) {
			activation.playerStateChanged =
				script.setPlayerCombatResourceCount(2, 16) || activation.playerStateChanged;
			activation.playerStateChanged =
				script.setPlayerCombatResourceCount(3, 16) || activation.playerStateChanged;
			activation.playerStateChanged =
				script.setPlayerCombatResourceCount(14, 16) || activation.playerStateChanged;
			activation.inventoryChanged = addInventoryObjects(
				script, kLodgeWeaponObjects, ARRAYSIZE(kLodgeWeaponObjects));
		} else {
			activation.playerStateChanged =
				script.setPlayerCombatResourceCount(4, 8) || activation.playerStateChanged;
			activation.playerStateChanged =
				script.setPlayerCombatResourceCount(5, 6) || activation.playerStateChanged;
			activation.inventoryChanged = addInventoryObjects(
				script, kTownWeaponObjects, ARRAYSIZE(kTownWeaponObjects));
		}
		break;
	case kCheatAllItems:
		activation.inventoryChanged = hasHarvestBlade
			? addInventoryObjects(script, kLodgeItemObjects, ARRAYSIZE(kLodgeItemObjects))
			: addInventoryObjects(script, kTownItemObjects, ARRAYSIZE(kTownItemObjects));
		break;
	case kCheatLodgeLevel2:
		activation.hasInteraction = script.executeActionTag(
			"FOUNTAIN_ART1", interaction, true, roomName);
		break;
	case kCheatLodgeLevel3:
		activation.clearInteractionState = true;
		activation.hasInteraction = script.executeActionTag(
			"3RD_2_3RDNTRYR", interaction, true, roomName);
		break;
	case kCheatLodgeFinalLevel:
		activation.clearInteractionState = true;
		activation.hasInteraction = script.executeActionTag(
			"L2_M1", interaction, true, roomName);
		break;
	case kCheatInvincibility:
		activation.clearInteractionState = true;
		_playerDamageDisabled = !_playerDamageDisabled;
		activation.playerStateChanged = true;
		break;
	case kCheatLodgeLevel1:
		activation.clearInteractionState = true;
		activation.hasInteraction = script.executeActionTag(
			"WARP_TO_LODGE", interaction, true, roomName);
		break;
	}

	if ((cheatId == kCheatLodgeLevel1 || cheatId == kCheatLodgeLevel2 ||
			cheatId == kCheatLodgeLevel3 || cheatId == kCheatLodgeFinalLevel) &&
			!activation.hasInteraction) {
		debugC(1, kDebugGeneral,
			"Harvester: cheat action tag produced no interaction id='%s' room='%s'",
			describe(cheatId), roomName.c_str());
	}

	debugC(2, kDebugGeneral,
		"Harvester: cheat activated id='%s' inventory_changed=%d player_state_changed=%d interaction=%d damage_disabled=%d",
		describe(cheatId), activation.inventoryChanged ? 1 : 0,
		activation.playerStateChanged ? 1 : 0, activation.hasInteraction ? 1 : 0,
		_playerDamageDisabled ? 1 : 0);
}

void CheatSystem::reset() {
	_inputBuffer.clear();
	_playerDamageDisabled = false;
}

} // End of namespace Harvester
