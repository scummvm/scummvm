/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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

#ifndef RIPPER_PUZZLES_KEYPAD_SEQUENCE_H
#define RIPPER_PUZZLES_KEYPAD_SEQUENCE_H

#include "audio/mixer.h"
#include "common/array.h"
#include "common/rect.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class KeypadSequencePuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit KeypadSequencePuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	enum HoverControl {
		kHoverNone,
		kHoverCode,
		kHoverExit,
		kHoverKey
	};

	bool loadAssets();
	bool loadFrame(const Common::String &path, BitmapAssetFrame &frame);
	bool prepareEntryDisplay();
	bool openKeypad();
	bool closeKeypad();
	void drawFrame(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void render() const;
	int findKey(const Common::Point &point) const;
	HoverControl findControl(const Common::Point &point, int &key) const;
	bool updateCursor(const Common::Point &point);
	bool serviceBlink();
	bool waitFeedbackTicks(uint ticks);
	void animateClear(bool success);
	bool pressKey(uint key, uint completionFlag, Result &result);
	void resetEntry();
	Common::String enteredSequence() const;
	void stopAudio();

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _extraFrames;
	Common::Array<BitmapAssetFrame> _keyFrames;
	IndexedDisplaySnapshot _baseDisplay;
	Common::Array<byte> _enteredDigits;
	Audio::SoundHandle _pressAudioHandle;
	Audio::SoundHandle _releaseAudioHandle;
	Audio::SoundHandle _feedbackAudioHandle;
	uint16 _slotMask;
	uint32 _lastBlinkMillis;
	int _hoveredKey;
	int _pressedKey;
	HoverControl _hoveredControl;
	bool _keypadOpen;
	bool _blinkLit;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_KEYPAD_SEQUENCE_H
