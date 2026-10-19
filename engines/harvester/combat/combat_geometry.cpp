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

#include "harvester/combat/combat.h"

#include "common/util.h"
#include "harvester/player/player.h"
#include "harvester/entity.h"

namespace Harvester {

namespace {

int computeRectAxisGap(int minA, int maxA, int minB, int maxB) {
	if (maxA <= minB)
		return minB - maxA;
	if (maxB <= minA)
		return minA - maxB;
	return 0;
}

} // End of anonymous namespace

float clampRoomDepthForEvent(const RoomSetupState &state, float z) {
	return CLIP<float>(z,
		static_cast<float>(MIN(state.roomMinZ, state.roomMaxZ)),
		static_cast<float>(MAX(state.roomMinZ, state.roomMaxZ)));
}

int computeRuntimeEntityHorizontalGap(const Entity &left, const Entity &right) {
	const Common::Rect leftRect = left.getScreenRect();
	const Common::Rect rightRect = right.getScreenRect();
	return computeRectAxisGap(leftRect.left, leftRect.right, rightRect.left, rightRect.right);
}

bool doRuntimeEntityDepthExtentsOverlap(const Entity &first, const Entity &second) {
	const float firstZMin = first.getZ();
	const float firstZMax = firstZMin + first.getZExtent();
	const float secondZMin = second.getZ();
	const float secondZMax = secondZMin + second.getZExtent();
	return !(firstZMax < secondZMin || secondZMax < firstZMin);
}

bool areCombatantsWithinRoomCombatReach(const RoomSetupState &state,
		const Entity &attacker, float attackerZ,
		const Entity &target, float targetZ, int engageDistance) {
	if (attacker.overlapsEntity(target))
		return true;

	const float zDelta = attackerZ >= targetZ ? attackerZ - targetZ : targetZ - attackerZ;
	if (zDelta > kNativeMonsterPursuitZTolerance)
		return false;

	const float nearZ = attackerZ < targetZ ? attackerZ : targetZ;
	const int nativeTolerance = roundRoomCombatFloat(
		Player::computeDepthScale(state, nearZ) * kNativeMonsterHorizontalWaypointTolerance);
	return computeRuntimeEntityHorizontalGap(attacker, target) <= MAX(MAX(0, engageDistance), nativeTolerance);
}

bool isWithinNativeMonsterAttackEntryRange(const Entity &monster, float monsterZ,
		const Entity &player, float playerZ, int engageDistance) {
	const float zDelta = monsterZ >= playerZ ? monsterZ - playerZ : playerZ - monsterZ;
	if (zDelta > kNativeMonsterPursuitZTolerance)
		return false;

	const Common::Rect monsterRect = monster.getScreenRect();
	const Common::Rect playerRect = player.getScreenRect();
	const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
	const int playerCenterX = playerRect.left + playerRect.width() / 2;
	return ABS(playerCenterX - monsterCenterX) <= MAX(0, engageDistance);
}

int roundRoomCombatFloat(float value) {
	return value >= 0.0f ? static_cast<int>(value + 0.5f) : static_cast<int>(value - 0.5f);
}

} // End of namespace Harvester
