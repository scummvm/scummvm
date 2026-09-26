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

#ifndef RIPPER_PUZZLES_KJ_BLOB_SHOOTER_H
#define RIPPER_PUZZLES_KJ_BLOB_SHOOTER_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/random.h"
#include "common/rect.h"

#include "ripper/media.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Common {
class INIFile;
}

namespace Ripper {

class KjBlobShooter : public Puzzle, public MediaSequenceCallback {
public:
	explicit KjBlobShooter(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }

private:
	static const uint kBlobTypeCount = 5;
	static const uint kAudioCueCount = 9;
	static const uint kMaximumActiveBlobs = 30;

	struct Config {
		int frameRate;
		int startWhooshFrame;
		int spawnDelayTicks;
		int spawnDecreaseDelayTicks;
		int spawnDecreaseTicks;
		int missesAllowed;
		int shotsPerBlob;
		int winRateTicks;
		int rapidFireTicks;
		int weaponDischarge;
		int weaponCharge;
		int weaponChargeTicks;

		Config();
	};

	struct Blob {
		uint type;
		uint frame;
		uint hits;

		Blob(uint blobType = 0) : type(blobType), frame(0), hits(0) {}
	};

	struct Effect {
		int x;
		int y;
		uint frame;
		uint scalePercent;

		Effect(int effectX, int effectY, uint scale) :
			x(effectX), y(effectY), frame(0), scalePercent(scale) {}
	};

	bool loadConfig(uint difficulty);
	bool loadResources(uint difficulty);
	bool loadVisualAssets();
	int configInt(const Common::INIFile &ini, const char *section,
		const char *key, int fallback) const;

	uint16 serviceKeyboard(uint backgroundFrame);
	void serviceWeapon(const Common::Point &point, bool firing, uint32 now);
	int findHitBlob(const Common::Point &point) const;
	void hitBlob(uint index, int hitX, int hitY, bool scripted);
	void spawnBlob(uint32 now);
	uint16 advanceBlobs();
	uint16 updateSceneState(uint32 now);
	bool beginVictory(uint32 now, const char *reason);
	bool markSolved(const char *reason);
	void logProgress(const char *reason, uint backgroundFrame) const;

	bool drawOverlay();
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y, uint scalePercent, bool centered) const;
	void drawEffects(byte *screen, uint pitch, Common::Array<Effect> &effects,
		const Common::Array<BitmapAssetFrame> &frames);

	void applyCursor();
	void playCue(uint index, uint volumePercent = 100, bool loop = false);
	void stopCue(uint index);
	void stopAllAudio();

	Config _config;
	Common::Array<BitmapAssetFrame> _blobFrames[kBlobTypeCount];
	Common::Array<BitmapAssetFrame> _explosionFrames;
	Common::Array<BitmapAssetFrame> _fireFrames;
	BitmapAssetFrame _cursorFrame;
	Common::Array<Blob> _activeBlobs;
	Common::Array<Effect> _explosions;
	Common::Array<Effect> _fireEffects;
	Audio::SoundHandle _audioHandles[kAudioCueCount];
	Audio::SoundHandle _movementHandles[kBlobTypeCount];
	Common::RandomSource _random;
	uint _completionFlag;
	uint _keywordIndex;
	uint _presentationTicks;
	uint _phase;
	uint _ambientCue;
	int _spawnDelayTicks;
	int _missedBlobs;
	int _destroyedBlobs;
	int _weaponEnergyPercent;
	uint32 _nextSpawnMillis;
	uint32 _nextSpawnDecreaseMillis;
	uint32 _nextAmbientMillis;
	uint32 _phaseStartMillis;
	uint32 _lastShotMillis;
	uint32 _lastRechargeMillis;
	bool _shotArmed;
	bool _scriptedAutoHit;
	Result _result;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KJ_BLOB_SHOOTER_H
