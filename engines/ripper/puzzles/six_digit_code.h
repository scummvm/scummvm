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

#ifndef RIPPER_PUZZLES_SIX_DIGIT_CODE_H
#define RIPPER_PUZZLES_SIX_DIGIT_CODE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class SixDigitCodePuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit SixDigitCodePuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool loadFrame(const Common::String &path, BitmapAssetFrame &frame);
	bool loadAssets();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void render(bool showDigits = true) const;
	int findControl(const Common::Point &point) const;
	void updateHover(const Common::Point &point);
	bool playCue(uint cue);
	bool waitTicks(uint ticks);
	bool activateControl(uint control, uint completionFlag, Result &result);
	bool validateEntry(uint completionFlag, Result &result);
	void resetEntry(const char *reason);
	void stopAudio();
	Common::String enteredCode() const;

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _numberFrames;
	Common::Array<BitmapAssetFrame> _controlFrames;
	IndexedDisplaySnapshot _baseDisplay;
	Audio::SoundHandle _audioHandles[5];
	Common::Array<byte> _enteredDigits;
	int _hoveredControl;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_SIX_DIGIT_CODE_H
