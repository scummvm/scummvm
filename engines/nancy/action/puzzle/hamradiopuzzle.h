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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_HAMRADIOPUZZLE_H
#define NANCY_ACTION_HAMRADIOPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// A puzzle that has the player input radio frequencies, and
// send morse code data via ham radio. Used in nancy6.
class HamRadioPuzzle : public RenderActionRecord {
public:
	HamRadioPuzzle() : RenderActionRecord(7) {}
	virtual ~HamRadioPuzzle() {}

	void init() override;
	void updateGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "HamRadioPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void setFrequency(const Common::Array<uint16> &freq);

	// 0-10 are the digit buttons
	enum ButtonPress { kNone = -1, kConnect = 10, kDot = 11, kDash = 12, kSend = 13, kDelete = 14, kReset = 15 };

	struct CCSound {
		Common::String text;
		SoundDescription sound;

		void readData(Common::SeekableReadStream &stream);
		void loadAndPlay();
	};

	struct Frequency {
		Common::Array<uint16> frequency;
		CCSound sound;
		FlagDescription flag;

		void readData(Common::SeekableReadStream &stream, uint16 numDigits);
	};

	Common::Path _imageName;

	uint16 _numDigits = 0;

	Frequency _startFreq;
	Frequency _correctFreq;

	uint16 _passwordMaxSize = 0;
	Common::String _password;
	FlagDescription _passwordFlag;
	Common::String _codeWord;

	Common::Array<Common::Rect> _digitDests;
	Common::Array<Common::Rect> _buttonDests;

	Common::Array<Common::Rect> _digitSrcs;
	Common::Array<Common::Rect> _buttonSrcs;

	SoundDescription _digitRollSound;
	CCSound _frequencyButtonSound;
	CCSound _connectButtonSound;
	CCSound _dotButtonSound;
	CCSound _dashButtonSound;
	CCSound _sendButtonSound;
	CCSound _deleteButtonSound;
	CCSound _resetButtonSound;
	CCSound _badLetterSound;
	CCSound _longMorseOtherSound;
	CCSound _goodPasswordSound;
	CCSound _longMorseSound;
	Common::Array<CCSound> _badFrequencySounds;

	SceneChangeWithFlag _solveScene;
	uint16 _solveSoundDelay = 0; // not used
	CCSound _solveSound;

	Common::Rect _exitButtonDest;
	Common::Rect _exitButtonSrc;

	SceneChangeWithFlag _exitScene;
	uint16 _exitSoundDelay = 0; // not used
	SoundDescription _exitSound; // not used

	Common::Array<Frequency> _otherFrequencies;

	Graphics::ManagedSurface _image;

	// Frequency display data
	bool _digitsRolling = true;
	Common::Array<uint16> _curDigits;
	Common::Array<uint16> _displayedDigitFrames;
	Common::Array<uint32> _nextDigitFrameTimes;

	// Sent morse code
	Common::String _curMorseString;
	Common::String _curCharString;

	int _pressedButton = kNone;
	uint32 _buttonEndTime = 0;

	bool _isOnCorrectFrequency = false;
	bool _solvedPassword = false;
	bool _solvedCodeword = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_HAMRADIOPUZZLE_H
