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

#ifndef NANCY_ACTION_ORDERINGPUZZLE_H
#define NANCY_ACTION_ORDERINGPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Class implementing several action records of the type where
// the player has to press a sequence of buttons in a certain order.
//		- OrderingPuzzle: The most simple type. Allows for manual depressing of buttons
//		- PianoPuzzle: Buttons always auto-depress; every button has unique sound file
//		- OrderItemsPuzzle: Buttons may depress or stay down, but player can't depress manually.
//			Has second button state that is activated when player is holding a specific item. (see fingerprint keypad puzzle in nancy4)
//		- KeypadPuzzle: Buttons may auto-depress, stay down, and can be depressed manually by player.
//			Adds an optional button for manually checking for correct solution, number of possible buttons is 30.
//		- KeypadPuzzleTerse: Same as above, but data format is shorter, and supports up to 100 buttons
class OrderingPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kPlaySound, kWaitForSound };
	enum PuzzleType { kOrdering, kPiano, kOrderItems, kKeypad, kKeypadTerse };
	OrderingPuzzle(PuzzleType type) : RenderActionRecord(7), _puzzleType(type) {}
	virtual ~OrderingPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override;
	bool isViewportRelative() const override { return true; }

	void pushDown(uint id);
	void setToSecondState(uint id);
	void popUp(uint id);
	void clearAllElements();

	Common::Path _imageName;
	bool _hasSecondState = false;
	bool _itemsStayDown = true;
	bool _needButtonToCheckSuccess = false;
	bool _checkOrder = true;
	Common::Rect _checkButtonSrc;
	Common::Rect _checkButtonDest;
	Common::Array<Common::Rect> _down1Rects;
	Common::Array<Common::Rect> _up2Rects;
	Common::Array<Common::Rect> _down2Rects;
	Common::Array<Common::Rect> _destRects;
	Common::Array<Common::Rect> _hotspots;
	Common::Array<uint16> _correctSequence;

	uint16 _specialCursor1Id = CursorManager::kHotspot;
	Common::Rect _specialCursor1Dest;
	uint16 _specialCursor2Id = CursorManager::kHotspot;
	Common::Rect _specialCursor2Dest;

	Common::Array<Common::String> _pianoSoundNames; // nancy8 and up

	uint16 _state2InvItem = 0;
	Common::Array<Common::Rect> _overlaySrcs;
	Common::Array<Common::Rect> _overlayDests;

	Nancy::SoundDescription _pushDownSound;
	Nancy::SoundDescription _itemSound;
	Nancy::SoundDescription _popUpSound;

	SceneChangeWithFlag _solveExitScene;
	uint16 _solveSoundDelay = 0;
	Nancy::SoundDescription _solveSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	SolveState _solveState = kNotSolved;
	Graphics::ManagedSurface _image;
	Common::Array<uint16> _clickedSequence;
	Common::Array<bool> _downItems;
	Common::Array<bool> _secondStateItems;
	Time _solveSoundPlayTime;
	bool _checkButtonPressed = false;

	PuzzleType _puzzleType;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ORDERINGPUZZLE_H
