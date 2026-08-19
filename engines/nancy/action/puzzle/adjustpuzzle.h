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

#ifndef NANCY_ACTION_ADJUSTPUZZLE_H
#define NANCY_ACTION_ADJUSTPUZZLE_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/action/actionrecord.h"

namespace Nancy {
namespace Action {

// Adjustment puzzle, new in Nancy14 (AR 178). Used for the plotter-repair
// puzzle: several elements each have a state the player nudges up/down with
// +/- controls. Testing the current settings matches the state tuple against a
// table of combinations to pick a result; hitting the "perfect" result solves
// the puzzle, any other shows a result overlay (Perfect-Vert / -Hor / -Mirror /
// -Lite, ...) and fails.
class AdjustPuzzle : public RenderActionRecord {
public:
	AdjustPuzzle() : RenderActionRecord(7) {}
	virtual ~AdjustPuzzle() {}

	void init() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	bool isViewportRelative() const override { return true; }

protected:
	Common::String getRecordTypeName() const override { return "AdjustPuzzle"; }

	// One adjustable element. rects[0]/rects[2] are the decrement/increment
	// hotspots; subRects are the per-state display frames (blitted at boundRect).
	struct Piece {
		Common::Rect rects[4];
		Common::Array<Common::Rect> subRects;
		byte initialState = 0;
		Common::Rect boundRect;
		int state = 0;
	};

	// A row of the result table: the required state of each piece, plus the
	// result index produced when the states match it.
	struct MatrixRow {
		Common::Array<int16> cols;
		byte result = 0;
	};

	struct Outcome {
		int16 sceneID = 0;
		int16 frameID = 0;
		FlagDescription flag;
		RandomSoundBlock sound;
	};

	int pieceControlAtCursor(const Common::Point &mousePos, int &delta) const;
	byte evaluateResult() const;
	void runTest();
	void redraw();
	void applyOutcome(const Outcome &outcome);

	// -- File data --
	Common::Path _imageName;			// 0x1c0
	int16 _field3d = 0;					// 0x3d
	Common::Array<Piece> _pieces;		// 0x3f

	Common::Rect _rectA5;				// 0xa5
	Common::Rect _rectB5;				// 0xb5, "test"/go control

	RandomSoundBlock _pieceSound;		// 0x4f, after the pieces
	RandomSoundBlock _testSound;		// 0xc5

	Common::Path _adjustName;			// 0x11b (plotter animation movie)
	bool _hasAdjustRect = false;
	Common::Rect _adjustRect;			// 0x11f
	RandomSoundBlock _adjustSound;		// 0x12f (present only when _adjustName is set)

	int16 _field18a = 0;						// 0x18a
	Common::Array<Common::Path> _overlayNames;	// 0x18c, result overlays
	Common::Array<MatrixRow> _matrix;			// 0x1b0

	byte _defaultResult = 0;	// 0x1c4
	byte _perfectResult = 0;	// 0x1c5
	Outcome _winScene;			// 0x21c (field0), scene at 0x21e
	Outcome _loseScene;			// 0x27d (field0), scene at 0x27f

	// Give-up hotspot (count-prefixed 23-byte trailer): click to leave the puzzle.
	Common::Rect _exitHotspot;
	uint16 _exitCursorType = 0;
	SceneChangeDescription _exitScene;
	FlagDescription _exitFlag;

	// -- Runtime state --
	Graphics::ManagedSurface _image;
	Common::Array<Graphics::ManagedSurface> _overlayImages;
	byte _resultIndex = 0;
	bool _showResult = false;
	bool _solved = false;
	bool _lost = false;
	bool _outcomeApplied = false;
	bool _exitRequested = false;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ADJUSTPUZZLE_H
