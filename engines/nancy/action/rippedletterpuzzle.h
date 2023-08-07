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

#ifndef NANCY_ACTION_RIPPEDLETTERPUZZLE_H
#define NANCY_ACTION_RIPPEDLETTERPUZZLE_H

#include "engines/nancy/action/actionrecord.h"

namespace Nancy {

struct RippedLetterPuzzleData;

namespace Action {

class RippedLetterPuzzle : public RenderActionRecord {
public:
	enum SolveState { kNotSolved, kWaitForSound };
	RippedLetterPuzzle() : RenderActionRecord(7), _pickedUpPiece(8) {}
	virtual ~RippedLetterPuzzle() {}

	void init() override;
	void registerGraphics() override;
	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

	Common::String _imageName;
	Common::Array<Common::Rect> _srcRects;
	Common::Array<Common::Rect> _destRects;
	Common::Rect _rotateHotspot;
	Common::Rect _takeHotspot;
	Common::Rect _dropHotspot;
	Common::Array<int8> _initOrder;
	Common::Array<byte> _initRotations;
	Common::Array<int8> _solveOrder;
	Common::Array<byte> _solveRotations;
	SoundDescription _takeSound;
	SoundDescription _dropSound;
	SoundDescription _rotateSound;
	SceneChangeWithFlag _solveExitScene;
	SoundDescription _solveSound;
	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	RenderObject _pickedUpPiece;
	int8 _pickedUpPieceID = -1;
	byte _pickedUpPieceRot = 0;

	Graphics::ManagedSurface _image;
	SolveState _solveState = kNotSolved;
	RippedLetterPuzzleData *_puzzleState = nullptr;

protected:
	Common::String getRecordTypeName() const override { return "RippedLetterPuzzle"; }
	bool isViewportRelative() const override { return true; }

	void drawPiece(const uint pos, const byte rotation, const int pieceID = -1);
};

} // End of namespace Action
} // End of namespace Nancy

#endif //NANCY_ACTION_RIPPEDLETTERPUZZLE_H
