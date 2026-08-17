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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/combat/combat.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"

#include "ripper/detection.h"
#include "ripper/ini.h"
#include "ripper/ripper.h"

namespace Ripper {

CombatEncounter::MeterConfig::MeterConfig() : charge(0), discharge(0),
		chargeTicks(0), dischargeTicks(0) {
	for (uint i = 0; i < kDamageCount; ++i)
		damage[i] = 0;
}

Common::String CombatEncounter::configString(const Common::INIFile &ini,
		const char *section, const char *key, const Common::String &fallback) const {
	return getIniString(ini, section, key, fallback);
}

int CombatEncounter::configInt(const Common::INIFile &ini, const char *section,
		const char *key, int fallback) const {
	return getIniInt(ini, section, key, fallback);
}

bool CombatEncounter::loadConfig(const Common::String &name, Common::INIFile &ini) {
	Common::File file;
	if (!file.open(Common::Path(name))) {
		warning("Ripper: could not open combat configuration '%s'", name.c_str());
		return false;
	}
	ini.requireKeyValueDelimiter();
	if (!ini.loadFromStream(file)) {
		warning("Ripper: could not parse combat configuration '%s'", name.c_str());
		return false;
	}
	return true;
}

void CombatEncounter::loadMeterConfig(const Common::INIFile &ini, const char *section,
		MeterConfig &config) {
	for (uint i = 0; i < kDamageCount; ++i) {
		const Common::String key = Common::String::format("damage%u", i + 1);
		config.damage[i] = configInt(ini, section, key.c_str(), 0);
	}
	config.charge = configInt(ini, section, "charge", 0);
	config.discharge = configInt(ini, section, "discharge", 0);
	config.chargeTicks = configInt(ini, section, "charge time", 0);
	config.dischargeTicks = configInt(ini, section, "discharge time", 0);
}

bool CombatEncounter::loadFrameRegions(const Common::String &name, SceneData &scene) {
	Common::File file;
	if (!file.open(Common::Path(name))) {
		warning("Ripper: could not open combat frame-region table '%s'", name.c_str());
		return false;
	}

	PresentationFrameRegionTable table;
	if (!decodePresentationFrameRegionTable(file, table)) {
		warning("Ripper: invalid combat frame-region table '%s'", name.c_str());
		return false;
	}

	uint regionCount = 0;
	scene.frames.resize(table.frames.size());
	for (uint frame = 0; frame < table.frames.size(); ++frame) {
		const PresentationFrameRegion &sourceFrame = table.frames[frame];
		FrameState &state = scene.frames[frame];
		state.attack = sourceFrame.state;
		state.creatureVolume = sourceFrame.auxiliary;
		regionCount += sourceFrame.regions.size();
		for (uint i = 0; i < sourceFrame.regions.size(); ++i) {
			const PresentationRegion &source = sourceFrame.regions[i];
			// The hit-test loop at 0x320d0 compares the first coordinate pair
			// with the logical pointer Y and the second pair with pointer X.
			// DAT regions therefore store type, y, x, height, width.
			const int y = source.coordinate1;
			const int x = source.coordinate2;
			const int height = source.extent1;
			const int width = source.extent2;
			if (width <= 0 || height <= 0)
				continue;
			HitRegion region;
			region.type = source.type;
			region.bounds = Common::Rect(x, y, x + width, y + height);
			scene.frames[frame].hitRegions.push_back(region);
		}
	}

	debugC(2, kDebugCombat,
		"Ripper: loaded combat frame-region table file='%s' frames=%u regions=%u",
		name.c_str(), table.frames.size(), regionCount);
	debugC(3, kDebugCombat,
		"Ripper: decoded combat hit-region records file='%s' layout=type,y,x,height,width",
		name.c_str());
	return true;
}

bool CombatEncounter::loadFrameCues(const Common::String &name, SceneData &scene) {
	Common::File file;
	if (!file.open(Common::Path(name))) {
		// LoadPresentationFrameAudioCueMap at 0x3574a returns its zero-filled
		// two-byte-per-frame allocation when the optional PRJ cannot be opened.
		scene.frameCues.resize(scene.frames.size());
		debugC(2, kDebugCombat,
			"Ripper: combat frame-audio map absent file='%s' frames=%u cues=0",
			name.c_str(), scene.frames.size());
		return true;
	}

	PresentationFrameAudioMap map;
	if (!decodePresentationFrameAudioMap(file, scene.frames.size(), map)) {
		warning("Ripper: invalid combat frame-audio map '%s'", name.c_str());
		return false;
	}

	for (uint i = 0; i < map.sounds.size(); ++i)
		scene.cueSounds.push_back(normalizeCueName(map.sounds[i]));
	scene.frameCues.resize(map.cues.size());
	for (uint frame = 0; frame < scene.frameCues.size(); ++frame) {
		scene.frameCues[frame].soundIndex = map.cues[frame].soundIndex;
		scene.frameCues[frame].volume = map.cues[frame].volume;
	}

	debugC(2, kDebugCombat,
		"Ripper: loaded combat frame-audio map file='%s' frameRate=%u frames=%u sounds=%u",
		name.c_str(), map.frameRate, scene.frameCues.size(), map.sounds.size());
	return true;
}

bool CombatEncounter::loadSceneData(SceneData &scene) {
	return loadFrameRegions(scene.basename + "dat.dat", scene) &&
		loadFrameCues(scene.basename + "prj.prj", scene);
}

bool CombatEncounter::loadBitmapAssets() {
	ResourceManager *resources = _engine->getResources();
	_combatFrames.clear();
	_crosshairFrames.clear();
	for (uint i = 0; i < 5; ++i) {
		BitmapAssetSequence sequence;
		const Common::String name = Common::String::format("combat%u.bbm", i);
		if (!resources->loadBitmapSequence(name, sequence) || sequence.frames.empty())
			return false;
		_combatFrames.push_back(Common::move(sequence.frames[0]));
		const BitmapAssetFrame &frame = _combatFrames.back();
		debugC(3, kDebugCombat,
			"Ripper: loaded combat overlay bitmap index=%u size=%ux%u transparent=%u paletteColors=%u",
			i, frame.width, frame.height, frame.transparentColor,
			frame.palette.size() / 3);
	}
	for (uint i = 0; i < 4; ++i) {
		BitmapAssetSequence sequence;
		const Common::String name = Common::String::format("%s%u.bbm",
			_definition.crosshairPrefix, i);
		if (!resources->loadBitmapSequence(name, sequence) || sequence.frames.empty())
			return false;
		_crosshairFrames.push_back(Common::move(sequence.frames[0]));
	}
	for (uint i = 0; i < kEffectGroupCount; ++i) {
		if (!resources->loadBitmapLibrary(_definition.effectLibraries[i], _effectFrames[i]))
			return false;
		debugC(2, kDebugCombat,
			"Ripper: loaded combat effect group=%u library='%s' frames=%u transparent=%u",
			i, _definition.effectLibraries[i], _effectFrames[i].size(),
			_effectFrames[i][0].transparentColor);
	}
	return true;
}

Common::String CombatEncounter::selectConfigName(uint difficulty) const {
	const Common::String requested = Common::String::format(
		_definition.iniPattern, difficulty);
	if (!_definition.fallbackIni || Common::File::exists(Common::Path(requested)))
		return requested;

	const Common::String fallback(_definition.fallbackIni);
	if (!Common::File::exists(Common::Path(fallback)))
		return requested;

	// RunCombatEncounterScene at 0x31563 always formats the configured level.
	// Some retail data layouts only contain ATKINI1.INI and RATINI1.INI, so
	// retain the original lookup first and use the packaged definition when its
	// numbered peer is absent.
	debugC(2, kDebugCombat,
		"Ripper: combat configuration fallback type='%s' requested='%s' selected='%s' reason=missing-numbered-config",
		_definition.name, requested.c_str(), fallback.c_str());
	return fallback;
}

bool CombatEncounter::loadResources(const Common::String &configName, uint difficulty) {
	Common::INIFile ini;
	if (!loadConfig(configName, ini))
		return false;

	_scenes.clear();
	for (uint i = 1; ; ++i) {
		const Common::String key = Common::String::format("scene%u", i);
		const Common::String basename = configString(ini, "SCENE", key.c_str());
		if (basename.empty())
			break;
		SceneData scene;
		scene.basename = basenameFromPath(basename);
		if (!loadSceneData(scene))
			return false;
		_scenes.push_back(Common::move(scene));
	}
	if (_scenes.empty()) {
		warning("Ripper: combat configuration '%s' has no scenes", configName.c_str());
		return false;
	}

	loadMeterConfig(ini, "HEALTH", _meterConfig[kHealthMeter]);
	loadMeterConfig(ini, "CREATURE", _meterConfig[kCreatureMeter]);
	loadMeterConfig(ini, "WEAPON", _meterConfig[kWeaponMeter]);
	loadMeterConfig(ini, "SHIELD", _meterConfig[kShieldMeter]);
	_weaponRapidFireTicks = configInt(ini, "WEAPON", "rapid fire", 0);
	_ambientSound = configString(ini, "SCENE", "sound", _definition.ambientSound);
	_creatureSound = configString(ini, "CREATURE", "sound", _definition.creatureSound);
	_weaponSound = configString(ini, "WEAPON", "sound", _definition.weaponSound);
	_shieldSound = configString(ini, "SHIELD", "sound", _definition.shieldSound);
	for (uint i = 0; i < kEffectGroupCount; ++i) {
		const Common::String key = Common::String::format("hit%u", i + 1);
		_creatureHitSounds[i] = configString(ini, "CREATURE", key.c_str());
	}
	for (uint i = 0; i < kDamageCount; ++i) {
		const Common::String key = Common::String::format("hit%u", i + 1);
		_shieldHitSounds[i] = configString(ini, "SHIELD", key.c_str());
		_healthHitSounds[i] = configString(ini, "HEALTH", key.c_str());
	}
	for (uint meter = 0; meter < kMeterCount; ++meter) {
		const MeterConfig &config = _meterConfig[meter];
		debugC(2, kDebugCombat,
			"Ripper: combat meter config name='%s' damage=%d,%d,%d,%d,%d recharge=%d/%u drain=%d/%u",
			meterName(meter), config.damage[0], config.damage[1], config.damage[2],
			config.damage[3], config.damage[4], config.charge, config.chargeTicks,
			config.discharge, config.dischargeTicks);
	}

	if (!loadBitmapAssets())
		return false;
	debugC(1, kDebugCombat,
		"Ripper: loaded combat encounter type='%s' config='%s' difficulty=%u scenes=%u",
		_definition.name, configName.c_str(), difficulty, _scenes.size());
	return true;
}

} // End of namespace Ripper
