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

#ifndef RIPPER_PUZZLES_GC_CSH_H
#define RIPPER_PUZZLES_GC_CSH_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class GcCshPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit GcCshPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool captureBackground();
	bool loadAssets();
	bool playChoice(uint choice, uint sequenceSlot);
	bool resetSequence(const int enteredChoices[4], uint enteredChoiceCount);
	bool waitForCue(uint cue);
	void drawAnimationFrame(uint choice, uint sequenceSlot, uint frameIndex);
	void drawResetFrame(const int enteredChoices[4], uint enteredChoiceCount,
		uint frameIndex);
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void restoreSequenceBacking();
	void playCue(uint cue);
	void stopAudio();
	uint16 serviceInput();
	int findChoice(const Common::Point &point) const;
	bool isExitRegion(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _choiceFrames[4];
	IndexedDisplaySnapshot _backgroundDisplay;
	Audio::SoundHandle _audioHandles[2];
	int _hoveredChoice;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_GC_CSH_H
