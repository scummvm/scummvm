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

#ifndef RIPPER_PUZZLES_DATE_SELECTION_H
#define RIPPER_PUZZLES_DATE_SELECTION_H

#include "audio/mixer.h"
#include "common/str.h"

#include "ripper/media.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class DateSelectionPuzzle : public MediaSequenceCallback {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit DateSelectionPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);
	uint16 service(uint frame) override;
	bool continueAfterEnd() const override { return true; }
	bool ownsInput() const override { return true; }

private:
	enum DateGroup {
		kInvalidDate,
		kNovember19,
		kNovember20,
		kOtherAcceptedDate
	};

	bool loadAssets();
	bool playCue(uint cue, uint volumePercent = 100, bool loop = false);
	void stopAudio();
	DateGroup matchDate(const Common::String &text, uint &day) const;
	bool processSubmission();
	bool drawFeedback();
	bool waitTicks(uint ticks);

	RipperEngine *_engine;
	BitmapAssetFrame _waitFrame;
	Audio::SoundHandle _audioHandles[5];
	Common::String _submittedText;
	uint _mediaIndex;
	bool _textEntryActive;
	Result _result;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_DATE_SELECTION_H
