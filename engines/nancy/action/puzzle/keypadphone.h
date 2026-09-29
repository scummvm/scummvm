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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NANCY_ACTION_KEYPADPHONE_H
#define NANCY_ACTION_KEYPADPHONE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/util.h"
#include "engines/nancy/action/puzzlerecord.h"

namespace Nancy {
namespace Action {

// Nancy16 telephone (AR 116), which replaced the older Telephone records. The player
// dials on a keypad, and a short while after the last key the dialed number is looked
// up: a known number leads to its own scene, a number that fits one of the number
// classes (area code and length) leads to that class's scene, and anything else to
// the scene of the record's empty number.
class KeypadPhone : public PuzzleRecord {
public:
	KeypadPhone() : PuzzleRecord(7) {}
	virtual ~KeypadPhone() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "KeypadPhone"; }

	struct Number {
		Common::String digits;
		FlagDescription condition;		// the number only answers while this is set
		SceneChangeWithFlag scene;
	};

	struct Button {
		char key = 0;
		Common::Rect src;				// pressed look
		Common::Rect dest;
		RandomSoundBlock sound;
	};

	struct NumberClass {
		Common::String prefix;
		int32 minExtraDigits = 0;
		int32 maxExtraDigits = 0;
		SceneChangeWithFlag scene;
	};

	static void readScene(Common::SeekableReadStream &stream, SceneChangeWithFlag &scene);
	static Common::String keepDialable(const Common::String &text);

	const NumberClass *findClass() const;
	void pressButton(const Button &button);
	void dial();

	void redraw();

	// -- File data --
	uint32 _maxDigits = 0;
	int32 _noClassWaitTicks = 0;		// after the last key, when the digits fit no class yet
	int32 _classWaitTicks = 0;			// after the last key, when they do
	Common::Array<Number> _numbers;		// includes the one with no digits
	uint16 _buttonCursor = 0;
	int32 _pressedTicks = 0;
	Common::Array<Button> _buttons;
	Common::Array<NumberClass> _classes;
	RandomSoundBlock _dialToneBlock;
	Common::Array<ExitHotspot> _exits;

	// -- Runtime state --
	Common::String _dialed;
	int32 _dialWaitTicks = 0;
	int32 _pressedTicksLeft = 0;
	const Button *_pressedButton = nullptr;
	uint32 _lastTickTime = 0;

	SoundDescription _dialTone;
	SceneChangeWithFlag _endScene;
	bool _hasDialed = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_KEYPADPHONE_H
