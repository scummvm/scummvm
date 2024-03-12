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

#ifndef NANCY_ACTION_ASSEMBLYPUZZLE_H
#define NANCY_ACTION_ASSEMBLYPUZZLE_H

#include "engines/nancy/action/actionrecord.h"
#include "engines/nancy/misc/mousefollow.h"

namespace Nancy {

struct AssemblyPuzzleData;

namespace Action {

// Minigame where the player is provided with the broken pieces of something
// (a piece of pottery in nancy6), and has to assemble it.
class AssemblyPuzzle : public RenderActionRecord {
public:
	AssemblyPuzzle() : RenderActionRecord(7) {}
	virtual ~AssemblyPuzzle() {}

	void init() override;
	void registerGraphics() override;

	void readData(Common::SeekableReadStream &stream) override;
	void execute() override;
	void handleInput(NancyInput &input) override;

protected:
	Common::String getRecordTypeName() const override { return "AssemblyPuzzle"; };
	bool isViewportRelative() const override { return true; }

	void rotateBase(bool ccw);

	struct Piece : Misc::MouseFollowObject {
		Common::Array<Common::Rect> srcRects;
		Common::Array<Common::Rect> destRects;

		Common::Rect startRect;
		uint16 correctRotation = 0;
		uint16 layer = 0;
		bool placed = false;

		int curRotation = 0;
	};

	Common::Path _imageName;

	uint16 _height = 0;

	Common::Rect _cwCursorDest;
	Common::Rect _ccwCursorDest;

	Common::Array<Piece> _pieces;

	SoundDescription _rotateSound;
	SoundDescription _pickUpSound;
	SoundDescription _placeDownSound;

	bool _allowWrongPieceHotspot = false;

	Common::Array<SoundDescription> _wrongPieceSounds;
	Common::Array<Common::String> _wrongPieceTexts;

	SceneChangeWithFlag _solveScene; // has 9999 in nancy6, so the puzzle doesn't auto-exit
	SoundDescription _solveSound;
	Common::String _solveText;

	SceneChangeWithFlag _exitScene;
	Common::Rect _exitHotspot;

	Graphics::ManagedSurface _image;

	int _pickedUpPiece = -1;
	int _curRotation = 0;
	uint _layersAssembled = 0;
	bool _completed = false;

	AssemblyPuzzleData *_puzzleState = nullptr;
};

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ASSEMBLYPUZZLE_H
