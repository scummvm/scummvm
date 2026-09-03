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

#ifndef HARVESTER_CHEATS_H
#define HARVESTER_CHEATS_H

#include "common/str.h"

namespace Common {
struct KeyState;
}

namespace Harvester {

class HarvesterEngine;
class Script;
struct InteractionResult;

enum CheatInputResult {
	kCheatInputIgnored = 0,
	kCheatInputPartial,
	kCheatInputComplete
};

struct CheatActivation {
	bool activated = false;
	bool inventoryChanged = false;
	bool playerStateChanged = false;
	bool clearInteractionState = false;
	bool hasInteraction = false;
};

class CheatSystem {
public:
	explicit CheatSystem(HarvesterEngine &engine);

	CheatInputResult processKey(const Common::KeyState &key, Script &script,
		const Common::String &roomName, InteractionResult &interaction,
		CheatActivation &activation);
	void reset();
	bool isPlayerDamageDisabled() const { return _playerDamageDisabled; }

private:
	enum CheatId {
		kCheatMaxHealth = 0,
		kCheatAllWeapons,
		kCheatAllItems,
		kCheatLodgeLevel2,
		kCheatLodgeLevel3,
		kCheatLodgeFinalLevel,
		kCheatInvincibility,
		kCheatLodgeLevel1
	};

	CheatInputResult matchKey(uint16 ascii, CheatId &cheatId);
	static const char *describe(CheatId cheatId);
	void execute(CheatId cheatId, Script &script, const Common::String &roomName,
		InteractionResult &interaction, CheatActivation &activation);

	HarvesterEngine &_engine;
	Common::String _inputBuffer;
	bool _playerDamageDisabled = false;
};

} // End of namespace Harvester

#endif // HARVESTER_CHEATS_H
