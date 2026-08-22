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

#ifndef RIPPER_COMBAT_COMBAT_H
#define RIPPER_COMBAT_COMBAT_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/media.h"
#include "ripper/resources.h"
#include "ripper/scenes/scene.h"

namespace Common {
class INIFile;
}

namespace Ripper {

struct CombatEncounterDefinition {
	const char *name;
	const char *iniPattern;
	const char *fallbackIni;
	const char *ambientSound;
	const char *ambientAlternates[2];
	const char *creatureSound;
	const char *weaponSound;
	const char *shieldSound;
	const char *effectLibraries[2];
	const char *crosshairPrefix;
	uint helpResource;
	bool atkiniSceneRouting;
};

class CombatEncounter : public Scene, public MediaSequenceCallback {
public:
	explicit CombatEncounter(RipperEngine *engine,
		const CombatEncounterDefinition &definition);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }
	bool managesPalette() const override { return true; }
	void transformPalette(byte *palette, uint colorCount) const override;

private:
	static const uint kMeterCount = 4;
	static const uint kDamageCount = 5;
	static const uint kEffectGroupCount = 2;
	static const uint kEffectCapacity = 10;
	static const uint kCueHandleCount = 8;

	enum MeterIndex {
		kHealthMeter,
		kCreatureMeter,
		kWeaponMeter,
		kShieldMeter
	};

	enum PaletteEffect {
		kPaletteNormal,
		kPaletteShield,
		kPaletteTargetHit,
		kPalettePlayerHit
	};

	struct MeterConfig {
		int damage[kDamageCount];
		int charge;
		int discharge;
		uint chargeTicks;
		uint dischargeTicks;

		MeterConfig();
	};

	struct HitRegion {
		byte type;
		Common::Rect bounds;
	};

	struct FrameState {
		byte attack;
		byte creatureVolume;
		Common::Array<HitRegion> hitRegions;
	};

	struct FrameCue {
		int soundIndex;
		byte volume;

		FrameCue() : soundIndex(-1), volume(0) {}
	};

	struct SceneData {
		Common::String basename;
		Common::Array<FrameState> frames;
		Common::Array<Common::String> cueSounds;
		Common::Array<FrameCue> frameCues;
	};

	struct Effect {
		bool active;
		byte group;
		uint frame;
		int x;
		int y;

		Effect() : active(false), group(0), frame(0), x(0), y(0) {}
	};

	Common::String selectConfigName(uint difficulty) const;
	bool loadResources(const Common::String &configName, uint difficulty);
	bool loadConfig(const Common::String &name, Common::INIFile &ini);
	bool loadSceneData(SceneData &scene);
	bool loadFrameRegions(const Common::String &name, SceneData &scene);
	bool loadFrameCues(const Common::String &name, SceneData &scene);
	bool loadBitmapAssets();
	void loadMeterConfig(const Common::INIFile &ini, const char *section,
		MeterConfig &config);
	Common::String configString(const Common::INIFile &ini, const char *section,
		const char *key, const Common::String &fallback = Common::String()) const;
	int configInt(const Common::INIFile &ini, const char *section,
		const char *key, int fallback = 0) const;
	static Common::String basenameFromPath(const Common::String &path);
	static Common::String normalizeCueName(const Common::String &path);
	static const char *meterName(uint meter);

	void startEncounterAudio();
	void queueAlternateAmbient();
	void stopEncounterAudio();
	void queueCue(const Common::String &path, uint volumePercent = 100);
	void updateContinuousAudio(const FrameState &frame);
	void updateMeter(uint meter, uint32 now, bool recharge, bool drain);
	void applyIncomingAttack(byte attack, bool shieldHeld, uint32 now,
		uint frameIndex);
	void serviceWeapon(const FrameState &frame, const Common::Point &point,
		bool weaponHeld, uint32 now, uint frameIndex);
	const HitRegion *findHitRegion(const FrameState &frame,
		const Common::Point &point) const;
	void spawnEffect(byte group, const Common::Point &point);
	void advanceEffects();
	void drawOverlay(const Common::Point &point, bool targetActive,
		bool enemyAttackActive);
	void drawFrameScaled(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void drawMeters(byte *screen, uint pitch, int panelX) const;
	void drawIndicators(byte *screen, uint pitch, int panelX,
		bool enemyAttackActive) const;
	void drawCrosshair(byte *screen, uint pitch, const Common::Point &point) const;
	void drawEffects(byte *screen, uint pitch) const;
	uint16 serviceKeyboard();
	uint chooseNextScene();

	const CombatEncounterDefinition &_definition;
	Common::RandomSource _random;
	Common::Array<SceneData> _scenes;
	Common::Array<BitmapAssetFrame> _combatFrames;
	Common::Array<BitmapAssetFrame> _crosshairFrames;
	Common::Array<BitmapAssetFrame> _effectFrames[kEffectGroupCount];
	MeterConfig _meterConfig[kMeterCount];
	int _meterPercent[kMeterCount];
	uint32 _lastChargeMillis[kMeterCount];
	uint32 _lastDischargeMillis[kMeterCount];
	Common::String _ambientSound;
	Common::String _creatureSound;
	Common::String _weaponSound;
	Common::String _shieldSound;
	Common::String _creatureHitSounds[kEffectGroupCount];
	Common::String _shieldHitSounds[kDamageCount];
	Common::String _healthHitSounds[kDamageCount];
	Audio::SoundHandle _ambientHandle;
	Audio::SoundHandle _creatureHandle;
	Audio::SoundHandle _shieldHandle;
	Audio::SoundHandle _cueHandles[kCueHandleCount];
	uint _nextCueHandle;
	uint _weaponRapidFireTicks;
	Effect _effects[kEffectCapacity];
	const SceneData *_activeScene;
	uint _activeSceneIndex;
	uint _completionFlag;
	uint _arcadeKeywordIndex;
	uint32 _lastWeaponShotMillis;
	bool _weaponHeld;
	bool _singleShotReady;
	bool _shieldHeld;
	bool _lastShotHit;
	bool _overlayLogged;
	PaletteEffect _paletteEffect;
	Result _encounterResult;
};

} // End of namespace Ripper

#endif // RIPPER_COMBAT_COMBAT_H
