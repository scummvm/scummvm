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

#ifndef RIPPER_PUZZLES_EIGHT_BUTTON_SEQUENCE_H
#define RIPPER_PUZZLES_EIGHT_BUTTON_SEQUENCE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/media.h"
#include "ripper/puzzles/puzzle.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class EightButtonSequencePuzzle : public Puzzle, public MediaSequenceCallback {
public:
	explicit EightButtonSequencePuzzle(RipperEngine *engine);

	Result run(uint completionFlag) override;
	uint16 service(uint frame) override;
	bool continueAfterEnd() const override { return true; }
	bool ownsInput() const override { return true; }

private:
	bool loadAssets();
	bool loadFrame(const Common::String &path, BitmapAssetFrame &frame);
	bool captureBackings();
	bool captureRegion(const Common::Rect &bounds, Common::Array<byte> &pixels);
	void restoreRegion(byte *screen, uint pitch, const Common::Rect &bounds,
		const Common::Array<byte> &pixels) const;
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void updateHover(const Common::Point &point);
	int findButton(const Common::Point &point) const;
	void drawFeedback(bool visible);
	void drawSuccessDark();
	bool waitTicks(uint ticks);
	uint16 pressButton(uint button);
	bool playCue(uint cue);
	void stopAudio();
	Common::String enteredSequence() const;

	Common::Array<BitmapAssetFrame> _buttonFrames;
	Common::Array<BitmapAssetFrame> _feedbackFrames;
	BitmapAssetFrame _successDark;
	Common::Array<Common::Array<byte> > _buttonBackings;
	Common::Array<byte> _feedbackBacking;
	Audio::SoundHandle _audioHandles[5];
	Common::Array<byte> _enteredButtons;
	uint _completionFlag;
	int _hoveredButton;
	Result _result;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_EIGHT_BUTTON_SEQUENCE_H
