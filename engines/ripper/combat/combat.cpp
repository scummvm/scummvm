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
 */

#include "ripper/combat/combat.h"

#include "common/debug.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/milestones.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"
#include "ripper/settings.h"

namespace Ripper {

namespace {

static const uint kLogicalWidth = 320;
static const uint kLogicalHeight = 200;
static const uint kDisplayScale = 2;
static const uint kMeterSegments = 17;
static const uint kDosTicksPerSecond = 18;
static const uint16 kEncounterExitCommand = 1;
static const uint16 kEncounterFailureCommand = 2;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kScreenshotCommand = 0x1900;

static const int kMeterX[4] = { 11, 26, 11, 26 };
static const int kMeterBottomY[4] = { 54, 54, 119, 119 };
static const int kIndicatorX[4] = { 10, 25, 10, 25 };
static const int kIndicatorY[4] = { 59, 59, 124, 124 };
} // End of anonymous namespace

CombatEncounter::CombatEncounter(RipperEngine *engine,
		const CombatEncounterDefinition &definition) : Scene(engine),
		_definition(definition), _random("ripperCombat"), _nextCueHandle(0),
		_weaponRapidFireTicks(0),
		_activeScene(nullptr), _activeSceneIndex(0), _completionFlag(0),
		_arcadeKeywordIndex(0), _lastWeaponShotMillis(0), _weaponHeld(false),
		_singleShotReady(true), _shieldHeld(false), _lastShotHit(false),
		_overlayLogged(false),
		_paletteEffect(kPaletteNormal),
		_encounterResult(kExited) {
	for (uint i = 0; i < kMeterCount; ++i) {
		_meterPercent[i] = 100;
		_lastChargeMillis[i] = 0;
		_lastDischargeMillis[i] = 0;
	}
}

Common::String CombatEncounter::basenameFromPath(const Common::String &path) {
	uint start = 0;
	for (uint i = 0; i < path.size(); ++i) {
		if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
			start = i + 1;
	}
	return path.substr(start);
}

Common::String CombatEncounter::normalizeCueName(const Common::String &path) {
	Common::String name = basenameFromPath(path);
	if (name.equalsIgnoreCase("combat8.wav"))
		return "mechwav6.wav";
	if (name.equalsIgnoreCase("combat12.wav"))
		return "mechwav5.wav";
	return name;
}

const char *CombatEncounter::meterName(uint meter) {
	static const char *const names[kMeterCount] = {
		"health", "creature", "weapon", "shield"
	};
	return names[meter];
}

void CombatEncounter::queueCue(const Common::String &path, uint volumePercent) {
	if (path.empty())
		return;
	Audio::SoundHandle &handle = _cueHandles[_nextCueHandle++ % kCueHandleCount];
	_engine->getMedia()->stopSoundEffect(handle);
	_engine->getMedia()->playSoundEffect(normalizeCueName(path), handle, volumePercent, false);
}

void CombatEncounter::startEncounterAudio() {
	if (!_ambientSound.empty())
		_engine->getMedia()->playSoundEffect(_ambientSound, _ambientHandle, 100, true);
	if (!_creatureSound.empty())
		_engine->getMedia()->playSoundEffect(_creatureSound, _creatureHandle, 0, true);
}

void CombatEncounter::stopEncounterAudio() {
	stopAudio(_ambientHandle);
	stopAudio(_creatureHandle);
	stopAudio(_shieldHandle);
	for (uint i = 0; i < kCueHandleCount; ++i)
		stopAudio(_cueHandles[i]);
}

void CombatEncounter::updateContinuousAudio(const FrameState &frame) {
	_engine->getMedia()->setSoundEffectVolume(_creatureHandle, frame.creatureVolume);
	const uint mixVolume = _shieldHeld ? 50 : 100;
	_engine->getMedia()->setSoundEffectVolume(_ambientHandle, mixVolume);
	_engine->getMedia()->setSoundEffectVolume(_creatureHandle,
		frame.creatureVolume * mixVolume / 100);
}

void CombatEncounter::updateMeter(uint meter, uint32 now, bool recharge, bool drain) {
	MeterConfig &config = _meterConfig[meter];
	if (recharge && config.charge > 0 && _meterPercent[meter] < 100) {
		const uint32 interval = config.chargeTicks * 1000 / kDosTicksPerSecond;
		if (interval == 0 || now - _lastChargeMillis[meter] >= interval) {
			const int before = _meterPercent[meter];
			_meterPercent[meter] = MIN<int>(100, _meterPercent[meter] + config.charge);
			_lastChargeMillis[meter] = now;
			debugC(3, kDebugCombat,
				"Ripper: combat meter tick name='%s' cause=recharge amount=%d value=%d->%d intervalTicks=%u",
				meterName(meter), _meterPercent[meter] - before, before,
				_meterPercent[meter], config.chargeTicks);
		}
	}
	if (drain && config.discharge > 0 && _meterPercent[meter] > 0) {
		const uint32 interval = config.dischargeTicks * 1000 / kDosTicksPerSecond;
		if (interval == 0 || now - _lastDischargeMillis[meter] >= interval) {
			const int before = _meterPercent[meter];
			_meterPercent[meter] = MAX<int>(0, _meterPercent[meter] - config.discharge);
			_lastDischargeMillis[meter] = now;
			debugC(3, kDebugCombat,
				"Ripper: combat meter tick name='%s' cause=drain amount=%d value=%d->%d intervalTicks=%u",
				meterName(meter), before - _meterPercent[meter], before,
				_meterPercent[meter], config.dischargeTicks);
		}
	}
}

void CombatEncounter::applyIncomingAttack(byte attack, bool shieldHeld,
		uint32 now, uint frameIndex) {
	if (attack == 0 || attack > kDamageCount) {
		updateMeter(kShieldMeter, now, !shieldHeld, shieldHeld);
		return;
	}
	const uint damageIndex = attack - 1;
	const int healthBefore = _meterPercent[kHealthMeter];
	const int weaponBefore = _meterPercent[kWeaponMeter];
	const int shieldBefore = _meterPercent[kShieldMeter];
	_meterPercent[kWeaponMeter] = MAX<int>(0,
		_meterPercent[kWeaponMeter] - _meterConfig[kWeaponMeter].damage[damageIndex]);
	const int weaponDamage = weaponBefore - _meterPercent[kWeaponMeter];
	int healthDamage = 0;
	int shieldDamage = 0;
	if (shieldHeld && _meterPercent[kShieldMeter] > 0) {
		_meterPercent[kShieldMeter] = MAX<int>(0,
			_meterPercent[kShieldMeter] - _meterConfig[kShieldMeter].damage[damageIndex]);
		shieldDamage = shieldBefore - _meterPercent[kShieldMeter];
		queueCue(_shieldHitSounds[damageIndex]);
		_paletteEffect = kPaletteShield;
		// RunCombatEncounterScene applies the shield hit before its timed drain.
		updateMeter(kShieldMeter, now, false, true);
	} else {
		// The unshielded branch recharges the inactive shield before damaging
		// player health.
		updateMeter(kShieldMeter, now, true, false);
		_meterPercent[kHealthMeter] = MAX<int>(0,
			_meterPercent[kHealthMeter] - _meterConfig[kHealthMeter].damage[damageIndex]);
		healthDamage = healthBefore - _meterPercent[kHealthMeter];
		queueCue(_healthHitSounds[damageIndex]);
		_paletteEffect = kPalettePlayerHit;
	}
	debugC(2, kDebugCombat,
		"Ripper: combat enemy attack scene=%u frame=%u type=%u route='%s' weaponDamage=%d weapon=%d->%d healthDamage=%d health=%d->%d shieldDamage=%d shield=%d->%d shieldAfterTick=%d creature=%d",
		_activeSceneIndex, frameIndex, attack, shieldHeld ? "shield" : "health",
		weaponDamage, weaponBefore, _meterPercent[kWeaponMeter], healthDamage,
		healthBefore, _meterPercent[kHealthMeter], shieldDamage, shieldBefore,
		shieldBefore - shieldDamage, _meterPercent[kShieldMeter],
		_meterPercent[kCreatureMeter]);
}

const CombatEncounter::HitRegion *CombatEncounter::findHitRegion(
		const FrameState &frame, const Common::Point &point) const {
	for (uint i = 0; i < frame.hitRegions.size(); ++i) {
		if (frame.hitRegions[i].bounds.contains(point))
			return &frame.hitRegions[i];
	}
	return nullptr;
}

void CombatEncounter::spawnEffect(byte group, const Common::Point &point) {
	if (group >= kEffectGroupCount || _effectFrames[group].empty())
		return;
	uint slot = 0;
	for (; slot < kEffectCapacity && _effects[slot].active; ++slot) {
	}
	if (slot == kEffectCapacity) {
		slot = 0;
		for (uint candidate = 1; candidate < kEffectCapacity; ++candidate) {
			if (_effects[candidate].frame > _effects[slot].frame)
				slot = candidate;
		}
	}
	if (_effects[slot].active) {
		debugC(3, kDebugCombat,
			"Ripper: combat effect capacity reached replacing slot=%u group=%u frame=%u",
			slot, _effects[slot].group, _effects[slot].frame);
	}
	_effects[slot].active = true;
	_effects[slot].group = group;
	_effects[slot].frame = 0;
	_effects[slot].x = point.x;
	_effects[slot].y = point.y;
}

void CombatEncounter::serviceWeapon(const FrameState &frame, const Common::Point &point,
		bool weaponHeld, uint32 now, uint frameIndex) {
	if (!weaponHeld) {
		_weaponHeld = false;
		_singleShotReady = true;
		return;
	}
	if (!_weaponHeld) {
		_weaponHeld = true;
		_lastWeaponShotMillis = 0;
	}
	const uint rapidFireTicks = _weaponRapidFireTicks;
	const uint32 interval = rapidFireTicks * 1000 / kDosTicksPerSecond;
	if (_meterPercent[kWeaponMeter] == 0 || !_singleShotReady ||
			(_lastWeaponShotMillis != 0 && now - _lastWeaponShotMillis < interval))
		return;

	queueCue(_weaponSound);
	const int weaponBefore = _meterPercent[kWeaponMeter];
	_meterPercent[kWeaponMeter] = MAX<int>(0,
		_meterPercent[kWeaponMeter] - _meterConfig[kWeaponMeter].discharge);
	const HitRegion *region = findHitRegion(frame, point);
	const int creatureBefore = _meterPercent[kCreatureMeter];
	int hitType = -1;
	int hitRegion = -1;
	if (region) {
		const uint type = MIN<uint>(region->type, kEffectGroupCount - 1);
		hitType = type;
		hitRegion = (int)(region - frame.hitRegions.data());
		queueCue(_creatureHitSounds[type]);
		spawnEffect(type, point);
		_meterPercent[kCreatureMeter] = MAX<int>(0,
			_meterPercent[kCreatureMeter] - _meterConfig[kCreatureMeter].damage[type]);
		_paletteEffect = kPaletteTargetHit;
	}
	_lastShotHit = region != nullptr;
	_lastWeaponShotMillis = now;
	if (rapidFireTicks == 0)
		_singleShotReady = false;
	debugC(2, kDebugCombat,
		"Ripper: combat player shot scene=%u frame=%u hit=%d region=%d hitType=%d point=%d,%d bounds=%d,%d,%d,%d creatureDamage=%d creature=%d->%d weaponCost=%d weapon=%d->%d",
		_activeSceneIndex, frameIndex, region != nullptr, hitRegion, hitType,
		point.x, point.y, region ? region->bounds.left : 0,
		region ? region->bounds.top : 0, region ? region->bounds.right : 0,
		region ? region->bounds.bottom : 0,
		creatureBefore - _meterPercent[kCreatureMeter], creatureBefore,
		_meterPercent[kCreatureMeter], weaponBefore - _meterPercent[kWeaponMeter],
		weaponBefore, _meterPercent[kWeaponMeter]);
}

void CombatEncounter::drawFrameScaled(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel == frame.transparentColor)
				continue;
			const int targetX = (x + sourceX) * kDisplayScale;
			const int targetY = (y + sourceY) * kDisplayScale;
			for (uint dy = 0; dy < kDisplayScale; ++dy) {
				if (targetY + (int)dy < 0 || targetY + (int)dy >= (int)g_system->getHeight())
					continue;
				for (uint dx = 0; dx < kDisplayScale; ++dx) {
					if (targetX + (int)dx >= 0 && targetX + (int)dx < (int)g_system->getWidth())
						screen[(targetY + dy) * pitch + targetX + dx] = pixel;
				}
			}
		}
	}
}

void CombatEncounter::drawMeters(byte *screen, uint pitch, int panelX) const {
	if (_combatFrames.size() < 4)
		return;
	for (uint meter = 0; meter < kMeterCount; ++meter) {
		const uint filled = (_meterPercent[meter] * kMeterSegments + 99) / 100;
		for (uint segment = 0; segment < kMeterSegments; ++segment) {
			const BitmapAssetFrame &frame = _combatFrames[segment < filled ? 2 : 3];
			drawFrameScaled(screen, pitch, frame, panelX + kMeterX[meter],
				kMeterBottomY[meter] - segment * 2);
		}
	}
}

void CombatEncounter::drawIndicators(byte *screen, uint pitch, int panelX,
		bool enemyAttackActive) const {
	if (_combatFrames.size() < 5)
		return;
	const bool active[4] = {
		enemyAttackActive, _lastShotHit, _weaponHeld, _shieldHeld
	};
	for (uint indicator = 0; indicator < ARRAYSIZE(active); ++indicator) {
		const BitmapAssetFrame &frame = _combatFrames[active[indicator] ? 1 : 4];
		drawFrameScaled(screen, pitch, frame,
			panelX + kIndicatorX[indicator], kIndicatorY[indicator]);
	}
}

void CombatEncounter::drawCrosshair(byte *screen, uint pitch,
		const Common::Point &point) const {
	if (_crosshairFrames.size() < 4)
		return;
	drawFrameScaled(screen, pitch, _crosshairFrames[0],
		point.x - _crosshairFrames[0].width, point.y - 2);
	drawFrameScaled(screen, pitch, _crosshairFrames[1], point.x, point.y - 2);
	drawFrameScaled(screen, pitch, _crosshairFrames[2],
		point.x - 2, point.y - _crosshairFrames[2].height);
	drawFrameScaled(screen, pitch, _crosshairFrames[3], point.x - 2, point.y);
}

void CombatEncounter::drawEffects(byte *screen, uint pitch) const {
	for (uint i = 0; i < kEffectCapacity; ++i) {
		if (!_effects[i].active || _effects[i].group >= kEffectGroupCount ||
				_effects[i].frame >= _effectFrames[_effects[i].group].size())
			continue;
		const BitmapAssetFrame &frame = _effectFrames[_effects[i].group][_effects[i].frame];
		drawFrameScaled(screen, pitch, frame,
			_effects[i].x - frame.width / 2, _effects[i].y - frame.height / 2);
	}
}

void CombatEncounter::advanceEffects() {
	for (uint i = 0; i < kEffectCapacity; ++i) {
		if (!_effects[i].active)
			continue;
		++_effects[i].frame;
		if (_effects[i].group >= kEffectGroupCount ||
				_effects[i].frame >= _effectFrames[_effects[i].group].size())
			_effects[i].active = false;
	}
}

void CombatEncounter::drawOverlay(const Common::Point &point, bool targetActive,
		bool enemyAttackActive) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	byte *pixels = (byte *)screen->getPixels();
	// RunCombatEncounterScene at 0x31436 presents COMBAT0.BBM at
	// 320 - bitmapWidth. The meter anchors are local to that right-hand panel.
	const int panelX = _combatFrames.empty() ? 0 :
		(int)kLogicalWidth - (int)_combatFrames[0].width;
	// The retail loop composites effects and crosshairs first, then presents
	// the already-composed COMBAT0 status surface over them at 0x32790.
	drawEffects(pixels, screen->pitch);
	drawCrosshair(pixels, screen->pitch, point);
	if (!_combatFrames.empty())
		drawFrameScaled(pixels, screen->pitch, _combatFrames[0], panelX, 0);
	drawMeters(pixels, screen->pitch, panelX);
	drawIndicators(pixels, screen->pitch, panelX, enemyAttackActive);
	if (!_overlayLogged) {
		debugC(2, kDebugCombat,
			"Ripper: initialized combat overlay screen=%ux%u pitch=%u panelOrigin=%d,0 panelSize=%ux%u metersLocal=11/26,54/119",
			screen->w, screen->h, screen->pitch, panelX,
			_combatFrames.empty() ? 0 : _combatFrames[0].width,
			_combatFrames.empty() ? 0 : _combatFrames[0].height);
		_overlayLogged = true;
	}
	g_system->unlockScreen();
	debugC(11, kDebugCombat,
		"Ripper: drew combat overlay point=%d,%d target=%d attack=%d lastHit=%d weaponHeld=%d shieldHeld=%d health=%d creature=%d weapon=%d shield=%d",
		point.x, point.y, targetActive, enemyAttackActive, _lastShotHit,
		_weaponHeld, _shieldHeld, _meterPercent[kHealthMeter],
		_meterPercent[kCreatureMeter], _meterPercent[kWeaponMeter],
		_meterPercent[kShieldMeter]);
}

uint16 CombatEncounter::serviceKeyboard() {
	InputManager *input = _engine->getInput();
	bool paletteRefresh = false;
	while (input->hasPendingKey()) {
		const uint16 command = input->consumeKey();
		if (command == 0x1b) {
			_encounterResult = kExited;
			return kEncounterExitCommand;
		}
		if (command == kHelpCommand) {
			_engine->getModalDialog()->run(_definition.helpResource, true,
				ModalDialogManager::kMenubPresentation,
				ModalDialogManager::kPreserveActivePalette);
			paletteRefresh = true;
			continue;
		}
		if (command == kScreenshotCommand) {
			g_system->saveScreenshot();
			continue;
		}
		if (command <= 0xff) {
			byte character = command;
			if (character >= 'A' && character <= 'Z')
				character += 'a' - 'A';
			static const char keyword[] = "arcade";
			if (character == keyword[_arcadeKeywordIndex])
				++_arcadeKeywordIndex;
			else
				_arcadeKeywordIndex = 0;
			if (_arcadeKeywordIndex == sizeof(keyword) - 1) {
				_encounterResult = kSolved;
				debugC(1, kDebugCombat,
					"Ripper: combat encounter completed by arcade keyword type='%s'",
					_definition.name);
				return kEncounterExitCommand;
			}
		}
	}
	return paletteRefresh ? MediaSequenceCallback::kContinueRefreshPalette : 0;
}

uint16 CombatEncounter::service(uint frame) {
	// RunCombatEncounterScene at 0x31436 services one DAT state record before
	// advancing the active Smacker, so MediaPlayer's one-based callback frame
	// maps back to the zero-based frame table here.
	if (!_activeScene || _activeScene->frames.empty())
		return kEncounterFailureCommand;
	const uint frameIndex = MIN<uint>(frame == 0 ? 0 : frame - 1,
		_activeScene->frames.size() - 1);
	const FrameState &state = _activeScene->frames[frameIndex];
	if (frameIndex < _activeScene->frameCues.size()) {
		const FrameCue &cue = _activeScene->frameCues[frameIndex];
		if (cue.soundIndex >= 0 && (uint)cue.soundIndex < _activeScene->cueSounds.size())
			queueCue(_activeScene->cueSounds[cue.soundIndex], cue.volume);
	}

	const uint16 keyboardCommand = serviceKeyboard();
	if (keyboardCommand != 0 &&
			keyboardCommand != MediaSequenceCallback::kContinueRefreshPalette)
		return keyboardCommand;

	const MouseState &mouse = _engine->getInput()->peekMouseState();
	Common::Point point(CLIP<int>(mouse.position.x / (int)kDisplayScale, 0,
		(int)kLogicalWidth - 1), CLIP<int>(mouse.position.y / (int)kDisplayScale, 0,
		(int)kLogicalHeight - 1));
	const bool weaponHeld = (mouse.buttons & kMouseButtonLeft) != 0;
	bool shieldHeld = (mouse.buttons & kMouseButtonRight) != 0 &&
		_meterPercent[kShieldMeter] > 0;
	if (shieldHeld != _shieldHeld) {
		_shieldHeld = shieldHeld;
		if (_shieldHeld && !_shieldSound.empty())
			_engine->getMedia()->playSoundEffect(_shieldSound, _shieldHandle, 100, true);
		else
			stopAudio(_shieldHandle);
		debugC(2, kDebugCombat, "Ripper: combat shield active=%d", _shieldHeld);
	}

	_paletteEffect = shieldHeld ? kPaletteShield : kPaletteNormal;
	const uint32 now = g_system->getMillis(true);
	// RunCombatEncounterScene at 0x31436 services timed health/creature
	// changes, the incoming attack and shield branch, then the player weapon.
	updateMeter(kHealthMeter, now, true, true);
	updateMeter(kCreatureMeter, now, true, true);
	applyIncomingAttack(state.attack, shieldHeld, now, frameIndex);
	if (_meterPercent[kShieldMeter] == 0 && shieldHeld) {
		shieldHeld = false;
		_shieldHeld = false;
		stopAudio(_shieldHandle);
		_paletteEffect = kPaletteNormal;
		debugC(2, kDebugCombat, "Ripper: combat shield depleted");
	}
	if (!weaponHeld)
		updateMeter(kWeaponMeter, now, true, false);
	serviceWeapon(state, point, weaponHeld, now, frameIndex);
	updateContinuousAudio(state);
	const bool targetActive = findHitRegion(state, point) != nullptr;
	drawOverlay(point, targetActive, state.attack != 0);
	advanceEffects();

	if (_meterPercent[kCreatureMeter] == 0) {
		_encounterResult = kSolved;
		debugC(1, kDebugCombat,
			"Ripper: combat encounter creature defeated type='%s' frame=%u scene=%u",
			_definition.name, frameIndex, _activeSceneIndex);
		return kEncounterExitCommand;
	}
	if (_meterPercent[kHealthMeter] == 0) {
		_encounterResult = kExited;
		debugC(1, kDebugCombat,
			"Ripper: combat encounter player defeated type='%s' frame=%u scene=%u command=0x%04x health=%d creature=%d weapon=%d shield=%d",
			_definition.name, frameIndex, _activeSceneIndex, kEncounterExitCommand,
			_meterPercent[kHealthMeter], _meterPercent[kCreatureMeter],
			_meterPercent[kWeaponMeter], _meterPercent[kShieldMeter]);
		return kEncounterExitCommand;
	}
	return keyboardCommand;
}

void CombatEncounter::transformPalette(byte *palette, uint colorCount) const {
	if (_paletteEffect == kPaletteNormal || colorCount == 0)
		return;
	for (uint color = 0; color < MIN<uint>(colorCount, 256); ++color) {
		if (color == 0 || (color >= 4 && color <= 9) || color >= 240)
			continue;
		byte *rgb = palette + color * 3;
		const uint luminance = (rgb[0] * 30 + rgb[1] * 59 + rgb[2] * 11) / 100;
		if (_paletteEffect == kPalettePlayerHit) {
			rgb[0] = MIN<uint>(rgb[0] + 60, 252);
		} else if (_paletteEffect == kPaletteTargetHit) {
			rgb[0] = MIN<uint>(luminance + 60, 252);
			rgb[1] = luminance;
			rgb[2] = MIN<uint>(luminance + 40, 252);
		} else {
			rgb[0] = luminance;
			rgb[1] = luminance;
			rgb[2] = MIN<uint>(luminance + 40, 252);
		}
	}
}

uint CombatEncounter::chooseNextScene() {
	if (_scenes.size() <= 1)
		return 0;
	if (_definition.atkiniSceneRouting && _scenes.size() >= 6) {
		// RunCombatEncounterScene at 0x324a7 uses the tables at 0x3134c
		// and 0x31358 to keep the six ATKINI scenes moving between groups.
		static const uint sceneGroupOne[] = { 0, 4, 5 };
		static const uint sceneGroupTwo[] = { 1, 2, 3 };
		if (_activeSceneIndex == 3 || _activeSceneIndex == 4)
			return sceneGroupOne[_random.getRandomNumber(2)];
		if (_activeSceneIndex == 5)
			return sceneGroupTwo[_random.getRandomNumber(2)];
	}
	return _random.getRandomNumber(_scenes.size() - 2) + 1;
}

Scene::Result CombatEncounter::run(uint completionFlag) {
	prepare("combat-entry", 14, false);
	_completionFlag = completionFlag;
	// RunCombatEncounterScene at 0x31563 reads g_combatLevelSetting once and
	// formats the encounter's numbered INI before loading any combat resources.
	const uint difficulty = _engine->getSettings()->getCombatLevel();
	const Common::String configName = Common::String::format(
		_definition.iniPattern, difficulty);
	debugC(1, kDebugCombat,
		"Ripper: initiating combat encounter type='%s' difficulty=%u config='%s' completionFlag=%u",
		_definition.name, difficulty, configName.c_str(), completionFlag);
	_encounterResult = kExited;
	_arcadeKeywordIndex = 0;
	_activeSceneIndex = 0;
	_activeScene = nullptr;
	_nextCueHandle = 0;
	_weaponHeld = false;
	_singleShotReady = true;
	_shieldHeld = false;
	_lastShotHit = false;
	_overlayLogged = false;
	_paletteEffect = kPaletteNormal;
	const uint32 now = g_system->getMillis(true);
	for (uint meter = 0; meter < kMeterCount; ++meter) {
		_meterPercent[meter] = 100;
		_lastChargeMillis[meter] = now;
		_lastDischargeMillis[meter] = now;
	}
	for (uint i = 0; i < kEffectCapacity; ++i)
		_effects[i] = Effect();

	if (!loadResources(difficulty)) {
		_encounterResult = kLoadFailed;
	} else {
		startEncounterAudio();
		debugC(1, kDebugCombat,
			"Ripper: entered combat encounter type='%s' difficulty=%u config='%s' completionFlag=%u scenes=%u",
			_definition.name, difficulty, configName.c_str(), completionFlag,
			_scenes.size());
		while (!_engine->shouldQuit()) {
			_activeScene = &_scenes[_activeSceneIndex];
			uint16 command = 0;
			if (!_engine->getMedia()->playCombatSequence(
					_activeScene->basename + ".smk", this, &command)) {
				_encounterResult = kLoadFailed;
				break;
			}
			debugC(2, kDebugCombat,
				"Ripper: combat media returned scene=%u command=0x%04x result=%d health=%d creature=%d weapon=%d shield=%d",
				_activeSceneIndex, command, _encounterResult,
				_meterPercent[kHealthMeter], _meterPercent[kCreatureMeter],
				_meterPercent[kWeaponMeter], _meterPercent[kShieldMeter]);
			if (command == kEncounterFailureCommand) {
				_encounterResult = kLoadFailed;
				break;
			}
			if (command == kEncounterExitCommand ||
					_encounterResult == kSolved || _engine->shouldQuit())
				break;
			_activeSceneIndex = chooseNextScene();
			debugC(2, kDebugCombat,
				"Ripper: combat encounter selected scene=%u media='%s.smk'",
				_activeSceneIndex, _scenes[_activeSceneIndex].basename.c_str());
		}
	}

	debugC(2, kDebugCombat,
		"Ripper: cleaning up combat encounter result=%d difficulty=%u completionFlag=%u activeScene=%u",
		_encounterResult, difficulty, completionFlag, _activeSceneIndex);
	stopEncounterAudio();
	if (_encounterResult == kSolved &&
			!_engine->getMilestones()->set(completionFlag, true, "combat-encounter"))
		_encounterResult = kLoadFailed;
	finish("combat-exit", 0, true);
	debugC(_encounterResult == kLoadFailed ? 1 : 2, kDebugCombat,
		"Ripper: left combat encounter type='%s' result=%d difficulty=%u completionFlag=%u",
		_definition.name, _encounterResult, difficulty, completionFlag);
	return _encounterResult;
}

} // End of namespace Ripper
