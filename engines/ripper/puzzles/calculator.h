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

#ifndef RIPPER_PUZZLES_CALCULATOR_H
#define RIPPER_PUZZLES_CALCULATOR_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "ripper/display.h"
#include "ripper/resources.h"

namespace Ripper {

class RipperEngine;

class CalculatorPuzzle {
public:
	enum Result {
		kExited,
		kSolved,
		kLoadFailed
	};

	explicit CalculatorPuzzle(RipperEngine *engine);

	Result run(uint completionFlag);

private:
	bool captureBackground();
	void restoreBackground() const;
	bool loadAssets();
	void drawBitmap(byte *screen, uint pitch, const BitmapAssetFrame &frame,
		int x, int y) const;
	void drawDisplay(byte *screen, uint pitch) const;
	void render() const;
	void showButtonFeedback(uint16 command) const;
	int findButton(const Common::Point &point) const;
	void updateCursor(const Common::Point &point);
	bool processCommand(uint16 command, uint completionFlag, Result &result);
	void applyPendingOperator();
	void applyUnaryOperator(uint command);
	void formatDisplay(double value);
	double parseDisplay() const;
	void resetEntry();
	bool advanceUnlockSequence(uint command, uint completionFlag, Result &result);

	RipperEngine *_engine;
	Common::Array<BitmapAssetFrame> _buttons;
	Common::Array<BitmapAssetFrame> _glyphs;
	IndexedDisplaySnapshot _backgroundDisplay;
	Common::String _displayText;
	double _accumulator;
	double _currentOperand;
	uint _pendingOperator;
	uint _entryMode;
	uint _unlockSequenceProgress;
	int _hoveredButton;
	bool _poweredOn;
	bool _displayCleared;
	bool _decimalPointEntered;
	bool _inverseTrigMode;
};

} // End of namespace Ripper

#endif // RIPPER_PUZZLES_CALCULATOR_H
