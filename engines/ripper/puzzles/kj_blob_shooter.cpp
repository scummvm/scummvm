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

#include "ripper/puzzles/kj_blob_shooter.h"

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
static const uint kInitialSpawnLeadTicks = 0x48;
static const uint kInitialAmbientLeadTicks = 0x18c;
static const uint kAmbientIntervalTicks = 0x10e;
static const uint kVictoryCueDelayTicks = 0x24;
static const uint kAutoHitDelayTicks = 0x90;
static const uint kVictoryExitDelayTicks = 0x36;
static const uint kMinimumSpawnDelayTicks = 2;
static const uint kProgressInterval = 150;
static const uint16 kExitCommand = 1;
static const uint16 kFailureCommand = 2;
static const uint16 kHelpCommand = 0x3b00;
static const uint16 kScreenshotCommand = 0x1900;
static const uint kHelpSelectionTable = 0x1a8;
static const byte kBlobTransparentColor = 4;

// RunBlobShooterScene at 0x338a4 copies the vertical origins from 0x31383
// and the horizontal origins from 0x31397. Its original selection-point
// globals use vertical/horizontal ordering, so expose them here as x/y.
static const int kBlobX[5] = { 185, 0, 0, 0, 210 };
static const int kBlobY[5] = { 0, 0, 62, 0, 0 };
static const int kScriptedHitX[5] = { 239, 75, 36, 164, 267 };
static const int kScriptedHitY[5] = { 48, 68, 104, 46, 113 };

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

static uint32 ticksToMillis(int ticks) {
	return (uint32)MAX<int>(0, ticks) * kDosTickMillis;
}

} // End of anonymous namespace

KjBlobShooter::Config::Config() :
		frameRate(15), startWhooshFrame(8), spawnDelayTicks(54),
		spawnDecreaseDelayTicks(180), spawnDecreaseTicks(4),
		missesAllowed(100), shotsPerBlob(1), winRateTicks(4),
		rapidFireTicks(2), weaponDischarge(0), weaponCharge(0),
		weaponChargeTicks(0) {
}

KjBlobShooter::KjBlobShooter(RipperEngine *engine) :
		Scene(engine), _random("ripper-kj-blob-shooter"),
		_completionFlag(0), _keywordIndex(0), _presentationTicks(0),
		_phase(0), _ambientCue(7), _spawnDelayTicks(0),
		_missedBlobs(0), _destroyedBlobs(0), _weaponEnergyPercent(100),
		_nextSpawnMillis(0), _nextSpawnDecreaseMillis(0),
		_nextAmbientMillis(0), _phaseStartMillis(0), _lastShotMillis(0),
		_lastRechargeMillis(0), _shotArmed(true),
		_scriptedAutoHit(false), _result(kExited) {
}

int KjBlobShooter::configInt(const Common::INIFile &ini,
		const char *section, const char *key, int fallback) const {
	Common::String value;
	if (!ini.getKey(key, section, value))
		return fallback;
	value = stripIniComment(value);
	return value.empty() ? fallback : (int)value.asUint64();
}

bool KjBlobShooter::loadConfig(uint difficulty) {
	Common::String name = Common::String::format("kj%u.ini", difficulty);
	Common::File file;
	Common::INIFile ini;
	if (!file.open(Common::Path(name))) {
		// Some retail installations contain the shared KJ1 tuning file even
		// when the Options Panel retains another combat level.
		const Common::String fallbackName("kj1.ini");
		if (difficulty == 1 || !file.open(Common::Path(fallbackName))) {
			warning("Ripper: could not open KJ blob-shooter configuration '%s'",
				name.c_str());
			return false;
		}
		debugC(1, kDebugPuzzles,
			"Ripper: KJ blob-shooter configuration '%s' unavailable; "
			"using retail fallback '%s'",
			name.c_str(), fallbackName.c_str());
		name = fallbackName;
	}
	ini.requireKeyValueDelimiter();
	if (!ini.loadFromStream(file))
		return false;

	_config = Config();
	_config.frameRate = configInt(ini, "SCENE", "frame rate", _config.frameRate);
	_config.startWhooshFrame =
		configInt(ini, "SCENE", "start whoosh", _config.startWhooshFrame);
	_config.spawnDelayTicks =
		configInt(ini, "SCORE", "spawn delay", _config.spawnDelayTicks);
	_config.spawnDecreaseDelayTicks =
		configInt(ini, "SCORE", "spawn dec delay", _config.spawnDecreaseDelayTicks);
	_config.spawnDecreaseTicks =
		configInt(ini, "SCORE", "spawn dec", _config.spawnDecreaseTicks);
	_config.missesAllowed =
		configInt(ini, "SCORE", "misses allowed", _config.missesAllowed);
	_config.shotsPerBlob =
		configInt(ini, "SCORE", "shots", _config.shotsPerBlob);
	_config.winRateTicks =
		configInt(ini, "SCORE", "win rate", _config.winRateTicks);
	_config.rapidFireTicks =
		configInt(ini, "WEAPON", "rapid fire", _config.rapidFireTicks);
	_config.weaponDischarge =
		configInt(ini, "WEAPON", "discharge", _config.weaponDischarge);
	_config.weaponCharge =
		configInt(ini, "WEAPON", "charge", _config.weaponCharge);
	_config.weaponChargeTicks =
		configInt(ini, "WEAPON", "charge time", _config.weaponChargeTicks);

	if (_config.frameRate <= 0 || _config.startWhooshFrame < 0 ||
			_config.spawnDelayTicks < 0 ||
			_config.spawnDecreaseDelayTicks < 0 ||
			_config.spawnDecreaseTicks < 0 ||
			_config.missesAllowed < 0 || _config.shotsPerBlob < 0 ||
			_config.winRateTicks < 0 || _config.rapidFireTicks < 0 ||
			_config.weaponDischarge < 0 || _config.weaponCharge < 0 ||
			_config.weaponChargeTicks < 0) {
		warning("Ripper: KJ blob-shooter configuration '%s' contains invalid values",
			name.c_str());
		return false;
	}

	debugC(2, kDebugPuzzles,
		"Ripper: KJ blob-shooter config file='%s' frameRate=%d "
		"spawn[delay=%d decreaseDelay=%d decrease=%d winRate=%d] "
		"missesAllowed=%d shots=%d startWhoosh=%d "
		"weapon[rapidFire=%d discharge=%d charge=%d/%d]",
		name.c_str(), _config.frameRate, _config.spawnDelayTicks,
		_config.spawnDecreaseDelayTicks, _config.spawnDecreaseTicks,
		_config.winRateTicks, _config.missesAllowed, _config.shotsPerBlob,
		_config.startWhooshFrame, _config.rapidFireTicks,
		_config.weaponDischarge, _config.weaponCharge,
		_config.weaponChargeTicks);
	return true;
}

bool KjBlobShooter::loadVisualAssets() {
	ResourceManager *resources = _engine->getResources();
	BitmapAssetSequence cursor;
	if (!resources->loadBitmapSequence("kj_mouse.bbm", cursor) ||
			cursor.frames.empty())
		return false;
	_cursorFrame = Common::move(cursor.frames.front());

	for (uint type = 0; type < kBlobTypeCount; ++type) {
		if (!resources->loadBitmapLibrary(
				Common::String::format("kj_blob%u.pl", type),
				_blobFrames[type]) ||
				_blobFrames[type].empty())
			return false;
		for (uint frame = 0; frame < _blobFrames[type].size(); ++frame)
			_blobFrames[type][frame].transparentColor = kBlobTransparentColor;
		debugC(2, kDebugPuzzles,
			"Ripper: loaded KJ blob type=%u frames=%u origin=%d,%d "
			"size=%ux%u scriptedHit=%d,%d",
			type, _blobFrames[type].size(), kBlobX[type], kBlobY[type],
			_blobFrames[type][0].width, _blobFrames[type][0].height,
			kScriptedHitX[type], kScriptedHitY[type]);
	}
	if (!resources->loadBitmapLibrary("kj_exp1.pl", _explosionFrames) ||
			_explosionFrames.empty() ||
			!resources->loadBitmapLibrary("kj_fire.pl", _fireFrames) ||
			_fireFrames.empty())
		return false;

	debugC(2, kDebugPuzzles,
		"Ripper: loaded KJ blob-shooter visual assets cursor=%ux%u "
		"explosionFrames=%u fireFrames=%u",
		_cursorFrame.width, _cursorFrame.height,
		_explosionFrames.size(), _fireFrames.size());
	return true;
}

bool KjBlobShooter::loadResources(uint difficulty) {
	if (!loadConfig(difficulty) || !loadVisualAssets())
		return false;
	for (uint cue = 0; cue < kAudioCueCount; ++cue) {
		Common::File file;
		const Common::String name = Common::String::format("kj%u.wav", cue);
		if (!file.open(Common::Path(name))) {
			warning("Ripper: could not open KJ blob-shooter cue '%s'",
				name.c_str());
			return false;
		}
	}
	return true;
}

void KjBlobShooter::playCue(uint index, uint volumePercent, bool loop) {
	if (index >= kAudioCueCount)
		return;
	if (!_engine->getMedia()->playSoundEffect(
			Common::String::format("kj%u.wav", index),
			_audioHandles[index], volumePercent, loop)) {
		warning("Ripper: could not play KJ blob-shooter cue index=%u", index);
	}
}

void KjBlobShooter::stopCue(uint index) {
	if (index < kAudioCueCount)
		_engine->getMedia()->stopSoundEffect(_audioHandles[index]);
}

void KjBlobShooter::stopAllAudio() {
	for (uint cue = 0; cue < kAudioCueCount; ++cue)
		stopCue(cue);
	for (uint type = 0; type < kBlobTypeCount; ++type)
		_engine->getMedia()->stopSoundEffect(_movementHandles[type]);
}

void KjBlobShooter::applyCursor() {
	_engine->getCursor()->applyCustomCursor(_cursorFrame,
		_cursorFrame.width / 2, _cursorFrame.height / 2, kDisplayScale);
	_engine->getCursor()->setVisible(_phase == 0);
}

bool KjBlobShooter::markSolved(const char *reason) {
	if (_result == kSolved)
		return true;
	if (!_engine->getMilestones()->set(_completionFlag, true, reason)) {
		_result = kLoadFailed;
		return false;
	}
	_result = kSolved;
	return true;
}

bool KjBlobShooter::beginVictory(uint32 now, const char *reason) {
	if (_phase != 0)
		return true;
	if (!markSolved(reason))
		return false;
	_phase = 1;
	_phaseStartMillis = now;
	_engine->getCursor()->setVisible(false);
	debugC(1, kDebugPuzzles,
		"Ripper: KJ blob-shooter reached win rate spawnDelay=%d "
		"winRate=%d missed=%d destroyed=%d",
		_spawnDelayTicks, _config.winRateTicks,
		_missedBlobs, _destroyedBlobs);
	logProgress("victory-start", 0);
	return true;
}

uint16 KjBlobShooter::serviceKeyboard(uint backgroundFrame) {
	while (_engine->getInput()->hasPendingKey()) {
		const uint16 command = _engine->getInput()->consumeKey();
		if (command == 0x1b) {
			_result = kExited;
			debugC(1, kDebugPuzzles,
				"Ripper: KJ blob-shooter exited by Escape backgroundFrame=%u "
				"missed=%d destroyed=%d spawnDelay=%d",
				backgroundFrame, _missedBlobs, _destroyedBlobs,
				_spawnDelayTicks);
			logProgress("escape", backgroundFrame);
			return kExitCommand;
		}
		if (command == kHelpCommand) {
			debugC(1, kDebugPuzzles,
				"Ripper: KJ blob-shooter opening help table=0x%x",
				kHelpSelectionTable);
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
		static const char keyword[] = "caffeine";
		if (character == keyword[_keywordIndex])
			++_keywordIndex;
		else
			_keywordIndex = 0;
		if (_keywordIndex == sizeof(keyword) - 1) {
			if (!markSolved("kj-blob-shooter-keyword"))
				return kFailureCommand;
			debugC(1, kDebugPuzzles,
				"Ripper: KJ blob-shooter completed by caffeine keyword");
			return kExitCommand;
		}
	}
	return 0;
}

int KjBlobShooter::findHitBlob(const Common::Point &point) const {
	for (uint index = 0; index < _activeBlobs.size(); ++index) {
		const Blob &blob = _activeBlobs[index];
		if (blob.type >= kBlobTypeCount ||
				blob.frame >= _blobFrames[blob.type].size())
			continue;
		const BitmapAssetFrame &sprite = _blobFrames[blob.type][blob.frame];
		const Common::Rect bounds(kBlobX[blob.type], kBlobY[blob.type],
			kBlobX[blob.type] + sprite.width,
			kBlobY[blob.type] + sprite.height);
		if (!bounds.contains(point))
			continue;
		const uint pixelX = point.x - kBlobX[blob.type];
		const uint pixelY = point.y - kBlobY[blob.type];
		if (sprite.pixels[pixelY * sprite.width + pixelX] !=
				kBlobTransparentColor)
			return index;
	}
	return -1;
}

void KjBlobShooter::hitBlob(uint index, int hitX, int hitY, bool scripted) {
	if (index >= _activeBlobs.size())
		return;
	++_activeBlobs[index].hits;
	const Blob blob = _activeBlobs[index];
	const uint frameCount = _blobFrames[blob.type].size();
	const bool completedHitCount =
		_config.shotsPerBlob > 0 &&
		blob.hits == (uint)_config.shotsPerBlob;
	const uint scalePercent =
		frameCount == 0 ? 100 : MIN<uint>(100, blob.frame * 100 / frameCount);
	playCue(3, 75);
	if ((int)blob.frame >= _config.startWhooshFrame)
		_engine->getMedia()->stopSoundEffect(_movementHandles[blob.type]);
	_explosions.push_back(Effect(hitX, hitY, scalePercent));
	_activeBlobs.remove_at(index);
	++_destroyedBlobs;
	debugC(2, kDebugPuzzles,
		"Ripper: KJ blob-shooter blob hit type=%u frame=%u/%u "
		"scripted=%d point=%d,%d hits=%u/%d completed=%d "
		"active=%u missed=%d destroyed=%d "
		"spawnDelay=%d energy=%d",
		blob.type, blob.frame, frameCount, scripted, hitX, hitY,
		blob.hits, _config.shotsPerBlob, completedHitCount,
		_activeBlobs.size(), _missedBlobs, _destroyedBlobs,
		_spawnDelayTicks, _weaponEnergyPercent);
}

void KjBlobShooter::serviceWeapon(const Common::Point &point,
		bool firing, uint32 now) {
	if (_phase != 0 || !firing) {
		_shotArmed = true;
		if (_config.weaponCharge > 0 && _weaponEnergyPercent < 100 &&
				(_config.weaponChargeTicks == 0 ||
				now - _lastRechargeMillis >=
					ticksToMillis(_config.weaponChargeTicks))) {
			_weaponEnergyPercent = MIN<int>(100,
				_weaponEnergyPercent + _config.weaponCharge);
			_lastRechargeMillis = now;
		}
		return;
	}

	if (_weaponEnergyPercent == 0 || !_shotArmed ||
			now - _lastShotMillis < ticksToMillis(_config.rapidFireTicks))
		return;

	playCue(4, 50);
	_fireEffects.push_back(Effect(point.x, point.y, 100));
	_weaponEnergyPercent = MAX<int>(0,
		_weaponEnergyPercent - _config.weaponDischarge);
	const int hitIndex = findHitBlob(point);
	if (hitIndex >= 0)
		hitBlob(hitIndex, point.x, point.y, false);
	else
		debugC(3, kDebugPuzzles,
			"Ripper: KJ blob-shooter shot missed point=%d,%d "
			"active=%u energy=%d spawnDelay=%d",
			point.x, point.y, _activeBlobs.size(),
			_weaponEnergyPercent, _spawnDelayTicks);
	_lastShotMillis = now;
	if (_config.rapidFireTicks == 0)
		_shotArmed = false;
}

void KjBlobShooter::spawnBlob(uint32 now) {
	if (_activeBlobs.size() >= kMaximumActiveBlobs) {
		warning("Ripper: KJ blob-shooter active blob capacity reached");
		_nextSpawnMillis = now + ticksToMillis(_spawnDelayTicks);
		return;
	}
	const uint type = _random.getRandomNumber(kBlobTypeCount - 1);
	_activeBlobs.push_back(Blob(type));
	_nextSpawnMillis = now + ticksToMillis(_spawnDelayTicks);
	debugC(2, kDebugPuzzles,
		"Ripper: KJ blob-shooter blob spawned type=%u frames=%u "
		"origin=%d,%d size=%ux%u active=%u spawnDelay=%d "
		"winRate=%d missed=%d/%d",
		type, _blobFrames[type].size(), kBlobX[type], kBlobY[type],
		_blobFrames[type][0].width, _blobFrames[type][0].height,
		_activeBlobs.size(), _spawnDelayTicks, _config.winRateTicks,
		_missedBlobs, _config.missesAllowed);
}

uint16 KjBlobShooter::advanceBlobs() {
	for (uint index = 0; index < _activeBlobs.size();) {
		Blob &blob = _activeBlobs[index];
		++blob.frame;
		if ((int)blob.frame == _config.startWhooshFrame) {
			_engine->getMedia()->playSoundEffect("kj2.wav",
				_movementHandles[blob.type], 40);
		}
		if (blob.frame < _blobFrames[blob.type].size()) {
			++index;
			continue;
		}
		const uint type = blob.type;
		_activeBlobs.remove_at(index);
		++_missedBlobs;
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter blob escaped type=%u "
			"active=%u missed=%d/%d destroyed=%d spawnDelay=%d",
			type, _activeBlobs.size(), _missedBlobs,
			_config.missesAllowed, _destroyedBlobs, _spawnDelayTicks);
		if (_phase == 0 && _missedBlobs >= _config.missesAllowed) {
			_result = kExited;
			debugC(1, kDebugPuzzles,
				"Ripper: KJ blob-shooter lost missed=%d limit=%d "
				"spawnDelay=%d winRate=%d",
				_missedBlobs, _config.missesAllowed,
				_spawnDelayTicks, _config.winRateTicks);
			logProgress("miss-limit", 0);
			return kExitCommand;
		}
	}
	return 0;
}

uint16 KjBlobShooter::updateSceneState(uint32 now) {
	if (_phase < 5 &&
			(_activeBlobs.empty() || now >= _nextSpawnMillis)) {
		spawnBlob(now);
		if (_spawnDelayTicks > (int)kMinimumSpawnDelayTicks &&
				now >= _nextSpawnDecreaseMillis) {
			_spawnDelayTicks = MAX<int>(kMinimumSpawnDelayTicks,
				_spawnDelayTicks - _config.spawnDecreaseTicks);
			_nextSpawnDecreaseMillis =
				now + ticksToMillis(_config.spawnDecreaseDelayTicks);
			debugC(2, kDebugPuzzles,
				"Ripper: KJ blob-shooter spawn rate advanced "
				"delay=%d winRate=%d phase=%u",
				_spawnDelayTicks, _config.winRateTicks, _phase);
		}
		if (_phase == 0 && _spawnDelayTicks <= _config.winRateTicks &&
				!beginVictory(now, "kj-blob-shooter"))
			return kFailureCommand;
	}

	if (_phase == 1 &&
			now - _phaseStartMillis >= ticksToMillis(kVictoryCueDelayTicks)) {
		playCue(8);
		_phase = 2;
		_phaseStartMillis = now;
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter victory cue started");
	}
	if (_phase == 2 &&
			now - _phaseStartMillis >= ticksToMillis(kAutoHitDelayTicks)) {
		_scriptedAutoHit = true;
		_phase = 3;
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter scripted cleanup enabled active=%u",
			_activeBlobs.size());
	}
	if (_phase == 3 &&
			_spawnDelayTicks <= (int)kMinimumSpawnDelayTicks) {
		_phase = 5;
		_phaseStartMillis = now;
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter stopped spawning active=%u",
			_activeBlobs.size());
	}
	if (_phase == 5 && _activeBlobs.empty()) {
		_phase = 6;
		_phaseStartMillis = now;
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter blobs are done");
	}
	if (_phase == 6 &&
			now - _phaseStartMillis >= ticksToMillis(kVictoryExitDelayTicks)) {
		debugC(1, kDebugPuzzles,
			"Ripper: KJ blob-shooter victory sequence completed "
			"missed=%d destroyed=%d",
			_missedBlobs, _destroyedBlobs);
		return kExitCommand;
	}

	if (_phase == 0 && now >= _nextAmbientMillis) {
		playCue(_ambientCue);
		if (_ambientCue == 5)
			_ambientCue = 7;
		else
			--_ambientCue;
		_nextAmbientMillis = now + ticksToMillis(kAmbientIntervalTicks);
	}
	return 0;
}

void KjBlobShooter::drawBitmap(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y,
		uint scalePercent, bool centered) const {
	if (frame.width == 0 || frame.height == 0 || scalePercent == 0)
		return;
	const uint scaledWidth = MAX<uint>(1, frame.width * scalePercent / 100);
	const uint scaledHeight = MAX<uint>(1, frame.height * scalePercent / 100);
	const int logicalX = centered ? x - (int)scaledWidth / 2 : x;
	const int logicalY = centered ? y - (int)scaledHeight / 2 : y;
	for (uint targetY = 0; targetY < scaledHeight; ++targetY) {
		const uint sourceY = targetY * frame.height / scaledHeight;
		for (uint targetX = 0; targetX < scaledWidth; ++targetX) {
			const uint sourceX = targetX * frame.width / scaledWidth;
			const byte pixel =
				frame.pixels[sourceY * frame.width + sourceX];
			if (pixel == frame.transparentColor)
				continue;
			const int displayX = (logicalX + targetX) * kDisplayScale;
			const int displayY = (logicalY + targetY) * kDisplayScale;
			for (uint scaleY = 0; scaleY < kDisplayScale; ++scaleY) {
				if (displayY + (int)scaleY < 0 ||
						displayY + (int)scaleY >= g_system->getHeight())
					continue;
				for (uint scaleX = 0; scaleX < kDisplayScale; ++scaleX) {
					if (displayX + (int)scaleX >= 0 &&
							displayX + (int)scaleX < g_system->getWidth()) {
						screen[(displayY + scaleY) * pitch +
							displayX + scaleX] = pixel;
					}
				}
			}
		}
	}
}

void KjBlobShooter::drawEffects(byte *screen, uint pitch,
		Common::Array<Effect> &effects,
		const Common::Array<BitmapAssetFrame> &frames) {
	for (uint index = 0; index < effects.size();) {
		Effect &effect = effects[index];
		if (effect.frame >= frames.size()) {
			effects.remove_at(index);
			continue;
		}
		drawBitmap(screen, pitch, frames[effect.frame],
			effect.x, effect.y, effect.scalePercent, true);
		++effect.frame;
		++index;
	}
}

bool KjBlobShooter::drawOverlay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		warning("Ripper: KJ blob-shooter requires an indexed display");
		return false;
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint index = 0; index < _activeBlobs.size(); ++index) {
		const Blob &blob = _activeBlobs[index];
		if (blob.frame < _blobFrames[blob.type].size()) {
			drawBitmap(pixels, screen->pitch,
				_blobFrames[blob.type][blob.frame],
				kBlobX[blob.type], kBlobY[blob.type], 100, false);
		}
	}
	drawEffects(pixels, screen->pitch, _fireEffects, _fireFrames);
	drawEffects(pixels, screen->pitch, _explosions, _explosionFrames);
	g_system->unlockScreen();
	return true;
}

void KjBlobShooter::logProgress(const char *reason,
		uint backgroundFrame) const {
	int reductionsNeeded = 0;
	if (_spawnDelayTicks > _config.winRateTicks &&
			_config.spawnDecreaseTicks > 0) {
		reductionsNeeded =
			(_spawnDelayTicks - _config.winRateTicks +
				_config.spawnDecreaseTicks - 1) /
			_config.spawnDecreaseTicks;
	}
	const int missesRemaining =
		MAX<int>(0, _config.missesAllowed - _missedBlobs);
	debugC(2, kDebugPuzzles,
		"Ripper: KJ blob-shooter progress reason=%s tick=%u "
		"backgroundFrame=%u phase=%u standing=%s "
		"spawnDelay=%d winRate=%d reductionsNeeded=%d "
		"missed=%d/%d remaining=%d destroyed=%d active=%u energy=%d",
		reason, _presentationTicks, backgroundFrame, _phase,
		_phase != 0 || _spawnDelayTicks <= _config.winRateTicks ?
			"WINNING" : "RACING",
		_spawnDelayTicks, _config.winRateTicks, reductionsNeeded,
		_missedBlobs, _config.missesAllowed, missesRemaining,
		_destroyedBlobs, _activeBlobs.size(), _weaponEnergyPercent);
}

uint16 KjBlobShooter::service(uint frame) {
	++_presentationTicks;
	const uint32 now = g_system->getMillis(true);
	const uint16 keyboardCommand = serviceKeyboard(frame);
	if (keyboardCommand != 0)
		return keyboardCommand;

	const MouseState &mouse = _engine->getInput()->peekMouseState();
	const Common::Point point(
		CLIP<int>(mouse.position.x / (int)kDisplayScale,
			0, (int)kLogicalWidth - 1),
		CLIP<int>(mouse.position.y / (int)kDisplayScale,
			0, (int)kLogicalHeight - 1));
	serviceWeapon(point, (mouse.buttons & kMouseButtonLeft) != 0, now);

	if (_scriptedAutoHit && !_activeBlobs.empty()) {
		const Blob &blob = _activeBlobs.front();
		const uint frameCount = _blobFrames[blob.type].size();
		if (frameCount >= 4 && blob.frame >= frameCount - 4) {
			playCue(4, 50);
			_fireEffects.push_back(Effect(
				kScriptedHitX[blob.type], kScriptedHitY[blob.type], 100));
			if (blob.frame == frameCount - 2) {
				hitBlob(0, kScriptedHitX[blob.type],
					kScriptedHitY[blob.type], true);
			}
		}
	}

	if (!drawOverlay()) {
		_result = kLoadFailed;
		return kFailureCommand;
	}
	const uint16 blobCommand = advanceBlobs();
	if (blobCommand != 0)
		return blobCommand;
	const uint16 stateCommand = updateSceneState(now);
	if (stateCommand != 0)
		return stateCommand;
	if (_presentationTicks == 1 ||
			_presentationTicks % kProgressInterval == 0)
		logProgress("checkpoint", frame);
	_engine->getCursor()->setVisible(_phase == 0);
	return 0;
}

Scene::Result KjBlobShooter::run(uint completionFlag) {
	prepare("kj-blob-shooter-entry", 19, false);
	_completionFlag = completionFlag;
	_result = kExited;
	_config = Config();
	for (uint type = 0; type < kBlobTypeCount; ++type)
		_blobFrames[type].clear();
	_explosionFrames.clear();
	_fireFrames.clear();
	_activeBlobs.clear();
	_explosions.clear();
	_fireEffects.clear();
	_keywordIndex = 0;
	_presentationTicks = 0;
	_phase = 0;
	_ambientCue = 7;
	_missedBlobs = 0;
	_destroyedBlobs = 0;
	_weaponEnergyPercent = 100;
	_shotArmed = true;
	_scriptedAutoHit = false;

	const uint difficulty =
		CLIP<uint>(_engine->getSettings()->getCombatLevel(), 1, 3);
	debugC(1, kDebugPuzzles,
		"Ripper: entering KJ blob-shooter difficulty=%u "
		"config='kj%u.ini' completionFlag=%u",
		difficulty, difficulty, completionFlag);
	if (!loadResources(difficulty)) {
		_result = kLoadFailed;
	} else {
		_spawnDelayTicks = _config.spawnDelayTicks;
		_activeBlobs.push_back(Blob(0));
		const uint32 now = g_system->getMillis(true);
		_nextSpawnMillis = now +
			ticksToMillis(kInitialSpawnLeadTicks + _spawnDelayTicks);
		_nextSpawnDecreaseMillis = now +
			ticksToMillis(kInitialSpawnLeadTicks +
				_config.spawnDecreaseDelayTicks);
		_nextAmbientMillis = now +
			ticksToMillis(kInitialAmbientLeadTicks +
				kAmbientIntervalTicks);
		_phaseStartMillis = now;
		_lastShotMillis = now;
		_lastRechargeMillis = now;
		playCue(1, 60);
		playCue(0, 75);
		applyCursor();

		uint16 command = 0;
		if (!_engine->getMedia()->playBlobShooterSequence(
				"kj.smk", this, &command)) {
			_result = kLoadFailed;
		} else if (command == kFailureCommand) {
			_result = kLoadFailed;
		} else if (command == 0 && !_engine->shouldQuit()) {
			warning("Ripper: KJ blob-shooter media ended without scene result");
			_result = kLoadFailed;
		}
		debugC(2, kDebugPuzzles,
			"Ripper: KJ blob-shooter media returned command=0x%04x "
			"result=%d missed=%d destroyed=%d spawnDelay=%d energy=%d",
			command, _result, _missedBlobs, _destroyedBlobs,
			_spawnDelayTicks, _weaponEnergyPercent);
	}

	stopAllAudio();
	finish("kj-blob-shooter-exit", 0, true);
	debugC(_result == kLoadFailed ? 1 : 2, kDebugPuzzles,
		"Ripper: left KJ blob-shooter result=%d difficulty=%u "
		"completionFlag=%u missed=%d destroyed=%d spawnDelay=%d",
		_result, difficulty, completionFlag, _missedBlobs,
		_destroyedBlobs, _spawnDelayTicks);
	return _result;
}

} // End of namespace Ripper
