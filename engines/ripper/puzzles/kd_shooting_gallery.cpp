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

#include "ripper/puzzles/kd_shooting_gallery.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
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
static const uint kDosTickMillis = 55;
static const uint kProgressIntervalFrames = 150;
static const uint16 kExitCommand = 1;
static const uint16 kFailureCommand = 2;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kScreenshotCommand = 0x1900;
static const uint kHelpSelectionTable = 0x1a7;

static const uint kWeaponCue = 4;
static const uint kClass0HitCue = 5;
static const uint kClass1HitCue = 6;
static const uint kFrame995Cue = 7;
static const uint kFrame1965Cue = 8;
static const uint kAmbientCue = 9;
static const uint kFrame118Cue = 10;
static const uint kFrame118SecondaryCue = 11;
static const uint kFrame1005Cue = 12;

static const uint kFrame118 = 0x76;
static const uint kFrame995 = 0x3e3;
static const uint kFrame1005 = 0x3ed;
static const uint kFrame1952 = 0x7a0;
static const uint kFrame1965 = 0x7ad;

static const int kGoalTextX = 0x3a;
static const int kScoreTextX = 0xc3;
static const int kScoreTextY = 0x28;
static const byte kScoreShadowColor = 0x14;
static const byte kScoreForegroundColor = 0x5e;
static const int kCheckmarkStartX = 0x6c;
static const int kClass0CheckmarkY = 0x8f;
static const int kClass1CheckmarkY = 0x7a;
static const int kFlameX = 0x6e;
static const int kFlameY = 0x84;

static const char *targetClassName(uint targetClass) {
	switch (targetClass) {
	case 0:
		return "reward";
	case 1:
		return "penalty";
	default:
		return "unknown";
	}
}

static const char *targetMaskName(byte mask) {
	switch (mask) {
	case 0:
		return "none";
	case 1:
		return "reward";
	case 2:
		return "penalty";
	case 3:
		return "mixed";
	default:
		return "unknown";
	}
}

static Common::String stripIniComment(Common::String value) {
	for (uint i = 0; i < value.size(); ++i) {
		if (value[i] == ';') {
			value = value.substr(0, i);
			break;
		}
	}
	value.trim();
	return value;
}

} // End of anonymous namespace

KdShootingGallery::Config::Config() : frameRate(0), goal(0),
		unusedPenalty(0), rapidFireTicks(0), weaponDischarge(0),
		weaponCharge(0), weaponChargeTicks(0) {
	for (uint i = 0; i < kTargetClassCount; ++i) {
		requiredHits[i] = 0;
		points[i] = 0;
	}
}

KdShootingGallery::KdShootingGallery(RipperEngine *engine) : Scene(engine),
		_activeTargetGroup(0), _targetGroupSerial(0),
		_lastHitTargetGroupSerial(-1), _hitsInCurrentTargetGroup(0),
		_weaponEnergyPercent(100), _visibleTargetMask(0),
		_targetGroupTypeMask(0), _keywordIndex(0), _completionFlag(0),
		_lastShotMillis(0), _lastRechargeMillis(0), _shotArmed(true),
		_result(kExited) {
	for (uint i = 0; i < kTargetClassCount; ++i) {
		_rawHits[i] = 0;
		_distinctTargetsHit[i] = 0;
		_completedTargets[i] = 0;
	}
}

int KdShootingGallery::configInt(const Common::INIFile &ini,
		const char *section, const char *key, int fallback) const {
	Common::String value;
	if (!ini.getKey(key, section, value))
		return fallback;
	value = stripIniComment(value);
	return value.empty() ? fallback : (int)value.asUint64();
}

bool KdShootingGallery::loadConfig(uint difficulty) {
	const Common::String name = Common::String::format("kd%u.ini", difficulty);
	Common::File file;
	Common::INIFile ini;
	if (!file.open(Common::Path(name))) {
		warning("Ripper: could not open KD shooting-gallery configuration '%s'",
			name.c_str());
		return false;
	}
	ini.requireKeyValueDelimiter();
	if (!ini.loadFromStream(file))
		return false;

	_config = Config();
	_config.frameRate = configInt(ini, "SCENE", "frame rate");
	_config.requiredHits[0] = configInt(ini, "SCORE", "shots1");
	_config.requiredHits[1] = configInt(ini, "SCORE", "shots2");
	_config.points[0] = configInt(ini, "SCORE", "points1");
	_config.points[1] = configInt(ini, "SCORE", "points2");
	_config.goal = configInt(ini, "SCORE", "goal");
	// RunKdShootingGalleryScene reads penalty but never uses it in the score.
	_config.unusedPenalty = configInt(ini, "SCORE", "penalty");
	_config.rapidFireTicks = configInt(ini, "WEAPON", "rapid fire");
	_config.weaponDischarge = configInt(ini, "WEAPON", "discharge");
	_config.weaponCharge = configInt(ini, "WEAPON", "charge");
	_config.weaponChargeTicks = configInt(ini, "WEAPON", "charge time");
	if (_config.frameRate < 0 ||
			_config.requiredHits[0] < 0 || _config.requiredHits[1] < 0 ||
			_config.points[0] < 0 || _config.points[1] < 0 ||
			_config.goal < 0 || _config.unusedPenalty < 0 ||
			_config.rapidFireTicks < 0 ||
			_config.weaponDischarge < 0 || _config.weaponCharge < 0 ||
			_config.weaponChargeTicks < 0) {
		warning("Ripper: KD shooting-gallery configuration '%s' contains negative values",
			name.c_str());
		return false;
	}

	debugC(2, kDebugPuzzles,
		"Ripper: KD shooting-gallery config file='%s' frameRate=%d shots=%d,%d "
		"points=%d,%d goal=%d penalty=%d rapidFire=%d discharge=%d charge=%d/%d",
		name.c_str(), _config.frameRate,
		_config.requiredHits[0], _config.requiredHits[1],
		_config.points[0], _config.points[1], _config.goal,
		_config.unusedPenalty,
		_config.rapidFireTicks, _config.weaponDischarge,
		_config.weaponCharge, _config.weaponChargeTicks);
	debugC(2, kDebugPuzzles,
		"Ripper: KD shooting-gallery scoring goal=%d reward=+%d per %s "
		"(hitsPerTarget=%d) penalty=-%d per %s (hitsPerTarget=%d)",
		_config.goal, _config.points[0],
		_config.requiredHits[0] == 0 ? "hit" : "completed-target",
		_config.requiredHits[0], _config.points[1],
		_config.requiredHits[1] == 0 ? "hit" : "completed-target",
		_config.requiredHits[1]);
	return true;
}

bool KdShootingGallery::loadFrameData() {
	Common::File regionFile;
	if (!regionFile.open(Common::Path("kd.dat")) ||
			!decodePresentationFrameRegionTable(regionFile, _regionTable)) {
		warning("Ripper: could not load KD shooting-gallery frame regions 'kd.dat'");
		return false;
	}

	Common::File audioFile;
	if (!audioFile.open(Common::Path("kd.prj")) ||
			!decodePresentationFrameAudioMap(
				audioFile, _regionTable.frames.size(), _audioMap)) {
		warning("Ripper: could not load KD shooting-gallery frame audio 'kd.prj'");
		return false;
	}

	uint regionCount = 0;
	for (uint frame = 0; frame < _regionTable.frames.size(); ++frame)
		regionCount += _regionTable.frames[frame].regions.size();
	debugC(2, kDebugPuzzles,
		"Ripper: loaded KD shooting-gallery frame data frames=%u regions=%u frameRate=%u sounds=%u",
		_regionTable.frames.size(), regionCount, _audioMap.frameRate,
		_audioMap.sounds.size());
	return true;
}

bool KdShootingGallery::loadVisualAssets() {
	ResourceManager *resources = _engine->getResources();
	BitmapAssetSequence cursor;
	BitmapAssetSequence checkmark;
	if (!resources->loadBitmapSequence("kd_mouse.bbm", cursor) ||
			cursor.frames.empty() ||
			!resources->loadBitmapSequence("kd_check.bbm", checkmark) ||
			checkmark.frames.empty() ||
			!resources->loadBitmapLibrary("kd_flame.pl", _flameFrames) ||
			!resources->loadInterfaceBitmapFont("small.fnt", _font))
		return false;

	_cursorFrame = Common::move(cursor.frames.front());
	_checkmarkFrame = Common::move(checkmark.frames.front());
	debugC(2, kDebugPuzzles,
		"Ripper: loaded KD shooting-gallery visual assets cursor=%ux%u checkmark=%ux%u flameFrames=%u fontGlyphs=%u",
		_cursorFrame.width, _cursorFrame.height,
		_checkmarkFrame.width, _checkmarkFrame.height,
		_flameFrames.size(), _font.glyphs.size());
	return true;
}

bool KdShootingGallery::loadResources(uint difficulty) {
	if (!loadConfig(difficulty) || !loadFrameData() || !loadVisualAssets())
		return false;

	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		const Common::String name = Common::String::format("kd%u.wav", cue);
		Common::File file;
		if (!file.open(Common::Path(name))) {
			warning("Ripper: could not open KD shooting-gallery cue '%s'", name.c_str());
			return false;
		}
	}
	return true;
}

Common::String KdShootingGallery::basenameFromPath(const Common::String &path) {
	uint start = 0;
	for (uint i = 0; i < path.size(); ++i) {
		if (path[i] == '/' || path[i] == '\\' || path[i] == ':')
			start = i + 1;
	}
	return path.substr(start);
}

Common::String KdShootingGallery::formatScore(int score) {
	const bool negative = score < 0;
	const uint value = negative ? (uint)(-(int64)score) : (uint)score;
	const Common::String digits = Common::String::format("%u", value);
	Common::String result;
	if (negative)
		result += '-';
	for (uint i = 0; i < digits.size(); ++i) {
		if (i != 0 && (digits.size() - i) % 3 == 0)
			result += ',';
		result += digits[i];
	}
	return result;
}

const PresentationRegion *KdShootingGallery::findHitRegion(
		const PresentationFrameRegion &frame, const Common::Point &point) const {
	for (uint i = 0; i < frame.regions.size(); ++i) {
		const PresentationRegion &region = frame.regions[i];
		// The KD.DAT values and the hit-test in RunKdShootingGalleryScene at
		// 0x3288e use the same vertical/horizontal record order as combat:
		// type, y, x, height, width.
		const Common::Rect bounds(region.coordinate2, region.coordinate1,
			region.coordinate2 + region.extent2,
			region.coordinate1 + region.extent1);
		if (region.extent1 > 0 && region.extent2 > 0 && bounds.contains(point))
			return &region;
	}
	return nullptr;
}

void KdShootingGallery::updateTargetGroup(
		const PresentationFrameRegion &frame, uint frameIndex) {
	if (frame.state == _activeTargetGroup)
		return;
	if (_activeTargetGroup != 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery target ended frame=%u group=%u serial=%d "
			"type=%s hits=%d",
			frameIndex, _activeTargetGroup, _targetGroupSerial,
			targetMaskName(_targetGroupTypeMask), _hitsInCurrentTargetGroup);
		logProgress(frameIndex, "target-ended");
	}
	if (frame.state != 0)
		++_targetGroupSerial;
	if (frame.state != 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery target starting frame=%u group=%u serial=%d",
			frameIndex, frame.state, _targetGroupSerial);
	}
	_activeTargetGroup = frame.state;
	_hitsInCurrentTargetGroup = 0;
	_visibleTargetMask = 0;
	_targetGroupTypeMask = 0;
}

void KdShootingGallery::updateTargetVisibility(
		const PresentationFrameRegion &frame, uint frameIndex) {
	uint regionCounts[kTargetClassCount] = { 0, 0 };
	uint unknownRegions = 0;
	byte targetMask = 0;
	for (uint i = 0; i < frame.regions.size(); ++i) {
		const PresentationRegion &region = frame.regions[i];
		if (region.extent1 <= 0 || region.extent2 <= 0)
			continue;
		if (region.type < kTargetClassCount) {
			++regionCounts[region.type];
			targetMask |= 1 << region.type;
		} else {
			++unknownRegions;
			targetMask |= 4;
		}
	}
	if (targetMask == _visibleTargetMask)
		return;

	if (targetMask == 0) {
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery target hidden frame=%u group=%u "
			"serial=%d previousType=%s",
			frameIndex, _activeTargetGroup, _targetGroupSerial,
			targetMaskName(_visibleTargetMask));
	} else {
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery target visible frame=%u group=%u "
			"serial=%d type=%s regions[reward=%u penalty=%u unknown=%u] "
			"value[reward=+%d penalty=-%d]",
			frameIndex, _activeTargetGroup, _targetGroupSerial,
			targetMaskName(targetMask), regionCounts[0], regionCounts[1],
			unknownRegions, _config.points[0], _config.points[1]);
	}
	_visibleTargetMask = targetMask;
	_targetGroupTypeMask |= targetMask;
}

void KdShootingGallery::logProgress(uint frameIndex, const char *reason) const {
	const int score = currentScore();
	const int margin = score - _config.goal;
	const uint totalFrames = _regionTable.frames.size();
	const uint percent = totalFrames == 0 ? 0 :
		MIN<uint>(100, (frameIndex + 1) * 100 / totalFrames);
	const int needed = rewardUnitsNeeded();
	const char *rewardUnit =
		_config.requiredHits[0] == 0 ? "rewardHits" : "rewardTargets";

	debugC(2, kDebugPuzzles,
		"Ripper: KD shooting-gallery progress reason=%s frame=%u/%u (%u%%) "
		"standing=%s score=%d goal=%d margin=%+d need[%s=%d] "
		"reward[raw=%d distinct=%d completed=%d scoring=%d value=%d] "
		"penalty[raw=%d distinct=%d completed=%d scoring=%d value=-%d]",
		reason, frameIndex + 1, totalFrames, percent,
		margin >= 0 ? "WINNING" : "LOSING", score, _config.goal, margin,
		rewardUnit, needed,
		_rawHits[0], _distinctTargetsHit[0], _completedTargets[0],
		scoringTargetCount(0), scoreForClass(0),
		_rawHits[1], _distinctTargetsHit[1], _completedTargets[1],
		scoringTargetCount(1), scoreForClass(1));
}

void KdShootingGallery::playCue(uint index, const Common::String &path,
		uint volumePercent, bool loop) {
	if (index >= kAudioCueCount || path.empty())
		return;
	if (!_engine->getMedia()->playSoundEffect(
			basenameFromPath(path), _audioHandles[index], volumePercent, loop)) {
		warning("Ripper: could not play KD shooting-gallery cue index=%u path='%s'",
			index, path.c_str());
	}
}

void KdShootingGallery::playNumberedCue(uint index,
		uint volumePercent, bool loop) {
	playCue(index, Common::String::format("kd%u.wav", index),
		volumePercent, loop);
}

void KdShootingGallery::stopCue(uint index) {
	if (index < kAudioCueCount)
		_engine->getMedia()->stopSoundEffect(_audioHandles[index]);
}

void KdShootingGallery::stopAllAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		stopCue(cue);
}

void KdShootingGallery::serviceFrameCue(uint frameIndex) {
	if (frameIndex >= _audioMap.cues.size())
		return;
	const PresentationFrameAudioCue &cue = _audioMap.cues[frameIndex];
	if (cue.soundIndex >= 0 && (uint)cue.soundIndex < _audioMap.sounds.size() &&
			(uint)cue.soundIndex < kAudioCueCount) {
		playCue(cue.soundIndex, _audioMap.sounds[cue.soundIndex], cue.volume);
	}
}

void KdShootingGallery::serviceFixedCue(uint frameIndex) {
	if (frameIndex == kFrame118) {
		playNumberedCue(kFrame118Cue);
		playNumberedCue(kFrame118SecondaryCue);
	} else if (frameIndex == kFrame1005) {
		playNumberedCue(kFrame1005Cue);
	} else if (frameIndex == kFrame995) {
		playNumberedCue(kFrame995Cue);
		_engine->getMedia()->setSoundEffectVolume(
			_audioHandles[kAmbientCue], 50);
	} else if (frameIndex == kFrame1952) {
		stopCue(kFrame118Cue);
		stopCue(kFrame995Cue);
		stopCue(kAmbientCue);
	} else if (frameIndex == kFrame1965) {
		playNumberedCue(kFrame1965Cue);
	}
}

uint16 KdShootingGallery::serviceKeyboard(uint frameIndex) {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == 0x1b) {
			_result = kExited;
			debugC(1, kDebugPuzzles,
				"Ripper: KD shooting-gallery exited by Escape frame=%u score=%d",
				frameIndex, currentScore());
			logProgress(frameIndex, "escape");
			return kExitCommand;
		}
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: KD shooting-gallery opening help table=0x%x frame=%u",
				kHelpSelectionTable, frameIndex);
			if (!_engine->getModalDialog()->run(kHelpSelectionTable, true,
					ModalDialogManager::kMenubPresentation,
					ModalDialogManager::kPreserveActivePalette)) {
				_result = kLoadFailed;
				return kFailureCommand;
			}
			applyCursor();
			return MediaSequenceCallback::kContinueRefreshPalette;
		}
		if (command == kScreenshotCommand) {
			g_system->saveScreenshot();
			continue;
		}
		if (command > 0xff)
			continue;

		byte character = command;
		if (character >= 'A' && character <= 'Z')
			character += 'a' - 'A';
		static const char keyword[] = "paradise";
		if (character == keyword[_keywordIndex])
			++_keywordIndex;
		else
			_keywordIndex = 0;
		if (_keywordIndex == sizeof(keyword) - 1) {
			_result = kSolved;
			debugC(1, kDebugPuzzles,
				"Ripper: KD shooting-gallery completed by paradise keyword frame=%u",
				frameIndex);
			logProgress(frameIndex, "keyword");
			return kExitCommand;
		}
	}
	return 0;
}

void KdShootingGallery::serviceWeapon(const PresentationFrameRegion &frame,
		const Common::Point &point, bool firing, uint32 now, uint frameIndex) {
	if (!firing) {
		_shotArmed = true;
		if (_config.weaponCharge > 0 && _weaponEnergyPercent < 100) {
			const uint32 interval = _config.weaponChargeTicks * kDosTickMillis;
			if (interval == 0 || now - _lastRechargeMillis >= interval) {
				_weaponEnergyPercent = MIN<int>(100,
					_weaponEnergyPercent + _config.weaponCharge);
				_lastRechargeMillis = now;
			}
		}
		return;
	}

	const uint32 interval = _config.rapidFireTicks * kDosTickMillis;
	if (_weaponEnergyPercent == 0 || !_shotArmed ||
			now - _lastShotMillis < interval)
		return;

	playNumberedCue(kWeaponCue);
	const int energyBefore = _weaponEnergyPercent;
	const int scoreBefore = currentScore();
	_weaponEnergyPercent = MAX<int>(0,
		_weaponEnergyPercent - _config.weaponDischarge);
	const PresentationRegion *region = findHitRegion(frame, point);
	int targetClass = -1;
	if (region && region->type < kTargetClassCount) {
		targetClass = region->type;
		playNumberedCue(targetClass == 0 ? kClass0HitCue : kClass1HitCue,
			targetClass == 0 ? 100 : 50);
		++_rawHits[targetClass];
		++_hitsInCurrentTargetGroup;
		if (_hitsInCurrentTargetGroup == _config.requiredHits[targetClass])
			++_completedTargets[targetClass];
		if (_lastHitTargetGroupSerial != _targetGroupSerial) {
			++_distinctTargetsHit[targetClass];
			_lastHitTargetGroupSerial = _targetGroupSerial;
		}
	}
	_lastShotMillis = now;
	if (_config.rapidFireTicks == 0)
		_shotArmed = false;
	if (targetClass >= 0) {
		const int requiredHits = _config.requiredHits[targetClass];
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery target hit frame=%u type=%s class=%d "
			"scoreDelta=%+d scoring=%s completion=%d/%d group=%u serial=%d "
			"energy=%d->%d",
			frameIndex, targetClassName(targetClass), targetClass,
			currentScore() - scoreBefore,
			requiredHits == 0 ? "per-hit" : "on-completion",
			requiredHits == 0 ? 1 : MIN(_hitsInCurrentTargetGroup, requiredHits),
			requiredHits == 0 ? 1 : requiredHits,
			_activeTargetGroup, _targetGroupSerial,
			energyBefore, _weaponEnergyPercent);
		logProgress(frameIndex, targetClass == 0 ? "reward-hit" : "penalty-hit");
	} else {
		debugC(3, kDebugPuzzles,
			"Ripper: KD shooting-gallery shot missed frame=%u point=%d,%d "
			"group=%u serial=%d energy=%d->%d",
			frameIndex, point.x, point.y, _activeTargetGroup,
			_targetGroupSerial, energyBefore, _weaponEnergyPercent);
	}
}

int KdShootingGallery::scoringTargetCount(uint targetClass) const {
	if (targetClass >= kTargetClassCount)
		return 0;
	return _config.requiredHits[targetClass] == 0 ?
		_rawHits[targetClass] : _completedTargets[targetClass];
}

int KdShootingGallery::scoreForClass(uint targetClass) const {
	if (targetClass >= kTargetClassCount)
		return 0;
	return scoringTargetCount(targetClass) * _config.points[targetClass];
}

int KdShootingGallery::currentScore() const {
	return scoreForClass(0) - scoreForClass(1);
}

int KdShootingGallery::rewardUnitsNeeded() const {
	const int scoreGap = MAX<int>(0, _config.goal - currentScore());
	if (scoreGap == 0)
		return 0;
	if (_config.points[0] <= 0)
		return -1;
	return (scoreGap + _config.points[0] - 1) / _config.points[0];
}

uint KdShootingGallery::displayedTargetCount(uint targetClass) const {
	if (targetClass >= kTargetClassCount)
		return 0;
	return _config.requiredHits[targetClass] == 0 ?
		_distinctTargetsHit[targetClass] : _completedTargets[targetClass];
}

void KdShootingGallery::applyCursor() {
	_engine->getCursor()->applyCustomCursor(_cursorFrame,
		_cursorFrame.width / 2, _cursorFrame.height / 2, kDisplayScale);
	_engine->getCursor()->setVisible(true);
}

uint16 KdShootingGallery::service(uint frame) {
	if (_regionTable.frames.empty())
		return kFailureCommand;
	const uint frameIndex = MIN<uint>(
		frame == 0 ? 0 : frame - 1, _regionTable.frames.size() - 1);
	const PresentationFrameRegion &frameState = _regionTable.frames[frameIndex];

	serviceFrameCue(frameIndex);
	updateTargetGroup(frameState, frameIndex);
	updateTargetVisibility(frameState, frameIndex);
	const uint16 keyboardCommand = serviceKeyboard(frameIndex);
	if (keyboardCommand != 0)
		return keyboardCommand;

	const MouseState &mouse = _engine->getInput()->peekMouseState();
	const Common::Point point(
		CLIP<int>(mouse.position.x / (int)kDisplayScale, 0,
			(int)kLogicalWidth - 1),
		CLIP<int>(mouse.position.y / (int)kDisplayScale, 0,
			(int)kLogicalHeight - 1));
	serviceWeapon(frameState, point,
		(mouse.buttons & kMouseButtonLeft) != 0,
		g_system->getMillis(true), frameIndex);
	if (frameIndex == 0 ||
			(frameIndex + 1) % kProgressIntervalFrames == 0)
		logProgress(frameIndex, "checkpoint");
	serviceFixedCue(frameIndex);
	_engine->getCursor()->setVisible(true);
	return 0;
}

void KdShootingGallery::drawBitmapScaled(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const {
	for (uint sourceY = 0; sourceY < frame.height; ++sourceY) {
		for (uint sourceX = 0; sourceX < frame.width; ++sourceX) {
			const byte pixel = frame.pixels[sourceY * frame.width + sourceX];
			if (pixel == frame.transparentColor)
				continue;
			const int targetX = (x + sourceX) * kDisplayScale;
			const int targetY = (y + sourceY) * kDisplayScale;
			for (uint scaleY = 0; scaleY < kDisplayScale; ++scaleY) {
				if (targetY + (int)scaleY < 0 ||
						targetY + (int)scaleY >= g_system->getHeight())
					continue;
				for (uint scaleX = 0; scaleX < kDisplayScale; ++scaleX) {
					if (targetX + (int)scaleX >= 0 &&
							targetX + (int)scaleX < g_system->getWidth()) {
						screen[(targetY + scaleY) * pitch + targetX + scaleX] = pixel;
					}
				}
			}
		}
	}
}

void KdShootingGallery::drawTextScaled(byte *screen, uint pitch,
		int x, int y, const Common::String &text, byte color) const {
	int drawX = x;
	for (uint characterIndex = 0; characterIndex < text.size(); ++characterIndex) {
		const byte character = text[characterIndex];
		if (character == ' ') {
			drawX += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
				character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph =
			_font.glyphs[character - _font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte source = _font.pixels[
					glyph.pixelOffset + glyphY * glyph.width + glyphX];
				if (source == _font.transparentColor)
					continue;
				const int targetX =
					(drawX + glyph.xOffset + glyphX) * kDisplayScale;
				const int targetY =
					(y + glyph.yOffset + glyphY) * kDisplayScale;
				for (uint scaleY = 0; scaleY < kDisplayScale; ++scaleY) {
					if (targetY + (int)scaleY < 0 ||
							targetY + (int)scaleY >= g_system->getHeight())
						continue;
					for (uint scaleX = 0; scaleX < kDisplayScale; ++scaleX) {
						if (targetX + (int)scaleX >= 0 &&
								targetX + (int)scaleX < g_system->getWidth()) {
							screen[(targetY + scaleY) * pitch +
								targetX + scaleX] = color;
						}
					}
				}
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

bool KdShootingGallery::drawResults() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	const Common::String goal = formatScore(_config.goal);
	const Common::String score = formatScore(currentScore());
	drawTextScaled((byte *)screen->getPixels(), screen->pitch,
		kGoalTextX + 1, kScoreTextY + 1, goal, kScoreShadowColor);
	drawTextScaled((byte *)screen->getPixels(), screen->pitch,
		kGoalTextX, kScoreTextY, goal, kScoreForegroundColor);
	drawTextScaled((byte *)screen->getPixels(), screen->pitch,
		kScoreTextX + 1, kScoreTextY + 1, score, kScoreShadowColor);
	drawTextScaled((byte *)screen->getPixels(), screen->pitch,
		kScoreTextX, kScoreTextY, score, kScoreForegroundColor);

	for (uint targetClass = 0; targetClass < kTargetClassCount; ++targetClass) {
		const int y = targetClass == 0 ?
			kClass0CheckmarkY : kClass1CheckmarkY;
		int x = kCheckmarkStartX;
		const uint count = displayedTargetCount(targetClass);
		for (uint checkmark = 0; checkmark < count; ++checkmark) {
			drawBitmapScaled((byte *)screen->getPixels(), screen->pitch,
				_checkmarkFrame, x, y);
			x += _checkmarkFrame.width + 1;
		}
	}
	g_system->unlockScreen();
	presentScreen();
	debugC(2, kDebugPuzzles,
		"Ripper: KD shooting-gallery results goal=%d score=%d raw=%d,%d distinct=%d,%d completed=%d,%d checks=%u,%u",
		_config.goal, currentScore(), _rawHits[0], _rawHits[1],
		_distinctTargetsHit[0], _distinctTargetsHit[1],
		_completedTargets[0], _completedTargets[1],
		displayedTargetCount(0), displayedTargetCount(1));
	debugC(3, kDebugPuzzles,
		"Ripper: KD shooting-gallery result layout checks[startX=%d rewardY=%d penaltyY=%d advance=%u] flame=%d,%d",
		kCheckmarkStartX, kClass0CheckmarkY, kClass1CheckmarkY,
		_checkmarkFrame.width + 1, kFlameX, kFlameY);
	return true;
}

bool KdShootingGallery::animateResultsUntilInput() {
	uint maxWidth = 0;
	uint maxHeight = 0;
	for (uint frame = 0; frame < _flameFrames.size(); ++frame) {
		maxWidth = MAX<uint>(maxWidth, _flameFrames[frame].width);
		maxHeight = MAX<uint>(maxHeight, _flameFrames[frame].height);
	}
	IndexedDisplaySnapshot flameBacking;
	if (maxWidth != 0 && maxHeight != 0) {
		const Common::Rect bounds(
			kFlameX * kDisplayScale, kFlameY * kDisplayScale,
			MIN<int>((kFlameX + maxWidth) * kDisplayScale,
				g_system->getWidth()),
			MIN<int>((kFlameY + maxHeight) * kDisplayScale,
				g_system->getHeight()));
		if (!flameBacking.capture(bounds))
			return false;
	}

	uint flameFrame = 0;
	uint32 lastFlameMillis = 0;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	while (!_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		if (_engine->getInput()->hasPendingKey()) {
			_engine->getInput()->drainKeys();
			break;
		}
		const MouseState mouse = _engine->getInput()->publishMouseState();
		if ((mouse.pressed & (kMouseButtonLeft | kMouseButtonRight)) != 0)
			break;

		const uint32 now = g_system->getMillis(true);
		if (!_flameFrames.empty() &&
				(lastFlameMillis == 0 ||
					now - lastFlameMillis >= 2 * kDosTickMillis)) {
			if (flameBacking.isValid())
				flameBacking.restorePixels();
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen || screen->format.bytesPerPixel != 1) {
				if (screen)
					g_system->unlockScreen();
				return false;
			}
			drawBitmapScaled((byte *)screen->getPixels(), screen->pitch,
				_flameFrames[flameFrame], kFlameX, kFlameY);
			g_system->unlockScreen();
			presentScreen();
			flameFrame = (flameFrame + 1) % _flameFrames.size();
			lastFlameMillis = now;
		}
		_engine->getCursor()->setVisible(true);
		g_system->delayMillis(10);
	}
	return true;
}

bool KdShootingGallery::presentResults() {
	logProgress(_regionTable.frames.size() - 1, "final-score");
	if (!drawResults() || !animateResultsUntilInput())
		return false;
	if (!_engine->shouldQuit() && currentScore() >= _config.goal)
		_result = kSolved;
	debugC(1, kDebugPuzzles,
		"Ripper: KD shooting-gallery result dismissed result=%d goal=%d score=%d",
		_result, _config.goal, currentScore());
	return true;
}

Scene::Result KdShootingGallery::run(uint completionFlag) {
	prepare("kd-shooting-gallery-entry", 19, false);
	_completionFlag = completionFlag;
	_result = kExited;
	_config = Config();
	_regionTable.frames.clear();
	_audioMap = PresentationFrameAudioMap();
	_flameFrames.clear();
	_activeTargetGroup = 0;
	_targetGroupSerial = 0;
	_lastHitTargetGroupSerial = -1;
	_hitsInCurrentTargetGroup = 0;
	_weaponEnergyPercent = 100;
	_visibleTargetMask = 0;
	_targetGroupTypeMask = 0;
	_keywordIndex = 0;
	_shotArmed = true;
	for (uint i = 0; i < kTargetClassCount; ++i) {
		_rawHits[i] = 0;
		_distinctTargetsHit[i] = 0;
		_completedTargets[i] = 0;
	}

	const uint difficulty = _engine->getSettings()->getCombatLevel();
	debugC(1, kDebugPuzzles,
		"Ripper: entering KD shooting-gallery difficulty=%u config='kd%u.ini' completionFlag=%u",
		difficulty, difficulty, completionFlag);
	if (!loadResources(difficulty)) {
		_result = kLoadFailed;
	} else {
		const uint32 now = g_system->getMillis(true);
		_lastShotMillis = now;
		_lastRechargeMillis = now;
		applyCursor();
		playNumberedCue(kAmbientCue, 100, true);
		uint16 command = 0;
		if (!_engine->getMedia()->playShootingGallerySequence(
				"kd.smk", this, &command)) {
			_result = kLoadFailed;
		} else if (command == kFailureCommand) {
			_result = kLoadFailed;
		} else if (command == 0 && !_engine->shouldQuit() &&
				!presentResults()) {
			_result = kLoadFailed;
		}
		debugC(2, kDebugPuzzles,
			"Ripper: KD shooting-gallery media returned command=0x%04x result=%d score=%d energy=%d",
			command, _result, currentScore(), _weaponEnergyPercent);
	}

	stopAllAudio();
	if (_result == kSolved &&
			!_engine->getMilestones()->set(
				completionFlag, true, "kd-shooting-gallery"))
		_result = kLoadFailed;
	finish("kd-shooting-gallery-exit", 0, true);
	debugC(_result == kLoadFailed ? 1 : 2, kDebugPuzzles,
		"Ripper: left KD shooting-gallery result=%d difficulty=%u completionFlag=%u score=%d",
		_result, difficulty, completionFlag, currentScore());
	return _result;
}

} // End of namespace Ripper
