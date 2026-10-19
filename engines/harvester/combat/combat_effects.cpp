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

#include "common/debug.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/screen.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/player/player.h"

namespace Harvester {

const char *const kNativeHitEffectResourcePath = "blood.abm";
const int kNativeHitEffectAnimationRate = 10;
const float kNativeHitEffectRenderZBias = 0.01f;
const uint32 kCombatDebugDamagePopupDurationMs = 750;
const int kCombatDebugDamagePopupRisePixels = 18;
const int kCombatDebugDamagePopupVerticalGap = 6;

namespace {

static byte findNearestPaletteColor(const byte *palette, byte red, byte green, byte blue) {
	if (!palette)
		return 0;

	byte bestIndex = 0;
	uint32 bestDistance = 0xffffffffu;
	for (int i = 0; i < 256; ++i) {
		const int paletteOffset = i * 3;
		const int dr = static_cast<int>(palette[paletteOffset]) - red;
		const int dg = static_cast<int>(palette[paletteOffset + 1]) - green;
		const int db = static_cast<int>(palette[paletteOffset + 2]) - blue;
		const uint32 distance = static_cast<uint32>(dr * dr + dg * dg + db * db);
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = (byte)i;
		}
	}

	return bestIndex;
}

static void drawShadowedRoomText(Graphics::Screen &screen, const Graphics::Font &font,
		const Common::String &text, const Common::Point32 &position, byte textColor, byte shadowColor) {
	if (text.empty())
		return;

	const int width = font.getStringWidth(text);
	font.drawString(&screen, text, position.x + 1, position.y + 1, width, shadowColor);
	font.drawString(&screen, text, position.x, position.y, width, textColor);
}

} // End of anonymous namespace

CombatEffects::CombatEffects(EntityManager *entityManager) : _entityManager(entityManager) {
	debugC(1, kDebugCombat, "Harvester: created room combat effects entities=%d", _entityManager != nullptr);
}

void CombatEffects::clearHitEffects() {
	debugC(2, kDebugCombat, "Harvester: resetting room blood effects records=%u next_id=%u",
		static_cast<uint>(_hitEffectStates.size()), _nextCombatEffectId);
	_hitEffectStates.clear();
	_nextCombatEffectId = 0;
}

void CombatEffects::resolveHitEffectAnchor(const Entity &targetEntity, Common::Point &anchorPoint, float &renderZ) {
	const Common::Rect targetRect = targetEntity.getScreenRect();
	anchorPoint.x = targetRect.left + targetRect.width() / 2;
	anchorPoint.y = targetRect.top + targetRect.height() / 3;
	renderZ = targetEntity.getZ() - kNativeHitEffectRenderZBias;
}

void CombatEffects::resolveCombatDamagePopupAnchor(const Entity &targetEntity, Common::Point &anchorPoint) {
	const Common::Rect targetRect = targetEntity.getScreenRect();
	anchorPoint.x = targetRect.left + targetRect.width() / 2;
	anchorPoint.y = targetRect.top - kCombatDebugDamagePopupVerticalGap;
}

bool CombatEffects::spawnCombatHitEffect(Entity &sourceEntity, const Common::String &followTargetName) {
	if (!_entityManager)
		return false;

	HitEffectState effectState;
	effectState.entityName = Common::String::format("__combat_blood_%u", _nextCombatEffectId++);
	effectState.followTargetName = followTargetName;
	resolveHitEffectAnchor(sourceEntity, effectState.anchorPoint, effectState.renderZ);

	Entity *effectEntity = _entityManager->spawnSceneAnimationEntity(
		effectState.entityName, kNativeHitEffectResourcePath, effectState.anchorPoint,
		effectState.renderZ, kNativeHitEffectAnimationRate, true, true, false,
		false, false, 0);
	if (!effectEntity) {
		debugC(1, kDebugCombat, "Harvester: unable to spawn blood effect entity='%s' target='%s'",
			effectState.entityName.c_str(), followTargetName.c_str());
		return false;
	}

	_hitEffectStates.push_back(Common::move(effectState));
	return true;
}

void CombatEffects::spawnCombatDamagePopup(Entity &targetEntity, const Common::String &followTargetName, int damageAmount) {
	if (damageAmount <= 0)
		return;

	CombatDamagePopupState popupState;
	popupState.followTargetName = followTargetName;
	popupState.startTick = Player::getRuntimeClockTicks();
	popupState.damageAmount = damageAmount;
	resolveCombatDamagePopupAnchor(targetEntity, popupState.anchorPoint);
	_damagePopupStates.push_back(Common::move(popupState));
}

bool CombatEffects::syncCombatHitEffects() {
	if (!_entityManager)
		return false;

	bool changed = false;
	for (HitEffectState &effectState : _hitEffectStates) {
		Entity *effectEntity = _entityManager->findSceneEntityByName(effectState.entityName);
		if (!effectEntity)
			continue;

		if (!effectState.followTargetName.empty()) {
			Entity *targetEntity =
				_entityManager->findSceneEntityByName(effectState.followTargetName);
			if (targetEntity && targetEntity->isVisible()) {
				resolveHitEffectAnchor(*targetEntity, effectState.anchorPoint, effectState.renderZ);
			}
		}

		if (effectEntity->getX() != effectState.anchorPoint.x ||
				effectEntity->getY() != effectState.anchorPoint.y ||
				fabsf(effectEntity->getZ() - effectState.renderZ) > 0.0001f) {
			effectEntity->setPosition(
				effectState.anchorPoint.x, effectState.anchorPoint.y, effectState.renderZ);
			_entityManager->reinsertSceneEntity(effectEntity);
			changed = true;
		}
	}

	return changed;
}

void CombatEffects::drawCombatDamagePopups(Graphics::Screen &screen, bool combatDebugEnabled) {
	if (!combatDebugEnabled || _damagePopupStates.empty())
		return;

	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font)
		return;

	byte displayPalette[256 * 3];
	screen.getPalette(displayPalette);
	const byte white = findNearestPaletteColor(displayPalette, 0xff, 0xff, 0xff);
	const byte black = findNearestPaletteColor(displayPalette, 0x00, 0x00, 0x00);
	const uint32 now = Player::getRuntimeClockTicks();
	for (CombatDamagePopupState &popupState : _damagePopupStates) {
		if (_entityManager && !popupState.followTargetName.empty()) {
			Entity *targetEntity =
				_entityManager->findSceneEntityByName(popupState.followTargetName);
			if (targetEntity && targetEntity->isVisible())
				resolveCombatDamagePopupAnchor(*targetEntity, popupState.anchorPoint);
		}

		const uint32 elapsed = now - popupState.startTick;
		const uint32 clampedElapsed = MIN<uint32>(elapsed, kCombatDebugDamagePopupDurationMs);
		const int rise = static_cast<int>(
			clampedElapsed * static_cast<uint32>(kCombatDebugDamagePopupRisePixels) /
			kCombatDebugDamagePopupDurationMs);
		const Common::String popupText = Common::String::format("+%d", popupState.damageAmount);
		const int textWidth = font->getStringWidth(popupText);
		const int drawX = CLIP<int>(popupState.anchorPoint.x - textWidth / 2,
			0, MAX(0, screen.w - textWidth));
		const int drawY = CLIP<int>(
			popupState.anchorPoint.y - font->getFontHeight() - rise,
			0, MAX(0, screen.h - font->getFontHeight()));
		drawShadowedRoomText(screen, *font, popupText, Common::Point32(drawX, drawY), white, black);
	}
}

bool CombatEffects::pruneCombatHitEffects() {
	bool removedAny = false;
	for (uint i = 0; i < _hitEffectStates.size();) {
		const Common::String effectName = _hitEffectStates[i].entityName;
		Entity *effectEntity = _entityManager
			? _entityManager->findSceneEntityByName(effectName)
			: nullptr;
		if (!effectEntity) {
			_hitEffectStates.remove_at(i);
			continue;
		}

		const bool finished =
			!effectEntity->isAnimationEnabled() &&
			effectEntity->getCurrentFrame() == effectEntity->getLastFrame();
		if (finished && _hitEffectStates[i].finished) {
			Entity *removedEntity = _entityManager->detachSceneEntityByName(effectName);
			delete removedEntity;
			_hitEffectStates.remove_at(i);
			removedAny = true;
			continue;
		}

		_hitEffectStates[i].finished = finished;
		++i;
	}

	return removedAny;
}

bool CombatEffects::pruneCombatDamagePopups() {
	const uint32 now = Player::getRuntimeClockTicks();
	bool removedAny = false;
	for (uint i = 0; i < _damagePopupStates.size();) {
		if (now - _damagePopupStates[i].startTick < kCombatDebugDamagePopupDurationMs) {
			++i;
			continue;
		}

		_damagePopupStates.remove_at(i);
		removedAny = true;
	}

	return removedAny;
}

} // End of namespace Harvester
