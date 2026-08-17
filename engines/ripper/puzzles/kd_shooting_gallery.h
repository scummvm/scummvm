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

#ifndef RIPPER_PUZZLES_KD_SHOOTING_GALLERY_H
#define RIPPER_PUZZLES_KD_SHOOTING_GALLERY_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/media.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Common {
class INIFile;
}

namespace Ripper {

class KdShootingGallery : public Puzzle, public MediaSequenceCallback {
public:
	explicit KdShootingGallery(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool ownsInput() const override { return true; }

private:
	static const uint kTargetClassCount = 2;
	static const uint kAudioCueCount = 13;

	struct Config {
		int frameRate;
		int requiredHits[kTargetClassCount];
		int points[kTargetClassCount];
		int goal;
		int unusedPenalty;
		int rapidFireTicks;
		int weaponDischarge;
		int weaponCharge;
		int weaponChargeTicks;

		Config();
	};

	bool loadConfig(uint difficulty);
	bool loadResources(uint difficulty);
	bool loadFrameData();
	bool loadVisualAssets();
	int configInt(const Common::INIFile &ini, const char *section,
		const char *key, int fallback = 0) const;
	static Common::String basenameFromPath(const Common::String &path);
	static Common::String formatScore(int score);

	const PresentationRegion *findHitRegion(
		const PresentationFrameRegion &frame, const Common::Point &point) const;
	void updateTargetGroup(const PresentationFrameRegion &frame, uint frameIndex);
	void updateTargetVisibility(
		const PresentationFrameRegion &frame, uint frameIndex);
	void logProgress(uint frameIndex, const char *reason) const;
	void serviceFrameCue(uint frameIndex);
	void serviceFixedCue(uint frameIndex);
	uint16 serviceKeyboard(uint frameIndex);
	void serviceWeapon(const PresentationFrameRegion &frame,
		const Common::Point &point, bool firing, uint32 now, uint frameIndex);
	void playCue(uint index, const Common::String &path,
		uint volumePercent = 100, bool loop = false);
	void playNumberedCue(uint index, uint volumePercent = 100, bool loop = false);
	void stopCue(uint index);
	void stopAllAudio();
	int scoringTargetCount(uint targetClass) const;
	int scoreForClass(uint targetClass) const;
	int currentScore() const;
	int rewardUnitsNeeded() const;
	uint displayedTargetCount(uint targetClass) const;

	bool presentResults();
	bool drawResults();
	void drawBitmapScaled(byte *screen, uint pitch,
		const BitmapAssetFrame &frame, int x, int y) const;
	void drawTextScaled(byte *screen, uint pitch,
		int x, int y, const Common::String &text, byte color) const;
	bool animateResultsUntilInput();
	void applyCursor();

	Config _config;
	PresentationFrameRegionTable _regionTable;
	PresentationFrameAudioMap _audioMap;
	BitmapAssetFrame _cursorFrame;
	BitmapAssetFrame _checkmarkFrame;
	Common::Array<BitmapAssetFrame> _flameFrames;
	BitmapFontAsset _font;
	Audio::SoundHandle _audioHandles[kAudioCueCount];
	int _rawHits[kTargetClassCount];
	int _distinctTargetsHit[kTargetClassCount];
	int _completedTargets[kTargetClassCount];
	uint _activeTargetGroup;
	int _targetGroupSerial;
	int _lastHitTargetGroupSerial;
	int _hitsInCurrentTargetGroup;
	int _weaponEnergyPercent;
	byte _visibleTargetMask;
	byte _targetGroupTypeMask;
	uint _keywordIndex;
	uint _completionFlag;
	uint32 _lastShotMillis;
	uint32 _lastRechargeMillis;
	bool _shotArmed;
	Result _result;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KD_SHOOTING_GALLERY_H
