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

#include "harvester/combat.h"

#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/screen.h"
#include "harvester/cheats.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/player.h"
#include "harvester/interaction.h"
#include "harvester/scene_entities.h"
#include "harvester/entity.h"

namespace Harvester {

const float kNativeNpcMonsterZExtent = 5.0f;
const int kRoomNpcAmbientLastFrame = 0x3b;
const uint32 kNativeKeyboardAttackRepeatTicks = 25;
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
		const Common::String &text, int x, int y, byte textColor, byte shadowColor) {
	if (text.empty())
		return;

	const int width = font.getStringWidth(text);
	font.drawString(&screen, text, x + 1, y + 1, width, shadowColor);
	font.drawString(&screen, text, x, y, width, textColor);
}


Common::String resolveCombatLoadoutStatusLabel(Script &script, const ObjectRecord &object) {
	Common::String label = script.resolveInventoryTooltipText(object);
	if (!label.empty()) {
		for (uint i = 0; i < label.size(); ++i) {
			if (label[i] == '_')
				label.setChar(' ', i);
		}

		uint trimAt = label.size();
		for (uint i = 0; i < label.size(); ++i) {
			if (label[i] == ',' || label[i] == ';') {
				trimAt = i;
				break;
			}
		}
		label = label.substr(0, trimAt);
		while (!label.empty() && label.lastChar() == ' ')
			label.deleteLastChar();
		if (!label.empty())
			return label;
	}

	label = script.resolveObjectLabel(object);
	if (!label.empty())
		return label;

	label = object.objectName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

} // End of anonymous namespace

bool resolveCombatLoadoutHudInfo(int loadout, CombatLoadoutHudInfo &info) {
	switch (loadout) {
	case 2:
		info.iconIndex = 2;
		info.maxVisibleCount = 16;
		info.unitLabel = "nails";
		return true;
	case 3:
		info.iconIndex = 1;
		info.maxVisibleCount = 16;
		info.unitLabel = "shells";
		return true;
	case 4:
		info.iconIndex = 0;
		info.maxVisibleCount = 8;
		info.unitLabel = "bullets";
		return true;
	case 5:
		info.iconIndex = 0;
		info.maxVisibleCount = 6;
		info.unitLabel = "bullets";
		return true;
	case 14:
		info.iconIndex = 3;
		info.maxVisibleCount = 16;
		info.unitLabel = "gas";
		return true;
	default:
		return false;
	}
}

Common::String buildCombatLoadoutStatusMessage(Script &script, const ObjectRecord &object,
		int previousLoadout, int currentLoadout) {
	const Common::String label = resolveCombatLoadoutStatusLabel(script, object);
	if (label.empty())
		return Common::String();

	if (currentLoadout == 0 && previousLoadout != 0)
		return Common::String::format("Disarming %s...", label.c_str());

	CombatLoadoutHudInfo info;
	if (resolveCombatLoadoutHudInfo(currentLoadout, info))
		return Common::String::format("Arming %s, %d %s...", label.c_str(),
			script.getPlayerCombatResourceCount(currentLoadout), info.unitLabel);

	return Common::String::format("Arming %s...", label.c_str());
}

const char *describeCombatTargetClass(int classId) {
	switch (classId) {
	case kEntityClassNpc:
		return "npc";
	case kEntityClassMonster:
		return "monster";
	case kEntityClassPlayer:
		return "player";
	default:
		return "none";
	}
}

Combat::Combat(HarvesterEngine &engine, SceneState &scene, Player &player,
		EntityManager *entityManager, SceneEntities &sceneRuntime, CombatEffects &combatEffects,
		const CheatSystem &cheats, InteractionCallbacks &callbacks, Interaction &interactionProcessor)
	: _engine(engine), _scene(scene), _player(player), _entityManager(entityManager),
	  _sceneRuntime(sceneRuntime), _combatEffects(combatEffects), _cheats(cheats),
	  _callbacks(callbacks), _interactionProcessor(interactionProcessor) {
}

void Combat::initializeCombatStates() {
	_monsterCombatStates.resize(_scene.state.roomMonsters.size());
	_npcCombatStates.resize(_scene.state.roomNpcs.size());
	debugC(1, kDebugCombat, "Harvester: initialized room combat states monsters=%u npcs=%u",
		_monsterCombatStates.size(), _npcCombatStates.size());
}

void Combat::resizeCombatStates() {
	if (_npcCombatStates.size() != _scene.state.roomNpcs.size())
		_npcCombatStates.resize(_scene.state.roomNpcs.size());
	if (_monsterCombatStates.size() != _scene.state.roomMonsters.size())
		_monsterCombatStates.resize(_scene.state.roomMonsters.size());
}

Common::Array<MonsterCombatState> Combat::prepareMonsterCombatStatesForSceneChange(
		const Common::Array<MonsterRecord> &previousMonsters, const Common::Array<MonsterRecord> &updatedMonsters) const {
	Common::Array<MonsterCombatState> updatedMonsterCombatStates;
	updatedMonsterCombatStates.resize(updatedMonsters.size());
	for (uint i = 0; i < updatedMonsters.size(); ++i) {
		const MonsterRecord &updatedMonster = updatedMonsters[i];
		for (uint j = 0; j < previousMonsters.size() &&
				j < _monsterCombatStates.size(); ++j) {
			const MonsterRecord &previousMonster = previousMonsters[j];
			if (!previousMonster.monsterName.equalsIgnoreCase(updatedMonster.monsterName))
				continue;
			if (sameMonsterEntityState(previousMonster, updatedMonster))
				updatedMonsterCombatStates[i] = _monsterCombatStates[j];
			break;
		}
	}

	return updatedMonsterCombatStates;
}

void Combat::commitMonsterCombatStatesForSceneChange(Common::Array<MonsterCombatState> &&states) {
	_monsterCombatStates = Common::move(states);
	debugC(2, kDebugCombat, "Harvester: committed room monster combat states count=%u", _monsterCombatStates.size());
}

void Combat::resetForSceneRefresh() {
	// NPC death state survives refresh and is resized by index at the next combat update.
	_monsterCombatStates.clear();
	_monsterCombatStates.resize(_scene.state.roomMonsters.size());
	debugC(2, kDebugCombat, "Harvester: reset room monster combat states count=%u retained_npcs=%u",
		_monsterCombatStates.size(), _npcCombatStates.size());
}

void clearRoomMonsterCombatState(MonsterCombatState &combatState) {
	combatState = MonsterCombatState();
}

void clearRoomNpcCombatState(NpcCombatState &combatState) {
	combatState = NpcCombatState();
}

void clearRoomMonsterAttackState(MonsterCombatState &combatState) {
	combatState.attackActive = false;
	combatState.attackFirstFrame = -1;
	combatState.attackLastFrame = -1;
	combatState.attackContactFrame = -1;
	combatState.attackResumeFacing = -1;
	combatState.attackSoundPlayed = false;
	combatState.attackContactResolved = false;
	combatState.attackTargetName.clear();
}

void clearRoomMonsterHitState(MonsterCombatState &combatState) {
	combatState.hitActive = false;
	combatState.hitFirstFrame = -1;
	combatState.hitLastFrame = -1;
	combatState.hitResumeFacing = -1;
	combatState.hitKnockbackRemainingX = 0;
	combatState.hitKnockbackDecayStep = 0;
	combatState.hitSoundPlayed = false;
}

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
		drawShadowedRoomText(screen, *font, popupText, drawX, drawY, white, black);
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
