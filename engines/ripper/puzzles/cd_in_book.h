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

#ifndef RIPPER_PUZZLES_CD_IN_BOOK_H
#define RIPPER_PUZZLES_CD_IN_BOOK_H

#include "audio/mixer.h"
#include "common/random.h"

#include "ripper/display.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class CdInBookPuzzle : public Puzzle {
public:
	explicit CdInBookPuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;

private:
	bool loadAssets();
	bool loadBackground();
	bool loadBitmap(const Common::String &name, BitmapAssetFrame &frame);
	bool drawBackground();
	bool captureControls();
	void drawFrame(const BitmapAssetFrame &frame, int x, int y);
	void drawFeedbackMarkers();
	void removeFeedbackMarker(uint marker);
	void setButtonPressed(uint button, bool pressed);
	int findButton(const Common::Point &point) const;
	bool isExitRegion(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool playCue(uint cue);
	void stopAudio();
	bool waitMillis(uint32 duration);
	bool waitForCue(uint cue);
	bool resetFailedAttempt();

	AssetLibrary _library;
	BitmapAssetFrame _background;
	BitmapAssetFrame _buttonFrames[6];
	BitmapAssetFrame _feedbackFrame;
	IndexedDisplaySnapshot _buttonBackings[6];
	IndexedDisplaySnapshot _feedbackBackings[8];
	Audio::SoundHandle _audioHandles[3];
	Common::RandomSource _random;
	int _pressedButton;
	int _hoveredButton;
	bool _exitPressed;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_CD_IN_BOOK_H
